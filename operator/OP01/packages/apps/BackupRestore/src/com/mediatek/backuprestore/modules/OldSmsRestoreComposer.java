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
import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.telephony.SmsMessage;
import android.text.TextUtils;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.zip.ZipFile;

public class OldSmsRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/SmsRestoreComposer";
    private static final String SMSTAG = "SMS:";
    private static final Uri[] SMSURI = { Sms.Inbox.CONTENT_URI, Sms.Sent.CONTENT_URI, Sms.Draft.CONTENT_URI,
            Sms.Outbox.CONTENT_URI };
    private ArrayList<String> mFileNameList;
    private int mIdx;
    private long mTime;
    private ZipFile mZipFile;
    private ArrayList<ContentProviderOperation> mOperationList;
    private static final int MAX_OPERATIONS_PER_BATCH = 20;
    private HashMap<String, SmsXmlInfo> mRecord;

    public OldSmsRestoreComposer(Context context) {
        super(context);
    }

    @Override
    public int getModuleType() {
        return ModuleType.TYPE_SMS;
    }

    @Override
    public int getCount() {
        int count = 0;
        if (mFileNameList != null) {
            count = mFileNameList.size();
        }

        MyLogger.logD(CLASS_TAG, SMSTAG + "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;

        MyLogger.logD(CLASS_TAG, SMSTAG + "begin init:" + System.currentTimeMillis());
        mFileNameList = new ArrayList<String>();
        mOperationList = new ArrayList<ContentProviderOperation>();
        try {
            mTime = System.currentTimeMillis();
            mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true, "sms/sms[0-9]+");
            mZipFile = BackupZip.getZipFileFromFileName(mZipFileName);
            if (mFileNameList.size() > 0) {
                String content = BackupZip.readFile(mZipFile, "sms/msg_box.xml");
                mRecord = new HashMap<String, SmsXmlInfo>();
                if (content != null) {
                    ArrayList<SmsXmlInfo> recordList = SmsXmlParser.parse(content.toString());
                    for (SmsXmlInfo record : recordList) {
                        mRecord.put(record.getID(), record);
                    }
                } else {
                }
            }
            result = true;
        } catch (IOException e) {
        }

        MyLogger.logD(CLASS_TAG, SMSTAG + "init():" + result + ",count:" + mFileNameList.size());
        return result;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIdx >= mFileNameList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, SMSTAG + "isAfterLast():" + result);
        return result;
    }

    @Override
    public boolean implementComposeOneEntity() {
        MyLogger.logD(CLASS_TAG, SMSTAG + "mZipFileName:" + mZipFileName + ", mIdx:" + mIdx);

        boolean result = false;
        String pduFileName = mFileNameList.get(mIdx++);
        byte[] storePdu = BackupZip.readFileContent(mZipFile, pduFileName);
        if (storePdu != null) {
            SmsXmlInfo record = mRecord.get(pduFileName.subSequence(pduFileName.lastIndexOf(File.separator) + 1,
                    pduFileName.length()));
            if (record == null) {
                record = new SmsXmlInfo();
            }

            ContentValues values = parsePdu(storePdu, record.getMsgBox());
            if (values == null) {
                MyLogger.logD(CLASS_TAG, SMSTAG + "parsePdu():values=null");
            } else {

                if (record != null) {
                    values.put(Sms.READ, record.getIsRead());
                    values.put(Sms.SEEN, record.getSeen());
                    values.put(Sms.SIM_ID, record.getSimID());
                    values.put(Sms.DATE, record.getDate());
                    values.put(Sms.TYPE, record.getMsgBox());
                }

                // values.put("import_sms", true);
                // if (isAfterLast()) {
                //     values.remove("import_sms");
                // }

                ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(SMSURI[Integer
                        .parseInt(record.getMsgBox()) - 1]);
                builder.withValues(values);
                mOperationList.add(builder.build());
                if ((mIdx % MAX_OPERATIONS_PER_BATCH != 0) && !isAfterLast()) {
                    return true;
                }

                if (mOperationList.size() > 0) {
                    try {
                        mContext.getContentResolver().applyBatch("sms", mOperationList);
                    } catch (android.os.RemoteException e) {
                        e.printStackTrace();
                    } catch (android.content.OperationApplicationException e) {
                        e.printStackTrace();
                    } finally {
                        mOperationList.clear();
                    }
                }
                // if (mboxType <= SMSURI.length) {
                // SqliteWrapper.insert(mContext, mContext.getContentResolver(),
                // SMSURI[mboxType - 1], values);
                // }

                MyLogger.logD(CLASS_TAG, SMSTAG + "end restore:" + System.currentTimeMillis());
                result = true;
            }
        } else {
            if (super.mReporter != null) {
                super.mReporter.onErr(new IOException());
            }
        }

        return result;
    }

    private ContentValues parsePdu(byte[] pdu, String boxType) {
        if (boxType == null) {
            return null;
        }
        ContentValues values = null;
        try {
            // SmsMessage msg = SmsMessage.createFromPdu(pdu);
            // ////////
            byte[] tmppdu = Arrays.copyOfRange(pdu, 1, pdu.length);
            SmsMessage msg = SmsMessage.createFromPdu(tmppdu);
            // ////////
            if (msg != null) {
                values = extractContentValues(msg, boxType);
                values.put(Sms.BODY, msg.getDisplayMessageBody());
            } else {
                MyLogger.logD(CLASS_TAG, SMSTAG + "createFromPdu is null");
            }
        } catch (ArrayIndexOutOfBoundsException e) {
            MyLogger.logD(CLASS_TAG, SMSTAG + "out of bounds");
        } catch (Exception e) {
            MyLogger.logD(CLASS_TAG, "parsePdu: exception");
        }

        return values;
    }

    private ContentValues extractContentValues(SmsMessage sms, String boxType) {
        ContentValues values = new ContentValues();
        values.put(Sms.PROTOCOL, sms.getProtocolIdentifier());

        if (sms.getPseudoSubject().length() > 0) {
            values.put(Sms.SUBJECT, sms.getPseudoSubject());
        }

        String address;
        if (boxType.equals("1")) {
            address = sms.getDisplayOriginatingAddress();
        } else {
            address = sms.getDestinationAddress();
        }

        // String address = sms.getDestinationAddress();
        // String address = sms.getServiceCenterAddress();
        // String address = sms.getDisplayOriginatingAddress();
        if (TextUtils.isEmpty(address)) {
            address = "unknown";
        }

        values.put(Sms.ADDRESS, address);
        values.put(Sms.REPLY_PATH_PRESENT, sms.isReplyPathPresent() ? 1 : 0);
        values.put(Sms.SERVICE_CENTER, sms.getServiceCenterAddress());
        return values;
    }

    /*
     * private boolean deleteAllPhoneSms() { boolean result = false; if
     * (mContext != null) { MyLogger.logD(CLASS_TAG, SMSTAG + "begin delete:" +
     * System.currentTimeMillis()); int count =
     * mContext.getContentResolver().delete(Uri.parse("content://sms/"),
     * "type <> ?", new String[]{"1"}); count +=
     * mContext.getContentResolver().delete(Uri.parse("content://sms/"),
     * "date < ?", new String[]{Long.toString(mTime)});
     * 
     * int count2 = mContext.getContentResolver().delete(WapPush.CONTENT_URI,
     * null, null); MyLogger.logD(CLASS_TAG, SMSTAG + "deleteAllPhoneSms():" +
     * count + " sms deleted!" + count2 + "wappush deleted!"); result = true;
     * MyLogger.logD(CLASS_TAG, SMSTAG + "end delete:" +
     * System.currentTimeMillis()); }
     * 
     * return result; }
     * 
     * 
     * @Override public void onStart() { super.onStart(); deleteAllPhoneSms();
     * 
     * MyLogger.logD(CLASS_TAG, SMSTAG + "onStart()"); }
     */

    @Override
    public boolean onEnd() {
        super.onEnd();
        if (mFileNameList != null) {
            mFileNameList.clear();
        }

        if (mOperationList != null) {
            mOperationList = null;
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
        MyLogger.logD(CLASS_TAG, SMSTAG + "onEnd()");
        return true;
    }
}
