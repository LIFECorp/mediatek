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
import android.content.Intent;
import android.net.Uri;
import android.provider.MediaStore.Audio;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.Constants.ModulePath;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

/**
 * Describe class <code>MusicRestoreComposer</code> here.
 * 
 * @author
 * @version 1.0
 */
public class MusicRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MusicRestoreComposer";
    private int mIndex;
    // private File[] mFileList;
    private ArrayList<String> mFileNameList;
    private String mDestPath;
    private String mZipFileName;
    private boolean mImport;
    private String mDestFileName;
    // private ArrayList<String> mExistFileList = null;
    // private static final String[] mProjection = new String[] {
    // Audio.Media._ID, Audio.Media.DATA };

    /**
     * Creates a new <code>MusicRestoreComposer</code> instance.
     * 
     * @param context
     *            a <code>Context</code> value
     */
    public MusicRestoreComposer(Context context) {
        super(context);
    }

    /**
     * Describe <code>init</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean init() {
        boolean result = false;
        String path = mParentFolderPath + File.separator + ModulePath.FOLDER_MUSIC;
        mFileNameList = new ArrayList<String>();
        File folder = new File(path);
        if (folder.exists() && folder.isDirectory()) {
            try {
                mZipFileName = path + File.separator + ModulePath.MUSICZIP;
                mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true, ".*");
                String tmppath = (new File(mParentFolderPath)).getParent();
                mDestPath = tmppath.subSequence(0, tmppath.length() - 12)
                        + File.separator
                        + "Music"
                        + mParentFolderPath.subSequence(mParentFolderPath.lastIndexOf(File.separator),
                                mParentFolderPath.length());
                // mExistFileList = getExistFileList(mDestPath);
                result = true;
            } catch (IOException e) {
                e.printStackTrace();
            } catch (StringIndexOutOfBoundsException e) {
                e.printStackTrace();
            }
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + getCount());
        return result;
    }

    /**
     * Describe <code>getModuleType</code> method here.
     * 
     * @return an <code>int</code> value
     */
    public final int getModuleType() {
        return ModuleType.TYPE_MUSIC;
    }

    /**
     * Describe <code>getCount</code> method here.
     * 
     * @return an <code>int</code> value
     */
    public final int getCount() {
        int count = 0;
        if (mFileNameList != null) {
            count = mFileNameList.size();
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    /**
     * Describe <code>isAfterLast</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIndex >= mFileNameList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean implementComposeOneEntity() {
        boolean result = false;
        if (mDestPath == null) {
            return result;
        }
        MyLogger.logD(CLASS_TAG, "mDestPath:" + mDestPath);
        if (mFileNameList != null && mIndex < mFileNameList.size()) {
            // File file = mFileList[mIndex++];
            String musicName = mFileNameList.get(mIndex++);
            mDestFileName = mDestPath + File.separator + musicName;

            if (mImport) {
                File fileName = new File(mDestFileName);
                if (fileName.exists()) {
                    mDestFileName = rename(mDestFileName);
                }
            }
            try {
                MyLogger.logD(CLASS_TAG, "mDestFileName:" + mDestFileName);
                BackupZip.unZipFile(mZipFileName, musicName, mDestFileName);
//                Uri data = Uri.parse("file://" + mDestFileName);
//                mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
                result = true;
            } catch (IOException e) {
                if (super.mReporter != null) {
                    super.mReporter.onErr(e);
                }
                MyLogger.logD(CLASS_TAG, "unzipfile failed");
            }

            result = true;
        }
        
        if (mIndex % Constants.NUMBER_SEND_BROCAST_MUSIC_EACH == 0 || isAfterLast()) {
            Uri data = Uri.parse("file://" + mDestPath);
            mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
            MyLogger.logD(CLASS_TAG, "mIndex = " + mIndex + "sendBroadcast " + "isAfterLast() = " + isAfterLast());
        }

        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     * 
     */
    public void onStart() {
        super.onStart();
        if (mDestPath != null && super.checkedCommand()) {
            File tmpFolder = new File(mDestPath);
            deleteFolder(tmpFolder);

            if (!tmpFolder.exists()) {
                tmpFolder.mkdirs();
            }
        } else {
            mImport = true;
        }

        MyLogger.logD(CLASS_TAG, "onStart()");
    }

    /**
     * Describe <code>onEnd</code> method here.
     * 
     */
    public boolean onEnd() {
        super.onEnd();
        if (mDestPath != null) {
             Uri data = Uri.parse("file://" + mDestPath);
             mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
             MyLogger.logD(CLASS_TAG, "onEnd mIndex = " + mIndex + "sendBroadcast " + "isAfterLast() = " + isAfterLast());
        }
        return true;
    }

    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                try {
                    int count = mContext.getContentResolver().delete(Audio.Media.EXTERNAL_CONTENT_URI,
                            Audio.Media.DATA + " like ?", new String[] { file.getAbsolutePath() });
                    MyLogger.logD(CLASS_TAG, "deleteFolder():" + count + ":" + file.getAbsolutePath());
                    file.delete();
                } catch (Exception e) {
                    MyLogger.logD(CLASS_TAG, "deleteFolder: exception");
                }

            } else if (file.isDirectory()) {
                File files[] = file.listFiles();
                for (int i = 0; i < files.length; ++i) {
                    this.deleteFolder(files[i]);
                }
            }

            file.delete();
        }
    }

    /*
     * private ArrayList<String> getExistFileList(String path) {
     * ArrayList<String> fileList = new ArrayList<String>(); int len =
     * mFileNameList.size(); if (len > 0) { HashMap<String, Boolean> map = new
     * HashMap<String, Boolean>(); for (String fileName : mFileNameList) {
     * map.put(fileName, true); }
     * 
     * Cursor cur = mContext.getContentResolver().query(
     * Audio.Media.EXTERNAL_CONTENT_URI, mProjection, Audio.Media.DATA +
     * " like ?", new String[] { "%" + path + "%" }, null); if (cur != null) {
     * if (cur.moveToFirst()) { while (!cur.isAfterLast()) { int dataColumn =
     * cur .getColumnIndexOrThrow(Audio.Media.DATA); String data =
     * cur.getString(dataColumn); if ((data != null) && map.get(data)) {
     * fileList.add(data); MyLogger .logD(CLASS_TAG, "getExistFileList:" +
     * data); } cur.moveToNext(); } }
     * 
     * cur.close(); } }
     * 
     * return fileList; }
     */
    
    private String rename(String name) {
        String tmpName = name.subSequence(name.lastIndexOf(File.separator) + 1, name.length()).toString();
        String path = name.subSequence(0, name.lastIndexOf(File.separator) + 1).toString();
        MyLogger.logD(CLASS_TAG, " rename:tmpName:" + tmpName);
        String rename = null;
        File tmpFile = null;
        int id = tmpName.lastIndexOf(".");
        int id2;
        int leftLen;
        for (int i = 1; i < (1 << 12); ++i) {
            leftLen = 255 - (1 + Integer.toString(i).length() + tmpName.length() - id);
            id2 = id <= leftLen ? id : leftLen;
            rename = tmpName.subSequence(0, id2) + "~" + i + tmpName.subSequence(id, tmpName.length());
            tmpFile = new File(path + rename);
            String tmpFileName = tmpFile.getAbsolutePath();
            MyLogger.logD(CLASS_TAG, " rename:tmpFileName:" + tmpFileName);
            if (tmpFile.exists()) {
                continue;
            } else {
                MyLogger.logD(CLASS_TAG, " rename: rename:" + rename);
                break;
            }
        }
        return path + rename;
    }

}
