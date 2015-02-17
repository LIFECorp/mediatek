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

package com.mediatek.backuprestore;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.PowerManager;

import com.mediatek.backuprestore.RestoreEngine.OnRestoreDoneListner;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.utils.BackupRestoreNotification;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class RestoreService extends Service implements ProgressReporter, OnRestoreDoneListner {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreService";
    private static final String TAG = "CMCCPerformanceTest";

    interface OnRestoreStatusListener {
        void onComposerChanged(final int type, final int num);

        void onProgressChanged(Composer composer, int progress);

        void onRestoreEnd(boolean bSuccess, ArrayList<ResultEntity> resultRecord);

        void onRestoreErr(IOException e);
    }

    public static class RestoreProgress {
        int mType;
        int mMax;
        int mCurNum;
    }

    private RestoreBinder mBinder = new RestoreBinder();
    private int mState;
    private RestoreEngine mRestoreEngine;
    private ArrayList<ResultEntity> mResultList;
    private RestoreProgress mCurrentProgress = new RestoreProgress();
    private OnRestoreStatusListener mRestoreStatusListener;
    private ArrayList<ResultEntity> mAppResultList;
    private PowerManager.WakeLock mWakeLock;
    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();
    private int mCurrentComposerCount = 0;

    @Override
    public IBinder onBind(Intent intent) {
        stopForeground(true);
        mCurrentComposerCount = 0;
        MyLogger.logI(CLASS_TAG, "onbind");
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        stopForeground(true);
        super.onUnbind(intent);
        MyLogger.logI(CLASS_TAG, "onUnbind");
        return true;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        moveToState(State.INIT);
        stopForeground(true);
        MyLogger.logI(CLASS_TAG, "onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        MyLogger.logI(CLASS_TAG, "onStartCommand");
        return START_STICKY_COMPATIBILITY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MyLogger.logI(CLASS_TAG, "onDestroy");
        if (mRestoreEngine != null && mRestoreEngine.isRunning()) {
            mRestoreEngine.setOnRestoreEndListner(null);
            mRestoreEngine.cancel();
        }
        stopForeground(true);
        mCurrentComposerCount = 0;
    }

    public void moveToState(int state) {
        synchronized (this) {
            mState = state;
        }
    }

    private int getRestoreState() {
        synchronized (this) {
            return mState;
        }
    }

    public class RestoreBinder extends Binder {
        public int getState() {
            return getRestoreState();
        }

        public void setRestoreModelList(ArrayList<Integer> list, Boolean isMtkSms) {
            reset();
            if (mRestoreEngine == null) {
                mRestoreEngine = RestoreEngine.getInstance(RestoreService.this, RestoreService.this);
            }
            mRestoreEngine.setRestoreModelList(list, isMtkSms);
            BackupRestoreNotification.getInstance(RestoreService.this).setMaxPercent(list.size());
        }

        public void setRestoreModelList(ArrayList<Integer> list) {
            reset();
            if (mRestoreEngine == null) {
                mRestoreEngine = RestoreEngine.getInstance(RestoreService.this, RestoreService.this);
            }
            mRestoreEngine.setRestoreModelList(list);
        }

        public void setRestoreItemParam(int itemType, ArrayList<String> paraList) {
            mParasMap.put(itemType, paraList);
            mRestoreEngine.setRestoreItemParam(itemType, paraList);
            BackupRestoreNotification.getInstance(RestoreService.this).setMaxPercent(paraList.size());
        }

        public ArrayList<String> getRestoreItemParam(int itemType) {
            return mParasMap.get(itemType);
        }

        public boolean startRestore(String fileName) {
            createWakeLock();
            if(mWakeLock != null) {
                acquireWakeLock();
                MyLogger.logD(CLASS_TAG, "startRestore(): call acquireWakeLock()");
            }
            if (mRestoreEngine == null) {
                MyLogger.logE(CLASS_TAG, "startRestore Error: engine is not initialed");
                return false;
            }
            mRestoreEngine.setOnRestoreEndListner(RestoreService.this);
            mRestoreEngine.startRestore(fileName);
            moveToState(State.RUNNING);
            return true;
        }

        public boolean startRestore(String fileName, int command) {
            createWakeLock();
            if(mWakeLock != null) {
                acquireWakeLock();
                MyLogger.logD(CLASS_TAG, "startRestore: call acquireWakeLock()");
            }
            if (mRestoreEngine == null) {
                MyLogger.logE(CLASS_TAG, "startRestore Error: engine is not initialed");
                return false;
            }
            mRestoreEngine.setCommand(command);
            mRestoreEngine.setOnRestoreEndListner(RestoreService.this);
            mRestoreEngine.startRestore(fileName);
            moveToState(State.RUNNING);
            return true;
        }

        public void pauseRestore() {
            if (mState == State.INIT) {
                return;
            }
            moveToState(State.PAUSE);
            if (mRestoreEngine != null) {
                mRestoreEngine.pause();
            }
            MyLogger.logD(CLASS_TAG, "pauseRestore");
        }

        public void continueRestore() {
            if (mState == State.INIT) {
                return;
            }
            moveToState(State.RUNNING);
            if (mRestoreEngine != null) {
                mRestoreEngine.continueRestore();
            }
            MyLogger.logD(CLASS_TAG, "continueRestore");
        }

        public void cancelRestore() {
            if (mState == State.INIT) {
                return;
            }
            moveToState(State.CANCELLING);
            if (mRestoreEngine != null) {
                mRestoreEngine.cancel();
            }
            if(mWakeLock != null) {
                releaseWakeLock();
                MyLogger.logD(CLASS_TAG, "cancelRestore: call releseWakeLock()");
            }
            mCurrentComposerCount = 0;
            stopForeground(true);
            MyLogger.logD(CLASS_TAG, "cancelRestore and stopForeground");
        }

        public void reset() {
            moveToState(State.INIT);
            if (mResultList != null) {
                mResultList.clear();
            }
            if (mAppResultList != null) {
                mAppResultList.clear();
            }
            if (mParasMap != null) {
                mParasMap.clear();
            }
        }

        public RestoreProgress getCurRestoreProgress() {
            return mCurrentProgress;
        }

        public void setOnRestoreChangedListner(OnRestoreStatusListener listener) {
            mRestoreStatusListener = listener;
        }

        public ArrayList<ResultEntity> getRestoreResult() {
            return mResultList;
        }

        public ArrayList<ResultEntity> getAppRestoreResult() {
            return mAppResultList;
        }
    }

    public void onStart(Composer iComposer) {
        mCurrentProgress.mType = iComposer.getModuleType();
        mCurrentProgress.mMax = iComposer.getCount();
        mCurrentProgress.mCurNum = 0;
        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onComposerChanged(mCurrentProgress.mType, mCurrentProgress.mMax);
        }
        if(iComposer.getModuleType() == ModuleType.TYPE_APP) {
            mCurrentComposerCount = 0;
        }
        if (mCurrentProgress.mMax != 0) {
            BackupRestoreNotification.getInstance(RestoreService.this).initRestoreNotification(iComposer.getModuleType(), mCurrentComposerCount);
            startForeground(Constants.STARTFORGROUND, BackupRestoreNotification.getInstance(RestoreService.this).getNotification());
        }
    }

    public void onOneFinished(Composer composer, boolean result) {

        mCurrentProgress.mCurNum++;
        if ((composer.getModuleType() == ModuleType.TYPE_APP) && (mRestoreEngine.getRestoreFolder()) != null) {
            if (mAppResultList == null) {
                mAppResultList = new ArrayList<ResultEntity>();
            }
            ResultEntity entity = new ResultEntity(ModuleType.TYPE_APP, result ? ResultEntity.SUCCESS
                    : ResultEntity.FAIL);
            entity.setKey(mParasMap.get(ModuleType.TYPE_APP).get(mCurrentProgress.mCurNum - 1));
            mAppResultList.add(entity);
            BackupRestoreNotification.getInstance(RestoreService.this).updateNotification(composer.getModuleType(), mCurrentProgress.mCurNum);
            if(BackupRestoreNotification.getInstance(RestoreService.this) != null) {
                startForeground(Constants.STARTFORGROUND,BackupRestoreNotification.getInstance(RestoreService.this).getNotification());
            }
        }
        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onProgressChanged(composer, mCurrentProgress.mCurNum);
        }
    }

    public void onEnd(Composer composer, boolean result) {
        Xlog.i(TAG,"[CMCC Performance test][BackupAndRestore][Contact_Restore] Restore end ["+ System.currentTimeMillis() +"]");
        mCurrentComposerCount++;
        if (mResultList == null) {
            mResultList = new ArrayList<ResultEntity>();
        }
        ResultEntity item = new ResultEntity(composer.getModuleType(), result ? ResultEntity.SUCCESS
                : ResultEntity.FAIL);
        mResultList.add(item);
        if(composer.getModuleType() != ModuleType.TYPE_APP && BackupRestoreNotification.getInstance(RestoreService.this) != null) {
            BackupRestoreNotification.getInstance(RestoreService.this).updateNotification(composer.getModuleType(), mCurrentComposerCount);
            if(BackupRestoreNotification.getInstance(RestoreService.this).getNotification() != null){
                startForeground(Constants.STARTFORGROUND,BackupRestoreNotification.getInstance(RestoreService.this).getNotification());
            }
        }
    }

    public void onErr(IOException e) {

        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onRestoreErr(e);
        }
    }

    public void onFinishRestore(boolean bSuccess) {
        moveToState(State.FINISH);
        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onRestoreEnd(bSuccess, mResultList);
        }
        mCurrentComposerCount = 0;
        stopForeground(true);
        MyLogger.logD(CLASS_TAG, "onFinishRestore and stopForeground");

        if(mWakeLock != null) {
            releaseWakeLock();
            MyLogger.logD(CLASS_TAG, "onFinishRestore: call releseWakeLock()");
        }
    }

    private synchronized void createWakeLock() {
        // Create a new wake lock if we haven't made one yet.
        if (mWakeLock == null) {
            PowerManager pm = (PowerManager)getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "RestoreService");
            mWakeLock.setReferenceCounted(false);
            MyLogger.logD(CLASS_TAG, "createWakeLock");
        }
    }

    private void acquireWakeLock() {
        // It's okay to double-acquire this because we are not using it
        // in reference-counted mode.
        mWakeLock.acquire();
        MyLogger.logD(CLASS_TAG, "acquireWakeLock");
    }

    private void releaseWakeLock() {
        // Don't release the wake lock if it hasn't been created and acquired.
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
            mWakeLock = null;
            MyLogger.logD(CLASS_TAG, "releaseWakeLock");
        }
        MyLogger.logD(CLASS_TAG, "releaseLock");
    }
}
