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

import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.util.ArrayList;
import java.util.List;

public class MessageRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MessageRestoreComposer";
    private List<Composer> mMessageComposers;
    private long mTime;

    public MessageRestoreComposer(Context context) {
        super(context);
        mMessageComposers = new ArrayList<Composer>();
        mMessageComposers.add(new SmsRestoreComposer(context));
        mMessageComposers.add(new MmsRestoreComposer(context));
    }

    public MessageRestoreComposer(Context context, String old) {
        super(context);
        mMessageComposers = new ArrayList<Composer>();
        if (old.equals("Mtk")) {
            mMessageComposers.add(new OldMtkSmsRestoreComposer(context));
            MyLogger.logD(CLASS_TAG, "OldMtkSmsRestoreComposer");
        } else {
            mMessageComposers.add(new OldSmsRestoreComposer(context));
            MyLogger.logD(CLASS_TAG, "OldHaixinSmsRestoreComposer");
        }
        mMessageComposers.add(new OldMmsRestoreComposer(context));
    }

    @Override
    public void setZipFileName(String fileName) {
        super.setZipFileName(fileName);
        for (Composer composer : mMessageComposers) {
            composer.setZipFileName(fileName);
        }
    }

    @Override
    public int getModuleType() {
        return ModuleType.TYPE_MESSAGE;
    }

    @Override
    public int getCount() {
        int count = 0;
        for (Composer composer : mMessageComposers) {
            if (composer != null) {
                count += composer.getCount();
            }
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = true;
        mTime = System.currentTimeMillis();
        for (Composer composer : mMessageComposers) {
            if (composer != null) {
                if (!composer.init()) {
                    result = false;
                }
            }
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + getCount());
        return result;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        for (Composer composer : mMessageComposers) {
            if (composer != null && !composer.isAfterLast()) {
                result = false;
                break;
            }
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    @Override
    public boolean implementComposeOneEntity() {
        for (Composer composer : mMessageComposers) {
            if (composer != null && !composer.isAfterLast()) {
                return composer.composeOneEntity();
            }
        }

        return false;
    }

    private boolean deleteAllMessage() {
        boolean result = false;
        int count = 0;
        if (mContext != null) {
            MyLogger.logD(CLASS_TAG, "begin delete:" + System.currentTimeMillis());
            try {
                count = mContext.getContentResolver().delete(Uri.parse(Constants.URI_MMS_SMS), "date < ?",
                        new String[] { Long.toString(mTime) });
                MyLogger.logD(CLASS_TAG, "end delete:" + System.currentTimeMillis());

                result = true;
            } catch (Exception e) {
                MyLogger.logD(CLASS_TAG, "delete Exception");
            }
        }

        MyLogger.logD(CLASS_TAG, "deleteAllMessage(),result" + result + "," + count + " deleted!");
        return result;
    }

    @Override
    public void onStart() {
        super.onStart();
        if (super.checkedCommand()) {
            deleteAllMessage();
        }
        MyLogger.logD(CLASS_TAG, "onStart()");
    }

    @Override
    public boolean onEnd() {
        super.onEnd();
        boolean result = true;
        boolean tmpresult;
        for (Composer composer : mMessageComposers) {
            tmpresult = composer.onEnd();
            if (!tmpresult) {
                result = false;
            }
        }
        Intent intent = new Intent();
        intent.setAction("com.mediatek.backuprestore.module.MessageRestoreComposer.RESTORE_END");
        MyLogger.logD(CLASS_TAG, "message restore end,sendBroadcast to updata UI ");
        mContext.sendBroadcast(intent);

        MyLogger.logD(CLASS_TAG, "onEnd()");
        return result;
    }

    /**
     * Describe <code>setParentFolderPath</code> method here.
     * 
     * @param string
     *            a <code>String</code> value
     */
    public final void setParentFolderPath(final String path) {
        mParentFolderPath = path;
        for (Composer composer : mMessageComposers) {
            composer.setParentFolderPath(path);
        }
    }

}
