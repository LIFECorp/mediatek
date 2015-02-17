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
import com.mediatek.xlog.Xlog;

/**
 * Describe class BackupRestoreTest here.
 *
 *
 * Created: Sun Jul 15 15:01:46 2012
 *
 * @author <a href="mailto:mtk80359@mbjswglx259">Zhanying Liu (MTK80359)</a>
 * @version 1.0
 */
public class CMCCPerformanceTest extends ActivityInstrumentationTestCase2<MainActivity> {
    private static final String TAG = "CMCCPerformanceTest";
    private Solo mSolo = null;
    MainActivity activity = null;

    /**
     * Creates a new <code>BackupRestoreTest</code> instance.
     *
     */
    public CMCCPerformanceTest() {
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
        sleep(1000);
    }
    
    public void test01ContactRestore() {
        init();
        boolean result = false;
        sleep(1000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        Log.d(TAG, "CMCCPerformanceTest  getChildCount == " + mView.getChildCount());
        sleep(1000);
        mSolo.clickOnText("Contact");
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "CMCCPerformanceTest test01ContactRestore : sleep = ");
            sleep(1000);
        }
        Log.d(TAG, "CMCCPerformanceTest test01ContactRestore : wake");
        result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.selectall).toString());
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.import_data).toString());
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        Xlog.i(TAG,"[CMCC Performance test][BackupAndRestore][Contact_Restore] Restore start ["+ System.currentTimeMillis() +"]");
        sleep(1000);
        while (true) {
            result = mSolo.searchText(getActivity().getText(R.string.restore_result).toString());
            if(result) {
                break;
            }
            Log.d(TAG, "CMCCPerformanceTest test01ContactRestore : result sleep = ");
            sleep(1000);
        }        
        result = mSolo.searchText(getActivity().getText(R.string.result_success).toString());
        assertTrue(result);
        sleep(2000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        Log.d(TAG, "CMCCPerformanceTest test01ContactRestore : finish ! ");
        mSolo.goBack();
        sleep(1000);
    }

    public void test02ContactBackup() {
        Log.d(TAG, "CMCCPerformanceTest test02ContactBackup : begin ! ");
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(2000);
        boolean result = false;
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.selectall).toString());
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.contact_module).toString());
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.btn_ok).toString());
        Xlog.i(TAG,"[CMCC Performance test][BackupAndRestore][Contact_Backup] Backup start ["+ System.currentTimeMillis() +"]");
        sleep(500);
        while (true) {
            result = mSolo.searchText(getActivity().getText(R.string.backup_result).toString());
            if(result) {
                break;
            }
            Log.d(TAG, "CMCCPerformanceTest test02ContactBackup : result sleep = ");
            sleep(1000);
        }
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        Log.d(TAG, "CMCCPerformanceTest test02ContactBackup : finish ! ");
        sleep(200);
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
