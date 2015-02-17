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

/*
 * This is sample code for WLAN Test Mode Control
 */

#ifndef __LIBWIFITEST_H__
#define __LIBWIFITEST_H__

/* basic definitions
 * -------------------------------------------------------------------------- */
#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

/* types
 * -------------------------------------------------------------------------- */
#if defined(MTK_TC1_FEATURE)
typedef enum {
	WIFI_TEST_PREAMBLE_LONG,
	WIFI_TEST_PREAMBLE_SHORT,
} WIFI_PreambleType_t;
#else
typedef enum {
	WIFI_TEST_PREAMBLE_LONG,
	WIFI_TEST_PREAMBLE_SHORT,
} PreambleType_t;
#endif
typedef enum _ENUM_RF_AT_FUNCID_T {

    RF_AT_FUNCID_VERSION = 0,
    RF_AT_FUNCID_COMMAND,
    RF_AT_FUNCID_POWER,
    RF_AT_FUNCID_RATE,
    RF_AT_FUNCID_PREAMBLE,
    RF_AT_FUNCID_ANTENNA,
    RF_AT_FUNCID_PKTLEN,
    RF_AT_FUNCID_PKTCNT,
    RF_AT_FUNCID_PKTINTERVAL,
    RF_AT_FUNCID_TEMP_COMPEN,
    RF_AT_FUNCID_TXOPLIMIT,
    RF_AT_FUNCID_ACKPOLICY,
    RF_AT_FUNCID_PKTCONTENT,
    RF_AT_FUNCID_RETRYLIMIT,
    RF_AT_FUNCID_QUEUE,
    RF_AT_FUNCID_BANDWIDTH,
    RF_AT_FUNCID_GI,
    RF_AT_FUNCID_STBC,
    RF_AT_FUNCID_CHNL_FREQ,
    RF_AT_FUNCID_RIFS,
    RF_AT_FUNCID_TRSW_TYPE,
    RF_AT_FUNCID_RF_SX_SHUTDOWN,
    RF_AT_FUNCID_PLL_SHUTDOWN,
    RF_AT_FUNCID_SLOW_CLK_MODE,
    RF_AT_FUNCID_ADC_CLK_MODE,
    RF_AT_FUNCID_MEASURE_MODE,
    RF_AT_FUNCID_VOLT_COMPEN,
    RF_AT_FUNCID_DPD_TX_GAIN,
    RF_AT_FUNCID_DPD_MODE,
    RF_AT_FUNCID_TSSI_MODE,
    RF_AT_FUNCID_TX_GAIN_CODE,
    RF_AT_FUNCID_TX_PWR_MODE,

    /* Query command */
    RF_AT_FUNCID_TXED_COUNT = 32,
    RF_AT_FUNCID_TXOK_COUNT,
    RF_AT_FUNCID_RXOK_COUNT,
    RF_AT_FUNCID_RXERROR_COUNT,
    RF_AT_FUNCID_RESULT_INFO,
    RF_AT_FUNCID_TRX_IQ_RESULT,
    RF_AT_FUNCID_TSSI_RESULT,
    RF_AT_FUNCID_DPD_RESULT,
    RF_AT_FUNCID_RXV_DUMP,
    RF_AT_FUNCID_RX_PHY_STATIS,
    RF_AT_FUNCID_MEASURE_RESULT,
    RF_AT_FUNCID_TEMP_SENSOR,
    RF_AT_FUNCID_VOLT_SENSOR,
    RF_AT_FUNCID_READ_EFUSE,
    RF_AT_FUNCID_RX_RSSI,

    /* Set command */
    RF_AT_FUNCID_SET_DPD_RESULT = 64,
    RF_AT_FUNCID_SET_CW_MODE,
    RF_AT_FUNCID_SET_JAPAN_CH14_FILTER,
    RF_AT_FUNCID_WRITE_EFUSE,
    RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
    RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
    RF_AT_FUNCID_SET_RXOK_MATCH_RULE,

} ENUM_RF_AT_FUNCID_T;

typedef enum _ENUM_RF_AT_COMMAND_T {

    RF_AT_COMMAND_STOPTEST = 0,
    RF_AT_COMMAND_STARTTX,
    RF_AT_COMMAND_STARTRX,
    RF_AT_COMMAND_RESET,
    RF_AT_COMMAND_OUTPUT_POWER,     /* Payload */
    RF_AT_COMMAND_LO_LEAKAGE,       /* Local freq is renamed to Local leakage */
    RF_AT_COMMAND_CARRIER_SUPPR,    /* OFDM (LTF/STF), CCK (PI,PI/2) */
    RF_AT_COMMAND_TRX_IQ_CAL,
    RF_AT_COMMAND_TSSI_CAL,
    RF_AT_COMMAND_DPD_CAL,
    RF_AT_COMMAND_CW,
    RF_AT_COMMAND_NUM

} ENUM_RF_AT_COMMAND_T;

typedef enum _ENUM_RF_AT_PREAMBLE_T {

    RF_AT_PREAMBLE_NORMAL = 0,
    RF_AT_PREAMBLE_CCK_SHORT,
    RF_AT_PREAMBLE_11N_MM,
    RF_AT_PREAMBLE_11N_GF,
    RF_AT_PREAMBLE_NUM

} ENUM_RF_AT_PREAMBLE_T;

typedef enum _ENUM_RF_AT_BW_T {

    RF_AT_BW_20 = 0,
    RF_AT_BW_40,
    RF_AT_BW_U20,
    RF_AT_BW_L20,
    RF_AT_BW_NUM

} ENUM_RF_AT_BW_T, *P_ENUM_RF_AT_BW_T;

typedef enum _ENUM_RF_AT_RXOK_MATCH_RULE_T {

    RF_AT_RXOK_DISABLED = 0,
    RF_AT_RXOK_MATCH_RA_ONLY,
    RF_AT_RXOK_MATCH_TA_ONLY,
    RF_AT_RXOK_MATCH_RA_TA,
    RF_AT_RXOK_NUM

} ENUM_RF_AT_RXOK_MATCH_RULE_T, *P_ENUM_RF_AT_RXOK_MATCH_RULE_T;
/* WIFI_TEST_TxDataRate */
#define WIFI_TEST_RATE_AUTO         0
#define WIFI_TEST_RATE_1MBPS		1
#define WIFI_TEST_RATE_2MBPS		2
#define WIFI_TEST_RATE_5_5MBPS		3
#define WIFI_TEST_RATE_6MBPS		4
#define WIFI_TEST_RATE_9MBPS		5
#define WIFI_TEST_RATE_11MBPS		6
#define WIFI_TEST_RATE_12MBPS		7
#define WIFI_TEST_RATE_18MBPS		8
#define WIFI_TEST_RATE_24MBPS		9
#define WIFI_TEST_RATE_36MBPS		10
#define WIFI_TEST_RATE_48MBPS		11
#define WIFI_TEST_RATE_54MBPS		12

/* Supported MCS rates */
typedef enum _tagENUM_WIFI_TEST_MCS_RATE {
	WIFI_TEST_MCS_RATE_0 = 0,
	WIFI_TEST_MCS_RATE_1 = 1,
	WIFI_TEST_MCS_RATE_2 = 2,
	WIFI_TEST_MCS_RATE_3 = 3,
	WIFI_TEST_MCS_RATE_4 = 4,
	WIFI_TEST_MCS_RATE_5 = 5,
	WIFI_TEST_MCS_RATE_6 = 6,
	WIFI_TEST_MCS_RATE_7 = 7,
	WIFI_TEST_MCS_RATE_32= 32,
} ENUM_WIFI_TEST_MCS_RATE, *P_ENUM_WIFI_TEST_MCS_RATE;

/* Preamble Type */
typedef enum _tagENUM_WIFI_TEST_PREAMBLE_TYPE {
    WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE = 0,
    WIFI_TEST_PREAMBLE_TYPE_GREENFIELD = 1
} ENUM_WIFI_TEST_PREAMBLE_TYPE, *P_ENUM_WIFI_TEST_PREAMBLE_TYPE;

/* Guard Interval Type */
typedef enum _tagENUM_WIFI_TEST_GI_TYPE {
    WIFI_TEST_GI_TYPE_NORMAL_GI = 0,
    WIFI_TEST_GI_TYPE_SHORT_GI  = 1
} ENUM_WIFI_TEST_GI_TYPE, *P_ENUM_WIFI_TEST_GI_TYPE;

/* Bandwidth Type */
typedef enum _tagENUM_WIFI_BANDWIDTH {
    WIFI_TEST_BW_20MHZ  = 0,
    WIFI_TEST_BW_40MHZ  = 1,
    WIFI_TEST_BW_U20MHZ = 2,
    WIFI_TEST_BW_D20MHZ = 3,
    WIFI_TEST_BW_NUM    = 4,
} ENUM_WIFI_BANDWIDTH, *P_ENUM_WIFI_BANDWIDTH;

/* Wi-Fi TEST MODE */
typedef enum _tagENUM_WIFI_TEST_MODE {
    WIFI_TEST_MODE_BY_API_CONTROL   = 0,
    WIFI_TEST_MODE_CW_ONLY          = 1,
    WIFI_TEST_MODE_80211A_ONLY      = 2,
    WIFI_TEST_MODE_80211B_ONLY      = 3,
    WIFI_TEST_MODE_80211G_ONLY      = 4,
    WIFI_TEST_MODE_80211N_ONLY      = 5,
    WIFI_TEST_MODE_NUM              = 6
} ENUM_WIFI_TEST_MODE, *P_ENUM_WIFI_TEST_MODE;

#if defined(MTK_TC1_FEATURE)
#ifndef BIT
#define BIT(n)          ((uint32_t) 1 << (n))
#define BITS(m,n)       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

/* RF Test Properties */
#define RF_AT_PARAM_RATE_MCS_MASK   BIT(31)
#define RF_AT_PARAM_RATE_MASK       BITS(0,7)
#define RF_AT_PARAM_RATE_1M         0
#define RF_AT_PARAM_RATE_2M         1
#define RF_AT_PARAM_RATE_5_5M       2
#define RF_AT_PARAM_RATE_11M        3
#define RF_AT_PARAM_RATE_6M         4
#define RF_AT_PARAM_RATE_9M         5
#define RF_AT_PARAM_RATE_12M        6
#define RF_AT_PARAM_RATE_18M        7
#define RF_AT_PARAM_RATE_24M        8
#define RF_AT_PARAM_RATE_36M        9
#define RF_AT_PARAM_RATE_48M        10
#define RF_AT_PARAM_RATE_54M        11
#endif

/* functions
 * -------------------------------------------------------------------------- */
bool WIFI_TEST_OpenDUT(void);
bool WIFI_TEST_CloseDUT(void);

/* Style #A - Control API */
bool WIFI_TEST_TxDataRate(int TxDataRate);
#if defined(MTK_TC1_FEATURE)
bool WIFI_TEST_SetPreamble(WIFI_PreambleType_t PreambleType);
#else
bool WIFI_TEST_SetPreamble(PreambleType_t PreambleType);
#endif
bool WIFI_TEST_Channel(int ChannelNo);
bool WIFI_TEST_TxGain(int TxGain);
bool WIFI_TEST_TxBurstInterval(int SIFS);
bool WIFI_TEST_TxPayloadLength(int TxPayLength);
bool WIFI_TEST_TxBurstFrames(int Frames);
bool WIFI_TEST_TxDestAddress(unsigned char *addr);
bool WIFI_TEST_TxStart(void);
bool WIFI_TEST_TxStop(void);
bool WIFI_TEST_RxStart(void);
bool WIFI_TEST_RxStop(void);
bool WIFI_TEST_FRError(int *FError);
bool WIFI_TEST_FRGood(int *FRGood);
bool WIFI_TEST_RSSI(int *RSSI);
bool WIFI_TEST_IsRunning(void);
bool WIFI_TEST_IsUp(void);
bool WIFI_TEST_TxDataRate11n(ENUM_WIFI_TEST_MCS_RATE eDataRate11n, ENUM_WIFI_TEST_PREAMBLE_TYPE ePreambleType, ENUM_WIFI_TEST_GI_TYPE eGIType);
bool WIFI_TEST_FrequencyAccuracy(int ChannelNo);
bool WIFI_TEST_FrequencyAccuracy_Stop();

/* Style #B - Control API */
bool WIFI_TEST_SetMode(ENUM_WIFI_TEST_MODE eMode);
bool WIFI_TEST_GetSupportedMode(uint32_t *pu4SupportedMode);
bool WIFI_TEST_SetBandwidth(ENUM_WIFI_BANDWIDTH eBandwidth);
bool WIFI_TEST_SetFrequency(uint32_t u4FreqInMHz, uint32_t u4OffsetInKHz);
bool WIFI_TEST_SetRate(uint32_t u4Rate);
bool WIFI_TEST_SetTXPower(uint32_t u4Gain);
bool WIFI_TEST_SetTX(bool fgEnable);
bool WIFI_TEST_SetRX(bool fgEnable, char *aucSrcAddr, char *aucDstAddr);
bool WIFI_TEST_ClearResult(void);
bool WIFI_TEST_GetResult(uint32_t *pu4GoodFrameCount, uint32_t *pu4BadFrameCount);

#endif /* __LIBWIFITEST_H__ */
