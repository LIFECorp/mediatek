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

#ifndef META_CMMB_PARA_H
#define META_CMMB_PARA_H

// meta_cmmb_para.h
//
#include "cmmb_errcode.h"

#include "FT_Public.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	META_CMMB_CMD_INIT = 0,
	META_CMMB_CMD_TERM,
	META_CMMB_CMD_AUTO_SCAN,
	META_CMMB_CMD_CANCEL_SCAN,
	META_CMMB_CMD_SET_CHANNEL,
	META_CMMB_CMD_LIST_SRVS,
	META_CMMB_CMD_START_SRV,
	META_CMMB_CMD_STOP_SRV,
	META_CMMB_CMD_GET_PROPS,
	META_CMMB_CMD_END
} MetaCmmbCmd;

typedef enum
{
	META_CMMB_PROP_SNR,		
	META_CMMB_PROP_PRE_BER,	
	META_CMMB_PROP_RSSI		
} MetaCmmbProp;

typedef struct
{
	union {
		int channel;		// for META_CMMB_CMD_SET_CHANNEL
		int serviceId;		// for META_CMMB_CMD_START_SRV
	} data;

	char szMfsFile[260];	// for META_CMMB_CMD_START_SRV
} CmmbReqParam;

typedef struct
{
    FT_H                header;
    MetaCmmbCmd     	cmd;
    CmmbReqParam	param;
} MetaCmmbReq;

typedef struct
{
	int channel;
	int freq;
} CmmbChannelInfo;

typedef struct
{
	int SNR;
	int PRE_BER;
	int RSSI;
} CmmbProps;

typedef struct
{
        int flag;                      // 0: continue ,1: finished
        CmmbChannelInfo chInfo;
}CmmbScanChannel;

typedef struct
{
        int flag;                      // 0: continue ,1: finished
        int serviceId;
}CmmbListService;

typedef struct
{
	CmmbResult	errCode;	// command detal result

	union {
//		CmmbChannelInfo chInfo;	// for META_CMMB_CMD_AUTO_SCAN, maybe multi-confirm for this request
//		int serviceId;		// for META_CMMB_CMD_LIST_SRVS, maybe multi-confirm for this request
                CmmbScanChannel scanChn;
                CmmbListService listSrv;
		CmmbProps props;	// for META_CMMB_CMD_GET_PROPS
	} data;
} CmmbCnfParam;

typedef enum
{
	CMMB_CNF_DONE = 0,		// succeeded
	CMMB_CNF_FAIL,			// failed
//	CMMB_CNF_CONTINUE,		// command not finished, maybe more confirm message will arrival
//	CMMB_CNF_FINISHED		// command finished
} CmmbCnfStatus;

typedef struct
{
    FT_H		header;
    MetaCmmbCmd          cmd;
    CmmbCnfStatus	status;		
    CmmbCnfParam	result;
} MetaCmmbCnf;

void META_CMMB_OP(MetaCmmbReq* req);

#ifdef __cplusplus
}
#endif

#endif // META_CMMB_PARA_H
