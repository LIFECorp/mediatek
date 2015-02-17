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


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.agps.MtkAgpsManager;
import com.mediatek.common.agps.MtkAgpsProfile;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.HashMap;

public class AgpsReceiver extends BroadcastReceiver {

    private static final String TAG = "Settings/AgpsReceiver";
//    private static final String PREFERENCE_FILE = "com.android.settings_preferences";
/*
    public static final String ACTION_OMA_CP = "com.mediatek.omacp.settings";// add for omacp
    public static final String ACTION_OMA_CP_FEEDBACK = "com.mediatek.omacp.settings.result";
    public static final String ACTION_OMA_CP_CAPABILITY = "com.mediatek.omacp.capability";
    public static final String ACTION_OMA_CP_CAPABILITY_FEEDBACK = "com.mediatek.omacp.capability.result";
    public static final String APP_ID = "ap0004";
*/
 
    private MtkAgpsManager mAgpsMgr = null;


    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {

        mContext = context;

        String action = intent.getAction();
        log("onReceive action=" + action);

        if (FeatureOption.MTK_AGPS_APP && FeatureOption.MTK_GPS_SUPPORT) {

            // BroadcastReceiver will reset all of member after onReceive
            mAgpsMgr = (MtkAgpsManager) context.getSystemService(Context.MTK_AGPS_SERVICE);

            if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
                handleBootCompleted(context, intent);
            } else if (action.equals(MtkAgpsManager.AGPS_PROFILE_UPDATE)) {
                handleAgpsProfileUpdate(context, intent);
            } else if (action.equals(MtkAgpsManager.AGPS_STATUS_UPDATE)) {
                handleAgpsStatusUpdate(context, intent);
            } else if (action.equals(MtkAgpsManager.AGPS_DISABLE_UPDATE)) {
                handleAgpsDisableUpdate(context, intent);
            } 
        }
    }

    private void handleBootCompleted(Context context, Intent intent) {
        SharedPreferences prefs;

        prefs = context.getSharedPreferences("agps_disable", 0);
        boolean disableAfterReboot = false;
        if (prefs.getBoolean("changed", false)) {
            disableAfterReboot = prefs.getBoolean("status", false);
        }
        log("[agps_sky]boot complieted disableAfterReboot=" + disableAfterReboot);

        prefs = context.getSharedPreferences("agps_profile", 0);
        if (prefs.getBoolean("changed", false)) {
            MtkAgpsProfile profile = new MtkAgpsProfile();
            profile.name = prefs.getString("name", null);
            profile.addr = prefs.getString("addr", null);
            profile.backupSlpNameVar = prefs.getString("backupSlpNameVar", null);
            profile.port = prefs.getInt("port", 0);
            profile.tls = prefs.getInt("tls", 0);
            profile.showType = prefs.getInt("showType", 0);
            profile.code = prefs.getString("code", null);
            profile.addrType = prefs.getString("addrType", null);
            profile.providerId = prefs.getString("providerId", null);
            profile.defaultApn = prefs.getString("defaultApn", null);
            profile.optionApn = prefs.getString("optionApn", null);
            profile.optionApn2 = prefs.getString("optionApn2", null);
            profile.appId = prefs.getString("appId", null);

            mAgpsMgr.setProfile(profile);
        }

        prefs = context.getSharedPreferences("agps_status", 0);
        if (prefs.getBoolean("changed", false)) {
            log("[agps_sky]boot complieted need to check agps_status");
            boolean status = prefs.getBoolean("status", false);
            log("[agps_sky]boot complieted status = " + status + "disableAfterReboot = " + disableAfterReboot);
            if (status && !disableAfterReboot) {
                log("[agps_sky]boot complieted need to enable agps");
                mAgpsMgr.enable();
            } else {
                log("[agps_sky]boot complieted need to disable agps");
                mAgpsMgr.disable();
            }

            int roaming = prefs.getInt("roaming", 0);
            int molr = prefs.getInt("molrPositionType", 0);
            int niEnable = prefs.getInt("niEnable", 0);

            mAgpsMgr.setRoamingEnable((roaming == 0) ? false : true);
            mAgpsMgr.setUpEnable((molr == 0) ? true : false);
            mAgpsMgr.setNiEnable((niEnable == 0) ? false : true);

        } else {
            mAgpsMgr.extraCommand("USING_XML", null);
            if (!disableAfterReboot) {
                log("[agps_sky]boot complieted [agps status not changed] need to enable agps");
                mAgpsMgr.enable();
            } else {
                log("[agps_sky]boot complieted [agps status not changed] need to disable agps");
                mAgpsMgr.disable();
            }
        }
    }

    private void handleAgpsProfileUpdate(Context context, Intent intent) {
        Bundle bundle = intent.getExtras();
        String name = bundle.getString("name");
        String addr = bundle.getString("addr");
        String backup = bundle.getString("backupSlpNameVar");
        int port = bundle.getInt("port");
        int tls = bundle.getInt("tls");
        int showType = bundle.getInt("showType");
        String code = bundle.getString("code");
        String addrType = bundle.getString("addrType");
        String providerId = bundle.getString("providerId");
        String defaultApn = bundle.getString("defaultApn");
        String optionApn = bundle.getString("optionApn");
        String optionApn2 = bundle.getString("optionApn2");
        String appId = bundle.getString("appId");

        SharedPreferences prefs = context.getSharedPreferences("agps_profile", 0);
        prefs.edit().putString("name", name).putString("addr", addr).putString("backupSlpNameVar", backup)
                .putInt("port", port).putInt("tls", tls).putInt("showType", showType).putString("code", code)
                .putString("addrType", addrType).putString("providerId", providerId).putString("defaultApn", defaultApn)
                .putString("optionApn", optionApn).putString("optionApn2", optionApn2).putString("appId", appId)
                .putBoolean("changed", true).commit();
    }

    private void handleAgpsStatusUpdate(Context context, Intent intent) {
        Bundle bundle = intent.getExtras();
        boolean status = bundle.getBoolean("status", false);
        int roaming = bundle.getInt("roaming", 0);
        int molr = bundle.getInt("molrPositionType", 0);
        int niEnable = bundle.getInt("niEnable", 1);

        SharedPreferences prefs = context.getSharedPreferences("agps_status", 0);
        prefs.edit().putBoolean("status", status).putInt("roaming", roaming).putInt("molrPositionType", molr)
                .putInt("niEnable", niEnable).putBoolean("changed", true).commit();
    }

    private void handleAgpsDisableUpdate(Context context, Intent intent) {
        Bundle bundle = intent.getExtras();
        boolean status = bundle.getBoolean("status", false);

        SharedPreferences prefs = context.getSharedPreferences("agps_disable", 0);
        prefs.edit().putBoolean("status", status).putBoolean("changed", true).commit();
    }
    private void log(String info) {
        Xlog.d(TAG, info + " ");
    }
}
