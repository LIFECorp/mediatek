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

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.backuprestore.RestoreService.RestoreProgress;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.utils.BackupFilePreview;
import com.mediatek.backuprestore.utils.BackupXmlInfo;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.DialogID;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.FileUtils;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.OldBackupFilePreview;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.io.File;
import java.util.ArrayList;

public class PersonalDataRestoreActivity extends AbstractRestoreActivity {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/PersonalDataRestoreActivity";

    private PersonalDataRestoreAdapter mRestoreAdapter;
    private File mFile;
    private boolean mIsDataInitialed;
    private boolean mIsStoped = false;
    private boolean mNeedUpdateResult = false;
    private boolean mIsMtkSms = false;
    BackupFilePreview mPreview = null;
    OldBackupFilePreview mOldPreview = null;
    private PersonalDataRestoreStatusListener mRestoreStoreStatusListener;
    private boolean mIsCheckedRestoreStatus = false;
    private FilePreviewTask mPreviewTask;

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        mFile = new File(intent.getStringExtra(Constants.FILENAME));
        if (!mFile.exists()) {
            if ((mRestoreService != null) && (mRestoreService.getState() != State.INIT)) {
                MyLogger.logD(CLASS_TAG, "onCreate() mRestoreService.getState():" + mRestoreService.getState());
                return;
            }

            Toast.makeText(this, "file don't exist", Toast.LENGTH_LONG);
            finish();
            return;
        } else {
            mPreviewTask = new FilePreviewTask();
            mPreviewTask.execute();
        }
        Log.i(CLASS_TAG, "onCreate");
        init();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "onResume");

        if ((mRestoreService != null) && (mRestoreService.getState() != State.INIT) && mIsDataInitialed) {
            MyLogger.logD(CLASS_TAG, "onResume() mRestoreService.getState():" + mRestoreService.getState());
            return;
        }

        if (!mFile.exists()) {
            Toast.makeText(this, R.string.file_no_exist_and_update, Toast.LENGTH_LONG);
            finish();
            return;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        mIsStoped = false;
        updateResultIfneed();
        if ((mRestoreService != null) && (mRestoreService.getState() != State.INIT && mIsDataInitialed)) {
            MyLogger.logD(CLASS_TAG, "onStart() mRestoreService.getState():" + mRestoreService.getState());
            return;
        }

        if (!mFile.exists()) {
            Toast.makeText(this, R.string.file_no_exist_and_update, Toast.LENGTH_SHORT).show();
            finish();
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        mIsStoped = true;
    }

    @Override
    public void onDestroy() {
        MyLogger.logD(CLASS_TAG, "onDestroy()");
        super.onDestroy();
        if (mPreviewTask != null) {
            mPreviewTask.setCancel();
        }
        mPreviewTask = null;
        mHandler = null;
    }

    private void init() {
        initActionBar();
        //updateTitle();
    }

    public void updateTitle() {
        StringBuilder sb = new StringBuilder();
        sb.append(getString(R.string.backup_personal_data));
        int totalCount = getCount();
        int selectCount = getCheckedCount();
        sb.append("(" + selectCount + "/" + totalCount + ")");
        this.setTitle(sb.toString());
    }

    private void initActionBar() {
        ActionBar bar = this.getActionBar();
        bar.setDisplayShowHomeEnabled(false);

        String barTitle = mFile.getName();
        if (barTitle.endsWith(".zip")) {
            int index = barTitle.lastIndexOf(".");
            barTitle = barTitle.substring(0, index);
        }
        bar.setTitle(mFile.getName());
        StringBuilder builder = new StringBuilder(getString(R.string.backup_data));
        builder.append(" ");
        long size = FileUtils.computeAllFileSizeInFolder(mFile);
        builder.append(FileUtils.getDisplaySize(size, this));
        bar.setSubtitle(builder.toString());
    }

    private void showUpdatingTitle() {
        ActionBar bar = this.getActionBar();
        bar.setTitle(R.string.updating);
        bar.setSubtitle(null);
    }

    @Override
    public void onCheckedCountChanged() {
        super.onCheckedCountChanged();
        updateTitle();
    }

    @Override
    protected void notifyListItemCheckedChanged() {
        super.notifyListItemCheckedChanged();
        updateTitle();
    }

    private ArrayList<Integer> getSelectedItemList() {
        ArrayList<Integer> list = new ArrayList<Integer>();
        int count = getCount();
        for (int position = 0; position < count; position++) {
            PersonalItemData item = (PersonalItemData) getItemByPosition(position);
            if (isItemCheckedByPosition(position)) {
                list.add(item.getType());
            }
        }
        return list;
    }

    @Override
    protected BaseAdapter initAdapter() {
        ArrayList<PersonalItemData> list = new ArrayList<PersonalItemData>();
        mRestoreAdapter = new PersonalDataRestoreAdapter(this, list, R.layout.restore_personal_data_item);
        return mRestoreAdapter;
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;

        switch (id) {
        case DialogID.DLG_RESULT:
            dialog = ResultDialog.createResultDlg(this, R.string.restore_result, args, new OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (mRestoreService != null) {
                        mRestoreService.reset();
                    }
                    stopService();
                    // /M : add for checked SD card status
                    checkedSDCard();
                }
            });
            break;

        default:
            dialog = super.onCreateDialog(id, args);
            break;
        }
        return dialog;
    }

    @Override
    protected void onPrepareDialog(final int id, final Dialog dialog, final Bundle args) {
        switch (id) {
        case DialogID.DLG_RESULT:
            AlertDialog dlg = (AlertDialog) dialog;
            ListView view = (ListView) dlg.getListView();
            if (view != null) {
                ListAdapter adapter = ResultDialog.createResultAdapter(this, args);
                view.setAdapter(adapter);
            }
            break;
        default:
            break;
        }
    }

    private void showRestoreResult(ArrayList<ResultEntity> list) {
        dismissProgressDialog();
        Bundle args = new Bundle();
        args.putParcelableArrayList("result", list);
        showDialog(DialogID.DLG_RESULT, args);
    }

    private void updateResultIfneed() {

        if (mRestoreService != null && mNeedUpdateResult) {
            int state = mRestoreService.getState();
            if (state == State.FINISH) {
                MyLogger.logV(CLASS_TAG, "updateResult because of finish when onStop");
                showRestoreResult(mRestoreService.getRestoreResult());
            }
        }
        mNeedUpdateResult = false;
    }

    private String getProgressDlgMessage(int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.restoring));

        builder.append("(").append(ModuleType.getModuleStringFromType(this, type)).append(")");
        return builder.toString();
    }

    @Override
    public void startRestore(int command) {
        if (!isCanStartRestore()) {
            return;
        }
        startService();
        Log.v(CLASS_TAG, "startRestore");
        ArrayList<Integer> list = getSelectedItemList();
        mRestoreService.setRestoreModelList(list, mIsMtkSms);
        boolean ret = mRestoreService.startRestore(mFile.getAbsolutePath(), command);
        if (ret) {
            showProgressDialog();
            String msg = getProgressDlgMessage(list.get(0));
            setProgressDialogMessage(msg);
            setProgressDialogProgress(0);
            if (mPreview != null) {
                int count = mPreview.getItemCount(list.get(0));
                Log.v(CLASS_TAG, "mPreview.getItemCount(list.get(0)):" + count);
                setProgressDialogMax(count);
            } else if (mOldPreview != null) {
                int count = mOldPreview.getItemCount(list.get(0));
                Log.v(CLASS_TAG, "mOldPreview.getItemCount(list.get(0)):" + count);
                setProgressDialogMax(count);
            }
        } else {
            stopService();
        }
    }

    @Override
    protected void afterServiceConnected() {
        MyLogger.logD(CLASS_TAG, "afterServiceConnected, to checkRestorestate");
        checkRestoreState();
    }

    private void checkRestoreState() {
        if (mIsCheckedRestoreStatus) {
            MyLogger.logD(CLASS_TAG, "can not checkRestoreState, as it has been checked");
            return;
        }
        if (!mIsDataInitialed) {
            MyLogger.logD(CLASS_TAG, "can not checkRestoreState, wait data to initialed");
            return;
        }
        MyLogger.logD(CLASS_TAG, "all ready. to checkRestoreState");
        mIsCheckedRestoreStatus = true;
        if (mRestoreService != null) {
            int state = mRestoreService.getState();
            MyLogger.logD(CLASS_TAG, "checkRestoreState: state = " + state);
            if (state == State.RUNNING || state == State.PAUSE) {

                RestoreProgress p = mRestoreService.getCurRestoreProgress();
                MyLogger.logD(CLASS_TAG, "checkRestoreState: Max = " + p.mMax + " curprogress = " + p.mCurNum);
                String msg = getProgressDlgMessage(p.mType);

                if (state == State.RUNNING) {
                    showProgressDialog();
                }
                setProgressDialogMax(p.mMax);
                setProgressDialogProgress(p.mCurNum);
                setProgressDialogMessage(msg);
            } else if (state == State.FINISH) {
                if (mIsStoped) {
                    mNeedUpdateResult = true;
                } else {
                    showRestoreResult(mRestoreService.getRestoreResult());
                }
            } else if (state == State.ERR_HAPPEN) {
                errChecked();
            }
        }
    }

    @Override
    public void onConfigurationChanged(final Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e(CLASS_TAG, "onConfigurationChanged");
    }

    private class PersonalDataRestoreAdapter extends BaseAdapter {

        private ArrayList<PersonalItemData> mList;
        private int mLayoutId;
        private LayoutInflater mInflater;

        public PersonalDataRestoreAdapter(Context context, ArrayList<PersonalItemData> list, int resource) {
            mList = list;
            mLayoutId = resource;
            mInflater = LayoutInflater.from(context);
        }

        public void changeData(ArrayList<PersonalItemData> list) {
            mList = list;
        }

        public int getCount() {
            if (mList == null) {
                return 0;
            }
            return mList.size();
        }

        public Object getItem(int position) {
            if (mList == null) {
                return null;
            }
            return mList.get(position);

        }

        public long getItemId(int position) {
            PersonalItemData item = mList.get(position);
            return item.getType();

        }

        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(mLayoutId, parent, false);
            }

            PersonalItemData item = mList.get(position);
            ImageView image = (ImageView) view.findViewById(R.id.item_image);
            TextView textView = (TextView) view.findViewById(R.id.item_text);
            CheckBox chxbox = (CheckBox) view.findViewById(R.id.item_chkbox);
            image.setBackgroundResource(item.getIconId());
            textView.setText(item.getTextId());
            boolean enabled = item.isEnable();
            textView.setEnabled(enabled);
            chxbox.setEnabled(enabled);
            view.setClickable(!enabled);
            if (enabled) {
                chxbox.setChecked(isItemCheckedByPosition(position));
            } else {
                chxbox.setChecked(false);
                view.setEnabled(false);
            }
            return view;
        }
    }

    private class FilePreviewTask extends AsyncTask<Void, Void, Long> {

        private ArrayList<Integer> mModuleList;
        private boolean mIsCanceled = false;
        private Composer mCurrentComposer = null;

        public void setCancel() {
            mIsCanceled = true;
            MyLogger.logD(CLASS_TAG, "FilePreviewTask: set cancel");
/*            if (mCurrentComposer != null) {
                mCurrentComposer.setCancel(true);
            }*/
        }

        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            if (!mIsCanceled) {
                ArrayList<PersonalItemData> list = new ArrayList<PersonalItemData>();
                if (list != null) {
                    for (int type : mModuleList) {
                        PersonalItemData item = new PersonalItemData(type, true);
                        list.add(item);
                    }
                }
                updateUnCheckedStates(list, Constants.RESULT_PERSON_DATA);
                mRestoreAdapter.changeData(list);
                syncUnCheckedItems();
                setButtonsEnable(true);
                notifyListItemCheckedChanged();
                mIsDataInitialed = true;
                setProgressBarIndeterminateVisibility(false);
                initActionBar();
                if (mRestoreStoreStatusListener == null) {
                    mRestoreStoreStatusListener = new PersonalDataRestoreStatusListener();
                }
                setOnRestoreStatusListener(mRestoreStoreStatusListener);
                MyLogger.logD(CLASS_TAG, "mIsDataInitialed is ok");
                checkRestoreState();
            }
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            if (!mIsCanceled) {
                setButtonsEnable(false);
                setProgressBarIndeterminateVisibility(true);
                showUpdatingTitle();
            }
        }

        @Override
        protected Long doInBackground(Void... arg0) {
            if (mFile.getAbsolutePath().contains(".backup")) {
                MyLogger.logD(CLASS_TAG, "doInBackground:contain the .backup path");
                mOldPreview = new OldBackupFilePreview(mFile);
                BackupXmlInfo xmlInfo = new BackupXmlInfo();
                mModuleList = mOldPreview.getBackupModules(PersonalDataRestoreActivity.this);
                if (mOldPreview != null) {
                    MyLogger.logD(CLASS_TAG, "mOldPreview is not null");
                    xmlInfo = mOldPreview.getBackupXml();
                    if (xmlInfo != null) {
                        for (Integer moduleType : mModuleList) {
                            if (mIsCanceled) {
                                break;
                            }
                            if (moduleType == ModuleType.TYPE_CONTACT) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getContactNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getContactNum() + "");
                                continue;
                            } else if (moduleType == ModuleType.TYPE_MESSAGE) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getSmsNum() + xmlInfo.getMmsNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getSmsNum() + xmlInfo.getMmsNum()
                                        + "");
                                continue;
                            } else if (moduleType == ModuleType.TYPE_PICTURE) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getPictureNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getPictureNum() + "");
                                continue;
                            } else if (moduleType == ModuleType.TYPE_CALENDAR) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getCalendarNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getCalendarNum() + "");
                                continue;
                            } else if (moduleType == ModuleType.TYPE_APP) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getAppNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getAppNum() + "");
                                continue;
                            } else if (moduleType == ModuleType.TYPE_MUSIC) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getMusicNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getMusicNum() + "");
                                continue;
                            } else if (moduleType == ModuleType.TYPE_NOTEBOOK) {
                                mOldPreview.setItemCount(moduleType, xmlInfo.getNoteBookNum());
                                MyLogger.logD(CLASS_TAG, moduleType + ":" + xmlInfo.getNoteBookNum() + "");
                            }

                        }
                    }
                    mIsMtkSms = FileUtils.isMtkOldSmsData(mFile.getAbsolutePath());
                }
            } else {
                mPreview = new BackupFilePreview(mFile);
                if (mPreview != null) {
                    mModuleList = mPreview.getBackupModules(PersonalDataRestoreActivity.this);

                    for (Integer moduleType : mModuleList) {
                        if (mIsCanceled) {
                            break;
                        }
                        Log.v(CLASS_TAG, "parser type  count: type =  " + moduleType);
                        mCurrentComposer = ModuleType.getModuleRestoreComposerFromType(
                                PersonalDataRestoreActivity.this, moduleType);
                        if (mCurrentComposer != null && !mIsCanceled) {
                            mCurrentComposer.setParentFolderPath(mFile.getAbsolutePath());
                            mCurrentComposer.init();
                            if (!mIsCanceled) {
                                int count = mCurrentComposer.getCount();
                                Log.v(CLASS_TAG, "initNumByType: count = " + count);
                                mPreview.setItemCount(moduleType, count);
                            }
                        }
                    }
                }
            }

            Log.v(CLASS_TAG, "FilePreviewTask: doInBackground finish");
            return null;
        }
    }

    private class PersonalDataRestoreStatusListener extends NormalRestoreStatusListener {

        public void onComposerChanged(final int type, final int max) {
            Log.i(CLASS_TAG, "RestoreDetailActivity: onComposerChanged");

            if (mHandler != null) {
                mHandler.post(new Runnable() {

                    public void run() {
                        String msg = getProgressDlgMessage(type);
                        setProgressDialogMessage(msg);
                        setProgressDialogMax(max);
                        setProgressDialogProgress(0);
                    }
                });
            }
        }

        public void onRestoreEnd(boolean bSuccess, ArrayList<ResultEntity> resultRecord) {
            final ArrayList<ResultEntity> iResultRecord = resultRecord;
            MyLogger.logD(CLASS_TAG, "onRestoreEnd");
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        MyLogger.logD(CLASS_TAG, " Restore show Result Dialog");
                        if (mIsStoped) {
                            mNeedUpdateResult = true;
                        } else {
                            showRestoreResult(iResultRecord);
                        }
                    }
                });
            }
        }
    }

    @Override
    protected void showDialogModule(int dialogId) {
        super.showDialog(dialogId);
    }

    @Override
    protected void startRestore() {
    }

    public void checkedSDCard() {
        if (!SDCardUtils.isSdCardAvailable()) {
            Toast.makeText(this, R.string.sdcard_swap_remove, Toast.LENGTH_LONG).show();
            finish();
        }
    }
}
