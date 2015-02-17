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

// meta_cmmb.c
//
#include "meta_common.h"
#include "meta_cmmb_para.h"
#include "ftm_cmmb_api.h"
#include "WM2Linux.h"

#define null 0

unsigned short scan_token;                              // use for scan thread token parameter pass
unsigned short serv_token;                              // use for serv thread token parameter pass
int MetaServiceProc(int serviceId)
{
	META_LOG("MetaServiceProc(),  1\n");
	MetaCmmbCnf cnf;

	cnf.header.id = FT_CMMB_CNF_ID;
        cnf.header.token = serv_token;
	cnf.cmd = META_CMMB_CMD_LIST_SRVS;
	cnf.status = CMMB_CNF_DONE;
	cnf.result.errCode = CMMB_S_OK;
	cnf.result.data.listSrv.serviceId = serviceId;
	cnf.result.data.listSrv.flag= 0;

	WriteDataToPC(&cnf, sizeof(cnf), null, 0);
	META_LOG("MetaServiceProc(),  2\n");
	return 1; // continue
}

#define C_INVALID_TID  (-1)   /*invalid thread id*/

pthread_t cmmb_meta_th = C_INVALID_TID;

bool scan_begin = false;
bool scan_cancel = false;
bool scan_end = true;
bool thread_break = false;

int MetaAutoScanProc(int channel, int freq)
{
	META_LOG("MetaAutoScanProc(),  1\n");
	MetaCmmbCnf cnf;
	cnf.header.id = FT_CMMB_CNF_ID;
        cnf.header.token = scan_token;

	cnf.cmd = META_CMMB_CMD_AUTO_SCAN;
	cnf.status = CMMB_CNF_DONE;
	cnf.result.errCode = CMMB_S_OK;
	cnf.result.data.scanChn.chInfo.channel = channel;
	cnf.result.data.scanChn.chInfo.freq = freq;
	cnf.result.data.scanChn.flag =0;

	WriteDataToPC(&cnf, sizeof(cnf), null, 0);
	META_LOG("MetaAutoScanProc(),  2\n");

	if (scan_cancel || thread_break)
		return 0;
	else
		return 1; // continue
}

void *CmmbMetaThread(void *arg)
{
	CmmbResult errCode;
	MetaCmmbCnf cnf;
	cnf.header.id = FT_CMMB_CNF_ID;

	for (;;) {
		if (thread_break)
			break;

		// delay 100ms
		usleep(100000);

		if (thread_break)
			break;

		if (scan_begin)
		{
			META_LOG("META_CMMB_OP(), auto-scan begin\n");
			scan_end = false;
			cnf.cmd = META_CMMB_CMD_AUTO_SCAN;
			errCode = CmmbFtAutoScan(MetaAutoScanProc);

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			cnf.result.errCode = errCode;
                        cnf.header.token = scan_token;
                        cnf.result.data.scanChn.flag = 1;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			scan_begin = false;
			scan_end = true;
			META_LOG("META_CMMB_OP(), auto-scan end\n");
		}
	}

	return null;
}
void META_CMMB_OP(MetaCmmbReq* req)
{
	MetaCmmbCnf cnf;
	CmmbResult errCode;

	cnf.header.id = FT_CMMB_CNF_ID;
	cnf.header.token = req->header.token;
	cnf.cmd = req->cmd;

	int channel;
	int serviceId;
	int propId;
	int propVal;
	const char* szMfsFile;

	META_LOG("META_CMMB_OP() 1, (%d)\n", req->cmd);

	switch(req->cmd)
	{
		case META_CMMB_CMD_INIT:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_INIT 1\n");
			errCode = CmmbFtInit();

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			thread_break = false;

			if (pthread_create(&cmmb_meta_th, null, CmmbMetaThread, null))
			{
				META_LOG("META_CMMB_OP(), create CmmbMetaThread failed!\n");
				cnf.status = CMMB_CNF_FAIL;
			}

			cnf.result.errCode = errCode;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_INIT 2\n");
			break;

		case META_CMMB_CMD_TERM:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_TERM 1\n");
			errCode = CmmbFtTerm();

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			thread_break = true;
			pthread_join(cmmb_meta_th, null);

			cnf.result.errCode = errCode;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_TERM 2\n");
			break;

		case META_CMMB_CMD_AUTO_SCAN:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_AUTO_SCAN 1\n");
			if (scan_end)
			{
                                scan_token = req->header.token;
				scan_cancel = false;
				scan_begin = true;
			}
			else
			{
				META_LOG("META_CMMB_OP(), auto-scan not finished, wrong request!\n");
				cnf.status = CMMB_CNF_FAIL;
				cnf.result.errCode = CMMB_E_WRONGSTATE;
				WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			}
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_AUTO_SCAN 2\n");
			break;

		case META_CMMB_CMD_CANCEL_SCAN:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_CANCEL_SCAN\n");
			scan_cancel = true;
			break;

		case META_CMMB_CMD_SET_CHANNEL:                        // tune & getTs0
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_SET_CHANNEL 1\n");
			channel = req->param.data.channel;
			errCode = CmmbFtSetChannel(channel);

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			cnf.result.errCode = errCode;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_SET_CHANNEL 2\n");
			break;

		case META_CMMB_CMD_LIST_SRVS:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_LIST_SRVS 1\n");
                        serv_token = req->header.token;
			errCode = CmmbFtListServices(MetaServiceProc);

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			cnf.result.errCode = errCode;
			cnf.result.data.listSrv.flag= 1;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_LIST_SRVS 2\n");
			break;

		case META_CMMB_CMD_START_SRV:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_START_SRV 1\n");
			serviceId = req->param.data.serviceId;

			if (req->param.szMfsFile[0] == 0)
			{
				META_LOG("META_CMMB_OP() szMfsFile[0]=0 \n");   
				szMfsFile = null;
			}
			else{
				szMfsFile = req->param.szMfsFile;
				META_LOG("META_CMMB_OP(), szMfsFile=%s, len=%d \n",szMfsFile,strlen(szMfsFile));   
			}
			
			errCode = CmmbFtStartService(serviceId, szMfsFile);

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			cnf.result.errCode = errCode;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_START_SRV 2\n");
			break;

		case META_CMMB_CMD_STOP_SRV:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_STOP_SRV 1\n");
			errCode = CmmbFtStopService();

			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			cnf.result.errCode = errCode;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_STOP_SRV 2\n");
			break;

		case META_CMMB_CMD_GET_PROPS:
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_GET_PROPS 1\n");
#if 0			
			propVal = 0;
			errCode = CmmbFtGetProp(CMMB_PROP_SNR, &propVal);
			cnf.result.data.props.SNR = propVal;

			propVal = 0;
			errCode = CmmbFtGetProp(CMMB_PROP_PRE_BER, &propVal);
			cnf.result.data.props.PRE_BER = propVal;

			propVal = 0;
			errCode = CmmbFtGetProp(CMMB_PROP_RSSI, &propVal);
			cnf.result.data.props.RSSI = propVal;
#else
			CmmbProps Props;
			errCode = CmmbFtGetProp(&Props);
			cnf.result.data.props =Props;
			META_LOG("SNR=%d,ber=%d,rssi=%d\n",cnf.result.data.props.SNR,cnf.result.data.props.PRE_BER,cnf.result.data.props.RSSI);
#endif					 	
			if (errCode == CMMB_S_OK)
				cnf.status = CMMB_CNF_DONE;
			else
				cnf.status = CMMB_CNF_FAIL;

			cnf.result.errCode = errCode;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), META_CMMB_CMD_GET_PROPS 2\n");
			break;

		default:
			META_LOG("META_CMMB_OP(), default 1\n");
			cnf.status = CMMB_CNF_FAIL;
			cnf.result.errCode = CMMB_E_INVALID_ARG;
			WriteDataToPC(&cnf, sizeof(cnf), null, 0);
			META_LOG("META_CMMB_OP(), default 2\n");
			break;
	}

	META_LOG("META_CMMB_OP() 2\n");
}
