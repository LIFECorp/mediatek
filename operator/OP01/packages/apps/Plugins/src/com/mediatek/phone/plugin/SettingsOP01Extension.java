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

package com.mediatek.phone.plugin;

import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.phone.ext.SettingsExtension;

public class SettingsOP01Extension extends SettingsExtension {

    private static final String LOG_TAG = "SettingsOP01Extension";
    public static final String BUTTON_NETWORK_MODE_KEY = "gsm_umts_preferred_network_mode_key";//single sim
    public static final String BUTTON_PLMN_LIST = "button_plmn_key";

    public static final String BUTTON_NETWORK_MODE_EX_KEY = "button_network_mode_ex_key";//dual sim
    public static final String BUTTON_PREFERED_NETWORK_MODE = "preferred_network_mode_key";
    public static final String BUTTON_3G_SERVICE = "button_3g_service_key";
    public static final String BUTTON_2G_ONLY = "button_prefer_2g_key";

    public void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    public void customizeFeatureForOperator(PreferenceScreen prefSet,
        Preference mPreferredNetworkMode, Preference mButtonPreferencedNetworkMode,
        Preference mButtonPreferredGSMOnly) {
        prefSet.removePreference(mPreferredNetworkMode);
        prefSet.removePreference(mButtonPreferencedNetworkMode);
        prefSet.removePreference(mButtonPreferredGSMOnly);
        log("customizeFeatureForOperator");
    }

    public void customizeFeatureForOperator(PreferenceScreen prefSet) {
        log("customizeFeatureForOperator only one in value");
        Preference buttonPreferredNetworkModeEx = prefSet.findPreference(BUTTON_NETWORK_MODE_EX_KEY);
        ListPreference listPreferredNetworkMode = (ListPreference) prefSet.findPreference(BUTTON_PREFERED_NETWORK_MODE);
        Preference buttonPreferredGSMOnly = (CheckBoxPreference) prefSet.findPreference(BUTTON_2G_ONLY);
        ListPreference listgsmumtsPreferredNetworkMode = (ListPreference) prefSet.findPreference(BUTTON_NETWORK_MODE_KEY);
        if (buttonPreferredNetworkModeEx != null) {
            log("button_network_mode_ex_key");
            prefSet.removePreference(buttonPreferredNetworkModeEx);
        }
        if (listPreferredNetworkMode != null) {
            log("preferred_network_mode_key");
            prefSet.removePreference(listPreferredNetworkMode);
        }
        if (buttonPreferredGSMOnly != null) {
            log("button_prefer_2g_key");
            prefSet.removePreference(buttonPreferredGSMOnly);
        }
        if (listgsmumtsPreferredNetworkMode != null) {
            log("gsm_umts_preferred_network_mode_key");
            prefSet.removePreference(listgsmumtsPreferredNetworkMode);
        }
    }

    public void customizePLMNFeature(PreferenceScreen prefSet, Preference mPLMNPreference) {
        log("customizePLMNFeature");
        if (FeatureOption.MTK_CTA_SUPPORT) {
            log("customizePLMNFeature: FeatureOption.MTK_CTA_SUPPORT");
            prefSet.addPreference(mPLMNPreference);
        }
    }

    /**
     * for change feature ALPS00783794 add "removeNMOpFor3GSwitch" funtion,
     * should remove
     * @param prefsc
     * @param networkMode
     */
    public void removeNMOpFor3GSwitch(PreferenceScreen prefsc, Preference networkMode) {
        prefsc.removePreference(networkMode);
        log("removeNMOpFor3GSwitch");
    }

    /**
     * for change feature ALPS00783794 add "remove 3g switch off radio" funtion,
     * should remove
     */
    public boolean isRemoveRadioOffFor3GSwitchFlag() {
         return true;
    }
}
