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
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.media.AudioManager;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.mediatek.settings.ext.*;
import com.mediatek.audioprofile.AudioProfileManager;
import com.mediatek.audioprofile.AudioProfileManager.Scenario;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.jayway.android.robotium.solo.Solo;

import java.util.List;

public class OP01AudioProfileTest extends OP01PluginBase {

    private static final String TAG = "OP01AudioProfileTest";
    private Solo mSolo;
    private PreferenceActivity mActivity;
    private Context mContext;
    private ContentResolver mCr;
    private String mKey;
    
    private AudioProfileManager mProfileManager;
    private AudioManager mAudioManager;
    private String mActiveProfileKey;
    private String mGeneralKey;
    private String mOutdoorKey;
    //private AudioProfilePreference mGeneralPreference;
    //private AudioProfilePreference mPreference;
    private Instrumentation mInstr;
    private InputMethodManager inputMethodManager;
    private IAudioProfileExt mExt;
    
    private final Object mSync = new Object();
    private boolean mHasNotify;
    int mCurrentRingerMode = AudioManager.RINGER_MODE_NORMAL;
    private static final int MENUID_ENABLE= 2;
    private static final int MENUID_RENAME = 3;
    private static final int MENUID_DELETE = 4;
    
    private static final int GENERAL_INDEX = 0;
    private static final int SILENT_INDEX = 1;
    private static final int MEETING_INDEX = 2;
    private static final int OUTDOOR_INDEX = 3;
    
    private static final int MENUID_ADD = 0;
    
    private static Class<?> launcherActivityClass;
    private static final String PACKAGE_ID_STRING = "com.android.settings";
    private static final String ACTIVITY_FULL_CLASSNAME = "com.android.settings.Settings";

    static {
        try {
            launcherActivityClass = Class
                    .forName(ACTIVITY_FULL_CLASSNAME);
        } catch (ClassNotFoundException exception) {
            throw new RuntimeException(exception);
        }
    }

    public OP01AudioProfileTest() {
        super(PACKAGE_ID_STRING, launcherActivityClass);
    }

    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstr = getInstrumentation();
        mActivity = (PreferenceActivity)getActivity();
        mSolo = new Solo(mInstr, mActivity);

        mContext = mInstr.getTargetContext();
        mCr = mContext.getContentResolver();
        
        inputMethodManager = (InputMethodManager)mContext.getSystemService(Context.INPUT_METHOD_SERVICE);
        mAudioManager = (AudioManager) mActivity.getSystemService(Context.AUDIO_SERVICE);
        mProfileManager = (AudioProfileManager)mContext.getSystemService(Context.AUDIOPROFILE_SERVICE);
        mActiveProfileKey = mProfileManager.getActiveProfileKey();
        mGeneralKey = AudioProfileManager.PROFILE_PREFIX + Scenario.GENERAL.toString().toLowerCase();
        mOutdoorKey = AudioProfileManager.PROFILE_PREFIX + Scenario.OUTDOOR.toString().toLowerCase();
        
        mKey = AudioProfileManager.PROFILE_PREFIX + Scenario.GENERAL.toString().toLowerCase();
        mExt = (IAudioProfileExt)PluginManager.createPluginObject(mContext, IAudioProfileExt.class.getName());
        
        setActivityInitialTouchMode(false);
    }
    
    public void test01SetProfileValue() {
        Log.i(TAG,"test01SetProfileValue");
        assertNotNull(mAudioManager);
        //assertEquals(mGeneralKey, mActiveProfileKey);
        assertEquals(mProfileManager.getProfileCount(),AudioProfileManager.PREDEFINED_PROFILES_COUNT);

        
        if(mSolo.searchText("Audio profiles")) {
            mSolo.clickOnText("Audio profiles");
            
            mSolo.clickOnRadioButton(GENERAL_INDEX);
            assertEquals(mGeneralKey, mProfileManager.getActiveProfileKey());
            mSolo.sleep(1000);
            
            mSolo.clickOnRadioButton(SILENT_INDEX);    
            mSolo.sleep(1000);
            
            mSolo.clickOnRadioButton(MEETING_INDEX);
            mSolo.sleep(1000);
            mSolo.clickOnRadioButton(OUTDOOR_INDEX);
            assertEquals(mOutdoorKey, mProfileManager.getActiveProfileKey());
            
        }
        mInstr.waitForIdleSync();
    }
    
    /**
     * Enable Outdoor profile by context menu, and PreviousProfile is General.
     */
    public void test02SetOutdoorRingVolume() {
        Log.i(TAG,"test02_SetOutdoorVolume");

        //View cotentView = mSolo.getCurrentViews().get(0);
        
        Resources resource = mActivity.getResources();
        String packageName = mActivity.getPackageName();        
        int ringDescription = resource.getIdentifier("volume_ring_description", "string", packageName);        
        int outdoorSettingsTitle = resource.getIdentifier("outdoor_settings_title", "string", packageName);        
        int allVolumeTitle = resource.getIdentifier("all_volume_title", "string", packageName);        
        Log.i(TAG,"ringDescription" + ringDescription);
        Log.i(TAG, "PackageName:" + packageName);
        Log.i(TAG, "ringDescriptionStr:" + resource.getString(ringDescription));
        
        if(mSolo.searchText("Audio profiles")) {
            mSolo.clickOnText("Audio profiles");
            //mSolo.clickOnText(resource.getString(outdoorSettingsTitle)); 
            mSolo.clickOnText("General");
            mSolo.clickOnText(resource.getString(allVolumeTitle));
            
            int streamType = AudioProfileManager.STREAM_RING;
            int expect = mProfileManager.getStreamVolume(mKey, streamType) - 1;
            

            TextView tv = (TextView) mSolo.getText(resource.getString(ringDescription));
            LinearLayout parentLinearLayout = (LinearLayout) tv.getParent().getParent();
            LinearLayout childLinearLayout1 = (LinearLayout) parentLinearLayout.getChildAt(2);
            LinearLayout childLinearLayout2 = (LinearLayout) childLinearLayout1.getChildAt(1);
            SeekBar ringtoneSeekbar = (SeekBar) childLinearLayout2.getChildAt(1);
            ringtoneSeekbar.incrementProgressBy(-1);
            
            mSolo.clickOnButton(resource.getString(android.R.string.ok));
			mSolo.sleep(500);
            int audioResult = mProfileManager.getStreamVolume(mKey, streamType);
			Log.i(TAG, "expect = " + expect + "result = " + audioResult);
            assertEquals(expect, audioResult);
        }
        mInstr.waitForIdleSync();
        
    }
           
    
    @Override
    public void tearDown()throws Exception { 
        Log.i("SettingsTest","Audioprofile tearDown");
        mProfileManager.reset();
        Thread.sleep(5000);
        super.tearDown();
        try {
            mSolo.finishOpenedActivities();
        } catch (Exception e){
			Log.i(TAG, "catch exception");
        }
    }

}
