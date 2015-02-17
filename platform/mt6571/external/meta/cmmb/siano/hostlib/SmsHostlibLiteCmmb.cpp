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

#include "SmsHostLibLiteCmmb.h"
#include "sms_msgs.h"
#include "SmsHostLibLiteCommon.h"
#include "SmsLiteAppDriver.h"
#include "SmsHostUtils.h"
#include "Osw.h"

/*************************************************************************
*			 Macros
*************************************************************************/

#define MAX_DIR_NAME_SIZE	256
#define SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE (3)
// The buffer should hold one multiplex frame
// We set a bound of one max RS table (288 rows X 240 bytes per row).
// The bound is not set formally by the standard - it's a de-facto bound.
#define SMS_CMMB_DATA_BUFFER_SIZE (288*240)	
#define SMS_CMMB_MAX_CONTROL_FRAME_SIZE (8640)


/*************************************************************************
*			 Structs
*************************************************************************/

// Host lib state global to all instances
typedef struct SmsLiteCmmbGlobalState_S
{
	BOOL						IsLibInit;
	SmsHostLiteDataCbFunc		pDataCallback; 
	UINT32						Crystal;
	BOOL						SyncFlag;
	UINT32						ReceptionQualityHistory[SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE];
	UINT32						ReceptionQualityCounter;
	BOOL						IsBlockInterface;
	UINT32						SubframeIndexPerHandleArr[SMS_MAX_SERVICE_HANDLE+1];

	// MPX frame buffer state
	UINT8*	pMpxBuf;
	UINT32	MpxBufOffset;
	UINT32	MpxBufSize;
	UINT32	MpxFrameSize;
	UINT32	MpxHandleNum;
	UINT32	MpxFrameId;
} SmsLiteCmmbGlobalState_ST ;

/*************************************************************************
*			 Fwd Declarations
*************************************************************************/
static UINT32 SmsHostCmmbGetQuality( SMSHOSTLIB_STATISTICS_CMMB_ST *stats );
static void SmsLiteCmmbDataCallback(  UINT32 HandleNum, UINT8* pBuf, UINT32 BufSize );
static void SmsLiteCmmbControlRxCallback(  UINT32 HandleNum, UINT8* pBuf, UINT32 BufSize );
static BOOL SmsLiteCmmbIsMpxFrameHeader( const UINT8* pBuf, UINT32 BufSize, UINT32* pOutFrameSize, UINT32* pOutFrameId );
static void SmsFlushMpxBufferToApp( void );

/*************************************************************************
*			 Globals
*************************************************************************/
SmsLiteCmmbGlobalState_ST	g_LibCmmbState;
static UINT8 g_MpxFrameBuffer[SMS_CMMB_DATA_BUFFER_SIZE] = {0};


//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbLibInit( SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST* pInitLibParams )
{
	SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST LocalInitParams = SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST();
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();
	UINT32 i;

	SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB,"Crystal=%d", pInitLibParams->Crystal );
 
	if ( g_LibCmmbState.IsLibInit )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB | SMSLOG_ERROR,"Already init Return err 0x%x",SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED);
	//	return SMSHOSTLIB_ERR_LIB_ALREADY_INITIATED;
              return	SMSHOSTLIB_ERR_OK;
	}

	ZERO_MEM_OBJ(&g_LibCmmbState);
	for ( i = 0 ; i < SMS_MAX_SERVICE_HANDLE+1 ; i++ )
	{
		g_LibCmmbState.SubframeIndexPerHandleArr[i] = SMS_CMMB_DEMUX_ANY_SUBFRAME_INDEX;
	}

	if ( pInitLibParams == NULL 
		|| pInitLibParams->pCtrlCallback == NULL 
		|| pInitLibParams->Size == 0 )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB | SMSLOG_ERROR,"Return err 0x%x",SMSHOSTLIB_ERR_INVALID_ARG);
		return SMSHOSTLIB_ERR_INVALID_ARG;
	}

	memcpy( &LocalInitParams, pInitLibParams, pInitLibParams->Size );
	
	SmsLiteInit( LocalInitParams.pCtrlCallback );
	g_LibCmmbState.pDataCallback = LocalInitParams.pDataCallback;
	g_LibCmmbState.Crystal = LocalInitParams.Crystal;
	g_LibCmmbState.pMpxBuf = g_MpxFrameBuffer;
	g_LibCmmbState.MpxBufSize = sizeof( g_MpxFrameBuffer );

	if ( LocalInitParams.Crystal == 0 )
	{
		g_LibCmmbState.Crystal = SMSHOSTLIB_DEFAULT_CRYSTAL;
	}

  
	RetCode = SmsLiteAdrInit( SMSHOSTLIB_DEVMD_CMMB, SmsLiteCmmbControlRxCallback, SmsLiteCmmbDataCallback );
	if ( RetCode != SMSHOSTLIB_ERR_OK )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB | SMSLOG_ERROR,"Return err 0x%x",RetCode);
		return RetCode ;
	}

	// Set crystal message
	//Aaron - we don't check the Crystal parameter... just send the message to tuner.
	if ( g_LibCmmbState.Crystal != SMSHOSTLIB_DEFAULT_CRYSTAL )
	{
		SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
		SmsMsg.xMsgHeader.msgType  = MSG_SMS_NEW_CRYSTAL_REQ;
		SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

		SmsMsg.msgData[0] = g_LibCmmbState.Crystal;

		g_LibCmmbState.SyncFlag = FALSE;
		SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg );

		// Wait for device init response
		if ( !SmsHostWaitForFlagSet( &g_LibCmmbState.SyncFlag, 200 ) )
		{
			return SMSHOSTLIB_ERR_DEVICE_NOT_INITIATED;
		}
	}


#ifdef SMSHOST_ENABLE_LOGS
	//SmsLiteSetDeviceFwLogState();
#endif

	g_LibCmmbState.IsLibInit = TRUE ;

	//SmsLiteGetVersion_Req();
	 
	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"LibInit OK");
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbLibTerminate( void )
{
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"START");

	g_LibCmmbState.IsLibInit = FALSE;
	g_LibCmmbState.pDataCallback = NULL;

	RetCode = SmsLiteAdrTerminate();
	if ( RetCode != SMSHOSTLIB_ERR_OK )
	{
		SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB | SMSLOG_ERROR,"Return err 0x%x",RetCode);
		return RetCode;
	}

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbGetStatistics_Req( void )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();
	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"START");

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_GET_STATISTICS_EX_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}



//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbTune_Req( UINT32 Frequency, SMSHOSTLIB_FREQ_BANDWIDTH_ET Bandwidth )
{
	SmsMsgData2Args_ST SmsMsg = SmsMsgData2Args_ST();
	SMSHOST_LOG2(SMSLOG_APIS | SMSLOG_CMMB,"Frequency %u, Bandwidth %d", Frequency, Bandwidth );

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_TUNE_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_RF_TUNE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = Frequency;		
	SmsMsg.msgData[1] = Bandwidth;		

	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbStartTs0_Req( void )
{
	SmsMsgData2Args_ST SmsMsg = SmsMsgData2Args_ST();

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"START");
	
	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_START_CONTROL_INFO_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_START_CONTROL_INFO_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	SmsMsg.msgData[0] = SMS_CMMB_NETWORK_LEVEL_USE_CURRENT_NETWORK;	// Unique network level
	SmsMsg.msgData[1] = 0xFFFFFFFF;	// Don't care


	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbStopTs0_Req( void )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"START");

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_STOP_CONTROL_INFO_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbStartService_Req( UINT32 ServiceId )
{
	SmsMsgData3Args_ST SmsMsg = SmsMsgData3Args_ST();

	SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB,"ServiceId %d", ServiceId);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_START_SERVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);
	
	SmsMsg.msgData[0] = SMS_CMMB_NETWORK_LEVEL_USE_CURRENT_NETWORK;	// Unique network level
	SmsMsg.msgData[1] = 0xFFFFFFFF;	// Don't care
	SmsMsg.msgData[2] = ServiceId;


	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbStopService_Req( UINT32 ServiceHandle )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB,"Service Handle %d", ServiceHandle);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_STOP_SERVICE_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_STOP_SERVICE_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = ServiceHandle;

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbSetPeriodicStatistics_Req( BOOL IsEnabled )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB,"IsEnabled %d", IsEnabled);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_SET_PERIODIC_STATISTICS_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_SET_PERIODIC_STATISTICS_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = IsEnabled;

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbSetAutoOuputTs0_Req( BOOL IsEnabled )
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB,"IsEnabled %d", IsEnabled);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_SET_AUTO_OUTPUT_TS0_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsMsg.msgData[0] = IsEnabled;

	SmsLiteSendCtrlMsg( &SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}


//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbSetCaControlWords_Req( UINT32 SvcHdl, 
													 UINT32 SfIdx, 
													 SMSHOSTLIB_CA_CW_PAIR_ST *pControlWords)
{

	UINT8 pMsgBuf[sizeof(SmsMsgHdr_ST) + sizeof(SMSHOSTLIB_SET_CA_CW_ST)]; 
	SmsMsgData_ST *pSmsMsg = (SmsMsgData_ST*)pMsgBuf; 
	SMSHOSTLIB_SET_CA_CW_ST *pCwInfo = (SMSHOSTLIB_SET_CA_CW_ST *)pSmsMsg->msgData;
	
	SMSHOST_LOG4(SMSLOG_APIS | SMSLOG_CMMB,"Hdl=%d SfIdx=%d Key ids=(0x%02X, 0x%02X)", 
		SvcHdl, SfIdx, pControlWords->Even.Id, pControlWords->Odd.Id);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_SET_CA_CW_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	pCwInfo->SvcHdl = SvcHdl;
	pCwInfo->SfIdx = SfIdx;
	pCwInfo->CwPair = *pControlWords; 


	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( pSmsMsg );
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_CMMB_SET_CA_CW_REQ;
	pSmsMsg->xMsgHeader.msgLength = sizeof(pMsgBuf);


	SmsLiteSendCtrlMsg( pSmsMsg ); 


}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbSetCaSaltKeys_Req( UINT32 SvcHdl, 
												 UINT32 SfIdx, 
												 UINT8 pVidSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE], 
												 UINT8 pAudSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE],
												 UINT8 pDataSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE] )
{
	UINT8 pMsgBuf[sizeof(SmsMsgHdr_ST) + sizeof(SMSHOSTLIB_CA_SET_SALT_ST)]; 
	SmsMsgData_ST *pSmsMsg = (SmsMsgData_ST*)pMsgBuf; 
	SMSHOSTLIB_CA_SET_SALT_ST *pSaltInfo = (SMSHOSTLIB_CA_SET_SALT_ST *)pSmsMsg->msgData;

	SMSHOST_LOG2(SMSLOG_APIS | SMSLOG_CMMB,"Hdl=%d SfIdx=%d", 
		SvcHdl, SfIdx);

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_SET_CA_SALT_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	pSaltInfo->SvcHdl = SvcHdl;
	pSaltInfo->SfIdx = SfIdx;

	if (pVidSalt != NULL)
	{
		memcpy(pSaltInfo->pVidSalt, pVidSalt, SMSHOSTLIB_CMMB_CA_SALT_SIZE); 
	}

	if (pAudSalt != NULL)
	{
		memcpy(pSaltInfo->pAudSalt, pAudSalt, SMSHOSTLIB_CMMB_CA_SALT_SIZE); 
	}

	if (pDataSalt != NULL)
	{
		memcpy(pSaltInfo->pDataSalt, pDataSalt, SMSHOSTLIB_CMMB_CA_SALT_SIZE); 
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( pSmsMsg );
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_CMMB_SET_CA_SALT_REQ;
	pSmsMsg->xMsgHeader.msgLength = sizeof(pMsgBuf);


	SmsLiteSendCtrlMsg( pSmsMsg ); 

}


//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbUAMInit ()
{
	SmsMsgData_ST SmsMsg = SmsMsgData_ST();

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"");

	if ( !g_LibCmmbState.IsLibInit )
	{
		return SMSHOSTLIB_ERR_LIB_NOT_INITIATED;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_SMART_CARD_INIT_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);


	SmsLiteSendCtrlMsg( &SmsMsg ); 


	return SMSHOSTLIB_ERR_OK; 
}



#define SMS_MAX_SMARTCARD_MSG_SIZE 252


//*******************************************************************************
// 
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbUAMSend(UINT8* pBuff, UINT32 size)
{
	UINT32 isFirst = TRUE;
	UINT32 headerSize = sizeof(SmsMsgHdr_ST)+sizeof(UINT32);
	SmsMsgData_ST* pSmsMsg;
	UINT32 msgSize = 0;
	UINT32 payloadSize;
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;
	
	if ( !g_LibCmmbState.IsLibInit )
	{
		return SMSHOSTLIB_ERR_LIB_NOT_INITIATED;
	}

	while (size > 0)
	{
		msgSize = SMS_MIN (size+headerSize, SMS_MAX_SMARTCARD_MSG_SIZE);
		payloadSize = msgSize - headerSize;

		pSmsMsg = (SmsMsgData_ST*)OSW_MemAlloc(msgSize);

		if(pSmsMsg == NULL)
		{
			return SMSHOSTLIB_ERR_MEM_ALLOC_FAILED; 
		}

		pSmsMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
		pSmsMsg->xMsgHeader.msgDstId = HIF_TASK;
		pSmsMsg->xMsgHeader.msgFlags = 0;
		pSmsMsg->xMsgHeader.msgType  = MSG_SMS_SMART_CARD_WRITE_REQ;
		pSmsMsg->xMsgHeader.msgLength = (UINT16)msgSize;

		pSmsMsg->msgData[0] = isFirst;
		isFirst = FALSE;

		memcpy(&pSmsMsg->msgData[1], pBuff, payloadSize);

		SmsLiteSendCtrlMsg (pSmsMsg);

		size -= payloadSize;
		pBuff += payloadSize;
	}	

	// If last message was exactly one full buffer, send another zero-length message to terminate
	if (msgSize == SMS_MAX_SMARTCARD_MSG_SIZE)
	{
		pSmsMsg = (SmsMsgData_ST*)OSW_MemAlloc(headerSize);

		if(pSmsMsg == NULL)
		{
			return SMSHOSTLIB_ERR_MEM_ALLOC_FAILED; 
		}

		pSmsMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
		pSmsMsg->xMsgHeader.msgDstId = HIF_TASK;
		pSmsMsg->xMsgHeader.msgFlags = 0;
		pSmsMsg->xMsgHeader.msgType  = MSG_SMS_SMART_CARD_WRITE_REQ;
		pSmsMsg->xMsgHeader.msgLength = (UINT16)headerSize;
		pSmsMsg->msgData[0] = 0;
		SmsLiteSendCtrlMsg (pSmsMsg);
	}

	return SMSHOSTLIB_ERR_OK;
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbMbbmsProtoSend_Req(UINT8* pBuff, UINT32 Size)
{
	SmsMsgData_ST* pSmsMsg;
	UINT32 msgSize = 0;
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"");

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_MBBMS_RX_IND,
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
}

	msgSize = Size+sizeof(SmsMsgHdr_ST);
	if (msgSize > SMS_MAX_SMARTCARD_MSG_SIZE)
	{
		// Too large
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_MBBMS_RX_IND,
			SMSHOSTLIB_ERR_OUT_OF_BOUNDS, NULL, 0 );
		return;
	}

	
	pSmsMsg = (SmsMsgData_ST*)OSW_MemAlloc(msgSize);

	pSmsMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
	pSmsMsg->xMsgHeader.msgDstId = HIF_TASK;
	pSmsMsg->xMsgHeader.msgFlags = 0;
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_MBBMS_WRITE_REQ;
	pSmsMsg->xMsgHeader.msgLength = (UINT16)msgSize;

	memcpy(pSmsMsg->msgData, pBuff, Size);

	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)pSmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API SmsLiteCmmbSmdSn_Req()
{
	//Reply: 4 bytes status followed by 8 bytes sequence number
	SmsMsgData3Args_ST SmsMsg = SmsMsgData3Args_ST();

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"");

	if ( !g_LibCmmbState.IsLibInit )
	{
		SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_CMMB_SMD_SN_RES, 
			SMSHOSTLIB_ERR_LIB_NOT_INITIATED, NULL, 0 );
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( &SmsMsg );
	SmsMsg.xMsgHeader.msgType  = MSG_SMS_CMMB_SMD_SN_REQ;
	SmsMsg.xMsgHeader.msgLength = (UINT16)sizeof(SmsMsg);

	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)&SmsMsg ); 

	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");
}

//*******************************************************************************
// 
void SMSHOSTLIB_API
	SmsLiteCmmbSetEadtMockup( UINT32 EmmServiceId, UINT32 ReservedZero1, UINT32 ReservedZero2 )
{
	UINT8 pMsgBuf[sizeof(SmsMsgHdr_ST) + 4*sizeof(UINT32)]; 
	SmsMsgData3Args_ST* pSmsMsg = (SmsMsgData3Args_ST*)pMsgBuf; 

	SMSHOST_LOG3(SMSLOG_APIS | SMSLOG_CMMB,"EADT Mockup. EmmServiceId = %d, ReservedZero1 = %d, ReservedZero2 = %d", 
		EmmServiceId, ReservedZero1, ReservedZero2);

	if ( !g_LibCmmbState.IsLibInit )
	{
		return;
	}

	SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( pSmsMsg );
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_CMMB_SET_CA_SALT_REQ;
	pSmsMsg->xMsgHeader.msgLength = sizeof(pMsgBuf);


	pSmsMsg->msgData[0] = MSG_SMS_DEBUG_HOST_EVENT_REQ;
	
	pSmsMsg->msgData[1] = EmmServiceId; 
	pSmsMsg->msgData[2] = ReservedZero1; 
	pSmsMsg->msgData[3] = ReservedZero2; 

	SmsLiteSendCtrlMsg( (SmsMsgData_ST*)pSmsMsg ); 
	SMSHOST_LOG0(SMSLOG_APIS | SMSLOG_CMMB,"END");

}

//*******************************************************************************
// 
void SmsLiteCmmbControlRxCallback( UINT32 HandleNum, UINT8* pBuf, UINT32 BufSize )
{
	SmsMsgData_ST* pSmsMsg = (SmsMsgData_ST*)pBuf;
	SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_UNDEFINED_ERR;
	UINT32 ResponseMsgType = SMSHOSTLIB_MSG_INVALID_RESPONSE_VAL;
	UINT8* pPayload = NULL;
	UINT32* pPayloadUint32 = NULL;
	UINT32 PayloadLength = 0;

	// Return code and payload for the messages which have retcode as the first 4 bytes
	UINT8* pPayloadWoRetCode = NULL;
	UINT32 PayloadLengthWoRetCode = 0;
	SMSHOSTLIB_ERR_CODES_E RetCodeFromMsg = SMSHOSTLIB_ERR_UNDEFINED_ERR;

	SMS_ASSERT( HandleNum == 0 );
	SMS_ASSERT( pBuf != NULL );
	SMS_ASSERT( BufSize != 0 );
	SMS_ASSERT( BufSize >= pSmsMsg->xMsgHeader.msgLength );
	SMS_ASSERT( pSmsMsg->xMsgHeader.msgLength >= sizeof( SmsMsgHdr_ST ) );

	pPayload = (UINT8*)&pSmsMsg->msgData[0];
	PayloadLength = pSmsMsg->xMsgHeader.msgLength - sizeof( SmsMsgHdr_ST );
	pPayloadUint32 = (UINT32*)pPayload;

	if ( PayloadLength >= 4 )
	{
		RetCodeFromMsg = (SMSHOSTLIB_ERR_CODES_E)pSmsMsg->msgData[0];
		pPayloadWoRetCode = pPayload + 4;
		PayloadLengthWoRetCode = PayloadLength - 4;
	}
	
	/*
	SMSHOST_LOG3( SMSLOG_ERROR, "Control RX callback. Type %d, Retcode %#x, Payload Length %d", 
		pSmsMsg->xMsgHeader.msgType,
		RetCodeFromMsg,
		PayloadLength );
	 */

	switch( pSmsMsg->xMsgHeader.msgType )
	{
	case MSG_SMS_GET_STATISTICS_EX_RES:
		{
			SMSHOSTLIB_STATISTICS_CMMB_ST* pStats = (SMSHOSTLIB_STATISTICS_CMMB_ST*)pPayloadWoRetCode;
			UINT32 i;
			UINT32 RecQualSum = 0;
			SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_GET_STATISTICS_EX_RES. RetCode %#x", RetCodeFromMsg );
			ResponseMsgType = SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode; 

			if( RetCodeFromMsg == SMSHOSTLIB_ERR_OK )
			{
				i = g_LibCmmbState.ReceptionQualityCounter++ % SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE;
				g_LibCmmbState.ReceptionQualityHistory[i] = SmsHostCmmbGetQuality( pStats );
				for ( i = 0 ; i < SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE ; i++ )
				{
					RecQualSum += g_LibCmmbState.ReceptionQualityHistory[i];
				}
				pStats->ReceptionQuality = RecQualSum / SMS_CMMB_RECEPTION_QUALITY_HISTORY_SIZE;
			}
		}
		break;
	case MSG_SMS_RF_TUNE_RES:
		{
			SMSHOST_LOG2(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_RF_TUNE_RES. Freq %u RetCode %#x", 
				((UINT32*)pPayloadWoRetCode)[0],RetCodeFromMsg);
			ResponseMsgType = SMSHOSTLIB_MSG_TUNE_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_START_CONTROL_INFO_RES:
		{
			SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_CMMB_START_CONTROL_INFO_RES. RetCode %#x", RetCodeFromMsg );
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_START_CONTROL_INFO_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode; 
		}
		break;
	case MSG_SMS_CMMB_STOP_CONTROL_INFO_RES:
		{
			SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_CMMB_STOP_CONTROL_INFO_RES. RetCode %#x", RetCodeFromMsg );
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_STOP_CONTROL_INFO_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_START_SERVICE_RES:
		{

			SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_CMMB_START_SERVICE_RES. RetCode %#x", RetCodeFromMsg );
			if ( RetCodeFromMsg == SMSHOSTLIB_ERR_OK || RetCodeFromMsg == SMSHOSTLIB_ERR_ALREADY_ACTIVE )
			{
				UINT32 ServiceHandle = ((UINT32*)pPayloadWoRetCode)[0];
				UINT32 SubFrameIndex = ((UINT32*)pPayloadWoRetCode)[1];
				UINT32 ServiceId = ((UINT32*)pPayloadWoRetCode)[2];
				SMSHOST_LOG3(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_CMMB_START_SERVICE_RES. Handle %d, Subframe Index %d, Service ID %d", 
					ServiceHandle, SubFrameIndex, ServiceId );
				g_LibCmmbState.SubframeIndexPerHandleArr[ServiceHandle] = SubFrameIndex;
			}

			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_STOP_SERVICE_RES:
		{

			SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_CMMB_STOP_SERVICE_RES. RetCode %#x", RetCodeFromMsg );
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_STOP_SERVICE_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_HOST_NOTIFICATION_IND:
		{
			SMSHOST_LOG1(SMSLOG_APIS | SMSLOG_CMMB, "MSG_SMS_HOST_NOTIFICATION_IND. ID %d", ((UINT32*)pPayload)[0] );
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_HOST_NOTIFICATION_IND;
			RetCode = SMSHOSTLIB_ERR_OK;
		}
		break;
	case MSG_SMS_LOG_ENABLE_CHANGE_RES:
		// NOP
		break;
	case MSG_SMS_INTERFACE_LOCK_IND:
		{
			// Firmware has requested that the host avoid sending messages to it
			SMSHOST_LOG0(SMSLOG_COMM, "Interface lock");
			g_LibCmmbState.IsBlockInterface = TRUE;
		}
		break;
	case MSG_SMS_INTERFACE_UNLOCK_IND:
		{
			// Firmware allows host to resume transmission
			SMSHOST_LOG0(SMSLOG_COMM, "Interface unlock");
			g_LibCmmbState.IsBlockInterface = FALSE;
		}
		break;
	case MSG_SMS_SET_PERIODIC_STATISTICS_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_SET_PERIODIC_STATISTICS_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_SET_AUTO_OUTPUT_TS0_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break;
	case MSG_SMS_NEW_CRYSTAL_RES:
		{
			g_LibCmmbState.SyncFlag = TRUE;
		}
		break;

	case MSG_SMS_CMMB_SET_CA_CW_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_SET_CA_CW_RES;
			RetCode = RetCodeFromMsg;
		}
		break; 

	case MSG_SMS_CMMB_SET_CA_SALT_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_SET_CA_SALT_RES;
			RetCode = RetCodeFromMsg;
		}
		break; 

	case MSG_SMS_SMART_CARD_READ_IND:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_SMART_CARD_RX_IND;
			RetCode = SMSHOSTLIB_ERR_OK;
		}
		break;
	case MSG_SMS_CMMB_SMD_SN_RES:
		{
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_SMD_SN_RES;
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
		}
		break; 

	case MSG_SMS_NSCD_INIT_RES:
	case MSG_SMS_NSCD_OPEN_SESSION_RES:
	case MSG_SMS_NSCD_PROCESS_SECTION_RES:
	case MSG_SMS_DBD_CREATE_OBJECT_RES:
	case MSG_SMS_DBD_CONFIGURE_RES:
	case MSG_SMS_DBD_SET_KEYS_RES:
	case MSG_SMS_DBD_PROCESS_GET_DATA_RES:
	case MSG_SMS_DBD_PROCESS_HEADER_RES:
	case MSG_SMS_DBD_PROCESS_DATA_RES:
	case MSG_SMS_SEND_HOST_DATA_TO_DEMUX_RES:
		{
			RetCode = RetCodeFromMsg;
			pPayload = pPayloadWoRetCode;
			PayloadLength = PayloadLengthWoRetCode;
			ResponseMsgType = pSmsMsg->xMsgHeader.msgType;
		}
		break; 

	case MSG_SMS_CMMB_CA_SERVICE_IND:
		{
			const char* pStr = "Unknown";
			ResponseMsgType = SMSHOSTLIB_MSG_CMMB_CA_SERVICE_IND;
			if ( PayloadLength < 2*sizeof(UINT32) )
			{
				RetCode = SMSHOSTLIB_ERR_UNDEFINED_ERR;
				break;
			}

			switch(pPayloadUint32[0])
			{
			case SMS_CA_IND_ACCESS_DENIED:
				pStr = "Access Denied.";
				break;
			case SMS_CA_IND_EXCEEDED_MAX_SCRAMBLED_SERVICES:
				pStr = "Exceeded max services.";
				SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR, "Error - Exceeded max open scrambled services.");
				break;
			case SMS_CA_IND_SALT_KEYS_NOT_SET:
				pStr = "Salt keys not set.";
				break;
			case SMS_CA_IND_CW_NOT_SET:
				pStr = "Code words (CWs) not set.";
				break;
			case SMS_CA_IND_CW_MISSING_KEY:
				pStr = "Missing a CW for descrambling.";
				break;

			default:
				SMSHOST_LOG0(SMSLOG_APIS|SMSLOG_ERROR, "Error - Undefined CA service indication.");
			}

			SMSHOST_LOG2(SMSLOG_APIS, "CA Service indication - %s. Service %d", pStr, pPayloadUint32[1]);
			RetCode = SMSHOSTLIB_ERR_OK;
		}
		break; 
	case MSG_SMS_MBBMS_WRITE_RES:
		break;
	case MSG_SMS_MBBMS_READ_IND:
		ResponseMsgType = SMSHOSTLIB_MSG_MBBMS_RX_IND;
		RetCode = SMSHOSTLIB_ERR_OK;

		break;
	default:
		SmsLiteCommonControlRxHandler( HandleNum, pBuf, BufSize );
		break;
	}

	if ( !g_LibCmmbState.IsLibInit )
	{
		return;
	}

	// Call the user callback
	if ( ResponseMsgType != SMSHOSTLIB_MSG_INVALID_RESPONSE_VAL )
	{
		SmsLiteCallCtrlCallback( (SMSHOSTLIB_MSG_TYPE_RES_E)ResponseMsgType, 
								RetCode,
								pPayload,
								PayloadLength ); 
	} 
}

//*******************************************************************************
// 
static void SmsFlushMpxBufferToApp( void )
{
	if ( g_LibCmmbState.MpxBufOffset > 0 )
	{
		SMSHOST_LOG2( SMSLOG_DATA_FLOW, "Outputting data to app. Handle %d, Size %d",
			g_LibCmmbState.MpxHandleNum, 
			g_LibCmmbState.MpxBufOffset ); 

		g_LibCmmbState.pDataCallback( g_LibCmmbState.MpxHandleNum, 
			g_LibCmmbState.pMpxBuf,
			g_LibCmmbState.MpxBufOffset ); 
	}
	g_LibCmmbState.MpxBufOffset = 0;
	g_LibCmmbState.MpxHandleNum = 0;
	g_LibCmmbState.MpxFrameSize = 0;
	g_LibCmmbState.MpxFrameId = 40;
};

//*******************************************************************************
// Data callback from ADR
// This function is the entry point of data to the host library
// It is a good place to put a breakpoint when debugging data issues
//
// The function accumulates a full multiplex frame in memory and only then calls 
// the user data callback.
// 
//
static void SmsLiteCmmbDataCallback( UINT32 HandleNum, UINT8* pBuf, UINT32 BufSize )
{
	UINT32 NewFrameSize = 0;
	UINT32 NewFrameId = 0;
	UINT32 SizeToCopy = BufSize;

	if ( g_LibCmmbState.pDataCallback == NULL )
	{
		return;
	}
	
	SmsLiteCmmbIsMpxFrameHeader( pBuf, BufSize, &NewFrameSize, &NewFrameId );
	if ( g_LibCmmbState.MpxBufOffset > 0 )
	{
		if ( NewFrameSize > 0 // --> It's a beginning of a new frame )
			|| g_LibCmmbState.MpxBufOffset + BufSize > g_LibCmmbState.MpxBufSize 
			|| HandleNum != g_LibCmmbState.MpxHandleNum )
		{
			SMSHOST_LOG3( SMSLOG_WARNING|SMSLOG_DATA_FLOW, "Warning - flushing MPX frame. Buffer Handle %d, size in buffer %d, Frame size %d",
				g_LibCmmbState.MpxHandleNum,
				g_LibCmmbState.MpxBufOffset,
				g_LibCmmbState.MpxFrameSize );

			SmsFlushMpxBufferToApp();
		}
	}

	if ( NewFrameSize > 0 && BufSize > NewFrameSize )
	{
		//// The buffer contains more than the frame - weird... Might happen in some drivers
		//// when execution of data callback is held (breakpoint or something).
		//// Discard the remainder - we handle maximum one frame per callback
		//SMSHOST_LOG2( SMSLOG_WARNING|SMSLOG_CMMB, 
		//	"Warning - data callback contains more than one frame. Frame size %d, "
		//	"data callback buffer size %d. Discarding the remainder.",
		//	NewFrameSize,
		//	BufSize );
		
		SizeToCopy = NewFrameSize;		
	}
	g_LibCmmbState.MpxHandleNum = HandleNum;

	if ( SizeToCopy > g_LibCmmbState.MpxBufSize || SizeToCopy > g_LibCmmbState.MpxBufSize )
	{
		SMSHOST_LOG3( SMSLOG_ERROR|SMSLOG_DATA_FLOW, "Error! Data size too large. Buffer size %d, Current chunk size %d, Frame Size %d. Discarding data.",
			g_LibCmmbState.MpxBufSize,
			BufSize,
			g_LibCmmbState.MpxBufSize );

		SmsFlushMpxBufferToApp();
		return;
	}

	if ( g_LibCmmbState.MpxFrameSize == 0 )
	{
		if ( NewFrameSize == 0 )
		{
			// No frame header detected - discard data
			SMSHOST_LOG1( SMSLOG_ERROR, "Error - (handle %d) frame buffer is 0 and no frame header detected. Discarding data.",
				HandleNum );
			
			return;
		}

		g_LibCmmbState.MpxFrameSize = NewFrameSize;
		g_LibCmmbState.MpxFrameId = NewFrameId;
	}
    
	memcpy( &g_LibCmmbState.pMpxBuf[g_LibCmmbState.MpxBufOffset],
		pBuf,
		SizeToCopy );
	g_LibCmmbState.MpxBufOffset += SizeToCopy;

	if ( g_LibCmmbState.MpxBufOffset >= g_LibCmmbState.MpxFrameSize )
	{	
		SmsFlushMpxBufferToApp();
	}

}

//*******************************************************************************
// Reception quality tables
//

UINT32 SmsHostCmmbGetQuality( SMSHOSTLIB_STATISTICS_CMMB_ST *stats )
{
	UINT32 receptionQuality = 0 ;
	UINT32	sum = 0;
	UINT32	NumOfGrades = 0;
	// Quality table
	// Each row is a constellation
	// Each column is the threshold for LDPC cycles. (units are 100 cycles)
	// Column 0 is grade 100 quality, col 1 - 75, 2 - 50, 3 - 25,
	// and the rest is 0.
	const UINT8 QualityTable[3][4] = 
	{
		{ 20, 35, 45, 75 },	// BPSK 
		{ 15, 20, 35, 45 },	// QPSK 
		{ 16, 24, 32, 40 },	// 16QAM 
	};

	if (stats->NumActiveChannels > 0)
	{
		UINT32 i;
		for ( i = 0 ; i < stats->NumActiveChannels ; i++ )
		{
			UINT32 j;
			INT32 grade = 0 ;
			SMSHOSTLIB_CMMB_CHANNEL_STATS_ST* pChannelStats = &stats->ChannelsStatsArr[i];
			if (pChannelStats->LdpcCycleCountAvg > 0)
			{
				const UINT8* pRow = QualityTable[pChannelStats->Constellation];
				grade = 100;
				for ( j = 0 ; j < 4 ; j++ )
				{
					if ( pChannelStats->LdpcCycleCountAvg < (UINT32)pRow[j]*100 )
					{
						break;
					}
					grade -= 25;
				}
				sum += grade;
				NumOfGrades++;
			}

		}

		if ( NumOfGrades > 0 )
		{
			receptionQuality = (sum/NumOfGrades) * 4 / 100 + 1;
		}

		if (  stats->BER != SMS_CMMB_INVALID_BER && stats->BER > 0 )
		{
			// When there is BER - the reception quality is maximum 1
			receptionQuality = 1;
		}
	}

	return (UINT32)receptionQuality;
}

//*******************************************************************************
// Undocumented API to send a FW message
SMSHOSTLIB_API SMSHOSTLIB_ERR_CODES_E SmsLiteCmmbSendFwMsg( SmsMsgData_ST* pMsg )
{
	return SmsLiteSendCtrlMsg( pMsg );
}


//*******************************************************************************
// Check if the given pBuf is a multiplex frame header
//
static BOOL SmsLiteCmmbIsMpxFrameHeader( const UINT8* pBuf, UINT32 BufSize, UINT32* pOutFrameSize, UINT32* pOutFrameId )
{
	UINT32 HeaderLengthInclCrc;
	UINT32 NumSubFrames;
	UINT32 Offset;
	
	*pOutFrameSize = 0;
	// Check for start code ( 00 00 00 01 )
	if ( BufSize < 11 || pBuf[0] != 0 || pBuf[1] != 0 || pBuf[2] != 0 || pBuf[3] != 1 )
	{
		return FALSE;
	}

	// Get the header length
	HeaderLengthInclCrc = pBuf[4] + 4;
	if ( HeaderLengthInclCrc > BufSize )
	{
		return FALSE;
	}

	// Compute the header CRC
	if ( SmsCRC32Compute(0xFFFFFFFF, pBuf, HeaderLengthInclCrc) != 0 )
	{
		return FALSE;
	}

	// Frame ID is the lower 6 bits of the byte offset 6
	*pOutFrameId = pBuf[6]&0x3F;

	// Calculate the multiplex frame size
	// The size consists of header size + CRC + sizes of all sub frames
	// Sizes of sub frames are stored in an array at offset 11 in the 
	// frame header
	NumSubFrames = pBuf[11]&0xF;
	Offset = 12;
	// Add up all the sub frames' lengths
	while( Offset < BufSize && NumSubFrames > 0 )
	{
		*pOutFrameSize += pBuf[Offset] << 16;
		Offset++;
		*pOutFrameSize += pBuf[Offset] << 8;
		Offset++;
		*pOutFrameSize += pBuf[Offset];
		Offset++;
		NumSubFrames--;
	}
	*pOutFrameSize += HeaderLengthInclCrc;

	return TRUE;
}
