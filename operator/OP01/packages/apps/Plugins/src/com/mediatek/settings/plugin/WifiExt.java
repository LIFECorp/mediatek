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
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent; 
import android.content.res.Resources;

import android.database.ContentObserver;
import android.net.DhcpInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.KeyMgmt;

import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.LinkAddress;
import android.net.NetworkUtils;


import android.os.Handler;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceScreen;
import android.provider.Settings.System;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.LinearLayout;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.settings.ext.DefaultWifiExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import java.net.Inet4Address;
import java.util.Collection;
import java.util.List;
import java.util.ArrayList;




public class WifiExt extends DefaultWifiExt implements AdapterView.OnItemSelectedListener {
    private static final String TAG = "WifiExt";
    static final String CMCC_SSID = "CMCC";
    static final String CMCC_AUTO_SSID = "CMCC-AUTO";
    static final int SECURITY_NONE = 0;
    static final int SECURITY_WEP = 1;
    static final int SECURITY_PSK = 2;
    /// M: security type @{
    static final int SECURITY_WPA_PSK = 3;
    static final int SECURITY_WPA2_PSK = 4;
    static final int SECURITY_EAP = 5;
    static final int SECURITY_WAPI_PSK = 6;
    static final int SECURITY_WAPI_CERT = 7;

    /* These values from from "wifi_eap_method" resource array */
    static final int WIFI_EAP_METHOD_PEAP = 0;
    static final int WIFI_EAP_METHOD_TLS = 1;
    static final int WIFI_EAP_METHOD_TTLS = 2;
    static final int WIFI_EAP_METHOD_SIM = 4;
    static final int WIFI_EAP_METHOD_AKA = 5;

    static final int WIFI_EAP_METHOD_NUM = 3;
    static final int WIFI_EAP_LAYOUT_OFF = 4;
    static final int WIFI_EAP_SKIP_NUM = 2;

    private static final String KEY_CONNECT_TYPE = "connect_type";
    private static final String KEY_PRIORITY_TYPE = "priority_type";
    private static final String KEY_PRIORITY_SETTINGS = "priority_settings";
    //specify whether settings will auto connect wifi 
    /* //remove auto connect
    private static final String KEY_CONNECT_AP_TYPE = "connect_ap_type";
    */
    private static final String KEY_SELECT_SSID_TYPE = "select_ssid_type";

    private Context mContext;
    private Switch mSwitch;

    //here priority means order of its priority, the smaller value, the higher priority
    private int mPriority = -1;
    private Spinner mPrioritySpinner;
    private TextView mNetworkNetmaskView;
    private String[] mPriorityArray;

    private int mNetworkId = -1;
    private int mCurrentPriority;

    
    //remove auto connect
    //private CheckBoxPreference mConnectApTypePref;
    //delete 

    
    private ListPreference mConnectTypePref;
    private CheckBoxPreference mPriorityTypePref;
    private Preference mPrioritySettingPref;

    private Preference mGatewayPref;
    private Preference mNetmaskPref;

    public WifiExt(Context context) {
        super(context);
        mContext = context;
        Xlog.d(TAG,"WifiExt");
    }
    //wifi enabler
    public void registerAirplaneModeObserver(Switch iSwitch) {
        Xlog.d(TAG,"registerAirplaneModeObserver()");
        mSwitch = iSwitch;
        mContext.getContentResolver().registerContentObserver(
                System.getUriFor(System.AIRPLANE_MODE_ON), true,
                mAirplaneModeObserver);
    }
    public void unRegisterAirplaneObserver() {
        Xlog.d(TAG,"unRegisterObserver()");
        mContext.getContentResolver().unregisterContentObserver(mAirplaneModeObserver);
    }
    public boolean getSwitchState() {
        boolean state = System.getInt(mContext.getContentResolver(),
                System.AIRPLANE_MODE_ON, 0) != 0;
        Xlog.d(TAG,"getSwitchState():" + (!state));
        return !state;
    }
    public void initSwitchState(Switch iSwitch) {
        mSwitch = iSwitch;
        if (getSwitchState() == false) {
            mSwitch.setEnabled(false);
        }
    }

    private ContentObserver mAirplaneModeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Xlog.d(TAG,"onChange(),switch new state:" + getSwitchState());
            mSwitch.setEnabled(getSwitchState());
        }
    };
    //wifi access point enabler
    public String getWifiApSsid() {
        return mContext.getString(R.string.wifi_tether_configure_ssid_default_for_cmcc);
    }
    //wifi config controller
    public boolean shouldAddForgetButton(String ssid, int security) {
        return true;
    }
    public void setAPNetworkId(int apNetworkId) {
        mNetworkId = apNetworkId;
    }
    public void setAPPriority(int apPriority) {
        mPriority = apPriority;
    }
    public View getPriorityView(/*View view, int priorityId, int setterId*/) {
        View view;
        LayoutInflater inflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        view = inflater.inflate(R.layout.wifi_priority_cmcc, null);
        int priorityType = System.getInt(mContext.getContentResolver(),
                System.WIFI_PRIORITY_TYPE, System.WIFI_PRIORITY_TYPE_DEFAULT);
        if (priorityType == System.WIFI_PRIORITY_TYPE_DEFAULT) {       
            view.setVisibility(View.GONE);          
        } else {
            WifiManager mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
            List<WifiConfiguration> mConfigs = mWifiManager.getConfiguredNetworks();
            int configuredApCount = mConfigs == null ? 0 : mConfigs.size();

            //view.findViewById(priorityId).setVisibility(View.VISIBLE);
            mPrioritySpinner = (Spinner)view.findViewById(R.id.cmcc_priority_setter);
            if (mPrioritySpinner != null) {
                mPrioritySpinner.setOnItemSelectedListener(this);
                if (mNetworkId != -1) {
                    mPriorityArray = new String[configuredApCount];
                } else {
                    //new configured AP, have highest priority by default
                    mPriorityArray = new String[configuredApCount + 1];
                }
                for (int i = 0;i < mPriorityArray.length;i++) {
                    mPriorityArray[i] = String.valueOf(i + 1);
                }
                ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                        mContext, android.R.layout.simple_spinner_item, mPriorityArray);
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                mPrioritySpinner.setAdapter(adapter);
                int priorityCount = mPrioritySpinner.getCount();
                int priorityOrder;
                if (mNetworkId != -1) {
                    priorityOrder = priorityCount - mPriority + 1;
                    mPriority = priorityOrder;
                } else {
                  //new configured AP will have highest priority by default
                    priorityOrder = 1;
                    mPriority = 1;
                }
                Xlog.d(TAG, "onCreate(), priorityOrder=" + priorityOrder + ", mPriority=" + mPriority);
                mPrioritySpinner.setSelection(priorityCount < priorityOrder ? (priorityCount - 1) : (priorityOrder - 1));
            }
        }
        return view;
    }
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        if (parent.equals(mPrioritySpinner)) {
            mPriority = position + 1;
            Xlog.d(TAG, "change AP priority manually");
        } 
    }
    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        //
    }
    public void setSecurityText(TextView view) {
        view.setText(mContext.getString(R.string.wifi_security_cmcc));
        Xlog.d(TAG,"set wifi_security_cmcc");
    }
    public String getSecurityText(String security) {
        return mContext.getString(R.string.wifi_security_cmcc);
    }

    public boolean shouldSetDisconnectButton() {
        final ConnectivityManager connectivity = (ConnectivityManager)
                    mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivity != null
            && connectivity.getNetworkInfo(ConnectivityManager.TYPE_WIFI).isConnected()) {
            return true;
        }
        return false;
    }
    public int getPriority() {
        return mPriority;
    }
    public void closeSpinnerDialog() {
        if (mPrioritySpinner != null && mPrioritySpinner.isPopupShowing()) {
            mPrioritySpinner.dismissPopup();
        }
    }
    public void setProxyText(TextView view) {
        view.setText(mContext.getString(R.string.proxy_exclusionlist_label_cmcc));
    }

//advanced wifi settings
    public void initConnectView(Activity activity,PreferenceScreen screen) {
      /* //remove auto connect
        mConnectApTypePref = new CheckBoxPreference(mContext);
        mConnectApTypePref.setTitle(R.string.wifi_connect_ap_title);
        mConnectApTypePref.setSummary(R.string.wifi_connect_ap_summary);
        mConnectApTypePref.setKey(KEY_CONNECT_AP_TYPE);
        mConnectApTypePref.setOnPreferenceChangeListener(mPreferenceChangeListener);
        screen.addPreference(mConnectApTypePref);
      */
        mConnectTypePref = new ListPreference(activity);
        mConnectTypePref.setTitle(mContext.getString(R.string.wifi_connect_type_title));
        mConnectTypePref.setEntries(mContext.getResources().getTextArray(R.array.wifi_connect_type_entries));
        mConnectTypePref.setEntryValues(mContext.getResources().getTextArray(R.array.wifi_connect_type_values));
        mConnectTypePref.setKey(KEY_CONNECT_TYPE);
        mConnectTypePref.setOnPreferenceChangeListener(mPreferenceChangeListener);
        screen.addPreference(mConnectTypePref);

        mPriorityTypePref = new CheckBoxPreference(mContext);
        mPriorityTypePref.setTitle(R.string.wifi_priority_type_title);
        mPriorityTypePref.setSummary(R.string.wifi_priority_type_summary);
        mPriorityTypePref.setKey(KEY_PRIORITY_TYPE);
        mPriorityTypePref.setOnPreferenceChangeListener(mPreferenceChangeListener);
        screen.addPreference(mPriorityTypePref);

        mPrioritySettingPref = new Preference(mContext);
        mPrioritySettingPref.setTitle(R.string.wifi_priority_settings_title);
        mPrioritySettingPref.setSummary(R.string.wifi_priority_settings_summary);
        mPrioritySettingPref.setKey(KEY_PRIORITY_SETTINGS);
        mPrioritySettingPref.setOnPreferenceClickListener(mPreferenceclickListener);
        screen.addPreference(mPrioritySettingPref);

        mPrioritySettingPref.setDependency(KEY_PRIORITY_TYPE);
    }
    public void initNetworkInfoView(PreferenceScreen screen) {
        mGatewayPref = new Preference(mContext);
        mGatewayPref.setTitle(mContext.getString(R.string.wifi_gateway));
        screen.addPreference(mGatewayPref);

        mNetmaskPref = new Preference(mContext);
        mNetmaskPref.setTitle(mContext.getString(R.string.wifi_network_netmask));
        screen.addPreference(mNetmaskPref);
    }
    public void refreshNetworkInfoView() {
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();

        String gateway = null;
        String netmask = null;
        DhcpInfo dhcpInfo = wifiManager.getDhcpInfo();
        Xlog.d(TAG, "!!!!!!!!!!! refreshNetworkInfoView() dhcpInfo = " + dhcpInfo);
        Xlog.d(TAG, "!!!!!!!!!!! refreshNetworkInfoView() wifiInfo = " + wifiInfo);
        if (wifiInfo != null) {
            if (dhcpInfo != null) {
                int netmask_int = 0;
                gateway = ipTransfer(dhcpInfo.gateway);
                Xlog.d(TAG, "!!!!!!!!!!! refreshNetworkInfoView() dhcpInfo.netmask = " + dhcpInfo.netmask);
                ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                LinkProperties prop = cm.getLinkProperties(ConnectivityManager.TYPE_WIFI);
                Collection<LinkAddress> linkAddresses = prop.getLinkAddresses();
                for(LinkAddress addr : linkAddresses){
                    if (addr.getAddress() instanceof Inet4Address){
                        netmask_int = NetworkUtils.prefixLengthToNetmaskInt(addr.getNetworkPrefixLength());
                    }
                }
                netmask = ipTransfer(netmask_int);
            }
        }
        String defaultText = mContext.getString(R.string.status_unavailable);
        mGatewayPref.setSummary(gateway == null ? defaultText : gateway);
        mNetmaskPref.setSummary(netmask == null ? defaultText : netmask);
    }
    private OnPreferenceChangeListener mPreferenceChangeListener = new OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String key = preference.getKey();
            Xlog.d(TAG,"key=" + key);
            /* //remove auto connect
            if (KEY_CONNECT_AP_TYPE.equals(key)) {
                boolean checked = ((Boolean) newValue).booleanValue();
                System.putInt(mContext.getContentResolver(), System.WIFI_CONNECT_AP_TYPE, 
                        checked ? System.WIFI_CONNECT_AP_TYPE_AUTO : System.WIFI_CONNECT_AP_TYPE_MANUL);
            } else */
            if (KEY_CONNECT_TYPE.equals(key)) {
                Xlog.d(TAG, "Wifi connect type is " + newValue);
                try {
                    System.putInt(mContext.getContentResolver(),
                            System.WIFI_CONNECT_TYPE, Integer.parseInt(((String) newValue)));
                    if (mConnectTypePref != null) {
                        CharSequence[] array = mContext.getResources().getTextArray(R.array.wifi_connect_type_entries);
                        mConnectTypePref.setSummary((String)array[Integer.parseInt(((String) newValue))]);
                    }
                } catch (NumberFormatException e) {
                    Xlog.d(TAG, "set Wifi connect type error ");
                    return false;
                }
                try {
                    System.putInt(mContext.getContentResolver(),
                            System.WIFI_SELECT_SSID_TYPE, Integer.parseInt(((String) newValue)));
                } catch (NumberFormatException e) {
                    Xlog.d(TAG, "set Wifi SSID reselect type error ");
                    return false;
                }
            } else if (KEY_PRIORITY_TYPE.equals(key)) {
                boolean checked = ((Boolean) newValue).booleanValue();
                System.putInt(mContext.getContentResolver(), System.WIFI_PRIORITY_TYPE, 
                        checked ? System.WIFI_PRIORITY_TYPE_MAMUAL : System.WIFI_PRIORITY_TYPE_DEFAULT);
            }
            return true;
        }
    };
    private Preference.OnPreferenceClickListener mPreferenceclickListener = new Preference.OnPreferenceClickListener() {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            Intent intent = new Intent("com.mediatek.OP01.PRIORITY_SETTINGS");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(intent);
            return true;
        }
    };

    public void initPreference(ContentResolver contentResolver) {
            /* //remove auto connect
            if (mConnectApTypePref != null) {
                mConnectApTypePref.setChecked(System.getInt(contentResolver, 
                        System.WIFI_CONNECT_AP_TYPE, System.WIFI_CONNECT_AP_TYPE_AUTO) == System.WIFI_CONNECT_AP_TYPE_AUTO);
            }
            */
            if (mConnectTypePref != null) {
                int value = System.getInt(contentResolver, System.WIFI_CONNECT_TYPE, System.WIFI_CONNECT_TYPE_AUTO);
                mConnectTypePref.setValue(String.valueOf(value));
                CharSequence[] array = mContext.getResources().getTextArray(R.array.wifi_connect_type_entries);
                mConnectTypePref.setSummary((String)array[value]);

                int value1 = System.getInt(contentResolver, System.WIFI_SELECT_SSID_TYPE, System.WIFI_SELECT_SSID_AUTO);
                if (value != value1) {
                    System.putInt(contentResolver, System.WIFI_SELECT_SSID_TYPE, value);
                }
            }
            if (mPriorityTypePref != null) {
                mPriorityTypePref.setChecked(System.getInt(contentResolver, 
                        System.WIFI_PRIORITY_TYPE, System.WIFI_PRIORITY_TYPE_DEFAULT) == System.WIFI_PRIORITY_TYPE_MAMUAL);
            }
    }
    public int getSleepPolicy(ContentResolver contentResolver) {
        return System.getInt(contentResolver, System.WIFI_SLEEP_POLICY, System.WIFI_SLEEP_POLICY_NEVER);
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
//access point
    private boolean isCmccAp(String ssid, int security) {
        if (CMCC_SSID.equals(ssid) && (security == SECURITY_NONE)) {
            return true;
        } else if (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security) {
            Xlog.d(TAG, "WifiExt isCmccAp() CMCC_AUTO is CmccAp");
            return true;
        }
        return false;
    }
    public int getApOrder(String currentSsid, int currentSecurity, String otherSsid, int otherSecurity) {
        if (isCmccAp(currentSsid,currentSecurity) && isCmccAp(otherSsid,otherSecurity)) {
            Xlog.d(TAG, "WifiExt getaporder() currentSsid = " + currentSsid + ";otherSsid = " + otherSsid);
            if (currentSsid == otherSsid) {
                return 0;
            }

            if(CMCC_AUTO_SSID.equals(currentSsid)) {
                return -1;
            } else {
                return 1;  
            }
        } else {
            if (isCmccAp(currentSsid,currentSecurity)) {
                if (!isCmccAp(otherSsid,otherSecurity)) {
                    return -1;
                }
            } else if (isCmccAp(otherSsid,otherSecurity)) {
                return 1;
            }
        }

        return 0;
    }
    
    private static String addQuote(String s) {
        return "\"" + s + "\"";
    }
    private int getSecurity(WifiConfiguration config) {
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_PSK)) {
            return SECURITY_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_EAP) ||
                config.allowedKeyManagement.get(KeyMgmt.IEEE8021X)) {
            return SECURITY_EAP;
        }
        /// M: support wapi psk/cert @{
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_PSK)) {
            return SECURITY_WAPI_PSK;
        }

        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_CERT)) {
            return SECURITY_WAPI_CERT;
        }
        
        if (config.wepTxKeyIndex >= 0 && config.wepTxKeyIndex < config.wepKeys.length 
                && config.wepKeys[config.wepTxKeyIndex] != null) {
            return SECURITY_WEP;
        }
        ///@}
        return (config.wepKeys[0] != null) ? SECURITY_WEP : SECURITY_NONE;
    }
    
     public void setEapMethodArray(ArrayAdapter adapter, String ssid, int security) {
        if (FeatureOption.MTK_EAP_SIM_AKA) {
            Xlog.d(TAG,"[skyfyx]setEapMethodArray");
            if (ssid != null && (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security)) {
                String[] eapString = mContext.getResources().getStringArray(R.array.wifi_eap_method_values);
	            adapter.clear();
	            for (int i = 0; i < WIFI_EAP_METHOD_NUM; i++) {
	                adapter.insert(eapString[i], i);
	            }
            }
        }
    }
    
    public void hideWifiConfigInfo(Builder builder , Context context) {
        if (builder != null) {
            Xlog.d(TAG,"hideWifiConfigInfo");
            String ssid =  builder.getSsid();
            int security = builder.getSecurity();
            int networkId = builder.getNetworkId();
            boolean edit = builder.getEdit();
            View view = (View)builder.getViews();
            List<View> lists = new ArrayList<View>();
            Resources res = null;
            res = context.getResources();
            String packageName = context.getPackageName();
            lists.add(view.findViewById(res.getIdentifier("info", "id", packageName)));
            lists.add(view.findViewById(res.getIdentifier("priority_field", "id", packageName)));
            lists.add(view.findViewById(res.getIdentifier("proxy_settings_fields", "id", packageName)));
            lists.add(view.findViewById(res.getIdentifier("ip_fields", "id", packageName)));
            lists.add(view.findViewById(res.getIdentifier("wifi_advanced_togglebox", "id", packageName)));
            lists.add(view.findViewById(res.getIdentifier("eap", "id", packageName)));
           if (ssid == null || !("CMCC-AUTO".equals(ssid) && SECURITY_EAP == security)) {
               return;
            }
            if ((networkId == WifiConfiguration.INVALID_NETWORK_ID) || (networkId != WifiConfiguration.INVALID_NETWORK_ID && edit)) {
                if (FeatureOption.MTK_EAP_SIM_AKA) {
            	    for (int i = 0; i <lists.size()-2; i++) {
                    ((View)lists.get(i)).setVisibility(View.GONE);
                }
	                LinearLayout advanced = ((LinearLayout)lists.get(lists.size()-1));
				    int count = advanced.getChildCount();
	                Xlog.d(TAG, "[skyfyx] hideWifiConfigInfo() WifiExt eap count" + count);
	                for (int j = WIFI_EAP_SKIP_NUM; j <count; j++) {
	                    ((View)advanced.getChildAt(j)).setVisibility(View.GONE);
	                }
                } else {
                    Xlog.d(TAG, "hideWifiConfigInfo() WifiExt NO SUPPORT eap");
                    for (int i = 0; i <lists.size()-1; i++) {
	                    ((View)lists.get(i)).setVisibility(View.GONE);
                    }
	            }
            }
        }
    }

     public int getEapMethodbySpinnerPos(int spinnerPos, String ssid, int security) {
        Xlog.d(TAG, "[skyfyx]WifiExt getEapMethodbySpinnerPos ENTER" + spinnerPos);
        Xlog.d(TAG, "[skyfyx]WifiExt getEapMethodbySpinnerPos ssid" + ssid);
        if (FeatureOption.MTK_EAP_SIM_AKA) {
            Xlog.d(TAG, "WifiExt getEapMethodbySpinnerPos is CMCC-AUTO = " + CMCC_AUTO_SSID.equals(ssid));
            Xlog.d(TAG, "WifiExt getEapMethodbySpinnerPos is eap = " + (SECURITY_EAP == security));
            if (ssid != null && (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security)) {
                if (spinnerPos == 2) {
                    spinnerPos = WIFI_EAP_METHOD_AKA;
                } else {
                    spinnerPos = (spinnerPos==1) ? WIFI_EAP_METHOD_SIM:WIFI_EAP_METHOD_PEAP;
                }
            }
        }
        Xlog.d(TAG, "[skyfyx]WifiExt getEapMethodbySpinnerPos EXIT" + spinnerPos);
        if(spinnerPos < 0) {
            spinnerPos = 0;
        }
        return spinnerPos;
    }

    public int getPosByEapMethod(int spinnerPos, String ssid, int security) {
        Xlog.d(TAG, "WifiExt getPosByEapMethod ENTER" + spinnerPos);
        Xlog.d(TAG, "WifiExt getPosByEapMethod ssid" + ssid);
        if (FeatureOption.MTK_EAP_SIM_AKA) {
            Xlog.d(TAG, "WifiExt getPosByEapMethod is CMCC-AUTO = " + CMCC_AUTO_SSID.equals(ssid));
            Xlog.d(TAG, "WifiExt getPosByEapMethod is eap = " + (SECURITY_EAP == security));
            if (ssid != null && (CMCC_AUTO_SSID.equals(ssid) && SECURITY_EAP == security)) {
                if (spinnerPos == WIFI_EAP_METHOD_SIM) {
                    spinnerPos = 1;
                } else {
                    spinnerPos = (spinnerPos == WIFI_EAP_METHOD_AKA) ? 2:0;
                }
            }
        }
        Xlog.d(TAG, "WifiExt getPosByEapMethod EXIT" + spinnerPos);
        if(spinnerPos < 0) {
            spinnerPos = 0;
        }
        return spinnerPos;
    }
}


