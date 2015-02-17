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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.taskmanager;

import android.app.ActivityManager;
import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.android.internal.util.MemInfoReader;
import com.mediatek.xlog.Xlog;



public class SystemInfoFragment extends Fragment implements RunningState.OnRefreshUiListener,
    TaskManagerPageActivity.PrimarySetListener {
    private static final String TAG = "SystemInfoFragment";

    private View mContentView;

    ActivityManager mAm;
    RunningState mState;  

    private ProgressBar mProgressBar;
    private TextView mPercent;
    private TextView mUsedRAM;
    private TextView mFreeRAM; 
    private TextView mTotalRAM;

    long mSecondaryServerMem;
    byte[] mBuffer = new byte[1024];

    int mLastNumBackgroundProcesses = -1;
    int mLastNumForegroundProcesses = -1;
    int mLastNumServiceProcesses = -1;
    long mLastBackgroundProcessMemory = -1;
    long mLastForegroundProcessMemory = -1;
    long mLastServiceProcessMemory = -1;
    long mLastAvailMemory = -1;

    MemInfoReader mMemInfoReader = new MemInfoReader();

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mAm = (ActivityManager)getActivity().getSystemService(Context.ACTIVITY_SERVICE);
        mContentView = inflater.inflate(R.layout.system_info_view, null);
        mProgressBar = (ProgressBar)mContentView.findViewById(android.R.id.progress);
        mPercent = (TextView)mContentView.findViewById(R.id.precent);
        mUsedRAM = (TextView)mContentView.findViewById(R.id.used_ram_id);
        mFreeRAM = (TextView)mContentView.findViewById(R.id.free_ram_id);
        mTotalRAM = (TextView)mContentView.findViewById(R.id.total_ram_id);

        mState = RunningState.getInstance(getActivity());
        ActivityManager.MemoryInfo memInfo = new ActivityManager.MemoryInfo();
        mAm.getMemoryInfo(memInfo);
        mSecondaryServerMem = memInfo.secondaryServerThreshold;
        return mContentView;
    }

    @Override
    public void onResume() {
        Xlog.i(TAG,"onResume");
        super.onResume();
        //mState.resume(this);
       // if (mState.hasData()) {
            refreshUi();
       // }

    }
    @Override
    public void onPause() {
        Xlog.i(TAG,"onPause");    
        super.onPause();

    }    
    
    void refreshUi() {
        // This is the amount of available memory until we start killing
        // background services.
        mMemInfoReader.readMemInfo();
        long availMem = mMemInfoReader.getFreeSize() + mMemInfoReader.getCachedSize()
                - mSecondaryServerMem;
        if (availMem < 0) {
            availMem = 0;
        }

        synchronized (mState.mLock) {
            if (mLastNumBackgroundProcesses != mState.mNumBackgroundProcesses
                    || mLastBackgroundProcessMemory != mState.mBackgroundProcessMemory
                    || mLastAvailMemory != availMem) {
                mLastNumBackgroundProcesses = mState.mNumBackgroundProcesses;
                mLastBackgroundProcessMemory = mState.mBackgroundProcessMemory;
                mLastAvailMemory = availMem;

                long freeMem = mLastAvailMemory + mLastBackgroundProcessMemory;
                Xlog.d(TAG,"freeMem: " + freeMem);
                String sizeStr = formatSize(freeMem);
                mFreeRAM.setText(sizeStr);

                long total = mMemInfoReader.getTotalSize();
                long usedMem = total - freeMem;
                sizeStr  = formatSize(usedMem);           
                mUsedRAM.setText(sizeStr);

                sizeStr = formatSize(total);      
                mTotalRAM.setText(sizeStr);

                int curPercent = (int)(usedMem * 100 / total);
                Xlog.d(TAG,"curPercent: " + curPercent);
                mPercent.setText(getActivity().getResources().getString(
                R.string.percentage, curPercent));
                mProgressBar.setProgress(curPercent);
            }
            if (mLastNumForegroundProcesses != mState.mNumForegroundProcesses
                    || mLastForegroundProcessMemory != mState.mForegroundProcessMemory
                    || mLastNumServiceProcesses != mState.mNumServiceProcesses
                    || mLastServiceProcessMemory != mState.mServiceProcessMemory) {
                mLastNumForegroundProcesses = mState.mNumForegroundProcesses;
                mLastForegroundProcessMemory = mState.mForegroundProcessMemory;
                mLastNumServiceProcesses = mState.mNumServiceProcesses;
                mLastServiceProcessMemory = mState.mServiceProcessMemory;

            }
        }

    }

    @Override
    public void onRefreshUi(int what) {
        switch (what) {
            case REFRESH_DATA:
                refreshUi();
                break;
            case REFRESH_STRUCTURE:
                refreshUi();
                break;
            default:
                break;
        }
    }    

    @Override    
   public void onSetPrimary() {
        Xlog.i(TAG,"onSetPrimary");
        if (mState != null) {
              mState.pause();
              mState.resume(this);        
        }                
    }

    private  String formatSize(long number) {
        if (getActivity() == null) {
            return "";
        }

        float result = number;
        int suffix = com.android.internal.R.string.byteShort;
        if (result > 900) {
            suffix = com.android.internal.R.string.kilobyteShort;
            result = result / 1024;
        }
        if (result > 900) {
            suffix = com.android.internal.R.string.megabyteShort;
            result = result / 1024;
        }
        if (result > 900) {
            suffix = com.android.internal.R.string.gigabyteShort;
            result = result / 1024;
        }
        if (result > 900) {
            suffix = com.android.internal.R.string.terabyteShort;
            result = result / 1024;
        }
        if (result > 900) {
            suffix = com.android.internal.R.string.petabyteShort;
            result = result / 1024;
        }
        String value = String.format("%.2f", result);

        return getActivity().getResources().
            getString(com.android.internal.R.string.fileSizeSuffix,
                      value, getActivity().getString(suffix));
    }    
}

