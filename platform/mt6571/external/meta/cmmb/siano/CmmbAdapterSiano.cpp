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

// CmmbAdapterSiano.cpp - CMMB hostlib adapter layer, Siano version
//
#ifndef CMMB_CHIP_INNO

#include "SmsHostLibLiteCmmb.h"
#include "SmsHostLibLiteCommon.h"
//#include "SmsCmmbParser.h"
#include "sms_msgs.h"
#include "SmsLiteAppDriver.h"
//#include "CmmbServiceProvider.h"
#include "CmmbHelper.h"
#include "CmmbAdapter.h"
//xingyu 1126
#include  "cmmb_errcode.h"
#include "sms_msgs.h"
#include "SmsHostLibLiteCommon.h"

#define SMS_UAM_MAX_ATR_LEN		64
#define SMS_UAM_DEFAULT_ATR_LEN	21

extern UINT8  g_bufTs0[MAX_TS0_SIZE];
extern UINT32 g_sizeTs0;
// asynchronous response event
SMSHOSTLIB_ERR_CODES_E g_rspCode;

// used when start service
UINT32 g_serviceId;		// service ID
UINT32 g_subFrmIdx;		// sub-frame index
UINT32 g_serviceHdl;	// service handle

SMSHOSTLIB_STATISTICS_CMMB_ST g_cmmbStat;

struct CmmbUamAsyncData
{
	UINT8	pAttrBuf[SMS_UAM_MAX_ATR_LEN];
	UINT8	attrLen;
	UINT8*	pOutReadBuf;
	UINT32	readBufSize;
	UINT32* pOutReadLen;
};

CmmbUamAsyncData g_uamAsyncData;

void Siano_UserCtrlCallback(SMSHOSTLIB_MSG_TYPE_RES_E MsgType, SMSHOSTLIB_ERR_CODES_E ErrCode, UINT8* pPayload, UINT32 PayloadLen);
void Siano_UserDataCallback(UINT32 serviceHdl, UINT8* buffer, UINT32 bufSize);
//*******************************************************************************
//
CmmbResult ConvertErrorCode(SMSHOSTLIB_ERR_CODES_E smsCode)
{
	CmmbResult errCode;

	switch (smsCode)
	{
		case SMSHOSTLIB_ERR_OK:
			errCode = CMMB_S_OK;
			break;
/*
		case INNO_TIMEOUT_ERROR:            //?//xingyu  need modify for timeout
			errCode = CMMB_E_TIMEOUT;
			break;

		case INNO_PARAMETER_ERROR:            //?//xingyu  need modify for timeout
			errCode = CMMB_E_INVALID_ARG;
			break;
*/
		default:
			errCode = CMMB_E_UNKNOWN;
			break;
	}

	return errCode;
}
#if 0
void  SmsLiteCmmbTune_Req( UINT32 Frequency, SMSHOSTLIB_FREQ_BANDWIDTH_ET Bandwidth )
{
	SmsMsgData2Args_ST SmsMsg = SmsMsgData2Args_ST();
	SP_LOGW("Frequency %u, Bandwidth %d", Frequency, Bandwidth );

	if ( !g_IsLibInit )
	{
		SP_LOGE("Error Driver not init");	
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_RF_TUNE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = Frequency;		
	SmsMsg.msgData[1] = Bandwidth;		

	SMSHOSTLIB_ERR_CODES_E RetCode = SmsLiteAdrWriteMsg(SmsMsg);
	SP_LOGE("END");
}

void  SmsLiteCmmbStartTs0_Req( void )
{
	SmsMsgData2Args_ST SmsMsg = SmsMsgData2Args_ST();
	SP_LOGW("START");

	if ( !g_IsLibInit )
	{
		SP_LOGE("Driver not init");	
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_START_CONTROL_INFO_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = SMS_CMMB_NETWORK_LEVEL_USE_CURRENT_NETWORK;	// Unique network level
	SmsMsg.msgData[1] = 0xFFFFFFFF;	// Don't care

	SMSHOSTLIB_ERR_CODES_E RetCode = SmsLiteAdrWriteMsg(SmsMsg);
	SP_LOGE("END");
}

void  SmsLiteCmmbStopTs0_Req( void )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SP_LOGW("START");
	if ( !g_IsLibInit )
	{
		SP_LOGE("Driver not init");	
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SMSHOSTLIB_ERR_CODES_E RetCode = SmsLiteAdrWriteMsg(SmsMsg);
	SP_LOGE("END");
}

//*******************************************************************************
// 
void  SmsLiteCmmbStartService_Req( UINT32 ServiceId )
{
	SmsMsgData3Args_ST SmsMsg = SmsMsgData3Args_ST();

	SP_LOGW("START");
	if ( !g_IsLibInit )
	{
		SP_LOGE("Driver not init");	
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_START_SERVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = SMS_CMMB_NETWORK_LEVEL_USE_CURRENT_NETWORK;	// Unique network level
	SmsMsg.msgData[1] = 0xFFFFFFFF;	// Don't care
	SmsMsg.msgData[2] = ServiceId;

	SMSHOSTLIB_ERR_CODES_E RetCode = SmsLiteAdrWriteMsg(SmsMsg);
	SP_LOGE("END");
}

//*******************************************************************************
// 
void  SmsLiteCmmbStopService_Req( UINT32 ServiceHandle )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SP_LOGW("START");
	if ( !g_IsLibInit )
	{
		SP_LOGE("Driver not init");	
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_STOP_SERVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = ServiceHandle;

	SMSHOSTLIB_ERR_CODES_E RetCode = SmsLiteAdrWriteMsg(SmsMsg);
	SP_LOGE("END");
}
//////////////////////////////////////////////////////////////////////////////////////
#endif
CmmbResult HostLibInit()
{
	// initialize internal data
	g_curReq = REQ_NONE;
	g_rspEvent.Clear();

	// initialize host library
	typedef SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST CMMB_INITLIB_PARAMS;
	CMMB_INITLIB_PARAMS initParams = CMMB_INITLIB_PARAMS();
	SMSHOSTLIB_ERR_CODES_E smsCode;

	initParams.Size = sizeof(initParams);
	initParams.pCtrlCallback = Siano_UserCtrlCallback; 
	initParams.pDataCallback = Siano_UserDataCallback;

	smsCode = SmsLiteCmmbLibInit(&initParams);  
	return ConvertErrorCode(smsCode);
}

CmmbResult HostLibTerminate()
{
	SMSHOSTLIB_ERR_CODES_E smsCode;
	smsCode = SmsLiteCmmbLibTerminate();

	return ConvertErrorCode(smsCode);
}


int cmmbFrequency1[]=
{
	474000000,482000000,490000000,498000000,506000000,514000000,522000000,530000000,538000000,
	546000000,554000000,562000000,610000000,618000000,626000000,634000000,642000000,650000000,
	658000000,666000000,674000000,682000000,690000000,698000000,706000000,714000000,722000000,
	730000000,738000000,746000000,754000000,762000000,770000000,778000000,786000000,794000000
};
#define CMMB_CHANNEL_MIN	13
#define CMMB_CHANNEL_MAX	48
inline int CmmbFreqFromChannel1(int channel)
{
	return cmmbFrequency1[channel-CMMB_CHANNEL_MIN];
}

CmmbResult Tune(int channel)
{
	CmmbResult errCode;

	 UINT32 frequency=CmmbFreqFromChannel1(channel);
	// send request
	g_curReq = REQ_TUNE;
	g_rspEvent.Clear();
	g_rspCode = SMSHOSTLIB_ERR_UNDEFINED_ERR;
	SmsLiteCmmbTune_Req(frequency, BW_8_MHZ);

	// wait for response
	errCode = g_rspEvent.Wait(CMMB_CMD_RES_MAX_TIME);
	g_curReq = REQ_NONE;

	if (errCode == CMMB_S_OK
			&& g_rspCode != SMSHOSTLIB_ERR_OK)
	{
		errCode = ConvertErrorCode(g_rspCode);
	}

	return errCode;
}

CmmbResult StartTs0()
{
	CmmbResult errCode;

	// send request
	g_curReq = REQ_START_TS0;
	g_rspEvent.Clear();
	SmsLiteCmmbStartTs0_Req();

	// wait for response
	errCode = g_rspEvent.Wait(CMMB_CMD_RES_MAX_TIME);
	g_curReq = REQ_NONE;

	return errCode;
}

CmmbResult StopTs0()
{
	CmmbResult errCode;

	// send request
	g_curReq = REQ_STOP_TS0;
	g_rspEvent.Clear();
	SmsLiteCmmbStopTs0_Req();

	// wait for response
	errCode = g_rspEvent.Wait(CMMB_CMD_RES_MAX_TIME);
	g_curReq = REQ_NONE;

	return errCode;
}

CmmbResult StartService(int serviceId)
{
	CmmbResult errCode;

	g_serviceHdl = 0;

	// send request
	g_curReq = REQ_START_SERVICE;
	g_rspEvent.Clear();
	g_serviceId = serviceId;
	SmsLiteCmmbStartService_Req(serviceId);

	// wait for response
	errCode = g_rspEvent.Wait(CMMB_CMD_RES_MAX_TIME);
	g_curReq = REQ_NONE;

	return errCode;
}

CmmbResult StopService()
{
	CmmbResult errCode;

	// send request
	g_curReq = REQ_STOP_SERVICE;
	g_rspEvent.Clear();
	SmsLiteCmmbStopService_Req(g_serviceHdl);

	// wait for response
	errCode = g_rspEvent.Wait(CMMB_CMD_RES_MAX_TIME);
	g_curReq = REQ_NONE;

	return errCode;
}

CmmbResult GetStatistics()
{
	CmmbResult errCode;

	// send request
	g_curReq = REQ_GET_STAT;
	g_rspEvent.Clear();
	SmsLiteCmmbGetStatistics_Req();

	// wait for response
	errCode = g_rspEvent.Wait(CMMB_CMD_RES_MAX_TIME);
	g_curReq = REQ_NONE;

	return errCode;
}
CmmbProps  g_props ={0};
CmmbResult GetSystemStatus(CmmbProps* props)
{
         memset(&g_props,0,sizeof(CmmbProps));
         CmmbResult ret = GetStatistics();
	  if(ret == CMMB_S_OK)
        {
        	*props = g_props;
	 }
        return ret;
}	
/***********************************************************************************************
Function:  Siano_UserCtrlCallback
Parameters:
MsgType : Callback message type, defined in SMSHOSTLIB_MSG_TYPE_RES_E
ErrCode : Error code associated with the message
pPayload: Pointer to additional data associated with the message
PayloadLen: Length (in bytes) of the additional data
Returns:  none
Side Effects:  -
Description:  Control callback function. This function is being given to SMS11xx host library
as a callback for control events.
 ***********************************************************************************************/

void Siano_UserCtrlCallback(SMSHOSTLIB_MSG_TYPE_RES_E MsgType, SMSHOSTLIB_ERR_CODES_E ErrCode, UINT8* pPayload, UINT32 PayloadLen)
{
	SMSHOST_LOG2(SMSLOG_CMMB, "MsgType: %d, ErrCode: 0x%x", MsgType, ErrCode);

	switch(MsgType)
	{
		case SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES:
			if (g_curReq == REQ_GET_STAT)
			{
				SMSHOSTLIB_STATISTICS_CMMB_ST* cmmb_status =(SMSHOSTLIB_STATISTICS_CMMB_ST*)pPayload;
				g_props.RSSI= cmmb_status->InBandPwr;
				g_props.SNR = cmmb_status->SNR>>16;
				g_props.PRE_BER = cmmb_status->BER;
				SP_LOGE("SNR=%d,BER=%d,RSSI=%d",g_props.SNR,g_props.PRE_BER,g_props.RSSI);
				SetResponseEvent();
			}
			break;

		case SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES:
			if (g_curReq == REQ_START_SERVICE
					&& ErrCode == SMSHOSTLIB_ERR_OK
					&& g_serviceId == ((UINT32*)pPayload)[2])
			{
				g_serviceHdl = ((UINT32*)pPayload)[0];
				g_subFrmIdx  = ((UINT32*)pPayload)[1];
                            SP_LOGE("sevhdl=%d,sfidx=%d",g_serviceHdl ,g_subFrmIdx);
				SetResponseEvent();
			}
	
			break;

		case SMSHOSTLIB_MSG_TUNE_RES:
			if (g_curReq == REQ_TUNE)
			{
				g_rspCode = ErrCode;
				SetResponseEvent();
			}
			break;

		case SMSHOSTLIB_MSG_CMMB_STOP_TS0_RES:
			if (g_curReq == REQ_STOP_TS0)
				SetResponseEvent();
			break;

		case SMSHOSTLIB_MSG_CMMB_STOP_SERVICE_RES:
			if (g_curReq == REQ_STOP_SERVICE)
				SetResponseEvent();
			break;

		case SMSHOSTLIB_MSG_CMMB_START_TS0_RES:
			if (g_curReq == REQ_START_TS0){
				SP_LOGE("start ts0");
//				SetResponseEvent();
			}
			break;

		case SMSHOSTLIB_MSG_CMMB_HOST_NOTIFICATION_IND:
			// conditions: ESG changes, Emergency broadcast changes, Changes in any one of 
			// the control information tables, F/W descrambling error conditions.
			break;

		case SMSHOSTLIB_MSG_GET_VERSION_RES:
			// this is used to test if the Siano chip works normal
			SMSHOST_LOG1(SMSLOG_CMMB, "SMS Version: %s", (char*)pPayload);
			break;
		default:
			break;
	}
}

/***********************************************************************************************
Function:  Siano_UserDataCallback
Parameters:
serviceHdl :  Handle associated with the service which this data belongs to
The handle is identical to the service handle received in the response to
a previous start service call
buffer :	      Pointer to buffer holding a multiplex frame
bufSize: 	    Length of the multiplex frame (in bytes).
Description:  Control callback function. This function is being given to
SMS11xx host library as a callback for data events.
 ***********************************************************************************************/

void Siano_UserDataCallback(UINT32 serviceHdl, UINT8* buffer, UINT32 bufSize)
{
	SMSHOSTLIB_ERR_CODES_E smsCode;

	// for backward compatibility
	if (bufSize == 0)
		return;

	SMSHOST_LOG1(SMSLOG_CMMB, "get a Mpx frame, serviceHdl: %d", serviceHdl);

	if (serviceHdl == SERVICE_HANDLE_TS0)	
	{ 
		// TS0: containing control tables
		if (g_curReq == REQ_START_TS0)
		{
			// save TS0 data
			assert(bufSize <= MAX_TS0_SIZE);
			memcpy(g_bufTs0, buffer, bufSize);
			g_sizeTs0 = bufSize;

			SetResponseEvent();
		}
	}
	else // not TS0: containing service data or ESG data part
	{
		if (mfsWriter.isValid()
			&& mfsWriter.fileSize() < MAX_MFS_FILE_SIZE)
		{
			mfsWriter.write(buffer, bufSize);
		}
	}

	SMSHOST_LOG1(SMSLOG_CMMB, "get a Mpx frame end, serviceHdl: %d", serviceHdl);
}
/*
UINT32 GetSignalQuality()
{
	UINT32 quality = 0;

	if (GetStatistics() == CMMB_ERR_OK)
		quality = g_cmmbStat.ReceptionQuality;

	return quality;
}

UINT32 GetSignalFrequency()
{
	UINT32 freq = 0;

	if (GetStatistics() == CMMB_ERR_OK)
		freq = g_cmmbStat.Frequency;

	return freq;
}

UINT32 GetModemState()
{
	UINT32 state = 0;

	if (GetStatistics() == CMMB_ERR_OK)
		state = g_cmmbStat.ModemState;

	return state;
}
*/
#endif // !CMMB_CHIP_INNO
