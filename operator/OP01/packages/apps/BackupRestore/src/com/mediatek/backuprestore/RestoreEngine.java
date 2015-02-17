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

import android.content.Context;

import com.mediatek.backuprestore.modules.AppRestoreComposer;
import com.mediatek.backuprestore.modules.CalendarRestoreComposer;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.modules.ContactRestoreComposer;
import com.mediatek.backuprestore.modules.MessageRestoreComposer;
import com.mediatek.backuprestore.modules.MmsRestoreComposer;
import com.mediatek.backuprestore.modules.MusicRestoreComposer;
import com.mediatek.backuprestore.modules.NoteBookRestoreComposer;
import com.mediatek.backuprestore.modules.OldAppRestoreComposer;
import com.mediatek.backuprestore.modules.OldCalendarRestoreComposer;
import com.mediatek.backuprestore.modules.OldContactRestoreComposer;
import com.mediatek.backuprestore.modules.OldMmsRestoreComposer;
import com.mediatek.backuprestore.modules.OldMusicRestoreComposer;
import com.mediatek.backuprestore.modules.OldPictureRestoreComposer;
import com.mediatek.backuprestore.modules.OldSmsRestoreComposer;
import com.mediatek.backuprestore.modules.PictureRestoreComposer;
import com.mediatek.backuprestore.modules.SmsRestoreComposer;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class RestoreEngine {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreEngine";

    interface OnRestoreDoneListner {
        void onFinishRestore(boolean bSuccess);
    }

    private Context mContext;
    private String mRestoreFolder;
    private String mZipFileName;
    private OnRestoreDoneListner mRestoreDoneListner;
    private boolean mIsRunning = false;
    private boolean mIsPause = false;
    private boolean mIsMtkSms = false;
    private Object mLock = new Object();
    private ProgressReporter mReporter;
    private List<Composer> mComposerList;
    private static RestoreEngine sSelfInstance;
    private int mCommand;

    public static synchronized RestoreEngine getInstance(final Context context, final ProgressReporter reporter) {
        if (sSelfInstance == null) {
            sSelfInstance = new RestoreEngine(context, reporter);
        } else {
            sSelfInstance.updateInfo(context, reporter);
        }
        return sSelfInstance;
    }

    private RestoreEngine(Context context, ProgressReporter reporter) {
        mContext = context;
        mReporter = reporter;
        mComposerList = new ArrayList<Composer>();
    }

    private void updateInfo(final Context context, final ProgressReporter reporter) {
        mContext = context;
        mReporter = reporter;
    }

    public String getRestoreFolder() {
        if (mRestoreFolder != null) {
            return mRestoreFolder;
        } else {
            return null;
        }

    }

    public boolean isRunning() {
        return mIsRunning;
    }

    public void pause() {
        mIsPause = true;
    }

    public boolean isPaused() {
        return mIsPause;
    }

    public void continueRestore() {
        if (mIsPause) {
            synchronized (mLock) {
                mIsPause = false;
                mLock.notify();
            }
        }
    }

    public void cancel() {
        if (mComposerList != null && mComposerList.size() > 0) {
            for (Composer composer : mComposerList) {
                composer.setCancel(true);
            }
            continueRestore();
        }
    }

    public void setOnRestoreEndListner(OnRestoreDoneListner restoreEndListner) {
        mRestoreDoneListner = restoreEndListner;
    }

    ArrayList<Integer> mModuleList;

    public void setRestoreModelList(ArrayList<Integer> moduleList, Boolean isMtkSms) {
        reset();
        mModuleList = moduleList;
        mIsMtkSms = isMtkSms;
    }

    public void setRestoreModelList(ArrayList<Integer> moduleList) {
        reset();
        mModuleList = moduleList;
    }

    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();

    public void setRestoreItemParam(int itemType, ArrayList<String> paraList) {
        mParasMap.put(itemType, paraList);
    }

    public void startRestore(final String path) {
        mZipFileName = null;
        mRestoreFolder = null;
        if (path.contains(".backup") && mModuleList.size() > 0) {
            MyLogger.logD(CLASS_TAG, "startRestore path:" + path);
            // mZipFileName =
            // path.substring(path.lastIndexOf(File.separator)+1);
            mZipFileName = path;
            MyLogger.logD(CLASS_TAG, "mZipFileName" + mZipFileName);
            setupOldComposer(mModuleList);
        } else if (path != null && mModuleList.size() > 0) {
            mRestoreFolder = path;
            setupComposer(mModuleList);
        }
        mIsRunning = true;
        new RestoreThread().start();
    }

    public void startRestore(String path, List<Integer> list) {
        reset();
        if (path != null && list.size() > 0) {
            mRestoreFolder = path;
            setupComposer(list);
            mIsRunning = true;
            new RestoreThread().start();
        }
    }

    private void addOldComposer(Composer composer) {
        if (composer != null) {
            composer.setReporter(mReporter);
            composer.setZipFileName(mZipFileName);
            mComposerList.add(composer);
        }
    }

    private void addComposer(Composer composer) {
        if (composer != null) {
            int type = composer.getModuleType();
            ArrayList<String> params = mParasMap.get(type);
            if (params != null) {
                composer.setParams(params);
            }
            composer.setReporter(mReporter);
            composer.setParentFolderPath(mRestoreFolder);
            mComposerList.add(composer);
        }
    }

    private void reset() {
        if (mComposerList != null) {
            mComposerList.clear();
        }
        mIsPause = false;
    }

    private boolean setupComposer(List<Integer> list) {
        boolean bSuccess = true;
        for (int type : list) {
            switch (type) {
            case ModuleType.TYPE_CONTACT:
                addComposer(new ContactRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_MESSAGE:
                addComposer(new MessageRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_SMS:
                addComposer(new SmsRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_MMS:
                addComposer(new MmsRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_PICTURE:
                addComposer(new PictureRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_CALENDAR:
                addComposer(new CalendarRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_APP:
                addComposer(new AppRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_MUSIC:
                addComposer(new MusicRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_NOTEBOOK:
                addComposer(new NoteBookRestoreComposer(mContext));
                break;

            default:
                bSuccess = false;
                break;
            }
        }

        return bSuccess;
    }

    // To compatible with old data.
    private boolean setupOldComposer(List<Integer> list) {
        boolean bSuccess = true;
        for (int type : list) {
            switch (type) {
            case ModuleType.TYPE_CONTACT:
                addOldComposer(new OldContactRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_MESSAGE:
                String old = "othersSms";
                MyLogger.logD(CLASS_TAG, "FileUtils.isMtkOldSmsData(mZipFileName)" + mZipFileName);
                // boolean isMtkSms = FileUtils.isMtkOldSmsData(mZipFileName);
                if (mIsMtkSms) {
                    old = "Mtk";
                }
                addOldComposer(new MessageRestoreComposer(mContext, old));
                break;

            case ModuleType.TYPE_SMS:
                addOldComposer(new OldSmsRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_MMS:
                addOldComposer(new OldMmsRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_PICTURE:
                addOldComposer(new OldPictureRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_CALENDAR:
                addOldComposer(new OldCalendarRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_APP:
                addOldComposer(new OldAppRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_MUSIC:
                addOldComposer(new OldMusicRestoreComposer(mContext));
                break;

            case ModuleType.TYPE_NOTEBOOK:
                addOldComposer(new NoteBookRestoreComposer(mContext));
                break;

            default:
                bSuccess = false;
                break;
            }
        }

        return bSuccess;
    }

    public class RestoreThread extends Thread {
        @Override
        public void run() {
            MyLogger.logD(CLASS_TAG, "RestoreThread begin...");
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            for (Composer composer : mComposerList) {
                MyLogger.logD(CLASS_TAG, "RestoreThread composer:" + composer.getModuleType() + " start..");
                MyLogger.logD(CLASS_TAG, "begin restore:" + System.currentTimeMillis());
                if (!composer.isCancel()) {
                    composer.setCommand(mCommand);
                    composer.init();
                    MyLogger.logD(CLASS_TAG, "RestoreThread composer:" + composer.getModuleType() + " init finish");
                    composer.onStart();
                    while (!composer.isAfterLast() && !composer.isCancel()) {
                        if (mIsPause) {
                            synchronized (mLock) {
                                try {
                                    MyLogger.logD(CLASS_TAG, "RestoreThread wait...");
                                    mLock.wait();
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        }

                        composer.composeOneEntity();
                        MyLogger.logD(CLASS_TAG, "RestoreThread composer:" + composer.getModuleType()
                                + "composer one entiry");
                    }
                }

                try {
                    sleep(Constants.TIME_SLEEP_WHEN_COMPOSE_ONE);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                composer.onEnd();
                MyLogger.logD(CLASS_TAG, "end restore:" + System.currentTimeMillis());
                MyLogger.logD(CLASS_TAG, "RestoreThread composer:" + composer.getModuleType() + " composer finish");
            }

            MyLogger.logD(CLASS_TAG, "RestoreThread run finish");
            mIsRunning = false;

            if (mRestoreDoneListner != null) {
                mRestoreDoneListner.onFinishRestore(true);
            }
        }
    }

    public void setCommand(int command) {
        mCommand = command;

    }
}
