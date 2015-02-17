package com.mediatek.backuprestore;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import android.app.ActionBar;
import android.app.ListActivity;
import android.app.ActionBar.Tab;
import android.content.ContentValues;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageDeleteObserver;
import android.content.pm.PackageManager;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageInstallObserver;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.Cursor;

import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.provider.Telephony.Mms;
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
import com.mediatek.backuprestore.modules.AppBackupComposer;
import com.mediatek.backuprestore.modules.AppRestoreComposer;
import com.mediatek.backuprestore.modules.SmsXmlInfo;
import com.mediatek.backuprestore.utils.BackupXmlInfo;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

/**
 * Describe class BackupRestoreTest here.
 * 
 * 
 * Created: Sun Jul 15 15:01:46 2012
 * 
 * @author <a href="mailto:mtk80359@mbjswglx259">Zhanying Liu (MTK80359)</a>
 * @version 1.0
 */
public class BackupRestoreTest extends ActivityInstrumentationTestCase2<MainActivity> {
    private static final String TAG = "BackupRestoreTest";
    private Solo mSolo = null;
    MainActivity activity = null;
    private Object mLock = new Object();
    private static final Uri[] MMSURIARRAY = { Mms.Sent.CONTENT_URI,
        // Mms.Outbox.CONTENT_URI,
        // Mms.Draft.CONTENT_URI,
        Mms.Inbox.CONTENT_URI };
    private Cursor[] mMmsCursorArray = { null, null };
    private static final String[] MMS_EXCLUDE_TYPE = { "134", "130" };
    /**
     * Creates a new <code>BackupRestoreTest</code> instance.
     * 
     */
    public BackupRestoreTest() {
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

    public void test1() {
        Log.d(TAG, "BackupRestoreTest test1 : test for backup all apps");
        boolean result = false;
        //startCurrentActivity(new Intent(activity, AppBackupActivity.class));
        mSolo.clickOnText(getActivity().getText(R.string.backup_app).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test1 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        assertTrue(result);
        mSolo.clickOnButton(getActivity().getText(R.string.selectall).toString());
        sleep(1000);
        Log.d(TAG, "1 : result 3 = " + result);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(200);
        int count = 0;
        while (count++ < 10000) {
            result = mSolo.searchText(getActivity().getText(R.string.backup_result).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        sleep(2000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        Log.d(TAG, "test1 for backup all app_module finish");
        sleep(2000);
        mSolo.goBack();
    }

    public void test2() {
        Log.d(TAG, "BackupRestoreTest test2 : test for restore app");
        String sdcardPath = SDCardUtils.getStoragePath();
        sdcardPath = sdcardPath+ File.separator + "App" + File.separator +"com.mediatek.backuprestore.tests";
        Log.d(TAG, "sdcardPath = " + sdcardPath);
        String appPath = sdcardPath +".apk";
        String appData = sdcardPath +".tar";
        File app = new File(appPath);
        File data = new File(appData);
        if (app.exists()) {
            app.delete();
            Log.d(TAG, "app = " + appPath + "delete!");
        }
        if(data.exists()) {
            data.delete();
        }

        init();
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        result = mSolo.searchText(getActivity().getText(R.string.backup_app_data_preference_title).toString());
        Log.d(TAG, "BackupRestoreTest test2 : searchText result = " + result);
        assertTrue(result);
        sleep(2000);
        Log.d(TAG, "BackupRestoreTest test2 : searchText view = " + mView.getChildAt(mView.getChildCount() - 1));
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
        Log.d(TAG, "BackupRestoreTest test2 finish !");
        mSolo.goBack();
        sleep(1000);
    }

    /**
     * test for person_data and no_data
     */

    public void test3() {
        Log.d(TAG, "BackupRestoreTest test for person_data and no_data !");
        boolean result = false;
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(5000);
//        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
//        while (la.getListAdapter().getCount() != 0) {
//            Log.d(TAG, "BackupRestoreTest test3 : sleep = ");
//            sleep(1000);
//        }

        TextView data = (TextView)mSolo.getView(R.id.empty);
        Log.d(TAG,"data.getText() == "+data.getText());
        result = mSolo.searchText(data.getText().toString());
        assertTrue(result);
        copyPicture();
        sleep(1000);
        copyMusic();
        sleep(500);
        mSolo.goBack();
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(500);
        mSolo.goBack();
        Log.d(TAG, "BackupRestoreTest test3 finish !");
    }

    /**
     * BackupRestoreTest test4 : test for restore old data"
     */
    public void test4() {
        init();
        Log.d(TAG, "BackupRestoreTest test4 : test for restore old data");
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        Log.d(TAG, "BackupRestoreTest test4 : getChildCount == " + mView.getChildCount());
        result = mSolo.searchText(getActivity().getText(R.string.backup_app_data_preference_title).toString());
        assertTrue(result);
        sleep(2000);
        mSolo.clickOnText("old");
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test4 : sleep = ");
            sleep(1000);
        }
        Log.d(TAG, "BackupRestoreTest test4 : wake");
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.import_data).toString());
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
        Log.d(TAG, "BackupRestoreTest test4 : finish ! ");
        mSolo.goBack();
        sleep(1000);
    }

    /**
     * test for person_data ui changed
     */
    public void test5() {
        Log.d(TAG, "BackupRestoreTest test5 : test for person_data ui changed");
        boolean result = false;

        //startCurrentActivity(new Intent(activity, AppBackupActivity.class));
        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test5 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        if(!result) {
            result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
            sleep(1000);
            mSolo.clickOnText(getActivity().getText(R.string.selectall).toString());
        }
        assertTrue(result);
        mSolo.clickOnText(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        mSolo.clickOnView(la.getListView().getChildAt(0));
        sleep(1000);
        View configView = (View)mSolo.getView(R.id.setting);
        mSolo.clickOnView(configView);
        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.contact_module).toString());
        assertTrue(result);
        sleep(1000);
        /*mSolo.clickOnText(getActivity().getText(R.string.contact_phone).toString());
        sleep(1000);*/
        mSolo.clickOnText(getActivity().getText(R.string.btn_ok).toString());
        sleep(1000);
        mSolo.goBack();
    }

    /**
     * BackupRestoreTest test6 : test for sms"
     */
    public void test6() {
        init();
        Log.d(TAG, "BackupRestoreTest test6 : test for sms ");
        boolean result;
        sleep(2000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        Log.d(TAG, "BackupRestoreTest test6 : getChildCount == " + mView.getChildCount());
        result = mSolo.searchText(getActivity().getText(R.string.backup_app_data_preference_title).toString());
        assertTrue(result);
        sleep(2000);
        mSolo.clickOnText("sms");
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test6 : sleep = ");
            sleep(1000);
        }
        Log.d(TAG, "BackupRestoreTest test6 : wake");
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.import_data).toString());
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

        SmsXmlInfo sx = new SmsXmlInfo();
        assertNotNull(sx.getCategory());
        assertNotNull(sx.getLocalDate());
        assertNotNull(sx.getST());
        assertNotNull(sx.getSize());
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        Log.d(TAG, "BackupRestoreTest test6 : finish ! ");
        mSolo.goBack();
        sleep(1000);
    }

    /**
     * Describe <code>test7</code> method here. test for restore all module
     */
    public void test7() {
        init();
        sleep(1000);
        Log.d(TAG, "BackupRestoreTest test7 : test for restore personaldata");
        boolean result;
        sleep(1000);
        this.sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        this.sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test7 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.import_data).toString());
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        int count = 0;
        while (count++ < 1000) {
            result = mSolo.searchText(getActivity().getText(R.string.restore_result).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.result_success).toString());
        assertTrue(result);
        BackupXmlInfo bx = new BackupXmlInfo();
        assertNull(bx.getDevicetype());
        assertNull(bx.getSystem());
        assertNotNull(bx.getTotalNum());
        assertNotNull(getMMScount());
        sleep(2000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(2000);
        Log.d(TAG, "BackupRestoreTest test7 : finish ");
        mSolo.goBack();
        sleep(500);
    }

    /**
     * BackupRestoreTest test8 : test for mount sdcard"
     */
    public void test8() {
        init();
        Log.d(TAG, "BackupRestoreTest test8 : test for mount sdcard ");
        boolean result;
        testSend(Intent.ACTION_MEDIA_MOUNTED);
        sleep(4000);
        ListView mView = (ListView) mSolo.getView(android.R.id.list);
        Log.d(TAG, "BackupRestoreTest test8 : getChildCount == " + mView.getChildCount());
        result = mSolo.searchText(getActivity().getText(R.string.backup_app_data_preference_title).toString());
        assertTrue(result);
        mSolo.clickLongOnView(mView.getChildAt(mView.getChildCount() - 1));
        Log.d(TAG, "BackupRestoreTest test8 : clicklongonView app ");
        sleep(2000);
        View cancle_select = (View)mSolo.getView(R.id.cancel_select);
        View deleteView = (View)mSolo.getView(R.id.delete);
        View selectAll = (View)mSolo.getView(R.id.select_all);
        sleep(2000);
        mSolo.clickOnView(selectAll);
        sleep(1000);
        mSolo.clickOnView(cancle_select);
        sleep(1000);
        mSolo.clickOnView(mView.getChildAt(mView.getChildCount() - 1));
        sleep(1000);
        mSolo.clickOnView(deleteView);
        sleep(1000);
        while (mSolo.searchText(getActivity().getText(R.string.delete_please_wait).toString()) ){
            sleep(1000);
        }

        testSend(Intent.ACTION_MEDIA_UNMOUNTED);
        sleep(2000);
        Log.d(TAG, "BackupRestoreTest test8 : delete finish ");
        sleep(2000);
    }
    /**
     * test for ui change
     */

    public void test9() {
        Log.d(TAG, "BackupRestoreTest test9 : test for ui change");
        boolean result = false;

        //startCurrentActivity(new Intent(activity, AppBackupActivity.class));
        mSolo.clickOnText(getActivity().getText(R.string.backup_app).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test9 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        if(!result) {
            mSolo.clickOnText(getActivity().getText(R.string.selectall).toString());
        }
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.unselectall).toString());
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnView(la.getListView().getChildAt(0));
        sleep(1000);
        mSolo.goBack();
        sleep(1000);
        mSolo.clickOnText(getActivity().getText(R.string.backup_app).toString());
        sleep(2000);
        ListActivity la1 = (ListActivity) mSolo.getCurrentActivity();
        while (la1.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test9 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        if(la1.getListAdapter().getCount() == 1) {
            result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        } else {
            result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
        }
        assertTrue(result);
        Log.d(TAG, "test9 for test for ui change finish");
        sleep(2000);
        mSolo.goBack();
        sleep(500);
    }

    /**
     * Describe <code>test10</code> method here. test for restore all module and replace data
     */
    public void testRestorePersonData() {
        init();
        sleep(1000);
        Log.d(TAG, "BackupRestoreTest test10 : test for restore personaldata and replace data");
        boolean result;
        sleep(1000);
        this.sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        this.sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sleep(1000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest test10 : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.restore).toString());
        sleep(500);
        mSolo.clickOnText(getActivity().getText(R.string.replace_data).toString());
        sleep(500);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(500);
        int count = 0;
        while (count++ < 1000) {
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
        sleep(2000);
        Log.d(TAG, "BackupRestoreTest test10 : finish !");
        mSolo.goBack();
        sleep(500);
    }

    public void testClickTwoButtonSameTime() {
        Log.d(TAG, "BackupRestoreTest testClickTwoButtonSameTime");
        boolean result = false;

        mSolo.clickOnText(getActivity().getText(R.string.backup_personal_data).toString());
        sleep(2000);
        ListActivity la = (ListActivity) mSolo.getCurrentActivity();
        while (la.getListAdapter().getCount() == 0) {
            Log.d(TAG, "BackupRestoreTest testClickTwoButtonSameTime : sleep = ");
            sleep(1000);
        }
        sleep(1000);
        result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        if(!result) {
            result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
            sleep(1000);
            mSolo.clickOnButton(getActivity().getText(R.string.selectall).toString());
        }
        sleep(1000);
        assertTrue(result);

        mSolo.clickOnButton(getActivity().getText(R.string.unselectall).toString());
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);
        int count = 0;
        while (count++ < 100) {
            result = mSolo.searchButton(getActivity().getText(R.string.selectall).toString());
            if (result) {
                break;
            }
            sleep(1000);
        }
        Log.d(TAG, "BackupRestoreTest testClickTwoButtonSameTime : result = " + result);
        if (!result) {
            mSolo.clickOnButton(getActivity().getText(android.R.string.cancel).toString());
            sleep(1000);
            result = mSolo.searchButton(getActivity().getText(R.string.unselectall).toString());
        }
        sleep(1000);
        assertTrue(result);
        sleep(2000);
        mSolo.goBack();
    }

    public void testUninstallApp (){
        List<ApplicationInfo>  list = AppBackupComposer.getUserAppInfoList(activity);
        for (ApplicationInfo mlist : list) {
            uninstallPackage(mlist.packageName);
        }
        mSolo.clickOnText(getActivity().getText(R.string.backup_app).toString());
        sleep(2000);
        mSolo.goBack();
    }

    public boolean uninstallPackage(String packageName) {
        PackageManager packageManager = activity.getPackageManager();
        PackageDeleteObserver deleteObserver = new PackageDeleteObserver();
        packageManager.deletePackage(packageName, deleteObserver, PackageManager.DELETE_KEEP_DATA);
        synchronized (mLock) {
            while (!deleteObserver.mFinished) {
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (deleteObserver.mResult == PackageManager.DELETE_SUCCEEDED) {
                return true;
            } else {
                return false;
            }
        }
    }

    class PackageDeleteObserver extends IPackageDeleteObserver.Stub {
        private boolean mFinished = false;
        private int mResult;

        @Override
        public void packageDeleted(String name, int status) {
            synchronized (mLock) {
                mFinished = true;
                mResult = status;
                mLock.notifyAll();
            }
        }
    }
    public void copyPicture() {
        String dest = Environment.getExternalStorageDirectory().getPath() + File.separator + "Pictures"
                + File.separator + "ttt.png";
        String src = SDCardUtils.getStoragePath() + File.separator + "ttt.png";
        Log.d(TAG, "path == " + dest);
        Log.d(TAG, "path sdcard == " + src);
        try {
            copyFile(src, dest);
            ContentValues v = new ContentValues();
            File file = new File(dest);
            v.put(Images.Media.SIZE, file.length());
            v.put(Images.Media.DATA, file.getAbsolutePath());
            Uri tmpUri = getActivity().getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, v);
            Log.d(TAG, "tmpUri:" + tmpUri + "," + file.getAbsolutePath());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void copyMusic() {
        String dest = Environment.getExternalStorageDirectory().getPath() + File.separator + "Music" + File.separator
                + "ni.mp3";
        String src = SDCardUtils.getStoragePath() + File.separator + "ni.mp3";
        Log.d(TAG, "path == " + dest);
        Log.d(TAG, "path sdcard == " + src);
        try {
            copyFile(src, dest);
            Uri data = Uri.parse("file://" + dest);
            getActivity().sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void copyFile(String srcFile, String destFile) throws IOException {
        try {
            File f1 = new File(srcFile);
            if (f1.exists() && f1.isFile()) {
                InputStream inStream = new FileInputStream(srcFile);
                FileOutputStream outStream = new FileOutputStream(destFile);
                byte[] buf = new byte[1024];
                int byteRead = 0;
                while ((byteRead = inStream.read(buf)) != -1) {
                    outStream.write(buf, 0, byteRead);
                }
                outStream.flush();
                outStream.close();
                inStream.close();
            }
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public int getMMScount() {
        int result = 0;
        for (int i = 0; i < MMSURIARRAY.length; ++i) {
            if (MMSURIARRAY[i] == Mms.Inbox.CONTENT_URI) {
                mMmsCursorArray[i] = activity.getContentResolver().query(MMSURIARRAY[i], null,
                        "m_type <> ? AND m_type <> ?", MMS_EXCLUDE_TYPE, null);
            } else {
                mMmsCursorArray[i] = activity.getContentResolver().query(MMSURIARRAY[i], null, null, null, null);
            }
            if (mMmsCursorArray[i] != null) {
                mMmsCursorArray[i].moveToFirst();
            }
        }
        for (Cursor cur : mMmsCursorArray) {
            if (cur != null && cur.getCount() > 0) {
                result += cur.getCount();
                cur.close();
                }
            }
        Log.d(TAG, "BackupRestoreTest getMMScount = "+ result);
        return result;
    }

    public void init() {
        ActionBar mActionBar = activity.getActionBar();
        Tab mTab = mActionBar.getTabAt(1);
        Log.d(TAG, "BackupRestoreTest init : mTab " + mTab.getText());
        mSolo.clickOnText(mTab.getText().toString());
        sleep(2000);
    }

    public void testSend(String action){
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
