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
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

/**
 * Describe class <code>MusicRestoreComposer</code> here.
 * 
 * @author
 * @version 1.0
 */
public class OldMusicRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/OldMusicRestoreComposer";
    private static final String MUSICTAG = "Music:";
    private int mIdx;
    private ArrayList<String> mFileNameList;
    private String mDestPath = null;
    private boolean mImport;
    private String mDestFileName;

    public OldMusicRestoreComposer(Context context) {
        super(context);
    }

    /**
     * Describe <code>init</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    @Override
    public final boolean init() {
        boolean result = false;
        mFileNameList = new ArrayList<String>();
        try {
            mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true, "music/.*");
            // String path = BackupRestoreUtils.getStoragePath();
            String temp = SDCardUtils.getStoragePath();
            if (temp == null) {
                return result;
            }
            int index = temp.lastIndexOf(File.separator);
            String path = temp.substring(0, index + 1) + ".backup";
            MyLogger.logD(CLASS_TAG, MUSICTAG + "init():path=" + path);

            mDestPath = path.subSequence(0, path.lastIndexOf(File.separator))
                    + File.separator
                    + "Music"
                    + File.separator
                    + mZipFileName.subSequence(mZipFileName.lastIndexOf(File.separator) + 1,
                            mZipFileName.lastIndexOf(".")).toString();
            MyLogger.logD(CLASS_TAG, MUSICTAG + "init():mDestPath=" + mDestPath);
            result = true;
        } catch (IOException e) {
            e.printStackTrace();
        }

        MyLogger.logD(CLASS_TAG, MUSICTAG + "init():" + result + ",count:" + mFileNameList.size());
        return result;
    }

    /**
     * Describe <code>getModuleType</code> method here.
     * 
     * @return an <code>int</code> value
     */
    @Override
    public final int getModuleType() {
        return ModuleType.TYPE_MUSIC;
    }

    /**
     * Describe <code>getCount</code> method here.
     * 
     * @return an <code>int</code> value
     */
    @Override
    public final int getCount() {
        int count = 0;
        if (mFileNameList != null) {
            count = mFileNameList.size();
        }

        MyLogger.logD(CLASS_TAG, MUSICTAG + "getCount():" + count);
        return count;
    }

    /**
     * Describe <code>isAfterLast</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    @Override
    public final boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIdx >= mFileNameList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, MUSICTAG + "isAfterLast():" + result);
        return result;
    }

    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    @Override
    public final boolean implementComposeOneEntity() {
        boolean result = false;
        if (mDestPath == null) {
            return result;
        }

        String musicName = mFileNameList.get(mIdx++);
        mDestFileName = mDestPath
                + musicName.subSequence(musicName.lastIndexOf(File.separator), musicName.length()).toString();
        if (mImport) {
            File fileName = new File(mDestFileName);
            if (fileName.exists()) {
                mDestFileName = rename(mDestFileName);
            }
        }
        MyLogger.logD(CLASS_TAG, MUSICTAG + "restoreOnePicture(),zipFileName:" + mZipFileName + "\nmusicName:"
                + musicName + "\ndestFileName:" + mDestFileName);

        try {
            BackupZip.unZipFile(mZipFileName, musicName, mDestFileName);
            Uri data = Uri.parse("file://" + mDestFileName);
            mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
            result = true;
        } catch (IOException e) {
            if (super.mReporter != null) {
                super.mReporter.onErr(e);
            }
            MyLogger.logD(CLASS_TAG, MUSICTAG + "unzipfile failed");
        } catch (Exception e) {
            e.printStackTrace();
        }

        return result;
    }

    @Override
    public void onStart() {
        super.onStart();

        if (mDestPath != null && super.checkedCommand()) {
            File tmpFolder = new File(mDestPath);
            deleteFolder(tmpFolder);
        } else {
            mImport = true;
        }
    }

    @Override
    public boolean onEnd() {
        // Uri data = Uri.parse("file://" + mDestPath);
        // mContext.sendBroadcast(new
        // Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
        super.onEnd();
        return true;
    }

    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                try {
                    int count = mContext.getContentResolver().delete(Audio.Media.EXTERNAL_CONTENT_URI,
                            Audio.Media.DATA + " like ?", new String[] { file.getAbsolutePath() });
                    MyLogger.logD(CLASS_TAG, MUSICTAG + "deleteFolder():" + count + ":" + file.getAbsolutePath());
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
    
    private String rename(String name) {
        String tmpName = name.subSequence(name.lastIndexOf(File.separator) + 1, name.length()).toString();
        MyLogger.logD(CLASS_TAG, MUSICTAG + "tmpName:" + tmpName);
        String rename = null;
        File tmpFile = null;
        int id = tmpName.lastIndexOf(".");
        int id2;
        int leftLen;
        for (int i = 1; i < (1 << 12); ++i) {
            leftLen = 255 - (1 + Integer.toString(i).length() + tmpName.length() - id);
            id2 = id <= leftLen ? id : leftLen;
            rename = tmpName.subSequence(0, id2) + "~" + i + tmpName.subSequence(id, tmpName.length());
            tmpFile = new File(mDestPath + File.separator + rename);
            String tmpFileName = tmpFile.getAbsolutePath();
            MyLogger.logD(CLASS_TAG, MUSICTAG + "tmpFileName:" + tmpFileName);
            if (tmpFile.exists()) {
                continue;
            } else {
                MyLogger.logD(CLASS_TAG, MUSICTAG + "rename:" + rename);
                break;
            }
        }
        return mDestPath + File.separator + rename;
    }

}
