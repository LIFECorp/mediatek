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

import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.telephony.SmsMessage;
import android.text.TextUtils;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.io.IOException;
import java.util.ArrayList;
import java.util.zip.ZipFile;

public class OldMtkSmsRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/OldMtkSmsRestoreComposer";
    private static final String SMSTAG = "SMS:";
    private static final Uri[] SMSURI = { Sms.Inbox.CONTENT_URI, Sms.Sent.CONTENT_URI, Sms.Draft.CONTENT_URI,
            Sms.Outbox.CONTENT_URI };
    // private final int timeLength = 13;
    private static final int READLENTH = 1;
    private static final int SEENLENTH = 1;
    private static final int BOXLENGTH = 1;
    private static final int SIMCARDLENGTH = 1;
    private int mBoxType;
    private ArrayList<String> mFileNameList;
    private int mIdx;
    private long mTime;
    private ZipFile mZipFile;
    private ArrayList<ContentProviderOperation> mOperationList;
    private static final int MAX_OPERATIONS_PER_BATCH = 20;

    public OldMtkSmsRestoreComposer(Context context) {
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
            result = true;
        } catch (IOException e) {
            e.printStackTrace();
        }

        MyLogger.logD(CLASS_TAG, SMSTAG + "end init:" + System.currentTimeMillis());
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
        MyLogger.logD(CLASS_TAG, SMSTAG + "begin readFileContent:" + System.currentTimeMillis());

        MyLogger.logD(CLASS_TAG, SMSTAG + "mZipFileName:" + mZipFileName + ", mIdx:" + mIdx);

        boolean result = false;
        String pduFileName = mFileNameList.get(mIdx++);
        byte[] storePdu = BackupZip.readFileContent(mZipFile, pduFileName);
        MyLogger.logD(CLASS_TAG, SMSTAG + "end readFileContent:" + System.currentTimeMillis());
        if (storePdu != null) {
            MyLogger.logD(CLASS_TAG, SMSTAG + "begin parse:" + System.currentTimeMillis());
            ContentValues values = parsePdu(storePdu);
            MyLogger.logD(CLASS_TAG, SMSTAG + "end parse:" + System.currentTimeMillis());
            if (values == null) {
                MyLogger.logD(CLASS_TAG, SMSTAG + "parsePdu():values=null");
            } else {
                MyLogger.logD(CLASS_TAG, SMSTAG + "mBoxType:" + mBoxType);
                MyLogger.logD(CLASS_TAG, SMSTAG + "begin restore:" + System.currentTimeMillis());
                if (isAfterLast()) {
                    values.remove("import_sms");
                }

                ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(SMSURI[mBoxType - 1]);
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
                    } catch (Exception e) {
                        MyLogger.logD(CLASS_TAG, "Exception e");
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

    private ContentValues parsePdu(byte[] pdu) {
        // if (pdu == null) {
        // return null;
        // }

        try {
            int curIndex = 0;
            int timeLength = pdu[curIndex++] & 0xff;
            String timeStamp = new String(pdu, curIndex, timeLength);
            curIndex += timeLength;
            String readStamp = new String(pdu, curIndex, READLENTH);
            curIndex += READLENTH;
            String seenStamp = new String(pdu, curIndex, SEENLENTH);
            curIndex += SEENLENTH;
            String boxStamp = new String(pdu, curIndex, BOXLENGTH);
            mBoxType = pdu[curIndex] - 0x30;
            if (mBoxType > 4) {
                return null;
            }
            curIndex += BOXLENGTH;
            String simcardStamp = new String(pdu, curIndex, SIMCARDLENGTH);
            curIndex += SIMCARDLENGTH;
            simcardStamp = simcardStamp.equals("-") ? "-1" : simcardStamp;
            MyLogger.logD(CLASS_TAG, SMSTAG + "parsePdu timeStamp:" + timeStamp + "timeLength:" + timeLength
                    + ",readStamp:" + readStamp + ",boxStamp:" + boxStamp + ",seenStamp:" + seenStamp
                    + ",simcardStamp:" + simcardStamp);
            int num = pdu[curIndex++] & 0xff;
            byte[] pduLens = new byte[num];
            System.arraycopy(pdu, curIndex, pduLens, 0, num);
            curIndex += num;

            StringBuilder bodyBuilder = new StringBuilder();
            ContentValues values = null;
            for (int i = 0; i < num; ++i) {
                int tmpLen = pduLens[i] & 0xff;
                byte[] rawPdu = new byte[tmpLen];
                System.arraycopy(pdu, curIndex, rawPdu, 0, tmpLen);
                curIndex += tmpLen;
                MyLogger.logD(CLASS_TAG, SMSTAG + "begin createFromPdu:" + System.currentTimeMillis());
                SmsMessage msg = SmsMessage.createFromPdu(rawPdu);
                MyLogger.logD(CLASS_TAG, SMSTAG + "end createFromPdu:" + System.currentTimeMillis());
                if (msg != null) {
                    bodyBuilder.append(msg.getDisplayMessageBody());
                    if (i == 0) {
                        MyLogger.logD(CLASS_TAG, SMSTAG + "begin extractContentValues:" + System.currentTimeMillis());
                        values = extractContentValues(msg);
                        MyLogger.logD(CLASS_TAG, SMSTAG + "end extractContentValues:" + System.currentTimeMillis());
                    }
                } else {
                    MyLogger.logD(CLASS_TAG, SMSTAG + "createFromPdu is null");
                }
            }

            if (values != null) {
                values.put(Sms.BODY, bodyBuilder.toString());
                values.put(Sms.READ, readStamp);
                values.put(Sms.SEEN, seenStamp);
                values.put(Sms.SIM_ID, simcardStamp);
                values.put(Sms.DATE, timeStamp);
                values.put(Sms.TYPE, boxStamp);
                values.put("import_sms", true);
            }

            return values;
        } catch (ArrayIndexOutOfBoundsException e) {
            MyLogger.logD(CLASS_TAG, SMSTAG + "out of bounds");
        } catch (Exception e) {
            e.printStackTrace();
            MyLogger.logD(CLASS_TAG, SMSTAG + "parse exception");
        }

        return null;
    }

    private ContentValues extractContentValues(SmsMessage sms) {
        ContentValues values = new ContentValues();
        values.put(Sms.PROTOCOL, sms.getProtocolIdentifier());

        if (sms.getPseudoSubject().length() > 0) {
            values.put(Sms.SUBJECT, sms.getPseudoSubject());
        }

        String address = sms.getDestinationAddress();
        // Log.d(LogTag.RESTORE, SMSTAG +
        // "getOriginatingAddress():" + sms.getOriginatingAddress() +
        // ",getDestinationAddress():" + sms.getDestinationAddress() +
        // ",getServiceCenterAddress:" + sms.getServiceCenterAddress());
        if (TextUtils.isEmpty(address)) {
            address = "unknown";
        }
        // Log.d(LogTag.RESTORE, SMSTAG + "getNewContentValues address:" +
        // address);
        values.put(Sms.ADDRESS, address);
        values.put(Sms.REPLY_PATH_PRESENT, sms.isReplyPathPresent() ? 1 : 0);
        values.put(Sms.SERVICE_CENTER, sms.getServiceCenterAddress());
        // values.put(Sms.BODY, sms.getDisplayMessageBody());

        // Long threadId = values.getAsLong(Sms.THREAD_ID);
        // if (threadId == null || threadId == 0) {
        // try {
        // Log.d(LogTag.RESTORE, SMSTAG + "begin extractContentValues5:" +
        // System.currentTimeMillis());
        // threadId = Threads.getOrCreateThreadId(mContext, address);
        // Log.d(LogTag.RESTORE, SMSTAG + "end   extractContentValues5:" +
        // System.currentTimeMillis());
        // } catch (IllegalArgumentException iae) {
        // Log.e(LogTag.RESTORE, SMSTAG +
        // "getOrCreateThreadId failed for this time");
        // }
        // }
        // Log.d(LogTag.RESTORE, SMSTAG + "threadId:" + threadId);
        // values.put(Sms.THREAD_ID, threadId);

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
     */

    /*
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
