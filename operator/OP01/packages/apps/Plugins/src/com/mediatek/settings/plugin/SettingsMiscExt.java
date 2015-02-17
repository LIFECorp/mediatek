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
package com.mediatek.settings.plugin;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.preference.PreferenceActivity.Header;
import android.provider.Settings;
import android.provider.Telephony;
import android.os.UserManager;
import android.util.Log;

import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;

import com.mediatek.settings.ext.DefaultSettingsMiscExt;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op01.plugin.R;
import com.mediatek.telephony.SimInfoManager;

public class SettingsMiscExt extends DefaultSettingsMiscExt {
    
    private static final String TAB_SIM_1 = "sim1";
    private static final String TAB_SIM_2 = "sim2";
    private static final String TAB_MOBILE = "mobile";
    private static final String GSETTINGS_PROVIDER = "com.google.settings";
    private static final String TAG = "SettingsMiscExt";
    private Context mContext;
    private Preference mAgpsEnterPref;

    public SettingsMiscExt(Context context) {
        super(context);
        mContext = context;
        Log.d(TAG,"SettingsMiscExt");
    }

    private Preference.OnPreferenceClickListener mPreferenceclickListener = new Preference.OnPreferenceClickListener() {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            Intent intent = new Intent("com.mediatek.settings.launch_agps_setting_enter");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(intent);
            return true;
        }
    };

    /*  Used in AppWidget.
     * From CMCC requirement, in airplane mode disable wifi toggle in Widget.
     */
    public boolean isWifiToggleCouldDisabled(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, 0) == 0;
    }
    /*  Used in Tethering settings.
     * From CMCC requirement, the string is specified. 
     */
    
    public String getTetherWifiSSID(Context ctx) {
        return ctx.getString(R.string.wifi_tether_configure_ssid_default_for_cmcc);
    }

    public void setFactoryResetTitle(Object obj) {
        if (mContext.getPackageManager().resolveContentProvider(GSETTINGS_PROVIDER, 0) == null) {
            String title = mContext.getString(R.string.privacy_settings);
            if (obj instanceof Header) {
                Header header = (Header)obj;
                header.titleRes = 0;
                header.title = title;
                Log.d(TAG, "header.title = " + title);
            } else if (obj instanceof Activity) {
                Activity activity = (Activity)obj;
                activity.setTitle(title);
                Log.d(TAG, "activity.title = " + title);
           }
        }
    }

    public void initCustomizedLocationSettings(PreferenceScreen root, int order) {
        mAgpsEnterPref = new Preference(mContext);
        mAgpsEnterPref.setTitle(R.string.gps_settings_title);
        mAgpsEnterPref.setOnPreferenceClickListener(mPreferenceclickListener);
        mAgpsEnterPref.setOrder(order);
        mAgpsEnterPref.setEnabled(false);
        root.addPreference(mAgpsEnterPref);   
    }

    public void updateCustomizedLocationSettings() {
        int mode = Settings.Secure.getInt(getContentResolver(), Settings.Secure.LOCATION_MODE,
                Settings.Secure.LOCATION_MODE_OFF);
        final UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        boolean restricted =um.hasUserRestriction(UserManager.DISALLOW_SHARE_LOCATION);
        boolean enabled = false;
        if ((mode == Settings.Secure.LOCATION_MODE_HIGH_ACCURACY)
            || (mode == Settings.Secure.LOCATION_MODE_SENSORS_ONLY)) {
            enabled = true;
        }

        mAgpsEnterPref.setEnabled(enabled && !restricted);
    }

}
