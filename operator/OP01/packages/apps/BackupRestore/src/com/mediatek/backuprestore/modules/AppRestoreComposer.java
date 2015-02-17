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

package com.mediatek.backuprestore.modules;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageDeleteObserver;
import android.content.pm.IPackageInstallObserver;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.IBinder;

import com.mediatek.backuprestore.utils.BackupRestoreSrv;
import com.mediatek.backuprestore.utils.FileUtils;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.TarToolUtils;
import com.mediatek.common.featureoption.FeatureOption;

import java.io.File;
import java.io.IOException;
import java.util.List;

public class AppRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AppRestoreComposer";
    private int mIndex;
    private List<String> mFileNameList;
    private Object mLock = new Object();
    private boolean mIsAidlServiceConnected;

    public AppRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_APP;
    }

    public int getCount() {
        int count = 0;
        if (mFileNameList != null) {
            count = mFileNameList.size();
        }
        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;
        if (mParams != null) {
            mFileNameList = mParams;
            result = true;
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ", count:" + getCount());
        return result;
    }

    public boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIndex >= mFileNameList.size());
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    public boolean implementComposeOneEntity() {
        boolean result = true;
        if (mFileNameList == null || mIndex >= mFileNameList.size()) {
            return false;
        }

        synchronized (mLock) {
            while (!mIsAidlServiceConnected) {
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        try {
            String apkFileName = mFileNameList.get(mIndex++);
            File apkFile = new File(apkFileName);
            if (apkFile == null || !apkFile.exists()) {
                return false;
            }
            PackageManager packageManager = mContext.getPackageManager();
            PackageInstallObserver installObserver = new PackageInstallObserver();

            packageManager.installPackage(Uri.fromFile(apkFile), installObserver,
                    PackageManager.INSTALL_REPLACE_EXISTING, null);
            synchronized (mLock) {
                while (!installObserver.mFinished) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                if (installObserver.mResult == PackageManager.INSTALL_SUCCEEDED) {
                    MyLogger.logD(CLASS_TAG, "install success");
                } else {
                    MyLogger.logD(CLASS_TAG, "install fail, result:" + installObserver.mResult);
                    return false;
                }
            }
            MyLogger.logD(CLASS_TAG, "implementComposeOneEntity()");
            if (!FeatureOption.OP01_CTS_COMPATIBLE) {
                boolean tmpResult = false;
                if (mService != null) {
                    tmpResult = mService.disableApp(installObserver.mPackageName);
                }
                MyLogger.logD(CLASS_TAG, "disableApp()" + tmpResult);

                result = appDataRestore(installObserver.mPackageName);

                if (mService != null) {
                    tmpResult = mService.enableApp(installObserver.mPackageName);
                }
                MyLogger.logD(CLASS_TAG, "enableApp()" + tmpResult);
            } else {
                MyLogger.logD(CLASS_TAG, "this is CTS project,can't start native service to restore app data");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    public boolean appDataRestore(String packageName) {
        boolean result = true;

        // get the info of the app that being restored.
        ApplicationInfo appInfo = new ApplicationInfo();
        try {
            PackageManager pm = mContext.getPackageManager();
            appInfo = pm.getApplicationInfo(packageName, PackageManager.GET_ACTIVITIES);
        } catch (NameNotFoundException e) {
            e.printStackTrace();
            return false;
        }

        int uid = appInfo.uid;
        MyLogger.logD(CLASS_TAG, "appInfo.uid " + appInfo.uid);

        String appDataTar = mParentFolderPath + File.separator + appInfo.packageName + ".tar";
        String dearchiveTo = "/data/data/com.mediatek.backuprestore/tmp";
        String appDataDest = appInfo.dataDir;
        String unTarPath = dearchiveTo + "/data/data/" + appInfo.packageName;
        // compate with data/data/###
        String tempPath = dearchiveTo + "/" + appInfo.packageName;
        MyLogger.logD(CLASS_TAG, "appDataTar " + appDataTar);
        MyLogger.logD(CLASS_TAG, "unTarPath " + unTarPath);

        File path = new File(dearchiveTo);
        if (!path.exists()) {
            path.mkdirs();
        }

        if ((new File(appDataTar)).exists()) {
            MyLogger.logD(CLASS_TAG, "File(appDataTar)!= null");
            try {
                TarToolUtils.dearchive(appDataTar, dearchiveTo);
            } catch (IOException e) {
                e.printStackTrace();
                File unTarFile = new File(dearchiveTo);
                if (unTarFile.exists()) {
                    FileUtils.deleteFileOrFolder(unTarFile);
                }
                MyLogger.logD(CLASS_TAG, "dearchive failed");
                result = false;
            }
            if (result) {
                if (!(new File(unTarPath).exists()) && new File(tempPath).exists()) {
                    unTarPath = tempPath;
                    MyLogger.logD(CLASS_TAG, "tatPath = " + unTarPath);
                }
                int dataRestoreResult = new BackupRestoreSrv().restore(unTarPath, appDataDest);
                if (dataRestoreResult < 0) {
                    MyLogger.logD(CLASS_TAG, "apk install succeed,but the app data restore fail");
                    result = false;
                }
            }
            FileUtils.deleteFileOrFolder(path);
            if (!result) {
                if (uninstallPackage(packageName)) {
                    MyLogger.logD(CLASS_TAG, "uninstall successed");
                } else {
                    MyLogger.logD(CLASS_TAG, "uninstall failed");
                }
            }
        } else {
            MyLogger.logD(CLASS_TAG, "no tar app data exit");
        }
        return result;
    }

    public void onStart() {
        super.onStart();
        Intent intent = new Intent();
        intent.setAction("com.mediatek.backuprestore.modules.AIDL_SERVICE");
        try {
            mContext.bindService(intent, mConn, Service.BIND_AUTO_CREATE);
        } catch (Exception e) {
            mIsAidlServiceConnected = true;
            e.printStackTrace();
        }

    }

    public boolean onEnd() {
        super.onEnd();
        if (mFileNameList != null) {
            mFileNameList.clear();
        }
        // delteTempFolder();
        MyLogger.logD(CLASS_TAG, "onEnd()");
        try {
            mContext.unbindService(mConn);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return true;
    }

    class PackageInstallObserver extends IPackageInstallObserver.Stub {
        private boolean mFinished = false;
        private int mResult;
        private String mPackageName = " ";

        @Override
        public void packageInstalled(String name, int status) {
            MyLogger.logD(CLASS_TAG, "packageInstalled" + name);
            synchronized (mLock) {
                mFinished = true;
                mResult = status;
                mPackageName = name;
                mLock.notifyAll();
            }
        }
    }

    public boolean uninstallPackage(String packageName) {
        PackageManager packageManager = mContext.getPackageManager();
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
                MyLogger.logD(CLASS_TAG, "uninstall success");
                return true;
            } else {
                MyLogger.logD(CLASS_TAG, "uninstall fail, result:" + deleteObserver.mResult);
                return false;
            }
        }
    }

    class PackageDeleteObserver extends IPackageDeleteObserver.Stub {
        private boolean mFinished = false;
        private int mResult;

        @Override
        public void packageDeleted(String name, int status) {
            MyLogger.logD(CLASS_TAG, "packageInstalled" + name);
            synchronized (mLock) {
                mFinished = true;
                mResult = status;
                mLock.notifyAll();
            }
        }
    }

    AppService mService;
    private ServiceConnection mConn = new ServiceConnection() {
        @Override
        public void onServiceConnected(final ComponentName name, final IBinder service) {
            MyLogger.logD(CLASS_TAG, "onServiceConnected" + name);
            mService = AppService.Stub.asInterface(service);
            synchronized (mLock) {
                mIsAidlServiceConnected = true;
                mLock.notifyAll();
            }
        }

        @Override
        public void onServiceDisconnected(final ComponentName name) {
            MyLogger.logD(CLASS_TAG, "onServiceDisconnected" + name);
            mIsAidlServiceConnected = false;
            mService = null;
        }
    };

}
