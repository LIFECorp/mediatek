package com.mediatek.configurecheck2;

import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.app.AlarmManager;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContentValues;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.net.Uri;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.Environment;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.BrowserContract;
import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;


import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.RIL;
import com.android.internal.view.RotationPolicy;
import com.android.internal.widget.LockPatternUtils;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.storage.StorageManagerEx;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.String;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class CheckBJTime extends CheckItemBase {
    private static final String TAG = " MTBF CheckBJTime";
    private static final String BjTimeZoneID = "Asia/Shanghai";
    private static final String TIMEZONE_PROPERTY = "persist.sys.timezone";
    private int mAutoTime;
    private int mNetworkTime;
    private Context cc;
    private boolean isTimeZone = false;
    private boolean mReseting = false;
    
    CheckBJTime (Context c, String key) {
        super(c, key);
        cc = c;
        setTitle(R.string.title_time);  
        setNote(R.string.note_time);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }

 
     
     public check_result getCheckResult() {
         /*
          * implement check function here
          */             
         if (mReseting) {
            return mResult;
         }

         /* set time zone first */
         String current = SystemProperties.get(TIMEZONE_PROPERTY);
         if (!current.equals(BjTimeZoneID)) {                
             isTimeZone = true;
         } else {
             isTimeZone = false;
         }
         try {
             mNetworkTime = Settings.Global.getInt(cc.getContentResolver(), 
                 Settings.Global.AUTO_TIME);  
             CTSCLog.d(TAG, "get auto time value = " + mNetworkTime);
         } catch (SettingNotFoundException e) {
              CTSCLog.d(TAG, "get Settings item exception");
         }
         if (isTimeZone && mNetworkTime != 1) {
             setValue(R.string.value_time_time_zone);
             mResult = check_result.WRONG;
         } else if (isTimeZone) {
             setValue(R.string.value_time_zone);
             mResult = check_result.WRONG;
         } else if (mNetworkTime != 1) {
             setValue(R.string.value_time_time);
             mResult = check_result.WRONG;
         } else {
             setValue(R.string.value_time);
             mResult = check_result.RIGHT;
         }
     
         return mResult;
     } 
     
     public boolean onReset() {
         /*
          * implement your reset function here
          */
         CTSCLog.i(TAG, "onReset");
         if (!isConfigurable()) {
             //On no, this instance is check only, not allow auto config.
             return false;
         }  
 
         /* set time zone */
         if (isTimeZone) {
             mReseting = true;
             AlarmManager alarm = (AlarmManager)cc.getSystemService(Context.ALARM_SERVICE);
             alarm.setTimeZone(BjTimeZoneID);
             new Handler().postDelayed(new Runnable() {
             public void run() {
                CTSCLog.d(TAG, "screen beijing zone done send set refresh");
                sendBroadcast();
                mReseting = false;
                mResult = check_result.RIGHT;
                setValue(R.string.value_time);
               }
            }, 500);
         }
 
         if (mNetworkTime != 1) {
             Settings.Global.putInt(cc.getContentResolver(), Settings.Global.AUTO_TIME, 1); 
         }
         mResult = check_result.RIGHT;
         setValue(R.string.value_time);
         /*  */
         return true;
     }
 }
 
 class CheckScreenOn extends CheckItemBase {
     private static final String TAG = " MTBF CheckScreenOn";
     private boolean isTimeMax = true;
     private boolean isScreenLock = true;
     private Context cc;
     
     CheckScreenOn (Context c, String key) {
        super(c, key);

        cc = c;
  
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        if (key.equals(CheckItemKeySet.CI_SCREEN_ON_SLEEP)) {
            setTitle(R.string.title_sleep);
            setNote(R.string.note_sleep);
        } else if (key.equals(CheckItemKeySet.CI_SCREEN_ON_UNLOCK)) {            
            setTitle(R.string.title_lockscreen);
            setNote(R.string.note_lockscreen);
        }
     }

     public boolean onCheck() {
         CTSCLog.d(TAG, " oncheck");
         if (getKey().equals(CheckItemKeySet.CI_SCREEN_ON_SLEEP)) {             
             /*get time unlock time check if 30 minites or not*/
             int currentValue = Settings.System.getInt(cc.getContentResolver(), Settings.System.SCREEN_OFF_TIMEOUT, 30000); 
             CTSCLog.d(TAG, " current sleep time is " + currentValue);

             if (currentValue == 1800000) {
                mResult = check_result.RIGHT;
                isTimeMax = true;
             } else {
                mResult = check_result.WRONG;
                isTimeMax = false;
             }
             String mit = String.valueOf(currentValue/60000);
             setValue(getContext().getString(R.string.value_sleep)+ mit + getContext().getString(R.string.value_sleep_end));
         } else if (getKey().equals(CheckItemKeySet.CI_SCREEN_ON_UNLOCK)) {
             int isOn = Settings.Global.getInt(cc.getContentResolver(), Settings.Global.STAY_ON_WHILE_PLUGGED_IN, 0);
             CTSCLog.d(TAG, "the lock screen is " + isOn);

             isScreenLock = (isOn <= 0);
             if (isOn <= 0) {
                 setValue(R.string.value_lockscreen);
                 mResult = check_result.WRONG;
             } else {                 
                 setValue(R.string.value_unlockscreen);
                 mResult = check_result.RIGHT;
             }
         }
         return true;
     }

     public check_result getCheckResult() {
         /*
          * implement check function here
          */             
         return mResult;
     } 
     
     public boolean onReset() {
         /*
          * implement your reset function here
          */
         CTSCLog.i(TAG, "onReset");
         if (!isConfigurable()) {
             //On no, this instance is check only, not allow auto config.
             return false;
         }  

         if (!isTimeMax) {
            CTSCLog.d(TAG, " onReset, set time max");
            setValue(R.string.value_sleep_max);
            Settings.System.putInt(cc.getContentResolver(), Settings.System.SCREEN_OFF_TIMEOUT, 1800000);
         }

         if (isScreenLock) {
             CTSCLog.d(TAG, " onReset, unmark screen lock");
             setValue(R.string.value_unlockscreen);
             Settings.Global.putInt(cc.getContentResolver(), Settings.Global.STAY_ON_WHILE_PLUGGED_IN, 3);
         }
         mResult = check_result.RIGHT;         
         return true;
     }
 }


class CheckScreenRotate extends CheckItemBase {
    private static final String TAG = " MTBF CheckScreenRotate";
    private Context cc;
    private boolean mReseting = false;
     
    CheckScreenRotate (Context c, String key) {
        super(c, key);
        cc = c;
        setTitle(R.string.title_screen_rotate);
        setNote(R.string.note_screen_rotate);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }

    public boolean onCheck() {
        CTSCLog.d(TAG, "onCheck");
        if (mReseting) {
            return true;
        }
        if (RotationPolicy.isRotationLocked(cc)) {
            mResult = check_result.RIGHT;  
            setValue(R.string.value_off);
        } else {
            mResult = check_result.WRONG;
            setValue(R.string.value_on);
        }
        CTSCLog.d(TAG, "constructor the Rotation lock result " + mResult);
        return true;
    }
     
     public check_result getCheckResult() {
         /*
          * implement check function here
          */             
         return mResult;
     } 
     
     public boolean onReset() {
         /*
          * implement your reset function here
          */
         CTSCLog.i(TAG, "onReset");
         if (!isConfigurable()) {
             //On no, this instance is check only, not allow auto config.
             return false;
         } 
         /* lock rotate screen */
         RotationPolicy.setRotationLock(cc, true);

          new Handler().postDelayed(new Runnable() {
             public void run() {
                CTSCLog.d(TAG, "screen rotation send set refresh");
                sendBroadcast();
                mReseting = false;
                setValue(R.string.value_off);
                mResult = check_result.RIGHT;
           }
        }, 300);
         return true;
     }
 }


class CheckUrl extends CheckItemBase {
    private static final String TAG = " MTBF CheckUrl";
    private static final String BAIDU = "http://wap.baidu.com";
    private static final String TEXT = 
        "http://218.206.177.209:8080/waptest/fileDownLoad?file=Text&groupname=11&fenzu=WAP2.0";
    private static final String MP3 = 
        "http://218.206.177.209:8080/waptest/fileDownLoad?file=mp3";
    private static final String JPG = 
        "http://218.206.177.209:8080/waptest/fileDownLoad?file=Picture&groupname=11&fenzu=WAP2.0";
    private static final String GP3 = 
        "http://218.206.177.209:8080/waptest/fileDownLoad?file=Video&groupname=11&fenzu=WAP2.0";
    private static final String STREAMING1 = 
        "http://42.96.171.2/list/rtsp.jsp";

    private static final Uri BOOKMARKS_URI = Uri
            .parse("content://com.android.browser/bookmarks");
    private static Context cc;
    private boolean mAsyncDone = true;
     
    CheckUrl (Context c, String key) {
        super(c, key);
        
        CTSCLog.d(TAG, " checkUrl");
        cc = c;
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        setTitle(R.string.title_bookmark);
        setNote(R.string.note_bookmark);
    }
 
    public boolean onCheck() {
        CTSCLog.d(TAG, " onCheck()");
        if (!mAsyncDone) {
            return true;
        }
        mAsyncDone = false;
        new Thread(new Runnable() {
            public void run() {
                CTSCLog.d(TAG, " onCheck, run syncbookmark");
                int id1 = syncBookmark("Baidu", BAIDU);
                int id2 = syncBookmark("Text", TEXT);
                int id3 = syncBookmark("Mp3", MP3);
                int id4 = syncBookmark("Jpg", JPG);
                int id5 = syncBookmark("3gp", GP3);
                int id6 = syncBookmark("Strea", STREAMING1);
                mAsyncDone = true;
                CTSCLog.d(TAG, "sync id1= " + id1 + " ;id2 = " + id2 + ";id3 = " + id3 + 
                    ";id4 = " + id4 + ";id5 = " + id5 + "; id6 = " + id6);
                if (id1 == -1 || id2 == -1 || id3 == -1 ||
                    id4 == -1 || id5 == -1 || id6 == -1 ) {
                    /* not exist*/
                    mResult = check_result.WRONG;
                    setValue(R.string.value_bookmark_uncompleted);
                } else {
                    mResult = check_result.RIGHT;
                    setValue(R.string.value_bookmark_completed);
                }                
                CTSCLog.d(TAG, "sync done refresh UI");
                sendBroadcast();
            }
        }, "async-bookmark-thread").start();
        
        return true;
    }
 
    public check_result getCheckResult() {
        /*
         * implement check function here
         */ 
        if (!mAsyncDone) {
            mResult = check_result.UNKNOWN;
            setValue(R.string.ctsc_querying);
            return mResult;
        }
        CTSCLog.d(TAG, "getCheckResult mResult = " + mResult);
        return mResult;
    } 
     
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        } 
        mAsyncDone = false;
        new Thread(new Runnable() {
            public void run() {
                CTSCLog.d(TAG, " onCheck, run addbookmark");
                //delete all fisrt
                deleteSameUrl(BAIDU, 1);
                deleteSameUrl(TEXT, 1);
                deleteSameUrl(MP3, 1);
                deleteSameUrl(JPG, 1);
                deleteSameUrl(GP3, 1);
                deleteSameUrl(STREAMING1, 1);

                addBookmark("3gp", GP3);
                addBookmark("Baidu", BAIDU);
                addBookmark("Jpg", JPG);                
                addBookmark("Mp3", MP3);                
                addBookmark("Strea", STREAMING1);
                addBookmark("Text", TEXT);
                mAsyncDone = true;
                mResult = check_result.RIGHT;
                setValue(R.string.value_bookmark_completed);
                CTSCLog.d(TAG, "add done refresh UI");
                sendBroadcast();                
            }
        }, "async-addbookmark-thread").start();
        
        return true;
    }

    private int syncBookmark(String name, String url) {
        int id = getIdByNameOrUrl(name, url, 1);
        CTSCLog.d(TAG, "sync bookmark name = " + name + "; id = " + id);
        return id;
    }
    
    private void addBookmark(String name, String url) {
        ContentValues values = new ContentValues();
        CTSCLog.d(TAG, "add start");
        try {
            values.put(BrowserContract.Bookmarks.TITLE, name);
            values.put(BrowserContract.Bookmarks.URL, url);
            values.put(BrowserContract.Bookmarks.IS_FOLDER, 0);                   
            values.put(BrowserContract.Bookmarks.PARENT, 1);
            cc.getContentResolver().insert(BrowserContract.Bookmarks.CONTENT_URI, values);
        } catch (IllegalStateException e) {
            CTSCLog.d(TAG, "addBookmark" + e);
        }
    }

    
    private static void deleteSameUrl(String url, long parent) {
        CTSCLog.d(TAG, "deleteSameUrl url:" + url);
        if (url == null || url.length() == 0) {
            return;
        }
        ContentValues values = new ContentValues();
        values.put(BrowserContract.Bookmarks.IS_DELETED, 1);
        int count = cc.getContentResolver().update(
             BrowserContract.Bookmarks.CONTENT_URI, values, 
             BrowserContract.Bookmarks.URL + " =? AND " + BrowserContract.Bookmarks.PARENT
             + " =? AND " + BrowserContract.Bookmarks.IS_DELETED + " =?", new String[] {
             url, String.valueOf(parent), String.valueOf(0)
          });
        CTSCLog.d(TAG, "same url delete :" + count);
    }

    static int getIdByNameOrUrl( String name, String url, long parentId) {
        String where = BrowserContract.Bookmarks.PARENT + " = ?"
        + " AND (" + BrowserContract.Bookmarks.TITLE + " = ?" 
        + " OR " + BrowserContract.Bookmarks.URL + " = ?"
        + " OR " + BrowserContract.Bookmarks.URL + " = ?)"
        + " AND " + BrowserContract.Bookmarks.IS_FOLDER + "= 0";   
     
        String[] projection = {BrowserContract.Bookmarks._ID};
        Cursor cursor = cc.getContentResolver().query(BrowserContract.Bookmarks.CONTENT_URI, projection, where, 
                new String[] {String.valueOf(parentId), name, url,
                url.endsWith("/")? url.substring(0, url.lastIndexOf("/")): (url+"/") }, 
                BrowserContract.Bookmarks._ID + " DESC");
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    return cursor.getInt(0);
                }
            } finally {
                cursor.close();
            }
        }
        return -1;
    }   
}


class CheckDefaultStorage extends CheckItemBase {
    private static final String TAG = " MTBF CheckDefaultStorage";
    //private static final String USBSTORAGEPATH = "/mnt/usbotg";
    private boolean isResetting = false;
    
    private StorageManager mStorageManager;
    private Context cc;
    
    CheckDefaultStorage (Context c, String key) {
        super(c, key);
        cc = c;
        setTitle(R.string.title_default_storage);
        setNote(R.string.note_default_storage_phone);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }

    public boolean onCheck() {
        if (isResetting) {
            return true;
        }
        String externalStoragePath = StorageManagerEx.getExternalStoragePath();
        String defaultpath = StorageManagerEx.getDefaultPath();
        CTSCLog.d(TAG, " Current default path=" + defaultpath + "; external path =" + externalStoragePath);
        if (defaultpath.equals(externalStoragePath) ||
            defaultpath.equals(Environment.DIRECTORY_USBOTG)) {
            mResult = check_result.WRONG;
            setValue(R.string.value_default_storage_card);
        } else {
            mResult = check_result.RIGHT;
            setValue(R.string.value_default_storage_phone);
        }
        CTSCLog.d(TAG, "the default storage check result = " + mResult);
        return true;        
    }
    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */             
        return mResult;
    } 
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        } 
        isResetting = true;
        final String path0 = "/storage/sdcard0";
        final String path1 = "/storage/sdcard1";
        String defaultpath = StorageManagerEx.getDefaultPath();
        if (path0.equals(defaultpath)) {
            StorageManagerEx.setDefaultPath(path1);
            setValue("set default storage is sdcard1");
        } else {
            StorageManagerEx.setDefaultPath(path0);
            setValue("set default storage is sdcard0");
        }
        new Handler().postDelayed(new Runnable() {
             public void run() {
                isResetting = false;
                CTSCLog.d(TAG, "default storage set refresh");
                sendBroadcast();
                
                String defaultpath1 = StorageManagerEx.getDefaultPath();
                CTSCLog.d(TAG, "after reset the default storage is " + defaultpath1);
           }
        }, 1000);
        return true;       

        
        /*
        mStorageManager = StorageManager.from(cc);
        String[] mPathList = mStorageManager.getVolumePaths();
        StorageVolume[] volumes = mStorageManager.getVolumeList();

        
        List<String> mVolumePathList = new ArrayList<String>();
        List<StorageVolume> storageVolumes = new ArrayList<StorageVolume>();
        
          for (int i = 0; i < mPathList.length; i++) {
              CTSCLog.d(TAG, "Volume: " + volumes[i].getDescription(cc) + " ,state is: " + 
                      mStorageManager.getVolumeState(mPathList[i]) + " ,emulated is: " + volumes[i].isEmulated()
                      + ", path is " + volumes[i].getPath());
        
              if (!"not_present".equals(mStorageManager.getVolumeState(mPathList[i]))) {
                  mVolumePathList.add(mPathList[i]);
                  storageVolumes.add(volumes[i]);
              }
          }

        int length = storageVolumes.size();
        String externalStoragePath = StorageManagerEx.getExternalStoragePath();        
        CTSCLog.d(TAG, "default path group length = " + length + " external patch " + externalStoragePath);
        for (int i = 0; i < storageVolumes.size(); i++) {
            StorageVolume volume = storageVolumes.get(i);
            CTSCLog.d(TAG, "volume get path = " + volume.getPath() + ";volume = " + volume);
            if (!((volume.getPath()).equals(externalStoragePath)) &&
                !((volume.getPath()).equals(USBSTORAGEPATH))) {
                 String phoneStorage = volume.getPath();
                 StorageManagerEx.setDefaultPath(phoneStorage);
                 setValue("Default storage is phone storage");
                 mResult = check_result.RIGHT;
                 CTSCLog.d(TAG, "set phone storage sucess phone storage path = " + phoneStorage);
                 String temp = StorageManagerEx.getDefaultPath();
                 CTSCLog.d(TAG, "set phone storage sucess get default path = " + temp);
                 break;
            }
        }    
        
        return true;*/
    }
}

class CheckShortcut extends CheckItemBase {
    private static final String TAG = "MTBF CheckShortcut";
    private static final String DESKCLOCKAPNAME = "com.android.deskclock";
    private static final String FILESYSTEMAPNAME = "com.mediatek.filemanager";
    private static final String MUSICAPNAME = "com.android.music";
    private static final String SETTINGAPNAME = "com.android.settings";
    private static final String EMAILAPNAME = "com.android.email";
    private static final String TODOAPNAME = "com.mediatek.todos";
    private static final String RECORDERAPNAME = "com.android.soundrecorder";
    private static final String CALENDERAPNAME = "com.android.calendar";
    
    private static final String GALLERYNAME = "com.android.gallery3d";//KK
    private static final String TASKMANAGERNAME = "com.mediatek.taskmanager";
    private static final String VIDEOPLAYERNAME = "com.mediatek.videoplayer";//JB & KK
    private static final String DOWNLOADNAME = "com.android.providers.downloads.ui";
    private static final String CALCULATORNAME = "com.android.calculator2";
    
    //bookmark shortcut
    private static final String BAIDUNAME = "Baidu";
    private static final String BAIDUURL = "http://wap.baidu.com/";
    private static final String BROWSERNAME = "com.android.browser";
    public static final String EXTRA_APPLICATION_ID = "com.android.browser.application_id";
    
    private Context cc;
    
    CheckShortcut (Context c, String key) {
        super(c, key);
        cc = c;
        setTitle(R.string.title_shortcut);
        setNote(R.string.note_shortcut_not_checkable);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }

    public boolean onCheck() {
        CTSCLog.d(TAG, "onCheck");
        //this check item do not do check,
        //because launcher not allow to query its provider
        mResult = check_result.UNKNOWN;
        return true;
    }
    
    public boolean onReset() {
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        new Thread(new Runnable() {
            public void run() {
                CTSCLog.d(TAG, "uninstall shortcut, then install shortcut");
                deleteShortcut(DESKCLOCKAPNAME);
                deleteShortcut(FILESYSTEMAPNAME);
                deleteShortcut(MUSICAPNAME);
                deleteShortcut(SETTINGAPNAME);
                deleteShortcut(EMAILAPNAME, "com.android.email.activity.Welcome");
                deleteShortcut(TODOAPNAME);
                deleteShortcut(RECORDERAPNAME);
                deleteShortcut(CALENDERAPNAME);
                
                deleteShortcut(SETTINGAPNAME, "com.android.settings.wifi.WifiSettings");
                deleteShortcut(GALLERYNAME, "com.android.camera.CameraLauncher");
                deleteShortcut(TASKMANAGERNAME);
                deleteShortcut(VIDEOPLAYERNAME);
                deleteShortcut(GALLERYNAME, "com.android.gallery3d.app.GalleryActivity");
                deleteShortcut(DOWNLOADNAME);
                deleteShortcut(CALCULATORNAME);
                deleteBookmarkShortcut(BAIDUNAME, BAIDUURL);
                
                createShortcut(DESKCLOCKAPNAME);
                createShortcut(FILESYSTEMAPNAME);
                createShortcut(MUSICAPNAME);
                createShortcut(SETTINGAPNAME);
                createShortcut(EMAILAPNAME, "com.android.email.activity.Welcome");
                createShortcut(TODOAPNAME);
                createShortcut(RECORDERAPNAME);
                createShortcut(CALENDERAPNAME);
                
                createShortcut(SETTINGAPNAME, "com.android.settings.wifi.WifiSettings");
                createShortcut(GALLERYNAME, "com.android.camera.CameraLauncher");
                createShortcut(TASKMANAGERNAME);
                createShortcut(VIDEOPLAYERNAME);
                createShortcut(GALLERYNAME, "com.android.gallery3d.app.GalleryActivity");
                createShortcut(DOWNLOADNAME);
                createShortcut(CALCULATORNAME);
                createBookmarkShortcut(BAIDUNAME, BAIDUURL);
                
            }
        }, "async-CreateShortcut-thread").start();
        return true;
    }
    
    private void deleteBookmarkShortcut(String name, String url) {
        CTSCLog.d(TAG, "deleteBookmarkShortcut");
        //bookmark's icon just use the browser's application'a icon
        Intent shortcut = constructShortcutIntent(false, name, 
                getShortcutIcon(BROWSERNAME, null), getBookmarkSCIntent(url));
        cc.sendBroadcast(shortcut);
    }
    
    private void createBookmarkShortcut(String name, String url) {
        CTSCLog.d(TAG, "createBookmarkShortcut");
        //bookmark's icon just use the browser's application'a icon
        Intent shortcut = constructShortcutIntent(true, name, 
                getShortcutIcon(BROWSERNAME, null), getBookmarkSCIntent(url));
        cc.sendBroadcast(shortcut);
    }
    
    private Intent getBookmarkSCIntent(String url) {
        Intent shortcutIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
        long urlHash = url.hashCode();
        long uniqueId = (urlHash << 32) | shortcutIntent.hashCode();
        shortcutIntent.putExtra(EXTRA_APPLICATION_ID, Long.toString(uniqueId));
        return shortcutIntent;
    }

    private void deleteShortcut(String packageName) {  
        CTSCLog.d(TAG, "deleteShortcut");
        try {
            PackageManager pm = cc.getPackageManager();
            PackageInfo info = pm.getPackageInfo(packageName, PackageManager.GET_ACTIVITIES);
            deleteShortcut(packageName, info.activities[0].name);//the first activity tag in AndroidManifest
        } catch (NameNotFoundException e) {
            CTSCLog.d(TAG, "NameNotFoundException " + e);        
        }
    }
    
    private void deleteShortcut(String packageName, String activityName) {  
        CTSCLog.d(TAG, "deleteShortcut");
        Intent shortcut = constructShortcutIntent(false, getShortcutName(packageName, activityName), 
                getShortcutIcon(packageName, activityName), getShortcutIntent(packageName, activityName));
        cc.sendBroadcast(shortcut);
    }

    private void createShortcut(String packageName) {  
        CTSCLog.d(TAG, "createShortcut");
        try {
            PackageManager pm = cc.getPackageManager();
            PackageInfo info = pm.getPackageInfo(packageName, PackageManager.GET_ACTIVITIES);
            createShortcut(packageName, info.activities[0].name);//the first activity tag in AndroidManifest
        } catch (NameNotFoundException e) {
            CTSCLog.d(TAG, "NameNotFoundException " + e);        
        }
    }
    
    private void createShortcut(String packageName, String activityName) {  
        CTSCLog.d(TAG, "createShortcut");
        Intent shortcut = constructShortcutIntent(true, getShortcutName(packageName, activityName), 
                getShortcutIcon(packageName, activityName), getShortcutIntent(packageName, activityName));
        cc.sendBroadcast(shortcut);
    }
    
    private String getShortcutName(String packageName, String activityName) {
        String name = null;
        try {
            PackageManager pm = cc.getPackageManager();
            PackageInfo info = pm.getPackageInfo(packageName, PackageManager.GET_ACTIVITIES);
            name = info.applicationInfo.loadLabel(pm).toString();//shortcut name default is application'a label
            
            if (activityName.equalsIgnoreCase("com.android.settings.wifi.WifiSettings")) {
                name = getActivityLabel(pm, info, activityName);
            }
            if (activityName.equalsIgnoreCase("com.android.camera.CameraLauncher")) {
                name = getActivityLabel(pm, info, activityName);
            }
        } catch (NameNotFoundException e) {
            CTSCLog.d(TAG, "NameNotFoundException " + e);
        }
        return name;
    }
    
    private Parcelable getShortcutIcon(String packageName, String activityName) {
        Parcelable parcelable = null;
        try {
            PackageManager pm = cc.getPackageManager();
            PackageInfo info = pm.getPackageInfo(packageName, PackageManager.GET_ACTIVITIES);
            int icon = info.applicationInfo.icon;//shortcut name default is application'a icon
            
            if (null != activityName && activityName.equalsIgnoreCase("com.android.camera.CameraLauncher")) {
                icon = getActivityIcon(info, activityName);
            }
            
            Context appContext = cc.createPackageContext(packageName, 0);
            parcelable = Intent.ShortcutIconResource.fromContext(appContext, icon);
        } catch (NameNotFoundException e) {
            CTSCLog.d(TAG, "NameNotFoundException " + e);
        }
        return parcelable;
    }
    
    private Intent getShortcutIntent(String packageName, String activityName) {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_LAUNCHER);
        ComponentName cmpName = new ComponentName(packageName, activityName);
        intent.setComponent(cmpName);
        return intent;
    }
    
    private Intent constructShortcutIntent(boolean isInstall, String shortcutName, 
            Parcelable shortcutIcon, Intent shortcutIntent) {  
        CTSCLog.d(TAG, "constructShortcutIntent, isInstall = " + isInstall);
        Intent shortcut = null;
        if (isInstall) {
            shortcut = new Intent("com.android.launcher.action.INSTALL_SHORTCUT");
        } else {
            shortcut = new Intent("com.android.launcher.action.UNINSTALL_SHORTCUT");
        }
        shortcut.putExtra("duplicate", false);//not allow to install duplicate shortcut

        shortcut.putExtra(Intent.EXTRA_SHORTCUT_NAME, shortcutName);
        shortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, shortcutIcon);
        shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, shortcutIntent);        
        return shortcut;
    }
    
    private String getActivityLabel(PackageManager pm, PackageInfo pInfo, String activityName) {
        for (ActivityInfo aInfo : pInfo.activities) {
            if (aInfo.name.equalsIgnoreCase(activityName)) {
                return aInfo.loadLabel(pm).toString();
            }
        }
        throw new RuntimeException("No such activity:" + activityName);
    }
    
    private int getActivityIcon(PackageInfo pInfo, String activityName) {
        for (ActivityInfo aInfo : pInfo.activities) {
            if (aInfo.name.equalsIgnoreCase(activityName)) {
                return aInfo.icon;
            }
        }
        throw new RuntimeException("No such activity:" + activityName);
    }
}

class CheckDefaultIME extends CheckItemBase {
    private static final String TAG = " MTBF CheckDefaultIME";
    private static final String IME1 = "com.android.inputmethod.latin/.LatinIME";
    private static final String IME2 = "com.google.inputmethod.latin/com.android.inputmethod.latin.LatinIME";
    private Context cc;
    
    CheckDefaultIME (Context c, String key) {
        super(c, key);
        cc = c;
        if (key.equals(CheckItemKeySet.CI_MANUL_CHECK)) {
            setTitle(R.string.title_ime_setting);
            setNote(R.string.note_ime_setting);
        } else {
            setTitle(R.string.title_ime);
            setNote(R.string.note_ime);
            setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        }
    }

    public boolean onCheck() {
        if (getKey().equals(CheckItemKeySet.CI_MANUL_CHECK)) {
            return true;
        }
        String ime = Settings.Secure.getString(cc.getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
        if (ime.equals(IME1) || ime.equals(IME2)) {
            mResult = check_result.RIGHT;  
            setValue(R.string.value_ime_right);
        } else {
            mResult = check_result.WRONG;
            setValue(R.string.value_ime_wrong);
        }
        CTSCLog.d(TAG, "constructor the Default ime result " + mResult);
        return true;
    }

    public check_result getCheckResult() {
        /*
         * implement check function here
         */             
        return mResult;
    } 
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        if (getKey().equals(CheckItemKeySet.CI_MANUL_CHECK)) {
            return true;
        }
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        CTSCLog.i(TAG, "onReset set ime");
        Settings.Secure.putString(cc.getContentResolver(),Settings.Secure.DEFAULT_INPUT_METHOD, IME1);
        mResult = check_result.RIGHT;
        return true;
    }
}


class CheckWIFIControl extends CheckItemBase {
    private static final String TAG = " MTBF CheckWIFIControl";
    private Context cc;
    private WifiManager mWifiManager;
    private boolean isReseting = false;
    
    CheckWIFIControl (Context c, String key) {
        super(c, key);
        cc = c;
        setTitle(R.string.title_WLAN);
        setNote(R.string.note_WLAN);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);        
        CTSCLog.d(TAG, "constructor the WLAN control result " + mResult);
    }
    
    public boolean onCheck() { 
        if (isReseting) {
            setValue("Quering");
            return true;
        }
        mWifiManager = (WifiManager)cc.getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED) {
            mResult = check_result.WRONG;
            setValue(R.string.value_on);
            CTSCLog.d(TAG, "Wifi enabled");
        } else {
            mResult = check_result.RIGHT;
            setValue(R.string.value_off);
            CTSCLog.d(TAG, "Wifi disabled");
        }
        return true;
    }

    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */             
        return mResult;
    } 
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        mWifiManager.setWifiEnabled(false);
        new Handler().postDelayed(new Runnable() {
             public void run() {
                CTSCLog.d(TAG, "wifi refresh");
                sendBroadcast();                
                mResult = check_result.RIGHT;
                setValue(R.string.value_off);
           }
        }, 10);
        return true;
    }
}


class CheckDualSIMAsk extends CheckItemBase {
    private static final String TAG = " MTBF CheckDualSIMAsk";
    
    CheckDualSIMAsk (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_dual_SIM_ask);
        setNote(R.string.note_dual_SIM_ask);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckDualSIMAsk" + mResult);
    }    
}

class CheckWebFont extends CheckItemBase {
    private static final String TAG = " MTBF CheckWebFont";
    
    CheckWebFont (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_web_font);
        setNote(R.string.note_web_font);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckWebFont" + mResult);
    }    
}

class CheckLogger extends CheckItemBase {
    private static final String TAG = " MTBF CheckLogger";
    
    CheckLogger (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_log);
        setNote(R.string.note_log);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckLogger" + mResult);
    }    
}

class CheckRedScreenOff extends CheckItemBase {
    private static final String TAG = " MTBF CheckRedScreenOff";
    
    CheckRedScreenOff (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_clear_read_Screen);
        setNote(R.string.note_clear_read_Screen);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckRedScreenOff" + mResult);
    }    
}

class CheckMMSSetting extends CheckItemBase {
    private static final String TAG = " MTBF CheckMMSSetting";
    
    CheckMMSSetting (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_mms_setting);
        setNote(R.string.note_mms_setting);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckMMSSetting" + mResult);
    }    
}

class CheckEmailSetting extends CheckItemBase {
    private static final String TAG = " MTBF CheckEmailSetting";
    
    CheckEmailSetting (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_email_setting);
        setNote(R.string.note_email_setting);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckEmailSetting" + mResult);
    }    
}

class CheckDefaultStorageSetting extends CheckItemBase {
    private static final String TAG = " MTBF CheckDefaultStorageSetting";
    
    CheckDefaultStorageSetting (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_default_storage_setting);
        setNote(R.string.note_default_storage_setting);
        mResult = check_result.UNKNOWN;
        CTSCLog.d(TAG, "CheckDefaultStorageSetting" + mResult);
    }    
}

class CheckRedScreen extends CheckItemBase {
    private static final String TAG = " MTBF CheckRedScreen";
    private static final String sAEE_BUILD_INFO = "ro.aee.build.info";
    private static final String sDAL_SETTING = "persist.mtk.aee.dal";
    private static final String sAEE_MODE = "persist.mtk.aee.mode";
    private static final String PROPERTY_RO_BUILD_TYPE = "ro.build.type";
    
    CheckRedScreen(Context c, String key) {
        super(c, key);
        
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        setTitle(R.string.red_screen_title);
        setNote(R.string.red_screen_note);
    }

    public boolean onCheck() {
        CTSCLog.d(TAG, " oncheck");
        boolean enabled = false;
        
        String aeebuildinfo = SystemProperties.get(sAEE_BUILD_INFO);
        CTSCLog.d(TAG, "[ro.aee.build.info] = " + aeebuildinfo);
        if (aeebuildinfo == null || !aeebuildinfo.equals("mtk")) {
            CTSCLog.i(TAG, "this load has no red screen mechanism");
            setValue(R.string.no_red_screen);
            mResult = check_result.RIGHT;
            return true;
        }

        String dalOptionIndexString = SystemProperties.get(sDAL_SETTING);
        String aeemodeIndexString = SystemProperties.get(sAEE_MODE);
        String buildType = SystemProperties.get(PROPERTY_RO_BUILD_TYPE);
        CTSCLog.d(TAG, "[persist.mtk.aee.dal] = " + dalOptionIndexString
                + " [persist.mtk.aee.mode] = " + aeemodeIndexString
                + " [ro.build.type] = " + buildType);
        
        if (dalOptionIndexString != null && !dalOptionIndexString.isEmpty()) {
            int dalOptionIndex = Integer.valueOf(dalOptionIndexString);
            if (dalOptionIndex == 1) {
                enabled = true;
            } else {
                enabled = false;
            }
        } else if (aeemodeIndexString != null && !aeemodeIndexString.isEmpty()) {
            int aeemodeIndex = Integer.valueOf(aeemodeIndexString);
            if (aeemodeIndex == 1) {
                enabled = true;
            } else {
                enabled = false;
            }
        } else if (buildType != null && !buildType.isEmpty()) {
            if (buildType.equals("eng")) {
                enabled = true;
            } else {
                enabled = false;
            }
        }
        
        if (enabled) {
            CTSCLog.i(TAG, "red screen is enabled");
            setValue(R.string.ctsc_enabled);
            mResult = check_result.WRONG;
        } else {
            CTSCLog.i(TAG, "red screen is disabled");
            setValue(R.string.ctsc_disabled);
            mResult = check_result.RIGHT;
        }

        return true;
    }

    public check_result getCheckResult() {          
        return mResult;
    } 
    
    public boolean onReset() {
        CTSCLog.i(TAG, "onReset");
        //systemexec("adb shell aee -s off");
        systemexec("aee -s off");
        return true;
    }
    
    private StringBuffer systemexec(String command) {
        StringBuffer output = new StringBuffer();
        try {
            Process process = Runtime.getRuntime().exec(command);
            BufferedReader reader = new BufferedReader(new InputStreamReader(process
                    .getInputStream()));
            String line = new String();
            while ((line = reader.readLine()) != null) {
                output.append(line + "\n");
            }
            CTSCLog.d(TAG, output.toString());
            process.waitFor();
            reader.close();
        } catch (Exception e) {
            e.printStackTrace();
            CTSCLog.e(TAG, "Operation failed.");
        }
        return output;
    }
}

class CheckAddCallLog extends CheckItemBase {
    private static final String TAG = "CheckAddCallLog";
    
    CheckAddCallLog(Context c, String key) {
        super(c, key);
        
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        setTitle(R.string.mocall_log_title);
        setNote(R.string.mocall_log_note);
    }

    public boolean onCheck() {
        CTSCLog.d(TAG, " oncheck");
        boolean isExisted = false;
        
        final ContentResolver resolver = getContext().getContentResolver();
        Cursor c = null;
        String where = CallLog.Calls.TYPE + " = " + CallLog.Calls.OUTGOING_TYPE
                + " and " + CallLog.Calls.NUMBER + " = " + "\"10086\"";
        CTSCLog.v(TAG, "where = " + where);
        
        try {
            c = resolver.query(
                CallLog.Calls.CONTENT_URI,
                new String[] {CallLog.Calls.NUMBER},
                where,
                null,
                CallLog.Calls.DEFAULT_SORT_ORDER + " LIMIT 1");
            if (c == null || !c.moveToFirst()) {
                isExisted = false;
            } else {
                isExisted = true;
            }
            CTSCLog.v(TAG, "isExisted = " + isExisted);
        } finally {
            if (c != null) c.close();
        }
        
        if (isExisted) {
            setValue(R.string.mocall_log_existed);
            mResult = check_result.RIGHT;
        } else {
            setValue(R.string.mocall_log_unexisted);
            mResult = check_result.WRONG;
        }
        
        return true;
    }

    public check_result getCheckResult() {          
        return mResult;
    } 
    
    public boolean onReset() {
        CTSCLog.i(TAG, "onReset");
        try {
            // May block.
            Calls.addCall(null, getContext(), "10086", 0, 2, System.currentTimeMillis(), 0, 0, 0);
        } catch (Exception e) {
            CTSCLog.e(TAG, "Exception raised during adding CallLog entry: " + e);
            e.printStackTrace();
        }
        return true;
    }
}
