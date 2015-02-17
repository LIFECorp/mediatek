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
*   FILE NAME:      ant_rx_chardev.c
*
*   BRIEF:
*      This file implements the receive thread function which will loop reading
*      ANT messages until told to exit.
*
*
\******************************************************************************/

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h> /* for uint64_t */

#include "ant_types.h"
#include "antradio_power.h"
#include "ant_rx_chardev.h"
#include "ant_hci_defines.h"
#include "ant_log.h"
#include "ant_native.h"  // ANT_HCI_MAX_MSG_SIZE, ANT_MSG_ID_OFFSET, ANT_MSG_DATA_OFFSET,
                         // ant_radio_enabled_status()

extern ANTStatus ant_tx_message_flowcontrol_none(ant_channel_type eTxPath, ANT_U8 ucMessageLength, ANT_U8 *pucTxMessage);

#undef LOG_TAG
#define LOG_TAG "antradio_rx"

#define ANT_POLL_TIMEOUT         ((int)30000)
#define PACKAGE_TYPE              0
#define DATA_LOCATION             1
#define DATA_OTHER                2
#define COMMAND_HEDAER            12
#define HEADER_TYPE               14

static ANT_U8 aucRxBuffer[NUM_ANT_CHANNELS][ANT_HCI_MAX_MSG_SIZE];

char rBuffer[ANT_HCI_MAX_MSG_SIZE];
ANT_U8 auRxBuffer[ANT_HCI_MAX_MSG_SIZE];

char *pTempBuff;

static pthread_mutex_t stReadStatusLock = PTHREAD_MUTEX_INITIALIZER;

#ifdef ANT_DEVICE_NAME // Single transport path
	static int iRxBufferLength[NUM_ANT_CHANNELS] = {0};
#else
	static int iRxBufferLength[NUM_ANT_CHANNELS] = {0, 0};
#endif // 

// Defines for use with the poll() call
#define EVENT_DATA_AVAILABLE (POLLIN|POLLRDNORM)
#define EVENT_DISABLE (POLLHUP)
#define EVENT_HARD_RESET (POLLERR|POLLPRI|POLLRDHUP)

#define EVENTS_TO_LISTEN_FOR (EVENT_DATA_AVAILABLE|EVENT_DISABLE|EVENT_HARD_RESET)

// Plus one is for the eventfd shutdown signal.
#define NUM_POLL_FDS (NUM_ANT_CHANNELS + 1)
#define EVENTFD_IDX NUM_ANT_CHANNELS

void doReset(ant_rx_thread_info_t *stRxThreadInfo);
int readChannelMsg(ant_channel_type eChannel, ant_channel_info_t *pstChnlInfo);

/*
 * Function to check that all given flags are set in a particular value.
 * Designed for use with the revents field of pollfds filled out by poll().
 *
 * Parameters:
 *    - value: The value that will be checked to contain all flags.
 *    - flags: Bitwise-or of the flags that value should be checked for.
 *
 * Returns:
 *    - true IFF all the bits that are set in 'flags' are also set in 'value'
 */
ANT_BOOL areAllFlagsSet(short value, short flags)
{
   value &= flags;
   return (value == flags);
}

/*
 * This thread waits for ANT messages from a VFS file.
 */
void *fnRxThread(void *ant_rx_thread_info)
{
   int iMutexLockResult;
   int iPollRet;
   ant_rx_thread_info_t *stRxThreadInfo;
   struct pollfd astPollFd[NUM_POLL_FDS];
   ant_channel_type eChannel;
   ANT_FUNC_START();

   stRxThreadInfo = (ant_rx_thread_info_t *)ant_rx_thread_info;
   for (eChannel = 0; eChannel < NUM_ANT_CHANNELS; eChannel++) {
      astPollFd[eChannel].fd = stRxThreadInfo->astChannels[eChannel].iFd;
      astPollFd[eChannel].events = EVENTS_TO_LISTEN_FOR;
	  ANT_DEBUG_V("Rx number is %d", NUM_ANT_CHANNELS);
   }
   // Fill out poll request for the shutdown signaller.
   astPollFd[EVENTFD_IDX].fd = stRxThreadInfo->iRxShutdownEventFd;
   astPollFd[EVENTFD_IDX].events = POLL_IN;

   /* continue running as long as not terminated */
   while (stRxThreadInfo->ucRunThread) {
      /* Wait for data available on any file (transport path) */
      iPollRet = poll(astPollFd, NUM_POLL_FDS, ANT_POLL_TIMEOUT);
      if (!iPollRet) {
         ANT_DEBUG_V("poll timed out, checking exit cond");
      } else if (iPollRet < 0) {
         ANT_ERROR("unhandled error: %s, attempting recovery.", strerror(errno));
         doReset(stRxThreadInfo);
         goto out;
      } else {
         for (eChannel = 0; eChannel < NUM_ANT_CHANNELS; eChannel++) {
            if (areAllFlagsSet(astPollFd[eChannel].revents, EVENT_HARD_RESET)) {
               ANT_ERROR("Hard reset indicated by %s. Attempting recovery.",
                            stRxThreadInfo->astChannels[eChannel].pcDevicePath);
               doReset(stRxThreadInfo);
               goto out;
            } else if (areAllFlagsSet(astPollFd[eChannel].revents, EVENT_DISABLE)) {
               /* chip reported it was disabled, either unexpectedly or due to us closing the file */
               ANT_DEBUG_D(
                     "poll hang-up from %s. exiting rx thread", stRxThreadInfo->astChannels[eChannel].pcDevicePath);

               // set flag to exit out of Rx Loop
               stRxThreadInfo->ucRunThread = 0;

            } else if (areAllFlagsSet(astPollFd[eChannel].revents, EVENT_DATA_AVAILABLE)) {
               ANT_DEBUG_D("data on %s. reading it",
                            stRxThreadInfo->astChannels[eChannel].pcDevicePath);

               if (readChannelMsg(eChannel, &stRxThreadInfo->astChannels[eChannel]) < 0) {
                  // set flag to exit out of Rx Loop
                  stRxThreadInfo->ucRunThread = 0;
               }
            } else if (areAllFlagsSet(astPollFd[eChannel].revents, POLLNVAL)) {
               ANT_ERROR("poll was called on invalid file descriptor %s. Attempting recovery.",
                     stRxThreadInfo->astChannels[eChannel].pcDevicePath);
               doReset(stRxThreadInfo);
               goto out;
            } else if (areAllFlagsSet(astPollFd[eChannel].revents, POLLERR)) {
               ANT_ERROR("Unknown error from %s. Attempting recovery.",
                     stRxThreadInfo->astChannels[eChannel].pcDevicePath);
               doReset(stRxThreadInfo);
               goto out;
            } else if (astPollFd[eChannel].revents) {
               ANT_DEBUG_W("unhandled poll result %#x from %s",
                            astPollFd[eChannel].revents,
                            stRxThreadInfo->astChannels[eChannel].pcDevicePath);
            }
         }
         // Now check for shutdown signal
         if(areAllFlagsSet(astPollFd[EVENTFD_IDX].revents, POLLIN))
         {
            ANT_DEBUG_I("rx thread caught shutdown signal.");
            // reset the counter by reading.
            uint64_t counter;
            read(stRxThreadInfo->iRxShutdownEventFd, &counter, sizeof(counter));
            // don't care if read error, going to close the thread anyways.
            stRxThreadInfo->ucRunThread = 0;
         } else if (astPollFd[EVENTFD_IDX].revents != 0) {
            ANT_ERROR("Shutdown event descriptor had unexpected event: %#x. exiting rx thread.",
                  astPollFd[EVENTFD_IDX].revents);
            stRxThreadInfo->ucRunThread = 0;
         }
      }
   }

   /* disable ANT radio if not already disabling */
   // Try to get stEnabledStatusLock.
   // if you get it then no one is enabling or disabling
   // if you can't get it assume something made you exit
   ANT_DEBUG_V("try getting stEnabledStatusLock in %s", __FUNCTION__);
   iMutexLockResult = pthread_mutex_trylock(stRxThreadInfo->pstEnabledStatusLock);
   if (!iMutexLockResult) {
      ANT_DEBUG_V("got stEnabledStatusLock in %s", __FUNCTION__);
      ANT_WARN("rx thread has unexpectedly crashed, cleaning up");

      // spoof our handle as closed so we don't try to join ourselves in disable
      stRxThreadInfo->stRxThread = 0;

      if (g_fnStateCallback) {
         g_fnStateCallback(RADIO_STATUS_DISABLING);
      }

      ant_disable();

      if (g_fnStateCallback) {
         g_fnStateCallback(ant_radio_enabled_status());
      }

      ANT_DEBUG_V("releasing stEnabledStatusLock in %s", __FUNCTION__);
      pthread_mutex_unlock(stRxThreadInfo->pstEnabledStatusLock);
      ANT_DEBUG_V("released stEnabledStatusLock in %s", __FUNCTION__);
   } else if (iMutexLockResult != EBUSY) {
      ANT_ERROR("rx thread closing code, trylock on state lock failed: %s",
            strerror(iMutexLockResult));
   } else {
      ANT_DEBUG_V("stEnabledStatusLock busy");
   }

   out:
   ANT_FUNC_END();
#ifdef ANDROID
   return NULL;
#endif
}

void doReset(ant_rx_thread_info_t *stRxThreadInfo)
{
   int iMutexLockResult;

   ANT_FUNC_START();
   /* Chip was reset or other error, only way to recover is to
    * close and open ANT chardev */
   stRxThreadInfo->ucChipResetting = 1;

   if (g_fnStateCallback) {
      g_fnStateCallback(RADIO_STATUS_RESETTING);
   }

   stRxThreadInfo->ucRunThread = 0;

   ANT_DEBUG_V("getting stEnabledStatusLock in %s", __FUNCTION__);
   iMutexLockResult = pthread_mutex_lock(stRxThreadInfo->pstEnabledStatusLock);
   if (iMutexLockResult < 0) {
      ANT_ERROR("chip was reset, getting state mutex failed: %s",
            strerror(iMutexLockResult));
      stRxThreadInfo->stRxThread = 0;
   } else {
      ANT_DEBUG_V("got stEnabledStatusLock in %s", __FUNCTION__);

      stRxThreadInfo->stRxThread = 0; /* spoof our handle as closed so we don't
                                       * try to join ourselves in disable */

      ant_disable();

      if (ant_enable()) { /* failed */
         if (g_fnStateCallback) {
            g_fnStateCallback(RADIO_STATUS_DISABLED);
         }
      } else { /* success */
         if (g_fnStateCallback) {
            g_fnStateCallback(RADIO_STATUS_RESET);
         }
      }

      ANT_DEBUG_V("releasing stEnabledStatusLock in %s", __FUNCTION__);
      pthread_mutex_unlock(stRxThreadInfo->pstEnabledStatusLock);
      ANT_DEBUG_V("released stEnabledStatusLock in %s", __FUNCTION__);
   }

   stRxThreadInfo->ucChipResetting = 0;

   ANT_FUNC_END();
}

////////////////////////////////////////////////////////////////////
//  setFlowControl
//
//  Sets the flow control "flag" to the value provided and signals the transmit
//  thread to check the value.
//
//  Parameters:
//      pstChnlInfo   the details of the channel being updated
//      ucFlowSetting the value to use
//
//  Returns:
//      Success:
//          0
//      Failure:
//          -1
////////////////////////////////////////////////////////////////////
int setFlowControl(ant_channel_info_t *pstChnlInfo, ANT_U8 ucFlowSetting)
{
   int iRet = -1;
   int iMutexResult;
   ANT_FUNC_START();

   ANT_DEBUG_V("getting stFlowControlLock in %s", __FUNCTION__);
   iMutexResult = pthread_mutex_lock(pstChnlInfo->pstFlowControlLock);
   if (iMutexResult) {
      ANT_ERROR("failed to lock flow control mutex during response: %s", strerror(iMutexResult));
   } else {
      ANT_DEBUG_V("got stFlowControlLock in %s", __FUNCTION__);

      pstChnlInfo->ucFlowControlResp = ucFlowSetting;

      ANT_DEBUG_V("releasing stFlowControlLock in %s", __FUNCTION__);
      pthread_mutex_unlock(pstChnlInfo->pstFlowControlLock);
      ANT_DEBUG_V("released stFlowControlLock in %s", __FUNCTION__);

      pthread_cond_signal(pstChnlInfo->pstFlowControlCond);

      iRet = 0;
   }

   ANT_FUNC_END();
   return iRet;
}

int readChannelMsg(ant_channel_type eChannel, ant_channel_info_t *pstChnlInfo)
{
   int iRet = -1;
   int iRxLenRead;
   int iCurrentHciPacketOffset;
   int iHciDataSize;
   int iLockResult;
   ANT_FUNC_START();

   iLockResult = pthread_mutex_lock(&stReadStatusLock);
   if(iLockResult) {
      ANT_ERROR("enable failed to get state lock: %s", strerror(iLockResult));
      goto out;
   }
   ANT_DEBUG_V("got stEnabledStatusLock in %s", __FUNCTION__);

   // Keep trying to read while there is an error, and that error is EAGAIN
   //while (((iRxLenRead = read(pstChnlInfo->iFd, &aucRxBuffer[eChannel][iRxBufferLength[eChannel]], (sizeof(aucRxBuffer[eChannel]) - iRxBufferLength[eChannel]))) < 0)
   //                && errno == EAGAIN)

   while (((iRxLenRead = read(pstChnlInfo->iFd, &rBuffer[0], ANT_HCI_MAX_MSG_SIZE)) < 0)
                  && errno == EAGAIN)
      ;

   ANT_DEBUG_V("Receive Package Length is %d \n", iRxLenRead);

   /*
   int k = 0;
   while(k < iRxLenRead)
   {
        ANT_DEBUG_V("Receive %d byte is 0x%x \n", k, rBuffer[k]);
		
		if(k == (iRxLenRead - 1)){
           ANT_DEBUG_V(" \n");
		}
		
		k++;
   }
   */

   memcpy(auRxBuffer , rBuffer, iRxLenRead);	 

   //hci_event_packet_t *event_packet = (hci_event_packet_t *)rBuffer;
   //int hci_payload_len = event_packet->packet_length;

   ANT_DEBUG_V("Receive Package type byte is 0x%x \n", auRxBuffer[0]);
   ANT_DEBUG_V("Receive Submit Package Length is 0x%x \n", auRxBuffer[1]);

   ANT_SERIAL(auRxBuffer, iRxLenRead, 'R');

   int number = 0; 
   int nLength = iRxLenRead;
   int i = 0;
   int j = 1;


   // Estimate the package header whether is correct
   if((atoi(&auRxBuffer[PACKAGE_TYPE]) != COMMAND_HEDAER ) || (atoi(&auRxBuffer[PACKAGE_TYPE]) != HEADER_TYPE)){

       ANT_DEBUG_V("Receiva data package header is 0x%x \n ", atoi(&auRxBuffer[PACKAGE_TYPE]));
	   while( i < nLength)
	   {
		   if((atoi(&auRxBuffer[i]) != 0x0C) || (atoi(&auRxBuffer[i]) != 0x0E)){
             i++;
		   }
		   else
		   {
			   int temp = sizeof(*pTempBuff);
               memcpy((pTempBuff + temp) , &auRxBuffer, i);	
			   pstChnlInfo->fnRxCallback((i+ temp) ,pTempBuff);
			   memcpy(&auRxBuffer, &auRxBuffer[i], nLength - i);
               nLength = nLength - i;
			   break;
		   }
	   }
   }

   // Estimate package numbers(more than one package)
   if( nLength > (auRxBuffer[DATA_LOCATION] + DATA_OTHER) ){

      
	   ANT_DEBUG_V("The receive data Length is %d \n", nLength);

	   while(nLength > number ){

		 if((number + DATA_OTHER + auRxBuffer[number + DATA_LOCATION]) <= nLength)
		 {
		   ANT_DEBUG_V("The Send Package number is %d \n ", j);
           ANT_DEBUG_V("The Array number is %d \n",number);
		   ANT_SERIAL(&auRxBuffer[number + DATA_OTHER] , auRxBuffer[number + DATA_LOCATION] , 'R'); 

           pstChnlInfo->fnRxCallback(auRxBuffer[number+DATA_LOCATION], \
                           &auRxBuffer[number + DATA_OTHER]);
		 }
		 else{
		   ANT_DEBUG_V("Receiva a incomplete data package \n ");

		   memcpy(pTempBuff , &auRxBuffer[number], nLength - number);
		 }
		  number = auRxBuffer[number + DATA_LOCATION] + DATA_OTHER + number;
		  j++;
	   }
       iRet = 0;
	   goto out;       	
   }

   if (iRxLenRead < 0) {
      if (errno == ENODEV) {
         ANT_ERROR("%s not enabled, exiting rx thread",
               pstChnlInfo->pcDevicePath);

         goto out;
      } else if (errno == ENXIO) {
         ANT_ERROR("%s there is no physical ANT device connected",
               pstChnlInfo->pcDevicePath);

         goto out;
      } else {
         ANT_ERROR("%s read thread exiting, unhandled error: %s",
               pstChnlInfo->pcDevicePath, strerror(errno));

         goto out;
      }
   } else {
      // ANT_SERIAL(event_packet, iRxLenRead, 'R');

      iRxLenRead += iRxBufferLength[eChannel];   // add existing data on
      
      // if we didn't get a full packet, then just exit
      if (iRxLenRead < (aucRxBuffer[eChannel][1] + ANT_HCI_HEADER_SIZE + ANT_HCI_FOOTER_SIZE)) {
         iRxBufferLength[eChannel] = iRxLenRead;
         iRet = 0;
         goto out;
      }

      iRxBufferLength[eChannel] = 0;    // reset buffer length here since we should have a full packet
      
#if ANT_HCI_OPCODE_SIZE == 1  // Check the different message types by opcode
      ANT_U8 opcode = aucRxBuffer[eChannel][ANT_HCI_OPCODE_OFFSET];


      if(ANT_HCI_OPCODE_COMMAND_COMPLETE == opcode) {
         // Command Complete, so signal a FLOW_GO
         if(setFlowControl(pstChnlInfo, ANT_FLOW_GO)) {
            goto out;
         }
      } else if(ANT_HCI_OPCODE_FLOW_ON == opcode) {
         // FLow On, so resend the last Tx
#ifdef ANT_FLOW_RESEND
         // Check if there is a message to resend
         if(pstChnlInfo->ucResendMessageLength > 0) {
            ant_tx_message_flowcontrol_none(eChannel, pstChnlInfo->ucResendMessageLength, pstChnlInfo->pucResendMessage);
         } else {
            ANT_DEBUG_D("Resend requested by chip, but tx request cancelled");
         }
#endif // ANT_FLOW_RESEND
      } else if(ANT_HCI_OPCODE_ANT_EVENT == opcode)
         // ANT Event, send ANT packet to Rx Callback
#endif // ANT_HCI_OPCODE_SIZE == 1
      {
      // Received an ANT packet
        // iCurrentHciPacketOffset = 0;
		   iCurrentHciPacketOffset = 1;

         while(iCurrentHciPacketOffset < iRxLenRead) {

            // TODO Allow HCI Packet Size value to be larger than 1 byte
            // This currently works as no size value is greater than 255, and little endian
            iHciDataSize = auRxBuffer[iCurrentHciPacketOffset + ANT_HCI_SIZE_OFFSET];

			// ANT_DEBUG_V("The HCI Package Data Size is %d\n, iHciDataSize");

            if ((iHciDataSize + ANT_HCI_HEADER_SIZE + ANT_HCI_FOOTER_SIZE + iCurrentHciPacketOffset) > 
                  iRxLenRead) {

			  ANT_DEBUG_V("Receive is a not whole package");
               // we don't have a whole packet
               iRxBufferLength[eChannel] = iRxLenRead - iCurrentHciPacketOffset;
               memcpy(aucRxBuffer[eChannel], &aucRxBuffer[eChannel][iCurrentHciPacketOffset], iRxBufferLength[eChannel]);
               // the increment at the end should push us out of the while loop
            } else
#ifdef ANT_MESG_FLOW_CONTROL
            if (aucRxBuffer[eChannel][iCurrentHciPacketOffset + ANT_HCI_DATA_OFFSET + ANT_MSG_ID_OFFSET] == 
                  ANT_MESG_FLOW_CONTROL) {
               // This is a flow control packet, not a standard ANT message
               if(setFlowControl(pstChnlInfo, \
                     aucRxBuffer[eChannel][iCurrentHciPacketOffset + ANT_HCI_DATA_OFFSET + ANT_MSG_DATA_OFFSET])) {
                  goto out;
               }
            } else
#endif // ANT_MESG_FLOW_CONTROL
            {				
               if (pstChnlInfo->fnRxCallback != NULL) {

				    ANT_DEBUG_V("HCI packets are written to callback");

                  // Loop through read data until all HCI packets are written to callback
					/*
                     pstChnlInfo->fnRxCallback(iHciDataSize, \
                           &aucRxBuffer[eChannel][iCurrentHciPacketOffset + ANT_HCI_DATA_OFFSET]);   
						   */

					ANT_SERIAL(&auRxBuffer[iCurrentHciPacketOffset + ANT_HCI_DATA_OFFSET] , iHciDataSize , 'R'); 

					pstChnlInfo->fnRxCallback(iHciDataSize, \
                           &auRxBuffer[iCurrentHciPacketOffset + ANT_HCI_DATA_OFFSET]);  

               } else {
                  ANT_WARN("%s rx callback is null", pstChnlInfo->pcDevicePath);
               }
            }
            
            iCurrentHciPacketOffset = iCurrentHciPacketOffset + ANT_HCI_HEADER_SIZE + ANT_HCI_FOOTER_SIZE + iHciDataSize;               
         }         
      }

      iRet = 0;
   }

out:
   pthread_mutex_unlock(&stReadStatusLock);
   ANT_FUNC_END();
   return iRet;
}