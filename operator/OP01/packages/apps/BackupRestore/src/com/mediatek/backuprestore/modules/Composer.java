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

import com.mediatek.backuprestore.ProgressReporter;
import com.mediatek.backuprestore.utils.Constants;

import java.util.List;

public abstract class Composer {
    protected Context mContext;
    protected ProgressReporter mReporter;
    protected boolean mIsCancel = false;
    protected int mComposeredCount = 0;
    protected String mParentFolderPath;
    protected String mZipFileName;
    protected List<String> mParams;
    protected boolean mNeedAppDate;
    private int mCommand;

    public Composer(Context context) {
        mContext = context;
    }

    public void setParentFolderPath(String path) {
        mParentFolderPath = path;
    }

    public void setZipFileName(String fileName) {
        mZipFileName = fileName;
    }

    public void setReporter(ProgressReporter reporter) {
        mReporter = reporter;
    }

    public synchronized void setCancel(boolean cancel) {
        mIsCancel = cancel;
    }

    public synchronized boolean isCancel() {
        return mIsCancel;
    }

    public int getComposed() {
        return mComposeredCount;
    }

    public void increaseComposed(boolean result) {
        if (result) {
            ++mComposeredCount;
        }

        if (mReporter != null) {
            mReporter.onOneFinished(this, result);
        }
    }

    public void onStart() {
        if (mReporter != null) {
            mReporter.onStart(this);
        }
    }

    public boolean onEnd() {
        if (mReporter != null) {
            mReporter.onEnd(this, (getCount() == mComposeredCount && mComposeredCount > 0) ? true : false);
        }
        return true;
    }

    public void setParams(List<String> params) {
        mParams = params;
    }

    public boolean composeOneEntity() {
        boolean result = implementComposeOneEntity();
        if (result) {
            ++mComposeredCount;
        }

        if (mReporter != null && !isCancel()) {
            mReporter.onOneFinished(this, result);
        }
        return result;
    }

    public void setCommand(int command) {
        mCommand = command;
    }

    protected boolean checkedCommand() {
        boolean mCheckedCommand = mCommand == Constants.REPLACE_DATA;
        return mCheckedCommand && this.getCount() > 0;
    }
    
    public void setNeedAppData(boolean needAppDate) {
        mNeedAppDate = needAppDate;
    }

    public abstract int getModuleType();

    public abstract int getCount();

    public abstract boolean isAfterLast();

    public abstract boolean init();

    protected abstract boolean implementComposeOneEntity();
}
