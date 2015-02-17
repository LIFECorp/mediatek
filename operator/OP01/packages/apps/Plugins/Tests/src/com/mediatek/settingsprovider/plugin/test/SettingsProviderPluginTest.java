package com.mediatek.settingsprovider.plugin.test;

import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;
import android.test.InstrumentationTestCase;

import com.mediatek.op01.plugin.R;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.providers.settings.ext.IDatabaseHelperExt;
import com.mediatek.settingsprovider.plugin.DatabaseHelperExt;

public class SettingsProviderPluginTest extends InstrumentationTestCase {
    
    private static DatabaseHelperExt sDb01Ext = null;
    private Context mContext;
    private ContentResolver mCr;
    @Override
    public void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        mCr = mContext.getContentResolver();
        Object plugin = PluginManager.createPluginObject(mContext, IDatabaseHelperExt.class.getName());
        if (plugin instanceof DatabaseHelperExt) {
            sDb01Ext = (DatabaseHelperExt) plugin;
        }
    }
    
    // test the function of getResStr(Context context, String name, int defResId)
    public void test01_getStringValue() {
        // just use test data
        String testName = "testString";
        int testRefId = 0;
        String value = sDb01Ext.getResStr(mContext, testName, testRefId);
        assertTrue(value == null);
    }
   
    // test the function of getResBoolean(Context context, String name, int defResId)
    public void test02_getBooleanValue() {
        // test the auto_time
        String autoTime = Settings.System.AUTO_TIME;
        int autoTimeId = R.bool.def_auto_time_op01;
        String autoTimevalue = sDb01Ext.getResBoolean(mContext, autoTime, autoTimeId);
        assertEquals("0",autoTimevalue);
        // test the auto_time_zone
        String autoTimeZone = Settings.System.AUTO_TIME_ZONE;
        int autoTimeZoneId = R.bool.def_auto_time_zone_op01;
        String autoTimeZonevalue = sDb01Ext.getResBoolean(mContext, autoTimeZone, autoTimeZoneId);
        assertEquals("0",autoTimeZonevalue);
        // test the haptic_feedback_enabled
        String feedback = Settings.System.HAPTIC_FEEDBACK_ENABLED;
        int feedbackId = R.bool.def_haptic_feedback_op01;
        String feedbakcValue = sDb01Ext.getResBoolean(mContext, feedback, feedbackId);
        assertEquals("0",feedbakcValue);
        // test the haptic_feedback_enabled
        String batteryPer = Settings.Secure.BATTERY_PERCENTAGE;
        int batteryId = R.bool.def_battery_percentage_op01;
        String batteryValue = sDb01Ext.getResBoolean(mContext, batteryPer, batteryId);
        assertEquals("1",batteryValue);
        // test the others
        String testName = "testBool";
        int testRefId = 0;
        String value = sDb01Ext.getResBoolean(mContext, testName, testRefId);
        assertTrue(value == null);
    }
    
    // test the function of getResInteger(Context context, String name, int defResId)
    public void test03_getIntegerValue() {
        // just use test data
        String testName = "testInteger";
        int testRefId = 0;
        String value = sDb01Ext.getResInteger(mContext, testName, testRefId);
        assertTrue(value == null);
    }
    
    // test the function of getResFraction(Context context, String name, int defResId,int defBase)
    public void test04_getFractionValue() {
        // just use test data
        String testName = "testFract";
        int testRefId = 0;
        int defBaseId = 0;
        String value = sDb01Ext.getResFraction(mContext, testName, testRefId, defBaseId);
        assertTrue(value == null);
    }   
   
    @Override    
    public void tearDown() throws Exception {
        super.tearDown();
        sDb01Ext = null;
    }

}
