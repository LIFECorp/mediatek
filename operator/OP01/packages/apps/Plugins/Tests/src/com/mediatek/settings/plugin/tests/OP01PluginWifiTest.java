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

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.res.Resources;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.widget.CheckBox;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import android.provider.Settings.System;
import android.provider.Settings.Secure;
import android.net.DhcpInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

import com.mediatek.op01.plugin.R;
import com.mediatek.settings.ext.IWifiSettingsExt;
import com.mediatek.settings.ext.IWifiExt;
import com.mediatek.settings.ext.ISettingsMiscExt;

import java.util.List;
import com.jayway.android.robotium.solo.Solo;
import com.mediatek.xlog.Xlog;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;


public class OP01PluginWifiTest extends OP01PluginBase {
    public static final String TAG = "WifiPluginTest";
    public static final String MTKLAB = "mtklab";
    public static final String MTKGUEST = "mtkguest";
    private static final String MTKLABPWD = "1234567890";
    public static final String MTKCMCC = "CMCC";
    public static final String MTKCMCCAUTO = "CMCC-AUTO";
    public static final String MTKCMCCEdu = "CMCC-EDU";
    public static final String PrioritySettings = "AP priority settings";
    public static final String PRIORITY1= "1";
    private static final int TWO_THOUSANDS_MILLISECOND = 2000;
    private static final int FIVE_THOUSANDS_MILLISECOND = 5000;

    /** These values are matched in string arrays -- changes must be kept in sync */
    static final int SECURITY_NONE = 0;
    static final int SECURITY_WEP = 1;
    static final int SECURITY_PSK = 2;
    // M: security type @{
    static final int SECURITY_WPA_PSK = 3;
    static final int SECURITY_WPA2_PSK = 4;
    static final int SECURITY_EAP = 5;
    static final int SECURITY_WAPI_PSK = 6;
    static final int SECURITY_WAPI_CERT = 7;
    // @}

    private PreferenceActivity mActivity;
    private Context mContext;
    private Instrumentation mInst;
    private Solo mSolo;
    private Switch mwifiSwitch;
    Resources mResource;
    String mPackageName;  

    private long mStartTime = 0;
    private long mEndTime = 0;
    private boolean mConnectFinish = false;
    private boolean mSwitchStatus = false;
    IWifiSettingsExt mWifiSettingsExt;
    IWifiExt mWifiExt;
    private WifiManager mWifiManager;
    private List<WifiConfiguration> mConfigs;

    private static Class<?> launcherActivityClass;
    private static final String PACKAGE_ID_STRING = "com.android.settings";
    private static final String ACTIVITY_FULL_CLASSNAME = "com.android.settings.wifi.WifiSettings";

    static {
        try {
            launcherActivityClass = Class.forName(ACTIVITY_FULL_CLASSNAME);
        } catch (Exception exception) {
            throw new RuntimeException(exception);
        } 
    }

    public OP01PluginWifiTest() {
        super(PACKAGE_ID_STRING, launcherActivityClass);
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();

        Xlog.i(TAG, "setUp()");
        setActivityInitialTouchMode(false);
        mActivity = (PreferenceActivity)getActivity();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mSolo = new Solo(mInst, mActivity);
        mwifiSwitch = (Switch) mActivity.getActionBar().getCustomView();

        mWifiManager = (WifiManager) mActivity.getSystemService(Context.WIFI_SERVICE);

        mWifiSettingsExt = (IWifiSettingsExt)PluginManager.createPluginObject(mContext, IWifiSettingsExt.class.getName());
        Xlog.i(TAG , "WifiSettingsExt Plugin object created");
        mWifiExt = (IWifiExt)PluginManager.createPluginObject(mContext, IWifiExt.class.getName());
        Xlog.i(TAG , "WifiExt Plugin object created");

        mResource = mActivity.getResources();
        mPackageName = mActivity.getPackageName();   
    }

    @Override
    public void tearDown() throws Exception {
        Xlog.d(TAG, "tearDown");
        super.tearDown();
        try {
            mSolo.finishOpenedActivities();
        } catch(Exception e) {
            Xlog.d(TAG, "tearDown exception");;
        }
    }

    public void test01_WifiOnOff() {
        Xlog.i(TAG, "test01_WifiOnOff() begin");

        assertNotNull(mActivity);
        assertNotNull(mInst);
        assertNotNull(mSolo);
        assertNotNull(mwifiSwitch);
        assertNotNull(mWifiSettingsExt);
        assertNotNull(mWifiExt);

        //make sure the wifi is enable
 	    if (!mwifiSwitch.isChecked()){
	        mSolo.clickOnView(mwifiSwitch);
	        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            if (!mwifiSwitch.isChecked()) {
                Xlog.i(TAG, "enable WIFI fail");
            	assertTrue(false);
            }
	    }

        //disable wifi
        mSolo.clickOnView(mwifiSwitch);
        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
        if (mwifiSwitch.isChecked()) {
            Xlog.i(TAG, "disable WIFI fail");
            assertTrue(false);
        }
    }

    public void test02_WifiTrustandMenu() {
        Xlog.i(TAG, "test02_WifiTrustandMenu() begin");

        // make sure the wifi is enable
        turnOnWifi();

        int advancedid = mResource.getIdentifier("wifi_menu_advanced", "string", mPackageName);
        String advanced = mResource.getString(advancedid);

        // open auto connect in advanced
        if (mWifiSettingsExt.isCatogoryExist()) {
            if (!enterAdvancedSettings()) {
	            Xlog.i(TAG, "entry advance fail");
	            assertTrue(false);
	        }

            //auto connnect is the 4th check box
            if (!mSolo.isCheckBoxChecked(3)) {
                mSolo.clickOnCheckBox(3);
            }
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

            mSolo.goBack();

            // check WifiSettingsExt.isTustAP()
            if ((mWifiSettingsExt.isTustAP(MTKCMCC, SECURITY_NONE) && !mWifiSettingsExt.isTustAP(MTKCMCCEdu, SECURITY_NONE))
                &&(!mWifiSettingsExt.isTustAP("metguest", SECURITY_NONE) && !mWifiSettingsExt.isTustAP("SOHU-guest", SECURITY_NONE))){
                Xlog.i(TAG, "isTustAP() verify pass");
            } else {
                Xlog.i(TAG, "isTustAP() verify fail");
                assertTrue(false);
            }

            // check WifiSettingsExt.shouldAddDisconnectMenu()
            //if (mWifiSettingsExt.shouldAddDisconnectMenu()){
            //    Xlog.i(TAG, "shouldAddDisconnectMenu() verify pass");
            //} else {
            //    Xlog.i(TAG, "shouldAddDisconnectMenu() verify fail");
            //    assertTrue(false);
            //}

            // check WifiSettingsExt.shouldAddForgetMenu()
            if ((mWifiSettingsExt.shouldAddForgetMenu(MTKCMCC, SECURITY_NONE) && mWifiSettingsExt.shouldAddForgetMenu(MTKCMCCEdu, SECURITY_NONE))
                && (mWifiSettingsExt.shouldAddForgetMenu("metguest", SECURITY_NONE) && mWifiSettingsExt.shouldAddForgetMenu("SOHU-guest", SECURITY_WEP))){
                Xlog.i(TAG, "shouldAddForgetMenu() verify pass");
            } else {
                Xlog.i(TAG, "shouldAddForgetMenu() verify fail");
                assertTrue(false);
            }
        }
        else
        {
            Xlog.i(TAG, "isCatogoryExist() verify fail");
            assertTrue(false);
        }
    }

    public void test03_WifiConnectApTypeTest() {
	    Xlog.i(TAG, "test03_WifiConnectApTypeTest() begin");

        if (System.getInt(mActivity.getContentResolver(), System.WIFI_CONNECT_AP_TYPE, System.WIFI_CONNECT_AP_TYPE_AUTO) != System.WIFI_CONNECT_AP_TYPE_AUTO) {
            Xlog.i(TAG, "auto connect status fail");
            assertTrue(false);
        }
	}

    public void test04_WifiPriorityTypeTest() {
        Xlog.i(TAG, "test04_WifiPriorityTypeTest() begin");

        // make sure the wifi switch is enable
        turnOnWifi();

        if (!enterAdvancedSettings()) {
            Xlog.i(TAG, "entry advance fail");
            assertTrue(false);
        }

        // verify check box of priority set which is 4rd checkbox
        if (!mSolo.isCheckBoxChecked(3)) {
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_PRIORITY_TYPE, System.WIFI_PRIORITY_TYPE_DEFAULT) != System.WIFI_PRIORITY_TYPE_DEFAULT) {
                Xlog.i(TAG, "default priority status fail");
                assertTrue(false);
            }

            mSolo.clickOnCheckBox(3);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_PRIORITY_TYPE, System.WIFI_PRIORITY_TYPE_DEFAULT) != System.WIFI_PRIORITY_TYPE_MAMUAL) {
                Xlog.i(TAG, "manual priority status fail");
                assertTrue(false);
            }
        }else{
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_PRIORITY_TYPE, System.WIFI_PRIORITY_TYPE_DEFAULT) != System.WIFI_PRIORITY_TYPE_MAMUAL) {
                Xlog.i(TAG, "2 manual priority status fail");
                assertTrue(false);
            }

            mSolo.clickOnCheckBox(3);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_PRIORITY_TYPE, System.WIFI_PRIORITY_TYPE_DEFAULT) != System.WIFI_PRIORITY_TYPE_DEFAULT) {
                Xlog.i(TAG, "2 default priority status fail");
                assertTrue(false);
            }
        }

        if (!mSolo.isCheckBoxChecked(3)) {
            mSolo.clickOnCheckBox(3);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
        }
    }

	public void test05_WifiGatewayNetmaskTest() {
	    Xlog.i(TAG, "test05_WifiGatewayNetmaskTest() begin");

	    // make sure the wifi switch is enable
        turnOnWifi();

	    if (!enterAdvancedSettings()) {
	        Xlog.i(TAG, "entry advance fail");
	        assertTrue(false);
	    }

	    scrollBottom();

	    int defaultTextId = mResource.getIdentifier("status_unavailable", "string", mPackageName);
	    String defaultText = mResource.getString(defaultTextId);
	    String wifi_gateway = "Gateway";
	    String wifi_netmask = "Netmask";

	    // verify gateway and netmask
	    WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
	    String gateway = null;
	    String netmask = null;
	    DhcpInfo dhcpInfo = mWifiManager.getDhcpInfo();
	    if (wifiInfo != null) {
	        if (dhcpInfo != null) {
	            gateway = ipTransfer(dhcpInfo.gateway);
	            netmask = ipTransfer(dhcpInfo.netmask);
	        }
	    }
	    if (gateway == null) {
	        gateway = defaultText;
	    }
	    if (netmask == null) {
	        netmask = defaultText;
	    }
	    String showgateway = getPreferenceSummary(wifi_gateway);
	    String shownetmask = getPreferenceSummary(wifi_netmask);
	    Xlog.i(TAG, "gateway is :" + gateway);
	    Xlog.i(TAG, "netmask is :" + netmask);
	    assertEquals(gateway, showgateway);
	    assertEquals(netmask, shownetmask);

	    //back to wifi settings
	    mSolo.goBack();
	    mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	}

    public void test06_WifiConnectApTest() {
        Xlog.i(TAG, "test06_WifiConnectApTest() begin");

        // make sure the wifi switch is enable
        turnOnWifi();

        int forgetid = mResource.getIdentifier("wifi_forget", "string", mPackageName);
        String forget = mResource.getString(forgetid);
        int connectid = mResource.getIdentifier("wifi_setup_connect", "string", mPackageName);
        String connect = mResource.getString(connectid);
        int wifi_statusId = mResource.getIdentifier("wifi_status", "array", mPackageName);
        CharSequence[] array = mResource.getTextArray(wifi_statusId);

        // check auto connect and manually priority
        if (!enterAdvancedSettings()) {
            Xlog.i(TAG, "entry advance fail");
            assertTrue(false);
        }

        // manually priority is the 3rd check box, check it
        if (!mSolo.isCheckBoxChecked(2)) {
            mSolo.clickOnCheckBox(2);
        }
        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
        mSolo.goBack();

        // if the ap is connected, need forget it 
        scrollTop();
        if (mSolo.searchText(MTKLAB)) {
            mSolo.clickOnText(MTKLAB);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

            if (mSolo.searchButton(forget, true)) {
                mSolo.clickOnButton(forget);
            	mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            }
        }

        scrollTop();
        if (mSolo.searchText(MTKLAB)) {
            mSolo.clickOnText(MTKLAB);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            mSolo.enterText(0, MTKLABPWD);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            // select the first spinner which is priority, set as 2
            mSolo.pressSpinnerItem(0, 1);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            mSolo.clickOnButton(connect);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
        }

        //mtklab should be connected now
        scrollTop();
        if (mSolo.searchText(MTKLAB)) {
            String state = getPreferenceSummary(MTKLAB);
            String policy = (String) array[5];
            if (!state.contentEquals(policy)) {
                Xlog.i(TAG, "WifiConnectApTest fail");
                assertTrue(false);
            }
        } else {
            Xlog.i(TAG, "can not find the AP, fail");
            assertTrue(false);
        }

        //if the ap is connected, forget it to original
        scrollTop();
        if (mSolo.searchText(MTKLAB)) {
            mSolo.clickOnText(MTKLAB);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

            if (mSolo.searchButton(forget, true)) {
                mSolo.clickOnButton(forget);
                mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            }
        }
    }

    public void test07_WifiForgetApTest() {
        Xlog.i(TAG, "test07_WifiForgetApTest() begin");

        //make sure the wifi switch is enable
        turnOnWifi();

        int forgetId = mResource.getIdentifier("wifi_forget", "string", mPackageName);
        String forget = mResource.getString(forgetId);
        int wifi_cancelId = mResource.getIdentifier("wifi_cancel", "string", mPackageName);
        String wifi_cancel = mResource.getString(wifi_cancelId);
        int wifi_statusId = mResource.getIdentifier("wifi_status", "array", mPackageName);
        CharSequence[] array = mResource.getTextArray(wifi_statusId);

        // make sure the mtkguest is connected
        if (mSolo.searchText(MTKGUEST)) {
            mSolo.clickOnText(MTKGUEST);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

            if (mSolo.searchButton(forget, true)) {
                mSolo.clickOnButton(wifi_cancel);
                mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            }
        }

        // forget mtkguest
        scrollTop();
        if (mSolo.searchText(MTKGUEST)) {
            mSolo.clickOnText(MTKGUEST);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

            if (mSolo.searchButton(forget, true)) {
                mSolo.clickOnButton(forget);
                mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            }
        }
        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);

        //there maybe one popup of wifi to gprs
        if (mSolo.searchText("Cancel")) {
            mSolo.clickOnText("Cancel");
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
        }

        // mtkguest should be disconnected
        scrollTop();
        if (mSolo.searchText(MTKGUEST)) {
            String state = getPreferenceSummary(MTKGUEST);
            String policy = (String) array[0];
            if (!state.contentEquals(policy)) {
                Xlog.i(TAG, "AP state verify fail");
                assertTrue(false);
            }
        } else {
            Xlog.i(TAG, "can not find the AP, fail");
            assertTrue(false);
        }
    }

    public void test09_WifiSsidReselectTest() {
        Xlog.i(TAG, "test09_WifiSsidReselectTest() begin");

        // make sure the wifi switch is enable
        turnOnWifi();

        if (!enterAdvancedSettings()) {
            Xlog.i(TAG, "entry advance fail");
            assertTrue(false);
        }

        // verify select ssid
        // String slectSSID = mActivity.getString(com.mediatek.op01.plugin.R.string.select_ssid_type_title);
        String slect_ssid = "Set network connection method";
        if (mSolo.searchText(slect_ssid)) {
            // set as always ask
            mSolo.clickOnText(slect_ssid);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            mSolo.clickInList(3);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_SELECT_SSID_TYPE, System.WIFI_SELECT_SSID_ASK) != System.WIFI_SELECT_SSID_ASK) {
                Xlog.i(TAG, "SSID reselect always ask fail");
                assertTrue(false);
            }

            // set as manually
            mSolo.clickOnText(slect_ssid);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            mSolo.clickInList(2);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_SELECT_SSID_TYPE, System.WIFI_SELECT_SSID_ASK) != System.WIFI_SELECT_SSID_MANUL) {
                Xlog.i(TAG, "SSID reselect manually fail");
                assertTrue(false);
            }

            // set as automatically
            mSolo.clickOnText(slect_ssid);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            mSolo.clickInList(1);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (System.getInt(mActivity.getContentResolver(), System.WIFI_SELECT_SSID_TYPE, System.WIFI_SELECT_SSID_ASK) != System.WIFI_SELECT_SSID_AUTO) {
                Xlog.i(TAG, "SSID reselect manually fail");
                assertTrue(false);
            }
        }else{
            Xlog.i(TAG, "SSID reselect can't find, fail");
            assertTrue(false);
        }
    }

	public void test10_WifiConnectTypeTest() {
	    Xlog.i(TAG, "test10_WifiConnectTypeTest() begin");

	    // make sure the wifi switch is enable
        turnOnWifi();

	    if (!enterAdvancedSettings()) {
	        Xlog.i(TAG, "entry advance fail");
	        assertTrue(false);
	    }

	    // verify gsm to wlan
	    // String gtow = mResource.getString(com.mediatek.op01.plugin.R.string.gsm_to_wifi_connect_type_title);
	    String connect_type = "Set network connection method";
	    if (mSolo.searchText(connect_type)) {
	        // set as always ask
	        mSolo.clickOnText(connect_type);
	        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	        mSolo.clickInList(3);
	        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	        if (System.getInt(mActivity.getContentResolver(), System.WIFI_CONNECT_TYPE, System.WIFI_CONNECT_TYPE_AUTO) != System.WIFI_CONNECT_TYPE_ASK) {
	            Xlog.i(TAG, "gsm to wifi always ask fail");
	            assertTrue(false);
	        }

	        // set as manually
	        mSolo.clickOnText(connect_type);
	        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	        mSolo.clickInList(2);
	        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	        if (System.getInt(mActivity.getContentResolver(), System.WIFI_CONNECT_TYPE, System.WIFI_CONNECT_TYPE_AUTO) != System.WIFI_CONNECT_TYPE_MANUL) {
	            Xlog.i(TAG, "gsm to wifi manually fail");
	            assertTrue(false);
	        }

	        // set as automatically
	        mSolo.clickOnText(connect_type);
	        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	        mSolo.clickInList(1);
	        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
	        if (System.getInt(mActivity.getContentResolver(), System.WIFI_CONNECT_TYPE, System.WIFI_CONNECT_TYPE_AUTO) != System.WIFI_CONNECT_TYPE_AUTO) {
	            Xlog.i(TAG, "gsm to wifi auto fail");
	            assertTrue(false);
	        }
	    }else{
	        Xlog.i(TAG, "GSM to WLAN can't find, fail");
	        assertTrue(false);
	    }
	}

    public void test11_WifiSleepPolicyChangeTest() {
        Xlog.d(TAG, "test11_WifiSleepPolicyChangeTest() begin");

        if (!enterAdvancedSettings()) {
            Xlog.i(TAG, "entry advance fail");
            assertTrue(false);
        }

	    int sleepPolicyId = mResource.getIdentifier("wifi_setting_sleep_policy_title", "string", mPackageName);
    	String sleepPolicy = mResource.getString(sleepPolicyId);
        int sleepStatusId = mResource.getIdentifier("wifi_sleep_policy_entries", "array", mPackageName);
        CharSequence[] sleepStatus = mResource.getTextArray(sleepStatusId);
        mSolo.clickOnText(sleepPolicy);
        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

        // click always item
        String index = (String) sleepStatus[0];
        mSolo.clickOnText(index);
        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

        // get value from provider
        int value = mWifiExt.getSleepPolicy(mActivity.getContentResolver());

        assertEquals(2, value);
        mSolo.goBack();
        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
    }

    public void test12_WifiExtApTest() {
        Xlog.i(TAG, "test12_WifiExtApTest() begin");

        //make sure the option is enable
        turnOnWifi();

        //verify getApOrder()
        if (mWifiExt.getApOrder(MTKCMCC, SECURITY_NONE, MTKLAB, SECURITY_NONE) != -1){
            Xlog.i(TAG, "getApOrder verify 1 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKCMCC, SECURITY_NONE, MTKLAB, SECURITY_WEP) != -1){
            Xlog.i(TAG, "getApOrder verify 2 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKCMCC, SECURITY_WEP, MTKLAB, SECURITY_WEP) != 0){
            Xlog.i(TAG, "getApOrder verify 3 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKCMCC, 0, MTKCMCCEdu, 0) != -1){
            Xlog.i(TAG, "getApOrder verify 4 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKCMCCAUTO, SECURITY_EAP, MTKCMCC, 0) != -1){
            Xlog.i(TAG, "getApOrder verify 5 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKCMCC, 0, MTKCMCCAUTO, SECURITY_EAP) != 1){
            Xlog.i(TAG, "getApOrder verify 6 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKCMCCEdu, 0, MTKCMCCAUTO, SECURITY_EAP) != 1){
            Xlog.i(TAG, "getApOrder verify 7 fail");
            assertTrue(false);
        }
        if (mWifiExt.getApOrder(MTKLAB, SECURITY_WEP, MTKCMCC, 0) != 1){
            Xlog.i(TAG, "getApOrder verify 8 fail");
            assertTrue(false);
        }

        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

        //verify getWifiApSsid()
        String summary = mWifiExt.getWifiApSsid();
        String cmccSummary = "MTK Athens15";
        assertEquals(summary, cmccSummary);
        Xlog.i(TAG, "getWifiApSsid pass");
    }

    public void test13_WifiSettingsMiscTest() {
        Xlog.i(TAG, "test13_WifiSettingsMiscTest() begin");

        ISettingsMiscExt SettingsMiscExt;
        try {
            SettingsMiscExt = (ISettingsMiscExt)PluginManager.createPluginObject(mContext,
                ISettingsMiscExt.class.getName());
            Xlog.i(TAG , "SettingsMiscExt Plugin object created");

            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

	        //verify isWifiToggleCouldDisabled()
	        boolean status = false;
	        if (System.getInt(mContext.getContentResolver(), System.AIRPLANE_MODE_ON, 0) == 0) {
	            status = true;
	        }
	        if (SettingsMiscExt.isWifiToggleCouldDisabled(mContext) != status){
	            Xlog.i(TAG, "isWifiToggleCouldDisabled verify fail");
	            assertTrue(false);
	        }

	        //verify getTetherWifiSSID()
	        //String cmccSummary = "MTK Athens15";
	        //String summary = SettingsMiscExt.getTetherWifiSSID(mContext);
	        //assertEquals(summary, cmccSummary);
	        //Xlog.i(TAG, "getTetherWifiSSID pass");
        } catch (Plugin.ObjectCreationException e) {
            assertTrue(false);
        }
    }


    public void test14_WifiModifyApTest() {
        Xlog.i(TAG, "test14_WifiModifyApTest() begin");

        // make sure the wifi switch is enable
        if(!mwifiSwitch.isChecked()){
            mSolo.clickOnView(mwifiSwitch);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
        }

        int advancedId = mResource.getIdentifier("wifi_menu_advanced", "string", mPackageName);
        String advanced = mResource.getString(advancedId);
        int forgetId = mResource.getIdentifier("wifi_forget", "string", mPackageName);
        String wifi_forget = mResource.getString(forgetId);
        int wifi_menu_modifyId = mResource.getIdentifier("wifi_menu_modify", "string", mPackageName);
        String wifi_menu_modify = mResource.getString(wifi_menu_modifyId);
        int wifi_saveId = mResource.getIdentifier("wifi_save", "string", mPackageName);
        String wifi_save = mResource.getString(wifi_saveId);
        int wifi_disconnectId = mResource.getIdentifier("wifi_disconnect", "string", mPackageName);
        String wifi_disconnect = mResource.getString(wifi_disconnectId);
        int wifi_statusId = mResource.getIdentifier("wifi_status", "array", mPackageName);
        CharSequence[] array = mResource.getTextArray(wifi_statusId);

        // check auto connect and manually priority
        if (mWifiSettingsExt.isCatogoryExist()) {
            mSolo.sendKey(KeyEvent.KEYCODE_MENU);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (mSolo.searchText(advanced)) {
                mSolo.clickOnText(advanced);
                mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

                // manually priority is the 3rd check box, check it
                if (mSolo.isCheckBoxChecked(2)) {
                    mSolo.clickOnCheckBox(2);
                    mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
                }
                if (!mSolo.isCheckBoxChecked(2)) {
                    mSolo.clickOnCheckBox(2);
                }
                mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            }
            mSolo.goBack();
        }else {
            Xlog.i(TAG, "open auto connect and manually priority fail");
            assertTrue(false);
        }

        // if mtkguest connected or trust but not connect, forget it then connect
        scrollTop();
        if (mSolo.searchText(MTKGUEST)) {
            mSolo.clickOnText(MTKGUEST);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (mSolo.searchButton(wifi_forget, true)) {
                mSolo.clickOnButton(wifi_forget);
                mSolo.sleep(TWO_THOUSANDS_MILLISECOND);

                scrollTop();
                if (mSolo.searchText(MTKGUEST)) {
                    mSolo.clickOnText(MTKGUEST);
                    mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
                }
            }
            Xlog.i(TAG, "MTKGUEST connect success");
        }

        // modify CMCC priority as 1
        scrollTop();
        if (mSolo.searchText(MTKCMCC)) {
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            mSolo.clickLongOnText(MTKCMCC, 2);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            mSolo.clickOnText(wifi_menu_modify);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            // up 20
            mSolo.pressSpinnerItem(0, -20);
            Xlog.i(TAG, "set MTKCMCC priority as 1 success");
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            mSolo.clickOnText(wifi_save);
        } else {
            Xlog.i(TAG, "set MTKCMCC priority as 1 fail");
            assertTrue(false);
        }

        // if modify MTKGUEST priority reduce 1
        scrollTop();
        if (mSolo.searchText(MTKGUEST)) {
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            mSolo.clickLongOnText(MTKGUEST);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            mSolo.clickOnText(wifi_menu_modify);
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            //down 1
            mSolo.pressSpinnerItem(0, 1);
            Xlog.i(TAG, "set MTKGUEST priority reduce 1 success");
            mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
            mSolo.clickOnText(wifi_save);
        } else {
            Xlog.i(TAG, "set MTKGUEST priority reduce 1 fail");
            assertTrue(false);
        }

        // disconnnect the mtkguest, CMCC will auto connect
        if (mSolo.searchText(MTKGUEST)) {
            mSolo.clickOnText(MTKGUEST);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            if (mSolo.searchButton(wifi_disconnect, true)) {
                mSolo.clickOnButton(wifi_disconnect);
            }
        }

        //there maybe one popup of wifi to gprs
        if (mSolo.searchText("Cancel")) {
            mSolo.clickOnText("Cancel");
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
        }

        scrollBottom();
        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
        scrollTop();
        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
        scrollBottom();
        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);

        // CMCC should be connected now
        scrollTop();
        String state = getPreferenceSummary(MTKCMCC);
        String policy = (String) array[5];
        String policybak = (String) array[4];
        Xlog.d(TAG, "policy=" + policy + ", state=" + state);
        if (mSolo.searchText(MTKCMCC)) {
            if (!state.contentEquals(policy) && !state.contentEquals(policybak)) {
                Xlog.i(TAG, "test05_WifiModifyApTest() verify fail");
                assertTrue(false);
            }
        } else {
            Xlog.i(TAG, "can not find the AP, fail");
            assertTrue(false);
        }
    }


    private void scrollTop() {
        while (mSolo.scrollUp()) {
            mSolo.scrollUp();
        }
    }

    private void scrollBottom() {
        while (mSolo.scrollDown()) {
            mSolo.scrollDown();
        }
    }

    private boolean enterAdvancedSettings() {
        boolean state = false;
        int advancedId = mResource.getIdentifier("wifi_menu_advanced", "string", mPackageName);
        String advanced = mResource.getString(advancedId);

        // entry advance
        mSolo.sendKey(KeyEvent.KEYCODE_MENU);
        mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
        if (mSolo.searchText(advanced)) {
            Xlog.i(TAG, "entry advance success");
            mSolo.clickOnText(advanced);
            mSolo.sleep(TWO_THOUSANDS_MILLISECOND);
            state = true;
        }

        return state;
    }

    private String getPreferenceSummary(String title) {
        // get the IP address preference summary TextView
        TextView tv = (TextView)mSolo.getText(title);
        RelativeLayout rl = (RelativeLayout)tv.getParent();
        TextView summary = (TextView)rl.getChildAt(1);
        return summary.getText().toString();
    }

    private String ipTransfer(int value) {
        String result = null;
        if (value != 0) {
            if (value < 0) {
                value += 0x100000000L;
            }
            result = String.format("%d.%d.%d.%d",
                    value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF);
        }
        return result;
    }

	private void turnOnWifi() {
	    if(!mwifiSwitch.isChecked()){
	        mSolo.clickOnView(mwifiSwitch);
	        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
	    }
	}

	private void turnOffWifi() {
	    if (mwifiSwitch.isChecked()) {
	        mSolo.clickOnView(mwifiSwitch);
	        mSolo.sleep(FIVE_THOUSANDS_MILLISECOND);
	    }
	}

}
