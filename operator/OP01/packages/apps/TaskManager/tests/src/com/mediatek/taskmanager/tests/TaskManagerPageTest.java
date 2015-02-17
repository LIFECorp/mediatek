package com.mediatek.taskmanager.tests;



import android.app.ActionBar;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.ServiceManager;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.taskmanager.R;
import com.mediatek.taskmanager.RunningProcessFragment;
import com.mediatek.taskmanager.TaskManagerPageActivity;
import com.mediatek.xlog.Xlog;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Locale;

public class TaskManagerPageTest extends
        ActivityInstrumentationTestCase2<TaskManagerPageActivity> {

    public static final String TAG = "TaskManagerPageActivityTest";

    private Instrumentation mInst;
    private Solo mSolo;
    private Context mContext;
    private Activity mActivity;
    private int mCurrentTab;
    private ActionBar mActionBar;
    private ImageView mKillIcon;
    private ImageView mKillAll;
    private ListView mListView;
    private LayoutInflater mInflater;

    public TaskManagerPageTest() {
        super("com.mediatek.taskmanager", TaskManagerPageActivity.class);
    }

        @Override
        protected void setUp() throws Exception {
            super.setUp();
            mInst = getInstrumentation();
            mContext = mInst.getTargetContext();
            mActivity = getActivity();
            mSolo = new Solo(mInst, mActivity);
            mActionBar = mActivity.getActionBar();
            mCurrentTab = mActionBar.getSelectedTab().getPosition();
            mInflater = (LayoutInflater)mActivity.getSystemService(
                        Context.LAYOUT_INFLATER_SERVICE);

        }




    public void test01TaskManagerCase01LaunchOneApp() {
        Xlog.i(TAG, "test01_TaskManagerCase01LaunchOneApp()");
        switchTab(0);
        mSolo.sleep(3000);    

         try {
            ApplicationInfo aiDeskClock = mActivity.getPackageManager().getApplicationInfo("com.android.deskclock", 0);
            String processNameDeskclock = (aiDeskClock.loadLabel(mActivity.getPackageManager())).toString();
            mSolo.clickOnText(processNameDeskclock);
        } catch (PackageManager.NameNotFoundException e) {
            Xlog.e(TAG, "Package is not found");
            }
        mSolo.sleep(1000);
        sendKeys(KeyEvent.KEYCODE_BACK);
        
         try {
            ApplicationInfo aiContact = mActivity.getPackageManager().getApplicationInfo("com.android.contact", 0);
            String processNameContact = (aiContact.loadLabel(mActivity.getPackageManager())).toString();
            mSolo.clickOnText(processNameContact);
        } catch (PackageManager.NameNotFoundException e) {
            Xlog.e(TAG, "Package is not found");
            }
        mSolo.sleep(1000);
        sendKeys(KeyEvent.KEYCODE_BACK);
        
        mInst.waitForIdleSync();
    }

    public void test02TaskManagerCase02KillOneProcess() {
    
        Xlog.i(TAG, "test02_TaskManagerCase02KillOneProcess()");
        
        switchTab(0);
        
        mSolo.sleep(1000);
        mListView = mSolo.getCurrentViews(ListView.class).get(1);//mListView = mSolo.getCurrentListViews().get(1);
        assertNotNull(mListView);
                        
        View runningView = (View)mListView.getAdapter().getView(3, null, (ViewGroup)mListView.getParent());
        assertNotNull(runningView);
                
        RunningProcessFragment.ViewHolder vh = (RunningProcessFragment.ViewHolder)runningView.getTag();

        mSolo.clickOnView(vh.killIcon);
        Xlog.i(TAG,"Killed process name: " + vh.name.getText());
        
        mSolo.sleep(1000);
        mKillIcon = (ImageView)mSolo.getView(R.id.kill_icon);
        mSolo.clickOnView(mKillIcon);
        
        mSolo.sleep(1000);    
        
        switchTab(2);
        
        switchTab(0);
        mKillIcon = (ImageView)mSolo.getView(R.id.kill_icon);
        mSolo.clickOnView(mKillIcon);
        mSolo.sleep(1000);    

        mInst.waitForIdleSync();
    }

    
    public void test03TaskManagerCase03KillOneByOne() {
        Xlog.i(TAG, "test03_TaskManagerCase03KillOneByOne()");
        
        switchTab(0);
        
        mListView = (ListView)mSolo.getCurrentViews(ListView.class).get(1);//mListView = (ListView)mSolo.getCurrentListViews().get(1);
        int count = mListView.getAdapter().getCount();
        
        Xlog.i(TAG, "count=" + count);
        ArrayList<ImageView> killIconList = new ArrayList<ImageView>();
        for (int i = 0; i < count; i++) {
            
            sendKeysEvents(KeyEvent.KEYCODE_DPAD_DOWN, 1);
                  killIconList.add((ImageView)mSolo.getView(R.id.kill_icon));
            //killIcon = (ImageView)mSolo.getView(R.id.kill_icon);
               mSolo.clickOnView((ImageView)mSolo.getView(R.id.kill_icon));
               mSolo.sleep(1000);
        }
          mInst.waitForIdleSync();
    }/**/
    public void test04TaskManagerCase04KillAllProcess() {
        Xlog.i(TAG, "test04_TaskManagerCase03KillAllProcess()");
        
        switchTab(0);
        mSolo.sleep(1000);
        mKillAll = (ImageView)mSolo.getView(R.id.kill_all);
        //mSolo.clickOnView(mKillAll);
        
        mInst.waitForIdleSync();
    }
    public void test05TaskManagerCase05InstalledApp() {
        Xlog.i(TAG, "test05_TaskManagerCase04InstalledApp()");
        
        switchTab(1);
        
        try {
            ApplicationInfo mm = mActivity.getPackageManager().getApplicationInfo("com.aspire.mm", 0);
            String name = (mm.loadLabel(mActivity.getPackageManager())).toString();
            mSolo.clickOnText(name);
            mSolo.sleep(3000);
            sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);
            
            mSolo.sleep(1000);
            sendKeys(KeyEvent.KEYCODE_ENTER);
            mSolo.sleep(1000);
            
            sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
            
            mSolo.sleep(1000);
            sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);
            mSolo.sleep(1000);
            sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);
            mSolo.sleep(1000);
            sendKeys(KeyEvent.KEYCODE_ENTER);
            sendKeys(KeyEvent.KEYCODE_BACK);
            
        } catch (PackageManager.NameNotFoundException e) {
            Xlog.e(TAG, "Package is not found");
            }

        mInst.waitForIdleSync();    
    }

    public void test06TaskManagerCase06Ram() {
        Xlog.i(TAG, "test06_TaskManagerCase05Ram()");
        
        switchTab(2);
        mSolo.sleep(1000);
        
        TextView percent = (TextView)mSolo.getView(R.id.precent);
        TextView usedRAM = (TextView)mSolo.getView(R.id.used_ram_id);
        TextView freeRAM = (TextView)mSolo.getView(R.id.free_ram_id);
        TextView totalRAM = (TextView)mSolo.getView(R.id.total_ram_id);
        
        Xlog.i(TAG, "Percent:" + percent.getText().toString());
        Xlog.i(TAG, "UsedRAM:" + percent.getText().toString());
        Xlog.i(TAG, "FreeRAM:" + percent.getText().toString());
        Xlog.i(TAG, "TotalRAM:" + percent.getText().toString());
        
        mInst.waitForIdleSync();    
    }
    public void test07TaskManagerCase07SwitchTab() {
        Xlog.i(TAG, "test07_TaskManagerCase06SwitchTab()");
        
        switchTab(2);
        mSolo.sleep(1000);
        
        switchTab(1);
        mSolo.sleep(1000);
        //mSolo.scrollDownList(3);
        mSolo.sleep(1000);
        
        switchTab(0);
        //mSolo.scrollDownList(3);
        mSolo.sleep(1000);
        
        mSolo.scrollToSide(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sleep(1000);
        
        mSolo.scrollToSide(KeyEvent.KEYCODE_DPAD_LEFT);
        mSolo.sleep(1000);
        
        mInst.waitForIdleSync();

        
    }
    
    public void test08TaskManagerCase08() {

        
        Xlog.i(TAG, "test08_TaskManagerCase08()");
        int i = 3;
        mListView = (ListView)mSolo.getCurrentViews(ListView.class).get(1);//mListView = (ListView)mSolo.getCurrentListViews().get(1);
        int count = mListView.getAdapter().getCount();
        
        Xlog.i(TAG, "count=" + count);
        switchTab(0);
        do {
/*
            try {
                ApplicationInfo aiLauncher = mActivity.getPackageManager().getApplicationInfo("com.android.launcher", 0);
                String processNameLauncher = (aiLauncher.loadLabel(mActivity.getPackageManager())).toString();
                mSolo.clickOnText(processNameLauncher);
            } catch (PackageManager.NameNotFoundException e) {
                Xlog.e(TAG, "Package is not found");
                }
            mSolo.sleep(1000);
*/
            try {
                ApplicationInfo aiAcore = mActivity.getPackageManager().getApplicationInfo("com.process.acore", 0);
                String processNameAcore = (aiAcore.loadLabel(mActivity.getPackageManager())).toString();
                mSolo.clickOnText(processNameAcore);
            } catch (PackageManager.NameNotFoundException e) {
                Xlog.e(TAG, "Package is not found");
                }
            mSolo.sleep(1000);
/*
            try {
                ApplicationInfo aiDeskClock = mActivity.getPackageManager().getApplicationInfo("com.android.deskclock", 0);
                String processNameDeskclock = (aiDeskClock.loadLabel(mActivity.getPackageManager())).toString();
                mSolo.clickOnText(processNameDeskclock);
            } catch (PackageManager.NameNotFoundException e) {
                Xlog.e(TAG, "Package is not found");
                }
            mSolo.sleep(1000);
*/
            count = count - 6;
            sendKeysEvents(KeyEvent.KEYCODE_DPAD_DOWN, 6);
        } while (count > 0);
        mInst.waitForIdleSync();
        
    }

    
    
    public void test09TaskManagerCase09ReloadTaskManager() {
        
        Xlog.i(TAG, "test09_TaskManagerCase08ReloadTaskManager()");
        switchTab(0);
        mSolo.sleep(1000);
        
        sendKeysEvents(KeyEvent.KEYCODE_DPAD_DOWN, 30);

        try {        
            ApplicationInfo ai = mActivity.getPackageManager().getApplicationInfo("com.mediatek.taskmanager", 0);
            String processName = (ai.loadLabel(mActivity.getPackageManager())).toString();
            
            mSolo.clickOnText(processName);
        } catch (PackageManager.NameNotFoundException e) {
            Xlog.e(TAG, "Package is not found");
            }
        mSolo.sleep(1000);
        
        switchTab(1);
        mSolo.sleep(1000);
            
        mInst.waitForIdleSync();
    }

    
    public void test10ReorderInstalledAppList() {
        
        Xlog.i(TAG, "test10ReorderInstalledAppList()");
        switchTab(1);
        mSolo.sleep(1000);
        
        String alphaOrder = mActivity.getResources().getString(R.string.sort_order_alpha);
        String sizeOrder = mActivity.getResources().getString(R.string.sort_order_size);
        if (mSolo.searchText(alphaOrder)) {
            mSolo.clickOnText(alphaOrder);
        } else {
            mSolo.clickOnText(sizeOrder);
        }
        //sendKeys(KeyEvent.KEYCODE_MENU);
        //sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        //sendKeys(KeyEvent.KEYCODE_ENTER);
        //mInst.invokeMenuActionSync(mActivity, Menu.FIRST, 0);
        
        mSolo.sleep(2000);
        
        switchTab(0);
        mSolo.sleep(1000);
        switchTab(1);
        
        if (mSolo.searchText(alphaOrder)) {
            mSolo.clickOnText(alphaOrder);
        } else {
            mSolo.clickOnText(sizeOrder);
        }

        
        mInst.waitForIdleSync();
    }

    public void test11ChangeLanguageToResetRunningList() {
        Xlog.i(TAG, "test10ReorderInstalledAppList()");
        switchTab(0);
        mSolo.sleep(1000);
        Locale.setDefault(Locale.US);
        Intent iEN = new Intent(Intent.ACTION_CONFIGURATION_CHANGED);
        mActivity.sendBroadcast(iEN);
        mSolo.sleep(1000);
        
        Locale.setDefault(Locale.SIMPLIFIED_CHINESE);
        Intent iSC = new Intent(Intent.ACTION_CONFIGURATION_CHANGED);
        mActivity.sendBroadcast(iSC);
        
        mSolo.sleep(1000);
        
        mInst.waitForIdleSync();
    }

    public void test12NotifyPackageInfoChanged() {
        
        Xlog.i(TAG, "test12NotifyPackageInfoChanged()");
        
        /*Intent intentAdd = new Intent(Intent.ACTION_PACKAGE_ADDED,
                            Uri.fromParts("package", "com.aspire.mm", null));
        
        mActivity.sendBroadcast(intentAdd);
        mSolo.sleep(1000);
            mActivity.sendBroadcast(intentRemove);
            mSolo.sleep(1000);
            Intent intentChange = new Intent(Intent.ACTION_PACKAGE_CHANGED,
                                Uri.fromParts("package", "com.aspire.mm", null));
            Bundle extras = new Bundle(2);
            extras.putBoolean(Intent.EXTRA_DONT_KILL_APP, true);
            try {
                extras.putInt(Intent.EXTRA_UID, 
                      mActivity.getPackageManager().getApplicationInfo("com.aspire.mm", 0).uid);
            } catch (PackageManager.NameNotFoundException e) {
                Xlog.e(TAG, "Package is not found");
            }
            
            intentChange.putExtras(extras);
            
            mActivity.sendBroadcast(intentChange);*/
        int appId =-1;
        
        try {
            appId = mActivity.getPackageManager()
                    .getApplicationInfo("com.aspire.mm", 0).uid;
        } catch (PackageManager.NameNotFoundException e) {
            Xlog.e(TAG, "Package is not found");
        }
        
        Bundle extras = new Bundle(1);
        extras.putInt(Intent.EXTRA_UID, appId);
        extras.putBoolean(Intent.EXTRA_DATA_REMOVED, false);
        Intent intentRemove = new Intent(Intent.ACTION_PACKAGE_REMOVED,
                            Uri.fromParts("package", "com.aspire.mm", null));
        intentRemove.putExtras(extras);
        //mActivity.sendBroadcast(intentRemove);
        
        mSolo.sleep(3000);
        Intent intentUnavail = new Intent(Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE,
                            Uri.fromParts("package", "com.aspire.mm", null));
        int uidArr[] = new int[1];
        ArrayList<String> pkgList = new ArrayList<String>();
        uidArr[0] = appId;
        pkgList.add("com.aspire.mm");
        
        Bundle extrasUnavail = new Bundle();
        extrasUnavail.putStringArray(Intent.EXTRA_CHANGED_PACKAGE_LIST, pkgList
                .toArray(new String[1]));
        extrasUnavail.putIntArray(Intent.EXTRA_CHANGED_UID_LIST, uidArr);
        
        intentUnavail.putExtras(extrasUnavail);
        mActivity.sendBroadcast(intentUnavail);
        mSolo.sleep(3000);


        mInst.waitForIdleSync();
        
    }
    private void switchTab(final int position) {
         mInst.runOnMainSync(new Runnable() {
            public void run() {
                mActionBar.selectTab(mActionBar.getTabAt(position));
                
            }
         });
         mInst.waitForIdleSync();
         mCurrentTab = position;
    }

    private static Object getPrivateField(Object obj, String name) {
        Object result = null;
        try {
            Class thisClass = obj.getClass();
            Field thisField = thisClass.getDeclaredField(name);
            thisField.setAccessible(true);
            result = thisField.get(obj);
            thisField.setAccessible(false);
        } catch (IllegalAccessException e) {
            Xlog.e(TAG, "Fail to access private variable");
        } catch (NoSuchFieldException e) {
            Xlog.e(TAG, "Fail to get private field");
        }

        return result;
    }
    private void sendKeysEvents(int event, int num) {
        while (num > 0) {
                sendKeys(event);
                num--;
            }
        return;
    }
    
}
