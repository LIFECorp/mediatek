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

import android.accounts.Account;
import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.sqlite.SQLiteFullException;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;

import com.android.vcard.VCardConfig;
import com.android.vcard.VCardEntry;
import com.android.vcard.VCardEntryConstructor;
import com.android.vcard.VCardEntryCounter;
import com.android.vcard.VCardEntryHandler;
import com.android.vcard.VCardInterpreter;
import com.android.vcard.VCardParser;
import com.android.vcard.VCardParser_V21;
import com.android.vcard.VCardParser_V30;
import com.android.vcard.VCardSourceDetector;
import com.android.vcard.exception.VCardException;
import com.android.vcard.exception.VCardNestedException;
import com.android.vcard.exception.VCardNotSupportedException;
import com.android.vcard.exception.VCardVersionException;

import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.zip.ZipFile;

public class OldContactRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/OldContactRestoreComposer";
    public static final String ACTION_SHOULD_FINISHED = "com.android.contacts.ImportExportBridge.ACTION_SHOULD_FINISHED";
    private ArrayList<String> mFileNameList;
    private int mIdx;
    private ZipFile mZipFile;
    private static final int NUM = 1500;
    private ByteArrayOutputStream mTotalContent = new ByteArrayOutputStream();

    public OldContactRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_CONTACT;
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
        MyLogger.logD(CLASS_TAG, "begin init:" + System.currentTimeMillis());
        try {
            mFileNameList = (ArrayList<String>) BackupZip.getFileList(mZipFileName, true, true,
                    "contacts/contact[Ss]?[0-9]+\\.vcf");
            mZipFile = BackupZip.getZipFileFromFileName(mZipFileName);
            result = true;
        } catch (Exception e) {
            e.printStackTrace();
        }

        MyLogger.logD(CLASS_TAG, "end init:" + System.currentTimeMillis());
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

    public boolean composeOneEntity() {
        return implementComposeOneEntity();
    }

    public boolean implementComposeOneEntity() {
        MyLogger.logD(CLASS_TAG, "begin readFileContent:" + System.currentTimeMillis());
        byte[] content = null;
        boolean result = false;
        String contactFileName = mFileNameList.get(mIdx++);
        if (contactFileName != null && mZipFile != null) {
            content = BackupZip.readFileContent(mZipFile, contactFileName);
            MyLogger.logD(CLASS_TAG, "end readFileContent:" + System.currentTimeMillis());
        }

        if (content != null) {
            try {
                mTotalContent.write(content);
            } catch (IOException e) {
                e.printStackTrace();

            }

            if ((mIdx % NUM != 0) && !isAfterLast()) {
                return true;
            }

            MyLogger.logD(CLASS_TAG, "begin restore:" + System.currentTimeMillis());

            // Account account = new Account("Phone",
            // AccountType.ACCOUNT_TYPE_LOCAL_PHONE);
            Account account = new Account("Phone", "Local Phone Account");

            InputStream is = new ByteArrayInputStream(content);
            VCardEntryCounter counter = new VCardEntryCounter();
            VCardSourceDetector detector = new VCardSourceDetector();
            VCardParser_V21 vcardParser = new VCardParser_V21(VCardConfig.VCARD_TYPE_V21_GENERIC);
            try {
                vcardParser.addInterpreter(counter);
                vcardParser.addInterpreter(detector);
                vcardParser.parse(is);
            } catch (IOException e) {
                MyLogger.logD(CLASS_TAG, "IOException");
            } catch (VCardVersionException e) {
                MyLogger.logD(CLASS_TAG, "VCardVersionException");
            } catch (VCardException e) {
                MyLogger.logD(CLASS_TAG, "VCardException");
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }

            final String estimatedCharset = detector.getEstimatedCharset();
            final VCardEntryConstructor constructor = new VCardEntryConstructor(VCardConfig.VCARD_TYPE_V21_GENERIC,
                    account, estimatedCharset);
            final RestoreVCardEntryCommitter committer = new RestoreVCardEntryCommitter(mContext.getContentResolver());
            constructor.addEntryHandler(committer);

            final int[] possibleVCardVersions = new int[] { VCardConfig.VCARD_TYPE_V21_GENERIC,
                    VCardConfig.VCARD_TYPE_V30_GENERIC };

            is = new ByteArrayInputStream(mTotalContent.toByteArray());
            result = readOneVCard(is, VCardConfig.VCARD_TYPE_V21_GENERIC, constructor, possibleVCardVersions);

            if (!isAfterLast()) {
                mTotalContent = new ByteArrayOutputStream();
            } else {
                mTotalContent = null;
            }

            MyLogger.logD(CLASS_TAG, "end restore:" + System.currentTimeMillis());
        } else {
            if (super.mReporter != null) {
                super.mReporter.onErr(new IOException());
            }
        }

        MyLogger.logD(CLASS_TAG, "mZipFileName:" + mZipFileName + ",contactFileName:" + contactFileName + ",result:"
                + result);

        return result;
    }

    private boolean deleteAllContact() {
        if (mContext != null) {
            MyLogger.logD(CLASS_TAG, "begin delete:" + System.currentTimeMillis());
            int count = mContext.getContentResolver().delete(
                    Uri.parse(ContactsContract.RawContacts.CONTENT_URI.toString() + "?"
                            + ContactsContract.CALLER_IS_SYNCADAPTER + "=true"),
                    ContactsContract.RawContacts._ID + ">0 AND "+RawContacts.INDICATE_PHONE_SIM +"= -1", null);

            MyLogger.logD(CLASS_TAG, "end delete:" + System.currentTimeMillis());

            MyLogger.logD(CLASS_TAG, "deleteAllContact()," + count + " records deleted!");

            return true;
        }

        return false;
    }

    private boolean readOneVCard(InputStream is, int vcardType, final VCardInterpreter interpreter,
            final int[] possibleVCardVersions) {
        boolean successful = false;
        final int length = possibleVCardVersions.length;
        VCardParser vcardParser;

        for (int i = 0; i < length; i++) {
            final int vcardVersion = possibleVCardVersions[i];
            try {
                if (i > 0 && (interpreter instanceof VCardEntryConstructor)) {
                    // Let the object clean up internal temporary objects,
                    ((VCardEntryConstructor) interpreter).clear();
                }

                // We need synchronized block here,
                // since we need to handle mCanceled and mVCardParser at once.
                // In the worst case, a user may call cancel() just before
                // creating
                // mVCardParser.
                synchronized (this) {
                    vcardParser = (vcardVersion == VCardConfig.VCARD_TYPE_V21_GENERIC) ? new VCardParser_V21(vcardType)
                            : new VCardParser_V30(vcardType);
                    // if (isCancelled()) {
                    // Log.i(LOG_TAG,
                    // "ImportProcessor already recieves cancel request, so " +
                    // "send cancel request to vCard parser too.");
                    // mVCardParser.cancel();
                }

                vcardParser.parse(is, interpreter);
                successful = true;
                break;
            } catch (IOException e) {
                e.printStackTrace();
            } catch (VCardNestedException e) {
                e.printStackTrace();
            } catch (VCardNotSupportedException e) {
                e.printStackTrace();
            } catch (VCardVersionException e) {
                if (i == length - 1) {
                    e.printStackTrace();
                } else {
                    // We'll try the other (v30) version.
                }
            } catch (VCardException e) {
                e.printStackTrace();
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        MyLogger.logD(CLASS_TAG, "readOneVCard() " + successful);
        return successful;
    }

    public void onStart() {
        super.onStart();
        if (super.checkedCommand()) {
            deleteAllContact();
        }

        MyLogger.logD(CLASS_TAG, " onStart()");
    }

    @Override
    public boolean onEnd() {
        super.onEnd();
        if (mFileNameList != null) {
            mFileNameList.clear();
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
        MyLogger.logD(CLASS_TAG, " onEnd()");
        return true;
    }

    private class RestoreVCardEntryCommitter implements VCardEntryHandler {
        private final ContentResolver mContentResolver;
        // private long mTimeToCommit;
        // private int mCounter;
        private ArrayList<ContentProviderOperation> mOperationList;
        private final ArrayList<Uri> mCreatedUris = new ArrayList<Uri>();

        public RestoreVCardEntryCommitter(ContentResolver resolver) {
            mContentResolver = resolver;
        }

        public void onStart() {
        }

        public void onEnd() {
            if (mOperationList != null) {
                mCreatedUris.add(pushIntoContentResolver(mOperationList));
            }
        }

        public void onEntryCreated(final VCardEntry vcardEntry) {
            // final long start = System.currentTimeMillis();
            mOperationList = vcardEntry.constructInsertOperations(mContentResolver, mOperationList);
            // mCounter++;
            if (mOperationList != null && mOperationList.size() >= 480) {
                mCreatedUris.add(pushIntoContentResolver(mOperationList));
                // mCounter = 0;
                mOperationList = null;
            }
            // mTimeToCommit += System.currentTimeMillis() - start;
            increaseComposed(true);
        }

        private Uri pushIntoContentResolver(ArrayList<ContentProviderOperation> operationList) {
            try {
                final ContentProviderResult[] results = mContentResolver.applyBatch(ContactsContract.AUTHORITY,
                        operationList);

                // the first result is always the raw_contact. return it's uri
                // so
                // that it can be found later. do null checking for badly
                // behaving
                // ContentResolvers
                return ((results == null || results.length == 0 || results[0] == null) ? null : results[0].uri);
            } catch (RemoteException e) {
                e.printStackTrace();
                return null;
            } catch (OperationApplicationException e) {
                e.printStackTrace();
                return null;
            } catch (SQLiteFullException e) {
                MyLogger.logD(CLASS_TAG, "SQLiteFullException");
                e.printStackTrace();
                return null;
            }
        }

        /*        *//**
         * Returns the list of created Uris. This list should not be
         * modified by the caller as it is not a clone.
         */
        /*
         * public ArrayList<Uri> getCreatedUris() { return mCreatedUris; }
         */
    }

}
