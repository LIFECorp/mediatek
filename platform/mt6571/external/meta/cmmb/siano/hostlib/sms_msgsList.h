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
/* Copyright (C) 2005,2006 Siano Mobile Silicon Ltd. All rights RES,    ) \erved */
/*                                                                       */
/* PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the        */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#ifndef MSGS_LIST_H
#define MSGS_LIST_H

/*************************************************************************/
/*			 Includes, Defines and Macros								 */
/*************************************************************************/
#define NA 0xFFFF // used for not relevant msg length



#define GENERATE_MSGS_IDS\
	 X(    MSG_SMS_GET_VERSION_REQ,							8) \
     X(    MSG_SMS_GET_VERSION_RES,							11) \
     X(    MSG_SMS_GPIO_SET_LEVEL_REQ,						20) \
     X(    MSG_SMS_GPIO_SET_LEVEL_RES,						16) \
     X(    MSG_SMS_GPIO_GET_LEVEL_REQ,						16) \
     X(    MSG_SMS_GPIO_GET_LEVEL_RES,						16) \
     X(    MSG_SMS_LOG_ENABLE_CHANGE_REQ,					16) \
     X(    MSG_SMS_LOG_ENABLE_CHANGE_RES,					12) \
     X(    MSG_SMS_SET_MAX_TX_MSG_LEN_REQ,					NA) \
     X(    MSG_SMS_SET_MAX_TX_MSG_LEN_RES,					NA) \
     X(    MSG_SMS_SPI_HALFDUPLEX_TOKEN_HOST_TO_DEVICE,		NA) \
     X(    MSG_SMS_SPI_HALFDUPLEX_TOKEN_DEVICE_TO_HOST,		NA) \
     X(    MSG_WR_REG_RFT_REQ,								NA) \
     X(    MSG_WR_REG_RFT_RES,								NA) \
     X(    MSG_RD_REG_RFT_REQ,								NA) \
     X(    MSG_RD_REG_RFT_RES,								NA) \
     X(    MSG_RD_REG_ALL_RFT_REQ,							NA) \
     X(    MSG_RD_REG_ALL_RFT_RES,							NA) \
     X(    MSG_HELP_INT,		  							NA) \
     X(    MSG_RUN_SCRIPT_INT,	  							NA) \
     X(    MSG_SMS_RD_MEM_REQ,    							NA) \
     X(    MSG_SMS_RD_MEM_RES,    							NA) \
     X(    MSG_SMS_WR_MEM_REQ,    							NA) \
     X(    MSG_SMS_WR_MEM_RES,    							NA) \
     X(    MSG_SMS_RF_TUNE_REQ,    							20) \
     X(    MSG_SMS_RF_TUNE_RES,    							12) \
     X(    MSG_SMS_EEPROM_WRITE_REQ,    					NA) \
     X(    MSG_SMS_EEPROM_WRITE_RES,    					NA) \
     X(    MSG_SMS_INIT_DEVICE_REQ,    						12) \
     X(    MSG_SMS_INIT_DEVICE_RES,    						12) \
     X(    MSG_SMS_SUB_CHANNEL_START_REQ,				    NA) \
     X(    MSG_SMS_SUB_CHANNEL_START_RES,				    NA) \
     X(    MSG_SMS_SUB_CHANNEL_STOP_REQ,				    NA) \
     X(    MSG_SMS_SUB_CHANNEL_STOP_RES,				    NA) \
     X(    MSG_SMS_WAIT_CMD,								NA) \
     X(    MSG_SMS_ADD_PID_FILTER_REQ,						12) \
     X(    MSG_SMS_ADD_PID_FILTER_RES,						12) \
     X(    MSG_SMS_REMOVE_PID_FILTER_REQ,				    12) \
     X(    MSG_SMS_REMOVE_PID_FILTER_RES,				    12) \
     X(    MSG_SMS_FAST_INFORMATION_CHANNEL_REQ,			8) \
     X(    MSG_SMS_FAST_INFORMATION_CHANNEL_RES,			8) \
     X(    MSG_SMS_DAB_CHANNEL,								NA) \
     X(    MSG_SMS_GET_PID_FILTER_LIST_REQ,				    8) \
     X(    MSG_SMS_GET_PID_FILTER_LIST_RES,				    60) \
     X(    MSG_SMS_GET_STATISTICS_REQ,					    8) \
     X(    MSG_SMS_GET_STATISTICS_RES,					    224) \
     X(    MSG_SMS_SEND_DUMP,								NA) \
     X(    MSG_SMS_SCAN_START_REQ,						    NA) \
     X(    MSG_SMS_SCAN_START_RES,						    NA) \
     X(    MSG_SMS_SCAN_STOP_REQ,						    NA) \
     X(    MSG_SMS_SCAN_STOP_RES,						    NA) \
     X(    MSG_SMS_SCAN_PROGRESS_IND,						NA) \
     X(    MSG_SMS_SCAN_COMPLETE_IND,						NA) \
     X(    MSG_SMS_LOG_ITEM,								NA) \
     X(    MSG_SMS_DAB_SUBCHANNEL_RECONFIG_REQ,				NA) \
     X(    MSG_SMS_DAB_SUBCHANNEL_RECONFIG_RES,				NA) \
     X(    MSG_SMS_HO_PER_SLICES_IND,						108) \
     X(    MSG_SMS_HO_INBAND_POWER_IND,						16) \
     X(    MSG_SMS_MANUAL_DEMOD_REQ,						NA) \
     X(    MSG_SMS_HO_TUNE_ON_REQ,							NA) \
     X(    MSG_SMS_HO_TUNE_ON_RES,							NA) \
     X(    MSG_SMS_HO_TUNE_OFF_REQ,						    NA) \
     X(    MSG_SMS_HO_TUNE_OFF_RES,						    NA) \
     X(    MSG_SMS_HO_PEEK_FREQ_REQ,						NA) \
     X(    MSG_SMS_HO_PEEK_FREQ_RES,						NA) \
     X(    MSG_SMS_HO_PEEK_FREQ_IND,						NA) \
     X(    MSG_SMS_ENABLE_STAT_IN_I2C_REQ,				    12) \
     X(    MSG_SMS_ENABLE_STAT_IN_I2C_RES,				    12) \
     X(    MSG_SMS_GET_STATISTICS_EX_REQ,				    8) \
     X(    MSG_SMS_GET_STATISTICS_EX_RES,				    240) \
     X(    MSG_SMS_SLEEP_RESUME_COMP_IND,					NA) \
     X(    MSG_SMS_DATA_DOWNLOAD_REQ,					    NA) \
     X(    MSG_SMS_DATA_DOWNLOAD_RES,					    NA) \
     X(    MSG_SMS_DATA_VALIDITY_REQ,					    NA) \
     X(    MSG_SMS_DATA_VALIDITY_RES,					    NA) \
     X(    MSG_SMS_SWDOWNLOAD_TRIGGER_REQ,					NA) \
     X(    MSG_SMS_SWDOWNLOAD_TRIGGER_RES,					NA) \
     X(    MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ,				    NA) \
     X(    MSG_SMS_SWDOWNLOAD_BACKDOOR_RES,				    NA) \
     X(    MSG_SMS_GET_VERSION_EX_REQ,					    8) \
     X(    MSG_SMS_GET_VERSION_EX_RES,					    100) \
     X(    MSG_SMS_CLOCK_OUTPUT_CONFIG_REQ,    				NA) \
     X(    MSG_SMS_CLOCK_OUTPUT_CONFIG_RES,    				NA) \
     X(    MSG_SMS_I2C_SET_FREQ_REQ,    					NA) \
     X(    MSG_SMS_I2C_SET_FREQ_RES,    					NA) \
     X(    MSG_SMS_GENERIC_I2C_REQ,    						NA) \
     X(    MSG_SMS_GENERIC_I2C_RES,    						NA) \
     X(    MSG_SMS_DVBT_BDA_DATA,							NA) \
     X(    MSG_SMS_DATA_MSG,								NA) \
     X(    MSG_TABLE_UPLOAD_REQ,						    NA) \
     X(    MSG_TABLE_UPLOAD_RES,						    NA) \
     X(    MSG_SW_RELOAD_START_REQ,						    NA) \
     X(    MSG_SW_RELOAD_START_RES,						    NA) \
     X(    MSG_SW_RELOAD_EXEC_REQ, 						    NA) \
     X(    MSG_SW_RELOAD_EXEC_RES, 						    NA) \
     X(    MSG_SMS_SPI_INT_LINE_SET_REQ,					NA) \
     X(    MSG_SMS_SPI_INT_LINE_SET_RES,					NA) \
     X(    MSG_SMS_GPIO_CONFIG_EX_REQ,    					NA) \
     X(    MSG_SMS_GPIO_CONFIG_EX_RES,    					NA) \
     X(    MSG_SMS_WATCHDOG_ACT_REQ,    					NA) \
     X(    MSG_SMS_WATCHDOG_ACT_RES,    					NA) \
     X(    MSG_SMS_LOOPBACK_REQ,						    NA) \
     X(    MSG_SMS_LOOPBACK_RES,						    NA) \
     X(    MSG_SMS_RAW_CAPTURE_START_REQ,				    NA) \
     X(    MSG_SMS_RAW_CAPTURE_START_RES,				    NA) \
     X(    MSG_SMS_RAW_CAPTURE_ABORT_REQ,				    NA) \
     X(    MSG_SMS_RAW_CAPTURE_ABORT_RES,				    NA) \
     X(    MSG_SMS_RAW_CAPTURE_COMPLETE_IND,				NA) \
     X(    MSG_SMS_DATA_PUMP_IND,							NA) \
     X(    MSG_SMS_DATA_PUMP_REQ,						    NA) \
     X(    MSG_SMS_DATA_PUMP_RES,						    NA) \
     X(    MSG_SMS_FLASH_DL_REQ, 						    NA) \
     X(    MSG_SMS_FLASH_DL_RES, 						    NA) \
     X(    MSG_SMS_EXEC_TEST_1_REQ,							NA) \
     X(    MSG_SMS_EXEC_TEST_1_RES,							NA) \
     X(    MSG_SMS_ENBALE_TS_INTERFACE_REQ,					NA) \
     X(    MSG_SMS_ENBALE_TS_INTERFACE_RES,					NA) \
     X(    MSG_SMS_SPI_SET_BUS_WIDTH_REQ,					NA) \
     X(    MSG_SMS_SPI_SET_BUS_WIDTH_RES,					NA) \
     X(    MSG_SMS_DISABLE_TS_INTERFACE_REQ,				NA) \
     X(    MSG_SMS_DISABLE_TS_INTERFACE_RES,				NA) \
     X(    MSG_SMS_EXT_ANTENNA_REQ,							NA) \
     X(    MSG_SMS_EXT_ANTENNA_RES,							NA) \
     X(    MSG_SMS_CMMB_GET_NET_OF_FREQ_REQ_OBSOLETE,		NA) \
     X(    MSG_SMS_CMMB_GET_NET_OF_FREQ_RES_OBSOLETE,       NA) \
     X(    MSG_SMS_CMMB_INJECT_TABLE_REQ_OBSOLETE,			NA) \
     X(    MSG_SMS_CMMB_INJECT_TABLE_RES_OBSOLETE,			NA) \
     X(    MSG_SMS_FM_RADIO_BLOCK_IND,						NA) \
     X(    MSG_SMS_HOST_NOTIFICATION_IND,					NA) \
     X(    MSG_SMS_CMMB_GET_CONTROL_TABLE_REQ_OBSOLETE,		NA) \
     X(    MSG_SMS_CMMB_GET_CONTROL_TABLE_RES_OBSOLETE,		NA) \
     X(    MSG_SMS_CMMB_GET_NETWORKS_REQ,					NA) \
     X(    MSG_SMS_CMMB_GET_NETWORKS_RES,					NA) \
     X(    MSG_SMS_CMMB_START_SERVICE_REQ,					NA) \
     X(    MSG_SMS_CMMB_START_SERVICE_RES,					NA) \
     X(    MSG_SMS_CMMB_STOP_SERVICE_REQ,					NA) \
     X(    MSG_SMS_CMMB_STOP_SERVICE_RES,					NA) \
     X(    MSG_SMS_CMMB_ADD_CHANNEL_FILTER_REQ,				NA) \
     X(    MSG_SMS_CMMB_ADD_CHANNEL_FILTER_RES,				NA) \
     X(    MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_REQ,			NA) \
     X(    MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_RES,			NA) \
     X(    MSG_SMS_CMMB_START_CONTROL_INFO_REQ,				NA) \
     X(    MSG_SMS_CMMB_START_CONTROL_INFO_RES,				NA) \
     X(    MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ,				NA) \
     X(    MSG_SMS_CMMB_STOP_CONTROL_INFO_RES,				NA) \
     X(    MSG_SMS_ISDBT_TUNE_REQ,							24) \
     X(    MSG_SMS_ISDBT_TUNE_RES,							12) \
     X(    MSG_SMS_TRANSMISSION_IND,						56) \
     X(    MSG_SMS_PID_STATISTICS_IND,						NA) \
     X(    MSG_SMS_POWER_DOWN_IND,							NA) \
     X(    MSG_SMS_POWER_DOWN_CONF,							NA) \
     X(    MSG_SMS_POWER_UP_IND,							NA) \
     X(    MSG_SMS_POWER_UP_CONF,							NA) \
     X(    MSG_SMS_POWER_MODE_SET_REQ,						NA) \
     X(    MSG_SMS_POWER_MODE_SET_RES,						NA) \
     X(    MSG_SMS_DEBUG_HOST_EVENT_REQ,					NA) \
     X(    MSG_SMS_DEBUG_HOST_EVENT_RES,					NA) \
     X(    MSG_SMS_NEW_CRYSTAL_REQ,						    NA) \
     X(    MSG_SMS_NEW_CRYSTAL_RES,						    NA) \
     X(    MSG_SMS_CONFIG_SPI_REQ,							NA) \
     X(    MSG_SMS_CONFIG_SPI_RES,							NA) \
     X(    MSG_SMS_I2C_SHORT_STAT_IND,						NA) \
     X(    MSG_SMS_START_IR_REQ,							NA) \
     X(    MSG_SMS_START_IR_RES,							NA) \
     X(    MSG_SMS_IR_SAMPLES_IND,							NA) \
     X(    MSG_SMS_CMMB_CA_SERVICE_IND,						NA) \
     X(    MSG_SMS_SLAVE_DEVICE_DETECTED,					12) \
     X(    MSG_SMS_INTERFACE_LOCK_IND,						8) \
     X(    MSG_SMS_INTERFACE_UNLOCK_IND,					8) \
     X(    MSG_SMS_SET_AES128_KEY_REQ,						40) \
     X(    MSG_SMS_SET_AES128_KEY_RES,						40) \
     X(    MSG_SMS_IQ_STREAM_START_REQ,						NA) \
     X(    MSG_SMS_IQ_STREAM_START_RES,						NA) \
     X(    MSG_SMS_IQ_STREAM_STOP_REQ,						NA) \
     X(    MSG_SMS_IQ_STREAM_STOP_RES,						NA) \
     X(    MSG_SMS_IQ_STREAM_DATA_BLOCK,					NA) \
     X(    MSG_SMS_GET_EEPROM_VERSION_REQ,					NA) \
     X(    MSG_SMS_GET_EEPROM_VERSION_RES,					NA) \
     X(    MSG_SMS_SIGNAL_DETECTED_IND,						8) \
     X(    MSG_SMS_NO_SIGNAL_IND,							8) \
     X(    MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ,				    NA) \
     X(    MSG_SMS_MRC_SHUTDOWN_SLAVE_RES,				    NA) \
     X(    MSG_SMS_MRC_BRINGUP_SLAVE_REQ,				    NA) \
     X(    MSG_SMS_MRC_BRINGUP_SLAVE_RES,				    NA) \
     X(    MSG_SMS_SET_PERIODIC_STATISTICS_REQ,				NA) \
     X(    MSG_SMS_SET_PERIODIC_STATISTICS_RES,				NA) \
     X(    MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ,			NA) \
     X(    MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_RES,			NA) \
	 X(	   LOCAL_TUNE,										NA) \
	 X(    LOCAL_IFFT_H_ICI,								NA) \
     X(    MSG_RESYNC_REQ,									NA) \
     X(    MSG_SMS_CMMB_GET_MRC_STATISTICS_REQ,				NA) \
     X(    MSG_SMS_CMMB_GET_MRC_STATISTICS_RES,				NA) \
     X(    MSG_SMS_LOG_EX_ITEM,								NA) \
     X(    MSG_SMS_DEVICE_DATA_LOSS_IND,					NA) \
     X(    MSG_SMS_USER_MSG_REQ,							NA) \
     X(    MSG_SMS_USER_MSG_RES,							NA) \
     X(    MSG_SMS_SMART_CARD_INIT_REQ,						NA) \
     X(    MSG_SMS_SMART_CARD_INIT_RES,						NA) \
     X(    MSG_SMS_SMART_CARD_WRITE_REQ,					NA) \
     X(    MSG_SMS_SMART_CARD_WRITE_RES,					NA) \
     X(    MSG_SMS_SMART_CARD_READ_IND,						NA) \
     X(    MSG_SMS_TSE_ENABLE_REQ,							NA) \
     X(    MSG_SMS_TSE_ENABLE_RES,							NA) \
     X(    MSG_SMS_CMMB_GET_SHORT_STATISTICS_REQ,			NA) \
     X(    MSG_SMS_CMMB_GET_SHORT_STATISTICS_RES,			NA) \
     X(    MSG_SMS_LED_CONFIG_REQ,							NA) \
     X(    MSG_SMS_LED_CONFIG_RES,							NA) \
     X(    MSG_SMS_CMMB_SMD_SN_REQ,							NA) \
	 X(    MSG_SMS_CMMB_SMD_SN_RES,							NA) \
     X(    MSG_SMS_CMMB_SET_CA_CW_REQ,						NA) \
     X(    MSG_SMS_CMMB_SET_CA_CW_RES,						NA) \
     X(    MSG_SMS_CMMB_SET_CA_SALT_REQ,					NA) \
     X(    MSG_SMS_CMMB_SET_CA_SALT_RES,					NA) \
     X(    MSG_SMS_NSCD_INIT_REQ,							NA) \
     X(    MSG_SMS_NSCD_INIT_RES,							NA) \
     X(    MSG_SMS_NSCD_PROCESS_SECTION_REQ,				NA) \
     X(    MSG_SMS_NSCD_PROCESS_SECTION_RES,				NA) \
     X(    MSG_SMS_DBD_CREATE_OBJECT_REQ,					NA) \
     X(    MSG_SMS_DBD_CREATE_OBJECT_RES,					NA) \
     X(    MSG_SMS_DBD_CONFIGURE_REQ,						NA) \
     X(    MSG_SMS_DBD_CONFIGURE_RES,						NA) \
     X(    MSG_SMS_DBD_SET_KEYS_REQ,						NA) \
     X(    MSG_SMS_DBD_SET_KEYS_RES,						NA) \
     X(    MSG_SMS_DBD_PROCESS_HEADER_REQ,					NA) \
     X(    MSG_SMS_DBD_PROCESS_HEADER_RES,					NA) \
     X(    MSG_SMS_DBD_PROCESS_DATA_REQ,					NA) \
     X(    MSG_SMS_DBD_PROCESS_DATA_RES,					NA) \
     X(    MSG_SMS_DBD_PROCESS_GET_DATA_REQ,				NA) \
     X(    MSG_SMS_DBD_PROCESS_GET_DATA_RES,				NA) \
     X(    MSG_SMS_NSCD_OPEN_SESSION_REQ,					NA) \
     X(    MSG_SMS_NSCD_OPEN_SESSION_RES,					NA) 


#endif