package com.mediatek.cmcc.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.Cursor;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;

import com.jayway.android.robotium.solo.Solo;

import com.mediatek.cmcc.AfterServiceActivity;
import com.mediatek.cmcc.MainActivity;
import com.mediatek.cmcc.ManagerSetActivity;
import com.mediatek.cmcc.R;

// Build apk:
// mk -t -o=MTK_AUTO_TEST=yes mm mediatek/operator/OP01/packages/apps/CMCCClient
// or mk mm mediatek/operator/OP01/packages/apps/CMCCClient/tests
// Install apk:
// adb install -r out/target/product/mt6589tdv1_phone/data/app/CustomerServiceTests.apk
// Run apk:
// adb shell am instrument -w com.mediatek.cmcc.tests/.CustomerServiceTestRunner

// Run apk to generated code coverage data to /data/data/com.mediatek.cmcc/files/coverage.ec:
// adb shell am instrument -e coverage true -w com.mediatek.cmcc.tests/android.test.InstrumentationTestRunner
// Put the emma.jar, coverage.ec, coverage.em under the same directory and execute the following command under this directory:
// java -cp emma.jar emma report -r html -in coverage.ec -in coverage.em
// java -cp emma.jar emma report -r xml -in coverage.ec -in coverage.em to generate coverage.xml
// File path:
// emma.jar is from  alps/external/emma/lib
// coverage.ec is exported from data/data/com.mediatek.cmcc/files by DDMS
// coverage.em is from alps/out/target/common/obj/APPS/CMCCClient_intermediates/
// Generate junit-report.xml
// adb shell am instrument -e coverage true -w com.mediatek.cmcc.tests/com.zutubi.android.junitreport.JUnitReportTestRunner
// junit-report.xml is exported from data/data/com.mediatek.cmcc/files by DDMS
public class CustomerServiceTest extends ActivityInstrumentationTestCase2<MainActivity> {

    private static final String TAG = "CustomerServiceTest";
    private Instrumentation mInstrumentation = null;
    private MainActivity mMainActivity = null;
    private Solo mSolo = null;
    private ActivityMonitor mActivityMonitor = null;
    private Context mContext = null;

    private static final int ACTIVITY_CREATED_TIME = 5000;
    private static final int ACTIVITY_DISPLAY_TIME = 2000;
    private static final int ACTIVITY_WAIT_TIME = 1000;

    private static final String LAST_ENTER_STRING = "Mediatek";
    private static final String LAST_ENTER_NUMBER = "01082800818";

    public CustomerServiceTest() {
        super("com.mediatek.cmcc", MainActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        //mContext = mInstrumentation.getContext();
        mContext = mInstrumentation.getTargetContext();
        
        mMainActivity = (MainActivity)getActivity();
        mSolo = new Solo(mInstrumentation, mMainActivity);
        //mContext = mMainActivity.getApplicationContext();

        assertNotNull(mInstrumentation);
        assertNotNull(mContext);
        assertNotNull(mMainActivity);
        assertNotNull(mSolo);
    }

    @Override
    protected void tearDown() throws Exception {
        if (mActivityMonitor != null) {
            mInstrumentation.removeMonitor(mActivityMonitor);
            mActivityMonitor = null;
        }
        mSolo.finishOpenedActivities();

        super.tearDown();
    }

    public void testCase01ServiceGuide() {
        Log.i(TAG, ">> testCase01ServiceGuide begin");

        mActivityMonitor = mInstrumentation.addMonitor(AfterServiceActivity.class.getName(), null, false);
        mSolo.clickInList(1);
        mSolo.sleep(ACTIVITY_DISPLAY_TIME);
        Activity newActivity = mActivityMonitor.waitForActivityWithTimeout(ACTIVITY_CREATED_TIME);

        try {
            assertNotNull(newActivity);
        } finally {
            if (newActivity != null) {
                newActivity.finish();
                newActivity = null;
            }
        }

        Log.i(TAG, "<< testCase01ServiceGuide end");
    }

    public void testCase02SendSMS() {
        Log.i(TAG, ">> testCase02SendSMS begin");

        mSolo.clickInList(3);
        assertTrue(mSolo.searchText(mSolo.getString(R.string.sms_send_tips)) ||
            mSolo.searchText(mSolo.getString(R.string.sim_card_tips)));

        Log.i(TAG, "<< testCase02SendSMS end");
    }

    public void testCase03CustomerManager() {
        Log.i(TAG, ">> testCase03CustomerManager begin");

        Editor editor = mInstrumentation.getTargetContext().getSharedPreferences("user", Context.MODE_WORLD_READABLE).edit();
        editor.putString("name", null);
        editor.putString("number", null);
        editor.commit();

        mActivityMonitor = mInstrumentation.addMonitor(ManagerSetActivity.class.getName(), null, false);
        mSolo.clickInList(5);
        mSolo.sleep(ACTIVITY_DISPLAY_TIME);
        Activity newActivity = mActivityMonitor.waitForActivityWithTimeout(ACTIVITY_CREATED_TIME);

        try {
            assertNotNull(newActivity);
        } finally {
            if (newActivity != null) {
                newActivity.finish();
                newActivity = null;
            }
        }

        Log.i(TAG, "<< testCase03CustomerManager end");
    }

    public void testCase04CustomerServiceSetting() {
        Log.i(TAG, ">> testCase04CustomerServiceSetting begin");

        mActivityMonitor = mInstrumentation.addMonitor(ManagerSetActivity.class.getName(), null, false);
        mSolo.clickInList(6);
        mSolo.sleep(ACTIVITY_DISPLAY_TIME);
        Activity newActivity = mActivityMonitor.waitForActivityWithTimeout(ACTIVITY_CREATED_TIME);

        try {
            assertNotNull(newActivity);
        } finally {
            if (newActivity != null) {
                newActivity.finish();
                newActivity = null;
            }
        }

        Log.i(TAG, "<< testCase04CustomerServiceSetting end");
    }

    public void testCase05SaveSetting() {
        Log.i(TAG, ">> testCase05SaveSetting begin");

        mSolo.clickInList(6);
        mSolo.assertCurrentActivity("It's not ManagerSetActivity", ManagerSetActivity.class);

        mSolo.clickOnEditText(0);
        mSolo.clearEditText(0);
        mSolo.enterText(0, LAST_ENTER_STRING);
        mSolo.sleep(ACTIVITY_WAIT_TIME);

        mSolo.clickOnEditText(1);
        mSolo.clearEditText(1);
        mSolo.enterText(1, LAST_ENTER_NUMBER);
        mSolo.sleep(ACTIVITY_WAIT_TIME);

        mSolo.clickOnButton(mSolo.getString(R.string.set_save_btn));
        assertTrue(mSolo.searchText(mSolo.getString(R.string.save_success)));

        Log.i(TAG, "<< testCase05SaveSetting end");
    }

    public void testCase06VerifySavedSetting() {
        Log.i(TAG, ">> testCase06VerifySavedSetting begin");
    
        mSolo.clickInList(6);
        mSolo.assertCurrentActivity("It's not ManagerSetActivity", ManagerSetActivity.class);

        String findStr = mSolo.getEditText(0).getText().toString();        
        assertEquals(LAST_ENTER_STRING, findStr);

        String findNum = mSolo.getEditText(1).getText().toString();        
        assertEquals(LAST_ENTER_NUMBER, findNum);

        mSolo.sleep(ACTIVITY_WAIT_TIME);
    
        Log.i(TAG, "<< testCase06VerifySavedSetting end");
    }

    public void testCase07SaveErrrHandling() {
        Log.i(TAG, ">> testCase07SaveErrrHandling begin");

        mSolo.clickInList(6);
        mSolo.assertCurrentActivity("It's not ManagerSetActivity", ManagerSetActivity.class);

        mSolo.clickOnEditText(0);
        mSolo.clearEditText(0);
        mSolo.sleep(ACTIVITY_WAIT_TIME);
        
        mSolo.clickOnButton(mSolo.getString(R.string.set_save_btn));
        mSolo.sleep(ACTIVITY_WAIT_TIME);

        assertTrue(mSolo.searchText(mSolo.getString(R.string.set_number_or_name_empty)));

        mSolo.clickOnButton(mSolo.getString(R.string.str_ok));
        mSolo.sleep(ACTIVITY_WAIT_TIME);
        mSolo.assertCurrentActivity("It's not ManagerSetActivity", ManagerSetActivity.class);

        Log.i(TAG, "<< testCase07SaveErrrHandling end");
    }

    public void testCase08CancelEditedSetting() {
        Log.i(TAG, ">> testCase08CancelEditedSetting begin");

        mSolo.clickInList(6);
        mSolo.sleep(ACTIVITY_WAIT_TIME);
        mSolo.assertCurrentActivity("It's not ManagerSetActivity", ManagerSetActivity.class);

        // The steps of Using ActivityMonitor
        // Step1: call addMonitor
        mActivityMonitor = mInstrumentation.addMonitor(MainActivity.class.getName(), null, false);
        // Step2: your actions
        mSolo.clickOnButton(mSolo.getString(R.string.set_cancel_btn));
        mSolo.sleep(ACTIVITY_WAIT_TIME);
        // Step3: waitForActivityWithTimeout
        Activity newActivity = mActivityMonitor.waitForActivityWithTimeout(ACTIVITY_CREATED_TIME);        
        assertNotNull(newActivity);
        // Don't use the below assert because if will lead to assert fail!
        //mSolo.assertCurrentActivity("It's not MainActivity", MainActivity.class);

        Log.i(TAG, "<< testCase08CancelEditedSetting end");
    }

    public void testCase09Provider() {
        Log.i(TAG, ">> testCase09Provider begin");

        String[] mInfo = new String[3];
        Uri uri = Uri.parse("content://com.mediatek.cmcc.provider/phoneinfo");
        Cursor cursor = mContext.getContentResolver().query(uri, null, null, null, null);
        if (cursor != null && cursor.moveToFirst()) {
            do {
                mInfo[0] = cursor.getString(cursor.getColumnIndex("phone_model"));
                mInfo[1] = cursor.getString(cursor.getColumnIndex("service_number"));
                mInfo[2] = cursor.getString(cursor.getColumnIndex("service_web"));
            } while (cursor.moveToNext());
        }
        cursor.close();

        assertEquals(mInfo[0], mSolo.getString(R.string.phone_type_content));
        assertEquals(mInfo[1], mSolo.getString(R.string.after_service_phone_num));
        assertEquals(mInfo[2], mSolo.getString(R.string.web_address));

        mContext.getContentResolver().getType(uri);
        mContext.getContentResolver().insert(uri, null);
        mContext.getContentResolver().delete(uri, null, null);
        mContext.getContentResolver().update(uri, null, null, null);

        try {
            Uri uriError = Uri.parse("content://com.mediatek.cmcc.provider/phoneinfo/error");
            Cursor cursorError = mContext.getContentResolver().query(uriError, null, null, null, null);
        } catch (IllegalArgumentException e) {
            Log.i(TAG, "Unknown uri");
        }

        Log.i(TAG, "<< testCase09Provider end");
    }
}


