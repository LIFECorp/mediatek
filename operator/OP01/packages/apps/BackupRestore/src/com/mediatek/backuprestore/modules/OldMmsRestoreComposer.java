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
import android.net.Uri;
import android.provider.Telephony.Mms;

import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.PduParser;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.io.IOException;
import java.util.ArrayList;
import java.util.zip.ZipFile;

public class OldMmsRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MmsRestoreComposer";
    private static final String MMSTAG = "Mms:";
    private ArrayList<String> mFileNameList;
    private ArrayList<MmsXmlInfo> mRecordList;
    private int mIdx;
    private long mTime;
    private ZipFile mZipFile;
    private Object mLock = new Object();
    private ArrayList<MmsRestoreContent> mPduList = null;
    private ArrayList<MmsRestoreContent> mTmpPduList = null;
    private static final int MAX_NUM_PER_TIME = 5;

    public OldMmsRestoreComposer(Context context) {
        super(context);
    }

    @Override
    public int getModuleType() {
        return ModuleType.TYPE_MMS;
    }

    @Override
    public int getCount() {
        int count = 0;
        if (mFileNameList != null) {
            count = mFileNameList.size();
        }

        MyLogger.logD(CLASS_TAG, MMSTAG + "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;
        mTmpPduList = new ArrayList<MmsRestoreContent>();

        try {
            mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true, "mms/mms[0-9]+\\.pdu");
            mZipFile = BackupZip.getZipFileFromFileName(mZipFileName);
            if (mFileNameList.size() > 0) {
                String content = BackupZip.readFile(mZipFile, "mms/msg_box.xml");
                if (content != null) {
                    mRecordList = MmsXmlParser.parse(content.toString());
                } else {
                    mRecordList = new ArrayList<MmsXmlInfo>();
                }
            }
            mTime = System.currentTimeMillis();
            result = true;
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }

        MyLogger.logD(CLASS_TAG, MMSTAG + "init():" + result);
        return result;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIdx >= mFileNameList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, MMSTAG + "isAfterLast():" + result);
        return result;
    }

    public boolean composeOneEntity() {
        return implementComposeOneEntity();
    }

    @Override
    public boolean implementComposeOneEntity() {
        Uri msgUri = getMsgBoxUri(mIdx);
        MyLogger.logD(CLASS_TAG,
                MMSTAG + "mZipFileName:" + mZipFileName + ",mIdx:" + mIdx + ",msgUri:" + msgUri.toString());
        String pduFileName = mFileNameList.get(mIdx++);
        byte[] pduMid = BackupZip.readFileContent(mZipFile, pduFileName);
        MyLogger.logD(CLASS_TAG, MMSTAG + "readFileContent finish");

        MmsRestoreContent tmpContent = new MmsRestoreContent();
        tmpContent.msgUri = msgUri;
        tmpContent.mPduMid = pduMid;
        mTmpPduList.add(tmpContent);

        if (mIdx % MAX_NUM_PER_TIME == 0 || isAfterLast()) {
            if (mPduList != null) {
                synchronized (mLock) {
                    try {
                        MyLogger.logD(CLASS_TAG, MMSTAG + "wait for MmsRestoreThread");
                        mLock.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
            mPduList = mTmpPduList;
            new MmsRestoreThread().start();
            if (!isAfterLast()) {
                mTmpPduList = new ArrayList<MmsRestoreContent>();
            }
        }

        return pduMid != null ? true : false;
    }

    /*
     * private boolean deleteAllPhoneMms() { boolean result = false; if
     * (mContext != null) { int count =
     * mContext.getContentResolver().delete(Uri.parse("content://mms/"),
     * "msg_box <> ?", new String[]{"1"}); count +=
     * mContext.getContentResolver().delete(Uri.parse("content://mms/"),
     * "date < ?", new String[]{Long.toString(mTime)});
     * 
     * MyLogger.logD(CLASS_TAG, MMSTAG + "deleteAllPhoneMms():" + count +
     * " mms deleted!"); result = true; }
     * 
     * return result; }
     * 
     * @Override public void onStart() { super.onStart(); deleteAllPhoneMms();
     * 
     * MyLogger.logD(CLASS_TAG, MMSTAG + "onStart()"); }
     */

    @Override
    public boolean onEnd() {
        if (mPduList != null) {
            synchronized (mLock) {
                try {
                    MyLogger.logD(CLASS_TAG, MMSTAG + "wait for ZipThread:");
                    mLock.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        super.onEnd();
        if (mFileNameList != null) {
            // mFileNameList.clear();
            mFileNameList = null;
        }
        try {
            if (mZipFile != null) {
                mZipFile.close();
                mZipFile = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        // mZipFile = null;
        MyLogger.logD(CLASS_TAG, MMSTAG + "onEnd()");
        return true;
    }

    private Uri getMsgBoxUri(int id) {
        if (id < mFileNameList.size()) {
            String name = mFileNameList.get(id);
            for (MmsXmlInfo record : mRecordList) {
                if (name.equals("mms/" + record.getID())) {
                    String msgBox = record.getMsgBox();
                    if (msgBox.equals("1")) {
                        return Mms.Inbox.CONTENT_URI;
                    } else if (msgBox.equals("2")) {
                        return Mms.Sent.CONTENT_URI;
                    } else if (msgBox.equals("3")) {
                        return Mms.Draft.CONTENT_URI;
                    } else if (msgBox.equals("4")) {
                        return Mms.Outbox.CONTENT_URI;
                    }
                }
            }
        }

        return Mms.Inbox.CONTENT_URI;
    }

    private class MmsRestoreThread extends Thread {
        @Override
        public void run() {
            for (int j = 0; (mPduList != null) && (j < mPduList.size()); ++j) {
                byte[] pduMid = mPduList.get(j).mPduMid;
                Uri msgUri = mPduList.get(j).msgUri;
                if (pduMid != null) {
                    PduPersister persister = PduPersister.getPduPersister(mContext);
                    RetrieveConf retrieveConf = null;
                    NotificationInd indConf = null;
                    SendReq sendConf = null;
                    MyLogger.logD(CLASS_TAG, MMSTAG + "MmsRestoreThread parse begin");
                    if (msgUri == Mms.Inbox.CONTENT_URI) {
                        try {
                            retrieveConf = (RetrieveConf) new PduParser(pduMid).parse(true);
                        } catch (Exception e) {
                            e.printStackTrace();
                        } finally {
                            if (retrieveConf == null) {
                                indConf = (NotificationInd) new PduParser(pduMid).parse(true);
                            }
                        }
                    } else {
                        sendConf = (SendReq) new PduParser(pduMid).parse(true);
                    }
                    MyLogger.logD(CLASS_TAG, MMSTAG + "MmsRestoreThread parse finish");

                    try {
                        if (msgUri == Mms.Inbox.CONTENT_URI) {
                            persister.persist(retrieveConf != null ? retrieveConf : indConf, msgUri, true);
                        } else {
                            persister.persist(sendConf, msgUri);
                        }
                        MyLogger.logD(CLASS_TAG, MMSTAG + "MmsRestoreThread persist finish");
                        increaseComposed(true);
                    } catch (MmsException e) {
                        MyLogger.logE(CLASS_TAG, MMSTAG + "MmsRestoreThread MmsException");
                        e.printStackTrace();
                    } catch (Exception e) {
                        MyLogger.logE(CLASS_TAG, MMSTAG + "MmsRestoreThread Exception");
                        e.printStackTrace();
                    }
                } else {
                    if (mReporter != null) {
                        mReporter.onErr(new IOException());
                    }
                }
            }

            synchronized (mLock) {
                mPduList = null;
                mLock.notifyAll();
            }
        }
    }

    private class MmsRestoreContent {
        public Uri msgUri;
        byte[] mPduMid;
    }
}
