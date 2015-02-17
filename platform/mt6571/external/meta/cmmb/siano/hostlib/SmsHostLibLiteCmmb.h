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

#ifndef _SMS_HOST_LIB_LITE_CMMB_H_
#define _SMS_HOST_LIB_LITE_CMMB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "SmsHostLibTypes.h" 

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
	
//! Structure of the init parameters passed to #SmsLiteCmmbLibInit
typedef struct SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_S
{
	UINT32						Size;				//!< Put sizeof(#SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST)into this field
	SmsHostLiteCbFunc			pCtrlCallback;		//!< Callback for control responses
	SmsHostLiteDataCbFunc		pDataCallback;		//!< Callback for asynchronous data reading
	UINT32						Crystal;			//!< The crystal frequency used in the chip. 12MHz is the default - use 0 to leave unchanged.
} SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST;

/*************************************************************************
*			 SMS Host Library Lite APIs for CMMB
**************************************************************************/

/** @defgroup group2 SMS Host Library Lite APIs for CMMB
*  @{
*/

/*************************************************************************/
/*!
Initiates the SMS host library.
	\param[in]	pInitLibParams	Library initialization information, according to #SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST structure
	\return		Error code by  #SMSHOSTLIB_ERR_CODES_E enumerator.
	\remark		This function is synchronous.
*/
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbLibInit( SMSHOSTLIBLITE_CMMB_INITLIB_PARAMS_ST* pInitLibParams );

/*************************************************************************/
/*!
Terminates the SMS host library.
	\return		Error code by  #SMSHOSTLIB_ERR_CODES_E enumerator.
	\remark		This command execution is synchronous.
*/
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbLibTerminate( void );


/*************************************************************************/
/*!
Request the version of the firmware and the host library.
	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with
	\c			a message type #SMSHOSTLIB_MSG_GET_VERSION_RES
	\c			The response callback payload contains a NULL terminated ASCII 
	\c			string, containing the version of the host library and firmware.
*/
void SMSHOSTLIB_API SmsLiteGetVersion_Req( void );

/*************************************************************************/
/*!
Requests statistics info of the tuner
		\return		The return value is void. 
		\c			The status of the function is returned by an 
		\c			asynchronous call to the control callback, with
		\c			a message type #SMSHOSTLIB_MSG_GET_STATISTICS_EX_RES

	\c			The response payload contains a structure that contains the statistics.
	\c			The structure is #SMSHOSTLIB_STATISTICS_CMMB_ST 
*/
void SMSHOSTLIB_API SmsLiteCmmbGetStatistics_Req( void ); 

/*************************************************************************/
/*!
Set a mask value that controls the logging from the host library
	\param[in]	newDbgLogMask - A bit mask that controls which logs are outputted, and which not.
	\c			a value of 0 means no logs. A value of 0xFFFFFFFF means all the possible logs
	\c			The effect of each bit in the mask is undocumented.
	\return		status #SMSHOSTLIB_ERR_CODES_E 
*/
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteSetDbgLogMask(UINT32 newDbgLogMask);

/*************************************************************************/
/*!
Get the current log mask value 
	\param[out]	pCurDbgLogMask - the current mask is returned in this out parameter
	\return		status #SMSHOSTLIB_ERR_CODES_E 
	\see		#SmsLiteSetDbgLogMask
*/
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteGetDbgLogMask(UINT32* pCurDbgLogMask);

/*************************************************************************/
/*!
Tune to a specific frequency
	\param[in] Frequency	Frequency Frequency (Hz)
	\param[in] Bandwidth	Bandwidth, according to #SMSHOSTLIB_FREQ_BANDWIDTH_ET
	\c						enumerator. In CMMB the value should be BW_8_MHZ
	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with
	\c			a message type #SMSHOSTLIB_MSG_TUNE_RES .

	\c			The response is called after the tuner has switched to the 
	\c			frequency and determined if it contains a valid CMMB transmission.
	\c			A status of #SMSHOSTLIB_ERR_OK means that the signal is valid.
	\c			A status of #SMSHOSTLIB_ERR_MODEM_NOT_LOCKED means that the tuner did not find
	\c			a valid CMMB transmission.
	\remark		The response may be called a few seconds after the request.
*/
void SMSHOSTLIB_API SmsLiteCmmbTune_Req( UINT32 Frequency, SMSHOSTLIB_FREQ_BANDWIDTH_ET Bandwidth );

/*************************************************************************/
/*!
Start to output the content time slot 0.
Time slot 0 contains control information tables.
After a call to this function time slot 0 data will start flowing
to the user data callback with service handle 1.
	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with a message 
	\c			type #SMSHOSTLIB_MSG_CMMB_START_TS0_RES .
*/
void SMSHOSTLIB_API SmsLiteCmmbStartTs0_Req( void );

/*************************************************************************/
/*!
Stop to output the content time slot 0.
	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with a message 
	\c			type #SMSHOSTLIB_MSG_CMMB_STOP_TS0_RES .
*/
void SMSHOSTLIB_API SmsLiteCmmbStopTs0_Req( void );

/*************************************************************************/
/*!
Requests to begin reception of a given service
	\param[in]  ServiceId			Service Identifier
	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with a message 
	\c			type #SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES .
	
	\c	Additional data is returned in the response payload. The response payload format is:
	\c		4 bytes: UINT32 ServicHandle - The service handle
	\c		4 bytes: UINT32 SubFrameIndex - The service sub-frame index
	\c		4 bytes: UINT32 ServiceId - The echoed back the requested service ID.  
	\c
	\c 	The SubFrameIndex value is  needed when there is more than one service in 
	\c 	a multiplex frame. It is sometimes the case with audio-only services.
	\c 	In such case, when you get the output of a multiplex frame, you need to know
	\c 	the sub-frame index to use in order to play the requested Service ID.
*/
void SMSHOSTLIB_API SmsLiteCmmbStartService_Req( UINT32 ServiceId );

/*************************************************************************/
/*!
Requests to stop reception of a service

	\param[in]	ServiceHandle	Handle of given service, obtained from a successful response
	\c							of #SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES

	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with a message 
	\c			type #SMSHOSTLIB_MSG_CMMB_STOP_SERVICE_RES .
*/
void SMSHOSTLIB_API SmsLiteCmmbStopService_Req( UINT32 ServiceHandle ); 

/*************************************************************************/
/*!
Enable/disable periodic statistics 

Configure the SMS device to output statistics periodically 
The statistics will be outputted at least once a second, 
without a call to SmsLiteCmmbGetStatistics_Req

	\param[in]	IsEnabled	0 means disabled, otherwise - enabled

	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with a message 
	\c			type #SMSHOSTLIB_MSG_SET_PERIODIC_STATISTICS_RES .
*/
void SMSHOSTLIB_API SmsLiteCmmbSetPeriodicStatistics_Req( BOOL IsEnabled );

/*************************************************************************/
/*!
Enable/disable automatic outputting of time slot 0 

Configure the SMS device to output the control information (time slot 0) 
when any changes in it are detected.
After enabling this - the control information will be outputted occasionally
on service handle 1.
It will be outputted whenever the tuner has detected a change in the control 
information, but unlike SmsHostApiCmmbStartControlInfo_Req, it will not necessarily
be every second, thus it does not raise the power consumption of the tuner.

	\param[in]	IsEnabled	0 means disabled, otherwise - enabled

	\return		The return value is void. 
	\c			The status of the function is returned by an 
	\c			asynchronous call to the control callback, with a message 
	\c			type #SMSHOSTLIB_MSG_CMMB_SET_AUTO_OUTPUT_TS0_RES.
*/
void SMSHOSTLIB_API SmsLiteCmmbSetAutoOuputTs0_Req( BOOL IsEnabled );

/*************************************************************************/
/*!
Set CA control words pair (current and next) to F/W descrambler

\param[in]	SvcHdl           The service handle
\param[in]	SfIdx			 The sub-frame index of the service (see SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES parameters)
\param[in]	pControlWords    The control words - see #SMSHOSTLIB_CA_CW_PAIR_ST.
\c					The pControlWords struct contains a pair of control words, held in pControlWords->Odd and
\c					pControlWords->Even. To set a single control word - put it in the Odd control word in the pair,
\c					and use #SMSHOSTLIB_CMMB_INVALID_CW_ID as the ID of the Even control word.

\return		The return value is void. 
\c			The status of the function is returned by an 
\c			asynchronous call to the control callback, with a message 
\c			type #SMSHOSTLIB_MSG_CMMB_SET_CA_CW_RES.
*/
void SMSHOSTLIB_API SmsLiteCmmbSetCaControlWords_Req( UINT32 SvcHdl, UINT32 SfIdx, SMSHOSTLIB_CA_CW_PAIR_ST *pControlWords);



/*************************************************************************/
/*!
Set ISMACrypt video and audio salt keys to F/W descrambler



\param[in]	SvcHdl           The service handle
\param[in]	SfIdx			 The sub-frame index of the service (see SMSHOSTLIB_MSG_CMMB_START_SERVICE_RES parameters)
\param[in]	pVidSalt		 The video salt key
\param[in]	pAudSalt		 The audio salt key
\param[in]	pAudSalt		 The data salt key
\c							 Any of the salt keys can be NULL - in that case they will be ignored.

\return		The return value is void. 
\c			The status of the function is returned by an 
\c			asynchronous call to the control callback, with a message 
\c			type #SMSHOSTLIB_MSG_CMMB_SET_CA_SALT_RES.
*/
void SMSHOSTLIB_API SmsLiteCmmbSetCaSaltKeys_Req( UINT32 SvcHdl, 
												 UINT32 SfIdx, 
												 UINT8 pVidSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE], 
												 UINT8 pAudSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE],
												 UINT8 pDataSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE] );

/*************************************************************************/
/*!
Initialize the User Authorization Module (UAM)

\return		The return value is void.

*/
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbUAMInit (void);


/*************************************************************************/
/*!
Write data to UAM 
Write a buffer of data to the on-chip UAM, and reads a response.
The response is delivered asynchronously with an indication sent to
the control callback - see #SMSHOSTLIB_MSG_SMART_CARD_RX_IND.
This API should be used only with 1186 chips. Other chips (e.g. 1180) do
not contain a UAM.

\param[in]	pBuff		Pointer to data to transmit
\param[in]	size		size of data in bytes

\return		Error code by  #SMSHOSTLIB_ERR_CODES_E enumerator.

*/
SMSHOSTLIB_ERR_CODES_E SMSHOSTLIB_API SmsLiteCmmbUAMSend (UINT8* pBuff, UINT32 size);


/*************************************************************************/
/*!
Send an MBBMS protocol message to device
Send an opaque buffer to the device.
The buffer contains a proprietary protocol used to communicate with 
the FW for accessing the UAM. 
The content of these messages are set by the Siano MBBMS library.
This function is meant to be used only as a porting layer for the
Siano MBBMS stack. It should not be used for any other application
use.

\param[in]	pBuff		Pointer to data to transmit
\param[in]	size		size of data in bytes

\return		The return value is void. 
\c			The MBBMS protocol messages always have response
\c			messages. When the status is OK, the response
\c			is delivered with a message type of #SMSHOSTLIB_MSG_MBBMS_RX_IND

*/
void SMSHOSTLIB_API SmsLiteCmmbMbbmsProtoSend_Req(UINT8* pBuff, UINT32 Size);


/*************************************************************************/
/*!
Get the serial number of an on-board Nagra SMD card 

The response payload contains the SMD SN in an array of 8 bytes.
Note that after power-up, the SMD take a few seconds to initialize.
During that period this response may return an error value 
of SMSHOSTLIB_ERR_NOT_INITIALIZED. In this case, the
application should wait for up to 5 seconds.

\return		The return value is void. 
\c			The information will be returned in asynchronous call to the control callback, 
\c          with a message type #SMSHOSTLIB_MSG_CMMB_SMD_SN_RES.
*/
void SMSHOSTLIB_API SmsLiteCmmbSmdSn_Req( void ); 


/*************************************************************************/
/*!
Set values for mock-up of EADT table (CA table).

This API is for development only - it is for use in labs only.
This API should not be used in a commercial application version.
It is needed when using an SG (signal generator) to broadcast an encrypted 
stream. Then you need to feed the EADT table values artificially (if the SG
does not have such a capability).
To use it, the user should call this function only once - before tuning for the 
first time.
Note that this is a debugging API, and it might change or be removed.

\param[in]	EmmServiceId	The ID of the service carrying the EMM
\param[in]	ReservedZero1	Reserved for future use. Must be zero.
\param[in]	ReservedZero2	Reserved for future use. Must be zero.
\return		Error code by  #SMSHOSTLIB_ERR_CODES_E enumerator.
\remark		This command execution is asynchronous, no response is issued.
\c			The user should call this function and sleep for 20 ms.
*/
void SMSHOSTLIB_API
	SmsLiteCmmbSetEadtMockup( UINT32 EmmServiceId, UINT32 ReservedZero1, UINT32 ReservedZero2 );

/**
*  @}
*/


#ifdef __cplusplus
}
#endif

#endif //_SMS_HOST_LIB_LITE_CMMB_H_
