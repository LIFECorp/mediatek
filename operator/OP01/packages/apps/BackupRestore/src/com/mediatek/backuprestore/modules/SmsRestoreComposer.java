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
import android.net.Uri; //import android.provider.Telephony;
import android.provider.Telephony.Sms;
import android.util.Log;

import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.ModulePath;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;

public class SmsRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/SmsRestoreComposer";
    //private static final String COLUMN_NAME_IMPORT_SMS = "import_sms";
    private static final Uri[] SMSURIARRAY = { Sms.Inbox.CONTENT_URI, Sms.Sent.CONTENT_URI
    // Sms.Draft.CONTENT_URI,
    // Sms.Outbox.CONTENT_URI
    };
    private int mIndex;
    private long mTime;
    private ArrayList<ContentProviderOperation> mOperationList;
    private ArrayList<SmsRestoreEntry> mVmessageList;

    public SmsRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_SMS;
    }

    public int getCount() {
        int count = 0;
        if (mVmessageList != null) {
            count = mVmessageList.size();
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;

        MyLogger.logD(CLASS_TAG, "begin init:" + System.currentTimeMillis());
        mOperationList = new ArrayList<ContentProviderOperation>();
        try {
            mTime = System.currentTimeMillis();
            mVmessageList = getSmsRestoreEntry();
            result = true;
        } catch (Exception e) {
            MyLogger.logD(CLASS_TAG, "init failed");
        }

        MyLogger.logD(CLASS_TAG, "end init:" + System.currentTimeMillis());
        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + mVmessageList.size());
        return result;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        if (mVmessageList != null) {
            result = (mIndex >= mVmessageList.size()) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    @Override
    public boolean implementComposeOneEntity() {
        boolean result = false;
        SmsRestoreEntry vMsgFileEntry = mVmessageList.get(mIndex++);

        if (vMsgFileEntry != null) {
            ContentValues values = parseVmessage(vMsgFileEntry);
            if (values == null) {
                MyLogger.logD(CLASS_TAG, "parseVmessage():values=null");
            } else {
                MyLogger.logD(CLASS_TAG, "begin restore:" + System.currentTimeMillis());
                int mboxType = vMsgFileEntry.getBoxType().equals("INBOX") ? 1 : 2;
                MyLogger.logD(CLASS_TAG, "mboxType:" + mboxType);
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(SMSURIARRAY[mboxType - 1]);
                builder.withValues(values);
                mOperationList.add(builder.build());
                if ((mIndex % Constants.NUMBER_IMPORT_SMS_EACH != 0) && !isAfterLast()) {
                    return true;
                }

                // if (isAfterLast()) {
                // values.remove(COLUMN_NAME_IMPORT_SMS);
                //  }

                if (mOperationList.size() > 0) {
                    try {
                        mContext.getContentResolver().applyBatch("sms", mOperationList);
                    } catch (android.os.RemoteException e) {
                        e.printStackTrace();
                    } catch (android.content.OperationApplicationException e) {
                        e.printStackTrace();
                    } catch (Exception e) {
                        e.printStackTrace();
                    } finally {
                        mOperationList.clear();
                    }
                }

                MyLogger.logD(CLASS_TAG, "end restore:" + System.currentTimeMillis());
                result = true;
            }
        } else {
            if (super.mReporter != null) {
                super.mReporter.onErr(new IOException());
            }
        }

        return result;
    }

    private ContentValues parseVmessage(SmsRestoreEntry pdu) {

        ContentValues values = new ContentValues();

        values.put(Sms.ADDRESS, pdu.getSmsAddress());
        String decodeBody = pdu.getBody();
        // values.put(Sms.SUBJECT, null);
        // String decodeBody = pdu.getBody().replaceAll(REPLACED_STR,
        // ESCAPE_STR);
        // MyLogger.logD(CLASS_TAG, "decodeBody1 =" + decodeBody);
        boolean mInd = decodeBody.endsWith(ESCAPE_STR);
        MyLogger.logD(CLASS_TAG, "parseVmessage mInd = " + mInd);
        if (mInd) {
            decodeBody = decodeBody.substring(0, decodeBody.lastIndexOf(ESCAPE_STR));
            // MyLogger.logD(CLASS_TAG, "decodeBody = " + decodeBody);
        }
        decodeBody = decodeQuotedPrintable(decodeBody.getBytes());
//        MyLogger.logD(CLASS_TAG, "decodeBody2 = " + decodeBody);
        if (decodeBody == null) {
            return null;
        }
        int m = decodeBody.indexOf("END:VBODY");
        if (m > 0) {
            StringBuffer tempssb = new StringBuffer(decodeBody);
            do {
                if (m > 0) {
                    tempssb.deleteCharAt(m - 1);
                } else {
                    break;
                }
            } while ((m = tempssb.indexOf("END:VBODY", m + "END:VBODY".length())) > 0);
            decodeBody = tempssb.toString();
        }
        values.put(Sms.BODY, decodeBody);
//        MyLogger.logD(CLASS_TAG, "readorunread :" + pdu.getReadByte());

        values.put(Sms.READ, (pdu.getReadByte().equals("UNREAD") ? 0 : 1));
        values.put(Sms.SEEN, pdu.getSeen());
        values.put(Sms.LOCKED, (pdu.getLocked().equals("LOCKED") ? "1" : "0"));
        String simCardid = "-1";
        SimInfoManager.SimInfoRecord si = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            int soltid = Integer.parseInt(pdu.getSimCardid()) - 1;
            if (soltid < 0) {
                soltid = 0;
            }
            si = SimInfoManager.getSimInfoBySlot(mContext, soltid);
            MyLogger.logD(CLASS_TAG, "gemini,si = " + si);
        } else {
            si = SimInfoManager.getSimInfoBySlot(mContext, 0);
        }
        if (si == null) {
            simCardid = "0";
        } else {
            simCardid = String.valueOf(si.mSimInfoId);
        }
        values.put(Sms.SIM_ID, simCardid);
        values.put(Sms.DATE, pdu.getTimeStamp());
        values.put(Sms.TYPE, (pdu.getBoxType().equals("INBOX") ? 1 : 2));
        //values.put(COLUMN_NAME_IMPORT_SMS, true);

        return values;
    }

    // private ContentValues extractContentValues(SmsMessage sms) {
    // ContentValues values = new ContentValues();
    // values.put(Sms.PROTOCOL, sms.getProtocolIdentifier());

    // if (sms.getPseudoSubject().length() > 0) {
    // values.put(Sms.SUBJECT, sms.getPseudoSubject());
    // }

    // String address = sms.getDestinationAddress();
    // if (TextUtils.isEmpty(address)) {
    // address = "unknown";
    // }
    // values.put(Sms.ADDRESS, address);
    // values.put(Sms.REPLY_PATH_PRESENT, sms.isReplyPathPresent() ? 1 : 0);
    // values.put(Sms.SERVICE_CENTER, sms.getServiceCenterAddress());

    // return values;
    // }

    /*
     * private boolean deleteAllPhoneSms() { boolean result = false; if
     * (mContext != null) { MyLogger.logD(CLASS_TAG, "begin delete:" +
     * System.currentTimeMillis()); int count =
     * mContext.getContentResolver().delete(Uri.parse(Constants.URI_SMS),
     * "type <> ?", new String[] { Constants.MESSAGE_BOX_TYPE_INBOX }); count +=
     * mContext.getContentResolver().delete(Uri.parse(Constants.URI_SMS),
     * "date < ?", new String[] { Long.toString(mTime) });
     * 
     * int count2 = mContext.getContentResolver().delete(WapPush.CONTENT_URI,
     * null, null); MyLogger.logD(CLASS_TAG, "deleteAllPhoneSms():" + count +
     * " sms deleted!" + count2 + "wappush deleted!"); result = true;
     * MyLogger.logD(CLASS_TAG, "end delete:" + System.currentTimeMillis()); }
     * 
     * return result; }
     */

    private String decodeQuotedPrintable(byte[] bytes) {
        if (bytes == null) {
            return null;
        }
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        int length = bytes.length;
        for (int j = length - 1; j > 0; j--) {
            if (bytes[j] == ESCAPE_CHAR || '\r' == (char) bytes[j] || '\n' == (char) bytes[j]) {
            // MyLogger.logD(CLASS_TAG, "decodeQuotedPrintable:end with'\r','\n'or'=' ");
            } else {
                length = j + 1;
                break;
            }
        }
        for (int i = 0; i < length; i++) {
            int b = bytes[i];
            if (b == ESCAPE_CHAR && i + 2 < length) {
                try {
                    if (i + 2 < length && '\r' == (char) bytes[i + 1] && '\n' == (char) bytes[i + 2]) {
                        i += 2;
                        continue;
                    }
                    if (i + 1 < length && ('\r' == (char) bytes[i + 1])) {
                        i += 1;
                        continue;
                    }
                    if (i + 1 < length && ('\n' == (char) bytes[i + 1])) {
                        i += 1;
                        continue;
                    }
                    int u = Character.digit((char) bytes[++i], 16);
                    int l = Character.digit((char) bytes[++i], 16);
                    if (u == -1 || l == -1) {
                        return null;
                    }
                    buffer.write((char) ((u << 4) + l));

                } catch (ArrayIndexOutOfBoundsException e) {
                    String ssssBody = new String(buffer.toByteArray());
                 //   MyLogger.logD(CLASS_TAG, "ArrayIndexOutOfBoundsException ssssBody =" + ssssBody);
                    return null;
                }
            } else {
                buffer.write(b);
            }
        }
        String sBody = new String(buffer.toByteArray());
        return sBody;
    }

    /*
     * @Override public void onStart() { super.onStart(); deleteAllPhoneSms();
     * 
     * MyLogger.logD(CLASS_TAG, "onStart()"); }
     */

    @Override
    public boolean onEnd() {
        super.onEnd();
        if (mVmessageList != null) {
            mVmessageList.clear();
        }

        if (mOperationList != null) {
            mOperationList = null;
        }

        MyLogger.logD(CLASS_TAG, "onEnd()");
        MyLogger.logD(CLASS_TAG, "smsRestore end:" + System.currentTimeMillis());
        return true;
    }

    private static final String VMESSAGE_END_OF_LINE = "\r\n";
    private static final String BEGIN_VMSG = "BEGIN:VMSG";
    private static final String END_VMSG = "END:VMSG";
    private static final String VERSION = "VERSION:";
    private static final String BEGIN_VCARD = "BEGIN:VCARD";
    private static final String END_VCARD = "END:VCARD";
    private static final String BEGIN_VBODY = "BEGIN:VBODY";
    private static final String END_VBODY = "END:VBODY";
    private static final String FROMTEL = "TEL:";
    private static final String XBOX = "X-BOX:";
    private static final String XREAD = "X-READ:";
    private static final String XSEEN = "X-SEEN:";
    private static final String XSIMID = "X-SIMID:";
    private static final String XLOCKED = "X-LOCKED:";
    private static final String XTYPE = "X-TYPE:";
    private static final String DATE = "Date:";
    private static final String SUBJECT = "Subject";

    private static final String ESCAPE_STR = "=";
    private static final String REPLACED_STR = "==";
    private static final byte ESCAPE_CHAR = '=';

    private static final String VMESSAGE_END_OF_COLON = ":";

    class SmsRestoreEntry {
        private String mTimeStamp;

        private String mReadByte;

        private String mSeen;

        private String mBoxType;

        private String mSimCardid;

        private String mLocked;

        private String mSmsAddress;

        private String mBody;

        public String getTimeStamp() {
            return mTimeStamp;
        }

        public void setTimeStamp(String timeStamp) {
            this.mTimeStamp = timeStamp;
        }

        public String getReadByte() {
            return mReadByte == null ? "READ" : mReadByte;
        }

        public void setReadByte(String readByte) {
            this.mReadByte = readByte;
        }

        public String getSeen() {
            return mSeen == null ? "1" : mSeen;
        }

        public void setSeen(String seen) {
            this.mSeen = seen;
        }

        public String getBoxType() {
            return mBoxType;
        }

        public void setBoxType(String boxType) {
            this.mBoxType = boxType;
        }

        public String getSimCardid() {
            return mSimCardid;
        }

        public void setSimCardid(String simCardid) {
            this.mSimCardid = simCardid;
        }

        public String getLocked() {
            return mLocked;

        }

        public void setLocked(String locked) {
            this.mLocked = locked;
        }

        public String getSmsAddress() {
            return mSmsAddress;
        }

        public void setSmsAddress(String smsAddress) {
            this.mSmsAddress = smsAddress;
        }

        public String getBody() {
            return mBody;
        }

        public void setBody(String body) {
            this.mBody = body;
        }
    }

    public ArrayList<SmsRestoreEntry> getSmsRestoreEntry() {
        ArrayList<SmsRestoreEntry> smsEntryList = new ArrayList<SmsRestoreEntry>();
        SimpleDateFormat sd = new SimpleDateFormat("yyyy/MM/dd kk:mm:ss");
        try {
            File file = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_SMS + File.separator
                    + ModulePath.SMS_VMSG);
            InputStream instream = new FileInputStream(file);
            InputStreamReader inreader = new InputStreamReader(instream);
            BufferedReader buffreader = new BufferedReader(inreader);
            String line = null;
            StringBuffer tmpbody = new StringBuffer();
            boolean appendbody = false;
            SmsRestoreEntry smsentry = null;
            while ((line = buffreader.readLine()) != null) {
                if (isCancel()) {
                    smsEntryList.clear();
                    break;
                }

                if (line.startsWith(BEGIN_VMSG) && !appendbody) {
                    smsentry = new SmsRestoreEntry();
                    // Log.d(CLASS_TAG, "startsWith(BEGIN_VMSG)");
                }
                if (line.startsWith(FROMTEL) && !appendbody) {
                    smsentry.setSmsAddress(line.substring(FROMTEL.length()));
                    // Log.d(CLASS_TAG, "startsWith(TEL)");
                }
                if (line.startsWith(XBOX) && !appendbody) {
                    smsentry.setBoxType(line.substring(XBOX.length()));
                    // Log.d(CLASS_TAG, "startsWith(XBOX)");
                }
                if (line.startsWith(XREAD) && !appendbody) {
                    smsentry.setReadByte(line.substring(XREAD.length()));
                    // Log.d(CLASS_TAG, "startsWith(XREAD)");
                }
                if (line.startsWith(XSEEN) && !appendbody) {
                    smsentry.setSeen(line.substring(XSEEN.length()));
                    // Log.d(CLASS_TAG, "startsWith(XSEEN)");
                }
                if (line.startsWith(XSIMID) && !appendbody) {
                    smsentry.setSimCardid(line.substring(XSIMID.length()));
                    // Log.d(CLASS_TAG, "startsWith(XSIMID)");
                }
                if (line.startsWith(XLOCKED) && !appendbody) {
                    smsentry.setLocked(line.substring(XLOCKED.length()));
                    // Log.d(CLASS_TAG, "startsWith(XLOCKED)");
                }
                // if (line.startsWith(XTYPE)) {
                // smsentry.set(line.substring(XTYPE.length()));
                // MyLogger.logD(CLASS_TAG, "startsWith(XTYPE)
                // line.substring(XTYPE.length()) =
                // "+line.substring(XTYPE.length()));
                // }
                if (line.startsWith(DATE) && !appendbody) {
                    long result = sd.parse(line.substring(DATE.length())).getTime();
                    smsentry.setTimeStamp(String.valueOf(result));
                    // Log.d(CLASS_TAG, "startsWith(DATE)");
                }

                if (line.startsWith(SUBJECT) && !appendbody) {
                    String bodySlash = line.substring(line.indexOf(VMESSAGE_END_OF_COLON) + 1);

                    tmpbody.append(bodySlash);
                    appendbody = true;
                    // Log.d(CLASS_TAG, "startsWith(SUBJECT) bodySlash=" +
                    // bodySlash);
                    continue;
                }
                if (line.startsWith(END_VBODY)) {
                    appendbody = false;
                    smsentry.setBody(tmpbody.toString());
                    smsEntryList.add(smsentry);
                    tmpbody.setLength(0);
                    // MyLogger.logD(CLASS_TAG, "startsWith(END_VBODY)");
                    continue;
                }
                if (appendbody) {
                    if (tmpbody.toString().endsWith(ESCAPE_STR)) {
                        tmpbody.delete(tmpbody.lastIndexOf(ESCAPE_STR), tmpbody.length());
                    }
                    tmpbody.append(line);
                  // MyLogger.logD(CLASS_TAG, "appendbody=true,tmpbody=" + tmpbody.toString());
                }
            }
            instream.close();
        } catch (Exception e) {
            MyLogger.logE(CLASS_TAG, "init failed");
        }

        return smsEntryList;
    }

}
