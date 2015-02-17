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
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.View;

import com.mediatek.settings.ext.IApnSettingsExt;
import com.jayway.android.robotium.solo.Solo;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.List;
import java.lang.reflect.*;


public class OP01ApnSettingsTests extends OP01PluginBase {
    private Instrumentation mInst;
    private Context mContext;
    PreferenceGroup mApnList;
    private Preference mPrf;
    private List<SimInfoRecord> mSimList = null;
    private static final String TAG =  "OP01ApnSettingsTests";
    private Solo mSolo;
    private PreferenceActivity mActivity;
    
    IApnSettingsExt mExt;

    private static Class<?> launcherActivityClass;
    private static final String PACKAGE_ID_STRING = "com.android.settings";
    private static final String ACTIVITY_FULL_CLASSNAME = "com.android.settings.ApnSettings";

    
    static {
        try {
            launcherActivityClass = Class.forName(ACTIVITY_FULL_CLASSNAME);
        } catch (Exception exception) {
            throw new RuntimeException(exception);
        } 
    }
    // Constructor
    public OP01ApnSettingsTests() {
        super(PACKAGE_ID_STRING, launcherActivityClass);
    }
  
    // Sets up the test environment before each test.
    @Override
    protected void setUp() throws Exception {      
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = (PreferenceActivity)getActivity(); 
        mSolo = new Solo(mInst, mActivity);
        
        Intent i = new Intent();        
        mSimList = SimInfoManager.getInsertedSimInfoList(mContext);
        Log.i(TAG, "mSimList size : "+mSimList.size());
        if (mSimList == null || mSimList.size() == 0) {
            return;
        }
        
        SimInfoRecord simInfo = mSimList.get(0);            
        int simSlot = simInfo.mSimSlotId;
        Log.i(TAG, "Slot = "+simSlot); 
        i.putExtra("slotid", simSlot);
        setActivityIntent(i);
        setActivityInitialTouchMode(false);

        mApnList = (PreferenceGroup) mActivity.findPreference("apn_list");
        
        mExt = (IApnSettingsExt)PluginManager.createPluginObject(mContext, IApnSettingsExt.class.getName());

    }

    public void testCase01_EditApnType() {
        mSimList = SimInfoManager.getInsertedSimInfoList(mContext);
        Log.i(TAG, "mSimList size : " + mSimList.size());
        if (mSimList == null || mSimList.size() == 0) {
            return;
        }

        //View cotentView = mSolo.getCurrentViews().get(0);
        Resources resource = mActivity.getResources();
        String packageName = mActivity.getPackageName();        

        int orangeId = resource.getIdentifier("apn_type_orange", "array", packageName);
        //int cmccId = resource.getIdentifier("com.mediatek.settings.plugin.R.apn_type_cmcc", "array", "com.mediatek.settings.plugin");
        int generalId = resource.getIdentifier("apn_type_generic", "array", packageName);
        String[] apnTypeString = {"default","mms","supl","wap","net","cmmail"} ;
        for (String apnType : apnTypeString) {
            Log.i(TAG, "apnTypeString: " + apnType);
        }
        int ranIndex = 0;
        int count = mApnList.getPreferenceCount();
        
        Log.i(TAG, "APN List count : " + count);
        if (count > 0) {
            while (ranIndex < count) {
                if (isEditPreference(ranIndex)) {
                    
                    mSolo.clickInList(ranIndex + 1);
                    
                    mSolo.clickInList(3);
                    
                    String[] apnTypePluginString = mExt.getApnTypeArray(mContext,generalId, false);
                    
                    sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
                    sendKeys(KeyEvent.KEYCODE_ENTER);
                    mSolo.sleep(500);
                    sendKeys(KeyEvent.KEYCODE_ENTER);
                    
                    mSolo.clickOnButton(mActivity.getString(android.R.string.ok));
                    mInst.waitForIdleSync();
                    
                    int i = 0;
                    for (String apn : apnTypePluginString) {
                        Log.i(TAG, "apnTypePluginString : " + apn);
                        assertNotNull(apn);
                        assertNotNull(apnTypeString[i]);
                        assert(apn.equals(apnTypeString[i]));
                        i++;
                    }
                    break;
                }
                ranIndex++;
            }
        }
        
    }
    
    
    private boolean isEditPreference(int index) {
        mPrf = mApnList.getPreference(index);
        String smy = mPrf.getSummary().toString();
        Log.i(TAG, "mPrf(): " + mPrf.getSummary());
        Boolean res = true;
        try {
            Class pc = Class.forName("com.android.settings.ApnPreference");
            Field field = pc.getDeclaredField("mEditable");
            field.setAccessible(true);
            res = (Boolean) field.get(mPrf);
            Log.i(TAG, "res: " + res);
            } catch (IllegalAccessException e) {            
                Log.e(TAG, "Fail to access private variable");        
            } catch (NoSuchFieldException e) {             
                Log.e(TAG, "Fail to get private field");          
            } catch (ClassNotFoundException e) {
                Log.e(TAG, "Class not found");
            }
        return res;
    }

   /*
     * (non-Javadoc)
     * @see android.test.ActivityInstrumentationTestCase2#tearDown()
     */   
    @Override
    public void tearDown() throws Exception {
        super.tearDown();
        if (mActivity!=null) {
            mActivity.finish();
        }
    }
}
