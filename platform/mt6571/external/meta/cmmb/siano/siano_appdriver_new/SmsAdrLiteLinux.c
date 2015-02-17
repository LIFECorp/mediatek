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
/* Copyright (C) 2005,2006 Siano Mobile Silicon Ltd. All rights reserved */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                                             */
/*                                                                       */
/*      AppDriverLinux.c                                                 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Implementation Of Application driver interface For Linux	 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*************************************************************************/

#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>                                              /* mmap() */
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "SmsHostLibTypes.h"
#include "smsioctl.h"
#include "sms_msgs.h"
#include "SmsLiteAppDriver.h"
#include "Osw.h"

#define  _hfilefw
#ifdef _hfilefw
#include "cmmb_ming_app.h"
#endif

#define SMSHOST_ENABLE_LOGS
#ifdef SMSHOST_ENABLE_LOGS
#define ADR_LOGE(fmt, args...) ALOGE( "sms-appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGW(fmt, args...) ALOGW( "sms-appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGI(fmt, args...) ALOGI( "sms-appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#define ADR_LOGD(fmt, args...) ALOGD( "sms-appdriver - %s(): " fmt "\n", __FUNCTION__, ##args)
#else
#define ADR_LOGE(fmt, args...) 
#define ADR_LOGW(fmt, args...)
#define ADR_LOGI(fmt, args...)
#define ADR_LOGD(fmt, args...)
#endif

#define ADR_MAX_NUM_OF_HANDLES			(SMS_MAX_SERVICE_HANDLE+1)
#define ADR_TH_TASK_NAME_LEN 			(14)
#define SMS_ADR_LITE_MAX_DEVICE_NAME_LEN 	(256)

typedef void (*PDATA_CALLBACK)(UINT32 handle_num, void *Buffer, unsigned long Size);

void SmsHwPnpCallback(char* DeviceName, BOOL fArrival);
void SmsHwRxCallback(UINT32 handle_num, void *Buffer, unsigned long Size);
static SMSHOSTLIB_ERR_CODES_E ADR_OpenHandle(UINT32 handle_num);
static SMSHOSTLIB_ERR_CODES_E ADR_CloseHandle(UINT32 handle_num);
static SMSHOSTLIB_ERR_CODES_E ADR_DeviceTerminate();
static SMSHOSTLIB_ERR_CODES_E ADR_SetDeviceMode(int fd, SMSHOSTLIB_DEVICE_MODES_E DeviceMode);

///////////////////////////////////////////////////////////////////////////////

typedef struct _handle_data
{
	unsigned int handle_num;		//! ctrl/data handle number
	int fd; 				//! device file descriptor
	int stopped;				//! task stop flag
	PDATA_CALLBACK callback;		//! RX path callback
	unsigned char *common_buffer;		//! memory mapping for Linux core driver common buffer
	int common_buffer_size;			//! Linux core driver common buffer size
} handle_data_t;

typedef struct _adr_data 
{
	int IsInit;
	SmsLiteAdr_pfnFuncCb pfnCtrlCallback;			//! registered ctrl callback
	SmsLiteAdr_pfnFuncCb pfnDataCallback;			//! registered data callback
} adr_data_t;

typedef struct _device_data
{
	int IsFound;
	char DeviceName[SMS_ADR_LITE_MAX_DEVICE_NAME_LEN];	

	SMSHOSTLIB_DEVICE_MODES_E DeviceMode;			//! device operation mode
	handle_data_t AdrHandles[ADR_MAX_NUM_OF_HANDLES];	//! ctrl/data handles array
	struct pollfd poll_fd[ADR_MAX_NUM_OF_HANDLES];		//! file descriptor polling sturct 
	OSW_TaskId pCtrlTask;			                //! ctrl push thread				
	OSW_TaskId pDataTask;			                //! data push thread 
	struct smschar_get_fw_filename_ioctl_t get_fn_ioctl;	//! params structure for SMSCHAR_GET_FW_FILE_NAME
	struct smschar_send_fw_file_ioctl_t send_fw_ioctl;	//! params structure for SMSCHAR_SEND_FW_FILE

	int NumOfHandles;
	int thread_done;                                 //! thread done indication
	
	
} device_data_t;

adr_data_t g_adrData = {0};
device_data_t g_devData = {0};

///////////////////////////////////////////////////////////////////////////////

/*! a background task listening for ctrl message, reading it 
 *  from the device using ioctl, then calls a registered application ctrl
 *  callback.
 *  \param[in]	arg: handle (pointer to handle_data_t)
 *  \return		ignored
 */
static UINT32 CtrlPushThread(void* param)
{
	struct smschar_buffer_t buffer;
	handle_data_t *hd = &g_devData.AdrHandles[0];
 
	while ( !g_devData.AdrHandles[0].stopped )
	{

		if (0 > ioctl(hd->fd, SMSCHAR_WAIT_GET_BUFFER, &buffer))
		{
			//error condition
			ADR_LOGE("SMSCHAR_WAIT_GET_BUFFER error %d\n", errno);
			break;
		}

		if (buffer.size > 0)
		{
		       ADR_LOGD("callback size=%d-->",buffer.size);
			hd->callback(hd->handle_num, &hd->common_buffer[buffer.offset], buffer.size);
			ADR_LOGD("callback ok <--");
		}
		else
		{
                      ADR_LOGE("buffer.size:%d <=0",buffer.size);
		}
	}
	
	return 0;
}


/*! a background task listening for data availability, polling and reading it 
 *  from the device using ioctl, then calls a registered application data
 *  callback.
 *  \param[in]	arg: handle (pointer to handle_data_t)
 *  \return		ignored
 */
static UINT32 DataPushThread(void* param)
{
	struct smschar_buffer_t buffer;
	handle_data_t *hd;
	int i;

	//this means that thread is alive
   	g_devData.thread_done = FALSE;
    
	//AdrHandles[1] is the first one to be stopped during terminate
	while ( !g_devData.AdrHandles[1].stopped )
	{
		int count = 0;

		//wait with an infinite timeout until there is pending data 
		//in the char device
		if (poll(&g_devData.poll_fd[1], g_devData.NumOfHandles-1, -1) < 0)
		{
			ADR_LOGE("poll error\n");
			break;
		}

		//read until there is no data pending
		//in any case, the code in this critical section must NEVER block		
		for (i=1; i < g_devData.NumOfHandles; i++)
		{
		    if ((g_devData.poll_fd[i].revents & POLLIN) != 0)
		    {
		        count++;
			hd = &g_devData.AdrHandles[i];
    		    do {
    		    	if (0 > ioctl(hd->fd, SMSCHAR_WAIT_GET_BUFFER, &buffer))
    			    {
    				    //error condition
    				    ADR_LOGE("SMSCHAR_WAIT_GET_BUFFER error %d\n", errno);
    				    break;
         			}
     
        			if (buffer.size > 0)
        			{
                                   ADR_LOGD("callback size=%d-->",buffer.size);
        				hd->callback(hd->handle_num, &hd->common_buffer[buffer.offset], buffer.size);
					ADR_LOGD("callback ok <--");
				}
				else
				{
		                      ADR_LOGE("buffer.size:%d <=0",buffer.size);
				}
        			//poll without blocking for pending data 
        			//in the char device driver
        			if (poll(&g_devData.poll_fd[i], 1, 0) < 0)
        			{
        				//error condition
        				ADR_LOGE("poll error\n");
        				break;
        			}	
        		} while (g_devData.poll_fd[i].revents & POLLIN);
		    }
            
		}

		//sanity check
		if (!count)
		{
		    ADR_LOGE("error - poll didn't recieve POLLIN event\n");
		    break; 
		}

	}
	g_devData.thread_done = TRUE;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
SMSHOSTLIB_ERR_CODES_E SmsLiteAdrInit(SMSHOSTLIB_DEVICE_MODES_E DeviceMode,                    
				SmsLiteAdr_pfnFuncCb pfnControlCb, 
				SmsLiteAdr_pfnFuncCb pfnDataCb )
{
	int i;
       ADR_LOGD("Enter");
	if (g_adrData.IsInit) 
	{
		return 	SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED; 
	}
	g_adrData.pfnCtrlCallback = pfnControlCb;
	g_adrData.pfnDataCallback = pfnDataCb;

	memset(&g_devData, 0, sizeof(device_data_t));
	g_devData.DeviceMode = DeviceMode;
	for ( i = 0 ; i < ADR_MAX_NUM_OF_HANDLES; i++ )
	{
		g_devData.AdrHandles[i].fd = 0;
	}
	
	//call PNP callback - arrival                                             
	SmsHwPnpCallback("SmsDevice", TRUE);
	if (!g_devData.IsFound)
	{
		return SMSHOSTLIB_ERR_DEVICE_DOES_NOT_EXIST;
	}

	g_adrData.IsInit = TRUE;
       ADR_LOGD("OK");
	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E SmsLiteAdrTerminate( void )
{
       ADR_LOGD("Enter");
	if (!g_adrData.IsInit) 
	{
	       ADR_LOGW("Warning:Driver is not init already");
		return SMSHOSTLIB_ERR_OK;
	}

	//call PNP callback - disconnect
	SmsHwPnpCallback("SmsDevice", FALSE);

	g_adrData.IsInit = FALSE;
       ADR_LOGD("OK");
	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

/*! inform the char device driver to set the device operation mode
 *  if needed, send the firmware buffer to the friver before setting device mode
 *  \fd[in]	open device file descriptor (ctrl)
 *  \DeviceMode[in]	required operation mode
 */
SMSHOSTLIB_ERR_CODES_E ADR_SetDeviceMode(int fd, SMSHOSTLIB_DEVICE_MODES_E DeviceMode)
{
	int i, curMode;
	FILE *ffw;
	
	g_devData.get_fn_ioctl.mode = DeviceMode;
#ifndef  _hfilefw
	if (0 > ioctl(fd, SMSCHAR_GET_FW_FILE_NAME, &g_devData.get_fn_ioctl))
	{
		close(fd);
		ADR_LOGE("SMSCHAR_GET_FW_FILE_NAME failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	if (strlen(g_devData.get_fn_ioctl.filename) > 0)
	{
		// kernel not supprt request_firmware,
		// send the file to driver for download */
		ADR_LOGE("Read and send %s to driver\n", g_devData.get_fn_ioctl.filename);
		ffw = fopen(g_devData.get_fn_ioctl.filename, "rb" );
		if (!ffw)
		{
			close(fd);
			ADR_LOGE("failed to read fw file %s\n", g_devData.get_fn_ioctl.filename);
			return SMSHOSTLIB_ERR_COMM_ERR;
		}

		// get file size
		fseek(ffw,0,SEEK_END);
		g_devData.send_fw_ioctl.fw_size = ftell(ffw);
		fseek(ffw,0,SEEK_SET);

		// alloc fw_buf
		g_devData.send_fw_ioctl.fw_buf = malloc(g_devData.send_fw_ioctl.fw_size);
		if (!g_devData.send_fw_ioctl.fw_buf)
		{
			fclose(ffw);
			close(fd);
			ADR_LOGE("failed to alloc fw buf\n");
			return SMSHOSTLIB_ERR_COMM_ERR;
		}

		// read file to fw_buf
		fread(g_devData.send_fw_ioctl.fw_buf,1,g_devData.send_fw_ioctl.fw_size,ffw);
		fclose(ffw);

		if (0 > ioctl(fd, SMSCHAR_SEND_FW_FILE, &g_devData.send_fw_ioctl))
		{
			free(g_devData.send_fw_ioctl.fw_buf);
			close(fd);
			ADR_LOGE("SMSCHAR_SEND_FW_FILE failed %d\n", errno);
			return SMSHOSTLIB_ERR_COMM_ERR;
		}
		free(g_devData.send_fw_ioctl.fw_buf);
	}
#else
	 // set buffer
	 ADR_LOGE("download fw using h file\n");
	 g_devData.send_fw_ioctl.fw_size = FIRMWARE_BUF_DATA_SIZE;
	 // alloc fw_buf
	g_devData.send_fw_ioctl.fw_buf = (char*)malloc(g_devData.send_fw_ioctl.fw_size);
	memcpy(g_devData.send_fw_ioctl.fw_buf ,rawData,FIRMWARE_BUF_DATA_SIZE);
	//g_devData.send_fw_ioctl.fw_buf = (char*)rawData;
	 
	if (0 > ioctl(fd, SMSCHAR_SEND_FW_FILE, &g_devData.send_fw_ioctl))
	{
		ADR_LOGE("SMSCHAR_SEND_FW_FILE failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
	free(g_devData.send_fw_ioctl.fw_buf);
#endif            //_hfilefw
	if (0 > ioctl(fd, SMSCHAR_SET_DEVICE_MODE, (int) DeviceMode))
	{
		close(fd);
		ADR_LOGE("SMSCHAR_SET_DEVICE_MODE failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	close(fd);

	for (i = 0; i < 17; i ++, sleep(1))
	{
		fd = open( "/dev/mdtvctrl", O_RDWR );
		if (fd < 0)
			continue;

		if (0 > ioctl(fd, SMSCHAR_GET_DEVICE_MODE, &curMode))
		{
			close(fd);
			continue;
		}

		close(fd);

		if ((int) DeviceMode == curMode) {
			g_devData.DeviceMode = DeviceMode;
			return SMSHOSTLIB_ERR_OK;
		}
	}

	ADR_LOGE("SMSCHAR_GET_DEVICE_MODE failed after setting curMode %d\n", errno);
	return SMSHOSTLIB_ERR_COMM_ERR;
}

///////////////////////////////////////////////////////////////////////////////
//TODO - no real indication for device init (no protection against multiple init)
SMSHOSTLIB_ERR_CODES_E ADR_DeviceInit(SMSHOSTLIB_DEVICE_MODES_E DeviceMode)
{
	int i, curMode, fd;
	SMSHOSTLIB_ERR_CODES_E Ret;
	g_devData.NumOfHandles = ADR_MAX_NUM_OF_HANDLES;

	if (g_devData.IsFound)
	{
		// Already have a device - ignore
		return SMSHOSTLIB_ERR_OK;
	}

	//set decvice mode
	fd = open( "/dev/mdtvctrl", O_RDWR );
	if (fd < 0)
	{
		ADR_LOGE("failed openning device file for control.\n");
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	ADR_LOGE("SMSCHAR_DEVICE_POWER_ON\r\n");
	if (0 > ioctl(fd, SMSCHAR_DEVICE_POWER_ON))
	{
		close(fd);
		ADR_LOGE("SMSCHAR_DEVICE_POWER_ON failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
;
	if (0 > ioctl(fd, SMSCHAR_GET_DEVICE_MODE, &curMode))
	{
		close(fd);
		ADR_LOGE("SMSCHAR_GET_DEVICE_MODE failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	if ((int) DeviceMode != curMode)
	{
		ADR_LOGE("Changing device mode from %d to %d\n", curMode, DeviceMode);
		Ret = ADR_SetDeviceMode(fd, DeviceMode);
		if (Ret != SMSHOSTLIB_ERR_OK)
		{
			close(fd);
			return Ret;
		}
	}
	g_devData.DeviceMode = DeviceMode;
	close(fd);

	switch( DeviceMode )                                       
	{
	case SMSHOSTLIB_DEVMD_DVBT:
	case SMSHOSTLIB_DEVMD_DVBT_BDA:
	case SMSHOSTLIB_DEVMD_ISDBT:
	case SMSHOSTLIB_DEVMD_ISDBT_BDA:
		g_devData.NumOfHandles = 2;
		break;
	default:
		break;
	}

	//open all handles
	for ( i = 0 ; i < g_devData.NumOfHandles ; i++ )
	{
		Ret = ADR_OpenHandle( i );
		if ( Ret != SMSHOSTLIB_ERR_OK )
		{
			while( i > 0 )
			{
				ADR_CloseHandle(i);
				i--;
			}
			return Ret;
		}
	}

   	//create threads for ctrl/data handles
	char taskname[20];

	snprintf(taskname,
		ADR_TH_TASK_NAME_LEN,
		"SmsRxCtrlThread");

	g_devData.pCtrlTask = OSW_TaskCreate(taskname,
				OSW_SMSHOSTLIBTASK_PRI,
				OSW_SMSHOSTLIBTASK_STACK,
				(TaskFunc)CtrlPushThread,
				(void*)taskname );

	if (g_devData.pCtrlTask == NULL)
	{
		ADR_LOGE("SmsRxCtrlThread creation failed\n");
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	snprintf(taskname,
		ADR_TH_TASK_NAME_LEN,
		"SmsRxDataThread");

	g_devData.pDataTask = OSW_TaskCreate(taskname,
				OSW_SMSHOSTLIBTASK_PRI,
				OSW_SMSHOSTLIBTASK_STACK,
				(TaskFunc)DataPushThread,
				(void*)taskname );

	if (g_devData.pDataTask == NULL)
	{
		ADR_LOGE("SmsRxDataThread creation failed\n");
		return SMSHOSTLIB_ERR_COMM_ERR;
	}



	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E ADR_DeviceTerminate()
{
	SMSHOSTLIB_ERR_CODES_E Ret;
	SMSHOSTLIB_ERR_CODES_E FinalRet = SMSHOSTLIB_ERR_OK;
	int i;
	OSW_TaskId TaskIdCopy;

	if (!g_devData.IsFound)
	{
		return FinalRet;
	}
    
	//close all handles
	for ( i = 0 ; i < g_devData.NumOfHandles ; i++ )
	{
		Ret = ADR_CloseHandle( i );
		if ( Ret != SMSHOSTLIB_ERR_OK )
		{
			FinalRet = SMSHOSTLIB_ERR_UNDEFINED_ERR;
		}
	}

	//double check - close data push thread, wait till thread is done
	while(g_devData.thread_done != TRUE){OSW_TaskSleep(10);};                                 
       TaskIdCopy = g_devData.pDataTask;
	OSW_TaskCleanup(TaskIdCopy);

	//close ctrl push thread
   	TaskIdCopy = g_devData.pCtrlTask;
	OSW_TaskCleanup(TaskIdCopy);

       //power off
	int fd;
	fd = open( "/dev/mdtvctrl", O_RDWR );
	if(fd < 0)
	{
		ADR_LOGE("failed openning device file for control.\n");
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	ADR_LOGE("SMSCHAR_DEVICE_POWER_OFF\n");

	if (0 > ioctl(fd, SMSCHAR_DEVICE_POWER_OFF))
	{
		close(fd);
		ADR_LOGE("SMSCHAR_DEVICE_POWER_OFF failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
	close(fd);
	return FinalRet;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E ADR_OpenHandle( UINT32 handle_num )
{
	handle_data_t *hd;
	char file_name[32];

	if (handle_num >= ADR_MAX_NUM_OF_HANDLES)
	{
		ADR_LOGE("invalid handle_num %d\n", (int)handle_num);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
	
	hd = &g_devData.AdrHandles[handle_num];
	if (hd->fd != 0)
	{
		ADR_LOGE("handle is already open");
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	memset(hd, 0, sizeof(handle_data_t));

	if (handle_num)                                                                              
		sprintf(file_name, "/dev/mdtv%d", (int) handle_num);
	else
		strcpy(file_name, "/dev/mdtvctrl");

	ADR_LOGE("opening device handle %s.\n",file_name);
	hd->fd = open(file_name, O_RDWR );
	if(hd->fd < 0)
	{
		ADR_LOGE("failed openning device file %s\n", file_name);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
	
	if (0 > ioctl(hd->fd, SMSCHAR_GET_BUFFER_SIZE, &hd->common_buffer_size))
	{
		close(hd->fd);
		hd->fd = 0;
		ADR_LOGE("SMSCHAR_GET_COMMON_BUFFER_SIZE failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
	
	hd->common_buffer = (unsigned char *) mmap (NULL, hd->common_buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, hd->fd, 0);
	if (hd->common_buffer == MAP_FAILED)
	{
		close(hd->fd);
		ADR_LOGE("mmap failed %d\n", errno);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	hd->stopped = 0;
	hd->callback = SmsHwRxCallback;
	hd->handle_num = handle_num;
	g_devData.poll_fd[handle_num].fd = hd->fd;
	g_devData.poll_fd[handle_num].events = POLLIN | POLLNVAL;

	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E ADR_CloseHandle(	UINT32 handle_num )
{
	handle_data_t *hd;
	int status;

	if (handle_num >= ADR_MAX_NUM_OF_HANDLES)
	{
		ADR_LOGE("invalid handle_num %d\n", (int)handle_num);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}

	hd = &g_devData.AdrHandles[handle_num];	
	if (hd->fd == 0)
	{
		return SMSHOSTLIB_ERR_OK;
	}

	hd->stopped = 1;
	ioctl(hd->fd, SMSCHAR_CANCEL_WAIT_BUFFER, NULL);
	ioctl(hd->fd, SMSCHAR_CANCEL_POLL, NULL);

	status = munmap(hd->common_buffer, hd->common_buffer_size);
	if (status != 0)
		ADR_LOGE("munmap failed %d\n", errno);

	close(hd->fd);
	hd->fd = 0;
	ADR_LOGE("device handle num %d is closed.\n", (int)handle_num );
	return SMSHOSTLIB_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

SMSHOSTLIB_ERR_CODES_E SmsLiteAdrWriteMsg( SmsMsgData_ST*	p_msg )
{
	handle_data_t *hd = &g_devData.AdrHandles[0];
	int ret;
	int len;

	if (!g_devData.IsFound)
	{
		return SMSHOSTLIB_ERR_DEVICE_DOES_NOT_EXIST;
	}

	if (hd->fd == 0)
	{
		ADR_LOGE("control handle is not open\n");
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
	// get message len
	len = p_msg->xMsgHeader.msgLength;
	ADR_LOGD("writing msg %d len %d\n",p_msg->xMsgHeader.msgType,p_msg->xMsgHeader.msgLength);

	// write message to device file descriptor
	ret = write(hd->fd, (char*)p_msg, len);
	if(ret == len)
		return SMSHOSTLIB_ERR_OK;
	else
	{
		ADR_LOGE("Error. %d bytes written instead of %d\n", ret, len);
		return SMSHOSTLIB_ERR_COMM_ERR;
	}
}

///////////////////////////////////////////////////////////////////////////////

void SmsHwPnpCallback(char* DeviceName, BOOL fArrival)
{
	if (fArrival)
	{	
		if (g_devData.IsFound)
		{
			// Already have a device - ignore
			return;
		}

		SMS_ASSERT((strlen(DeviceName) + 1) < SMS_ADR_LITE_MAX_DEVICE_NAME_LEN );
		strncpy(g_devData.DeviceName, DeviceName, SMS_ADR_LITE_MAX_DEVICE_NAME_LEN);

		if (ADR_DeviceInit(g_devData.DeviceMode) != SMSHOSTLIB_ERR_OK)
		{
			g_devData.IsFound = FALSE;
			return;
		}
		
		g_devData.IsFound = TRUE;
	}
	else
	{
		BOOL IsSameName = (strncmp( g_devData.DeviceName, DeviceName, SMS_ADR_LITE_MAX_DEVICE_NAME_LEN ) == 0 );
	
		if (!g_devData.IsFound)
		{
			return;
		}

		if (!IsSameName)
		{
			return;
		}

		ADR_DeviceTerminate();
		g_devData.IsFound = FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////////

void SmsHwRxCallback(UINT32 handle_num, void *Buffer, unsigned long Size)
{
	if (handle_num >=  ADR_MAX_NUM_OF_HANDLES)
	{
		return;
	}
	
	if ( handle_num == 0 )
	{
		if ( g_adrData.pfnCtrlCallback != NULL )
		{
			g_adrData.pfnCtrlCallback(handle_num, (UINT8*)Buffer, Size);
		}
	}
	else
	{
		if ( g_adrData.pfnDataCallback != NULL )
		{
			g_adrData.pfnDataCallback(handle_num, (UINT8*)Buffer, Size);
		}
	}
}
