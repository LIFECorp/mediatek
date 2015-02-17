/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.apkinstaller;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.FileObserver;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.storage.StorageManager;
import android.os.storage.StorageEventListener;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.AnimationUtils;

/**
 * @author mtk54093
 * @function: list the apk files which are in the SD card and are not installed
 */
public class APKInstaller extends PreferenceActivity {

    private static final String TAG = "APKInstaller";

    private View mLoadingContainer;
    private View mListContainer;
    private File mFileRoot;
    private PackageManager mPm;
    private PreferenceScreen mParentPreference;

    private static final int MENUID_FRESH = 0;
    private static final int MSG_REFRESH_UI = MENUID_FRESH + 1;
    private static final int MSG_SHOW_LOADING = MENUID_FRESH + 2;
    private static final int MSG_FILE_OBSERVER = MENUID_FRESH + 3;

    private static final int MSG_RELOAD_APK_FILE = MENUID_FRESH + 4;
    private static final int MSG_LOAD_ENTRIES = MENUID_FRESH + 5;
    private static final int MSG_LOAD_ICONS = MENUID_FRESH + 6;

    private HandlerThread mThread;
    private BackgroundHandler mBackgroundHandler;
    private PkgAddBroadcastReceiver mPkgReceiver;

    private boolean mExitAddPrf = false;

    private StorageManager mStorageManager = null;

    List<String> mVolumePathList = new ArrayList<String>();
    private HashMap<String, FileObserver> mFileObserverMap = new HashMap<String, FileObserver>();

    private List<File> mFiles = new ArrayList<File>();
    private HashMap<String, AppEntry> mEntriesMap = new HashMap<String, AppEntry>();
    private ArrayList<AppEntry> mAppEntries = new ArrayList<AppEntry>();
    private HashMap<String, AppEntry> mAppDetailMap = new HashMap<String, AppEntry>();
    private ArrayList<AppEntry> mAppDetail = new ArrayList<AppEntry>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mPm = getPackageManager();

        setContentView(R.layout.list);
        mLoadingContainer = findViewById(R.id.loading_container);
        mListContainer = findViewById(R.id.list_container);

        addPreferencesFromResource(R.xml.apk_installer);
        mParentPreference = getPreferenceScreen();
        mParentPreference.setOrderingAsAdded(false);

        if (mStorageManager == null) {
            mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
            mStorageManager.registerListener(mStorageListener);
        }

        // get the provided path list
        String[] mPathList = mStorageManager.getVolumePaths();

        int len = mPathList.length;
        for (int i = 0; i < len; i++) {
            if (!mStorageManager.getVolumeState(mPathList[i]).equals(
                    "not_present")) {
                mVolumePathList.add(mPathList[i]);
            }
        }

        for (int i = 0; i < mVolumePathList.size(); i++) {
            mFileObserverMap.put(mVolumePathList.get(i), new FileObserver(
                    mVolumePathList.get(i)) {
                @Override
                public void onEvent(int event, String path) {
                    switch (event) {
                    case FileObserver.CREATE:
                    case FileObserver.DELETE:
                        mUiHandler.sendEmptyMessage(MSG_SHOW_LOADING);
                        mBackgroundHandler
                                .sendEmptyMessage(MSG_RELOAD_APK_FILE);
                        break;
                    default:
                        break;
                    }
                }
            });
        }
        Log.d(TAG, "mVolumePathList size" + mVolumePathList.size());

        mLoadingContainer.setVisibility(View.VISIBLE);
        mListContainer.setVisibility(View.INVISIBLE);

        mThread = new HandlerThread("APKInstaller.Loader",
                Process.THREAD_PRIORITY_BACKGROUND);
        mThread.start();
        mBackgroundHandler = new BackgroundHandler(mThread.getLooper());
        mBackgroundHandler.sendEmptyMessage(MSG_RELOAD_APK_FILE);

        // response to the package status
        mPkgReceiver = new PkgAddBroadcastReceiver();
        IntentFilter filter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
        filter.addDataScheme("package");
        registerReceiver(mPkgReceiver, filter);
    }

    @Override
    public void onResume() {
        Log.i(TAG, " start onResume()");
        super.onResume();
        mUiHandler.sendEmptyMessage(MSG_FILE_OBSERVER);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENUID_FRESH, 0, R.string.menu_stats_refresh);
        // .setIcon(com.android.internal.R.drawable.ic_menu_refresh);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENUID_FRESH:
            mUiHandler.sendEmptyMessage(MSG_SHOW_LOADING);
            mBackgroundHandler.sendEmptyMessage(MSG_LOAD_ENTRIES);
            return true;
        }
        return false;
    }

    @Override
    protected void onPause() {
        super.onPause();
        for (int i = 0; i < mFileObserverMap.size(); i++) {
            if (mStorageManager.getVolumeState(mVolumePathList.get(i)).equals(
                    Environment.MEDIA_MOUNTED)) {
                mFileObserverMap.get(mVolumePathList.get(i)).stopWatching();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mExitAddPrf = true;
        if (mStorageManager != null && mStorageListener != null) {
            mStorageManager.unregisterListener(mStorageListener);
        }
        unregisterReceiver(mPkgReceiver);
        if (null != mThread) {
            mThread.quit();
        }
    }

    final Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_SHOW_LOADING:
                showLoadingContainer();
                mParentPreference.removeAll();
                break;
            case MSG_REFRESH_UI:
                int size = mAppDetail.size();
                Log.i(TAG, "mAppDetail.size() when refreshing UI: " + size);
                for (int i = 0; i < size; i++) {
                    AppEntry app = mAppDetail.get(i);
                    addPreference(app.icon, app.name, app.intent);
                }
                showListContainer();
                break;
            case MSG_FILE_OBSERVER:
                startObserver();
                break;
            default:
                break;
            }
        }
    };

    class BackgroundHandler extends Handler {
        public BackgroundHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_RELOAD_APK_FILE:
                // get apk files
                mFiles.clear();
                mEntriesMap.clear();
                mAppEntries.clear();
                mAppDetailMap.clear();
                mAppDetail.clear();
                // get the file existed in the mounted sd cards.
                for (int i = 0; i < mVolumePathList.size(); i++) {
                    if (mStorageManager.getVolumeState(mVolumePathList.get(i))
                            .equals(Environment.MEDIA_MOUNTED)) {
                        getApkFile(new File(mVolumePathList.get(i)));
                    }
                }
                // load the entries
                Log.i(TAG, "Files size: " + mFiles.size());
                mBackgroundHandler.sendEmptyMessage(MSG_LOAD_ENTRIES);
                break;
            case MSG_LOAD_ENTRIES:
                loadAppEntry();
                break;
            case MSG_LOAD_ICONS:
                loadIcon();
                break;
            default:
                break;
            }
        }
    }

    private void getApkFile(File file) {
        File[] fileList = file.listFiles();
        if (fileList == null) {
            // if null ,stands for the file is not a directory
            return;
        }
        for (File f : fileList) {
            if (f.isDirectory()) {
                getApkFile(f);
            } else {
                if (mExitAddPrf) {
                    return;
                }
                String fileName = f.getName();
                int start = fileName.lastIndexOf(".") + 1;
                int end = fileName.length();
                String extension = fileName.substring(start, end);
                if ("apk".equals(extension)) {
                    mFiles.add(f);
                }
            }
        }
    }

    private void loadAppEntry() {
        // use the variable numDone to control message cycle
        int numDone = 0;
        for (int i = 0; i < mFiles.size() && numDone < 6; i++) {
            File f = mFiles.get(i);
            String filePath = f.getAbsolutePath();
            PackageInfo info = mPm.getPackageArchiveInfo(filePath, 0);
            if (info != null) {
                String packageName = info.packageName;
                if (!isPkgInstalled(packageName)) {
                    if (mEntriesMap.get(packageName) == null) {
                        AppEntry entry = new AppEntry(info, f);
                        mEntriesMap.put(packageName, entry);
                        mAppEntries.add(entry);
                        numDone++;
                    }
                }
            }
        }

        if (numDone >= 6) {
            mBackgroundHandler.sendEmptyMessage(MSG_LOAD_ENTRIES);
        } else {
            mBackgroundHandler.sendEmptyMessage(MSG_LOAD_ICONS);
        }
    }

    private void loadIcon() {
        int numDone = 0;
        Log.i(TAG, "------loadIcon()----mAppEntries.size() "
                + mAppEntries.size());
        for (int i = 0; i < mAppEntries.size() && numDone < 2; i++) {
            AppEntry entry = mAppEntries.get(i);
            PackageInfo info = entry.info;
            if (mAppDetailMap.get(info.packageName) == null) {
                File f = entry.file;
                Drawable icon = getApkIcon(info, f.getAbsolutePath());
                String name = f.getName();
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.fromFile(f),
                        "application/vnd.android.package-archive");
                AppEntry detail = new AppEntry(icon, name, intent);
                mAppDetailMap.put(info.packageName, detail);
                mAppDetail.add(detail);
                numDone++;
            }
        }
        if (numDone >= 2) {
            mBackgroundHandler.sendEmptyMessage(MSG_LOAD_ICONS);
        }
        mUiHandler.sendEmptyMessage(MSG_REFRESH_UI);
    }

    class AppEntry {
        Drawable icon;
        String name;
        Intent intent;

        PackageInfo info;
        File file;

        AppEntry(Drawable icon, String name, Intent intent) {
            this.icon = icon;
            this.name = name;
            this.intent = intent;
        }

        AppEntry(PackageInfo info, File f) {
            this.info = info;
            this.file = f;
        }
    }

    private Drawable getApkIcon(PackageInfo info, String filePath) {
        ApplicationInfo appInfo = info.applicationInfo;
        if (appInfo != null) {
            filePath.trim();
            appInfo.publicSourceDir = appInfo.sourceDir = filePath;
            Drawable icon = mPm.getApplicationIcon(appInfo);
            return icon;
        }
        return null;
    }

    private void addPreference(Drawable icon, String name, Intent intent) {
        if (mParentPreference.findPreference(name) == null) {
            IconPreferenceScreen preference = new IconPreferenceScreen(this);
            preference.setKey(name);
            preference.setTitle(name);
            if (icon != null) {
                preference.setIcon(icon);
            }
            preference.setIntent(intent);
            mParentPreference.addPreference(preference);
        }

    }

    private void showListContainer() {
        if (View.VISIBLE == mLoadingContainer.getVisibility()) {
            mLoadingContainer.startAnimation(AnimationUtils.loadAnimation(
                    APKInstaller.this, android.R.anim.fade_out));
            mListContainer.startAnimation(AnimationUtils.loadAnimation(
                    APKInstaller.this, android.R.anim.fade_in));
        }
        mListContainer.setVisibility(View.VISIBLE);
        mLoadingContainer.setVisibility(View.INVISIBLE);
    }

    private void showLoadingContainer() {
        if (View.INVISIBLE == mLoadingContainer.getVisibility()) {
            mLoadingContainer.setVisibility(View.VISIBLE);
            mListContainer.setVisibility(View.INVISIBLE);
        }
    }

    void startObserver() {
        for (int i = 0; i < mFileObserverMap.size(); i++) {
            if (mStorageManager.getVolumeState(mVolumePathList.get(i)).equals(
                    Environment.MEDIA_MOUNTED)) {
                mFileObserverMap.get(mVolumePathList.get(i)).startWatching();
            }
        }
    }

    // judge packageName apk is installed or not
    private boolean isPkgInstalled(String packageName) {
        if (packageName != null) {
            try {
                mPm.getPackageInfo(packageName, 0);
            } catch (NameNotFoundException e) {
                return false;
            }
        } else {
            Log.i(TAG, "the package name cannot be null!");
            return false;
        }
        return true;
    }

    StorageEventListener mStorageListener = new StorageEventListener() {
        @Override
        public void onStorageStateChanged(String path, String oldState,
                String newState) {
            Log.i(TAG, "Received storage state changed notification that "
                    + path + " changed state from " + oldState + " to "
                    + newState);

            boolean flag = true;
            for (int i = 0; i < mVolumePathList.size(); i++) {
                flag = flag
                        && mStorageManager.getVolumeState(
                                mVolumePathList.get(i)).equals(
                                Environment.MEDIA_MOUNTED);
            }
            if (flag) {
                mExitAddPrf = true;
                APKInstaller.this.finish();
            }

            // modify the watching file observe
            if (oldState.equals(Environment.MEDIA_MOUNTED)) {
                mFileObserverMap.get(path).stopWatching();
            } else if (newState.equals(Environment.MEDIA_MOUNTED)) {
                mFileObserverMap.get(path).startWatching();
            }

            // reload the system
            mUiHandler.sendEmptyMessage(MSG_SHOW_LOADING);
            mBackgroundHandler.sendEmptyMessage(MSG_RELOAD_APK_FILE);
        }
    };

    private class PkgAddBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "PkgAddBroadcastReceiver");
            mUiHandler.sendEmptyMessage(MSG_SHOW_LOADING);
            mBackgroundHandler.sendEmptyMessage(MSG_RELOAD_APK_FILE);
        }
    }

}
