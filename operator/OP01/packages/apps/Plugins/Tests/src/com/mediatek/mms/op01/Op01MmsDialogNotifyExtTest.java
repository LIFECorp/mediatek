package com.mediatek.op01.tests;

import android.content.Context;
import android.content.Intent;
//import android.provider.MediaStore;
import android.test.InstrumentationTestCase;
import android.view.KeyEvent;
//import com.jayway.android.robotium.solo.Solo;
import android.view.Menu;
import android.view.MenuItem;
import java.util.ArrayList;
import android.view.SubMenu;
import android.content.ComponentName;
import android.graphics.drawable.Drawable;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.view.ActionProvider;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.provider.Telephony.WapPush;
import android.telephony.SmsMessage;
import android.text.TextUtils;
import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.provider.Telephony.Sms.Inbox;
//import com.jayway.android.robotium.solo.Solo;
import android.view.KeyEvent;
import android.content.ContentResolver;
import android.app.Instrumentation;
import android.database.sqlite.SqliteWrapper;
import android.provider.Telephony.Sms.Inbox;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.app.ActivityManager.RunningTaskInfo;
import android.app.ActivityManager;
import java.util.List;

import com.mediatek.xlog.Xlog;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
//import com.mediatek.mms.op01.Op01MmsDialogNotifyExt;
//import com.android.mms.ui.ComposeMessageActivity;

public class Op01MmsDialogNotifyExtTest extends InstrumentationTestCase {
/*
    private static final String TAG = "Dialog test";
    public static final String URI_SMS = "content://sms";
    
    private Context mContext = null;
    private Instrumentation mInst = null;
    private Op01MmsDialogNotifyExt mPlugin = null;
    private Uri mUri = null;
    //private Solo mSolo;
    
    @Override
    protected void setUp() throws Exception {
        Xlog.d(TAG, "setUp");
        super.setUp();
        
        mContext = this.getInstrumentation().getContext();
        mInst = this.getInstrumentation();
        mPlugin = 
            (Op01MmsDialogNotifyExt)PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsDialogNotify");

        if (mPlugin == null) {
            Xlog.d(TAG, "get plugin failed");
        }

        genTestSms();
    }
    
    @Override    
    protected void tearDown() throws Exception {
        Xlog.d(TAG, "tearDown");
        super.tearDown();
        clearTestSms();
    }

    private void genTestSms() {
        Xlog.d(TAG, "genTestSms");
        
        ContentValues values = new ContentValues();
        String timeStamp = "0";
        String readStamp = "0";
        String seenStamp = "0";
        String boxStamp = "0";
        String simcardStamp = "0";

        values.put(Sms.PROTOCOL, 0);
        values.put(Sms.ADDRESS, "10086");
        values.put(Sms.REPLY_PATH_PRESENT, 0);
        values.put(Sms.SERVICE_CENTER, "13800138000");
        values.put(Sms.BODY, "Auto Test");
        values.put(Sms.READ, "0");
        values.put(Sms.SEEN, "0");
        values.put(Sms.SIM_ID, "0");
        values.put(Sms.DATE, "0");
        values.put(Sms.TYPE, "1");
        values.put("import_sms", true);

        //ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(Sms.Inbox.CONTENT_URI);
        //builder.withValues(values);
        //private ArrayList<ContentProviderOperation> mOperationList = new ArrayList<ContentProviderOperation>();
        //mOperationList.add(builder.build());
        ContentResolver resolver = mContext.getContentResolver();
        mUri = SqliteWrapper.insert(mContext, resolver, Inbox.CONTENT_URI, values);    

        Xlog.d(TAG, "mUri=" + mUri);
    }

    public void testNewSmsInLauncher() {
        Xlog.d(TAG, "testNewSmsInLauncher");
        
        // Go to launcher
        //mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_HOME);
        gotoLauncher();
        
        try {
        Thread.sleep(1000);
        } catch (InterruptedException ie) {
            Xlog.d(TAG, "sleep1");
        }
        if (mUri == null) {
            Xlog.d(TAG, "no uri");
            assertEquals(0, 1);
            return;
        }
        mPlugin.notifyNewSmsDialog(mUri);
        try {
        Thread.sleep(1000);
        } catch (InterruptedException ie) {
            Xlog.d(TAG, "sleep1");
        }

        boolean showDialog = isDialogShown(mContext);
        Xlog.d(TAG, "show dialog?" + showDialog);
        assertEquals(true, showDialog);
    }

    public void testNewSmsNotInLauncher() {
        Xlog.d(TAG, "testNewSmsNotInLauncher");
        
        // Go to search
        //mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_HOME);
        gotoLauncher();
        try {
        Thread.sleep(1000);
        } catch (InterruptedException ie) {
            Xlog.d(TAG, "sleep1");
        }
        //mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_SEARCH);
        openAnotherActivity();
        try {
        Thread.sleep(1000);
        } catch (InterruptedException ie) {
            Xlog.d(TAG, "sleep1");
        }
        if (mUri == null) {
            Xlog.d(TAG, "no uri");
            assertEquals(0, 1);
            return;
        }
        mPlugin.notifyNewSmsDialog(mUri);
        try {
        Thread.sleep(1000);
        } catch (InterruptedException ie) {
            Xlog.d(TAG, "sleep1");
        }

        boolean showDialog = isDialogShown(mContext);
        Xlog.d(TAG, "show dialog?" + showDialog);
        assertEquals(false, showDialog);
    }

    private boolean isDialogShown(Context context){
        String packageName;
        String className;
        boolean ret = false;

        Xlog.d(TAG, "isDialogShown");
        
        ActivityManager activityManager = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);  
        List<RunningTaskInfo> rti = activityManager.getRunningTasks(2);  
        
        packageName = rti.get(0).topActivity.getPackageName();
        className = rti.get(0).topActivity.getClassName();
        Xlog.d(TAG, "package0="+packageName+"class0="+className);

        if (className.equals("com.android.mms.ui.DialogModeActivity")) {
            ret = true;
        }
        return ret;
    }  

    private void clearTestSms() {
        if (mUri == null) {
            return;
        }

        int count = mContext.getContentResolver().delete(mUri, null, null);
        Xlog.d(TAG, "delete count=" + count);
        gotoLauncher();
    }

    private void gotoLauncher() {
        Intent intent = new Intent();
        intent.setAction("action.intent.action.MAIN");
        intent.setClassName("com.android.launcher", "com.android.launcher2.Launcher");
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    private void openAnotherActivity() {
        //startActivity(ComposeMessageActivity.createIntent(mContext, 0));
        Intent i = new Intent().setClassName("com.android.email", "com.android.email.activity.setup.AccountSetupBasics");
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(i);
*/
}

