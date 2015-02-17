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
/*      smsioctl.h                                                       */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      Linux char device ioctl commnads and definition			 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*************************************************************************/

#ifndef __SMS_IOCTL_H__
#define __SMS_IOCTL_H__

#include <linux/ioctl.h>

struct smschar_buffer_t {
	unsigned long offset;	/* offset in common buffer (mapped to user) */
	int size;
};

struct smschar_get_fw_filename_ioctl_t {
	int mode;
	char filename[200];
};

struct smschar_send_fw_file_ioctl_t {
	char *fw_buf;
	int fw_size;
};

#define SMSCHAR_SET_DEVICE_MODE		_IOW('K', 0, int)
#define SMSCHAR_GET_DEVICE_MODE		_IOR('K', 1, int)
#define SMSCHAR_GET_BUFFER_SIZE		_IOR('K', 2, int)
#define SMSCHAR_WAIT_GET_BUFFER		_IOR('K', 3, struct smschar_buffer_t)
#define SMSCHAR_IS_DEVICE_PNP_EVENT 	_IOR('K', 4, int)
#define SMSCHAR_GET_FW_FILE_NAME	\
	_IOWR('K', 5, struct smschar_get_fw_filename_ioctl_t)
#define SMSCHAR_SEND_FW_FILE		\
	_IOW('K', 6, struct smschar_send_fw_file_ioctl_t)
#define SMSCHAR_CANCEL_WAIT_BUFFER	_IO('K', 7)
#define SMSCHAR_CANCEL_POLL		_IO('K', 8)

#define SMSCHAR_DEVICE_POWER_ON	   _IO('K', 9)                //1215

#define SMSCHAR_DEVICE_POWER_OFF   _IO('K', 10)

#endif /* __SMS_IOCTL_H__ */
