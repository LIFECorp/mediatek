package com.mediatek.backuprestore;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import android.app.ActionBar;
import android.app.ListActivity;
import android.app.ActionBar.Tab;
import android.content.ContentValues;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;
import com.jayway.android.robotium.solo.Solo;
import com.mediatek.backuprestore.MainActivity;
import com.mediatek.backuprestore.AppBackupActivity;
import com.mediatek.backuprestore.PersonalDataRestoreActivity;
import com.mediatek.backuprestore.AppRestoreActivity;
import com.mediatek.backuprestore.SDCardReceiver;
import com.mediatek.backuprestore.modules.SmsXmlInfo;
import com.mediatek.backuprestore.utils.BackupXmlInfo;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

/**
 * Describe class CmccStanderdDetaTest here. this calss is used to test cmcc
 * standard backup data including 1.0 and 2.0 data.
 *
 * Created: Tue Apr 15 15:01:46 2013
 *
 *@author <a href="mailto:mtk81368@mbjswglx259">N Wang (MTK81368)</a>
 *@version 1.0
 */
public class CmccStandardDataTest extends ActivityInstrumentationTestCase2<MainActivity> {
    private static final String TAG = "CmccStandardDataTest";
    private Solo mSolo = null;
    MainActivity activity = null;

    /**
     *Creates a new <code>CmccStandardDataTes</code> instance.
     *
     */
    public CmccStandardDataTest() {
        super(MainActivity.class);
    }

    /**
     *Describe <code>setUp</code> method here.
     *
     * @exception Exception
     *if an error occurs
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
        if (activity != null) {
            activity.finish();
            activity = null;
        }
        super.tearDown();
        Log.d(TAG, "tearDown");
        sleep(5000);
    }

    public void testCmcc950StandardData() {
        Log.d(TAG, " testCmccFiveStandardData : testCmcc950StandardData");
        boolean result = false;
        result = renameFile("950", "orignal", "backup");
        assertTrue(result);
        result = renameFile(".backup", ".backup1");
        Log.d(TAG, "testCmcc950StandardData:renameFile result = " + result);
        assertTrue(result);
        Log.d(TAG, "Cmcc950StandardData : call test2 to check app restore ");
        RestoreApp();
        sleep(1000);
        Log.d(TAG, "Cmcc950StandardData : check personal data ");
        RestorePersonalData();

        result = renameFile("orignal", "950", "backup");
        result = renameFile(".backup1", ".backup");
        Log.d(TAG, "testCmcc950StandardData:renameFile result = " + result);
        assertTrue(result);
        mSolo.goBack();
        sleep(1000);
    }

    public void testCmcc310StandardData() {
        Log.d(TAG, " testCmccFiveStandardData : testCmcc310StandardData");
        boolean result = false;
        result = renameFile("310", "orignal", "backup");
        assertTrue(result);
        result = renameFile(".backup", ".backup1");
        Log.d(TAG, "testCmcc310StandardData:renameFile result = " + result);
        assertTrue(result);
        Log.d(TAG, "Cmcc310StandardData : call RestoreApp to check app restore ");
        RestoreApp();
        sleep(1000);
        Log.d(TAG, "Cmcc310StandardData : check personal data ");
        RestorePersonalData();

        result = renameFile("orignal", "310", "backup");
        Log.d(TAG, "testCmcc310StandardData:renameFile result = " + result);
        assertTrue(result);
        result = renameFile(".backup1", ".backup");
        assertTrue(result);
        mSolo.goBack();
        sleep(1000);
    }

    public void testCmcc7568StandardData() {
        Log.d(TAG, " testCmccFiveStandardData : testCmcc7568StandardData");
        boolean result = false;
        result = renameFile("7568", "orignal", "backup");
        Log.d(TAG, "testCmcc7568StandardData:renameFile result = " + result);
        assertTrue(result);
        result = renameFile(".backup", ".backup1");
        assertTrue(result);
        Log.d(TAG, "testCmcc7568StandardData : check personal data ");
        RestorePersonalData();

        result = renameFile("orignal", "7568", "backup");
        Log.d(TAG, "testCmcc7568StandardData:renameFile result = " + result);
        assertTrue(result);
        result = renameFile(".backup1", ".backup");
        assertTrue(result);
        mSolo.goBack();
        sleep(1000);
    }

    public void testCmcc795StandardData() {
        Log.d(TAG, " testCmccFiveStandardData : testCmcc795StandardData");
        boolean result = false;
        result = renameFile("795", "orignal", "backup");
        Log.d(TAG, "renameFile result = " + result);
        assertTrue(result);
        result = renameFile(".backup", ".backup1");
        assertTrue(result);
        Log.d(TAG, "testCmcc795StandardData : call RestoreApp to check app restore ");
        RestoreApp();
        sleep(1000);
        Log.d(TAG, "testCmcc795StandardData : check personal data ");
        RestorePersonalData();

        result = renameFile("orignal", "795", "backup");
        Log.d(TAG, "renameFile result = " + result);
        assertTrue(result);
        result = renameFile(".backup1", ".backup");
        assertTrue(result);
        mSolo.goBack();
        sleep(1000);
    }

    public void testOldCmccStandardData() {
        Log.d(TAG, " testCmccv1.0StandardData ");
        boolean result = false;
        result = renameFile("backupOld", "orignalOld", ".backup");
        Log.d(TAG, "testCmccvOldStandardData:renameFile result = " + result);
        assertTrue(result);
        result = renameFile("backup", "backup1");
        assertTrue(result);
        Log.d(TAG, "testCmccvOldStandardData : check personal data ");
        RestorePersonalData();
        result = renameFile("orignalOld", "backupOld", ".backup");
        Log.d(TAG, "testCmccvOldStandardData:renameFile result = " + result);
        assertTrue(result);
        result = renameFile("backup1", "backup");
        assertTrue(result);
        mSolo.goBack();
        sleep(1000);
    }

    public void RestoreApp() {
        init();
        Log.d(TAG, "CmccStandardDataTest : test for restore app");
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);

        result = mSolo.searchText(getActivity().getText(R.string.backup_app_data_preference_title).toString());
        Log.d(TAG, "CmccStandardDataTest : searchText result = " + result);
        assertTrue(result);
        sleep(2000);
        Log.d(TAG, "CmccStandardDataTest : searchText view = " + mView.getChildAt(mView.getChildCount() - 1));
        mSolo.clickOnView(mView.getChildAt(mView.getChildCount() - 1));
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(1000);
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
        result = mSolo.searchText(getActivity().getText(R.string.result_success).toString());
        assertTrue(result);
        sleep(2000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        Log.d(TAG, "CmccStandardDataTest finish !");
        mSolo.goBack();
        sleep(1000);
    }

    /**
     * Describe <code>testRestorePersonData</code> method here. test for restore
     * all the PersonalData under the backup foder.
     */
    public void RestorePersonalData() {
        init();
        Log.d(TAG, " testCmccFiveStandardData : : RestorePersonalData");
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        int index = 0;
        Log.d(TAG, " testCmccFiveStandardData  : getChildCount == " + mView.getChildCount());
        result = mSolo.searchText(getActivity().getText(R.string.backup_app_data_preference_title).toString());
        if (result) {
            index = mView.getChildCount() - 3;
        } else {
            index = mView.getChildCount() - 1;
        }
        sleep(2000);
        while (index > 0) {
            mSolo.clickOnView(mView.getChildAt(index));
            index--;
            sleep(1000);
            ListActivity la = (ListActivity) mSolo.getCurrentActivity();
            while (la.getListAdapter().getCount() == 0) {
                Log.d(TAG, " testCmccFiveStandardData  : sleep = ");
                sleep(1000);
            }
            Log.d(TAG, " testCmccFiveStandardData  : wake");
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
            result = mSolo.searchText(getActivity().getText(R.string.result_success).toString());
            assertTrue(result);
            sleep(2000);
            mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
            sleep(500);
            Log.d(TAG, " RestorePersonalData : finish ! ");
            mSolo.goBack();
            sleep(1000);
        }
    }

    public String getSDcardPath() {
        File sdcard = null;
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            sdcard = Environment.getExternalStorageDirectory();
        }
        return sdcard.toString();
    }

    public boolean renameFile(String src, String des) {
        boolean result = false;
        String sdcardPath = SDCardUtils.getStoragePath();
        sdcardPath = sdcardPath.subSequence(0, sdcardPath.lastIndexOf(File.separator)).toString();
        Log.d(TAG, "sdcardPath = " + sdcardPath);
        String srcPath = sdcardPath + "/" + src;
        String desPath = sdcardPath + "/" + des;
        File srcFile = new File(srcPath);
        File desFile = new File(desPath);

        if (srcFile.exists()) {
            srcFile.renameTo(desFile);
            result = true;
        }
        return result;
    }

    public boolean renameFile(String standardName, String orignal, String backup) {
        String sdcardPath = SDCardUtils.getStoragePath();
        sdcardPath = sdcardPath.subSequence(0, sdcardPath.lastIndexOf(File.separator)).toString();
        Log.d(TAG, "sdcardPath = " + sdcardPath);

        String backupFilepath = sdcardPath + "/" + backup;
        Log.d(TAG, "backupFilepath = " + backupFilepath);
        File backupFile = new File(backupFilepath);
        backupFile.renameTo(new File(sdcardPath + "/" + orignal));
        Log.d(TAG, "backupFile = " + backupFile);
        Log.d(TAG, "backupFile rename suffess");

        File StandaredData = new File(sdcardPath + "/" + standardName);
        Log.d(TAG, "StandaredData = " + sdcardPath + "/" + standardName);
        if (StandaredData.exists()) {
            Log.d(TAG, "331backupFilepath = " + backupFilepath);
            StandaredData.renameTo(new File(backupFilepath));
            Log.d(TAG, "Cmcc950StandardData : rename success ");
            return true;
        } else {
            Log.d(TAG, "testCmccFiveStandardData1 : cannt find the " + StandaredData);
            File original = new File(sdcardPath + "/" + standardName);
            original.renameTo(new File(backupFilepath));
            return false;
        }
    }

    public void init() {
        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(1);
        Log.d(TAG, " init : mTab " + mTab.getText());
        mSolo.clickOnText(mTab.getText().toString());
        sleep(2000);
    }

    public void testSend(String action) {
        SDCardReceiver sr = SDCardReceiver.getInstance();
        Intent intent = new Intent(action);
        sr.onReceive(getInstrumentation().getContext(), intent);
    }

    /**
     * Describe startActivity.
     *
     */
    public void startCurrentActivity(Intent i) {
        activity.startActivity(i);
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
