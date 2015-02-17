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
import android.util.Log;

import com.mediatek.backuprestore.utils.Constants.ModulePath;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

public class BackupFilePreview {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupFilePreview";

    private File mFolderName = null;
    private long mSize = 0;
    private String mBackupTime;
    private ArrayList<Integer> mModuleTypeList;
    private HashMap<Integer, Integer> mNumberMap = new HashMap<Integer, Integer>();

    public BackupFilePreview(File file) {
        if (file == null) {
            MyLogger.logE(CLASS_TAG, "constractor error! file is null");
            return;
        }
        mNumberMap.clear();
        mFolderName = file;
        MyLogger.logD(CLASS_TAG, "new BackupFilePreview: file is " + file.getAbsolutePath());
        computeSize();
    }

    private void computeSize() {
        mSize = FileUtils.computeAllFileSizeInFolder(mFolderName);
    }

    public File getFile() {
        return mFolderName;
    }

    public String getFileName() {
        return mFolderName.getName();
    }

    public String getBackupTime() {
        if (mBackupTime == null) {
            Long time = mFolderName.lastModified();
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
            mBackupTime = dateFormat.format(new Date(time));
        }
        return mBackupTime;
    }

    public void setBackupTime(String backupTime) {
        this.mBackupTime = backupTime;
    }

    public long getFileSize() {
        return mSize;
    }

    public ArrayList<Integer> getBackupModules(Context context) {
        if (mModuleTypeList == null) {
            mModuleTypeList = peekBackupModules(context);
        }
        return mModuleTypeList;
    }

    /**
     * parse backup items.
     */
    private ArrayList<Integer> peekBackupModules(Context context) {

        File[] files = mFolderName.listFiles();
        ArrayList<Integer> list = new ArrayList<Integer>();
        HashMap hm = new HashMap();
        if (files != null) {
            for (File file : files) {
                if (file.isDirectory() && !FileUtils.isEmptyFolder(file)) {
                    String name = file.getName();
                    MyLogger.logD(CLASS_TAG, "parseItemTypes: file finded = " + name);
                    if (name.equalsIgnoreCase(ModulePath.FOLDER_CONTACT)) {
                        hm.put(ModulePath.FOLDER_CONTACT, true);
                        MyLogger.logD(CLASS_TAG, "list add:Module" + name);
                    } else if (name.equalsIgnoreCase(ModulePath.FOLDER_PICTURE)) {
                        hm.put(ModulePath.FOLDER_PICTURE, true);
                        MyLogger.logD(CLASS_TAG, "list add:Module" + name);
                    } else if (name.equalsIgnoreCase(ModulePath.FOLDER_CALENDAR)) {
                        hm.put(ModulePath.FOLDER_CALENDAR, true);
                    } else if (name.equalsIgnoreCase(ModulePath.FOLDER_MUSIC)) {
                        hm.put(ModulePath.FOLDER_MUSIC, true);
                        MyLogger.logD(CLASS_TAG, "list add:Module" + name);
                    } else if (name.equalsIgnoreCase(ModulePath.FOLDER_NOTEBOOK)) {
                        hm.put(ModulePath.FOLDER_NOTEBOOK, true);
                        MyLogger.logD(CLASS_TAG, "list add:Module" + name);
                    } else if (name.equalsIgnoreCase(ModulePath.FOLDER_MMS)) {
                        hm.put(ModulePath.FOLDER_MMS, true);
                        MyLogger.logD(CLASS_TAG, "list add:Module" + name);
                    } else if (name.equalsIgnoreCase(ModulePath.FOLDER_SMS)) {
                        hm.put(ModulePath.FOLDER_SMS, true);
                        MyLogger.logD(CLASS_TAG, "list add:Module" + name);
                    }
                }
            }
        }
        if (hm.containsKey(ModulePath.FOLDER_CONTACT)) {
            list.add(ModuleType.TYPE_CONTACT);
            MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_CONTACT);
        }
        if (hm.containsKey(ModulePath.FOLDER_SMS)) {
            if (!list.contains(ModuleType.TYPE_MESSAGE)) {
                list.add(ModuleType.TYPE_MESSAGE);
                MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_SMS);
            }
        }
        if (hm.containsKey(ModulePath.FOLDER_MMS)) {
            if (!list.contains(ModuleType.TYPE_MESSAGE)) {
                list.add(ModuleType.TYPE_MESSAGE);
                MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_MMS);
            }
        }
        if (hm.containsKey(ModulePath.FOLDER_PICTURE)) {
            list.add(ModuleType.TYPE_PICTURE);
            MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_PICTURE);
        }
        if (hm.containsKey(ModulePath.FOLDER_CALENDAR)) {
            list.add(ModuleType.TYPE_CALENDAR);
            MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_CALENDAR);
        }
        if (hm.containsKey(ModulePath.FOLDER_MUSIC)) {
            list.add(ModuleType.TYPE_MUSIC);
            MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_MUSIC);
        }
        if (hm.containsKey(ModulePath.FOLDER_NOTEBOOK)) {
            list.add(ModuleType.TYPE_NOTEBOOK);
            MyLogger.logD(CLASS_TAG, "parseItemTypes:Module" + ModulePath.FOLDER_NOTEBOOK);
        }

        return list;
    }

    public int getItemCount(int type) {
        int count = mNumberMap.get(type);
        Log.v(CLASS_TAG, "getItemCount: type = " + type + ",count = " + count);
        return count;
    }

    public void setItemCount(int type, int count) {
        Log.v(CLASS_TAG, "setItemCount: type = " + type + ",count = " + count);
        mNumberMap.put(type, count);
    }
}
