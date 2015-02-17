/*
 * ANT Stack
 *
 * Copyright 2009 Dynastream Innovations
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
*   FILE NAME:      ant_native.h
*
*   BRIEF:
*		This file defines the interface to the ANT transport layer
*
*
\******************************************************************************/

#ifndef __ANT_NATIVE_H
#define __ANT_NATIVE_H

/*******************************************************************************
 *
 * Include files
 *
 ******************************************************************************/
#include "ant_types.h"

/*******************************************************************************
 *
 * Constants
 *
 ******************************************************************************/

/*******************************************************************************
 *
 * Types
 *
 ******************************************************************************/
typedef void (*ANTNativeANTEventCb)(ANT_U8 ucLen, ANT_U8* pucData);
typedef void (*ANTNativeANTStateCb)(ANTRadioEnabledStatus uiNewState);

/*******************************************************************************
 *
 * Data Structures
 *
 ******************************************************************************/

/*******************************************************************************
 *
 * Function declarations
 *
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * ant_init()
 *
 * Called to initialize the ANT stack
 */
ANTStatus ant_init(void);

/*------------------------------------------------------------------------------
 * ant_deinit()
 *
 * Called to de-initialize the ANT stack
 */
ANTStatus ant_deinit(void);

/*------------------------------------------------------------------------------
 * ant_enable_radio()
 *
 * Power on the chip and set it to a state which can read / write to it
 */
ANTStatus ant_enable_radio(void);

/*------------------------------------------------------------------------------
 * ant_disable_radio()
 *
 * Kill power to the ANT chip and set state which doesnt allow read / write
 */
ANTStatus ant_disable_radio(void);

/*------------------------------------------------------------------------------
 * ant_radio_enabled_status()
 *
 * Gets if the chip/transport is disabled/disabling/enabling/enabled
 */
ANTRadioEnabledStatus ant_radio_enabled_status(void);

/*------------------------------------------------------------------------------
 * set_ant_rx_callback()
 *
 * Sets the callback function for receiving ANT messages
 */
ANTStatus set_ant_rx_callback(ANTNativeANTEventCb rx_callback_func);

/*------------------------------------------------------------------------------
 * set_ant_state_callback()
 *
 * Sets the callback function for any ANT radio enabled status state changes
 */
ANTStatus set_ant_state_callback(ANTNativeANTStateCb state_callback_func);

/*------------------------------------------------------------------------------
 * ant_tx_message()
 *
 * Sends an ANT message command to the chip.
 */
ANTStatus ant_tx_message(ANT_U8 ucLen, ANT_U8 *pucMesg);

/*------------------------------------------------------------------------------
 * ant_radio_hard_reset()
 *
 * If supported, forces the chip to do a power cycle reset
 */
ANTStatus ant_radio_hard_reset(void);

/*------------------------------------------------------------------------------
 * ant_get_lib_version()
 *
 * Gets the current version and transport type of libantradio.so
 */
const char *ant_get_lib_version(void);

#endif /* ifndef __ANT_NATIVE_H */
