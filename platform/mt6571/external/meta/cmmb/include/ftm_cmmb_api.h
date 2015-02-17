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

#ifndef FTM_CMMB_API_H
#define FTM_CMMB_API_H
/* 
 * ftm_cmmb_api.h - Factory Test Module of CMMB, interface
 */
#include "cmmb_errcode.h"
#include "meta_cmmb_para.h"
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return value:
 *	CMMB_S_OK: success, others: fail
 */
CmmbResult CmmbFtInit();

/*
 * Return value:
 *	CMMB_S_OK: success, others: fail
 */
CmmbResult CmmbFtTerm();

/* 
 * Parameters:
 *	int channel	- channel number, from 13 to 48
 *	int freq	- frequency, from 474000000 to 794000000
 * 
 * Return value:
 *	0: break scan, others: continue
 */
typedef int (*CMMB_AUTOSCAN_CALLBACK)(int channel, int freq);

CmmbResult CmmbFtAutoScan(CMMB_AUTOSCAN_CALLBACK scanProc);

/* 
 * Description:
 *	Test if the signal in this channel is available
 *
 * Parameters:
 *	[in] int channel	- channel number, from 13 to 48
 *
 * Return value:
 *	CMMB_S_OK: the channel is valid 
 *	Others: the channel is invalid
 */
CmmbResult CmmbFtChannelTest(int channel);

/* 
 * Description:
 *	Begin receiving the signal from this channel
 *
 * Parameters:
 *	[in] int channel	- channel number, from 13 to 48
 *
 * Return value:
 *	CMMB_S_OK: success, others: fail
 */
CmmbResult CmmbFtSetChannel(int channel);

/* 
 * Parameters:
 *	int serviceId	- service ID
 * 
 * Return value:
 *	0: break callback, others: continue
 */
typedef int (*CMMB_SERVICE_CALLBACK)(int serviceId);

CmmbResult CmmbFtListServices(CMMB_SERVICE_CALLBACK srvProc);

/* 
 * Parameters:
 *	const char* szMfsFile - full path of MFS file
 *
 * Return value:
 *	CMMB_S_OK: success, others: fail
 */
CmmbResult CmmbFtStartService(int serviceId, const char* szMfsFile);

/*
 * Return value:
 *	CMMB_S_OK: success, others: fail
 */
CmmbResult CmmbFtStopService();

#define CMMB_PROP_SNR				1
#define CMMB_PROP_PRE_BER		2
#define CMMB_PROP_RSSI				3

/* 
 * Parameters:
 *	[in] int propId		- property ID
 *	[out]int* propVal	- property value
 *
 * Return value:
 *	CMMB_S_OK: success, others: fail
 */

CmmbResult CmmbFtGetProp(CmmbProps* props);


#ifdef __cplusplus
}
#endif

#endif // FTM_CMMB_API_H
