/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.InCallUI.plugin;



import java.util.Date;

import android.content.Context;

import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.util.Config;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.android.services.telephony.common.Call;
import com.mediatek.incallui.ext.CallCardExtension;
import com.mediatek.op01.plugin.R;

public class CallCardOP01Extension extends CallCardExtension {

    private static final String LOG_TAG = "CallCardOP01Extension";
    private static final boolean DBG = true;
    private View mView;
    private Context mContext;
    private TextView mCallStateLabel;
    private static CallCardOP01Extension sInstance;

    public static synchronized CallCardOP01Extension getInstance() {
        if (sInstance == null) {
            sInstance = new CallCardOP01Extension();
        }
        return sInstance;
    }

    CallCardOP01Extension(){
        if (sInstance == null) {
            sInstance = this;
        }
    }

    public void onViewCreated(Context context, View rootView) {
        mContext = context;
        mView = rootView;
        log("mContext: " + mContext);
        log("CallCard mView: " + mView);
        Resources resource = mContext.getResources();
        String packageName = mContext.getPackageName();
        mCallStateLabel = (TextView) mView.findViewById(resource.getIdentifier("callStateLabel", "id", packageName));
    }

    public void onStateChange(Call call) {
        if (DBG) {
            log("setCallState(call " + call + ")...");
        }
        final int state = call.getState();
        String strCallStateLabel = null;
        switch (state) {
            case Call.State.DISCONNECTED:
            case Call.State.DISCONNECTING:
                final long startTime = call.getConnectTime();
                final long duration = System.currentTimeMillis() - startTime;
                if (null == mCallStateLabel || duration <= 0 || startTime <= 0 || !call.isVideoCall()) {
                    if (DBG) {
                        log("mCallStateLabel is null,or duration<=0,or is not video call, just return false");
                    }
                    break;
                }
                String sTime = "00";
                try {
                    Context context = mContext.createPackageContext("com.mediatek.op01.plugin",Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                    sTime = context.getString(R.string.vt_start_time_from) 
                    + " " + DateFormat.getTimeFormat(mContext).format(new Date(startTime));
                } catch(NameNotFoundException e){
                    log("no com.mediatek.op01.plugin packages");
                }
                if (!TextUtils.isEmpty(mCallStateLabel.getText())) {
                    strCallStateLabel = mCallStateLabel.getText() + ", " + sTime;
                }
                if (DBG) {
                    log("updateCallStateWidgets(), strCallStateLabel = " + strCallStateLabel);
                }
                mCallStateLabel.setText(strCallStateLabel);
                break;
            default:
               break;
        }
    }

    public void setCallCardDisplayScreenMode(final boolean isFullScreenMode) {
        log("setCallCardDisplayScreenMode(), isFullScreenMode is " + isFullScreenMode);
        log("CallCard mView: " + mView);
        if (null != mView) {
            if (isFullScreenMode) {
                mView.setVisibility(View.INVISIBLE);
            } else {
                mView.setVisibility(View.VISIBLE);
            }
        }
    }

    private static void log(String msg) {
        if (Config.LOGD) {
            Log.d(LOG_TAG, msg);
        }
    }
}
