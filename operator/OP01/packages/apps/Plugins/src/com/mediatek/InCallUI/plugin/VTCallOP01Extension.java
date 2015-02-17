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

import android.app.Activity;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.RemoteException;
import android.util.Config;
import android.util.FloatMath;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.mediatek.incallui.ext.VTCallExtension;
import com.android.internal.telephony.CallManager;
import com.android.services.telephony.common.ICallCommandService;

public class VTCallOP01Extension extends VTCallExtension {

    private static final String LOG_TAG = "VTInCallOP01Extension";

    private static final int NONEPOINT = 0;
    private static final int DRAGPOINT = 1; // 1 point
    private static final int ZOOMPOINT = 2; // 2 point

    private SurfaceView mVTHighVideo;
    private SurfaceView mVTLowVideo;
    private Context mContext;

    private int mMode = NONEPOINT;
    private float mOldDist;
    private float mChangeThreshold;
    public boolean mVTFullScreen;
    public long mCallStartDate;
    private ICallCommandService mICallCommandService;
    
    public void onViewCreated(Context context, View view, View.OnTouchListener listener, ICallCommandService service) {
        mContext = context;
        mICallCommandService = service;
        Resources resource = mContext.getResources();
        String packageName = mContext.getPackageName();
        mVTHighVideo = (SurfaceView) view.findViewById(resource.getIdentifier("VTHighVideo", "id", packageName));
        mVTLowVideo = (SurfaceView) view.findViewById(resource.getIdentifier("VTLowVideo", "id", packageName));
        if (null != mVTHighVideo) {
            mVTHighVideo.setOnTouchListener(listener);
        }
        mChangeThreshold = mContext.getResources().getDisplayMetrics().density * 20;
    }

    public boolean onTouch(View v, MotionEvent event, boolean isVTPeerBigger) {
        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_DOWN:
                log("MotionEvent.ACTION_DOWN");
                mMode = DRAGPOINT;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                log("MotionEvent.ACTION_UP");
                log("MotionEvent.ACTION_POINTER_UP");
                mMode = NONEPOINT;
                mOldDist = 0;
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                mOldDist = spacing(event);
                mMode = ZOOMPOINT;
                log("MotionEvent.ACTION_POINTER_DOWN, mOldDist is" + mOldDist);
                break;
            case MotionEvent.ACTION_MOVE:
                log("MotionEvent.ACTION_MOVE, mode is " + mMode);
                if (mMode == ZOOMPOINT) {
                    // moving first point
                    float newDist = spacing(event);
                    log("MotionEvent.ACTION_MOVE, new dist is " + newDist + 
                            ", old dist is " + mOldDist + " threshold is " + mChangeThreshold);
                    if ((newDist - mOldDist > mChangeThreshold)
                            && (!getVTFullScreenFlag())) {
                        setVTDisplayScreenMode(true, isVTPeerBigger);
                        mMode = NONEPOINT;
                        mOldDist = 0;
                    } else if ((mOldDist - newDist > mChangeThreshold)
                            && (getVTFullScreenFlag())) {
                        setVTDisplayScreenMode(false, isVTPeerBigger);
                        mMode = NONEPOINT;
                    }
                }
                break;
            default:
                break;
            }
        return false;
    }

    /**
     * Compute two point distance
     */
    private float spacing(MotionEvent event) {
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return FloatMath.sqrt(x * x + y * y);
    }

    private void setVTDisplayScreenMode(final boolean isFullScreenMode, boolean isVTPeerBigger) {
        log("setVTDisplayScreenMode(), isFullScreenMode is " + isFullScreenMode);
        CallCardOP01Extension.getInstance().setCallCardDisplayScreenMode(isFullScreenMode);
        CallButtonOP01Extension.getInstance().setCallButtonDisplayScreenMode(isFullScreenMode);
        if (isFullScreenMode) {
            setVTFullScreenFlag(true);
            if (null != mVTLowVideo && NONEPOINT != mMode) {
                mVTLowVideo.setBackgroundColor(Color.BLACK);
            }
        } else {
            setVTFullScreenFlag(false);
            if (null != mVTLowVideo && NONEPOINT != mMode) {
                mVTLowVideo.setBackgroundDrawable(null);
            }
        }
    }

    public boolean onPrepareOptionsMenu(Menu menu) {
        if (getVTFullScreenFlag()) {
            return true;
        }
        return false;
    }

    public void resetFlags() {
        mVTFullScreen = false;
        mCallStartDate = -1;
    }

    public boolean getVTFullScreenFlag() {
        return mVTFullScreen;
    }

    public void setVTFullScreenFlag(final boolean vtFullScreenFlag) {
        mVTFullScreen = vtFullScreenFlag;
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (getVTFullScreenFlag()) {
            setVTDisplayScreenMode(false, false);
            return false;
        }
        return false;
    }

    private static void log(String msg) {
        if (Config.LOGD) {
            Log.d(LOG_TAG, msg);
        }
    }
}
