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

import static android.provider.Telephony.Intents.SPN_STRINGS_UPDATED_ACTION;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.test.InstrumentationTestCase;
import android.view.KeyEvent;

import com.mediatek.settings.ext.IStatusExt;
import com.mediatek.settings.ext.DefaultStatusExt;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class OP01DeviceStatusTests extends InstrumentationTestCase {
    
    private Instrumentation mInst;
    private Context mContext;
    private IStatusExt mExt;
    private static final String TAG =  "OP01DeviceStatusTests";
    

  
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        
        mExt = (IStatusExt)PluginManager.createPluginObject(mContext, IStatusExt.class.getName());
    }

    public void testCase01EntryDeviceStatusScreen() {

            Xlog.i(TAG, "testCase01_EntryDeviceStatusScreen");

            
            assertNotNull(mInst);
            assertNotNull(mContext);

            
            IntentFilter filter = new IntentFilter();
            mExt.addAction(filter,SPN_STRINGS_UPDATED_ACTION);
            try {
                Class <?> iClass = Class.forName("com.android.settings.deviceinfo.Status");
				//Class <?> superClass = iClass.getSuperclass();
				Object iobject = iClass.newInstance();
                //Method findPref = superClass.getDeclaredMethod("findPreference", new Class[] { String.class });
                //findPref.setAccessible(true);
                //Preference p = (Preference)findPref.invoke(iobject, "operator_name");
			    //Preference p = ((PreferenceActivity)iobject).findPreference("operator_name");
			    Preference p = new Preference(mContext);
                mExt.updateOpNameFromRec(p, "CMCC"); 
				
                mExt.updateServiceState(p, "on");
                assertNotNull(p);
                assertNotNull(p.getSummary());
                assert(p.getSummary().equals("CMCC"));
           /*} catch (NoSuchMethodException e) {
                Xlog.i(TAG, "catch NoSuchMethodException");   
                throw new RuntimeException(e);*/
           } catch (IllegalAccessException e) {
                Xlog.i(TAG, "catch IllegalAccessException"); 
		        throw new RuntimeException(e);
           } catch (ClassNotFoundException e) {
                Xlog.i(TAG, "catch ClassNotFoundException"); 
		        throw new RuntimeException(e);
           } catch (InstantiationException e) {
                Xlog.i(TAG, "catch InstantiationException"); 
		   	    throw new RuntimeException(e);
           /*} catch (InvocationTargetException e) {
                Xlog.i(TAG, "catch InvocationTargetException"); 				
		   	    throw new RuntimeException(e);*/
           }
        }
    
    
    @Override
    public void tearDown() throws Exception {
        super.tearDown();
        
    }
}
