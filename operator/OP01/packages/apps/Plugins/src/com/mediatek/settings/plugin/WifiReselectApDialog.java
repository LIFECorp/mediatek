/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.op01.plugin;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.provider.Settings;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import com.android.internal.telephony.TelephonyIntents;
import android.os.ServiceManager;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import android.os.RemoteException;
import android.os.UserHandle;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.wifi.IWifiFwkExt;

public class WifiReselectApDialog extends AlertActivity implements DialogInterface.OnClickListener {
    private static final String TAG = "WifiReselectApDialog";
    private Context mContext;
    private boolean mIsLastDataOn;
    private long mLastDataSimId;
    private ITelephony mService;
    private boolean mIsDone;
    private WifiManager mWifiManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);
        Log.d(TAG,"onCreate()");
        mContext = this;
        mWifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
        createDialog();
    }

    private void createDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_info;
        p.mTitle = getString(R.string.confirm_title);
        p.mView = createView();
        p.mViewSpacingSpecified=true;
        p.mViewSpacingLeft=15;
        p.mViewSpacingRight=15;
        p.mViewSpacingTop=5;
        p.mViewSpacingBottom=5;
        p.mPositiveButtonText = getString(android.R.string.yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.no);
        p.mNegativeButtonListener = this;
        setupAlert();

        mService = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));

        // Record before selection
        mIsLastDataOn = false;
        mLastDataSimId = 0;
        mIsDone = false;

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mLastDataSimId = Settings.System.getLong(getContentResolver(),
                    Settings.System.GPRS_CONNECTION_SIM_SETTING,
                    Settings.System.DEFAULT_SIM_NOT_SET);
            if (mLastDataSimId > 0) {
                mIsLastDataOn = true;
            } else {
                mLastDataSimId = Settings.System.getLong(
                    getContentResolver(),
                    Settings.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
                    -1);
                Log.d(TAG,"onCreate, LAST_SIMID_BEFORE_WIFI_DISCONNECTED:" + mLastDataSimId);
                if (mLastDataSimId > 0) {
                    mIsLastDataOn = true;
                }
            }
        } else {
            ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cm != null) {
                mIsLastDataOn = cm.getMobileDataEnabled();
            }
        }
        Log.d(TAG, "onCreate : mIsLastDataOn="
                + mIsLastDataOn + ", mLastDataSimId=" + mLastDataSimId);
        // if ON, recored
        if (mIsLastDataOn) {
            onUserNegative(); // Disable firstly before user confirmed
            // workaround for [ALPS00356679] [Daily Use][Data Service]Data service can not use
            enableApn();
        }
    }

    private View createView() {
        TextView messageView = new TextView(this);
        messageView.setText(R.string.wifi_signal_found_msg);
        return messageView;
    }

    private void onUserPositive() {
        Log.d(TAG, "onUserPositive");

        ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm == null) {
            Log.d(TAG,
                    "onUserPositive : get CONNECTIVITY_SERVICE failed");
            return;
        }

        if (cm.getMobileDataEnabled()) {
            Log.d(TAG, "onUserPositive : already ON");
            return;
        }

        Log.d(TAG, "onUserPositive: gemini="
                + FeatureOption.MTK_GEMINI_SUPPORT + ", mIsLastDataOn="
                + mIsLastDataOn + ", mLastDataSimId=" + mLastDataSimId);

        if (FeatureOption.MTK_GEMINI_SUPPORT && mLastDataSimId > 0) {
            Intent intent = new Intent(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
            intent.putExtra("simid", mLastDataSimId);
            Log.d(TAG, "onUserPositive : data sim change");
            sendBroadcast(intent);
        } else {
            cm.setMobileDataEnabled(true);
            if (mService != null) {
                try {
                    mService.enableApnType(PhoneConstants.APN_TYPE_DEFAULT);
                } catch (RemoteException e) {
                    Log.e(TAG,
                            "Fail to call mService.enableApnType():"
                                    + e.getMessage());
                }
            }
        }
    }

    private void onUserNegative() {
        Log.d(TAG, "onUserNegative");

        ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            cm.setMobileDataEnabled(false);
        }
    }

    protected void enableApn() {
        try {
            if (mService != null) {
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    int gprsDefaultSlot = Settings.System.getInt(getContentResolver(), 
                            Settings.System.GPRS_CONNECTION_SETTING, 
                            Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
                    if (gprsDefaultSlot == PhoneConstants.GEMINI_SIM_1 || gprsDefaultSlot == PhoneConstants.GEMINI_SIM_2) {
                        int ret = mService.enableApnTypeGemini(PhoneConstants.APN_TYPE_DEFAULT, gprsDefaultSlot);
                        Log.d(TAG, "enableApnTypeGemini(): slot=" + gprsDefaultSlot + ", the return value" + ret);
                    }
                } else {
                    int ret = mService.enableApnType(PhoneConstants.APN_TYPE_DEFAULT);
                    Log.d(TAG, "enableApnType() the return value" + ret);
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            Log.d(TAG, "tryRestoreDefault(): Connect to phone service error");
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG,"onDestroy()");
        super.onDestroy();
        if (mIsLastDataOn && !mIsDone) {
            Log.d(TAG,"onDestroy() for dataconnection");
            mIsDone = true;
            ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo nwInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (nwInfo.isConnected()) {
                onUserPositive();
            } else {
                Intent failoverIntent = new Intent(TelephonyIntents.ACTION_WIFI_FAILOVER_GPRS_DIALOG);
                mContext.sendBroadcastAsUser(failoverIntent, UserHandle.ALL);
            }
            mIsLastDataOn = false;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        
        Log.e(TAG, "onPause entry");

        ActivityManager amgr = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
        ComponentName cpname = null;
        String classname = null;

        if (amgr.getRunningTasks(1) != null && amgr.getRunningTasks(1).get(0) != null) {
            cpname = amgr.getRunningTasks(1).get(0).topActivity;
        }

        if (cpname!=null) {
            classname = cpname.getClassName();
            Log.d(TAG, "ClassName:"+classname);
        } else {
            Log.e(TAG, "Component ClassName is null");
            return;
        }

        if (classname!=null && classname.equals("com.mediatek.data.DataConnectionDialog")) {
            Log.e(TAG, "Reselect dialog finish because DataConnectionDialog on its top");
            finish();
        }
    }
    
    private void onOK() {
        Log.d(TAG,"onOK()");
        mIsDone = true;
        if (mIsLastDataOn) {
            Log.d(TAG,"onOK() for dataconnection");
            onUserPositive();
            mIsLastDataOn = false;
        }
        Intent intent = new Intent();
        intent.setAction("android.settings.WIFI_SETTINGS");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(intent);
        finish();
    }

    private void onCancel() {
        Log.d(TAG,"onCancel()");
        mIsDone = true;
        if (mIsLastDataOn) {
            Log.d(TAG,"onCancel() for dataconnection");
            ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo nwInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (nwInfo.isConnected()) {
                Log.d(TAG,"onCancel() wifi is connecting");
                onUserPositive();
            } else {
                Log.d(TAG,"onCancel() wifi is disconnected, to open GPRS_DIALOG");
                Intent failoverIntent = new Intent(TelephonyIntents.ACTION_WIFI_FAILOVER_GPRS_DIALOG);
                mContext.sendBroadcastAsUser(failoverIntent, UserHandle.ALL);
            }
            mIsLastDataOn = false;
        }
        mWifiManager.suspendNotification(IWifiFwkExt.NOTIFY_TYPE_RESELECT);
        finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                onOK();
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                onCancel();
                break;
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(TAG,"onNewIntent()");
        boolean isLastDataOn = false;
        long lastDataSimId = 0;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            lastDataSimId = Settings.System.getLong(getContentResolver(),
                    Settings.System.GPRS_CONNECTION_SIM_SETTING,
                    Settings.System.DEFAULT_SIM_NOT_SET);
            if (lastDataSimId > 0) {
                isLastDataOn = true;
                mLastDataSimId = lastDataSimId;
            } else {
                lastDataSimId = Settings.System.getLong(
                    getContentResolver(),
                    Settings.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
                    -1);
                Log.d(TAG,"onNewIntent, LAST_SIMID_BEFORE_WIFI_DISCONNECTED:" + mLastDataSimId);
                if (lastDataSimId > 0) {
                    isLastDataOn = true;
                    mLastDataSimId = lastDataSimId;
                }
            }
        } else {
            ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cm != null) {
                isLastDataOn = cm.getMobileDataEnabled();
            }
        }
        Log.d(TAG, "onNewIntent : isLastDataOn="
                + isLastDataOn + ", lastDataSimId=" + lastDataSimId);
        Log.d(TAG, "onNewIntent : mIsLastDataOn="
                + mIsLastDataOn + ", mLastDataSimId=" + mLastDataSimId);
        // if ON, recored
        if (isLastDataOn) {
            mIsLastDataOn = isLastDataOn;
            onUserNegative(); // Disable firstly before user confirmed
            // workaround for [ALPS00356679] [Daily Use][Data Service]Data service can not use
            enableApn();
        }
    }
}

