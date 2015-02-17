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
/*   Copyright (C) 2005 Siano Mobile Silicon Ltd. All rights reserved    */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#ifndef _SMS_HOST_LIB_LITE_COMMON_H_
#define _SMS_HOST_LIB_LITE_COMMON_H_

#include "SmsHostLibTypes.h"

#ifdef __cplusplus
extern "C"{
#endif


/*************************************************************************
*			 Macros
*************************************************************************/
#define SMS_MAX_FORMATED_LOG_STRING (512)
#define ZERO_MEM_OBJ(pObj) memset((pObj), 0, sizeof(*(pObj)))
#define SMS_SET_HOST_DEVICE_STATIC_MSG_FIELDS( _pMsg ) \
	{	\
	(_pMsg)->xMsgHeader.msgSrcId = SMS_HOST_LIB; \
	(_pMsg)->xMsgHeader.msgDstId = HIF_TASK; \
	(_pMsg)->xMsgHeader.msgFlags = MSG_HDR_FLAG_STATIC_MSG; \
	}
#define SMSHOSTLIB_DEFAULT_CRYSTAL							(12000000L)	

/*************************************************************************
*			 Enums
*************************************************************************/

// Host Lib Debug Log mask bits
typedef enum
{
	// Severities bits
	SMSLOG_ERROR			= 1 << 30,
	SMSLOG_WARNING			= 1 << 29,

	// General bits
	SMSLOG_MAIN				= 1 << 0, // Main module and internal tasks communication
	SMSLOG_COMM				= 1 << 1, // communication between the host and the chip
	SMSLOG_APIS				= 1 << 2, // communication between the user and the library
	SMSLOG_SRVM				= 1 << 3, // SRVM module
	SMSLOG_SCAN				= 1 << 4, // SCAN module
	SMSLOG_AGING			= 1 << 5, // AGING module
	SMSLOG_FIC				= 1 << 6, // FIC module
	SMSLOG_DABCRC			= 1 << 7, // DABCRC module 
	SMSLOG_PWRMNG			= 1 << 8, // Power Management from FW
	SMSLOG_COMM_LOGS		= 1 << 9,// Logs from FW - as oposite to all other SMSLOG_COMM messages
	SMSLOG_CMMB				= 1 << 10,// Logs for CMMB
	SMSLOG_FW_DEBUG			= 1 << 11, // FW log message
	SMSLOG_FW_INFO			= 1 << 12,
	SMSLOG_FW_ERROR			= 1 << 13,
	SMSLOG_DATA_FLOW		= 1 << 14,// Logs related to service data

	SMSLOG_RESERVED_LAST	= 1 << 28
} SMSHOSTLIB_DBG_LOG_MASK_ET;

/*************************************************************************
*			 Structs/Typedefs
*************************************************************************/

#ifndef SMS_LITE_CB_DEFINED
#define SMS_LITE_CB_DEFINED

//! Callback function prototype for control response messages
typedef void (*SmsHostLiteCbFunc)(	SMSHOSTLIB_MSG_TYPE_RES_E	MsgType,		//!< Response type	
								  SMSHOSTLIB_ERR_CODES_E		ErrCode,		//!< Response success code
								  UINT8* 						pPayload,		//!< Response payload
								  UINT32						PayloadLen );	//!< Response payload length

//! Callback function prototype for service data
typedef void ( *SmsHostLiteDataCbFunc)(UINT32	ServiceHandle, 
									   UINT8*	pBuffer, 
									   UINT32	BufferSize );

#endif


/*************************************************************************
*			 Fwd Declarations
*************************************************************************/

void	SmsLiteCommonControlRxHandler(  UINT32 handle_num, UINT8* p_buffer, UINT32 buff_size );
SMSHOSTLIB_ERR_CODES_E SmsLiteSetDeviceFwLogState( void );
BOOL	SmsHostWaitForFlagSet(BOOL* pFlag, UINT32 TimeToWaitMs);
SMSHOSTLIB_ERR_CODES_E SmsLiteSendCtrlMsg( struct SmsMsgData_S* pMsg );
SMSHOSTLIB_ERR_CODES_E SmsLiteInit( SmsHostLiteCbFunc pCtrlCallback );
void	SmsLiteCallCtrlCallback( SMSHOSTLIB_MSG_TYPE_RES_E MsgType,
							 SMSHOSTLIB_ERR_CODES_E RetCode,
							 void* pPayload,
							 UINT32 PayloadLen);
UINT32 SmsProcessLog( SMSHOSTLIB_LOG_ITEM_ST *p_x_log_item );

/*************************************************************************
*			 Log Macros
*************************************************************************/
#define SMSHOST_ENABLE_LOGS

#ifdef SMSHOST_ENABLE_LOGS
	extern SMSHOSTLIB_API void  SmsHostLog(UINT32 mask,char* format, ...);

	#define SMSHOST_LOG0(mask,format) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION )
	#define SMSHOST_LOG1(mask,format,arg1) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1)
	#define SMSHOST_LOG2(mask,format,arg1,arg2) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2)
	#define SMSHOST_LOG3(mask,format,arg1,arg2,arg3) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3)
	#define SMSHOST_LOG4(mask,format,arg1,arg2,arg3,arg4) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4)
	#define SMSHOST_LOG5(mask,format,arg1,arg2,arg3,arg4,arg5) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5)
	#define SMSHOST_LOG6(mask,format,arg1,arg2,arg3,arg4,arg5,arg6) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6)
	#define SMSHOST_LOG7(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7)
	#define SMSHOST_LOG8(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
	#define SMSHOST_LOG9(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)
	#define SMSHOST_LOG10(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10) \
		SmsHostLog(mask, "l%d %s > " format, SMSHOST_LINE, SMSHOST_FUNCTION, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10)

#else	// SMSHOST_ENABLE_LOGS

	#define SMSHOST_LOG0(mask,format){}
	#define SMSHOST_LOG1(mask,format,arg1){}
	#define SMSHOST_LOG2(mask,format,arg1,arg2){}
	#define SMSHOST_LOG3(mask,format,arg1,arg2,arg3){}
	#define SMSHOST_LOG4(mask,format,arg1,arg2,arg3,arg4){}
	#define SMSHOST_LOG5(mask,format,arg1,arg2,arg3,arg4,arg5){}
	#define SMSHOST_LOG6(mask,format,arg1,arg2,arg3,arg4,arg5,arg6){}
	#define SMSHOST_LOG7(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7){}
	#define SMSHOST_LOG8(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8){}
	#define SMSHOST_LOG9(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9){}
	#define SMSHOST_LOG10(mask,format,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10){}

#endif	// SMSHOST_ENABLE_LOGS


#ifdef __cplusplus
}
#endif

#endif
 

