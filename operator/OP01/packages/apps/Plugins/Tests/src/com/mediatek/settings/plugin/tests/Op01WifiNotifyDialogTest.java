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

package com.mediatek.settings.plugin.tests;

import android.content.Context;
import android.app.ActionBar;
import android.app.Activity;
import android.app.Instrumentation;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;

import android.content.res.Resources;

import com.mediatek.op01.plugin.WifiNotifyDialog;
import com.mediatek.op01.plugin.R;

import java.util.List;
import com.jayway.android.robotium.solo.Solo;
import com.mediatek.xlog.Xlog;

public class Op01WifiNotifyDialogTest extends
        ActivityInstrumentationTestCase2<WifiNotifyDialog> {
    public static final String TAG = "Op01WifiNotifyDialogTest";
    private static final int TWO_THOUSANDS_MILLISECOND = 2000;
    public static final String MTKCMCC = "CMCC";

    private WifiNotifyDialog mActivity;
    private Instrumentation mInst;
    private Solo mSolo;

    public Op01WifiNotifyDialogTest() {
        super("com.mediatek.op01.plugin", WifiNotifyDialog.class);
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();

        Xlog.i(TAG, "setUp()");
        setActivityInitialTouchMode(false);

        Intent intent = new Intent(WifiManager.WIFI_NOTIFICATION_ACTION);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(WifiManager.EXTRA_NOTIFICATION_SSID, MTKCMCC);
        intent.putExtra(WifiManager.EXTRA_NOTIFICATION_NETWORKID, 1);
        mActivity = this.getActivity();
        mActivity.startActivity(intent);
        
        mInst = this.getInstrumentation();
        mSolo = new Solo(mInst, mActivity);
    }

    @Override
    public void tearDown() throws Exception {
        Xlog.d(TAG, "tearDown");
        mSolo.goBack();
    }

    public void test01_WifiNotifyApCancelTest() {
        Xlog.i(TAG, "test01_WifiNotifyApCancelTest() begin");

        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
/*
        String msg = mActivity.getString(R.string.msg_wlan_signal_found, MTKCMCC);
        if (mSolo.searchText(msg)) {
            Xlog.i(TAG, "message test pass");
        } else {
            Xlog.i(TAG, "message test fail");
            assertTrue(false);
        }

        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
*/
        String no = mActivity.getString(android.R.string.no);
        if (mSolo.searchText(no)) {
            mSolo.clickOnText(no);
            Xlog.i(TAG, "button test pass");
        } else {
            Xlog.i(TAG, "button test fail");
            assertTrue(false);
        }

        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
    }

    public void test02_WifiNotifyApPositiveTest() {
	    Xlog.i(TAG, "test02_WifiNotifyApPositiveTest() begin");

	    mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

	    String yes = mActivity.getString(android.R.string.yes);

	    if (mSolo.searchText(yes)) {
	        mSolo.clickOnText(yes);
	        Xlog.i(TAG, "button test pass");
	    } else {
	        Xlog.i(TAG, "button test fail");
	        assertTrue(false);
	    }

       mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
       mSolo.goBack();
       mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	}

}
