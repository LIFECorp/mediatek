/*
 * ANT Stack
 *
 * Copyright 2011 Dynastream Innovations
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************\
*
*   FILE NAME:      ant_native_chardev.c
*
*   BRIEF:
*      This file provides the VFS implementation of ant_native.h
*      VFS could be Character Device, TTY, etc.
*
*
\******************************************************************************/

#include <errno.h>
#include <fcntl.h> /* for open() */
#include <linux/ioctl.h> /* For hard reset */
#include <pthread.h>
#include <stdint.h> /* for uint64_t */
#include <sys/eventfd.h> /* For eventfd() */
#include <unistd.h> /* for read(), write(), and close() */

#include "ant_types.h"
#include "ant_native.h"
#include "ant_version.h"

#include "antradio_power.h"
#include "ant_rx_chardev.h"
#include "ant_hci_defines.h"
#include "ant_log.h"

#if ANT_HCI_SIZE_SIZE > 1
#include "ant_utils.h"  // Put HCI Size value across multiple bytes
#endif

#define MESG_BROADCAST_DATA_ID               ((ANT_U8)0x4E)
#define MESG_ACKNOWLEDGED_DATA_ID            ((ANT_U8)0x4F)
#define MESG_BURST_DATA_ID                   ((ANT_U8)0x50)
#define MESG_EXT_BROADCAST_DATA_ID           ((ANT_U8)0x5D)
#define MESG_EXT_ACKNOWLEDGED_DATA_ID        ((ANT_U8)0x5E)
#define MESG_EXT_BURST_DATA_ID               ((ANT_U8)0x5F)
#define MESG_ADV_BURST_DATA_ID               ((ANT_U8)0x72)

static ant_rx_thread_info_t stRxThreadInfo;
static pthread_mutex_t stEnabledStatusLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t stFlowControlLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t stFlowControlCond = PTHREAD_COND_INITIALIZER;
ANTNativeANTStateCb g_fnStateCallback;

static const uint64_t EVENT_FD_PLUS_ONE = 1L;

static void ant_channel_init(ant_channel_info_t *pstChnlInfo, const char *pcCharDevName);

////////////////////////////////////////////////////////////////////
//  ant_init
//
//  Initialises the native environment.
//
//  Parameters:
//      -
//
//  Returns:
//      ANT_STATUS_SUCCESS if intialize completed, else ANT_STATUS_FAILED
//
//  Psuedocode:
/*
Set variables to defaults
Initialise each supported path to chip
Setup eventfd object.
RESULT = ANT_STATUS_SUCCESS if no problems else ANT_STATUS_FAILED
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_init(void)
{
   ANTStatus status = ANT_STATUS_FAILED;
   ANT_FUNC_START();

   stRxThreadInfo.stRxThread = 0;
   stRxThreadInfo.ucRunThread = 0;
   stRxThreadInfo.ucChipResetting = 0;
   stRxThreadInfo.pstEnabledStatusLock = &stEnabledStatusLock;
   g_fnStateCallback = 0;

#ifdef ANT_DEVICE_NAME // Single transport path
    ANT_DEBUG_V("The charcter device is %s", ANT_DEVICE_NAME);
   ant_channel_init(&stRxThreadInfo.astChannels[SINGLE_CHANNEL], ANT_DEVICE_NAME);
#else // Separate data/command paths
   ant_channel_init(&stRxThreadInfo.astChannels[COMMAND_CHANNEL], ANT_COMMANDS_DEVICE_NAME);
   ant_channel_init(&stRxThreadInfo.astChannels[DATA_CHANNEL], ANT_DATA_DEVICE_NAME);
#endif // Separate data/command paths

   // Make the eventfd. Want it non blocking so that we can easily reset it by reading.
   stRxThreadInfo.iRxShutdownEventFd = eventfd(0, EFD_NONBLOCK);

   // Check for error case
   if(stRxThreadInfo.iRxShutdownEventFd == -1)
   {
      ANT_ERROR("ANT init failed. Could not create event fd. Reason: %s", strerror(errno));
   } else {
      status = ANT_STATUS_SUCCESS;
   }

   ANT_FUNC_END();
   return status;
}

////////////////////////////////////////////////////////////////////
//  ant_deinit
//
//  clean up eventfd object
//
//  Parameters:
//      -
//
//  Returns:
//      ANT_STATUS_SUCCESS
//
//  Psuedocode:
/*
RESULT = SUCCESS
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_deinit(void)
{
   ANTStatus result_status = ANT_STATUS_FAILED;
   ANT_FUNC_START();

   if(close(stRxThreadInfo.iRxShutdownEventFd) < 0)
   {
      ANT_ERROR("Could not close eventfd in deinit. Reason: %s", strerror(errno));
   } else {
      result_status = ANT_STATUS_SUCCESS;
   }

   ANT_FUNC_END();
   return result_status;
}


////////////////////////////////////////////////////////////////////
//  ant_enable_radio
//
//  Powers on the ANT part and initialises the transport to the chip.
//  Changes occur in part implementing ant_enable() call
//
//  Parameters:
//      -
//
//  Returns:
//      Success:
//          ANT_STATUS_SUCCESS
//      Failures:
//          ANT_STATUS_TRANSPORT_INIT_ERR if could not enable
//          ANT_STATUS_FAILED if failed to get mutex or init rx thread
//
//  Psuedocode:
/*
LOCK enable_LOCK
    State callback: STATE = ENABLING
    ant enable
    IF ant_enable success
        State callback: STATE = ENABLED
        RESULT = SUCCESS
    ELSE
        ant disable
        State callback: STATE = Current state
        RESULT = FAILURE
    ENDIF
UNLOCK
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_enable_radio(void)
{
   int iLockResult;
   ANTStatus result_status = ANT_STATUS_FAILED;
   ANT_FUNC_START();

   ANT_DEBUG_V("getting stEnabledStatusLock in %s", __FUNCTION__);
   iLockResult = pthread_mutex_lock(&stEnabledStatusLock);
   if(iLockResult) {
      ANT_ERROR("enable failed to get state lock: %s", strerror(iLockResult));
      goto out;
   }
   ANT_DEBUG_V("got stEnabledStatusLock in %s", __FUNCTION__);

   if (g_fnStateCallback) {
      g_fnStateCallback(RADIO_STATUS_ENABLING);
   }

   if (ant_enable() < 0) {
      ANT_ERROR("ant enable failed: %s", strerror(errno));

      ant_disable();

      if (g_fnStateCallback) {
         g_fnStateCallback(ant_radio_enabled_status());
      }
   } else {
      if (g_fnStateCallback) {
         g_fnStateCallback(RADIO_STATUS_ENABLED);
      }

      result_status = ANT_STATUS_SUCCESS;
   }

   ANT_DEBUG_V("releasing stEnabledStatusLock in %s", __FUNCTION__);
   pthread_mutex_unlock(&stEnabledStatusLock);
   ANT_DEBUG_V("released stEnabledStatusLock in %s", __FUNCTION__);

out:
   ANT_FUNC_END();
   return result_status;
}

////////////////////////////////////////////////////////////////////
//  ant_radio_hard_reset
//
//  IF SUPPORTED triggers a hard reset of the chip providing ANT functionality.
//
//  Parameters:
//      -
//
//  Returns:
//      Success:
//          ANT_STATUS_SUCCESS
//      Failures:
//          ANT_STATUS_NOT_SUPPORTED if the chip can't hard reset
//          ANT_STATUS_FAILED if failed to get mutex or enable
//
//  Psuedocode:
/*
IF Hard Reset not supported
    RESULT = NOT SUPPORTED
ELSE
  LOCK enable_LOCK
  IF Lock failed
        RESULT = FAILED
  ELSE
    Set Flag Rx thread that chip is resetting
    FOR each path to chip
        Send Reset IOCTL to path
    ENDFOR
    ant disable
    ant enable
    IF ant_enable success
        State callback: STATE = RESET
        RESULT = SUCCESS
    ELSE
        State callback: STATE = DISABLED
        RESULT = FAILURE
    ENDIF
    Clear Flag Rx thread that chip is resetting
  UNLOCK
ENDIF
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_radio_hard_reset(void)
{
   ANTStatus result_status = ANT_STATUS_NOT_SUPPORTED;
   ANT_FUNC_START();

#ifdef ANT_IOCTL_RESET
   ant_channel_type eChannel;
   int iLockResult;

   result_status = ANT_STATUS_FAILED;

   ANT_DEBUG_V("getting stEnabledStatusLock in %s", __FUNCTION__);
   iLockResult = pthread_mutex_lock(&stEnabledStatusLock);
   if(iLockResult) {
      ANT_ERROR("enable failed to get state lock: %s", strerror(iLockResult));
      goto out;
   }
   ANT_DEBUG_V("got stEnabledStatusLock in %s", __FUNCTION__);

   stRxThreadInfo.ucChipResetting = 1;
   if (g_fnStateCallback)
      g_fnStateCallback(RADIO_STATUS_RESETTING);

#ifdef ANT_IOCTL_RESET_PARAMETER
   ioctl(stRxThreadInfo.astChannels[0].iFd, ANT_IOCTL_RESET, ANT_IOCTL_RESET_PARAMETER);
#else
   ioctl(stRxThreadInfo.astChannels[0].iFd, ANT_IOCTL_RESET);
#endif  // ANT_IOCTL_RESET_PARAMETER

   ant_disable();

   if (ant_enable()) { /* failed */
      if (g_fnStateCallback)
         g_fnStateCallback(RADIO_STATUS_DISABLED);
   } else { /* success */
      if (g_fnStateCallback)
         g_fnStateCallback(RADIO_STATUS_RESET);
      result_status = ANT_STATUS_SUCCESS;
   }
   stRxThreadInfo.ucChipResetting = 0;

   ANT_DEBUG_V("releasing stEnabledStatusLock in %s", __FUNCTION__);
   pthread_mutex_unlock(&stEnabledStatusLock);
   ANT_DEBUG_V("released stEnabledStatusLock in %s", __FUNCTION__);
out:
#endif // ANT_IOCTL_RESET

   ANT_FUNC_END();
   return result_status;
}

////////////////////////////////////////////////////////////////////
//  ant_disable_radio
//
//  Powers off the ANT part and closes the transport to the chip.
//
//  Parameters:
//      -
//
//  Returns:
//      Success:
//          ANT_STATUS_SUCCESS
//      Failures:
//          ANT_STATUS_FAILED if failed to get mutex
//
//  Psuedocode:
/*
LOCK enable_LOCK
    State callback: STATE = DISABLING
    ant disable
    State callback: STATE = Current state
    RESULT = SUCCESS
UNLOCK
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_disable_radio(void)
{
   int iLockResult;
   ANTStatus ret = ANT_STATUS_FAILED;
   ANT_FUNC_START();

   ANT_DEBUG_V("getting stEnabledStatusLock in %s", __FUNCTION__);
   iLockResult = pthread_mutex_lock(&stEnabledStatusLock);
   if(iLockResult) {
      ANT_ERROR("disable failed to get state lock: %s", strerror(iLockResult));
      goto out;
   }
   ANT_DEBUG_V("got stEnabledStatusLock in %s", __FUNCTION__);

   if (g_fnStateCallback) {
      g_fnStateCallback(RADIO_STATUS_DISABLING);
   }

   ant_disable();

   if (g_fnStateCallback) {
      g_fnStateCallback(ant_radio_enabled_status());
   }

   ret = ANT_STATUS_SUCCESS;

   ANT_DEBUG_V("releasing stEnabledStatusLock in %s", __FUNCTION__);
   pthread_mutex_unlock(&stEnabledStatusLock);
   ANT_DEBUG_V("released stEnabledStatusLock in %s", __FUNCTION__);

out:
   ANT_FUNC_END();
   return ret;
}

////////////////////////////////////////////////////////////////////
//  ant_radio_enabled_status
//
//  Gets the current chip/transport state; either disabled, disabling,
//  enabling, enabled, or resetting.  Determines this on the fly by checking
//  if Rx thread is running and how many of the paths for the ANT chip have
//  open VFS files.
//
//  Parameters:
//      -
//
//  Returns:
//      The current radio status (ANTRadioEnabledStatus)
//
//  Psuedocode:
/*
IF Thread Resetting Flag is set
    RESULT = Resetting
ELSE
    COUNT the number of open files
    IF Thread Run Flag is Not Set
        IF there are open files OR Rx thread exists
            RESULT = Disabling
        ELSE
            RESULT = Disabled
        ENDIF
    ELSE
        IF All files are open (all paths) AND Rx thread exists
            RESULT = ENABLED
        ELSE IF there are open files (Not 0 open files) AND Rx thread exists
            RESULT = UNKNOWN
        ELSE (0 open files or Rx thread does not exist [while Thread Run set])
            RESULT = ENABLING
        ENDIF
    ENDIF
ENDIF
*/
////////////////////////////////////////////////////////////////////
ANTRadioEnabledStatus ant_radio_enabled_status(void)
{
   ant_channel_type eChannel;
   int iOpenFiles = 0;
   int iOpenThread;
   ANTRadioEnabledStatus uiRet = RADIO_STATUS_UNKNOWN;
   ANT_FUNC_START();

   if (stRxThreadInfo.ucChipResetting) {
      uiRet = RADIO_STATUS_RESETTING;
      goto out;
   }

   for (eChannel = 0; eChannel < NUM_ANT_CHANNELS; eChannel++) {
      if (stRxThreadInfo.astChannels[eChannel].iFd != -1) {
         iOpenFiles++;
      }
   }

   iOpenThread = (stRxThreadInfo.stRxThread) ? 1 : 0;

   if (!stRxThreadInfo.ucRunThread) {
      if (iOpenFiles || iOpenThread) {
         uiRet = RADIO_STATUS_DISABLING;
      } else {
         uiRet = RADIO_STATUS_DISABLED;
      }
   } else {
      if ((iOpenFiles == NUM_ANT_CHANNELS) && iOpenThread) {
         uiRet = RADIO_STATUS_ENABLED;
      } else if (!iOpenFiles && iOpenThread) {
         uiRet = RADIO_STATUS_UNKNOWN;
      } else {
         uiRet = RADIO_STATUS_ENABLING;
      }
   }

out:
   ANT_DEBUG_D("get radio enabled status returned %d", uiRet);

   ANT_FUNC_END();
   return uiRet;
}

////////////////////////////////////////////////////////////////////
//  set_ant_rx_callback
//
//  Sets which function to call when an ANT message is received.
//
//  Parameters:
//      rx_callback_func   the ANTNativeANTEventCb function to be used for
//                         received messages (from all transport paths).
//
//  Returns:
//          ANT_STATUS_SUCCESS
//
//  Psuedocode:
/*
FOR each transport path
    Path Rx Callback = rx_callback_func
ENDFOR
*/
////////////////////////////////////////////////////////////////////
ANTStatus set_ant_rx_callback(ANTNativeANTEventCb rx_callback_func)
{
   ANTStatus status = ANT_STATUS_SUCCESS;
   ANT_FUNC_START();

#ifdef ANT_DEVICE_NAME // Single transport path
   stRxThreadInfo.astChannels[SINGLE_CHANNEL].fnRxCallback = rx_callback_func;
   ANT_DEBUG_V("Using the Single transport path");
#else // Separate data/command paths
   stRxThreadInfo.astChannels[COMMAND_CHANNEL].fnRxCallback = rx_callback_func;
   stRxThreadInfo.astChannels[DATA_CHANNEL].fnRxCallback = rx_callback_func;
#endif // Separate data/command paths

   ANT_FUNC_END();
   return status;
}

////////////////////////////////////////////////////////////////////
//  set_ant_state_callback
//
//  Sets which function to call when an ANT state change occurs.
//
//  Parameters:
//      state_callback_func   the ANTNativeANTStateCb function to be used
//                            for received state changes.
//
//  Returns:
//          ANT_STATUS_SUCCESS
//
//  Psuedocode:
/*
    State Callback = state_callback_func
*/
////////////////////////////////////////////////////////////////////
ANTStatus set_ant_state_callback(ANTNativeANTStateCb state_callback_func)
{
   ANTStatus status = ANT_STATUS_SUCCESS;
   ANT_FUNC_START();

   g_fnStateCallback = state_callback_func;

   ANT_FUNC_END();
   return status;
}

////////////////////////////////////////////////////////////////////
//  ant_tx_message_flowcontrol_wait
//
//  Sends an ANT message to the chip and waits for a CTS signal
//
//  Parameters:
//      eTxPath          device to transmit message on
//      eFlowMessagePath device that receives CTS
//      ucMessageLength  the length of the message
//      pucMesg          pointer to the message data
//
//  Returns:
//      Success:
//          ANT_STATUS_SUCCESS
//      Failure:
//          ANT_STATUS_NOT_ENABLED
//
//  Psuedocode:
/*
        LOCK flow control
        IF Lock failed
            RESULT = FAILED
        ELSE
            SET flowMessagePath Flow Control response as FLOW_STOP
            WRITE txBuffer to txPath (only length of packet part)
            IF Wrote less then 0 bytes
                Log error
                RESULT = FAILED
            ELSE IF Didn't write 'length of packet' bytes
                Log error
                RESULT = FAILED
            ELSE
                IF flowMessagePath Flow Control response is not FLOW_GO
                    WAIT until flowMessagePath Flow Control response is FLOW_GO, UNTIL FLOW_GO Wait Timeout seconds (10) from Now
                    IF error Waiting
                        IF error is Timeout
                            RESULT = HARDWARE ERROR
                        ELSE
                            RESULT = FAILED
                        ENDIF
                    ELSE
                        RESULT = SUCCESS
                    ENDIF
                ELSE
                    RESULT = SUCCESS;
                ENDIF
            ENDIF
            UNLOCK flow control
        ENDIF
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_tx_message_flowcontrol_wait(ant_channel_type eTxPath, ant_channel_type eFlowMessagePath, ANT_U8 ucMessageLength, ANT_U8 *pucTxMessage)
{
   int iMutexResult;
   int iResult;
   struct timespec stTimeout;
   int iCondWaitResult;
   ANTStatus status = ANT_STATUS_FAILED;
   ANT_FUNC_START();

   ANT_DEBUG_V("getting stFlowControlLock in %s", __FUNCTION__);
   iMutexResult = pthread_mutex_lock(&stFlowControlLock);
   if (iMutexResult) {
      ANT_ERROR("failed to lock flow control mutex during tx: %s", strerror(iMutexResult));
      goto out;
   }
   ANT_DEBUG_V("got stFlowControlLock in %s", __FUNCTION__);

   stRxThreadInfo.astChannels[eFlowMessagePath].ucFlowControlResp = ANT_FLOW_STOP;

#ifdef ANT_FLOW_RESEND
   // Store Tx message so can resend it from Rx thread
   stRxThreadInfo.astChannels[eFlowMessagePath].ucResendMessageLength = ucMessageLength;
   stRxThreadInfo.astChannels[eFlowMessagePath].pucResendMessage = pucTxMessage;
#endif // ANT_FLOW_RESEND

   iResult = write(stRxThreadInfo.astChannels[eTxPath].iFd, pucTxMessage, ucMessageLength);
   if (iResult < 0) {
      ANT_ERROR("failed to write data message to device: %s", strerror(errno));
   } else if (iResult != ucMessageLength) {
      ANT_ERROR("bytes written and message size don't match up");
   } else {
      stTimeout.tv_sec = time(0) + ANT_FLOW_GO_WAIT_TIMEOUT_SEC;
      stTimeout.tv_nsec = 0;

      while (stRxThreadInfo.astChannels[eFlowMessagePath].ucFlowControlResp != ANT_FLOW_GO) {
         iCondWaitResult = pthread_cond_timedwait(&stFlowControlCond, &stFlowControlLock, &stTimeout);
         if (iCondWaitResult) {
            ANT_ERROR("failed to wait for flow control response: %s", strerror(iCondWaitResult));

            if (iCondWaitResult == ETIMEDOUT) {
               status = ANT_STATUS_HARDWARE_ERR;

#ifdef ANT_FLOW_RESEND
               // Clear Tx message so will stop resending it from Rx thread
               stRxThreadInfo.astChannels[eFlowMessagePath].ucResendMessageLength = 0;
               stRxThreadInfo.astChannels[eFlowMessagePath].pucResendMessage = NULL;
#endif // ANT_FLOW_RESEND
            }
            goto wait_error;
         }
      }

      status = ANT_STATUS_SUCCESS;
   }

wait_error:
   ANT_DEBUG_V("releasing stFlowControlLock in %s", __FUNCTION__);
   pthread_mutex_unlock(&stFlowControlLock);
   ANT_DEBUG_V("released stFlowControlLock in %s", __FUNCTION__);

out:
   ANT_FUNC_END();
   return status;
}

////////////////////////////////////////////////////////////////////
//  ant_tx_message_flowcontrol_none
//
//  Sends an ANT message to the chip without waiting for flow control
//
//  Parameters:
//      eTxPath         device to transmit on
//      ucMessageLength the length of the message
//      pucMesg         pointer to the message data
//
//  Returns:
//      Success:
//          ANT_STATUS_SUCCESS
//      Failure:
//          ANT_STATUS_NOT_ENABLED
//
//  Psuedocode:
/*
        WRITE txBuffer to Tx Path (only length of packet part)
        IF Wrote less then 0 bytes
            Log error
            RESULT = FAILED
        ELSE IF Didn't write 'length of packet' bytes
            Log error
            RESULT = FAILED
        ELSE
            RESULT = SUCCESS
        ENDIF
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_tx_message_flowcontrol_none(ant_channel_type eTxPath, ANT_U8 ucMessageLength, ANT_U8 *pucTxMessage)
{
   int iResult;
   ANTStatus status = ANT_STATUS_FAILED;
   ANT_FUNC_START();

   iResult = write(stRxThreadInfo.astChannels[eTxPath].iFd, pucTxMessage, ucMessageLength);
   if (iResult < 0) {
      ANT_ERROR("failed to write message to device: %s", strerror(errno));
   }  else if (iResult != ucMessageLength) {
      ANT_ERROR("bytes written and message size don't match up");
   } else {
      status = ANT_STATUS_SUCCESS;
   }

   ANT_FUNC_END();
   return status;
}

////////////////////////////////////////////////////////////////////
// ant_set_package_title
// 
// Set the package type,data or command

ANT_U8* ant_set_package_title(ANT_U8 *pucMesg, ANT_U8 ucLen ,int dataType, ANT_U8 *pOutMesg)
{
 	 ANT_FUNC_START();
     ANT_U8 pSendBuf[ucLen +1];
	 ANTStatus status = ANT_STATUS_FAILED;
	 // ANT_U8 *pBuf = (ANT_U8 *)malloc(ucLen+1);
	 ANT_U8 txMessageLength = ucLen + ANT_HCI_HEADER_SIZE + 1;
	 memset(pSendBuf , 0 , sizeof(pSendBuf));


	 ANT_DEBUG_V("Set Data or Command type\n");
     //set the package header
     if(1 == dataType)
     {
        pSendBuf[0] = 0x0C;		
     }
     else
     {
        pSendBuf[0] = 0x0E;		 
     }

	 ANT_DEBUG_V("Set ANT Tx Value\n");
	 memcpy(pSendBuf + 1, pucMesg, ucLen + 1);	 


	 ANT_SERIAL(pSendBuf, txMessageLength  , 'T');

	 status = ant_tx_message_flowcontrol_none(SINGLE_CHANNEL, txMessageLength + 1, pSendBuf);

	 //memcpy(pOutMesg, pSendBuf, ucLen + 2);


	 ANT_FUNC_END();

     return status;
}

////////////////////////////////////////////////////////////////////
//  ant_tx_message
//
//  Frames ANT data and decides which flow control method to use for sending the
//  ANT message to the chip
//
//  Parameters:
//      ucLen   the length of the message
//      pucMesg pointer to the message data
//
//  Returns:
//      Success:
//          ANT_STATUS_SUCCESS
//      Failure:
//          ANT_STATUS_NOT_ENABLED
//
//  Psuedocode:
/*
IF not enabled
    RESULT = BT NOT INITIALIZED
ELSE
    Create txBuffer, MAX HCI Message Size large
    PUT ucLen in txBuffer AT ANT HCI Size Offset (0)
    COPY pucMesg to txBuffer AT ANT HCI Header Size (1)     <- ? Not at offset?
    LOG txBuffer as a serial Tx (only length of packet part)
    IF is a data message
        Tx message on Data Path with FLOW_GO/FLOW_STOP flow control (ant_tx_message_flowcontrol_go_stop())
    ELSE
        Tx message on Command Path with no flow control (ant_tx_message_flowcontrol_none())
    ENDIF
ENDIF
*/
////////////////////////////////////////////////////////////////////
ANTStatus ant_tx_message(ANT_U8 ucLen, ANT_U8 *pucMesg)
{
   ANTStatus status = ANT_STATUS_FAILED;
   // TODO ANT_HCI_MAX_MSG_SIZE is transport (driver) dependent.
   ANT_U8 txBuffer[ANT_HCI_MAX_MSG_SIZE];
   // TODO Message length can be greater than ANT_U8 can hold.
   // Not changed as ANT_SERIAL takes length as ANT_U8.
   ANT_U8 txMessageLength = ucLen + ANT_HCI_HEADER_SIZE;
   ANT_FUNC_START();

   if (ant_radio_enabled_status() != RADIO_STATUS_ENABLED) {
      status = ANT_STATUS_FAILED_BT_NOT_INITIALIZED;
      goto out;
   }

#if ANT_HCI_OPCODE_SIZE == 1
   txBuffer[ANT_HCI_OPCODE_OFFSET] = ANT_HCI_OPCODE_TX;
#elif ANT_HCI_OPCODE_SIZE > 1
#error "Specified ANT_HCI_OPCODE_SIZE not currently supported"
#endif

#if ANT_HCI_SIZE_SIZE == 1
   txBuffer[ANT_HCI_SIZE_OFFSET] = ucLen;
#elif ANT_HCI_SIZE_SIZE == 2
   ANT_UTILS_StoreLE16(txBuffer + ANT_HCI_SIZE_OFFSET, (ANT_U16)ucLen);
#else
#error "Specified ANT_HCI_SIZE_SIZE not currently supported"
#endif

   memcpy(txBuffer + ANT_HCI_HEADER_SIZE, pucMesg, ucLen);

   // ANT_SERIAL(txBuffer, txMessageLength , 'T');


   

// #ifdef ANT_DEVICE_NAME // Single transport path
// status = ant_tx_message_flowcontrol_wait(SINGLE_CHANNEL, SINGLE_CHANNEL, txMessageLength, txBuffer);

/*
#else // Separate data/command paths
   switch (txBuffer[ANT_HCI_DATA_OFFSET + ANT_MSG_ID_OFFSET]) {
   case MESG_BROADCAST_DATA_ID:
   case MESG_ACKNOWLEDGED_DATA_ID:
   case MESG_BURST_DATA_ID:
   case MESG_EXT_BROADCAST_DATA_ID:
   case MESG_EXT_ACKNOWLEDGED_DATA_ID:
   case MESG_EXT_BURST_DATA_ID:
   case MESG_ADV_BURST_DATA_ID:
      status = ant_tx_message_flowcontrol_wait(DATA_CHANNEL, COMMAND_CHANNEL, txMessageLength, txBuffer);
      break;
   default:
      status = ant_tx_message_flowcontrol_none(COMMAND_CHANNEL, txMessageLength, txBuffer);
   }
#endif // Separate data/command paths

*/
   //ANT_U8 *pBuf = (ANT_U8 *)malloc(ucLen+1);
   switch (txBuffer[ANT_HCI_DATA_OFFSET + ANT_MSG_ID_OFFSET]) {
   case MESG_BROADCAST_DATA_ID:
   case MESG_ACKNOWLEDGED_DATA_ID:
   case MESG_BURST_DATA_ID:
   case MESG_EXT_BROADCAST_DATA_ID:
   case MESG_EXT_ACKNOWLEDGED_DATA_ID:
   case MESG_EXT_BURST_DATA_ID:
   case MESG_ADV_BURST_DATA_ID:
      //status = ant_tx_message_flowcontrol_wait(DATA_CHANNEL, COMMAND_CHANNEL, txMessageLength, txBuffer);
	   ANT_DEBUG_V("The ANT Data Package\n");
       status = ant_set_package_title(txBuffer ,ucLen , 0 ,txBuffer);
	   ANT_DEBUG_V("Call ant_set_package_title Finish\n");
      break;
   default:
      //status = ant_tx_message_flowcontrol_none(COMMAND_CHANNEL, txMessageLength, txBuffer);
	   ANT_DEBUG_V("The ANT Command Package\n");
       status = ant_set_package_title(txBuffer ,ucLen , 1 , txBuffer);
	   ANT_DEBUG_V("Call ant_set_package_title Finish\n");
   }

    //ANT_SERIAL(pBuf, (txMessageLength + 1) , 'T');
   //ANT_DEBUG_V("The send Data Package is %s\n", *pBuf);


       //status = ant_tx_message_flowcontrol_wait(SINGLE_CHANNEL, SINGLE_CHANNEL, txMessageLength + 1, pBuf);
     //status = ant_tx_message_flowcontrol_none(SINGLE_CHANNEL, txMessageLength + 1, pBuf);

     //free(pBuf);
	 //pBuf = NULL;
out:
   ANT_FUNC_END();

   return status;
}

//----------------- TODO Move these somewhere for multi transport path / dedicated channel support:

static void ant_channel_init(ant_channel_info_t *pstChnlInfo, const char *pcCharDevName)
{
   ANT_FUNC_START();

   // TODO Don't need to store, only accessed when trying to open:
   // Is however useful for logs.
   pstChnlInfo->pcDevicePath = pcCharDevName;

   // This is the only piece of info that needs to be stored per channel
   pstChnlInfo->iFd = -1;

   // TODO Only 1 of these (not per-channel) is actually ever used:
   pstChnlInfo->fnRxCallback = NULL;
   pstChnlInfo->ucFlowControlResp = ANT_FLOW_GO;
#ifdef ANT_FLOW_RESEND
   pstChnlInfo->ucResendMessageLength = 0;
   pstChnlInfo->pucResendMessage = NULL;
#endif // ANT_FLOW_RESEND
   // TODO Only used when Flow Control message received, so must only be Command path Rx thread
   pstChnlInfo->pstFlowControlCond = &stFlowControlCond;
   pstChnlInfo->pstFlowControlLock = &stFlowControlLock;

   ANT_FUNC_END();
}

static void ant_disable_channel(ant_channel_info_t *pstChnlInfo)
{
   ANT_FUNC_START();

   if (!pstChnlInfo) {
      ANT_ERROR("null channel info passed to channel disable function");
      goto out;
   }

   if (pstChnlInfo->iFd != -1) {
      if (close(pstChnlInfo->iFd) < 0) {
         ANT_ERROR("failed to close channel %s(%#x): %s", pstChnlInfo->pcDevicePath, pstChnlInfo->iFd, strerror(errno));
      }

      pstChnlInfo->iFd = -1; //TODO can this overwrite a still valid fd?
   } else {
      ANT_DEBUG_D("%s file is already closed", pstChnlInfo->pcDevicePath);
   }

out:
   ANT_FUNC_END();
}

static int ant_enable_channel(ant_channel_info_t *pstChnlInfo)
{
   int iRet = -1;
   ANT_FUNC_START();
   if (!pstChnlInfo) {
      ANT_ERROR("null channel info passed to channel enable function");
      errno = EINVAL;
      goto out;
   }
   if (pstChnlInfo->iFd == -1) {
      pstChnlInfo->iFd = open(pstChnlInfo->pcDevicePath, O_RDWR);
      if (pstChnlInfo->iFd < 0) {
         ANT_ERROR("failed to open dev %s: %s", pstChnlInfo->pcDevicePath, strerror(errno));
         goto out;
      }
   } else {
      ANT_DEBUG_D("%s is already enabled", pstChnlInfo->pcDevicePath);
   }
   iRet = 0;
out:
   ANT_FUNC_END();
   return iRet;
}

//----------------------------------------------------------------------- This is antradio_power.h:

int ant_enable(void)
{
   int iRet = -1;
   ant_channel_type eChannel;
   ANT_FUNC_START();

   // Reset the shutdown signal.
   uint64_t counter;
   ssize_t result = read(stRxThreadInfo.iRxShutdownEventFd, &counter, sizeof(counter));
   // EAGAIN result indicates that the counter was already 0 in non-blocking mode.
   if(result < 0 && errno != EAGAIN)
   {
      ANT_ERROR("Could not clear shutdown signal in enable. Reason: %s", strerror(errno));
      goto out;
   }

   stRxThreadInfo.ucRunThread = 1;

   for (eChannel = 0; eChannel < NUM_ANT_CHANNELS; eChannel++) {
      if (ant_enable_channel(&stRxThreadInfo.astChannels[eChannel]) < 0) {
         ANT_ERROR("failed to enable channel %s: %s",
                         stRxThreadInfo.astChannels[eChannel].pcDevicePath,
                         strerror(errno));
         goto out;
      }
   }

   if (stRxThreadInfo.stRxThread == 0) {
      if (pthread_create(&stRxThreadInfo.stRxThread, NULL, fnRxThread, &stRxThreadInfo) < 0) {
         ANT_ERROR("failed to start rx thread: %s", strerror(errno));
         goto out;
      }
   } else {
      ANT_DEBUG_D("rx thread is already running");
   }

   if (!stRxThreadInfo.ucRunThread) {
      ANT_ERROR("rx thread crashed during init");
      goto out;
   }

   iRet = 0;

out:
   ANT_FUNC_END();
   return iRet;
}

int ant_disable(void)
{
   int iRet = -1;
   ant_channel_type eChannel;
   ANT_FUNC_START();

   stRxThreadInfo.ucRunThread = 0;

   if (stRxThreadInfo.stRxThread != 0) {
      ANT_DEBUG_I("Sending shutdown signal to rx thread.");
      if(write(stRxThreadInfo.iRxShutdownEventFd, &EVENT_FD_PLUS_ONE, sizeof(EVENT_FD_PLUS_ONE)) < 0)
      {
         ANT_ERROR("failed to signal rx thread with eventfd. Reason: %s", strerror(errno));
         goto out;
      }
      ANT_DEBUG_I("Waiting for rx thread to finish.");
      if (pthread_join(stRxThreadInfo.stRxThread, NULL) < 0) {
         ANT_ERROR("failed to join rx thread: %s", strerror(errno));
         goto out;
      }
   } else {
      ANT_DEBUG_D("rx thread is not running");
   }

   for (eChannel = 0; eChannel < NUM_ANT_CHANNELS; eChannel++) {
      ant_disable_channel(&stRxThreadInfo.astChannels[eChannel]);
   }

   iRet = 0;

out:
   stRxThreadInfo.stRxThread = 0;
   ANT_FUNC_END();
   return iRet;
}

//---------------------------------------------------------

const char *ant_get_lib_version()
{
   return "libantradio.so: "ANT_CHIP_NAME". Version "
      LIBANT_STACK_MAJOR"."LIBANT_STACK_MINOR"."LIBANT_STACK_INCRE;
}
