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
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Message;
import android.provider.Settings.System;
import android.view.View;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.internal.util.AsyncChannel;
import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

public class WifiNotifyDialog extends AlertActivity implements DialogInterface.OnClickListener {
    private static final String TAG = "WifiNotifyDialog";

    private WifiManager mWm;
    private String mSsid;
    private int mNetworkId;

    private boolean mConnectApType;
    private WifiManager.ActionListener mConnectListener;

    private static void log(String msg) {
        Xlog.d(TAG, msg);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mWm = (WifiManager)getSystemService(Context.WIFI_SERVICE);
        mConnectListener = new WifiManager.ActionListener() {
                                   public void onSuccess() {
                                   }
                                   public void onFailure(int reason) {
                                   }
                               };

        Intent intent = getIntent();
        String action = intent.getAction();
        mSsid = intent.getStringExtra(IWifiFwkExt.EXTRA_NOTIFICATION_SSID);
        mNetworkId = intent.getIntExtra(IWifiFwkExt.EXTRA_NOTIFICATION_NETWORKID, -1);
        log("WifiNotifyDialog onCreate " + action);
        if (!action.equals(IWifiFwkExt.WIFI_NOTIFICATION_ACTION) || mNetworkId == -1) {
            Xlog.e(TAG, "Error: this activity may be started only with intent WIFI_NOTIFICATION_ACTION");
            finish();
        }
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
    }

    private View createView() {
        log("createView mSsid=" + mSsid);
        int value = System.getInt(getContentResolver(), System.WIFI_CONNECT_TYPE, System.WIFI_CONNECT_TYPE_AUTO);
        TextView messageView = new TextView(this);
        if (value == System.WIFI_CONNECT_TYPE_MANUL) {
            messageView.setText(getString(R.string.msg_wlan_signal_found_manul));
        } else if (value == System.WIFI_CONNECT_TYPE_ASK) {
            messageView.setText(getString(R.string.msg_wlan_signal_found, mSsid));
        }
        
        return messageView;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void onOK() {
        log("onOK mNetworkId=" + mNetworkId);
        int value = System.getInt(getContentResolver(), System.WIFI_CONNECT_TYPE, System.WIFI_CONNECT_TYPE_AUTO);
        mConnectApType = System.getInt(getContentResolver(), System.WIFI_CONNECT_AP_TYPE, System.WIFI_CONNECT_AP_TYPE_AUTO)==0;
        if (mConnectApType  && (value != System.WIFI_CONNECT_TYPE_MANUL)){
            log("onOK auto connect AP");
            mWm.connect(mNetworkId, mConnectListener);
        } else {
            Intent intent = new Intent();
            intent.setAction("android.settings.WIFI_SETTINGS");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
        finish();
    }

    private void onCancel() {
        log("onCancel mNetworkId=" + mNetworkId);
        mWm.suspendNotification(IWifiFwkExt.NOTIFY_TYPE_SWITCH);
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

}
