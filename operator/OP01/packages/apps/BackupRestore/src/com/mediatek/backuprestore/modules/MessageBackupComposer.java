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

//import com.mediatek.backuprestore.utils.BackupZip;
import com.mediatek.backuprestore.ProgressReporter;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.util.ArrayList;
import java.util.List;

public class MessageBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MessageBackupComposer";
    private List<Composer> mMessageComposers;

    public MessageBackupComposer(Context context) {
        super(context);
        mMessageComposers = new ArrayList<Composer>();
        mMessageComposers.add(new SmsBackupComposer(context));
        mMessageComposers.add(new MmsBackupComposer(context));
    }

    // @Override
    // public void setZipHandler(BackupZip handler) {
    // super.setZipHandler(handler);
    // for (Composer composer : mMessageComposers) {
    // composer.setZipHandler(handler);
    // }
    // }

    public int getModuleType() {
        return ModuleType.TYPE_MESSAGE;
    }

    public void setReporter(ProgressReporter reporter) {
        super.setReporter(reporter);
        if (reporter != null) {
            for (Composer composer : mMessageComposers) {
                composer.setReporter(reporter);
            }
        }
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

    @Override
    public boolean init() {
        boolean result = true;
        for (Composer composer : mMessageComposers) {
            if (composer != null) {
                if (!composer.init()) {
                    result = false;
                }
            }
        }

        MyLogger.logD(CLASS_TAG, "init():" + result);
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
        boolean result = false;
        for (Composer composer : mMessageComposers) {
            if (composer != null && !composer.isAfterLast()) {
                return composer.composeOneEntity();
            }
        }

        return result;
    }

    public int getComposed(int type) {
        int count = 0;
        for (Composer composer : mMessageComposers) {
            if (composer != null && composer.getModuleType() == type) {
                count = composer.getComposed();
                break;
            }
        }

        return count;
    }

    @Override
    public void onStart() {
        super.onStart();
        for (Composer composer : mMessageComposers) {
            if (composer != null) {
                composer.onStart();
            }
        }
    }

    @Override
    public boolean onEnd() {
        super.onEnd();
        boolean result = true;
        boolean tmpresult;
        for (Composer composer : mMessageComposers) {
            if (composer != null) {
                tmpresult = composer.onEnd();
                if (!tmpresult) {
                    result = false;
                }
            }
        }
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
