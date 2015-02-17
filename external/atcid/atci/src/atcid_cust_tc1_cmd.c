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
#if defined(MTK_TC1_FEATURE)

#include <DfoDefines.h>

#include "atcid_serial.h"
#include "atcid_util.h"
#include "atcid_cust_cmd.h"
#include "at_tok.h"
#include "cutils/properties.h"
#include <private/android_filesystem_config.h>
#include <stdlib.h>
#include "libwifitest.h"
#include "lgerft.h"

#define ATCI_SIM "persist.service.atci.sim"

#define TAG  "WLAN_TC1_AT"

int gRateCode = 0;
int gPreamble = 0;
int gGIType = 0;
int gWifiTestMode = 0;
uint32_t gWifiBw = 0;

uint32_t gLastErrorPkg = 0;
uint32_t gLastGoodpkg = 0;

char prevRate[20] = {'\0', '\0','\0','\0','\0',
			'\0','\0','\0','\0','\0',
			'\0','\0','\0','\0','\0',
			'\0','\0','\0','\0','\0'};

static ATRESPONSE_t pas_wisetrate(char* rate){
	bool err = false;
	ATRESPONSE_t ret = AT_ERROR;
	ENUM_WIFI_TEST_MODE eWifiTestMode = WIFI_TEST_MODE_BY_API_CONTROL;
	ENUM_WIFI_TEST_GI_TYPE eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
    	uint32_t u4RateCode;
	uint32_t u4Preamble = RF_AT_PREAMBLE_11N_MM;
	uint32_t u4Bandwidth = WIFI_TEST_BW_20MHZ;
	char *tmp = rate;

	LOGATCI(LOG_DEBUG, "pas_wisetrate %s\n", rate);
	WIFI_TEST_OpenDUT();

	/*set test mode*/
	switch(rate[0]){
		case 'B':
			eWifiTestMode = WIFI_TEST_MODE_80211B_ONLY;
			break;
		case 'G':
			eWifiTestMode = WIFI_TEST_MODE_80211G_ONLY;
			break;
		case 'N':
			eWifiTestMode = WIFI_TEST_MODE_80211N_ONLY;
			break;
		case 'H':
			eWifiTestMode = WIFI_TEST_MODE_80211N_ONLY;
			u4Bandwidth = WIFI_TEST_BW_40MHZ;
			break;
	}
	WIFI_TEST_SetMode(eWifiTestMode);

	/* validate rate by mode/bandwidth settings */
    	if(eWifiTestMode == WIFI_TEST_MODE_80211B_ONLY) {
		int num = 0;
		tmp = rate + 1;
		at_tok_nextint(&tmp, &num);

		LOGATCI(LOG_DEBUG, "B mode num is %d\n", num);
		switch(num) {
		case 1:
		u4RateCode = RF_AT_PARAM_RATE_1M;
		break;

		case 2:
		u4RateCode = RF_AT_PARAM_RATE_2M;
		break;

		case 5:
		u4RateCode = RF_AT_PARAM_RATE_5_5M;
		break;

		case 11:
		u4RateCode = RF_AT_PARAM_RATE_11M;
		break;

		default:
		return AT_ERROR;
        }
    }else if(eWifiTestMode == WIFI_TEST_MODE_80211G_ONLY || eWifiTestMode == WIFI_TEST_MODE_80211A_ONLY) {
	int num = 0;
	tmp = rate + 1;
	at_tok_nextint(&tmp, &num);

	LOGATCI(LOG_DEBUG, "G OR A Mode num is %d\n", num);
        switch(num) {
        case 6:
            u4RateCode = RF_AT_PARAM_RATE_6M;
            break;

        case 9:
            u4RateCode = RF_AT_PARAM_RATE_9M;
            break;

        case 12:
            u4RateCode = RF_AT_PARAM_RATE_12M;
            break;

        case 18:
            u4RateCode = RF_AT_PARAM_RATE_18M;
            break;

        case 24:
            u4RateCode = RF_AT_PARAM_RATE_24M;
            break;

        case 36:
            u4RateCode = RF_AT_PARAM_RATE_36M;
            break;

        case 48:
            u4RateCode = RF_AT_PARAM_RATE_48M;
            break;

        case 54:
            u4RateCode = RF_AT_PARAM_RATE_54M;
            break;

        default:
            return AT_ERROR;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && u4Bandwidth == WIFI_TEST_BW_20MHZ) {
       	int num = 0;
       	tmp = rate + 3;
	at_tok_nextint(&tmp, &num);

	//check the greenfield
	if('G' == rate[1])
		u4Preamble = RF_AT_PREAMBLE_11N_GF;

        if('S' == rate[2])
      		eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;

	LOGATCI(LOG_DEBUG, "N mode num is %d\n", num);
        switch(num) {
        case 0:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            break;
        case 1:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            break;
        case 2:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            break;
        case 3:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            break;
        case 4:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            break;
        case 5:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            break;
        case 6:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            break;
        case 7:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            break;
        default:
            return AT_ERROR;
        }
    }
	else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && u4Bandwidth == WIFI_TEST_BW_40MHZ){
       	int num = 0;
       	tmp = rate + 4;
		at_tok_nextint(&tmp, &num);

		if('N' != rate[1])
			return AT_ERROR;

		//check the greenfield
		if('G' == rate[2])
			u4Preamble = RF_AT_PREAMBLE_11N_GF;

        if('S' == rate[3])
      		eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;

		LOGATCI(LOG_DEBUG, "N mode num is %d\n", num);
        switch(num) {
        case 0:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            break;
        case 1:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            break;
        case 2:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            break;
        case 3:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            break;
        case 4:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            break;
        case 5:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            break;
        case 6:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            break;
        case 7:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            break;
        default:
            return AT_ERROR;
        }
    }
/*    else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && eWifiBw == WIFI_TEST_BW_40MHZ) {
        switch(u4Rate) {
        case 1350:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 1500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 2700:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 3000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 4050:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 4500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5400:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 6000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 8100:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 9000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 10800:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 12000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 12150:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 13500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 15000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        default:
            return false;
        }
    }*/
    else {
        return AT_ERROR;
    }

	/*TODO: set Green field*/
	if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY &&
		WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
			u4Preamble,
			NULL,
			NULL) != 0) {
		return AT_ERROR;
	}
    gPreamble = u4Preamble;

	//if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && u4Bandwidth == WIFI_TEST_BW_40MHZ)
	{
		if(WIFI_TEST_set(RF_AT_FUNCID_BANDWIDTH, u4Bandwidth, NULL, NULL) == 0)
		{
			LOGATCI(LOG_DEBUG, "[gWifiBw =%d] \n", u4Bandwidth);
		}
		else 
		{
			LOGATCI(LOG_DEBUG, "[gWifiBw Error] \n");
			return AT_ERROR;
		}
	}
	gWifiBw = u4Bandwidth;

	/* set GI type */
	if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY &&
		WIFI_TEST_set(RF_AT_FUNCID_GI,
			eGIType,
			NULL,
			NULL) != 0) {
		return AT_ERROR;
	}
    gGIType = eGIType;
    gWifiTestMode = eWifiTestMode;

	/* set rate code */
	if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
			u4RateCode,
			NULL,
			NULL) == 0) {
		gRateCode = u4RateCode;
        LOGATCI(LOG_DEBUG, "[gRateCode =%d] \n", gRateCode);
		return AT_OK;
	}
	else {
		return AT_ERROR;
	}

	return ret;
}

ATRESPONSE_t pas_witestmode_handler(char* cmdline, ATOP_t opType, char* response) {
	bool err = false;
	ATRESPONSE_t ret = AT_ERROR;

	LOGATCI(LOG_DEBUG, "pas_witestmode_handler is %s\n", cmdline);
	switch(opType){
		case AT_ACTION_OP:
		case AT_READ_OP:
            //strcpy(response, prevRate);
            sprintf(response, "%c%s%c",0x02,prevRate,0x03);
			ret = AT_OK;
			break;

		case AT_SET_OP:
			if(cmdline[0] == '0'){
				if (true == WIFI_TEST_CloseDUT()){
                   //strcpy(response, "WLAN OFF");
                   sprintf(response, "%cWLAN OFF%c",0x02, 0x03);
					strcpy(prevRate, response);
					gRateCode = 0;
                    gPreamble = 0;
                    gGIType = 0;
                    gWifiTestMode = 0;
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					strcpy(prevRate, response);
					ret = AT_ERROR;
				}
			}else{
				char *pValue = cmdline;

				if(AT_OK == pas_wisetrate(pValue)){
                    //sprintf(response, "WLAN ON %s", pValue);
                    sprintf(response, "%cWLAN ON %s%c",0x02, pValue, 0x03);
					strcpy(prevRate, response);
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					strcpy(prevRate, response);
					ret = AT_ERROR;
				}
			}
			break;
		default:
			break;
	}

	LOGATCI(LOG_DEBUG, "response %s\n", response);
	return ret;
}

char prevTxChannel[15] = {'\0', '\0', '\0', '\0', '\0',
			'\0', '\0', '\0', '\0', '\0',
			'\0', '\0', '\0', '\0', '\0'};
ATRESPONSE_t pas_witx2_handler(char* cmdline, ATOP_t opType, char* response) {
	bool err = false;
	ATRESPONSE_t ret = AT_ERROR;

	LOGATCI(LOG_DEBUG, "pas_witx2_handler is %s\n", cmdline);
	switch(opType){
		case AT_ACTION_OP:
		case AT_READ_OP:
            //strcpy(response, prevTxChannel);
            sprintf(response,"%c%s%c",0x02,prevTxChannel,0x03);
			ret = AT_OK;
			break;

		case AT_SET_OP:
		{
			int channel = 0;
			at_tok_nextint(&cmdline, &channel);

			LOGATCI(LOG_DEBUG, "pas_witx2_handler channel is %d\n", channel);
			if(0 == channel){
				if(true == LGE_RFT_TxStop()){
                    //strcpy(response, "Tx Test STOP");
                    sprintf(response, "%cTx Test STOP%c",0x02,0x03);
					strcpy(prevTxChannel, "0");
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					strcpy(prevTxChannel, response);
					ret = AT_ERROR;
				}
				LGE_RFT_CloseDUT();
			}else{
				LGE_RFT_OpenDUT();
#ifdef MTK_WLAN_SUPPORT
                LOGATCI(LOG_DEBUG, "pas_witx2_handler [TestMode=%d] \n", gWifiTestMode);
                LOGATCI(LOG_DEBUG, "pas_witx2_handler [Tx rate=%d] \n", gRateCode);
                LOGATCI(LOG_DEBUG, "pas_witx2_handler [Preamble=%d] \n", gPreamble);
                LOGATCI(LOG_DEBUG, "pas_witx2_handler [GI Type=%d] \n", gGIType);

                if(gWifiTestMode == WIFI_TEST_MODE_80211N_ONLY &&
                    WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                        gPreamble,
                        NULL,
                        NULL) != 0) {
                    LOGATCI(LOG_DEBUG, "pas_witx2_handler RF_AT_FUNCID_PREAMBLE Failed\n");
                }

                //if(gWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && gWifiBw == WIFI_TEST_BW_40MHZ)
                {
                    if(WIFI_TEST_set(RF_AT_FUNCID_BANDWIDTH, gWifiBw, NULL, NULL) != 0) 
                    {
	                    LOGATCI(LOG_DEBUG, "pas_witx2_handler RF_AT_FUNCID_PREAMBLE Failed\n");
                	}
                }
                
                /* set GI type */
                if(gWifiTestMode == WIFI_TEST_MODE_80211N_ONLY &&
                    WIFI_TEST_set(RF_AT_FUNCID_GI,
                    gGIType,
                    NULL,
                    NULL) != 0) {
                    LOGATCI(LOG_DEBUG, "pas_witx2_handler RF_AT_FUNCID_GI Failed\n");
                }

	            if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
			        gRateCode,
			        NULL,
			        NULL) == 0) {

                LOGATCI(LOG_DEBUG, "pas_witx2_handler WIFI_TEST_set RF_AT_FUNCID_RATE success.\n");

                }
	            else {
                    LOGATCI(LOG_DEBUG, "pas_witx2_handler WIFI_TEST_set RF_AT_FUNCID_RATE failed \n");
	            }
#endif

				if(true == LGE_RFT_Channel(channel) &&
				true == LGE_RFT_TxStart()){
                    //sprintf(response, "\"%d\"\", channel);
                    sprintf(response, "%c\"%d\"%c",0x02,channel, 0x03);
					strcpy(prevTxChannel, response);
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					strcpy(prevTxChannel, response);
					LGE_RFT_CloseDUT();
					ret = AT_ERROR;
				}

			}

			break;
		}
		default:

			break;
	}

	LOGATCI(LOG_DEBUG,"response %s\n", response);
	return ret;
}

char prevRxChannel[15] = {'\0', '\0', '\0', '\0', '\0',
			'\0', '\0', '\0', '\0', '\0',
			'\0', '\0', '\0', '\0', '\0'};
struct pkteng_rx  tmp_pkteng_rx;

ATRESPONSE_t pas_wirx2_handler(char* cmdline, ATOP_t opType, char* response) {
	bool err = false;
	ATRESPONSE_t ret = AT_ERROR;

	LOGATCI(LOG_DEBUG,"pas_wirx2_handler is %s\n", cmdline);

	switch(opType){
		case  AT_TEST_OP:
            //strcpy(response, "Channel Range : 1~13\n PER Range : 0.0 ~ 100.0");
            sprintf(response, "%cChannel Range : 1~13\n PER Range : 0.0 ~ 100.0%c",0x02,0x03);
			ret = AT_OK;
			break;

		case AT_ACTION_OP:
		case AT_READ_OP:
			strcpy(response, prevRxChannel);
			ret = AT_OK;
			break;

		case AT_SET_OP:
			if('"' == cmdline[0]){
				int channel = 0;
				char *tmp = cmdline + 1;
				at_tok_nextint(&tmp, &channel);
				LGE_RFT_OpenDUT();
				if(true == LGE_RFT_Channel(channel) &&
				true == LGE_RFT_RxStart(&tmp_pkteng_rx)){
                    //sprintf(response, "\"%d\"\", channel);
                    sprintf(response, "%c\"%d\"%c",0x02,channel, 0x03);
					strcpy(prevRxChannel, response);
                    gLastErrorPkg = 0;
                    gLastGoodpkg = 0;
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					strcpy(prevRxChannel, response);
					ret = AT_ERROR;
				}
			}else if('1' == cmdline[0]){
				int numError = 0, numGood  = 0;
                int numErrorCnt = 0, numGoodCnt = 0;;

				if(true == LGE_RFT_FRError(&numError) &&
				true == LGE_RFT_FRGood(&numGood)){
				LOGATCI(LOG_DEBUG,"numError %d,numGood %d\n", numError,numGood);
                LOGATCI(LOG_DEBUG, "gLastErrorPkg %d,gLastGoodpkg %d\n", gLastErrorPkg, gLastGoodpkg);
				    if(numError >= gLastErrorPkg)
                    {
				        numErrorCnt = numError - gLastErrorPkg;
                        gLastErrorPkg = numError;
                    }
                    if(numGood >= gLastGoodpkg)
                    {
				        numGoodCnt = numGood - gLastGoodpkg;
                        gLastGoodpkg = numGood;
                    }
                    LOGATCI(LOG_DEBUG, "numErrorCnt %d,numGoodCnt %d\n", numErrorCnt,numGoodCnt);

					int total = numErrorCnt + numGoodCnt;
					float per = 100 * numErrorCnt/ (numErrorCnt + numGoodCnt);
                    //sprintf(response, "\0x02\"%d\"\, \"%3.2f\"\0x03", numGoodCnt, per);
                    sprintf(response, "%c\"%d\"\, \"%3.2f\"%c", 0x02,numGoodCnt, per,0x03);
					ret = AT_OK;
				}else{
                    //strcpy(response, "0");
                    sprintf(response, "%c0%c",0x02, 0x03);
					ret = AT_ERROR;
				}

			}else if('0' == cmdline[0]){
				if(true == LGE_RFT_RxStop(&tmp_pkteng_rx)){
                    //strcpy(response, "Rx Test STOP");
                    sprintf(response, "%cRx Test STOP%c", 0x02,0x03);
                    gLastErrorPkg = 0;
                    gLastGoodpkg = 0;
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					ret = AT_ERROR;
				}
 				LGE_RFT_CloseDUT();
			}else if('2' == cmdline[0]){
             //strcpy(response, "Clear OK");
             sprintf(response, "%cClear OK%c",0x02, 0x03);
				ret = AT_OK;
			}else if('3' == cmdline[0]){
				int rssi = 0;

				if(true == LGE_RFT_RSSI(&rssi)){
                    rssi &= 0x7F;
                    //sprintf(response, "\"-%d\"", rssi);
                    sprintf(response, "%c\"-%d\"%c",0x02, rssi, 0x03);
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					ret = AT_ERROR;
				}
			}else
				ret = AT_ERROR;
			break;
		default:

			break;
	}

	LOGATCI(LOG_DEBUG,"response %s\n", response);

	return ret;
}

static bool convertMac(char *start, unsigned char *mac)
{
	unsigned char  tmpupperMac = 0, tmplowerMac= 0, tmpMac = 0;
	long tmp1 = 0, tmp2 = 0;
	int i = 0, count = 0, tmpmaccount = 0;
	char *p = NULL;

	LOGATCI(LOG_DEBUG, "convertMac %s\n", start);

	/*for(i = 0; i < 24; i++){
		if(start[i] == ',')
			start[i] = '\0';
	}*/

#if 1
	for(i = 0; i < 24; i++)
	{
	        if(('a' <= start[i]) && (start[i] <= 'f'))
		{
			tmpMac = start[i]-'a' + 10;
		}
		else if(('A' <= start[i]) && (start[i] <= 'F'))
		{
			tmpMac = start[i]-'A' + 10;
		}
		else if(('0' <= start[i]) && (start[i] <= '9'))
		{
			tmpMac = start[i]-'0';
		}
		else if((start[i] == ' ') || (start[i] == ','))
		{
			tmpMac = 0;
			continue;
		}
		else
		{
			return -1;
		}

		if(tmpmaccount % 2 == 0)
            	{
			tmpupperMac = tmpMac;
		}
		else
		{
			tmplowerMac = tmpMac;
			mac[tmpmaccount / 2] = (((tmpupperMac <<  4) & 0xF0) | (tmplowerMac & 0x0F));
		}

		tmpmaccount++;

		if(tmpmaccount >=12)
		{
			LOGATCI(LOG_DEBUG, "mac is %02X%02X%02X%02X%02X%02X\n",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			return true;
		}
	}
	return -1;
#else
	for(i = 0; i < 24; ){
		tmp1 = strtol(start + i, &p, 16);
		if(LONG_MAX == tmp1)

			return false;
		LOGATCI(LOG_DEBUG, "convertMac %s %x\n", start + i, tmp1);

		if(tmp1 > 0)
		continue;

		i++;

		tmp2 = strtol(start + i, &p, 16);
		if( LONG_MAX == tmp2)
			return false;
		LOGATCI(LOG_DEBUG, "convertMac %s  %x\n", start + i, tmp2);

		i++;
		tmpMac = (tmp1 << 4) + tmp2;
		mac[i/4] = tmpMac;
		LOGATCI(LOG_DEBUG, "mac[%x] is %x", i/4, tmpMac);

		i++;
	}
	LOGATCI(LOG_DEBUG, "mac is %02X%02X%02X%02X%02X%02X\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return true;
#endif
}

ATRESPONSE_t pas_wimac_handler(char* cmdline, ATOP_t opType, char* response) {
	bool err = false;
	ATRESPONSE_t ret = AT_ERROR;

	LOGATCI(LOG_DEBUG,"pas_wimac_handler is %s\n", cmdline);

	switch(opType){
		case  AT_TEST_OP:
            //strcpy(response, "AT\%MAC=[MAC ADDR : 12 HEX nibble => 6 Bytes]");
            sprintf(response,"%cAT\%MAC=[MAC ADDR : 12 HEX nibble => 6 Bytes]%c",0x02,0x03);
			ret = AT_OK;
			break;

		case AT_ACTION_OP:
		case AT_READ_OP:
			{
				unsigned char mac[6];

				WIFI_TEST_OpenDUT();
				if(true == LGE_RFT_GetMACAddr(mac)){
                    //sprintf(response, "%02X%02X%02X%02X%02X%02X",
                    //mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    sprintf(response, "%c%02X%02X%02X%02X%02X%02X%c",
                    0x02,mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],0x03);
					LOGATCI(LOG_DEBUG, "response %s\n", response);
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					ret = AT_ERROR;
				}
				WIFI_TEST_CloseDUT();

				break;
			}

		case AT_SET_OP:
			{
				unsigned char mac[6];

				if(false == convertMac(cmdline, mac)){
                    //strcpy(response, "Error Mac Address");
                    sprintf(response,"%cMAC FORMAT ERROR%c",0x02,0x03);
					return AT_ERROR;
				}

				WIFI_TEST_OpenDUT();
				if(true == LGE_RFT_SetMACAddr(mac)){
                    //strcpy(response, "MAC ADDRESS WRITE OK");
                    sprintf(response,"%cMAC ADDRESS WRITE OK%c",0x02,0x03);
					ret = AT_OK;
				}else{
                     //strcpy(response, "0");
                     sprintf(response, "%c0%c",0x02, 0x03);
					ret = AT_ERROR;
				}
				LOGATCI(LOG_DEBUG,"response %s\n", response);
				WIFI_TEST_CloseDUT();

				break;
			}
		default:

			break;
	}

	return ret;
}

ATRESPONSE_t pas_wimacck_handler(char* cmdline, ATOP_t opType, char* response) {
	bool err = false;
	ATRESPONSE_t ret = AT_ERROR;

	LOGATCI(LOG_DEBUG,"pas_wimacck_handler is %s\n", cmdline);

	switch(opType){
		case AT_TEST_OP:
		case AT_ACTION_OP:
		case AT_READ_OP:
		case AT_SET_OP:
			//hardcode 1 COB
             //strcpy(response, "1");
             sprintf(response, "%c1%c",0x02, 0x03);
			LOGATCI(LOG_DEBUG, "response %s\n", response);
			ret = AT_OK;

		default:
			break;
	}

	return ret;


}
#endif
