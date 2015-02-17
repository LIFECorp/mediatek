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

package com.mediatek.backuprestore.utils;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

import com.mediatek.backuprestore.utils.Constants.MessageID;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

public class BackupFileScanner {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupFileScanner";
    private Handler mHandler;
    private Object mObject = new Object();
    private ScanThread mScanThread;

    public BackupFileScanner(Context context, Handler handler) {
        mHandler = handler;
        if (mHandler == null) {
            MyLogger.logE(CLASS_TAG, "constuctor maybe failed!cause mHandler is null");
        }
    }

    public void setHandler(Handler handler) {
        synchronized (mObject) {
            mHandler = handler;
        }
    }

    public boolean isScanning() {
        return mScanThread != null;
    }

    public void startScan() {
        mScanThread = new ScanThread();
        mScanThread.start();
    }

    public void quitScan() {
        synchronized (mObject) {
            if (mScanThread != null) {
                mScanThread.cancel();
                mScanThread = null;
                MyLogger.logV(CLASS_TAG, "quitScan");
            }
        }
    }

    private class ScanThread extends Thread {
        boolean mIsCanceled = false;

        public void cancel() {
            mIsCanceled = true;
        }

        private File[] filterFile(File[] fileList) {
            if (fileList == null) {
                return null;
            }
            List<File> list = new ArrayList<File>();
            for (File file : fileList) {
                if (mIsCanceled) {
                    break;
                }

                if (!FileUtils.isEmptyFolder(file)) {
                    list.add(file);
                }
            }
            if (mIsCanceled) {
                return null;
            } else {
                return (File[]) list.toArray(new File[0]);
            }
        }

        private File[] scanBackupFiles(String path) {

            if (path != null && !mIsCanceled) {
                return filterFile(new File(path).listFiles());
            } else {
                return null;
            }
        }

        private File[] scanPersonalBackupFiles() {
            String path = SDCardUtils.getPersonalDataBackupPath();
            return scanBackupFiles(path);
        }

        private File[] scanOldBackupFiles() {
            String path = SDCardUtils.getStoragePath();
            if (path != null) {
                int index = path.lastIndexOf(File.separator);
                path = path.substring(0, index + 1) + ".backup";
                MyLogger.logE(CLASS_TAG, "The old backup data path:" + path);
                return scanBackupFiles(path);
            } else {
                return null;
            }

        }

        @Override
        public void run() {
            File[] files = scanPersonalBackupFiles(); // folders under the Data
                                                      // folder
            HashMap<String, List<?>> result = new HashMap<String, List<?>>();
            List<BackupFilePreview> backupItems = generateBackupFileItems(files);
            if (backupItems != null && backupItems.size() > 0) {
                MyLogger.logD(CLASS_TAG, "backup data contain personal data backupItems.size()" + backupItems.size());
                result.put(Constants.SCAN_RESULT_KEY_PERSONAL_DATA, backupItems);
            }

            File[] filesOld = scanOldBackupFiles();
            if (filesOld != null && filesOld.length != 0) {
                MyLogger.logE(CLASS_TAG, "The old backup data file:" + filesOld[0]);
            } else {
                MyLogger.logE(CLASS_TAG, "no old backup data file:");
            }

            List<OldBackupFilePreview> backupItemsOld = generateOldBackupFileItems(filesOld);
            if (backupItemsOld != null && backupItemsOld.size() > 0) {
                MyLogger.logD(CLASS_TAG, "backup data contain old data and backupItemsOld.size "
                        + backupItemsOld.size());
                result.put(Constants.SCAN_RESULT_KEY_OLD_DATA, backupItemsOld);
            }

            String appPath = SDCardUtils.getAppsBackupPath();
            MyLogger.logD(CLASS_TAG, "app Path is " + appPath);
            if (appPath != null) {
                File appFolderFile = new File(appPath);
                backupItems = new ArrayList<BackupFilePreview>();
                BackupFilePreview appBackupFile = null;
                if (appFolderFile.exists() && !FileUtils.isEmptyFolder(appFolderFile)) {
                    appBackupFile = new BackupFilePreview(appFolderFile);
                    backupItems.add(appBackupFile);
                }
                if (backupItems != null && backupItems.size() > 0) {
                    MyLogger
                            .logD(CLASS_TAG, "backup data contain app data and backupItems.size()" + backupItems.size());
                    result.put(Constants.SCAN_RESULT_KEY_APP_DATA, backupItems);
                }
            }

            synchronized (mObject) {
                if (!mIsCanceled && mHandler != null) {
                    MyLogger.logE(CLASS_TAG, "send message FINISH");
                    Message msg = null;
                    if ((result.containsKey(Constants.SCAN_RESULT_KEY_PERSONAL_DATA))
                            || (result.containsKey(Constants.SCAN_RESULT_KEY_OLD_DATA))
                            || (result.containsKey(Constants.SCAN_RESULT_KEY_APP_DATA))) {
                        MyLogger.logE(CLASS_TAG, "have data");
                        msg = mHandler.obtainMessage(MessageID.SCANNER_FINISH, result);
                    } else {
                        MyLogger.logE(CLASS_TAG, "no data");
                        msg = mHandler.obtainMessage(MessageID.SCANNER_FINISH, null);
                    }

                    mHandler.sendMessage(msg);
                }
            }
            mScanThread = null;
        }

        private List<BackupFilePreview> generateBackupFileItems(File[] files) {
            if (files == null || mIsCanceled) {
                return null;
            }
            List<BackupFilePreview> list = new ArrayList<BackupFilePreview>();
            for (File file : files) {
                if (mIsCanceled) {
                    break;
                }
                BackupFilePreview backupFile = new BackupFilePreview(file);
                if (backupFile != null) {
                    list.add(backupFile);
                }
            }
            if (!mIsCanceled) {
                sort(list);
                return list;
            } else {
                return null;
            }
        }

        private List<OldBackupFilePreview> generateOldBackupFileItems(File[] files) {
            if (files == null || mIsCanceled) {
                MyLogger.logE(CLASS_TAG, "generateOldBackupFileItems:There are no old backup data");
                return null;
            }
            List<OldBackupFilePreview> list = new ArrayList<OldBackupFilePreview>();
            for (File file : files) {
                if (mIsCanceled) {
                    break;
                }
                if (file.getAbsolutePath().endsWith(".zip")) {
                    MyLogger.logD(CLASS_TAG, "OldBackupFileItems:" + file.getName());
                    OldBackupFilePreview backupFile = new OldBackupFilePreview(file);
                    if (backupFile != null) {
                        list.add(backupFile);
                    }
                }
            }
            if (!mIsCanceled) {
                // sort(list);
                return list;
            } else {
                return null;
            }
        }

        private void sort(List<BackupFilePreview> list) {
            Collections.sort(list, new Comparator<BackupFilePreview>() {
                public int compare(BackupFilePreview object1, BackupFilePreview object2) {
                    String dateLeft = object1.getBackupTime();
                    String dateRight = object2.getBackupTime();
                    if (dateLeft != null && dateRight != null) {
                        return dateRight.compareTo(dateLeft);
                    } else {
                        return 0;
                    }
                }
            });
        }
    }

}
