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

#include "SmsLiteAppDriver.h"
//#include "SmsHostLibTypes.h"
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

//SMS SPI CONF. RELATED
#define SPI_GPIO_LINE				(4)
#define	SPI_PULSE_WIDTH			(400)
//#define	SPI_PULSE_WIDTH			(2000)                  //1202
#define SPI_CONTROLER_NUM		(0)

#define CMMB_USER_FREQUENCY	530000000
#define CMMB_USER_SERVICE		605
void SmsLiteCmmbControlRxCallback( UINT32 HandleNum, UINT8* pBuf, UINT32 BufSize )
{
       printf("xingyu[Test_Siano] conttol callback function is called \n ");
	return;
}

static void SmsLiteCmmbDataCallback( UINT32 HandleNum, UINT8* pBuf, UINT32 BufSize )
{
       printf("xingyu[Test_Siano] data callback function is called \n ");
	return;
}

//*******************************************************************************
// 
typedef struct SmsMsgDataArr_S
{
	SmsMsgHdr_ST				xMsgHeader;
	UINT32						msgData[6];
}SmsMsgDataArr_ST;

//*******************************************************************************

void PrepareGetVersion(SmsMsgData_ST* pSmsMsg)
{
	UINT16 Len = sizeof(SmsMsgHdr_ST);
	memset( pSmsMsg, 0, Len );
	pSmsMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
	pSmsMsg->xMsgHeader.msgDstId = HIF_TASK;
	pSmsMsg->xMsgHeader.msgFlags = 0;
	pSmsMsg->xMsgHeader.msgType  = MSG_SMS_GET_VERSION_EX_REQ;
	pSmsMsg->xMsgHeader.msgLength = Len;
}	

//*******************************************************************************
// 

void PrepareSetInterruptMsg( SmsMsgData_ST* pSetIntrMsg )
{	
	UINT16 Len = sizeof(SmsMsgHdr_ST) + 3 * sizeof(UINT32);
	memset( pSetIntrMsg, 0, Len );

	pSetIntrMsg->xMsgHeader.msgType = MSG_SMS_SPI_INT_LINE_SET_REQ;
	pSetIntrMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
	pSetIntrMsg->xMsgHeader.msgDstId = HIF_TASK;
	pSetIntrMsg->xMsgHeader.msgFlags = 0;
	pSetIntrMsg->xMsgHeader.msgLength = Len;
	pSetIntrMsg->msgData[0] = SPI_CONTROLER_NUM;	// Main Spi Controller (0) 
	pSetIntrMsg->msgData[1] = SPI_GPIO_LINE;		// GPIO 4
	pSetIntrMsg->msgData[2] = SPI_PULSE_WIDTH;		// pulse width 20
}	

//*******************************************************************************
//
void PrepareSetPeriodicStats( SmsMsgData_ST* pSmsMsg)
{
  UINT16 Len = sizeof(SmsMsgHdr_ST) + sizeof(UINT32);
  memset( pSmsMsg, 0, Len );
  pSmsMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB_INTERNAL;
  pSmsMsg->xMsgHeader.msgDstId = HIF_TASK;
  pSmsMsg->xMsgHeader.msgFlags = 0;
  pSmsMsg->xMsgHeader.msgType  = MSG_SMS_SET_PERIODIC_STATISTICS_REQ;
  pSmsMsg->xMsgHeader.msgLength = Len;
  pSmsMsg->msgData[0] = 1; // Set the periodic flag
}

//*******************************************************************************
// 
void PrepareCMMBTuneMsg(SmsMsgData_ST* pMsg, UINT32 Frequency)
{
  SmsMsgDataArr_ST* pCmmbTuneMsg  = (SmsMsgDataArr_ST*)pMsg;
  UINT32 Len = sizeof(SmsMsgHdr_ST) + 3*sizeof(UINT32);
  memset( pCmmbTuneMsg, 0, Len );

  pCmmbTuneMsg->xMsgHeader.msgType = MSG_SMS_RF_TUNE_REQ;
  pCmmbTuneMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB;
  pCmmbTuneMsg->xMsgHeader.msgDstId = HIF_TASK;
  pCmmbTuneMsg->xMsgHeader.msgFlags = 0;
  pCmmbTuneMsg->xMsgHeader.msgLength = Len;

  pCmmbTuneMsg->msgData[0] = Frequency;
  pCmmbTuneMsg->msgData[1] = BW_8_MHZ;
  pCmmbTuneMsg->msgData[2] = 0;
}


//*******************************************************************************
//
void PrepareStartServiceMsg(SmsMsgData_ST* pMsg, UINT32 ServiceId)
{
  SmsMsgDataArr_ST* pStartServiceMsg = (SmsMsgDataArr_ST*)pMsg;
  UINT32 Len = sizeof(SmsMsgHdr_ST) + 4*sizeof(UINT32);
  memset(pStartServiceMsg, 0, Len);

  pStartServiceMsg->xMsgHeader.msgType = MSG_SMS_CMMB_START_SERVICE_REQ;
  pStartServiceMsg->xMsgHeader.msgSrcId = SMS_HOST_LIB;
  pStartServiceMsg->xMsgHeader.msgDstId = HIF_TASK;
  pStartServiceMsg->xMsgHeader.msgFlags = 0;
  pStartServiceMsg->xMsgHeader.msgLength = Len;

  pStartServiceMsg->msgData[0] = 0xFFFFFFFF;
  pStartServiceMsg->msgData[1] = 0xFFFFFFFF;
  pStartServiceMsg->msgData[2] = ServiceId;
  pStartServiceMsg->msgData[3] = 0;
}


int main()
{
   SMSHOSTLIB_ERR_CODES_E RetCode = SMSHOSTLIB_ERR_OK;
   char Buffer[10];
		 
   RetCode = SmsLiteAdrInit( SMSHOSTLIB_DEVMD_CMMB, SmsLiteCmmbControlRxCallback, SmsLiteCmmbDataCallback );
   	if ( RetCode != SMSHOSTLIB_ERR_OK )
	{
		printf("xingyu[Test_Siano] Return err 0x%x\n",RetCode);              //init error
		return RetCode ;
	}
	else
             printf("xingyu[Test_Siano]SmsLiteAdrInit ok\n");


	SmsMsgData_ST* pStaticMsg = (SmsMsgData_ST*)(malloc(sizeof(SmsMsgData_ST)));	
	
	printf("Get version before FW downloading\n");
	PrepareGetVersion(pStaticMsg);
	SmsLiteAdrWriteMsg( pStaticMsg );                    //debug

	printf("Start to tune to frequency %d\n", CMMB_USER_FREQUENCY);
	PrepareCMMBTuneMsg(pStaticMsg, CMMB_USER_FREQUENCY);
	SmsLiteAdrWriteMsg( pStaticMsg );                    //debug


	OSW_TaskSleep(5000);
	printf("Start service %d\n", CMMB_USER_SERVICE);
	PrepareStartServiceMsg(pStaticMsg, CMMB_USER_SERVICE);
	SmsLiteAdrWriteMsg( pStaticMsg );                    //debug

	
	//OSW_TaskSleep(60000);
#if 0
      printf("[Test Siano] using hw\n");

   // driver loop back test, test driver working
      AdrLoopbackTest();
#else
    printf("xingyu[Test_Siano] waiting ............press 'c' stop.........\n ");

    while(1){
	fgets(Buffer, 10, stdin);
	if(Buffer[0] =='c')
		break;
    }
#endif


//driver terminate
       printf("xingyu[Test_Siano]SmsLiteAdrTerminate\n");
       SmsLiteAdrTerminate();
	   
  printf("xingyu[Test_Siano] ****************************** End ******************************* \n\n\n ");
  free(pStaticMsg);
   return 0;
}
