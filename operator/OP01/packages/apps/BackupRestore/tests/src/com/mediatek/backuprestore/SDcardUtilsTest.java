package com.mediatek.backuprestore;

import java.util.ArrayList;

import android.app.ActionBar;
import android.app.ListActivity;
import android.app.ActionBar.Tab;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TabHost;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.backuprestore.PersonalDataBackupActivity;
import com.mediatek.backuprestore.PersonalDataRestoreActivity;

/**
 * Describe class BackupRestoreTest here.
 *
 *
 * Created: Sun Jul 15 15:01:46 2012
 *
 * @author <a href="mailto:mtk80359@mbjswglx259">Zhanying Liu (MTK80359)</a>
 * @version 1.0
 */
public class SDcardUtilsTest extends ActivityInstrumentationTestCase2<MainActivity> {
    private static final String TAG = "SDcardUtils";
    private Solo mSolo = null;
    MainActivity activity = null;

    /**
     * Creates a new <code>BackupRestoreTest</code> instance.
     *
     */
    public SDcardUtilsTest() {
        super(MainActivity.class);
    }

    /**
     * Describe <code>setUp</code> method here.
     *
     * @exception Exception
     *                if an error occurs
     */
    public final void setUp() throws Exception {
        super.setUp();
        mSolo = new Solo(getInstrumentation(), getActivity());
        activity = getActivity();
        Log.d(TAG, "setUp");
    }

    /**
     *Describe <code>tearDown</code> method here.
     *
     *@exception Exception
     *                if an error occurs
     */
    public final void tearDown() throws Exception {
        //
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }

        if (activity != null) {
            Log.d(TAG, "tearDown : activity = " + activity);
            activity.finish();
            activity = null;
        }
        super.tearDown();
        Log.d(TAG, "tearDown");
        sleep(5000);
    }

    public void test01RepalceData() {
        init();
        Log.d(TAG, "SDcardUtils testRepalceData : test for restore and testRepalceData");
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        while (mView.getAdapter().getCount() == 0 ){
            sleep(1000);
            Log.d(TAG, "SDcardUtils 1: testRepalceData : sleep = ");
        }
        this.sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        this.sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "SDcardUtils 2: testRepalceData : sleep = ");
            sleep(1000);
        }
        Log.d(TAG, "BackupRestoreTest test3 : wake");
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.replace_data).toString());
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
        int count = 0;
        while (count++ < 10000) {
            result = mSolo.searchText(getActivity().getText(R.string.restore_result).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
        mSolo.goBack();
        sleep(200);
    }

    public void test02UnmountOldData() {
        init();
        Log.d(TAG, "SDcardUtils testUnmountOldData : test for restore old data and umount sdcard!");
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        while (mView.getAdapter().getCount() == 0 ){
            sleep(1000);
            Log.d(TAG, "SDcardUtils 1 test1 : sleep = ");
        }
        sleep(1000);
        mSolo.clickOnText("old");
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "SDcardUtils testUnmountOldData : sleep = ");
            sleep(1000);
        }
        Log.d(TAG, "BackupRestoreTest testUnmountOldData : wake up");
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.contact_module).toString());
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.import_data).toString());
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
        while (!mSolo.searchText(getActivity().getText(R.string.sdcard_removed).toString())) {
            sleep(1000);
        }
        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.sdcard_removed).toString());
        assertTrue(result);
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
    }

    public void test03() {
        boolean result = false;
        Log.d(TAG, "BackupRestoreTest test6 alps00428735 : no sdcard click canceal button no response when remove sdcard ");
        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(0);
        Log.d(TAG, "BackupRestoreTest init : mTab " + mTab.getText());
        mSolo.clickOnText(mTab.getText().toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest testClickTwoButtonSameTime : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);

        result = mSolo.searchText(getActivity().getText(R.string.nosdcard_notice).toString());
        sleep(1000);
        assertTrue(result);
        mSolo.clickOnButton(getActivity().getText(android.R.string.ok).toString());
        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        assertTrue(result);
        mSolo.clickOnButton(getActivity().getText(R.string.unselectall).toString());
        if (!result) {
            result = mSolo.searchText(getActivity().getText(R.string.selectall).toString());
            sleep(1000);
            assertTrue(result);
            mSolo.clickOnButton(getActivity().getText(R.string.selectall).toString());
        }
        assertTrue(result);
        sleep(2000);
    }

    public void test04SDcardFull() {
        Log.d(TAG, "SDcardUtils testSDcardFull : test for restore data and testSDcardFull !");
        boolean result = false;
        init();
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        while (mView.getAdapter().getCount() == 0 ){
            sleep(1000);
            Log.d(TAG, "SDcardUtils testSDcardFull : sleep = ");
        }

        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(0);
        mSolo.clickOnText(mTab.getText().toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "testSDcardFull : sleep = ");
            sleep(1000);
        }
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
        int count = 0;
        while (count++ < 1000) {
            result = mSolo.searchText(getActivity().getText(R.string.sdcard_is_full).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.sdcard_is_full).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
        mSolo.goBack();
        sleep(200);
    }

   public void test05() {
        boolean result = false;
        Log.d(TAG, "BackupRestoreTest test5 : ALPS00439985backup app when sdcard is full");
        init();
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        while (mView.getAdapter().getCount() == 0 ){
            sleep(1000);
            Log.d(TAG, "SDcardUtils 1 test5 : sleep = ");
        }

        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(0);
        Log.d(TAG, "BackupRestoreTest init : mTab " + mTab.getText());
        mSolo.clickOnText(mTab.getText().toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.backup_app).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test5 : sleep = ");
            sleep(1000);
        }
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.selectall).toString());
        sleep(1000);
        Log.d(TAG, "1 : result 3 = " + result);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        int count = 0;
        while (count++ < 1000) {
            result = mSolo.searchText(getActivity().getText(R.string.sdcard_is_full).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.sdcard_is_full).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
        mSolo.goBack();
    }

    public void test06() {
        Log.d(TAG, "SDcardUtilsTest test6 : test for remind note when sdcard full");
        boolean result = false;
        init();
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        while (mView.getAdapter().getCount() == 0 ){
            sleep(1000);
            Log.d(TAG, "SDcardUtils 1 test6 : sleep = ");
        }
        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(0);
        Log.d(TAG, "BackupRestoreTest init : mTab " + mTab.getText());
        mSolo.clickOnText(mTab.getText().toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test6 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        Log.d(TAG, "SDcardUtilsTest test6 : getChildCount == " + mView.getChildCount());
        int index = 0;
        int listCount = la.getListAdapter().getCount();
        Log.d(TAG, "BackupRestoreTest test6 : listCount = " + listCount);
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        while( index< listCount) {
            mSolo.clickOnView(la.getListView().getChildAt(index));
            sleep(1000);
            mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
            sleep(1000);
            result = mSolo.searchText(getActivity().getText(android.R.string.ok).toString());
            assertTrue(result);
            sleep(1000);
            mSolo.clickOnButton(getActivity().getText(android.R.string.ok).toString());
            int count = 0;
            while (count++ < 1000) {
                result = mSolo.searchText(getActivity().getText(R.string.sdcard_is_full).toString());
                if (result) {
                    break;
                }
                sleep(1000);
            }
            result = mSolo.searchText(getActivity().getText(R.string.sdcard_is_full).toString());
            assertTrue(result);
            sleep(1000);
            mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
            sleep(1000);
            mSolo.clickOnView(la.getListView().getChildAt(index));
            index ++;
        }
        sleep(1000);
        mSolo.goBack();
    }



    public void loading(){
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, " loading : sleep = ");
            sleep(1000);
        }
    }

    public void init() {
        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(1);
        Log.d(TAG, " init : mTab " + mTab.getText());
        mSolo.clickOnText(mTab.getText().toString());
        sleep(2000);
    }

    /**
     * Describe <code>sleep</code> method here.
     *
     * @param time
     *            a <code>int</code> value
     */
    public void sleep(int time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
        }
    }

}
