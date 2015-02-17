#define LOG_TAG "display_session_test"

#include <sys/mman.h>
#include <dlfcn.h>
#include <cutils/xlog.h>
#include <cutils/atomic.h>
#include <hardware/hardware.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/resource.h>
#include <linux/fb.h>
#include <wchar.h>
#include <pthread.h>
#include <linux/mmprofile.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <unistd.h>
#include "mtkfb.h"
#include <sync/sync.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <linux/disp_session.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

using namespace android;



//#pragma GCC optimize ("O0")

typedef struct {
    int ion_fd;
    int ion_shardfd;
    struct ion_handle * handle;
    void* buffer ;
    int fence_fd;
    int fence_idx;

}data_buffer_t;
//unsigned int bufsize=1024*1024*8+256;

static volatile char* inputBuffer;
static volatile char* outputBuffer;
//static struct fb_overlay_buffer overlaybuffer[4];
static disp_session_input_config session_input_config;
static disp_input_config fblayer[4];
//static int ion_fd[4];
static int display_fd;
#define LAYER_NUM 4
#define BUFFER_NUM 2
static data_buffer_t layerbuffer[LAYER_NUM][BUFFER_NUM];
#define  TIMEOUT_NEVER (-1)
static int layerindex[LAYER_NUM] ={0};
static int fbsize=0;

static int session_init_info(int width, int height)
{
	int i =0;
 	int layer = 0;
	void *inputBuffer = NULL;
	fbsize = width * height * 4;
	printf("width=%d, height=%d, fbsize=%d\n", width, height, fbsize);
	for(layer = 0;layer<LAYER_NUM;layer++)
	{
		for(i=0;i<BUFFER_NUM;i++)
	    	{ 
			int ion_fd = ion_open();
			unsigned int j =0;
			struct ion_handle* handle= NULL;
			int share_fd = -1;
			if (ion_fd < 0)
			{
				printf("Cannot open ion device\n");
				return 0;
			}

			if (ion_alloc_mm(ion_fd, fbsize, 4, 0, &handle))
			{
				printf("IOCTL[ION_IOC_ALLOC] failed!\n");
				return 0;
		        }

			if (ion_share(ion_fd, handle, &share_fd))
			{
			    	printf("IOCTL[ION_IOC_SHARE] failed!\n");
			    	return 0;
			}
			
			inputBuffer = ion_mmap(ion_fd, NULL, fbsize, PROT_READ|PROT_WRITE, MAP_SHARED, share_fd, 0);
			for (j=0; j< fbsize; j+=4)
			{
				#if 1
				//unsigned int value = (j%256) << (i*8);
				//if(i == 3) value = 0xffffff;
				unsigned int value = 0;
				#if 0
				unsigned int alpha = 0x22000000*layer;
				
				if(i==0) value = 0x00ffff;//blue
				else if(i==1) value = 0xff0000;//green
				else if(i==2) value = 0xffffff;//red
				else if(i==3) value = 0x000000;
				#endif
				
				if(layer == 0) value = i%2?0xff0000ff:0xffff0000;
				else if(layer == 1) value = i%2?0xffff0000:0xff00ff00;
				else if(layer == 2) value = i%2?0xff00ff00:0xff0000ff;
				else if(layer ==3) value = i%2?0xffffffff:0xff404040;

				*(volatile unsigned int*)(inputBuffer+j) = value;
				#endif
			}
			
			layerbuffer[layer][i].ion_fd = ion_fd;
			layerbuffer[layer][i].handle = handle;
			layerbuffer[layer][i].ion_shardfd = share_fd;
			layerbuffer[layer][i].buffer = inputBuffer;
			layerbuffer[layer][i].fence_fd = -1;
			layerbuffer[layer][i].fence_idx = 0;

			printf("layer=%d,index=%d ,ion_fd=%d\n", layer, i, ion_fd );
	    	}
	}

	return 1;
}

void releaseResource()
{
    	int i = 0;
	int layer = 0;
	for(layer=0;layer<LAYER_NUM;layer++)
	{
	    	for(i=0;i<BUFFER_NUM;i++)
	    	{ 
	        	ion_munmap(layerbuffer[layer][i].ion_fd, layerbuffer[layer][i].buffer , fbsize);
	        	ion_free(layerbuffer[layer][i].ion_fd, layerbuffer[layer][i].handle);
	        	ion_close(layerbuffer[layer][i].ion_fd);
			layerbuffer[layer][i].ion_fd = -1;
			printf("release resource, ion_fd=%d\n", layerbuffer[layer][i].ion_fd);
	    	}
	}
}

data_buffer_t * session_getfreebuffer(int layer_id)
{
	int index = layerindex[layer_id];
	if(layerbuffer[layer_id][index].fence_fd >= 0)
	{
	        int err = sync_wait(layerbuffer[layer_id][index].fence_fd, TIMEOUT_NEVER);
        	close(layerbuffer[layer_id][index].fence_fd);
        	layerbuffer[layer_id][index].fence_fd = -1;
    	}

	layerindex[layer_id]++;
	layerindex[layer_id] %= BUFFER_NUM;
    	//printf("get free buffer index=%d,ionfd=%d\n", index,layerbuffer[index].ion_fd);
    	return &(layerbuffer[layer_id][index]);
}

void session_prepare_input(int fd, unsigned int session_id, data_buffer_t * param, int layer_id, int layer_en)
{
	disp_buffer_info buffer;

	memset(&buffer, 0, sizeof(disp_buffer_info));
	buffer.session_id = session_id;
	buffer.layer_id   = layer_id;
	buffer.layer_en   = layer_en;
	buffer.ion_fd     = param->ion_shardfd;
	buffer.cache_sync = 0;
	ioctl(fd, DISP_IOCTL_PREPARE_INPUT_BUFFER, &buffer);
	param->fence_idx = buffer.index;
	param->fence_fd = buffer.fence_fd;
	//printf("prepare fence buffer idx=%d, fence_id=%d\n", param->fence_idx, param->fence_fd);
}

void session_setinput(int fd, disp_session_input_config *input_config, data_buffer_t * param, int width, int height, int layer_id, int layer_en)
{
	input_config->config_layer_num = 1;

	input_config->config[0].layer_id = layer_id;
	input_config->config[0].layer_enable = layer_en;
	input_config->config[0].src_base_addr = 0;
	input_config->config[0].src_phy_addr = 0;
	input_config->config[0].next_buff_idx = param->fence_idx;
	input_config->config[0].src_fmt = DISP_FORMAT_RGBA8888;
	input_config->config[0].src_pitch = width;
	input_config->config[0].src_offset_x = 0; 
	input_config->config[0].src_width = width;

	input_config->config[0].src_offset_y = height/4*layer_id;
	input_config->config[0].src_height = height/4;

	input_config->config[0].tgt_offset_x = 0;
	input_config->config[0].tgt_offset_y = height/4*layer_id;
	input_config->config[0].tgt_width = width;
	input_config->config[0].tgt_height = height/4;
	
	if(ioctl(fd, DISP_IOCTL_SET_INPUT_BUFFER, input_config) < 0)
    	{
       	 	printf("ioctl to set multi overlay layer enable fail\n");
    	}
}
static long int get_current_time_us(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}

#define TEST_RESULT(string, ret)	printf("Test Case: [%s] [%s]\n", string, (ret)?"PASS":"FAIL");
int main(int argc, char **argv)
{
	int ret = 0;
	int fd = 0;
	char dev_name[24];

	disp_session_config config;
	disp_session_info device_info;

	memset((void*)&config, 0, sizeof(disp_session_config));
	memset((void*)&device_info, 0, sizeof(disp_session_info));
	memset((void*)&session_input_config, 0, sizeof(disp_session_input_config));
	
	sprintf(dev_name, "/dev/%s", DISP_SESSION_DEVICE);
	printf("display session device is %s\n", dev_name);
	{
		fd = open(dev_name, O_RDWR);
		TEST_RESULT("OPEN DEVICE", (fd>0));
	}
	// blank SurfaceFlinger
	   sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	   sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
			   ISurfaceComposer::eDisplayIdMain);
	   SurfaceComposerClient::blankDisplay(display);

	// create session
	{
		config.type = DISP_SESSION_PRIMARY;
		config.device_id = 0;
		ret = ioctl(fd, DISP_IOCTL_CREATE_SESSION, &config);
		TEST_RESULT("DISP_IOCTL_CREATE_SESSION", (ret==0));
		printf("create session for primary display, session_id = %d\n", config.session_id);
	}

	// get session info
	{
		device_info.session_id = config.session_id;
		ret = ioctl(fd, DISP_IOCTL_GET_SESSION_INFO, &device_info);
		TEST_RESULT("DISP_IOCTL_GET_SESSION_INFO", (ret==0));
		printf("get primary display information, session_id=0x%08x\n", config.session_id);	
		printf("-->maxLayerNum=%d\n", device_info.maxLayerNum);
		printf("-->isHwVsyncAvailable=%d\n", device_info.isHwVsyncAvailable);
		printf("-->displayType=%d\n", device_info.displayType);
		printf("-->displayWidth=%d\n", device_info.displayWidth);
		printf("-->displayHeight=%d\n", device_info.displayHeight);
		printf("-->displayFormat=%d\n", device_info.displayFormat);
		printf("-->displayMode=%d\n", device_info.displayMode);
		printf("-->vsyncFPS=%d\n", device_info.vsyncFPS);
		printf("-->physicalWidth=%d\n", device_info.physicalWidth);
		printf("-->physicalHeight=%d\n", device_info.physicalHeight);
		printf("-->isConnected=%d\n", device_info.isConnected);
	}

	// prepare input buffer
	{
		ret = session_init_info(device_info.displayWidth, device_info.displayHeight);
		TEST_RESULT("init info", (ret>0));
	}

	int loop = 1000;
	if(argc > 1)
	{
	    	sscanf(argv[1], "%d", &loop);
	}
	
	printf("loop count %d \n",loop);
	disp_session_vsync_config vsync_config;
	
	memset((void*)&vsync_config, 0, sizeof(disp_session_vsync_config));
	vsync_config.session_id = config.session_id;
	unsigned int vsync_cnt = 0;
	long int last_vsync_ts = 0;
	long int t = 0;
	data_buffer_t * buffer;
	while(loop)
    	{   			
    		#if 0
		ret = ioctl(fd, DISP_IOCTL_WAIT_FOR_VSYNC, &vsync_config);
		if(ret)
		{
			TEST_RESULT("wait vsync", (ret==0));
		}
		#endif
		usleep(10000);
    		session_input_config.session_id = config.session_id;
        	buffer = session_getfreebuffer(0);
        	session_prepare_input(fd, config.session_id, buffer, 0, 1);
        	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 0, 1);

		#if 1
        	buffer = session_getfreebuffer(1);
        	session_prepare_input(fd,config.session_id,  buffer, 1, 1);
		session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 1, 1);
		
        	buffer = session_getfreebuffer(2);
        	session_prepare_input(fd, config.session_id, buffer, 2, 1);
		session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 2, 1);

        	buffer = session_getfreebuffer(3);
        	session_prepare_input(fd, config.session_id, buffer, 3, 1);
		session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 3, 1);
		#endif
		
		ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);
		if(ret)
		{
			TEST_RESULT("trigger session", (ret==0));
		}		
        	
        	loop--;
    	}
	
	buffer = session_getfreebuffer(3);
	session_prepare_input(fd, config.session_id, buffer, 3, 0);
	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 3, 0);
	
	buffer = session_getfreebuffer(1);
	session_prepare_input(fd, config.session_id, buffer,1, 0);
	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 1, 0);
	
	buffer = session_getfreebuffer(0);
	session_prepare_input(fd, config.session_id, buffer, 0, 0);
	session_setinput(fd, &session_input_config, buffer, device_info.displayWidth, device_info.displayHeight, 0, 0);
	ret = ioctl(fd, DISP_IOCTL_TRIGGER_SESSION, &config);

	// create session
	{
		config.type = DISP_SESSION_PRIMARY;
		config.device_id = 0;
		ret = ioctl(fd, DISP_IOCTL_DESTROY_SESSION, &config);
		TEST_RESULT("DISP_IOCTL_DESTROY_SESSION", (ret==0));
		printf("destroy session for primary display, session_id = %d\n", config.session_id);
	}
		
    	releaseResource();
		
		// unblank SurfaceFlinger
		SurfaceComposerClient::unblankDisplay(display);
		

    	return 0;
}
