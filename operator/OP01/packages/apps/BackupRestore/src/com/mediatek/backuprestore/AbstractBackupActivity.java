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

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.Service;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.backuprestore.BackupEngine.BackupResultType;
import com.mediatek.backuprestore.BackupService.BackupBinder;
import com.mediatek.backuprestore.BackupService.OnBackupStatusListener;
import com.mediatek.backuprestore.CheckedListActivity.OnCheckedCountChangedListener;
import com.mediatek.backuprestore.CheckedListActivity.OnUnCheckedChangedListener;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.utils.Constants.DialogID;
import com.mediatek.backuprestore.utils.Constants.MessageID;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.io.IOException;
import java.util.ArrayList;

public abstract class AbstractBackupActivity extends CheckedListActivity implements OnCheckedCountChangedListener,OnUnCheckedChangedListener {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AbstractBackupActivity";

    protected BaseAdapter mAdapter;
    private Button mButtonBackup;
    private Button mButtonSelect;
    protected ProgressDialog mProgressDialog;
    protected ProgressDialog mCancelDlg;
    protected Handler mHandler;
    protected BackupBinder mBackupService;
    private OnBackupStatusListener mBackupListener;

    private TextView mEmptyView;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.backup);
        init();
        Log.i(CLASS_TAG, "onCreate");
        if (savedInstanceState != null) {
            updateButtonState();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "onResume");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(CLASS_TAG, "onDestroy");
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
        if (mBackupService != null && mBackupService.getState() == State.INIT) {
            stopService();
        }
        if (mBackupService != null) {
            mBackupService.setOnBackupChangedListner(null);
        }
        unRegisterOnCheckedCountChangedListener(this);
        unRegisterOnUnCheckedChangedListener(this);
        unBindService();
        mHandler = null;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU && event.isLongPress()) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void init() {
        this.bindService();
        registerOnCheckedCountChangedListener(this);
        registerOnUnCheckedChangedListener(this);
        initButton();
        initHandler();
        createProgressDlg();
        mEmptyView = (TextView) findViewById(R.id.empty);
        getListView().setEmptyView(mEmptyView);
        mAdapter = initBackupAdapter();
        setListAdapter(mAdapter);

    }

    public void setEmptyViewInfo(int resid) {
        mEmptyView.setText(resid);
    }

    @Override
    public void onCheckedCountChanged() {
        mAdapter.notifyDataSetChanged();
        updateButtonState();
    }
    
    public void OnUnCheckedChanged() {
        updateButtonState();
    }

    public void setOnBackupStatusListener(OnBackupStatusListener listener) {
        mBackupListener = listener;
        if (mBackupListener != null && mBackupService != null) {
            mBackupService.setOnBackupChangedListner(mBackupListener);
        }
    }

    private ProgressDialog createProgressDlg() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setMessage(getString(R.string.backuping));
            mProgressDialog.setCancelable(true);
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setCancelMessage(mHandler.obtainMessage(MessageID.PRESS_BACK));
        }
        return mProgressDialog;
    }

    private ProgressDialog createCancelDlg() {
        if (mCancelDlg == null) {
            mCancelDlg = new ProgressDialog(this);
            mCancelDlg.setMessage(getString(R.string.cancelling));
            mCancelDlg.setCancelable(false);
        }
        return mCancelDlg;
    }

    protected void showProgress() {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.show();
    }

    protected boolean errChecked() {
        boolean ret = false;
        String path = SDCardUtils.getStoragePath();
        if (path == null) {
            // no sdcard
            Log.d(CLASS_TAG, "SDCard is removed");
            // ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_SDCARD_REMOVED);
                    }
                });
            }
        } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
            // no space
            Log.d(CLASS_TAG, "SDCard is full");
            // ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_SDCARD_FULL);
                    }
                });
            }
        } else {
            Log.e(CLASS_TAG, "unkown error");
        }
        return ret;
    }

    private void resetSelectButton() {
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                mButtonSelect.setOnClickListener(mOnClickSelect);
                setButtonStatus(true);
            }
        }, 500);
    }

    View.OnClickListener mOnClickSelect = new Button.OnClickListener() {
        @Override
        public void onClick(final View view) {
            Log.e(CLASS_TAG, "mButton---Select clicked ");

            if (isAllChecked(true)) {
                setAllChecked(false);
            } else {
                setAllChecked(true);
            }
        }
    };

    private void initButton() {
        mButtonSelect = (Button) findViewById(R.id.backup_bt_select);
        mButtonSelect.setOnClickListener(mOnClickSelect);

        mButtonBackup = (Button) findViewById(R.id.backup_bt_backcup);
        mButtonBackup.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(final View view) {
                mButtonSelect.setOnClickListener(null);
                Log.e(CLASS_TAG, "mButton---Backup clicked ");
                if (mBackupService == null || mBackupService.getState() != State.INIT) {
                    Log.e(CLASS_TAG, "Can not to start. BackupService not ready or BackupService is ruuning");
                    resetSelectButton();
                    return;
                }
                if (isAllChecked(false)) {
                    Log.e(CLASS_TAG, "to Backup List is null or empty");
                    resetSelectButton();
                    return;
                }
                String path = SDCardUtils.getStoragePath();
                if (path != null) {
                    setButtonStatus(false);
                    Log.e(CLASS_TAG, "setButtonStatus");
                    startBackup();
                } else {
                    showDialog(DialogID.DLG_NO_SDCARD);
                }
                resetSelectButton();
            }
        });
    }

    protected void setButtonsEnable(boolean enable) {
        if (mButtonBackup != null) {
            mButtonBackup.setEnabled(enable);
        }
        if (mButtonSelect != null) {
            mButtonSelect.setEnabled(enable);
        }
    }

    protected void updateButtonState() {
        if (isAllChecked(false)) {
            mButtonBackup.setEnabled(false);
            mButtonSelect.setText(R.string.selectall);
        } else {
            mButtonBackup.setEnabled(true);
            if (isAllChecked(true)) {
                mButtonSelect.setText(R.string.unselectall);
            } else {
                mButtonSelect.setText(R.string.selectall);
            }
        }
    }

    protected final void initHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(final Message msg) {
                switch (msg.what) {
                case MessageID.PRESS_BACK:
                    if (mBackupService != null && mBackupService.getState() != State.INIT
                            && mBackupService.getState() != State.FINISH) {
                        mBackupService.pauseBackup();
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_CANCEL_CONFIRM);
                    }
                    break;
                default:
                    break;
                }
            }
        };
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;
        switch (id) {
        case DialogID.DLG_CANCEL_CONFIRM:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this).setTitle(R.string.warning)
                    .setMessage(R.string.cancel_backup_confirm)
                    .setPositiveButton(R.string.bt_yes, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface arg0, final int arg1) {
                            if (mBackupService != null && mBackupService.getState() != State.INIT
                                    && mBackupService.getState() != State.FINISH) {
                                if (mCancelDlg == null) {
                                    mCancelDlg = createCancelDlg();
                                }
                                mCancelDlg.show();
                                mBackupService.cancelBackup();
                            }
                        }
                    }).setNegativeButton(R.string.bt_no, new DialogInterface.OnClickListener() {
                        public void onClick(final DialogInterface arg0, final int arg1) {

                            if (mBackupService != null && mBackupService.getState() == State.PAUSE) {
                                mBackupService.continueBackup();
                            }
                            if (mProgressDialog != null) {
                                mProgressDialog.show();
                            }
                        }
                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_SDCARD_REMOVED:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this).setTitle(R.string.warning)
                    .setMessage(R.string.sdcard_removed)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(final DialogInterface dialog, final int which) {
                            if (mBackupService != null && mBackupService.getState() == State.PAUSE) {
                                mBackupService.cancelBackup();
                            }
                        }

                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_SDCARD_FULL:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this).setTitle(R.string.warning)
                    .setMessage(R.string.sdcard_is_full)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface dialog, final int which) {
                            if (mBackupService != null && mBackupService.getState() == State.PAUSE) {
                                mBackupService.cancelBackup();
                            }
                        }
                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_NO_SDCARD:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.notice)
                    .setMessage(R.string.nosdcard_notice).setPositiveButton(android.R.string.ok, null).create();
            break;

        case DialogID.DLG_CREATE_FOLDER_FAILED:
            String name = args.getString("name");
            String msg = String.format(getString(R.string.create_folder_fail), name);
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.notice).setMessage(msg)
                    .setPositiveButton(android.R.string.ok, null).create();
            break;

        default:
            break;
        }
        return dialog;
    }

    /**
     * when backup button click, if can start backup, will call startBackup
     */
    public abstract void startBackup();

    /**
     * init Backup Adapter, when activity will can this function. after call
     * this, activity can't change the adapter.
     * 
     * @return
     */
    public abstract BaseAdapter initBackupAdapter();

    /**
     * after service connected, will can the function, the son activity can do
     * anything that need service connected
     */
    protected abstract void afterServiceConnected();

    @Override
    public void onConfigurationChanged(final Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e(CLASS_TAG, "onConfigurationChanged");
    }

    /**
     * after service connected and data initialed, to check restore state to
     * restore UI. only to check once after onCreate, always used for activity
     * has been killed in background.
     */
    protected void checkBackupState() {
        if (mBackupService != null) {
            int state = mBackupService.getState();
            switch (state) {
            case State.ERR_HAPPEN:
                errChecked();
                break;
            default:
                break;
            }
        }
    }

    private void bindService() {
        this.getApplicationContext().bindService(new Intent(this, BackupService.class), mServiceCon,
                Service.BIND_AUTO_CREATE);
    }

    private void unBindService() {
        if (mBackupService != null) {
            mBackupService.setOnBackupChangedListner(null);
        }
        this.getApplicationContext().unbindService(mServiceCon);
    }

    protected void startService() {
        this.startService(new Intent(this, BackupService.class));
    }

    protected void stopService() {
        if (mBackupService != null) {
            mBackupService.reset();
        }
        this.stopService(new Intent(this, BackupService.class));
    }

    private ServiceConnection mServiceCon = new ServiceConnection() {
        @Override
        public void onServiceConnected(final ComponentName name, final IBinder service) {
            mBackupService = (BackupBinder) service;
            if (mBackupService != null) {
                if (mBackupListener != null) {
                    mBackupService.setOnBackupChangedListner(mBackupListener);
                }
            }
            // checkBackupState();
            afterServiceConnected();
            Log.i(CLASS_TAG, "onServiceConnected");
        }

        @Override
        public void onServiceDisconnected(final ComponentName name) {
            mBackupService = null;
            Log.i(CLASS_TAG, "onServiceDisconnected");
        }
    };

    public class NomalBackupStatusListener implements OnBackupStatusListener {
        @Override
        public void onBackupEnd(final BackupResultType resultCode, final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {
            // do nothing
        }

        @Override
        public void onBackupErr(final IOException e) {
            if (errChecked()) {
                if (mBackupService != null && mBackupService.getState() != State.INIT
                        && mBackupService.getState() != State.FINISH) {
                    mBackupService.pauseBackup();
                }
            } else {
                if (mBackupService != null) {
                    mBackupService.cancelBackup();
                    mBackupService.reset();
                }
            }
        }

        @Override
        public void onComposerChanged(final Composer composer) {

        }

        @Override
        public void onProgressChanged(final Composer composer, final int progress) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                        }
                    }
                });
            }
        }
    }

}
