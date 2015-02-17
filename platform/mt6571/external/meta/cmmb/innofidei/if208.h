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

#ifndef _IF101_H_
//if208.h
#define _IF101_H_
//#include "stdio.h"

#include <linux/types.h>
//=======IOCTL COMMANDS==============

#define INNO_IOC_MAGIC		'i'
#define INNO_GET_INTR_TYPE			_IOR(INNO_IOC_MAGIC, 1, int)
#define INNO_MMIS_READ				_IOWR(INNO_IOC_MAGIC, 2, int)
#define INNO_MMIS_WRITE				_IOWR(INNO_IOC_MAGIC, 3, int)
#define INNO_MMIS_CMD				_IOWR(INNO_IOC_MAGIC, 4, int)
#define INNO_READ_REG				_IOWR(INNO_IOC_MAGIC, 5, int)
#define INNO_WRITE_REG				_IOWR(INNO_IOC_MAGIC, 6, int)
#define INNO_SCAN_FREQUENCY			_IOWR(INNO_IOC_MAGIC, 7, int)
#define INNO_SCAN_FREQUENCY_DOT		_IOWR(INNO_IOC_MAGIC, 8, int)
#define INNO_SET_FREQUENCY			_IOWR(INNO_IOC_MAGIC, 9, int)
#define INNO_SET_FREQUENCY_DOT		_IOWR(INNO_IOC_MAGIC, 10, int)
#define INNO_SET_CHANNEL_CONFIG		_IOWR(INNO_IOC_MAGIC, 11, int)
#define INNO_GET_CHANNEL_CONFIG		_IOWR(INNO_IOC_MAGIC, 12, int)
#define INNO_GET_SYS_STATE			_IOWR(INNO_IOC_MAGIC, 13, int)
#define INNO_SET_PM					_IOWR(INNO_IOC_MAGIC, 14, int)
#define INNO_GET_FW_VER				_IOWR(INNO_IOC_MAGIC, 15, int)
#define INNO_FW_DOWNLOAD			_IOWR(INNO_IOC_MAGIC, 16, int)
#define INNO_SET_CP_TYPE			_IOWR(INNO_IOC_MAGIC, 17, int)
#define INNO_UPDATE_FW				_IOWR(INNO_IOC_MAGIC, 22, int)
#define INNO_GET_BUFFERLEN			_IOR(INNO_IOC_MAGIC, 23, int)
#define INNO_ENABLE_IRQ				_IOWR(INNO_IOC_MAGIC, 24, int)
#define INNO_DISABLE_IRQ			_IOWR(INNO_IOC_MAGIC, 25, int)
#define INNO_SHUTDOWN_IRQ			_IOWR(INNO_IOC_MAGIC, 26, int)

#define INNO_UAM_SET_CMD			_IOW(INNO_IOC_MAGIC, 31, int)
#define INNO_UAM_SET_CMDL			_IOW(INNO_IOC_MAGIC, 32, int)
#define INNO_UAM_READ_RES			_IOR(INNO_IOC_MAGIC, 33, int)
#define INNO_UAM_READ_STATUS			_IOR(INNO_IOC_MAGIC, 34, int)
#define INNO_UAM_READ_STATUS_LEN                _IOR(INNO_IOC_MAGIC, 35, int)
#define INNO_UAM_TRANSFER                 _IOR(INNO_IOC_MAGIC, 36, int)                       //xingyu add
#define INNO_UAM_INIT                          _IOR(INNO_IOC_MAGIC, 37, int)                       //xingyu add
#define INNO_SEND_UAM_CMD                _IOR(INNO_IOC_MAGIC, 38, int)                       //xingyu add
#define INNO_SEND_CMD                         _IOR(INNO_IOC_MAGIC, 39, int)                       //xingyu add
#define INNO_GET_FW_BYTES                _IOR(INNO_IOC_MAGIC, 40, int)                       //xingyu add
#define INNO_STOP_POLL                        _IOR(INNO_IOC_MAGIC, 41, int)                       //xingyu add
#define INNO_MEMSET_MFS                    _IOR(INNO_IOC_MAGIC, 42, int)                       //xingyu add

struct INNOCHAR_FW_DATA {                      //xingyu add
	char *fw_buf;
	int fw_size;
};
struct uam_cmd_par{
	unsigned char* cmd;
	int len;
};
typedef struct _Rx_Data
{
	int rx_len;		      //mfs file length   about 50k
	unsigned char channel_id; 				// the data come from channel
	unsigned char * rx_buf;             // data buffer
} Rx_Data_t;

struct inno_channel_config{
	BYTE	ch_id;		//0--logic channel 0, 1--logic channel 1
	BYTE	ch_close:1;		//1--close this logic channel
	BYTE	ts_start:6;
	BYTE	ts_count;	
	BYTE	ldpc:2;
	BYTE 	itlv:2;
	BYTE	rs:2;
	BYTE	modulate:2;
	BYTE	subframe_ID;
};

struct inno_uam_parameter {
	unsigned char pBufIn[260];
	unsigned int bufInLen;
	unsigned char pBufOut[256];
	unsigned int pBufOutLen;
	unsigned short sw;
};

#endif
