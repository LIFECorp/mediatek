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
/*******************************************************************************\
*
*   FILE NAME:      ant_rx_chardev.h
*
*   BRIEF:
*      This file defines the receive thread function, the ant_rx_thread_info_t
*      type for storing the -configuration- <state> of the receive thread, the
*      ant_channel_info_t type for storing a channel's (transport path)
*      configuration, and an enumeration of all ANT channels (transport paths).
*
*
\*******************************************************************************/

#ifndef __ANT_RX_NATIVE_H
#define __ANT_RX_NATIVE_H

#include "ant_native.h"
#include "ant_hci_defines.h"

/* same as HCI_MAX_EVENT_SIZE from hci.h, but hci.h is not included for vfs */
// #define ANT_HCI_MAX_MSG_SIZE 260
#define ANT_HCI_MAX_MSG_SIZE  16384

#define ANT_MSG_SIZE_OFFSET     ((ANT_U8)0)
#define ANT_MSG_ID_OFFSET       ((ANT_U8)1)
#define ANT_MSG_DATA_OFFSET     ((ANT_U8)2)

/* This struct defines the info passed to an rx thread */
typedef struct {
   /* Device path */
   const char *pcDevicePath;
   /* File descriptor to read from */
   int iFd;
   /* Callback to call with ANT packet */
   ANTNativeANTEventCb fnRxCallback;
   /* Flow control response if channel supports it */
   ANT_U8 ucFlowControlResp;
   /* Handle to flow control condition */
   pthread_cond_t *pstFlowControlCond;
   /* Handle to flow control mutex */
   pthread_mutex_t *pstFlowControlLock;
#ifdef ANT_FLOW_RESEND
   /* Length of message to resend on request from chip */
   ANT_U8 ucResendMessageLength;
   /* The message to resend on request from chip */
   ANT_U8 *pucResendMessage;
#endif // ANT_FLOW_RESEND
} ant_channel_info_t;

typedef enum {
#ifdef ANT_DEVICE_NAME // Single transport path
   SINGLE_CHANNEL,
#else // Separate data/command paths
   DATA_CHANNEL,
   COMMAND_CHANNEL,
#endif // Separate data/command paths
   NUM_ANT_CHANNELS
} ant_channel_type;

typedef struct {
   /* Thread handle */
   pthread_t stRxThread;
   /* Exit condition */
   ANT_U8 ucRunThread;
   /* Set state as resetting override */
   ANT_U8 ucChipResetting;
   /* Handle to state change lock for crash cleanup */
   pthread_mutex_t *pstEnabledStatusLock;
   /* ANT channels */
   ant_channel_info_t astChannels[NUM_ANT_CHANNELS];
   /* Event file descriptor used to interrupt the poll() loop in the rx thread. */
   int iRxShutdownEventFd;
} ant_rx_thread_info_t;

extern ANTNativeANTStateCb g_fnStateCallback;  // TODO State callback should be inside ant_rx_thread_info_t.

/* This is the rx thread function. It loops reading ANT packets until told to
 * exit */
void *fnRxThread(void *ant_rx_thread_info);

#endif /* ifndef __ANT_RX_NATIVE_H */
