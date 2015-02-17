#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>

#include "atci_service.h"
#include "atcid_util.h"
#include "atci_touch_cmd.h"

// ============================================================ //
//define
// ============================================================ //
#define TP_EMU_RELEASE      0
#define TP_EMU_DEPRESS      1
#define TP_EMU_SINGLETAP    2
#define TP_EMU_DOUBLETAP    3

// ============================================================ //
// Global variable
// ============================================================ //
int lcm_width;
int lcm_height;
// ============================================================ //
// function prototype
// ============================================================ //


// ============================================================ //
//extern variable
// ============================================================ //

// ============================================================ //
//extern function
// ============================================================ //

void get_fb_info()
{
    int fd;
    struct fb_var_screeninfo vi;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        ALOGE("cannot open fb0");
        return;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        ALOGE("failed to get fb0 info");
        close(fd);
        return;
    }

    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3) || 0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
    {
	    lcm_width = (unsigned int)vi.yres;
    	lcm_height = (unsigned int)vi.xres;
	}
	else
    {
		lcm_width = (unsigned int)vi.xres;
    	lcm_height = (unsigned int)vi.yres;
	}

    ALOGD("LCM_WIDTH=%d, LCM_HEIGHT=%d", lcm_width, lcm_height);

    close(fd);
}

void rotate_relative_to_lcm(int *x, int *y)
{
    int temp;
    
	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3))
	{
		// X reverse; Y reverse
		*x = lcm_width - *x - 1;
        *y = lcm_height- *y - 1;
        ALOGD("[rotate_relative_to_lcm] 180");
	}
	else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
	{
		//X,Y change; X reverse
		temp = *x;
        *x = lcm_width - *y - 1;
        *y = temp;
        ALOGD("[rotate_relative_to_lcm] 270");
	}
	else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
	{
		//X, Y change;  Y reverse
		temp = *x;
        *x = *y;
        *y = lcm_height - temp - 1;
        ALOGD("[rotate_relative_to_lcm] 90");
    }    
	else
    {   
    	// no change
    	ALOGD("[rotate_relative_to_lcm] 0");
    }   
}

void touch_down(int dev, int x, int y)
{
    char touch_event_1[] = "sendevent /dev/input/event3 3 48 40";   //ABS_MT_TOUCH_MAJOR
    char touch_event_2[] = "sendevent /dev/input/event3 3 57 0";    //ABS_MT_TRACKING_ID
    char touch_event_3[] = "sendevent /dev/input/event3 1 330 1";   //BTN_TOUCH
    char touch_event_4[50];                                         //ABS_MT_POSITION_X
    char touch_event_5[50];                                         //ABS_MT_POSITION_Y
    char touch_event_6[] = "sendevent /dev/input/event3 0 2 0";     //SYN_MT_REPORT
    char touch_event_7 [] = "sendevent /dev/input/event3 0 0 0";    //SYN_REPORT

    snprintf(touch_event_4, sizeof(touch_event_4), "sendevent /dev/input/event3 3 53 %d", x);
    snprintf(touch_event_5, sizeof(touch_event_5), "sendevent /dev/input/event3 3 54 %d", y);

    system(touch_event_1);
    system(touch_event_2);
    system(touch_event_3);
    system(touch_event_4);
    system(touch_event_5);
    system(touch_event_6);
    system(touch_event_7);    

    return;
}

void touch_up()
{
    char touch_event_1[] = "sendevent /dev/input/event3 1 330 0";   //BTN_TOUCH
    char touch_event_2[] = "sendevent /dev/input/event3 0 2 0";     //SYN_MT_REPORT
    char touch_event_3[] = "sendevent /dev/input/event3 0 0 0";     //SYN_REPORT   

    system(touch_event_1);
    system(touch_event_2);
    system(touch_event_3);

    return;
}

void touch_emu_cmd(TP_POINT points[], int point_num)
{
    int i;
    int down_flag;

    //ALOGD("[touch_emu_cmd]:num=%d", point_num);
    
    for (i=0; i<point_num; i++)
    {
        //ALOGD("[touch_emu_cmd]act=%d,x=%d,y=%d", points[i].action, points[i].x, points[i].y);
        
        if (points[i].action == TP_EMU_DEPRESS) //Down
        {
            ALOGD("[DOWN %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_down(3, points[i].x, points[i].y);
            down_flag = 1;
        }
        else if(points[i].action == TP_EMU_RELEASE) //Up
        {
            ALOGD("[UP %d]:(%d, %d)", i, points[i].x, points[i].y);
            if (down_flag) touch_down(3, points[i].x, points[i].y);
            touch_up();
            break;
        }
        else if(points[i].action == TP_EMU_SINGLETAP) //Single tap
        {
            ALOGD("[SingleTap %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_down(3, points[i].x, points[i].y);
            touch_up();
            break;
        }
        else if(points[i].action == TP_EMU_DOUBLETAP) //Double tap
        {
            ALOGD("[DoubleTap %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_down(3, points[i].x, points[i].y);
            touch_up();
            touch_down(3, points[i].x, points[i].y);
            touch_up();
            break;
        }

    }
}

int touch_cmd_parser(char *cmdline, TP_POINT points[], int *point_num)
{
    int ret = 0;
    int num = 0;
    int valid_para = 0;
    char legency_cmd[200] = {0};
    char *pcmd = cmdline;

    get_fb_info();
    
    while(1)
    {
        if(num == 0)
            valid_para = sscanf(pcmd, "%d,%d,%d%199s", &points[num].action, &points[num].x, &points[num].y, legency_cmd);
        else
            valid_para = sscanf(legency_cmd, "%*[^=]=%d,%d,%d%199s", &points[num].action, &points[num].x, &points[num].y, legency_cmd);


        ALOGD("[Point%d]:%d,%d,%d,(%s); para_num=%d\n", num, points[num].action, points[num].x, points[num].y, legency_cmd, valid_para);

        // (0,0) is fixed at left-up
        // rotate_relative_to_lcm(&points[num].x, &points[num].y);      
        num++;

        if (valid_para < 3) // parameter is wrong
        {
            ret = -1;
            break;
        }
        else if (valid_para == 3) // parser end
        {
            break;
        }
        else if (valid_para > 3) // parser continue
        {
            continue;
        }
    }
    *point_num = num;

    return ret;

}


int touch_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    int ret = 0;
    int point_num = 0;
    TP_POINT points[10];
    
    ALOGD("touch cmdline=%s, at_op=%d, \n", cmdline, at_op);
    
    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
            sprintf(response, "\r\n+CTSA: (0,1,2,3)\r\n");
            break;
        case AT_SET_OP:
            ret = touch_cmd_parser(cmdline, points, &point_num);
            if (ret < 0)
            {
                sprintf(response,"\r\n+CME ERROR: 50\r\n");
                break;
            }
            touch_emu_cmd(points, point_num);
            sprintf(response,"\r\nOK\r\n");
            break;
    default:
        break;
    }

    return 0;
}

