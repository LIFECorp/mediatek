/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.taskmanager;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageStatsObserver;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PackageStats;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemClock;
import android.text.format.Formatter;

import com.mediatek.xlog.Xlog;

import java.io.File;
import java.text.Collator;
import java.text.Normalizer;
import java.text.Normalizer.Form;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.regex.Pattern;


/**
 * Keeps track of information about all installed applications, lazy-loading
 * as needed.
 */
public class ApplicationsState {
    static final String TAG = "ApplicationsState";
    static final boolean DEBUG = false;
    static final boolean DEBUG_LOCKING = false;

    public interface Callbacks {
        void onRunningStateChanged(boolean running);
        void onPackageListChanged();
        void onRebuildComplete(ArrayList<AppEntry> apps);
        void onPackageIconChanged();
        void onPackageSizeChanged(String packageName);
        void onAllSizesComputed();
    }

    public interface AppFilter {
        void init();
        boolean filterApp(ApplicationInfo info);
    }

    static final int SIZE_UNKNOWN = -1;
    static final int SIZE_INVALID = -2;

    static final Pattern REMOVE_DIACRITICALS_PATTERN
            = Pattern.compile("\\p{InCombiningDiacriticalMarks}+");

    public static String normalize(String str) {
        String tmp = Normalizer.normalize(str, Form.NFD);
        return REMOVE_DIACRITICALS_PATTERN.matcher(tmp)
                .replaceAll("").toLowerCase();
    }

    public static class SizeInfo {
        long mCacheSize;
        long mCodeSize;
        long mDataSize;
    }
    
    public static class AppEntry extends SizeInfo {
        final File mApkFile;
        final long mId;
        String mLabel;
        long mSize;

        boolean mMounted;
        
        String getNormalizedLabel() {
            if (mNormalizedLabel != null) {
                return mNormalizedLabel;
            }
            mNormalizedLabel = normalize(mLabel);
            return mNormalizedLabel;
        }

        // Need to synchronize on 'this' for the following.
        ApplicationInfo mInfo;
        Drawable mIcon;
        String mSizeStr;
        boolean mSizeStale;
        long mSizeLoadStart;

        String mNormalizedLabel;

        AppEntry(Context context, ApplicationInfo info, long id) {
            mApkFile = new File(info.sourceDir);
            this.mId = id;
            this.mInfo = info;
            this.mSize = SIZE_UNKNOWN;
            this.mSizeStale = true;
            ensureLabel(context);
        }
        
        void ensureLabel(Context context) {
            if (this.mLabel == null || !this.mMounted) {
                if (!this.mApkFile.exists()) {
                    this.mMounted = false;
                    this.mLabel = mInfo.packageName;
                } else {
                    this.mMounted = true;
                    CharSequence label = mInfo.loadLabel(context.getPackageManager());
                    this.mLabel = label != null ? label.toString() : mInfo.packageName;
                }
            }
        }
        
        boolean ensureIconLocked(Context context, PackageManager pm) {
            if (this.mIcon == null) {
                if (this.mApkFile.exists()) {
                    this.mIcon = this.mInfo.loadIcon(pm);
                    return true;
                } else {
                    this.mMounted = false;
                    this.mIcon = context.getResources().getDrawable(
                            com.android.internal.R.drawable.sym_app_on_sd_unavailable_icon);
                }
            } else if (!this.mMounted) {
                // If the app wasn't mounted but is now mounted, reload
                // its icon.
                if (this.mApkFile.exists()) {
                    this.mMounted = true;
                    this.mIcon = this.mInfo.loadIcon(pm);
                    return true;
                }
            }
            return false;
        }
    }

    public static final Comparator<AppEntry> ALPHA_COMPARATOR = new Comparator<AppEntry>() {
        private final Collator mCollator = Collator.getInstance();
        @Override
        public int compare(AppEntry object1, AppEntry object2) {
            return mCollator.compare(object1.mLabel, object2.mLabel);
        }
    };

    public static final Comparator<AppEntry> SIZE_COMPARATOR = new Comparator<AppEntry>() {
        private final Collator mCollator = Collator.getInstance();
        @Override
        public int compare(AppEntry object1, AppEntry object2) {
            if (object1.mSize < object2.mSize) {
                return 1;
            }
            if (object1.mSize > object2.mSize) {
                return -1;
            }
            return mCollator.compare(object1.mLabel, object2.mLabel);
        }
    };

    public static final AppFilter THIRD_PARTY_FILTER = new AppFilter() {
        public void init() {
        }
        
        @Override
        public boolean filterApp(ApplicationInfo info) {
            if ((info.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0) {
                return true;
            } else if ((info.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
                return true;
            }
            return false;
        }
    };

    final Context mContext;
    final PackageManager mPm;
    PackageIntentReceiver mPackageIntentReceiver;

    boolean mResumed;
    Callbacks mCurCallbacks;

    // Information about all applications.  Synchronize on mAppEntries
    // to protect access to these.
    final HashMap<String, AppEntry> mEntriesMap = new HashMap<String, AppEntry>();
    final ArrayList<AppEntry> mAppEntries = new ArrayList<AppEntry>();
    List<ApplicationInfo> mApplications = new ArrayList<ApplicationInfo>();
    long mCurId = 1;
    String mCurComputingSizePkg;

    // Rebuilding of app list.  Synchronized on mRebuildSync.
    final Object mRebuildSync = new Object();
    boolean mRebuildRequested;
    boolean mRebuildAsync;
    AppFilter mRebuildFilter;
    Comparator<AppEntry> mRebuildComparator;
    ArrayList<AppEntry> mRebuildResult;

    /**
     * Receives notifications when applications are added/removed.
     */
    private class PackageIntentReceiver extends BroadcastReceiver {
         Locale mLastLocale;
         IntentFilter mFilter;
         IntentFilter mSdFilter;
         IntentFilter mLangChangedFilter;
         void registerReceiver() {
             mFilter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
             mFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
             mFilter.addAction(Intent.ACTION_PACKAGE_CHANGED);
             mLastLocale = Locale.getDefault();
             mFilter.addDataScheme("package");
             mContext.registerReceiver(this, mFilter);
             // Register for events related to sdcard installation.
             mSdFilter = new IntentFilter();
             mSdFilter.addAction(Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE);
             mSdFilter.addAction(Intent.ACTION_EXTERNAL_APPLICATIONS_UNAVAILABLE);
             mContext.registerReceiver(this, mSdFilter);

             mLangChangedFilter = new IntentFilter();
             mLangChangedFilter.addAction(Intent.ACTION_CONFIGURATION_CHANGED);
             mContext.registerReceiver(this, mLangChangedFilter);
             
         }
         void unregisterReceiver() {
             mContext.registerReceiver(this, mFilter);
             mContext.registerReceiver(this, mSdFilter);
             mContext.registerReceiver(this, mLangChangedFilter);
         }

         @Override
         public void onReceive(Context context, Intent intent) {
             String actionStr = intent.getAction();
             if (Intent.ACTION_PACKAGE_ADDED.equals(actionStr)) {
                 Uri data = intent.getData();
                 String pkgName = data.getEncodedSchemeSpecificPart();
                 addPackage(pkgName);
             } else if (Intent.ACTION_PACKAGE_REMOVED.equals(actionStr)) {
                 Uri data = intent.getData();
                 String pkgName = data.getEncodedSchemeSpecificPart();
                 removePackage(pkgName);
             } else if (Intent.ACTION_PACKAGE_CHANGED.equals(actionStr)) {
                 Uri data = intent.getData();
                 String pkgName = data.getEncodedSchemeSpecificPart();
                 removePackage(pkgName);
                 addPackage(pkgName);
             } else if (Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE.equals(actionStr) ||
                     Intent.ACTION_EXTERNAL_APPLICATIONS_UNAVAILABLE.equals(actionStr)) {
                 // When applications become available or unavailable (perhaps because
                 // the SD card was inserted or ejected) we need to refresh the
                 // AppInfo with new label, icon and size information as appropriate
                 // given the newfound (un)availability of the application.
                 // A simple way to do that is to treat the refresh as a package
                 // removal followed by a package addition.
                 String pkgList[] = intent.getStringArrayExtra(Intent.EXTRA_CHANGED_PACKAGE_LIST);
                 if (pkgList == null || pkgList.length == 0) {
                     // Ignore
                     return;
                 }
                 boolean avail = Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE.equals(actionStr);
                 if (avail) {
                     for (String pkgName : pkgList) {
                         removePackage(pkgName);
                         addPackage(pkgName);
                     }
                 }
             } else if (Intent.ACTION_CONFIGURATION_CHANGED.equals(actionStr)) {
                 if (!mLastLocale.equals(Locale.getDefault())) {
                     mEntriesMap.clear();
                     mAppEntries.clear();
                     mLastLocale = Locale.getDefault();
                 } // consistent with Theme
             }/*else if (Intent.ACTION_SKIN_CHANGED.equals(actionStr)) {
                    mEntriesMap.clear();
                    mAppEntries.clear();
             }*/
         }
    }

    class MainHandler extends Handler {
        static final int MSG_REBUILD_COMPLETE = 1;
        static final int MSG_PACKAGE_LIST_CHANGED = 2;
        static final int MSG_PACKAGE_ICON_CHANGED = 3;
        static final int MSG_PACKAGE_SIZE_CHANGED = 4;
        static final int MSG_ALL_SIZES_COMPUTED = 5;
        static final int MSG_RUNNING_STATE_CHANGED = 6;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_REBUILD_COMPLETE: 
                    if (mCurCallbacks != null) {
                        mCurCallbacks.onRebuildComplete((ArrayList<AppEntry>)msg.obj);
                    }
                break;
                case MSG_PACKAGE_LIST_CHANGED: 
                    if (mCurCallbacks != null) {
                        mCurCallbacks.onPackageListChanged();
                    }
                break;
                case MSG_PACKAGE_ICON_CHANGED: 
                    if (mCurCallbacks != null) {
                        mCurCallbacks.onPackageIconChanged();
                    }
                break;
                case MSG_PACKAGE_SIZE_CHANGED: 
                    if (mCurCallbacks != null) {
                        mCurCallbacks.onPackageSizeChanged((String)msg.obj);
                    }
                break;
                case MSG_ALL_SIZES_COMPUTED: 
                    if (mCurCallbacks != null) {
                        mCurCallbacks.onAllSizesComputed();
                    }
                break;
                case MSG_RUNNING_STATE_CHANGED: 
                    if (mCurCallbacks != null) {
                        mCurCallbacks.onRunningStateChanged(msg.arg1 != 0);
                    }
                break;
                default:
                   break;
            }
        }
    }

    final MainHandler mMainHandler = new MainHandler();

    // --------------------------------------------------------------

    static final Object SLOCK = new Object();
    static ApplicationsState sInstance;

    static ApplicationsState getInstance(Application app) {
        synchronized (SLOCK) {
            if (sInstance == null) {
                sInstance = new ApplicationsState(app);
            }
            return sInstance;
        }
    }

    private ApplicationsState(Application app) {
        mContext = app;
        mPm = mContext.getPackageManager();
        mThread = new HandlerThread("ApplicationsState.Loader",
                Process.THREAD_PRIORITY_BACKGROUND);
        mThread.start();
        mBackgroundHandler = new BackgroundHandler(mThread.getLooper());
        
        /**
         * This is a trick to prevent the foreground thread from being delayed.
         * The problem is that Dalvik monitors are initially spin locks, to keep
         * them lightweight.  This leads to unfair contention -- Even though the
         * background thread only holds the lock for a short amount of time, if
         * it keeps running and locking again it can prevent the main thread from
         * acquiring its lock for a long time...  sometimes even > 5 seconds
         * (leading to an ANR).
         * 
         * Dalvik will promote a monitor to a "real" lock if it detects enough
         * contention on it.  It doesn't figure this out fast enough for us
         * here, though, so this little trick will force it to turn into a real
         * lock immediately.
         */
        synchronized (mEntriesMap) {
            try {
                mEntriesMap.wait(1);
            } catch (InterruptedException e) {
                Xlog.v(TAG, "catch exception");
            }
        }
    }

    void resume(Callbacks callbacks) {
        if (DEBUG_LOCKING) {
            Xlog.v(TAG, "resume about to acquire lock...");
        }
        synchronized (mEntriesMap) {
            mCurCallbacks = callbacks;
            mResumed = true;
            if (mPackageIntentReceiver == null) {
                mPackageIntentReceiver = new PackageIntentReceiver();
                mPackageIntentReceiver.registerReceiver();
            }
            mApplications = mPm.getInstalledApplications(
                    /*PackageManager.GET_UNINSTALLED_PACKAGES |*/
                    PackageManager.GET_DISABLED_COMPONENTS);
            if (mApplications == null) {
                mApplications = new ArrayList<ApplicationInfo>();
            }
            for (int i = 0; i < mAppEntries.size(); i++) {
                mAppEntries.get(i).mSizeStale = true;
            }
            for (int i = 0; i < mApplications.size(); i++) {
                final ApplicationInfo info = mApplications.get(i);
                final AppEntry entry = mEntriesMap.get(info.packageName);
                if (entry != null) {
                    entry.mInfo = info;
                }
            }
            mCurComputingSizePkg = null;
            if (!mBackgroundHandler.hasMessages(BackgroundHandler.MSG_LOAD_ENTRIES)) {
                mBackgroundHandler.sendEmptyMessage(BackgroundHandler.MSG_LOAD_ENTRIES);
            }
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "...resume releasing lock");
            }
        }
    }

    void pause() {
        if (DEBUG_LOCKING) {
            Xlog.v(TAG, "pause about to acquire lock...");
        }
        synchronized (mEntriesMap) {
            mCurCallbacks = null;
            mResumed = false;
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "...pause releasing lock");
            }
        }
       if (mPackageIntentReceiver != null) {
           mPackageIntentReceiver.unregisterReceiver();
        }
    }

    // Creates a new list of app entries with the given filter and comparator.
    ArrayList<AppEntry> rebuild(AppFilter filter, Comparator<AppEntry> comparator) {
        synchronized (mRebuildSync) {
            mRebuildRequested = true;
            mRebuildAsync = false;
            mRebuildFilter = filter;
            mRebuildComparator = comparator;
            mRebuildResult = null;
            if (!mBackgroundHandler.hasMessages(BackgroundHandler.MSG_REBUILD_LIST)) {
                mBackgroundHandler.sendEmptyMessage(BackgroundHandler.MSG_REBUILD_LIST);
            }

            // We will wait for .25s for the list to be built.
            long waitend = SystemClock.uptimeMillis() + 250;

            while (mRebuildResult == null) {
                long now = SystemClock.uptimeMillis();
                if (now >= waitend) {
                    break;
                }
                try {
                    mRebuildSync.wait(waitend - now);
                } catch (InterruptedException e) {
                    Xlog.v(TAG, "Catch exception");
                }
            }

            mRebuildAsync = true;

            return mRebuildResult;
        }
    }

    void handleRebuildList() {
        AppFilter filter;
        Comparator<AppEntry> comparator;
        synchronized (mRebuildSync) {
            if (!mRebuildRequested) {
                return;
            }

            filter = mRebuildFilter;
            comparator = mRebuildComparator;
            mRebuildRequested = false;
            mRebuildFilter = null;
            mRebuildComparator = null;
        }

        Process.setThreadPriority(Process.THREAD_PRIORITY_FOREGROUND);

        if (filter != null) {
            filter.init();
        }
        
        List<ApplicationInfo> apps;
        synchronized (mEntriesMap) {
            apps = new ArrayList<ApplicationInfo>(mApplications);
        }

        ArrayList<AppEntry> filteredApps = new ArrayList<AppEntry>();
        if (DEBUG) {
            Xlog.i(TAG, "Rebuilding...");
        }
        for (int i = 0; i < apps.size(); i++) {
            ApplicationInfo info = apps.get(i);
            if (filter == null || filter.filterApp(info)) {
                synchronized (mEntriesMap) {
                    if (DEBUG_LOCKING) {
                        Xlog.v(TAG, "rebuild acquired lock");
                    }
                    AppEntry entry = getEntryLocked(info);
                    entry.ensureLabel(mContext);
                    if (DEBUG) {
                        Xlog.i(TAG, "Using " + info.packageName + ": " + entry);
                    }
                    filteredApps.add(entry);
                    if (DEBUG_LOCKING) {
                        Xlog.v(TAG, "rebuild releasing lock");
                    }
                }
            }
        }

        Collections.sort(filteredApps, comparator);

        synchronized (mRebuildSync) {
            if (!mRebuildRequested) {
                if (!mRebuildAsync) {
                    mRebuildResult = filteredApps;
                    mRebuildSync.notifyAll();
                } else {
                    if (!mMainHandler.hasMessages(MainHandler.MSG_REBUILD_COMPLETE)) {
                        Message msg = mMainHandler.obtainMessage(
                                MainHandler.MSG_REBUILD_COMPLETE, filteredApps);
                        mMainHandler.sendMessage(msg);
                    }
                }
            }
        }

        Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
    }

    AppEntry getEntry(String packageName) {
        if (DEBUG_LOCKING) {
            Xlog.v(TAG, "getEntry about to acquire lock...");
        }
        synchronized (mEntriesMap) {
            AppEntry entry = mEntriesMap.get(packageName);
            if (entry == null) {
                for (int i = 0; i < mApplications.size(); i++) {
                    ApplicationInfo info = mApplications.get(i);
                    if (packageName.equals(info.packageName)) {
                        entry = getEntryLocked(info);
                        break;
                    }
                }
            }
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "...getEntry releasing lock");
            }
            return entry;
        }
    }
    
    void ensureIcon(AppEntry entry) {
        if (entry.mIcon != null) {
            return;
        }
        synchronized (entry) {
            entry.ensureIconLocked(mContext, mPm);
        }
    }
    
    void requestSize(String packageName) {
        if (DEBUG_LOCKING) {
            Xlog.v(TAG, "requestSize about to acquire lock...");
        }
        synchronized (mEntriesMap) {
            AppEntry entry = mEntriesMap.get(packageName);
            if (entry != null) {
                mPm.getPackageSizeInfo(packageName, mBackgroundHandler.mStatsObserver);
            }
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "...requestSize releasing lock");
            }
        }
    }

    long sumCacheSizes() {
        long sum = 0;
        if (DEBUG_LOCKING) {
            Xlog.v(TAG, "sumCacheSizes about to acquire lock...");
        }
        synchronized (mEntriesMap) {
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "-> sumCacheSizes now has lock");
            }
            for (int i = mAppEntries.size() - 1; i >= 0; i--) {
                sum += mAppEntries.get(i).mCacheSize;
            }
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "...sumCacheSizes releasing lock");
            }
        }
        return sum;
    }
    
    int indexOfApplicationInfoLocked(String pkgName) {
        for (int i = mApplications.size() - 1; i >= 0; i--) {
            if (mApplications.get(i).packageName.equals(pkgName)) {
                return i;
            }
        }
        return -1;
    }

    void addPackage(String pkgName) {
        try {
            synchronized (mEntriesMap) {
                if (DEBUG_LOCKING) {
                    Xlog.v(TAG, "addPackage acquired lock");
                }
                if (DEBUG) {
                    Xlog.i(TAG, "Adding package " + pkgName);
                }
                if (!mResumed) {
                    // If we are not resumed, we will do a full query the
                    // next time we resume, so there is no reason to do work
                    // here.
                    if (DEBUG_LOCKING) {
                        Xlog.v(TAG, "addPackage release lock: not resumed");
                    }
                    return;
                }
                if (indexOfApplicationInfoLocked(pkgName) >= 0) {
                    if (DEBUG) {
                        Xlog.i(TAG, "Package already exists!");
                    }
                    if (DEBUG_LOCKING) {
                        Xlog.v(TAG, "addPackage release lock: already exists");
                    }
                    return;
                }
                ApplicationInfo info = mPm.getApplicationInfo(pkgName,
                        PackageManager.GET_UNINSTALLED_PACKAGES |
                        PackageManager.GET_DISABLED_COMPONENTS);
                mApplications.add(info);
                if (!mBackgroundHandler.hasMessages(BackgroundHandler.MSG_LOAD_ENTRIES)) {
                    mBackgroundHandler.sendEmptyMessage(BackgroundHandler.MSG_LOAD_ENTRIES);
                }
                if (!mMainHandler.hasMessages(MainHandler.MSG_PACKAGE_LIST_CHANGED)) {
                    mMainHandler.sendEmptyMessage(MainHandler.MSG_PACKAGE_LIST_CHANGED);
                }
                if (DEBUG_LOCKING) {
                    Xlog.v(TAG, "addPackage releasing lock");
                }
            }
        } catch (NameNotFoundException e) {
            Xlog.v(TAG, "catch exception");
        }
    }

    void removePackage(String pkgName) {
        synchronized (mEntriesMap) {
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "removePackage acquired lock");
            }
            int idx = indexOfApplicationInfoLocked(pkgName);
            if (DEBUG) {
                Xlog.i(TAG, "removePackage: " + pkgName + " @ " + idx);
            }
            if (idx >= 0) {
                AppEntry entry = mEntriesMap.get(pkgName);
                if (DEBUG) {
                    Xlog.i(TAG, "removePackage: " + entry);
                }
                if (entry != null) {
                    mEntriesMap.remove(pkgName);
                    mAppEntries.remove(entry);
                }
                mApplications.remove(idx);
                if (!mMainHandler.hasMessages(MainHandler.MSG_PACKAGE_LIST_CHANGED)) {
                    mMainHandler.sendEmptyMessage(MainHandler.MSG_PACKAGE_LIST_CHANGED);
                }
            }
            if (DEBUG_LOCKING) {
                Xlog.v(TAG, "removePackage releasing lock");
            }
        }
    }

    AppEntry getEntryLocked(ApplicationInfo info) {
        AppEntry entry = mEntriesMap.get(info.packageName);
        if (DEBUG) {
            Xlog.i(TAG, "Looking up entry of pkg " + info.packageName + ": " + entry);
        }
        if (entry == null) {
            if (DEBUG) {
                Xlog.i(TAG, "Creating AppEntry for " + info.packageName);
            }
            entry = new AppEntry(mContext, info, mCurId++);
            mEntriesMap.put(info.packageName, entry);
            mAppEntries.add(entry);
        } else if (entry.mInfo != info) {
            entry.mInfo = info;
        }
        return entry;
    }

    // --------------------------------------------------------------

    private long getTotalSize(PackageStats ps) {
        if (ps != null) {
            return ps.codeSize + ps.dataSize;
        }
        return SIZE_INVALID;
    }

    public String getSizeStr(long size) {
        if (size >= 0) {
            return Formatter.formatFileSize(mContext, size);
        }
        return null;
    }

    final HandlerThread mThread;
    final BackgroundHandler mBackgroundHandler;
    class BackgroundHandler extends Handler {
        static final int MSG_REBUILD_LIST = 1;
        static final int MSG_LOAD_ENTRIES = 2;
        static final int MSG_LOAD_ICONS = 3;
        static final int MSG_LOAD_SIZES = 4;

        boolean mRunning;

        final IPackageStatsObserver.Stub mStatsObserver = new IPackageStatsObserver.Stub() {
            public void onGetStatsCompleted(PackageStats stats, boolean succeeded) {
                boolean sizeChanged = false;
                synchronized (mEntriesMap) {
                    if (DEBUG_LOCKING) {
                        Xlog.v(TAG, "onGetStatsCompleted acquired lock");
                    }
                    AppEntry entry = mEntriesMap.get(stats.packageName);
                    if (entry != null) {
                        synchronized (entry) {
                            entry.mSizeStale = false;
                            entry.mSizeLoadStart = 0;
                            long newSize = getTotalSize(stats);
                            //Xlog.i("TaskManagerActivity","newSize: "+newSize);
                            if (entry.mSize != newSize ||
                                    entry.mCacheSize != stats.cacheSize ||
                                    entry.mCodeSize != stats.codeSize ||
                                    entry.mDataSize != stats.dataSize) {
                                entry.mSize = newSize;
                                entry.mCacheSize = stats.cacheSize;
                                entry.mCodeSize = stats.codeSize;
                                entry.mDataSize = stats.dataSize;
                                entry.mSizeStr = getSizeStr(entry.mSize);
                                if (DEBUG) {
                                    Xlog.i(TAG, "Set size of " + entry.mLabel + " " + entry
                                        + ": " + entry.mSizeStr);
                                }
                                sizeChanged = true;
                            }
                        }
                        if (sizeChanged) {
                            Message msg = mMainHandler.obtainMessage(
                                    MainHandler.MSG_PACKAGE_SIZE_CHANGED, stats.packageName);
                            mMainHandler.sendMessage(msg);
                        }
                    }
                    if (mCurComputingSizePkg == null
                            || mCurComputingSizePkg.equals(stats.packageName)) {
                        mCurComputingSizePkg = null;
                        sendEmptyMessage(MSG_LOAD_SIZES);
                    }
                    if (DEBUG_LOCKING) {
                        Xlog.v(TAG, "onGetStatsCompleted releasing lock");
                    }
                }
            }
        };

        BackgroundHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            // Always try rebuilding list first thing, if needed.
            handleRebuildList();
            int numDone = 0;
            switch (msg.what) {
                case MSG_REBUILD_LIST:
                    break;
                case MSG_LOAD_ENTRIES: 
                    numDone = 0;
                    synchronized (mEntriesMap) {
                        if (DEBUG_LOCKING) {
                            Xlog.v(TAG, "MSG_LOAD_ENTRIES acquired lock");
                        }
                        for (int i = 0; i < mApplications.size() && numDone < 6; i++) {
                            if (!mRunning) {
                                mRunning = true;
                                Message m = mMainHandler.obtainMessage(
                                        MainHandler.MSG_RUNNING_STATE_CHANGED, 1);
                                mMainHandler.sendMessage(m);
                            }
                            ApplicationInfo info = mApplications.get(i);
                            if (mEntriesMap.get(info.packageName) == null) {
                                numDone++;
                                getEntryLocked(info);
                            }
                        }
                        if (DEBUG_LOCKING) {
                            Xlog.v(TAG, "MSG_LOAD_ENTRIES releasing lock");
                        }
                    }

                    if (numDone >= 6) {
                        sendEmptyMessage(MSG_LOAD_ENTRIES);
                    } else {
                        sendEmptyMessage(MSG_LOAD_ICONS);
                    }
                    break;
                
                case MSG_LOAD_ICONS: 
                    numDone = 0;
                    synchronized (mEntriesMap) {
                        if (DEBUG_LOCKING) {
                            Xlog.v(TAG, "MSG_LOAD_ICONS acquired lock");
                        }
                        for (int i = 0; i < mAppEntries.size() && numDone < 2; i++) {
                            AppEntry entry = mAppEntries.get(i);
                            if (entry.mIcon == null || !entry.mMounted) {
                                synchronized (entry) {
                                    if (entry.ensureIconLocked(mContext, mPm)) {
                                        if (!mRunning) {
                                            mRunning = true;
                                            Message m = mMainHandler.obtainMessage(
                                                    MainHandler.MSG_RUNNING_STATE_CHANGED, 1);
                                            mMainHandler.sendMessage(m);
                                        }
                                        numDone++;
                                    }
                                }
                            }
                        }
                        if (DEBUG_LOCKING) {
                            Xlog.v(TAG, "MSG_LOAD_ICONS releasing lock");
                        }
                        
                    }
                    if (numDone > 0) {
                        if (!mMainHandler.hasMessages(MainHandler.MSG_PACKAGE_ICON_CHANGED)) {
                            mMainHandler.sendEmptyMessage(MainHandler.MSG_PACKAGE_ICON_CHANGED);
                        }
                    }
                    if (numDone >= 2) {
                        sendEmptyMessage(MSG_LOAD_ICONS);
                    } else {
                        sendEmptyMessage(MSG_LOAD_SIZES);
                    }
                    break;
                
                case MSG_LOAD_SIZES:
                    synchronized (mEntriesMap) {
                        if (DEBUG_LOCKING) {
                            Xlog.v(TAG, "MSG_LOAD_SIZES acquired lock");
                        }
                        if (mCurComputingSizePkg != null) {
                            if (DEBUG_LOCKING) {
                                Xlog.v(TAG, "MSG_LOAD_SIZES releasing: currently computing");
                            }
                            return;
                        }

                        long now = SystemClock.uptimeMillis();
                        for (int i = 0; i < mAppEntries.size(); i++) {
                            AppEntry entry = mAppEntries.get(i);
                            if (entry.mSize == SIZE_UNKNOWN || entry.mSizeStale) {
                                if (entry.mSizeLoadStart == 0 ||
                                        (entry.mSizeLoadStart < (now - 20 * 1000))) {
                                    if (!mRunning) {
                                        mRunning = true;
                                        Message m = mMainHandler.obtainMessage(
                                                MainHandler.MSG_RUNNING_STATE_CHANGED, 1);
                                        mMainHandler.sendMessage(m);
                                    }
                                    entry.mSizeLoadStart = now;
                                    mCurComputingSizePkg = entry.mInfo.packageName;
                                    mPm.getPackageSizeInfo(mCurComputingSizePkg, mStatsObserver);
                                }
                                if (DEBUG_LOCKING) {
                                    Xlog.v(TAG, "MSG_LOAD_SIZES releasing: now computing");
                                }
                                return;
                            }
                        }
                        if (!mMainHandler.hasMessages(MainHandler.MSG_ALL_SIZES_COMPUTED)) {
                            mMainHandler.sendEmptyMessage(MainHandler.MSG_ALL_SIZES_COMPUTED);
                            mRunning = false;
                            Message m = mMainHandler.obtainMessage(
                                    MainHandler.MSG_RUNNING_STATE_CHANGED, 0);
                            mMainHandler.sendMessage(m);
                        }
                        if (DEBUG_LOCKING) {
                            Xlog.v(TAG, "MSG_LOAD_SIZES releasing lock");
                        }
                    }
                    break;
                default:
                    break;
            }
        }

    }
}
