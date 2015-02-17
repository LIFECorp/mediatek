/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.telephony;

import static android.Manifest.permission.READ_PHONE_STATE;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.Rlog;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.gemini.MTKPhoneFactory;
import com.android.internal.telephony.gsm.LteDcPhone;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.worldphone.LteModemSwitchHandler;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.telephony.WorldPhoneUtil;

/**
 *@hide
 */
public class LteWorldPhoneOp01 extends Handler implements IWorldPhone {
    private static Object sLock = new Object();
    private static Context sContext;
    private static Phone sPhone;
    private static Phone[] sGsmPhone;
    private static Phone[] sLtePhone;
    private static String sOperatorSpec;
    private static String sPlmnSs;
    private static String[] sImsi;
    private static String[] sGsmPlmnStrings;
    private static int sRilRadioTechnology;
    private static int sRegState;
    private static int sState;
    private static int sUserType;
    private static int sRegion;
    private static int sDenyReason;
    private static int sSuspendId;
    private static int s3gSimSlot;
    private static int[] sIccCardType;
    private static boolean sWaitForDesignateService;
    private static boolean[] sSuspendWaitImsi;
    private static boolean[] sFirstSelect;
    private static CommandsInterface[] sGsmCi;
    private static CommandsInterface[] sLteCi;
    private static UiccController[] sUiccController;
    private static IccRecords[] sIccRecordsInstance;
    private static ServiceState sServiceState2g;
    private static ServiceState sServiceState3_4g;
    private static LteModemSwitchHandler sLteModemSwitchHandler;
    private static final boolean sIsLteDcSupport = PhoneFactory.isLteDcSupport();
    private static final String[] MCCMNC_TABLE_TYPE1 = {
        "46000", "46002", "46007",
        // Lab test IMSI
        "00101", "00211", "00321", "00431", "00541", "00651",
        "00761", "00871", "00902", "01012", "01122", "01232",
        "46004", "46009", "46602", "50270", "46003"
    };
    private static final String[] MCCMNC_TABLE_TYPE3 = {
        "46001", "46006", "45407", "46005", "45502"
    };
    private static final String[] MCC_TABLE_DOMESTIC = {
        "460",
        // Lab test PLMN
        "001", "002", "003", "004", "005", "006",
        "007", "008", "009", "010", "011", "012"
    };
    private static int sTddStandByCounter;
    private static int sFddStandByCounter;
    private static boolean sWaitInTdd;
    private static boolean sWaitInFdd;
    private static final int[] FDD_STANDBY_TIMER = {
        60
    };
    private static final int[] TDD_STANDBY_TIMER = {
        0, 5, 10
    };
    
    public LteWorldPhoneOp01() {
        logd("Constructor invoked");
        sOperatorSpec = SystemProperties.get("ro.operator.optr", NO_OP);
        logd("Operator Spec:" + sOperatorSpec);
        LteModemSwitchHandler.setModemType(LteModemSwitchHandler.MD_TYPE_LTNG);
        logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));
        sPhone = MTKPhoneFactory.getDefaultPhone();
        sGsmPhone = new Phone[PhoneConstants.GEMINI_SIM_NUM];
        sGsmCi = new CommandsInterface[PhoneConstants.GEMINI_SIM_NUM];
        if (sIsLteDcSupport) {
            sLtePhone = new Phone[PhoneConstants.GEMINI_SIM_NUM];
            sLteCi = new CommandsInterface[PhoneConstants.GEMINI_SIM_NUM];
        }
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            logd("Gemini Project");
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                sGsmPhone[i] = ((PhoneProxy)(((GeminiPhone)sPhone).getPhonebyId(i))).getActivePhone();
                if (sIsLteDcSupport) {
                    sLtePhone[i] = MTKPhoneFactory.getLteDcPhone();
                }
            }
        } else {
            logd("Single Card Project");
            sGsmPhone[0] = ((PhoneProxy)sPhone).getActivePhone();
            if (sIsLteDcSupport) {
                sLtePhone[0] = MTKPhoneFactory.getLteDcPhone();
            }
        }
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (sIsLteDcSupport) {
                sLteCi[i] = ((PhoneBase)sLtePhone[i]).mCi;
                sLteCi[i].setOnPlmnChangeNotification(this, EVENT_LTE_PLMN_CHANGED_1 + i, null);
                sLteCi[i].setOnGSMSuspended(this, EVENT_LTE_SUSPENDED_1 + i, null);
                sLteCi[i].registerForOn(this, EVENT_LTE_RADIO_ON_1 + i, null);
            }
            sGsmCi[i] = ((PhoneBase)sGsmPhone[i]).mCi;
            sGsmCi[i].setOnPlmnChangeNotification(this, EVENT_GSM_PLMN_CHANGED_1 + i, null);
            sGsmCi[i].setOnGSMSuspended(this, EVENT_GSM_SUSPENDED_1 + i, null);
            sGsmCi[i].registerForOn(this, EVENT_RADIO_ON_1 + i, null);
        }

        sLteModemSwitchHandler = new LteModemSwitchHandler(sGsmCi[0]);

        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        if (sIsLteDcSupport) {
            intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED_LTE_DC);
        }
        if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
            logd("3G Switch Supported");
            intentFilter.addAction(TelephonyIntents.EVENT_3G_SWITCH_DONE);
        } else {
            logd("3G Switch Not Supported");
        }
        sContext = sPhone.getContext();
        sContext.registerReceiver(mReceiver, intentFilter);
        
        sUiccController = new UiccController[PhoneConstants.GEMINI_SIM_NUM];
        sIccRecordsInstance = new IccRecords[PhoneConstants.GEMINI_SIM_NUM];
        sImsi = new String[PhoneConstants.GEMINI_SIM_NUM];
        sIccCardType = new int[PhoneConstants.GEMINI_SIM_NUM];
        sSuspendWaitImsi = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        sFirstSelect = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        resetAllProperties();
        sTddStandByCounter = 0;
        sFddStandByCounter = 0;
        sWaitInTdd = false;
        sWaitInFdd = false;
        sRegion = REGION_UNKNOWN;
        if (Settings.Global.getInt(sContext.getContentResolver(), Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 1) == 0) {
            logd("Auto select disable");
            s3gSimSlot = AUTO_SELECT_DISABLE;
        } else {
            logd("Auto select enable");
        }
    }

    private boolean isAllowCampOn(String plmnString, int slotId) {
        logd("[isAllowCampOn]+ " + plmnString);
        logd("Slot" + slotId + " is 3G SIM");
        logd("User type:" + sUserType);
        sRegion = getRegion(plmnString);
        if (sUserType == sType1User || sUserType == sType2User) {
            sWaitForDesignateService = true;
            if (sRegion == REGION_DOMESTIC) {
                if (LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LTNG) {
                    sDenyReason = CAMP_ON_NOT_DENIED;
                    sWaitForDesignateService = false;
                    removeModemStandByTimer();
                    logd("TDD modem, stop searching TD service");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                } else {
                    sDenyReason = DENY_CAMP_ON_REASON_DOMESTIC_WCDMA;
                    removeModemStandByTimer();
                    setRatMode(SET_RAT_TO_2G, slotId);
                    logd("FDD modem, RAT=2g, expecting TD service");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                }
            } else if (sRegion == REGION_FOREIGN) {
                if (LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LTNG) {
                    sDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                    logd("TDD modem, expecting TD service");
                    logd("[isAllowCampOn]- Camp on REJECT");
                    return false;
                } else {
                    sDenyReason = CAMP_ON_NOT_DENIED;
                    removeModemStandByTimer();
                    setRatMode(SET_RAT_TO_AUTO, slotId);
                    logd("FDD modem, expecting TD service");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                }
            } else {
                logd("Unknow region");
            }
        } else if (sUserType == sType3User) {
            sWaitForDesignateService = false;
            if (LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LTNG) {
                // should not enter this state
                logd("Should not happen! TDD modem, Type3 user allow GSM full service");
                logd("[isAllowCampOn]- Camp on OK");
                return true;
            } else {
                sDenyReason = CAMP_ON_NOT_DENIED;
                logd("FDD modem, GSM full service or WCDMA limited service");
                logd("[isAllowCampOn]- Camp on OK");
                return true;
            }
        } else {
            logd("Unknown user type");
        }
        sWaitForDesignateService = true;
        sDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
        logd("[isAllowCampOn]- Camp on REJECT");
        return false;
    }

    private void handleNoService() {
        logd("[handleNoService]+ Can not find service");
        logd("Type" + sUserType + " user");
        logd(WorldPhoneUtil.regionToString(sRegion));
        if (sUserType == sType1User || sUserType == sType2User) {
            sWaitForDesignateService = true;
            if (LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LTNG) {
                // Set timer, switch to FDD modem when time up
                logd("TDD modem, expecting TD service");
                if (TDD_STANDBY_TIMER[sTddStandByCounter] >= 0) {
                    if (!sWaitInTdd) {
                        sWaitInTdd = true;
                        logd("Wait " + TDD_STANDBY_TIMER[sTddStandByCounter] + "s. Timer index = " + sTddStandByCounter);
                        postDelayed(mTddStandByTimerRunnable, TDD_STANDBY_TIMER[sTddStandByCounter]*1000);
                    } else {
                        logd("Timer already set:" + TDD_STANDBY_TIMER[sTddStandByCounter] +"s");
                    }
                } else {
                    logd("Standby in TDD modem");
                }
            } else {
                logd("FDD modem, expecting TD service");
                if (FDD_STANDBY_TIMER[sFddStandByCounter] >= 0) {
                    if (sRegion == REGION_FOREIGN) {
                        if (!sWaitInFdd) {
                            sWaitInFdd = true;
                            logd("Wait " + FDD_STANDBY_TIMER[sFddStandByCounter] + "s. Timer index = " + sFddStandByCounter);
                            postDelayed(mFddStandByTimerRunnable, FDD_STANDBY_TIMER[sFddStandByCounter]*1000);
                        } else {
                            logd("Timer already set:" + FDD_STANDBY_TIMER[sFddStandByCounter] +"s");
                        }
                    } else {
                        if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_USIM) {
                            handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTNG);
                        } else if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_SIM) {
                            handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTG);
                        }
                    }
                } else {
                    logd("Standby in FDD modem");
                }
            }
        } else if (sUserType == sType3User) {
            sWaitForDesignateService = false;
            if (LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LTNG) {
                // Should not enter this state
                logd("Should not happen! Type3 user, TDD modem");
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LWG);
            } else {
                logd("FDD modem -> standby in FDD modem");
            }
        } else {
            logd("Unknow user type");
        }
        logd("[handleNoService]-");
        return;
    }

    private Runnable mTddStandByTimerRunnable = new Runnable() {
        public void run() {
            sTddStandByCounter++;
            if (sTddStandByCounter >= TDD_STANDBY_TIMER.length) {
                sTddStandByCounter = TDD_STANDBY_TIMER.length-1;
            }
            logd("TDD time out!");
            handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LWG);
        }
    };

    private Runnable mFddStandByTimerRunnable = new Runnable() {
        public void run() {
            sFddStandByCounter++;
            if (sFddStandByCounter >= FDD_STANDBY_TIMER.length) {
                sFddStandByCounter = FDD_STANDBY_TIMER.length-1;
            }
            logd("FDD time out!");
            if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_USIM) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTNG);
            } else if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_SIM) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTG);
            }
        }
    };

    private void removeModemStandByTimer() {
        if (sWaitInTdd) {
            logd("Remove TDD wait timer. Set sWaitInTdd = false");
            sWaitInTdd = false;
            removeCallbacks(mTddStandByTimerRunnable);
        }
        if (sWaitInFdd) {
            logd("Remove FDD wait timer. Set sWaitInFdd = false");
            sWaitInFdd = false;
            removeCallbacks(mFddStandByTimerRunnable);
        }
    }

    private void searchForDesignateService(String strPlmn) {
        logd("[searchForDesignateService]+ Search for TD srvice");
        if (strPlmn == null) {
            logd("[searchForDesignateService]- null source");
            return;
        }
        strPlmn = strPlmn.substring(0, 5);
        for (String mccmnc : MCCMNC_TABLE_TYPE1) {
            if (strPlmn.equals(mccmnc)) {
                logd("sUserType:" + sUserType + " sRegion:" + sRegion);
                logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));
                logd("Find TD service");
                if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_USIM) {
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTNG);
                } else if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_SIM) {
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTG);
                }
                break;
            }    
        }
        logd("[searchForDesignateService]-");
        return;
    }
    
    public void handleMessage(Message msg) {
        AsyncResult ar = (AsyncResult)msg.obj;
        switch (msg.what) {
            case EVENT_GSM_PLMN_CHANGED_1:
                logd("handleMessage : <EVENT_GSM_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_GSM_SUSPENDED_1:
                logd("handleMessage : <EVENT_GSM_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_RADIO_ON_1:
                logd("handleMessage : <EVENT_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_LTE_PLMN_CHANGED_1:
                logd("handleMessage : <EVENT_LTE_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_LTE_SUSPENDED_1:
                logd("handleMessage : <EVENT_LTE_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_LTE_RADIO_ON_1:
                logd("handleMessage : <EVENT_LTE_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_GSM_PLMN_CHANGED_2:
                logd("handleMessage : <EVENT_GSM_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_GSM_SUSPENDED_2:
                logd("handleMessage : <EVENT_GSM_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_RADIO_ON_2:
                logd("handleMessage : <EVENT_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_LTE_PLMN_CHANGED_2:
                logd("handleMessage : <EVENT_LTE_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_LTE_SUSPENDED_2:
                logd("handleMessage : <EVENT_LTE_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_LTE_RADIO_ON_2:
                logd("handleMessage : <EVENT_LTE_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_SET_RAT_GSM_ONLY:
                logd("handleMessage : <EVENT_SET_RAT_GSM_ONLY>");
                if (ar.exception == null) {
                    logd("Set RAT=2g ok");
                } else {
                    logd("Set RAT=2g fail " + ar.exception);
                }
                break;
            case EVENT_SET_RAT_WCDMA_PREF:
                logd("handleMessage : <EVENT_SET_RAT_WCDMA_PREF>");
                if (ar.exception == null) {
                    logd("Set RAT=auto ok");
                } else {
                    logd("Set RAT=auto fail " + ar.exception);
                }
                break;
            default:
                logd("Unknown msg:" + msg.what);
        }
    }

    private void handleRadioOn(int slotId) {
        logd("Slot" + slotId);
        if (s3gSimSlot == UNKNOWN_3G_SLOT) {
            s3gSimSlot = get3gCapabilitySim();
        }
        if (s3gSimSlot == slotId) {
            if (sUserType == sType1User || sUserType == sType2User) {
                logd("Modem on, Type12 user");
                setRatMode(SET_RAT_TO_AUTO, slotId);
            } else if (sUserType == sType3User) {
                logd("Modem on, Type3 user");
                setRatMode(SET_RAT_TO_2G, slotId);
            } else {
                logd("Modem on, Unknown user");
                setRatMode(SET_RAT_TO_AUTO, slotId);
            }
        }
    }

    private void handlePlmnChange(AsyncResult ar, int slotId) {
        logd("Slot" + slotId);
        if (s3gSimSlot == UNKNOWN_3G_SLOT) {
            s3gSimSlot = get3gCapabilitySim();
        }
        if (ar.exception == null && ar.result != null) {
            sGsmPlmnStrings = (String[])ar.result;
            for (int i=0; i<sGsmPlmnStrings.length; i++) {
                logd("sGsmPlmnStrings[" + i + "]=" + sGsmPlmnStrings[i]);
            }
            if (s3gSimSlot == slotId && sWaitForDesignateService
                    && sDenyReason != DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD) {
                searchForDesignateService(sGsmPlmnStrings[0]);
            }
            // To speed up performance in foreign countries, once get PLMN(no matter which slot)
            // determine region right away and switch modem type if needed
            sRegion = getRegion(sGsmPlmnStrings[0]);
            if (s3gSimSlot != AUTO_SELECT_DISABLE && s3gSimSlot != NO_3G_CAPABILITY && 
                    sRegion == REGION_FOREIGN) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LWG);
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handlePlmnSuspend(AsyncResult ar, int slotId) {
        logd("Slot" + slotId);
        if (ar.exception == null && ar.result != null) {
            sSuspendId = ((int[]) ar.result)[0];
            logd("Suspending with Id=" + sSuspendId);
            if (s3gSimSlot == slotId) {
                if (sUserType != sUnknownUser) {
                    resumeCampingProcedure(slotId);
                } else {
                    sSuspendWaitImsi[slotId] = true;
                    logd("User type unknown, wait for IMSI");
                }
            } else {
                logd("Not 3G slot, camp on OK");
                if (sGsmCi[slotId].getRadioState().isOn()) {
                    sGsmCi[slotId].setResumeRegistration(sSuspendId, null);
                } else {
                    logd("Radio off or unavailable, can not send EMSR");
                }
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }
    
    private final BroadcastReceiver mReceiver = new  BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            logd("[BroadcastReceiver]+");
            String action = intent.getAction();
            logd("Action: " + action);
            int slotId;
            if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                logd("slotId: " + slotId + " simStatus: " + simStatus);
                if (simStatus.equals(IccCardConstants.INTENT_VALUE_ICC_IMSI)) {
                    if (s3gSimSlot == UNKNOWN_3G_SLOT) {
                        s3gSimSlot = get3gCapabilitySim();
                    }
                    //sIccRecordsInstance[slotId] = ((PhoneBase)sGsmCi[s3gSimSlot]).mIccRecords.get();
                    sUiccController[slotId] = UiccController.getInstance(slotId);
                    sIccRecordsInstance[slotId] = sUiccController[slotId].getIccRecords(UiccController.APP_FAM_3GPP);
                    sImsi[slotId] = sIccRecordsInstance[slotId].getIMSI();
                    sIccCardType[slotId] = getIccCardType(slotId);
                    logd("sImsi[" + slotId + "]:" + sImsi[slotId]);
                    if (slotId == s3gSimSlot) {
                        logd("3G slot");
                        sUserType = getUserType(sImsi[slotId]);
                        if (sFirstSelect[slotId]) {
                            sFirstSelect[slotId] = false;
                            if (sUserType == sType1User || sUserType == sType2User) {
                                sWaitForDesignateService = true;
                            } else if (sUserType == sType3User) {
                                sWaitForDesignateService = false;
                                logd("Type3 user, switch to FDD modem");
                                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LWG);
                            }
                        }
                        if (sSuspendWaitImsi[slotId]) {
                            sSuspendWaitImsi[slotId] = false;
                            logd("IMSI fot slot" + slotId + " now ready, resuming PLMN:" 
                                    + sGsmPlmnStrings[0] + " with ID:" + sSuspendId);
                            resumeCampingProcedure(slotId);
                        }
                    } else {
                        // not 3G slot, do not store into sUserType
                        getUserType(sImsi[slotId]);
                        logd("Not 3G slot");
                    }
                } else if (simStatus.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
                    sImsi[slotId] = "";
                    sFirstSelect[slotId] = true;
                    sSuspendWaitImsi[slotId] = false;
                    sIccCardType[slotId] = ICC_CARD_TYPE_UNKNOWN;
                    if (slotId == s3gSimSlot) {
                        logd("3G Sim removed, no world phone service");
                        sUserType = sUnknownUser;
                        sDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
                        sWaitForDesignateService = false;
                        s3gSimSlot = UNKNOWN_3G_SLOT;
                    } else {
                        logd("Slot" + slotId + " is not 3G slot");
                    }
                }
            } else if (action.equals(TelephonyIntents.EVENT_3G_SWITCH_DONE)) {
                if (s3gSimSlot != AUTO_SELECT_DISABLE) {
                    s3gSimSlot = intent.getIntExtra(TelephonyIntents.EXTRA_3G_SIM, 0);
                }
                handle3gSwitched();
            } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                sServiceState2g = ServiceState.newFromBundle(intent.getExtras());
                if (sServiceState2g != null) {
                    slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                    sPlmnSs = sServiceState2g.getOperatorNumeric();
                    sRilRadioTechnology = sServiceState2g.getRilVoiceRadioTechnology();
                    sRegState = sServiceState2g.getRegState();
                    sState = sServiceState2g.getState();
                    logd("slotId: " + slotId);
                    logd("s3gSimSlot=" + s3gSimSlot);
                    logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));
                    logd("isRoaming: " + sServiceState2g.getRoaming()
                            + " isEmergencyOnly: " + sServiceState2g.isEmergencyOnly());
                    logd("sPlmnSs: " + sPlmnSs);
                    logd("sState: " + WorldPhoneUtil.stateToString(sState));
                    logd("sRegState: " + WorldPhoneUtil.regStateToString(sRegState));
                    logd("sRilRadioTechnology: " + sServiceState2g.rilRadioTechnologyToString(sRilRadioTechnology));
                    if (slotId == s3gSimSlot && isNoService()) {
                        if (sIsLteDcSupport) {
                            if (LteModemSwitchHandler.getActiveModemType() == LteModemSwitchHandler.MD_TYPE_LTNG) {
                                logd("Check 3/4G service state");
                                sServiceState3_4g = sLtePhone[slotId].getServiceState();
                                if (sServiceState3_4g != null) {
                                    sPlmnSs = sServiceState3_4g.getOperatorNumeric();
                                    sRilRadioTechnology = sServiceState3_4g.getRilDataRadioTechnology();
                                    sRegState = sServiceState3_4g.getRegState();
                                    sState = sServiceState3_4g.getState();
                                    logd("isRoaming: " + sServiceState3_4g.getRoaming()
                                            + " isEmergencyOnly: " + sServiceState3_4g.isEmergencyOnly());
                                    logd("sPlmnSs: " + sPlmnSs);
                                    logd("sState: " + WorldPhoneUtil.stateToString(sState));
                                    logd("sRegState: " + WorldPhoneUtil.regStateToString(sRegState));
                                    logd("sRilRadioTechnology: " + sServiceState3_4g.rilRadioTechnologyToString(sRilRadioTechnology));
                                    if (isNoService()) {
                                        handleNoService();
                                    }
                                } else {
                                    logd("Null sServiceState3_4g");
                                }
                            } else {
                                handleNoService();
                            }
                        } else {
                            handleNoService();
                        }
                    }
                } else {
                    logd("Null sServiceState2g");
                }
            } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED_LTE_DC)) {
                sServiceState3_4g = ServiceState.newFromBundle(intent.getExtras());
                if (sServiceState3_4g != null) {
                    slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                    sPlmnSs = sServiceState3_4g.getOperatorNumeric();
                    sRilRadioTechnology = sServiceState3_4g.getRilDataRadioTechnology();
                    sRegState = sServiceState3_4g.getRegState();
                    sState = sServiceState3_4g.getState();
                    logd("slotId: " + slotId);
                    logd("s3gSimSlot=" + s3gSimSlot);
                    logd(LteModemSwitchHandler.modemToString(LteModemSwitchHandler.getActiveModemType()));
                    logd("isRoaming: " + sServiceState3_4g.getRoaming()
                            + " isEmergencyOnly: " + sServiceState3_4g.isEmergencyOnly());
                    logd("sPlmnSs: " + sPlmnSs);
                    logd("sState: " + WorldPhoneUtil.stateToString(sState));
                    logd("sRegState: " + WorldPhoneUtil.regStateToString(sRegState));
                    logd("sRilRadioTechnology: " + sServiceState3_4g.rilRadioTechnologyToString(sRilRadioTechnology));
                    if (slotId == s3gSimSlot && isNoService()) {
                        logd("Check 2G service state");
                        ServiceState sServiceState2g = sGsmPhone[slotId].getServiceState();
                        if (sServiceState2g != null) {
                            sPlmnSs = sServiceState2g.getOperatorNumeric();
                            sRilRadioTechnology = sServiceState2g.getRilVoiceRadioTechnology();
                            sRegState = sServiceState2g.getRegState();
                            sState = sServiceState2g.getState();
                            logd("isRoaming: " + sServiceState2g.getRoaming()
                                    + " isEmergencyOnly: " + sServiceState2g.isEmergencyOnly());
                            logd("sPlmnSs: " + sPlmnSs);
                            logd("sState: " + WorldPhoneUtil.stateToString(sState));
                            logd("sRegState: " + WorldPhoneUtil.regStateToString(sRegState));
                            logd("sRilRadioTechnology: " + sServiceState2g.rilRadioTechnologyToString(sRilRadioTechnology));
                            if (isNoService()) {
                                handleNoService();
                            }
                        } else {
                            logd("Null sServiceState2g");
                        }
                    }
                } else {
                    logd("Null sServiceState3_4g");
                }
            }
            logd("[BroadcastReceiver]-");
        }
    };

    private boolean isNoService() {
        if (sState == ServiceState.STATE_OUT_OF_SERVICE
                && sRegState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING) {
            return true;
        } else {
            return false;
        }
    }

    private void handle3gSwitched() {
        if (s3gSimSlot == NO_3G_CAPABILITY) {
            logd("3G capability turned off");
            removeModemStandByTimer();
            sUserType = sUnknownUser;
        } else if (s3gSimSlot == AUTO_SELECT_DISABLE) {
            logd("Auto Network Selection Disabled");
            removeModemStandByTimer();
        } else {
            logd("3G capability in slot" + s3gSimSlot);
            if (sImsi[s3gSimSlot].equals("")) {
                // may caused by receive 3g switched intent when boot up 
                logd("3G slot IMSI not ready");
                sUserType = sUnknownUser;
                return;
            }
            sIccCardType[s3gSimSlot] = getIccCardType(s3gSimSlot);
            sUserType = getUserType(sImsi[s3gSimSlot]);
            if (sUserType == sType1User || sUserType == sType2User) {
                sWaitForDesignateService = true;
            } else if (sUserType == sType3User) {
                sWaitForDesignateService = false;
            } else {
                logd("Unknown user type");
            }
            if (sGsmPlmnStrings != null) {
                sRegion = getRegion(sGsmPlmnStrings[0]);
            }
            if (sRegion != REGION_UNKNOWN && sUserType != sUnknownUser) {
                sFirstSelect[s3gSimSlot] = false;
                if (sUserType == sType3User || sRegion == REGION_FOREIGN) {
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LWG);
                } else {
                    if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_USIM) {
                        handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTNG);
                    } else if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_SIM) {
                        handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTG);
                    }
                }
            }
        }
    }

    public void setNetworkSelectionMode(int mode) {
        if (mode == SELECTION_MODE_AUTO) {
            logd("Network Selection <AUTO>");
            Settings.Global.putInt(sContext.getContentResolver(), Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 1);
            s3gSimSlot = get3gCapabilitySim();
        } else {
            logd("Network Selection <MANUAL>");
            Settings.Global.putInt(sContext.getContentResolver(), Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 0);
            s3gSimSlot = AUTO_SELECT_DISABLE;
            if (sUserType == sType1User || sUserType == sType2User) {
                setRatMode(SET_RAT_TO_AUTO, PhoneConstants.GEMINI_SIM_1);
            }
        }
        handle3gSwitched();
    }

    public static int getCurrentRegion() {
        logd("[getCurrentRegion] " + sRegion);
        return sRegion;
    }

    private void handleSwitchModem(int toModem) {
        if (toModem == LteModemSwitchHandler.getActiveModemType()) {
            if (toModem == LteModemSwitchHandler.MD_TYPE_LTNG) {
                logd("Already in MMDC modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LTG) {
                logd("Already in TDD CSFB modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LWG) {
                logd("Already in FDD CSFB modem");
            }
            return;
        } else {
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                if (sGsmPhone[i].getState() != PhoneConstants.State.IDLE) {
                    logd("Phone" + i + " is not idle, modem switch not allowed");
                    return;
                }
            }
            removeModemStandByTimer();
            if (toModem == LteModemSwitchHandler.MD_TYPE_LTNG) {
                logd("Switching to MMDC modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LTG) {
                logd("Switching to TDD CSFB modem");
            } else if (toModem == LteModemSwitchHandler.MD_TYPE_LWG) {
                logd("Switching to FDD CSFB modem");
            }
            LteModemSwitchHandler.switchModem(toModem);
            resetNetworkProperties();
        }
    }

    private void resumeCampingProcedure(int slotId) {
        logd("Resume camping slot" + slotId + " sSuspendId:" + sSuspendId);
        if (isAllowCampOn(sGsmPlmnStrings[0], slotId)) {
            if (sGsmCi[slotId].getRadioState().isOn()) {
                sGsmCi[slotId].setResumeRegistration(sSuspendId, null);
            } else {
                logd("Radio off or unavailable, can not send EMSR");
            }
        } else {
            logd("Because: " + WorldPhoneUtil.denyReasonToString(sDenyReason));
            if (sDenyReason == DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD) {
                handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LWG);
            } else if (sDenyReason == DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD) {
                if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_USIM) {
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTNG);
                } else if (sIccCardType[s3gSimSlot] == ICC_CARD_TYPE_SIM) {
                    handleSwitchModem(LteModemSwitchHandler.MD_TYPE_LTG);
                }
            }
        }
    }

    /* Might return -1 if 3G is off */
    private int get3gCapabilitySim() {
        int slot3g = sPhone.get3GCapabilitySIM();
        logd("s3gSimSlot=" + slot3g);

        return slot3g;
    }

    private int getIccCardType(int slotId) {
        int simType;
        String simString = sPhone.getIccCard().getIccCardType();

        if (simString.equals("SIM")) {
            logd("IccCard type: SIM");
            simType = ICC_CARD_TYPE_SIM;
        } else if (simString.equals("USIM")) {
            logd("IccCard type: USIM");
            simType = ICC_CARD_TYPE_USIM;
        } else {
            logd("IccCard type: Unknown");
            simType = ICC_CARD_TYPE_UNKNOWN;
        }

        return simType;
    }

    private void setRatMode(int ratMode, int slotId) {
        if (ratMode == SET_RAT_TO_AUTO) {
            logd("[setRatMode] Setting slot" + slotId + " RAT=auto");
            sGsmCi[slotId].setPreferredNetworkType(Phone.NT_MODE_WCDMA_PREF,
                obtainMessage(EVENT_SET_RAT_WCDMA_PREF));       
        } else {
            logd("[setRatMode] Setting slot" + slotId + " RAT=2G");
            sGsmCi[slotId].setPreferredNetworkType(Phone.NT_MODE_GSM_ONLY,
                obtainMessage(EVENT_SET_RAT_GSM_ONLY));
        }
    }

    private void resetAllProperties() {
        logd("[resetAllProperties]");
        sGsmPlmnStrings = null;
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            sFirstSelect[i] = true;
        }
        sWaitForDesignateService = false;
        sDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
        resetSimProperties();
        resetNetworkProperties();
    }
    
    private void resetNetworkProperties() {
        logd("[resetNetworkProperties]");
        synchronized (sLock) {        
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                sSuspendWaitImsi[i] = false;
            }
        }
    }

    private void resetSimProperties() {
        logd("[resetSimProperties]");
        synchronized (sLock) {        
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                sImsi[i] = "";
                sIccCardType[i] = ICC_CARD_TYPE_UNKNOWN;
            }
            sUserType = sUnknownUser;
            if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
                s3gSimSlot = UNKNOWN_3G_SLOT;
            } else {
                s3gSimSlot = DEFAULT_3G_SLOT;
            }
        }
    }

    private int getUserType(String sisImsi) {
        if (sisImsi != null && !sisImsi.equals("")) {
            sisImsi = sisImsi.substring(0, 5);
            logd("[getUserType] simPlmn:" + sisImsi);
            for (String mccmnc : MCCMNC_TABLE_TYPE1) {
                if (sisImsi.equals(mccmnc)) {
                    logd("[getUserType] Type1 user");
                    return sType1User;
                }    
            }
            for (String mccmnc : MCCMNC_TABLE_TYPE3) {
                if (sisImsi.equals(mccmnc)) {
                    logd("[getUserType] Type3 user");
                    return sType3User;
                }    
            }
            logd("[getUserType] Type2 user");
            return sType2User;
        } else {
            logd("[getUserType] null sisImsi");
            return sUnknownUser;
        }
    }

    private int getRegion(String srcMccOrPlmn) {
        String currentMcc;
        if (srcMccOrPlmn == null) {
            logd("[getRegion] null source");
            return REGION_UNKNOWN;
        }
        // Lab test PLMN 46602 & 50270 are Type1 & Domestic region
        // Other real world PLMN 466xx & 502xx are Type2 & Foreign region
        currentMcc = srcMccOrPlmn.substring(0, 5);
        if (currentMcc.equals("46602") || currentMcc.equals("50270")) {
            return REGION_DOMESTIC;
        }
        currentMcc = srcMccOrPlmn.substring(0, 3);
        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (currentMcc.equals(mcc)) {
                logd("[getRegion] REGION_DOMESTIC");
                return REGION_DOMESTIC;
            }    
        }
        logd("[getRegion] REGION_FOREIGN");
        return REGION_FOREIGN;
    }

    public void disposeWorldPhone() {
        sContext.unregisterReceiver(mReceiver);
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            sGsmCi[i].unSetOnPlmnChangeNotification(this);
            sGsmCi[i].unSetOnGSMSuspended(this);
            sGsmCi[i].unregisterForOn(this);
            if (sIsLteDcSupport) {
                sLteCi[i].unSetOnPlmnChangeNotification(this);
                sLteCi[i].unSetOnGSMSuspended(this);
                sLteCi[i].unregisterForOn(this);
            }
        }
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[LteWPOP01]" + msg);
    }
}
