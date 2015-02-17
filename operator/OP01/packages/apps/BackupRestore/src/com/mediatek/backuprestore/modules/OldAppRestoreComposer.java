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

import android.content.Context;
import android.content.pm.IPackageInstallObserver;
import android.content.pm.PackageManager;
import android.net.Uri;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

public class OldAppRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/OldAppRestoreComposer";
    private int mIdx;
    private ArrayList<String> mFileNameList;

    private Object mLock = new Object();

    public OldAppRestoreComposer(Context context) {
        super(context);
    }

    @Override
    public int getModuleType() {
        return ModuleType.TYPE_APP;
    }

    @Override
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
        mFileNameList = new ArrayList<String>();
        try {
            mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true, "apps/.*\\.apk");
            result = true;
        } catch (IOException e) {
            e.printStackTrace();
        }
        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + mFileNameList.size());
        return result;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIdx >= mFileNameList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    public boolean implementComposeOneEntity() {
        boolean result = false;

        String apkFileName = mFileNameList.get(mIdx++);

        String temp = SDCardUtils.getStoragePath();
        if (temp == null) {
            return result;
        }
        int index = temp.lastIndexOf(File.separator);
        String destFileName = temp.substring(0, index + 1) + ".backup" + "/" + "temp"
                + apkFileName.subSequence(apkFileName.lastIndexOf("/"), apkFileName.length()).toString();

        MyLogger.logD(CLASS_TAG, "restoreOneMms(),mZipFileName:" + mZipFileName + "\napkFileName:" + apkFileName
                + "\ndestFileName:" + destFileName);

        try {
            BackupZip.unZipFile(mZipFileName, apkFileName, destFileName);

            File apkFile = new File(destFileName);
            if (apkFile != null && apkFile.exists()) {
                PackageManager pkgManager = mContext.getPackageManager();
                PackageInstallObserver obs = new PackageInstallObserver();

                pkgManager.installPackage(Uri.fromFile(apkFile), obs, PackageManager.INSTALL_REPLACE_EXISTING, "test");

                synchronized (mLock) {
                    while (!obs.mFinished) {
                        try {
                            mLock.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }

                    if (obs.mResult == PackageManager.INSTALL_SUCCEEDED) {
                        result = true;
                        MyLogger.logD(CLASS_TAG, "install success");
                    } else {
                        MyLogger.logD(CLASS_TAG, "install fail, result:" + obs.mResult);
                    }
                }

                apkFile.delete();
            } else {
                MyLogger.logD(CLASS_TAG, "install failed");
            }
        } catch (IOException e) {
            if (super.mReporter != null) {
                super.mReporter.onErr(e);
            }
            MyLogger.logD(CLASS_TAG, "unzipfile failed");
        } catch (Exception e) {
            e.printStackTrace();
        }

        return result;
    }

    private void delteTempFolder() {
        String temp = SDCardUtils.getStoragePath();
        if (temp != null) {
            MyLogger.logD(CLASS_TAG, temp);
            int index = temp.lastIndexOf(File.separator);
            // String folder = temp.replaceAll("backup",".backup")+ "/temp";
            String folder = temp.substring(0, index) + ".backup" + "/temp";
            MyLogger.logD(CLASS_TAG, folder);
            File file = new File(folder);
            if (file.exists() && file.isDirectory()) {
                File files[] = file.listFiles();
                for (File f : files) {
                    if (f.getName().matches(".*\\.apk")) {
                        f.delete();
                    }
                }
                file.delete();
            }
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        delteTempFolder();
    }

    @Override
    public boolean onEnd() {
        super.onEnd();
        if (mFileNameList != null) {
            mFileNameList.clear();
        }
        delteTempFolder();
        MyLogger.logD(CLASS_TAG, " onEnd()");
        return true;
    }

    class PackageInstallObserver extends IPackageInstallObserver.Stub {
        boolean mFinished = false;
        int mResult;

        public void packageInstalled(String name, int status) {
            synchronized (mLock) {
                mFinished = true;
                mResult = status;
                mLock.notifyAll();
            }
        }
    }

}
