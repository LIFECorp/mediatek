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

import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.media.MediaScannerConnection.OnScanCompletedListener;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.Constants.ModulePath;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

/**
 * Describe class <code>PictureRestoreComposer</code> here.
 * 
 * @author
 * @version 1.0
 */
public class PictureRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/PictureRestoreComposer";
    // private static final String mStoragepath =
    // "/mnt/sdcard2/.backup/Data/picture";
    private int mIndex;
    private Object mLock = new Object();
    // private File[] mFileList;
    private ArrayList<String> mFileNameList;
    // private ArrayList<String> mExistFileList = null;
    private String mDestPath;
    private String mZipFileName;
    private static final String[] PROJECTION = new String[] { MediaStore.Images.Media._ID, MediaStore.Images.Media.DATA };
    private boolean mImport;
    private String mDestFileName;
    private MediaScannerConnection  mMediaScannerConnection;
    private boolean mIsMediaScannerConnected;


    /**
     * Creates a new <code>PictureRestoreComposer</code> instance.
     * 
     * @param context
     *            a <code>Context</code> value
     */
    public PictureRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_PICTURE;
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
        String path = mParentFolderPath + File.separator + ModulePath.FOLDER_PICTURE;
        mFileNameList = new ArrayList<String>();
        File folder = new File(path);
        if (folder.exists() && folder.isDirectory()) {
            try {
                mZipFileName = path + File.separator + ModulePath.PICTUREZIP;
                mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true, ".*");
                String tmppath = (new File(mParentFolderPath)).getParent();
                mDestPath = tmppath.subSequence(0, tmppath.length() - 12)
                        + File.separator
                        + "Pictures"
                        + mParentFolderPath.subSequence(mParentFolderPath.lastIndexOf(File.separator),
                                mParentFolderPath.length());
                // mExistFileList = getExistFileList(mDestPath);
                MyLogger.logD(CLASS_TAG, "mDestPath:" + mDestPath);
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

    public boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIndex >= mFileNameList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    public boolean implementComposeOneEntity() {
        boolean result = false;
        if (mDestPath == null) {
            return result;
        }
        synchronized (mLock) {
            while (!mIsMediaScannerConnected) {
                try {
                    mLock.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        if (mFileNameList != null && mIndex < mFileNameList.size()) {
            // File file = mFileList[mIndex++];
            String picName = mFileNameList.get(mIndex++);
            mDestFileName = mDestPath + picName;
            if (mImport) {
                File fileName = new File(mDestFileName);
                if (fileName.exists()) {
                    mDestFileName = rename(mDestFileName);
                }
            }
            try {
                BackupZip.unZipFile(mZipFileName, picName, mDestFileName);
                MyLogger.logD(CLASS_TAG, " insert database mDestFileName =" + mDestFileName );
//                mMediaScannerConnection.scanFile(mDestFileName, null);
                result = true;
            } catch (IOException e) {
                if (super.mReporter != null) {
                    super.mReporter.onErr(e);
                }
                result = false;
                e.printStackTrace();
            }
        }
        
        if (mIndex % Constants.NUMBER_SEND_BROCAST_MUSIC_EACH == 0 || isAfterLast()) {
            mMediaScannerConnection.scanFile(mDestPath, null);
            MyLogger.logD(CLASS_TAG, "mIndex = " + mIndex + "scanFile " + "isAfterLast() = " + isAfterLast());
        }

        MyLogger.logD(CLASS_TAG, "implementComposeOneEntity:" + result);
        return result;
    }

    /*
     * private ArrayList<String> getExistFileList(String path) {
     * ArrayList<String> fileList = new ArrayList<String>(); Cursor cur =
     * mContext.getContentResolver().query(Images.Media.EXTERNAL_CONTENT_URI,
     * PROJECTION, MediaStore.Images.Media.DATA + "  like ?", new String[] { "%"
     * + path + "%" }, null); if (cur != null) { if (cur.moveToFirst()) { while
     * (!cur.isAfterLast()) { int dataColumn =
     * cur.getColumnIndexOrThrow(MediaStore.Images.Media.DATA); String data =
     * cur.getString(dataColumn); if (data != null) { fileList.add(data); }
     * cur.moveToNext(); } }
     * 
     * cur.close(); }
     * 
     * return fileList; }
     */
    public boolean onEnd() {
        super.onEnd();
        if (mDestPath !=null) {
           mMediaScannerConnection.scanFile(mDestPath, null);
           MyLogger.logD(CLASS_TAG, "onEnd mIndex = " + mIndex + ".scanFile " + "isAfterLast() = " + isAfterLast());
        }
        mMediaScannerConnection.disconnect();
        return true;
    }
    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                try {
                    int count = mContext.getContentResolver().delete(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                            MediaStore.Images.Media.DATA + " like ?", new String[] { file.getAbsolutePath() });
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

    // private void deleteRecord(String path) {
    // if (mFileList != null) {
    // int count = 0;
    // for (File file : mFileList) {
    // count +=
    // mContext.getContentResolver().delete(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
    // MediaStore.Images.Media.DATA + " like ?",
    // new String[] { file.getAbsolutePath() });
    // MyLogger.logD(CLASS_TAG, "deleteRecord():" + count + ":" +
    // file.getAbsolutePath());
    // }

    // //MyLogger.logD(CLASS_TAG, "deleteRecord():" + count + ":" + path);
    // }
    // }

    public void onStart() {
        super.onStart();
        mMediaScannerConnection = new MediaScannerConnection(this.mContext, new ScanCompletedListener());
        mMediaScannerConnection.connect();
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
    
    private String rename(String name) {
        String tmpName = name.subSequence(name.lastIndexOf(File.separator) + 1, name.length()).toString();
        String path = name.subSequence(0, name.lastIndexOf(File.separator) + 1).toString();
        MyLogger.logD(CLASS_TAG, " rename:tmpName  ==== " + tmpName);
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
            MyLogger.logD(CLASS_TAG, " rename:tmpFileName == " + tmpFileName);
            if (tmpFile.exists()) {
                continue;
            } else {
                MyLogger.logD(CLASS_TAG, " rename: rename === " + rename);
                break;
            }
        }
        return path + rename;
    }

    class ScanCompletedListener implements MediaScannerConnectionClient{
        @Override
        public void onScanCompleted(String path, Uri uri) {
            MyLogger.logD(CLASS_TAG, mIndex + "nScanCompleted");
            if (uri != null) {
                MyLogger.logD(CLASS_TAG, mIndex + path + "insert to db successed!");
            } else {
                MyLogger.logD(CLASS_TAG, mIndex + path + "insert to db fail");
            }
        }

        @Override
        public void onMediaScannerConnected() {
            synchronized (mLock) {
                mIsMediaScannerConnected = true;
                MyLogger.logD(CLASS_TAG, "MediaScannerConnected");
                mLock.notifyAll();
            }
        }
    }
}
