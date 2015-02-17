package com.mediatek.backuprestore;

import java.util.ArrayList;

import android.app.ListActivity;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.EditText;
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
public class PersonalDataBackupTest extends ActivityInstrumentationTestCase2<PersonalDataBackupActivity> {
    private static final String TAG = "PersonalDataBackupTest";
    private Solo mSolo = null;
    PersonalDataBackupActivity activity = null;

    /**
     * Creates a new <code>BackupRestoreTest</code> instance.
     * 
     */
    public PersonalDataBackupTest() {
        super(PersonalDataBackupActivity.class);
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
     * Describe <code>tearDown</code> method here.
     * 
     * @exception Exception
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

    /**
     * Describe <code>test1</code> method here. test for backup app_module
     */
    public void test1() {
        Log.d(TAG, "test1 : test for backup all personalData");
        boolean result = false;
        loading();
        // startCurrentActivity(new
        // Intent(activity,PersonalDataBackupActivity.class));
        sleep(500);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        mSolo.goBack();
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.bt_no).toString());
        int count = 0;
        while (count++ < 100) {
            result = mSolo.searchText(getActivity().getText(R.string.backup_result).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.result_fail).toString());
        assertFalse(result);
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
    }

    /**
     * Describe <code>test2</code> method here. test for backuping cancel
     */
    public void test2() {
        Log.d(TAG, "test2 : test for backuping cancel");
        boolean result = false;
        loading();
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
        mSolo.goBack();
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.bt_yes).toString());
        sleep(200);
        result = mSolo.searchButton(getActivity().getText(R.string.cancel_backup_confirm).toString());
        sleep(500);
    }

    /**
     * Describe <code>test3</code> method here. test for backup and rename folder
     */
    public void test3() {
        Log.d(TAG, "test3 : test for backup and rename folder");
        boolean result = false;
        loading();
        // startCurrentActivity(new
        // Intent(activity,PersonalDataBackupActivity.class));
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.message_module).toString());
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        activity.runOnUiThread(new Runnable(){
            public void run() {
                // TODO Auto-generated method stub
                EditText et = (EditText)mSolo.getView(R.id.edit_folder_name);
                sleep(200);
                et.setText("");
            }
        });
        sleep(5000);
        mSolo.clickOnText(getActivity().getText(android.R.string.cancel).toString());
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        activity.runOnUiThread(new Runnable(){
            public void run() {
                // TODO Auto-generated method stub
                EditText et = (EditText)mSolo.getView(R.id.edit_folder_name);
                sleep(200);
                et.setText("test");
            }
        });
        sleep(5000);
        mSolo.clickOnText(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        int count = 0;
        while (count++ < 1000) {
            result = mSolo.searchText(getActivity().getText(R.string.backup_result).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.result_fail).toString());
        assertFalse(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        Log.d(TAG, "test3 : test for backup and rename folder finish");
        sleep(500);
    }

    /**
     * Describe <code>test4</code> method here. test for backup and rename folder
     */
    public void test4() {
        Log.d(TAG, "test4 : test for backup and rename override folder");
        boolean result = false;
        loading();
        // startCurrentActivity(new
        // Intent(activity,PersonalDataBackupActivity.class));
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.unselectall).toString());
        sleep(500);
        result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        assertTrue(result);
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.picture_module).toString());
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        activity.runOnUiThread(new Runnable(){
            public void run() {
                // TODO Auto-generated method stub
                EditText et = (EditText)mSolo.getView(R.id.edit_folder_name);
                sleep(200);
                et.setText("test");
            }
        });
        sleep(5000);
        mSolo.clickOnText(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        result = mSolo.searchText(getActivity().getText(R.string.backup_confirm_overwrite_notice).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
        int count = 0;
        while (count++ < 100) {
            result = mSolo.searchText(getActivity().getText(R.string.backup_result).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.result_fail).toString());
        assertFalse(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        Log.d(TAG, "test4 : test for backup and rename folder override finish");
        sleep(500);
    }

    /**
     * Describe <code>test5</code> method here. test for backup app_module
     */
    public void test5() {
        Log.d(TAG, "test5 : test for backup all personalData and remove sdCard");
        boolean result = false;
        loading();
        // startCurrentActivity(new
        // Intent(activity,PersonalDataBackupActivity.class));
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
        int count = 0;
        while (!mSolo.searchText(getActivity().getText(R.string.sdcard_removed).toString())) {
            sleep(1000);
        }

        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.sdcard_removed).toString());
        assertTrue(result);
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
    }

    /**
     * Describe <code>test6</code> method here. test for backup app_module
     */
    public void test6() {
        Log.d(TAG, "test6 : test for backup all personalData and no sdCard");
        boolean result = false;
        loading();
        // startCurrentActivity(new
        // Intent(activity,PersonalDataBackupActivity.class));
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);
        while ( !mSolo.searchText(getActivity().getText(R.string.nosdcard_notice).toString())){
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.nosdcard_notice).toString());
        assertTrue(result);
        sleep(200);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
    }

    public void loading(){
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest loading : sleep = ");
            sleep(1000);
        }
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
