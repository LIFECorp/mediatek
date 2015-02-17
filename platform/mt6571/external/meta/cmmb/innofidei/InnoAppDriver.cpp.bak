/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                                             */
/*                                                                       */
/*      InnoAppdriver.cpp                                                 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Implementation Of Application driver interface For Linux	 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*************************************************************************/

#include <string.h>
#include "fcntl.h"
#include <sys/ioctl.h>
#include <sys/mman.h>                                              /* mmap() */
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <linux/types.h>
#include "InnoAppDriver.h"
#include "if208.h"                          //ioctl
#include "CmmbHelper.h"

#define _check_fw_reset                                                      //check fw reset, when no einit
//#define _save_mfs_file

#ifdef _save_mfs_file
FILE* fd1;   
#define MaxSaveFizeSize (1024*56*60*10)                         //save 10minutes
#endif
int flag_channle0=0;
int flag_channle1=0;
int flag_channle2=0;                              //xingyu 0922
unsigned char g_readbuffer[1024*100];           //set to 100k more than the max mfs 60k
#define FILE_NAME  "/dev/innodev0"

#define NUM_HANDLES			4                      //inno device num  //xingyu 0922            //xingyu 0922

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CMMB_AppDriver"
#include "utils/Log.h"
#define INNOAPPDRIVER_ENABLE_LOGS
#ifdef INNOAPPDRIVER_ENABLE_LOGS
//#define ADR_LOGE(fmt, args...) printf( "appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGE(fmt, args...) LOGE( "appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGW(fmt, args...) LOGW( "appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGI(fmt, args...) LOGI( "appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGD(fmt, args...) LOGD( "appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#else
#define ADR_LOGE(fmt, args...) 
#define ADR_LOGW(fmt, args...)
#define ADR_LOGI(fmt, args...)
#define ADR_LOGD(fmt, args...)
#endif

#define n_max_size                              (0x400*100)            //xingyu max read size
///////////////////////////////////////////////////////////////////////////////
typedef struct _FILE_HANDLES
{
	unsigned int handle_num;		
	int fd; 			
	int IsStop;				
	DemuxFrameCallBack callback;		
} FILE_HANDLES;

typedef struct _CHAR_DEVICES
{
	FILE_HANDLES handles[NUM_HANDLES];	
	struct pollfd fdpoll[NUM_HANDLES];	        
	//struct INNOCHAR_FW_DATA fw_data;	
        void* pStreamTask; 

	int HandlesNum;
	int thread_complete;                               
} CHAR_DEVICES;

int DriverIsInit=0;
CHAR_DEVICES charDev;

static int StreamThread(void* param)
{
	FILE_HANDLES *FileHd;
	int len;
	int channel_id;
#if 1                       //setting the thread prioity
	struct sched_param sched_p;
	sched_p.sched_priority = 90;       

	if(0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
		ADR_LOGE("sched_setscheduler failed");
	} else {
		sched_p.sched_priority = 0;
		sched_getparam(0, &sched_p);
		ADR_LOGI("sched_setscheduler ok, priority: %d",sched_p.sched_priority);
	}
#endif
	charDev.thread_complete = FALSE;
	FileHd = &charDev.handles[0];

	int i,j;
#ifdef _check_fw_reset
	static int count_timeout=0;
#endif
#ifdef _save_mfs_file
	int bufsize = 0;
#endif
	ADR_LOGI("Datapush thread for get mfs file, poll timeout  10s");
	while ( !charDev.handles[0].IsStop ){
		int count_pollin = 0;
		if (poll(&charDev.fdpoll[0], NUM_HANDLES,10000 ) < 0){    //xingyu 0922                      //three channel:channel 0 channel 1  2 and uamerror channel     //xingyu 0922
			ADR_LOGE("poll error");
			break;
		}
		///  check if there is data ready
		for (i=0; i < NUM_HANDLES; i++){                 //xingyu 0922
			if ((charDev.fdpoll[i].revents & POLLIN) != 0){
				count_pollin++;
				FileHd = &charDev.handles[i]; 
				if (!FileHd->fd){
					ADR_LOGE("FileHd->fd is not open");
					return -1;
				}

				// read data from device					
				len = read(FileHd->fd, g_readbuffer, n_max_size);
#if 0  					//check copy to user mfs       xingyu
				ADR_LOGI("check copy to user mfs:len=%d,buf",len);
				for(j=0;j<6;j++)
					ADR_LOGI(" %d",g_readbuffer[j]);
				ADR_LOGI("end");
#endif					
				if(len < 0 ){
					ADR_LOGE("error - read mfs len =%d <0 ",len);   		//sanity check
				}
				else{
#ifdef _save_mfs_file
					bufsize+=len;
					if(bufsize<MaxSaveFizeSize)
						fwrite(g_readbuffer+1, 1, len, fd1);
					else
						ADR_LOGE("save buffer full");
#endif
					ADR_LOGI("callback is called,len=%d,channel_id=%d",len,g_readbuffer[0]);
					if(g_readbuffer[0]==3){                          //xingyu 0922
						int* guam_err = (int*)(g_readbuffer+1);
						ADR_LOGW("Warning:uam error status:0x%x",*guam_err);
					}
				FileHd->callback((unsigned char*)(g_readbuffer+1),len,g_readbuffer[0]);   
                                        ADR_LOGI("callback ok");
				}
			}	
		}
		if (!count_pollin){
			if(flag_channle2 ||flag_channle1 || flag_channle0){                                //if already start servcie,check no eint issue     //xingyu 0922
				ADR_LOGW("Warning:No data ready,flag_channle0=%d,flag_channle1=%d",flag_channle0,flag_channle1);
#ifdef _check_fw_reset			
				count_timeout++;
				if(count_timeout%2 ==0){              //if 2*10s,no Eint,check fw and signal quality
					count_timeout =0;
					ADR_LOGW("Warning:20s no data! Check FW reset and signal quality?");
					if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
						ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
						charDev.thread_complete = TRUE;
						return INNO_GENERAL_ERROR;
					}
				}
#endif			
			}
			//break; 
		}
	}
	ADR_LOGI("exit");
	charDev.thread_complete = TRUE;
	return 0;
}

int InnoOpenHd( int handle_num,DemuxFrameCallBack data_callback )
{
	FILE_HANDLES *FileHd;
	int j;
	char file_name[32];
	if(!data_callback){
		ADR_LOGE("Error:data_callback == NULL");
		return -1;
	}	
	for(j=0;j<handle_num;j++){
		FileHd = &charDev.handles[j];
		FileHd->IsStop = 0;
		FileHd->callback = data_callback;
		FileHd->handle_num = j;	
		sprintf(file_name, "/dev/innodev%d", j);		
		FileHd->fd = open(file_name, O_RDWR );
		if(FileHd->fd ==-1){
			ADR_LOGE("open file error");
			return -1;
		}	
		charDev.fdpoll[j].fd = FileHd->fd;
		charDev.fdpoll[j].events = POLLIN | POLLNVAL;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoAppDriverInit(DemuxFrameCallBack data_callback)
{
	int result =0;
	/**************** step1   variable init     *************/
	ADR_LOGI("step1 --char device create");
	if (DriverIsInit) {
		ADR_LOGW("Warning:Inno Adr already init and not deinit, should not init again");
		return 	INNO_NO_ERROR; 
	}
	charDev.HandlesNum =NUM_HANDLES;
	//callback function	
	result =InnoOpenHd(NUM_HANDLES,data_callback);
	if(result){
		ADR_LOGE("InnoOpenHd failed");
		return INNO_GENERAL_ERROR;				
	}
#ifdef  _save_mfs_file
	fd1 = fopen("/data/out.mfs", "wb");	                            
	if(fd1 ==-1){
		ADR_LOGE("open /data/out.mfs file error");
		return INNO_GENERAL_ERROR;
	}
#endif
#ifdef _check_fw_reset			
	flag_channle1 =0;                                                    //debug for check no eint issue
	flag_channle0 = 0;
	flag_channle2 = 0;                                    //xingyu 0922
#endif		
	/**************** step2   thread create     *************/
	//data callback thread
	ADR_LOGI("step2 --data push thread create");
	char taskname[20];
	snprintf(taskname,
			16,
			"InnoRxDataThread");
	charDev.pStreamTask = OSAL_TaskCreate(taskname,
			1, //normal
			( 0x800 ),
			(TaskFunc)StreamThread,
			(void*)taskname );
	if(!charDev.pStreamTask){
		ADR_LOGE("TaskCreate StreamThread fail");
		return INNO_GENERAL_ERROR;
	}	
	/**************** step3   Download fw     *************/
	ADR_LOGI("step3 --download fw using h file");
	FILE_HANDLES *FileHd = &charDev.handles[0];
	//charDev.fw_data.fw_size = FIRMWARE_BUF_DATA_SIZE;
	// alloc fw_buf
	//charDev.fw_data.fw_buf = (char*)malloc(charDev.fw_data.fw_size);
	//memcpy(charDev.fw_data.fw_buf ,fw_buf,FIRMWARE_BUF_DATA_SIZE);    //memcpy for save protect fw not modify
#if 0                                                                          //check fw bytes
	int i;
	ADR_LOGI("check fw buf");
	for(i=0;i<4;i++)
		ADR_LOGI(" %02x",charDev.fw_data.fw_buf [i]);
	ADR_LOGI("end");
	for(i=0;i<4;i++)
		ADR_LOGI(" %02x",charDev.fw_data.fw_buf [FIRMWARE_BUF_DATA_SIZE-i-1]);
	ADR_LOGI("end");	
#endif
	//if (0 > ioctl(FileHd->fd, INNO_FW_DOWNLOAD, &charDev.fw_data))    // fw download from user space to kernel
        if (0 > ioctl(FileHd->fd, INNO_FW_DOWNLOAD, 1))                     // fw download in kernel space
	{
		ADR_LOGE("INNO_FW_DOWNLOAD failed %d", errno);
		return INNO_GENERAL_ERROR;
	}
	//free(charDev.fw_data.fw_buf);
	//charDev.fw_data.fw_buf = NULL;

	DriverIsInit = TRUE;                              ///driver init success
	ADR_LOGI("ok");
	return INNO_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoAppDriverDeinit()
{
	void* pTask;
	INNO_RET retCode=INNO_NO_ERROR;
	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_NO_ERROR;
	}
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];
	FileHd->IsStop = 1;

	// stop polling
	ADR_LOGI("stop polling");
	if (0 > ioctl(FileHd->fd, INNO_STOP_POLL,1)){
		ADR_LOGE("INNO_STOP_POLL failed %d", errno);
		DriverIsInit =FALSE;
		return INNO_GENERAL_ERROR;
	}

	//double check - close data push thread, wait till thread is done
	ADR_LOGI("stop thread charDev.handles[0].IsStop =%d",charDev.handles[0].IsStop);
	while(!charDev.thread_complete){OSAL_TaskSleep(10);}
	pTask = charDev.pStreamTask;
	OSAL_TaskCleanup(pTask);

	ADR_LOGI("stop polling ok,then close the stop poll flag");
	if (0 > ioctl(FileHd->fd, INNO_STOP_POLL,0)){
		ADR_LOGE("INNO_STOP_POLL failed %d", errno);
		DriverIsInit =FALSE;
		return INNO_GENERAL_ERROR;
	}

	//close handle
	int i=0;
	for(i=0;i<NUM_HANDLES;i++){
		FileHd = &charDev.handles[i];
		if(0>close(FileHd->fd)){
			ADR_LOGE("close fd i=%d",i);
			DriverIsInit =FALSE;
			return INNO_GENERAL_ERROR;
		}
		FileHd->fd = 0;
	}

	DriverIsInit = FALSE;
#ifdef _save_mfs_file
	fclose(fd1);
	fd1 =0;
#endif
	ADR_LOGI("OK");
	return INNO_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////////
INNO_RET InnoMfsMemset(int flag)
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];
	ADR_LOGI("Enter,flag =%d",flag);
	if (0 > ioctl(FileHd->fd, INNO_MEMSET_MFS,flag))	{
		ADR_LOGE("iotcl INNO_MEMSET_MFS failed %d", errno);
		return INNO_GENERAL_ERROR;
	}
        ADR_LOGI("OK");
	return INNO_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoSetTunerFrequency(BYTE freq_dot)
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];
	unsigned long tmp_freq_dot = (unsigned long)freq_dot;
        ADR_LOGI("Enter");
	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_SET_FREQUENCY_DOT, &tmp_freq_dot))
	{
		ADR_LOGE("INNO_SET_FREQUENCY_DOT failed %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}

	// check tune if success sync
	OSAL_TaskSleep(500);                                    
	struct inno_sys_state tmp_sys_state;
	memset(&tmp_sys_state,0,sizeof(struct inno_sys_state));
	tmp_sys_state.stattype = STATTYPE_SYNC_STATE;

	INNO_RET RetCode  = InnoGetSysStatus(&tmp_sys_state);
	if ( RetCode != INNO_NO_ERROR ){
		ADR_LOGE(" InnoGetSysStatus failed Return err 0x%x",RetCode);              
		return RetCode ;
	}
	else if(1 == tmp_sys_state.statdata.sync_state)
		ADR_LOGI("InnoGetSysStatus:freq_dot =%d sync ok",freq_dot);
	else{
		ADR_LOGW("Warning:InnoGetSysStatus:freq_dot =%d sync fail",freq_dot);
		return INNO_GENERAL_ERROR;
	}
	return INNO_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoSetChannelConfig(BYTE channel_id, BYTE ts_start, BYTE ts_count, BYTE demod,BYTE subframe_ID)
{
	FILE_HANDLES *FileHd;
	struct inno_channel_config 	cfg;
        ADR_LOGI("Enter");
	if(channel_id>2 ){                 // onle two channel for use,0 and 1 2             //xingyu 0922
		ADR_LOGW("warning:channel_id>1,should 0 or 1");
		return INNO_GENERAL_ERROR;
	}
	FileHd = &charDev.handles[0];
	memset(&cfg,0,sizeof(struct inno_channel_config));

	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}
	cfg.ch_id    = channel_id;
	cfg.ts_start = ts_start;
	cfg.ts_count = ts_count;
	cfg.modulate = (demod & 0xc0)>>6 &0x03 ;
	cfg.rs = ((demod & 0x30)>>4) &0x03 ;
	cfg.itlv = ((demod & 0x0c)>>2) & 0x03;
	cfg.ldpc = demod & 0x03;
	cfg.subframe_ID = subframe_ID;

	if (0 > ioctl(FileHd->fd, INNO_SET_CHANNEL_CONFIG, &cfg))	{
		ADR_LOGE("iotcl INNO_SET_CHANNEL_CONFIG failed %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}
#ifdef _check_fw_reset				   // check which channel is open,then determine whether check no eint issue       
	if(channel_id==0)
		flag_channle0= 1;
	else if(channel_id ==1)
		flag_channle1 = 1;
	else if(channel_id ==2)
		flag_channle2 = 1;                        //xingyu 0922
	ADR_LOGD("open channel=%d,flag_channle0=%d,flag_channle1=%d,start check fw reset",channel_id,flag_channle0,flag_channle1);	   
#endif	
        ADR_LOGI("OK");
	return INNO_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoGetChannelConfig(BYTE channel_id, BYTE *ts_start, BYTE *ts_count, BYTE *demod,BYTE* subframe_ID)
{
	FILE_HANDLES *FileHd;
	struct inno_channel_config 	cfg;
        ADR_LOGI("Enter");
	if(channel_id>2 ){                 // onle two channel for use, 0 and 1 2   //xingyu 0922
		ADR_LOGW("warning:channel_id>1,should 0 or 1");
		return INNO_GENERAL_ERROR;
	}
	FileHd = &charDev.handles[0];
	cfg.ch_id = channel_id;

	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_GET_CHANNEL_CONFIG, &cfg)){
		ADR_LOGE("ioctl INNO_GET_CHANNEL_CONFIG fail %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}
	*ts_start = cfg.ts_start;
	*ts_count = cfg.ts_count;
	*demod = (cfg.modulate<<6) | (cfg.rs <<4) | (cfg.itlv<<2) | cfg.ldpc;
	*subframe_ID = cfg.subframe_ID;
	ADR_LOGI("ok");
	return INNO_NO_ERROR;
}
///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoCloseChannel(BYTE channel_id)
{
	FILE_HANDLES *FileHd;
	struct inno_channel_config 	cfg;
        ADR_LOGI("Enter");
	if(channel_id>2 ){                 // onle two channel for use, 0 and 1 2        //xingyu 0922
		ADR_LOGW("warning:channel_id>1,should 0 or 1");
		return INNO_GENERAL_ERROR;
	}
	FileHd = &charDev.handles[0];
	memset(&cfg,0,sizeof(struct inno_channel_config));

	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	cfg.ch_id    = channel_id;
	cfg.ch_close = 1;

	if (0 > ioctl(FileHd->fd, INNO_SET_CHANNEL_CONFIG, &cfg))	{
		ADR_LOGE("ioctl INNO_SET_CHANNEL_CONFIG failed %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}
	// check which channel is open,then determine whether check no eint issue
#ifdef _check_fw_reset			        
	if(channel_id==0)
		flag_channle0= 0;
	else if(channel_id ==1)
		flag_channle1 = 0;
	else if(channel_id ==2)                //xingyu 0922
		flag_channle2 = 0;
	ADR_LOGD("close channel=%d,flag_channle0=%d,flag_channle1=%d,stop check fw reset",channel_id,flag_channle0,flag_channle1);
#else
	ADR_LOGD("Ok");
#endif	
	return INNO_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
INNO_RET InnoGetSysStatus(struct inno_sys_state* sys_state)
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];
        ADR_LOGI("Enter");
	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_GET_SYS_STATE, sys_state))	{
		ADR_LOGE("ioctl INNO_GET_SYS_STATE failed %d", errno);
		return INNO_GENERAL_ERROR;
	}
        ADR_LOGI("ok");
	return INNO_NO_ERROR;
}

INNO_RET InnoUamInit()
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];
        ADR_LOGI("Enter");
	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_UAM_INIT, 1))	{
		ADR_LOGE("ioctl INNO_UAM_INIT failed %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}
        ADR_LOGI("OK");
	return INNO_NO_ERROR;
}

INNO_RET InnoUamReset(BYTE *pATRValue, unsigned int *pATRLen)
{
	return INNO_NO_ERROR;
}

INNO_RET InnoUamTransfer(BYTE *pBufIn, unsigned int bufInLen, BYTE *pBufOut, unsigned short *pBufOutLen, unsigned short *sw)
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];

	struct inno_uam_parameter uam_par;
        ADR_LOGI("Enter");
	uam_par.bufInLen = bufInLen;
	if(pBufIn)
		memcpy(uam_par.pBufIn,pBufIn,bufInLen);
	else{
		ADR_LOGE("error pBufIn==NULL");	
		return INNO_GENERAL_ERROR;
	}

	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_UAM_TRANSFER, &uam_par))	{
		ADR_LOGE("ioctl INNO_UAM_TRANSFER failed %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}

	*pBufOutLen = uam_par.pBufOutLen;
	*sw = uam_par.sw;
	ADR_LOGI("pBufOutLen=%d,sw=%d",*pBufOutLen,*sw);                      //sw = status, used for judge success or not?
	if(pBufOut)
		memcpy(pBufOut,uam_par.pBufOut,uam_par.pBufOutLen);
	else{
		ADR_LOGE("error pBufIn==NULL");	
		return INNO_GENERAL_ERROR;
	}
        ADR_LOGI("OK");
	return INNO_NO_ERROR;    
}

INNO_RET Inno_Send_UAM_Cmd0(unsigned char* cmd, int len)
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];

	uam_cmd_par tmp_par;
	tmp_par.cmd = cmd;
	tmp_par.len = len;

	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_SEND_UAM_CMD,&tmp_par)){
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		ADR_LOGE("ioctl INNO_SEND_UAM_CMD fail %d", errno);
		return INNO_GENERAL_ERROR;
	}
	return INNO_NO_ERROR;      
}

INNO_RET Inno_Send_Cmd(unsigned char* cmd)
{
	FILE_HANDLES *FileHd;
	FileHd = &charDev.handles[0];

	if (!DriverIsInit) {
		ADR_LOGW("Warning:Inno driver not init yet");
		return INNO_GENERAL_ERROR;
	}

	if (0 > ioctl(FileHd->fd, INNO_SEND_CMD, cmd)){
		ADR_LOGE("ioctl INNO_SEND_CMD fail %d", errno);
		// check fw reset
		if (0 > ioctl(FileHd->fd, INNO_GET_FW_BYTES,1)){
			ADR_LOGE("INNO_GET_FW_BYTES failed %d", errno);
			return INNO_GENERAL_ERROR;
		}
		return INNO_GENERAL_ERROR;
	}
	return INNO_NO_ERROR;
}
INNO_RET Inno_SetUAMOver(void)  
{
	/* 
	   CHIP_V5 CMD Description:
	   A command identify that a group of UAM APDU is over 

	   CMD[0] = CMD_SET_UAM_OVER
	   CMD[1] = 0
	   CMD[2] = 0
	   CMD[3] = 0
	   CMD[4] = 0
	   CMD[5] = 0
	   CMD[6] = 0
	   CMD[7] = 0
	 */
	INNO_RET ret = INNO_GENERAL_ERROR;
	unsigned char cmd[8] = {0};
	int retry = 0;

        ADR_LOGI("Enter");
	for(retry = 0; retry < 20; retry ++){
		// Send CMD
		cmd[0] = CMD_SET_UAM_OVER;
		ret = Inno_Send_Cmd(cmd);  
		if(ret != INNO_NO_ERROR)
			continue;
		else {		
			ADR_LOGI("INNO_Send_Cmd ok!");
			return ret;
		}
	}
	ADR_LOGE("Error INNO_Send_Cmd timeout,ret =%d",ret);
	return ret;
}
INNO_RET Inno_SetCardEnv(unsigned char airnetwork)  
{
	INNO_RET ret = INNO_GENERAL_ERROR;
	unsigned char cmd[8] = {0};
	int retry = 0;

        ADR_LOGI("Enter");
	for(retry = 0; retry < 20; retry ++){
		// Send CMD
		cmd[0] = CMD_SET_CARD_ENV;
		cmd[1] = airnetwork;		

		ret = Inno_Send_Cmd(cmd);     
		if(ret != INNO_NO_ERROR)
			continue;
		else{
			ADR_LOGI("Set Card Envirment ok!_%dG", airnetwork);
			return ret;
		}
	}
	ADR_LOGE("Error INNO_Send_Cmd timeout ret =%d",ret);
	return ret;
}


INNO_RET Inno_Set_MBBMS_ISMA(unsigned char isBase64, CMBBMS_ISMA mbbms_isma)
{
	unsigned char cmd[50] = {0};
	INNO_RET ret = INNO_GENERAL_ERROR;
	unsigned int retry = 0;

        ADR_LOGI("Enter");
	cmd[0] = CMD_SET_MBBMS_ISMA;
	cmd[1] = 0xAA;
	cmd[2] = 0x55;
	cmd[3] = 0x00;
	cmd[4] = 0x27;
	cmd[5] = 0x00;
	cmd[6] = 0x00;
	cmd[7] = 0x00;
	cmd[8] = isBase64;

	cmd[9] = (unsigned char)mbbms_isma.MBBMS_ECMDataType;
	memcpy(cmd+10, mbbms_isma.ISMACrypAVSK[0].ISMACrypSalt,18);
	cmd[28] = mbbms_isma.ISMACrypAVSK[0].SKLength;   
	memcpy(cmd+29, mbbms_isma.ISMACrypAVSK[1].ISMACrypSalt,18);
	cmd[47] = mbbms_isma.ISMACrypAVSK[1].SKLength;

	//	ADR_LOGD("Set ISMA---ECMDataType = 0x%x", (unsigned char)mbbms_isma.MBBMS_ECMDataType);
	//	ADR_LOGD("0---%s, %d", mbbms_isma.ISMACrypAVSK[0].ISMACrypSalt, mbbms_isma.ISMACrypAVSK[0].SKLength);
	//	ADR_LOGD("1---%s, %d", mbbms_isma.ISMACrypAVSK[1].ISMACrypSalt, mbbms_isma.ISMACrypAVSK[1].SKLength);

	for(retry = 0; retry < 20; retry ++){
		ret = Inno_Send_UAM_Cmd0(cmd, 48);

		if(ret != INNO_NO_ERROR)
			continue;
		else {	
			ADR_LOGI("Set MBBMS ISMA ok!ECMDataType = 0x%x",(unsigned char)mbbms_isma.MBBMS_ECMDataType);
			return ret;
		}
	}
	ADR_LOGE("Error INNO_Send_UAM_Cmd0 timeout ret =%d",ret);
	return ret;
}
INNO_RET Inno_Set_UAM_AID_3G(unsigned char *uam_aid, unsigned char aid_len)
{
	unsigned char cmd[50] = {0};
	INNO_RET ret = INNO_GENERAL_ERROR;
	unsigned int retry = 0;

        ADR_LOGI("Enter");
	if((uam_aid == NULL)/*||(aid_len < 0)*/)
		return INNO_PARAMETER_ERROR;

	cmd[0] = CMD_SET_UAM_AID_3G;
	cmd[1] = 0xAA;
	cmd[2] = 0x55;
	cmd[3] = 0x00;
	cmd[4] = 0x10;
	cmd[5] = 0x00;
	cmd[6] = 0x00;
	cmd[7] = 0x00;
	cmd[8] = 0x00;

	memcpy(cmd+9, uam_aid, aid_len);

	for(retry = 0; retry < 20; retry ++)
	{
		ret = Inno_Send_UAM_Cmd0(cmd, aid_len+9);

		if(ret != INNO_NO_ERROR)
			continue;
		else 
		{			
			ADR_LOGI("Set UAM 3G AID ok!");
			return ret;
		}
	}
	ADR_LOGE("Error INNO_Send_UAM_Cmd0 timeout ret =%d",ret);
	return ret;
}

#if 0                                    //xingyu now no use
unsigned long ParseErrStatus(unsigned char status)
{
	unsigned long ret = 0;

	switch ( status )
	{
		case CAS_OK:
			ret = 0x9000;
			break;
		case NO_MATCHING_CAS:   //Can not find ECM data
			ret = NO_MATCHING_CAS;
			break;
		case CARD_OP_ERROR:    //Time out or err
			ret = CARD_OP_ERROR;
			break;
		case MAC_ERR:
			ret = 0x9862;
			break;
		case GSM_ERR:
			ret = 0x9864;
			break;
		case KEY_ERR:
			ret = 0x9865;
			break;
		case KS_NOT_FIND:
			ret = 0x6985;
			break;
		case KEY_NOT_FIND:
			ret = 0x6a88;
			break;
		case CMD_ERR:
			ret = 0x6f00;
			break;
		default:
			ret=(unsigned long)status;
			break;			
	}

	return ret;
}

void INNO_ReadErrStatus(unsigned long *errstatus)
{
	unsigned char status = 0;

	if(errstatus == NULL)
		return;

	INNO_SPI_Read_Byte_Type2(FETCH_PER_COMM29, &status, 1);

	//parse err status
	*errstatus = ParseErrStatus(status);
	return;
}
#endif	
