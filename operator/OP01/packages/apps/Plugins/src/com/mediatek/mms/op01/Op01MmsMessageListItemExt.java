/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.mms.op01;

import com.mediatek.mms.ext.MmsMessageListItemImpl;

import android.content.Context;
import android.content.ContextWrapper;

import com.mediatek.op01.plugin.R;
import android.os.Handler;
import android.widget.Toast;
import android.os.Message;
import com.mediatek.xlog.Xlog;
import java.lang.CharSequence;

public class Op01MmsMessageListItemExt extends MmsMessageListItemImpl {
    private static final String TAG = "Mms/Op01MmsMessageListItemExt";
    
    public Op01MmsMessageListItemExt(Context context) {
        super(context);
    }

    /// M: New plugin API @{
    private static final int MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL = 2;
    private Context mContext = null;
    private ToastHandler mToastHandler = null;
    
    public boolean showStorageFullToast(Context context) {
        Xlog.d(TAG, "showStorageFullToast");
        mContext = context;
        if (null == mToastHandler) {
            mToastHandler = new ToastHandler();
        }
        mToastHandler.sendEmptyMessage(MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL);
        return true;
    }

    public final class ToastHandler extends Handler {
        public ToastHandler() {
            super();
        }

        @Override
        public void handleMessage(Message msg) {
            Xlog.d(TAG, "Toast Handler handleMessage :" + msg);
            
            switch (msg.what) {
                case MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL: {
                    CharSequence string = Op01MmsMessageListItemExt.this.getResources().getText(R.string.download_failed_due_to_full_memory);
                    Xlog.d(TAG, "string =" + string);
                    Toast.makeText(mContext, string, Toast.LENGTH_LONG).show();
                    break;
                }
            }
        }
    }
    /// @}
}