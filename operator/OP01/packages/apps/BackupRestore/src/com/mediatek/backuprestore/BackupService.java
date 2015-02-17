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
import android.content.IntentFilter;
import android.os.Binder;
import android.os.IBinder;
import android.os.PowerManager;

import com.mediatek.backuprestore.BackupEngine.BackupResultType;
import com.mediatek.backuprestore.BackupEngine.OnBackupDoneListner;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.utils.BackupRestoreNotification;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class BackupService extends Service implements ProgressReporter, OnBackupDoneListner {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupService";
    private static final String TAG = "CMCCPerformanceTest";
    private BackupBinder mBinder = new BackupBinder();
    private int mState;
    private BackupEngine mBackupEngine;
    private ArrayList<ResultEntity> mResultList;
    private BackupProgress mCurrentProgress = new BackupProgress();
    private OnBackupStatusListener mStatusListener;
    private BackupResultType mResultType;
    private ArrayList<ResultEntity> mAppResultList;
    private PowerManager.WakeLock mWakeLock;
    private int mCurrentComposerCount = 0;
    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();

    @Override
    public IBinder onBind(Intent intent) {
        stopForeground(true);
        mCurrentComposerCount = 0;
        MyLogger.logI(CLASS_TAG, "onBind and clearNotification");
        return mBinder;
    }

    public boolean onUnbind(Intent intent) {
        stopForeground(true);
        super.onUnbind(intent);
        MyLogger.logI(CLASS_TAG, "onUnbind");
        return true;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mState = State.INIT;
        stopForeground(true);
        MyLogger.logI(CLASS_TAG, "onCreate");
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        MyLogger.logI(CLASS_TAG, "onStartCommand");
        return START_STICKY_COMPATIBILITY;
    }

    public void onRebind(Intent intent) {
        super.onRebind(intent);
        MyLogger.logI(CLASS_TAG, "onRebind");
    }

    public void onDestroy() {
        stopForeground(true);
        super.onDestroy();
        MyLogger.logI(CLASS_TAG, "onDestroy");
        mCurrentComposerCount = 0;
        if (mBackupEngine != null && mBackupEngine.isRunning()) {
            mBackupEngine.setOnBackupDoneListner(null);
            mBackupEngine.cancel();
        }
    }

    public static class BackupProgress {
        Composer mComposer;
        int mType;
        int mMax;
        int mCurNum;
    }

    public class BackupBinder extends Binder {
        public int getState() {
            return mState;
        }

        public void setBackupModelList(ArrayList<Integer> list) {
            reset();
            if (mBackupEngine == null) {
                mBackupEngine = new BackupEngine(BackupService.this, BackupService.this);
            }
            mBackupEngine.setBackupModelList(list);
            if(!list.contains(ModuleType.TYPE_APP)){
                BackupRestoreNotification.getInstance(BackupService.this).setMaxPercent(list.size());
            }
        }

        public void setBackupItemParam(int itemType, ArrayList<String> paraList) {
            mParasMap.put(itemType, paraList);
            mBackupEngine.setBackupItemParam(itemType, paraList);
            if(itemType == ModuleType.TYPE_APP) {
                BackupRestoreNotification.getInstance(BackupService.this).setMaxPercent(paraList.size());
            }
        }
        
        public void setBackupAppData(boolean isNeedAppData) {
            mBackupEngine.setNeedAppData(isNeedAppData);
        }

        public ArrayList<String> getBackupItemParam(int itemType) {
            return mParasMap.get(itemType);
        }

        public boolean startBackup(String folderName) {
            createWakeLock();
            if(mWakeLock != null) {
                acquireWakeLock();
                MyLogger.logD(CLASS_TAG, "startBackup: call acquireWakeLock()");
            }
            boolean ret = false;
            mBackupEngine.setOnBackupDoneListner(BackupService.this);
            ret = mBackupEngine.startBackup(folderName);
            if (ret) {
                mState = State.RUNNING;
            } else {
                mBackupEngine.setOnBackupDoneListner(null);
            }
            MyLogger.logD(CLASS_TAG, "startBackup: " + ret);
            return ret;
        }

        public void pauseBackup() {
            mState = State.PAUSE;
            if (mBackupEngine != null) {
                mBackupEngine.pause();
            }
            MyLogger.logD(CLASS_TAG, "pauseBackup");
        }

        public void cancelBackup() {
            mState = State.CANCELLING;
            if (mBackupEngine != null) {
                mBackupEngine.cancel();
            }
            if(mWakeLock != null) {
                releaseWakeLock();
                MyLogger.logD(CLASS_TAG, "cancelBackup: call releseWakeLock()");
            }
            stopForeground(true);
            mCurrentComposerCount = 0;
            MyLogger.logD(CLASS_TAG, "cancelBackup and stopForeground and mCurrentComposerCount = 0");
        }

        public void continueBackup() {
            mState = State.RUNNING;
            if (mBackupEngine != null) {
                mBackupEngine.continueBackup();
            }
            MyLogger.logD(CLASS_TAG, "continueBackup");
        }

        public void reset() {
            mState = State.INIT;
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

        public BackupProgress getCurBackupProgress() {
            return mCurrentProgress;
        }

        public void setOnBackupChangedListner(OnBackupStatusListener listener) {
            mStatusListener = listener;
        }

        public ArrayList<ResultEntity> getBackupResult() {
            return mResultList;
        }

        public BackupResultType getBackupResultType() {
            return mResultType;
        }

        public ArrayList<ResultEntity> getAppBackupResult() {
            return mAppResultList;
        }
    }

    @Override
    public void onStart(Composer composer) {
        mCurrentProgress.mComposer = composer;
        mCurrentProgress.mType = composer.getModuleType();
        mCurrentProgress.mMax = composer.getCount();
        mCurrentProgress.mCurNum = 0;
        if (mStatusListener != null) {
            mStatusListener.onComposerChanged(composer);
        }
        if(composer.getModuleType() == ModuleType.TYPE_APP) {
            mCurrentComposerCount = 0;
        }
        if (mCurrentProgress.mMax != 0) {
            BackupRestoreNotification.getInstance(BackupService.this).initBackupNotification(composer.getModuleType(), mCurrentComposerCount);
            startForeground(Constants.STARTFORGROUND, BackupRestoreNotification.getInstance(BackupService.this).getNotification());
        }
        MyLogger.logD(CLASS_TAG, "onStart");
    }

    @Override
    public void onOneFinished(Composer composer, boolean result) {
        mCurrentProgress.mCurNum++;
        if (composer.getModuleType() == ModuleType.TYPE_APP) {
            if (mAppResultList == null) {
                mAppResultList = new ArrayList<ResultEntity>();
            }
            int type = result ? ResultEntity.SUCCESS : ResultEntity.FAIL;
            ResultEntity entity = new ResultEntity(ModuleType.TYPE_APP, type);
            entity.setKey(mParasMap.get(ModuleType.TYPE_APP).get(mCurrentProgress.mCurNum - 1));
            mAppResultList.add(entity);
            BackupRestoreNotification.getInstance(BackupService.this).updateNotification(composer.getModuleType(),mCurrentProgress.mCurNum);
            startForeground(Constants.STARTFORGROUND, BackupRestoreNotification.getInstance(BackupService.this).getNotification());
        }
        if (mStatusListener != null) {
            mStatusListener.onProgressChanged(composer, mCurrentProgress.mCurNum);
        }
        
    }

    @Override
    public void onEnd(Composer composer, boolean result) {
        int resultType = ResultEntity.SUCCESS;
        mCurrentComposerCount++;
        if (mResultList == null) {
            mResultList = new ArrayList<ResultEntity>();
        }
        if (!result) {
            if (composer.getCount() == 0) {
                resultType = ResultEntity.NO_CONTENT;
            } else {
                resultType = ResultEntity.FAIL;
            }
        }
        MyLogger.logD(CLASS_TAG, "one Composer end: type = " + composer.getModuleType() + ", result = " + resultType);
        ResultEntity item = new ResultEntity(composer.getModuleType(), resultType);
        mResultList.add(item);
        if(composer.getModuleType() != ModuleType.TYPE_APP && BackupRestoreNotification.getInstance(BackupService.this) != null){
            BackupRestoreNotification.getInstance(BackupService.this).updateNotification(composer.getModuleType(),mCurrentComposerCount);
            if(BackupRestoreNotification.getInstance(BackupService.this).getNotification() != null) {
                startForeground(Constants.STARTFORGROUND, BackupRestoreNotification.getInstance(BackupService.this).getNotification());
            }
        }
        MyLogger.logD(CLASS_TAG, "onEnd");
    }

    @Override
    public void onErr(IOException e) {
        MyLogger.logD(CLASS_TAG, "onErr " + e.getMessage());
        if (mStatusListener != null) {
            mStatusListener.onBackupErr(e);
        }
    }

    public void onFinishBackup(BackupResultType result) {
        MyLogger.logD(CLASS_TAG, "onFinishBackup result = " + result);
        Xlog.i(TAG,"[CMCC Performance test][BackupAndRestore][Contact_Backup] Backup end ["+ System.currentTimeMillis() +"]");
        mResultType = result;
        if (mStatusListener != null) {
            if (mState == State.CANCELLING) {
                result = BackupResultType.Cancel;
            }
            if (mResultList != null && result != BackupResultType.Success && result != BackupResultType.Cancel) {
                for (ResultEntity item : mResultList) {
                    if (item.getResult() == ResultEntity.SUCCESS) {
                        item.setResult(ResultEntity.FAIL);
                    }
                }
            }
            mState = State.FINISH;
            mStatusListener.onBackupEnd(result, mResultList, mAppResultList);
        } else {
            mState = State.FINISH;
        }
        mCurrentComposerCount = 0;
        stopForeground(true);
        MyLogger.logD(CLASS_TAG, "onFinishBackup stopForeground");
        if(mWakeLock != null) {
            releaseWakeLock();
            MyLogger.logD(CLASS_TAG, "onFinishBackup: call releseWakeLock()");
        }
    }

    interface OnBackupStatusListener {
        void onComposerChanged(Composer composer);

        void onProgressChanged(Composer composer, int progress);

        void onBackupEnd(final BackupResultType resultCode, final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord);

        void onBackupErr(IOException e);
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
