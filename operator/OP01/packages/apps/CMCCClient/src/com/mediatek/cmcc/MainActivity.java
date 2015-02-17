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

package com.mediatek.cmcc;

import android.app.ListActivity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class MainActivity extends ListActivity {
    private SimpleAdapter mAdapter;
    private ArrayList<Map<String, Object>> mListData = new ArrayList<Map<String, Object>>();
    private String[] mNameArray;
    private Resources mRes;
    private int[] mDrawableArray = { R.drawable.cmcc_service_guide, R.drawable.cmcc_service_hotline, 
            R.drawable.cmcc_sms_icon, R.drawable.cmcc_on_hands_icon,
            R.drawable.cmcc_manager_icon, R.drawable.cmcc_manager_settings };

    private String mServiceNum = "10086";
    private static final String TAG = "MainActivity";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_activity);

        mRes = this.getResources();
        mServiceNum = mRes.getString(R.string.service_num);
        initListData();

        mAdapter = new SimpleAdapter(this, mListData, R.layout.list_item, new String[] { "image", "name" }, new int[] {
                R.id.list_item_image, R.id.list_item_textview });

        setListAdapter(mAdapter);

        Log.v(TAG, "Adapter has been set.");
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        Log.v(TAG, "ListView position all is clicked.");
        switch (position) {
        case 0:
            startActivity(new Intent(this, AfterServiceActivity.class));
            //Log.v(TAG, "ListView position 0 is clicked.");
            break;
        case 1:
            makeCall(mServiceNum);
            break;
        case 2:
            sendSms();
            break;
        case 3:
            goWeb();
            break;
        case 4:
            callCustomerManager();
            break;
        case 5:
            startActivity(new Intent(this, ManagerSetActivity.class));
            //setListAdapter(mAdapter);
            break;
        default:
            break;
        }
    }

    private void initListData() {
        final Resources resource = this.getResources();
        mNameArray = resource.getStringArray(R.array.names);
        int length = mNameArray.length;
        for (int i = 0; i < length; i++) {
            Map<String, Object> item = new HashMap<String, Object>();
            item.put("image", mDrawableArray[i]);
            item.put("name", mNameArray[i]);
            mListData.add(item);
            Log.v(TAG, "set item." + " i=" + i);
        }
    }

    private void callCustomerManager() {
        try {
            SharedPreferences preferences = getSharedPreferences("user", MainActivity.MODE_PRIVATE);
            String number = preferences.getString("number", "");
            if (number == null || TextUtils.isEmpty(number)) {
                startActivity(new Intent(this, ManagerSetActivity.class));
            } else {
                makeCall(number);
            }
        } catch (ActivityNotFoundException e) {
            e.printStackTrace();
            Log.v("MainActivity", "callCustomerManager Exception");
        }        
    }

    private void makeCall(String number) {
        try {
            Intent myDialIntent = new Intent(Intent.ACTION_DIAL, Uri.parse("tel://" + number));
            startActivity(myDialIntent);
        } catch (ActivityNotFoundException e) {
            e.printStackTrace();
            Log.v("MainActivity", "makeCall Exception");
        }
    }

    private void sendSms() {
        try {
            TelephonyManager tm = (TelephonyManager) this.getSystemService(TELEPHONY_SERVICE);
            String tips;
            // Whether the sim card's status is readable.
            if (TelephonyManager.SIM_STATE_READY == tm.getSimState()) {
                String strDestAddress = mServiceNum;
                String strMessage = mServiceNum;
                SmsManager smsManager = SmsManager.getDefault();
                smsManager.sendTextMessage(strDestAddress, null, strMessage, null, null);
                tips = mRes.getString(R.string.sms_send_tips);
            } else {
                tips = mRes.getString(R.string.sim_card_tips);
            }

            Toast.makeText(this, tips, Toast.LENGTH_SHORT).show();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            Log.v("MainActivity", "sendSms Exception");
        }
    }

    private void goWeb() {

        try {
            boolean flag = false;
            ConnectivityManager cwjManager = (ConnectivityManager) this.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cwjManager.getActiveNetworkInfo() != null) {
                flag = cwjManager.getActiveNetworkInfo().isAvailable();
            }

            if (!flag) {
                Toast.makeText(this, mRes.getString(R.string.web_not_available), Toast.LENGTH_SHORT).show();
                return;
            }
            Uri uri = Uri.parse(mRes.getString(R.string.service_web));
            Intent it = new Intent(Intent.ACTION_VIEW, uri);
            startActivity(it);
        } catch (ActivityNotFoundException e) {
            e.printStackTrace();
            Log.v("MainActivity", "goWeb Exception");
        }
    }
}
