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
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.backuprestore.BackupEngine.BackupResultType;
import com.mediatek.backuprestore.BackupService.BackupProgress;
import com.mediatek.backuprestore.BackupService.OnBackupStatusListener;
import com.mediatek.backuprestore.CheckedListActivity.OnCheckedCountChangedListener;
import com.mediatek.backuprestore.CheckedListActivity.OnUnCheckedChangedListener;
import com.mediatek.backuprestore.ContactItemData;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.modules.AppBackupComposer;
import com.mediatek.backuprestore.modules.CalendarBackupComposer;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.modules.ContactBackupComposer;
import com.mediatek.backuprestore.modules.MessageBackupComposer;
import com.mediatek.backuprestore.modules.MmsBackupComposer;
import com.mediatek.backuprestore.modules.MusicBackupComposer;
import com.mediatek.backuprestore.modules.NoteBookBackupComposer;
import com.mediatek.backuprestore.modules.PictureBackupComposer;
import com.mediatek.backuprestore.modules.SmsBackupComposer;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.ContactType;
import com.mediatek.backuprestore.utils.Constants.DialogID;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.FileUtils;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class PersonalDataBackupActivity extends AbstractBackupActivity implements OnCheckedCountChangedListener,
        OnUnCheckedChangedListener {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/PersonalDataBackupActivity";
    private static final float DISABLE_ALPHA = 0.4f;
    private static final float ENABLE_ALPHA = 1.0f;
    public static final String ACTION_PHB_LOAD_FINISHED = "com.android.contacts.ACTION_PHB_LOAD_FINISHED";

    private ArrayList<PersonalItemData> mBackupItemDataList = new ArrayList<PersonalItemData>();
    private PersonalDataBackupAdapter mBackupListAdapter;
    private OnBackupStatusListener mBackupListener;
    private boolean[] mContactCheckTypes = new boolean[3];
    private static final String CONTACT_TYPE = "contact";
    private String mBackupFolder;
    //private InitPersonData mInitPersonData;
    private List<Composer> mComposerList;
    private long mResule;
    // /M add for simcard info display
    private ArrayList<ContactItemData> mContactItemDataList;
    private ContactItemData mContactItemData;
    private AlertDialog alertDialog;
    private SimInfoAdapter mSimInfoAdapter;
    private boolean[] mTemp;
    Map<String, Integer> mSimCountMap;
    private static final Object sLock = new Object();

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (savedInstanceState != null) {
            mContactCheckTypes = savedInstanceState.getBooleanArray(CONTACT_TYPE);
        } else {
            restoreInstanceStateFromBackupTab();
        }
        Log.i(CLASS_TAG, "onCreate");
        init();
    }

    private void restoreInstanceStateFromBackupTab() {
        Intent intent = getIntent();
        Bundle mBundle = intent.getExtras();
        if (mBundle != null) {
            // mContactCheckTypes = mBundle.getBooleanArray("contactType");
            if (mBundle.getBooleanArray("contactType") == null) {
                Log.i(CLASS_TAG, "mContactCheckTypes  is null and init ");
                initContactCheckTypes();
            } else {
                mContactCheckTypes = mBundle.getBooleanArray("contactType");
            }
        } else {
            initContactCheckTypes();
        }
        Log.i(CLASS_TAG, "restoreInstanceStateFromBackupTab  add mContactCheckTypes");
    }
    
    private void initContactCheckTypes() {
        for (int index = 0; index < 3; index++) {
            mContactCheckTypes[index] = true;
        }
    }

    @Override
    protected void onSaveInstanceState(final Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putBooleanArray(CONTACT_TYPE, mContactCheckTypes);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "onResume");
    }

    /**
     * When activated interface, check the number of all entries set to display
     * the status
     */
    @Override
    protected void onStart() {
        super.onStart();
        Log.i(CLASS_TAG, "onStart");
        /*if (mInitPersonData == null || !mInitPersonData.isRunning()) {
            mInitPersonData = new InitPersonData(this);
            mInitPersonData.execute();
            Log.i(CLASS_TAG, "onStart mInitPersonData is running !");
        }*/
    }

    private void init() {
        initActionBar();
        // do in background ... updateTitle
        // for new feature UI modify
        updateTitle();
        updateButtonState();
        getSimInfoList();
        //registerAirPlaneModeReceiver();
        setRequestCode(Constants.RESULT_PERSON_DATA);
    }
    
    /*private void registerAirPlaneModeReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_PHB_LOAD_FINISHED);
        this.registerReceiver(mAirPlaneModeReceiver, filter);
    }
    boolean mIsAirPlaneMode;
    private BroadcastReceiver mAirPlaneModeReceiver = new BroadcastReceiver(){

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(ACTION_PHB_LOAD_FINISHED)) {
                Log.i(CLASS_TAG, "onReceive = "+action);
                //if(alertDialog.isShowing()){
                    mInitPersonData = new InitPersonData(context);
                    mInitPersonData.execute();
                //}
            }
        }
    };*/

    public void updateTitle() {
        StringBuilder sb = new StringBuilder();
        sb.append(getString(R.string.backup_personal_data));
        int totalNum = getCount();
        int checkedNum = getCheckedCount();
        Log.i(CLASS_TAG, "updateTitle totalNum = " + totalNum + ",  checkedNum = " + checkedNum);
        sb.append("(" + checkedNum + "/" + totalNum + ")");
        this.setTitle(sb.toString());
        if(checkedNum == 0) {
            updateButtonState();
        }
    }

    private void initActionBar() {
        ActionBar bar = this.getActionBar();
        bar.setDisplayShowHomeEnabled(false);
    }

    private void startPersonalDataBackup(String folderName) {
        Log.v(CLASS_TAG, "startPersonalDataBackup, contactType = " + mContactCheckTypes);
        if (mBackupService != null) {
            ArrayList<Integer> list = getSelectedItemList();
            if (list == null || list.size() == 0) {
                MyLogger.logE(CLASS_TAG, "Error: no item to backup");
                return;
            }
            startService();
            mBackupService.setBackupModelList(list);
            if (list.contains(ModuleType.TYPE_CONTACT)) {
                ArrayList<String> params = new ArrayList<String>();
                if (mContactCheckTypes[0]) {
                    params.add(ContactType.PHONE);
                }
                if (mContactCheckTypes.length > 1 && mContactCheckTypes[1] && mSimCount >= 1) {
                    params.add(Long.toString(mSimInfoList.get(0).mSimSlotId));
                }
                if (mContactCheckTypes.length > 2 && mContactCheckTypes[2] && mSimCount == 2) {
                    params.add(Long.toString(mSimInfoList.get(1).mSimSlotId));
                }
                mBackupService.setBackupItemParam(ModuleType.TYPE_CONTACT, params);
            }
            boolean ret = mBackupService.startBackup(folderName);
            if (ret) {
                showProgress();
            } else {
                String path = SDCardUtils.getStoragePath();
                if (path == null) {
                    // no sdcard
                    Log.d(CLASS_TAG, "SDCard is removed");
                    ret = true;
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            showDialog(DialogID.DLG_SDCARD_REMOVED);
                        }
                    });
                } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
                    // no space
                    Log.d(CLASS_TAG, "SDCard is full");
                    ret = true;
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            showDialog(DialogID.DLG_SDCARD_FULL);
                        }
                    });
                } else {
                    Log.e(CLASS_TAG, "unkown error");
                    Bundle b = new Bundle();
                    b.putString("name", folderName.substring(folderName.lastIndexOf('/') + 1));
                    showDialog(DialogID.DLG_CREATE_FOLDER_FAILED, b);
                }

                stopService();
            }
        } else {
            stopService();
            MyLogger.logE(CLASS_TAG, "startPersonalDataBackup: error! service is null");
        }
    }

    @Override
    public void startBackup() {
        Log.v(CLASS_TAG, "startBackup");
        showDialog(DialogID.DLG_EDIT_FOLDER_NAME);
    }

    protected void afterServiceConnected() {
        mBackupListener = new PersonalDataBackupStatusListener();
        setOnBackupStatusListener(mBackupListener);
        checkBackupState();
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

    public BaseAdapter initBackupAdapter() {
        mBackupItemDataList = new ArrayList<PersonalItemData>();
        int types[] = new int[] { ModuleType.TYPE_CONTACT,
                ModuleType.TYPE_MESSAGE, ModuleType.TYPE_PICTURE,
                ModuleType.TYPE_CALENDAR, ModuleType.TYPE_MUSIC,
                ModuleType.TYPE_NOTEBOOK };
        int num = types.length;

        for (int i = 0; i < num; i++) {
            PersonalItemData item = new PersonalItemData(types[i], true);
            mBackupItemDataList.add(item);
        }

        mBackupListAdapter = new PersonalDataBackupAdapter(this,
                mBackupItemDataList, R.layout.backup_personal_data_item);
        //if(FeatureOption.MTK_CTA_SUPPORT) {
            updateUnCheckedStates(mBackupItemDataList,Constants.RESULT_PERSON_DATA);
        //}
        return mBackupListAdapter;
    }

    @Override
    public void onCheckedCountChanged() {
        super.onCheckedCountChanged();
        updateTitle();
    }

    private void showBackupResult(final BackupResultType result, final ArrayList<ResultEntity> list) {

        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }

        if (mCancelDlg != null && mCancelDlg.isShowing()) {
            mCancelDlg.dismiss();
        }

        if (result != BackupResultType.Cancel) {
            Bundle args = new Bundle();
            args.putParcelableArrayList(Constants.RESULT_KEY, list);
            showDialog(DialogID.DLG_RESULT, args);
        } else {
            stopService();
        }
    }

    private int getContactTypeNumber() {
        int count = (mSimCount + 1) < mContactCheckTypes.length ? (mSimCount + 1) : mContactCheckTypes.length;
        return count;
    }

    private boolean isAllValued(boolean[] array, int count, boolean value) {
        boolean ret = true;
        for (int position = 0; position < count; position++) {
            if (array[position] != value) {
                ret = false;
                break;
            }
        }
        return ret;
    }

    private void initContactConfig() {
        MyLogger.logD(CLASS_TAG, "initContactConfig ");
        mTemp = new boolean[mContactCheckTypes.length];
        SimInfoManager.SimInfoRecord mSIMInfo = null;
        for (int index = 0; index < mContactCheckTypes.length; index++) {
            mTemp[index] = mContactCheckTypes[index];
            MyLogger.logD(CLASS_TAG, "initContactConfig : mTemp[index] = " + mTemp[index]);
        }
        mContactItemDataList = new ArrayList<ContactItemData>();
        mContactItemData = new ContactItemData(-1, mTemp[0], getString(R.string.contact_phone), -1);
        mContactItemDataList.add(mContactItemData);
        if (mSimInfoList != null) {
            for (int i = 0; i < mSimInfoList.size(); i++) {
                SimInfoManager.SimInfoRecord simInfo = mSimInfoList.get(i);
                if (simInfo != null) {
                    mSIMInfo = SimInfoManager.getSimInfoBySlot(this, simInfo.mSimSlotId);
                }
                if (mSIMInfo != null && simInfo != null) {
                    mContactItemData = new ContactItemData(simInfo.mSimSlotId, mTemp[i + 1], simInfo.mDisplayName,
                            mSIMInfo.mColor);
                    MyLogger.logD(CLASS_TAG, "initContactConfig : mTemp[i + 1] = " + mTemp[i + 1] + "sim id  = "
                            + simInfo.mSimInfoId + ", name = " + simInfo.mDisplayName + ", slot = "
                            + simInfo.mSimSlotId);
                    mContactItemDataList.add(mContactItemData);
                }
            }
        }
        mSimInfoAdapter = new SimInfoAdapter(this, mContactItemDataList, mTemp);
        MyLogger.logD(CLASS_TAG, "initContactConfig mSimInfoAdapter = " + mSimInfoAdapter);
    }

/*    private void updateContactConfig() {
        MyLogger.logD(CLASS_TAG, "updateContactConfig :: ");
        boolean isChecked = false;
        if (mContactItemDataList != null) {
            mContactItemDataList.clear();
        }
        // mContactItemDataList = new ArrayList<ContactItemData>();
        if (isNeedShowSimCard(ContactType.PHONE)) {
            isChecked = true;
        }
        mContactItemData = new ContactItemData(-1, mContactCheckTypes[0], getString(R.string.contact_phone), -1,
                isNeedShowSimCard(ContactType.PHONE));
        mContactItemDataList.add(mContactItemData);
        MyLogger.logD(CLASS_TAG, "updateContactConfig :: 1 isChecked = " + isChecked + ", isNeedShowSimCard = "
                + isNeedShowSimCard(ContactType.PHONE) + ", mContactCheckTypes[0] = " + mContactCheckTypes[0]);
        if (mSimInfoList != null) {
            for (int i = 0; i < mSimInfoList.size(); i++) {
                SimInfoManager.SimInfoRecord simInfo = mSimInfoList.get(i);
                SimInfoManager.SimInfoRecord mSIMInfo = SimInfoManager.getSimInfoBySlot(this, simInfo.mSimSlotId);
                if (mSIMInfo != null && simInfo != null) {
                    if (isNeedShowSimCard(Long.toString(simInfo.mSimSlotId))) {
                        isChecked = true;
                    } else {
                        isChecked = false;
                    }
                    mContactItemData = new ContactItemData(simInfo.mSimSlotId, mContactCheckTypes[i + 1],
                            simInfo.mDisplayName, mSIMInfo.mColor, isNeedShowSimCard(Long.toString(simInfo.mSimSlotId)));
                    MyLogger.logD(CLASS_TAG, "updateContactConfig ::  2: isChecked = " + isChecked
                            + ",  isNeedShowSimCard = " + isNeedShowSimCard(Long.toString(simInfo.mSimSlotId))
                            + ", mContactCheckTypes[i + 1] == " + mContactCheckTypes[i + 1]);
                    mContactItemDataList.add(mContactItemData);
                }
            }
        }
        mSimInfoAdapter.updataData(mContactItemDataList);
    }*/

    private void showContactConfigDialog() {
        initContactConfig();
        alertDialog = new AlertDialog.Builder(this).setTitle(R.string.contact_module).setCancelable(false)
                .setAdapter(mSimInfoAdapter, null)
                .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        boolean empty = true;
                        for (int index = 0; index < mTemp.length; index++) {
                            mContactCheckTypes[index] = mTemp[index];
                        }
                        int count = getContactTypeNumber();
                        for (int index = 0; index < count; index++) {
                            if (mContactCheckTypes[index]) {
                                empty = false;
                                break;
                            }
                        }
                        if (empty) {
                            setItemCheckedByPosition(0, false);
                        } else {
                            setItemCheckedByPosition(0, true);
                        }

                        MyLogger.logD(CLASS_TAG, "positive2  mContactCheckTypes.length =  " + mContactCheckTypes.length);
                        // / M: save mContactCheckTypes for result activity
                        setContactCheckTypes(mContactCheckTypes);
                        if(mTemp != null) {
                            mTemp = null;
                        }
                    }
                }).setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        if(mTemp != null) {
                            mTemp = null;
                        }
                    }
                }).create();
        alertDialog.show();
        checkContactItem();
    }

    private boolean isNeedShowSimCard(String contactInfo) {
        if(contactInfo == null && mSimCountMap != null) {
            MyLogger.logD(CLASS_TAG, "W: SimInfoManager is error or the sim card is not already in");
            return false;
        }
        int contactsCount = mSimCountMap.get(contactInfo);
        if (contactsCount > 0) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;
        switch (id) {
        // input backup file name
        case DialogID.DLG_EDIT_FOLDER_NAME:
            dialog = createFolderEditorDialog();
            break;

        case DialogID.DLG_RESULT:
            final DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                @Override
                public void onClick(final DialogInterface dialog, final int which) {
                    stopService();
                }
            };
            dialog = ResultDialog.createResultDlg(this, R.string.backup_result, args, listener);
            break;

        case DialogID.DLG_BACKUP_CONFIRM_OVERWRITE:
            dialog = new AlertDialog.Builder(this).setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.notice).setMessage(R.string.backup_confirm_overwrite_notice)
                    .setNegativeButton(android.R.string.cancel, null)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            MyLogger.logI(CLASS_TAG, " to backup");
                            File folder = new File(mBackupFolder);
                            File[] files = folder.listFiles();
                            if (files != null && files.length > 0) {
                                DeleteFolderTask task = new DeleteFolderTask();
                                task.execute(files);
                            } else {
                                startPersonalDataBackup(mBackupFolder);
                            }
                        }
                    })
                    // .setCancelable(false)
                    .create();
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

        case DialogID.DLG_EDIT_FOLDER_NAME:
            EditText editor = (EditText) dialog.findViewById(R.id.edit_folder_name);
            if (editor != null) {
                SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
                String dateString = dateFormat.format(new Date(System.currentTimeMillis()));
                editor.setText(dateString);
            }
            break;
        default:
            super.onPrepareDialog(id, dialog, args);
            break;
        }
    }

    private AlertDialog createFolderEditorDialog() {
        LayoutInflater factory = LayoutInflater.from(this);
        final View view = factory.inflate(R.layout.dialog_edit_folder_name, null);
        EditText editor = (EditText) view.findViewById(R.id.edit_folder_name);

        final AlertDialog dialog = new AlertDialog.Builder(this).setTitle(R.string.edit_folder_name).setView(view)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        AlertDialog d = (AlertDialog) dialog;
                        EditText editor = (EditText) d.findViewById(R.id.edit_folder_name);
                        if (editor != null) {
                            String folderName = editor.getText().toString().trim();
                            String path = SDCardUtils.getPersonalDataBackupPath();
                            if (path == null) {
                                Toast.makeText(PersonalDataBackupActivity.this, R.string.sdcard_removed,
                                        Toast.LENGTH_SHORT).show();
                                return;
                            }

                            StringBuilder builder = new StringBuilder(path);
                            builder.append(File.separator);
                            builder.append(folderName);
                            MyLogger.logE(CLASS_TAG, "backup folder is " + builder.toString());
                            mBackupFolder = builder.toString();
                            File folder = new File(mBackupFolder);
                            File[] files = null;
                            if (folder.exists()) {
                                files = folder.listFiles();
                            }

                            if (files != null && files.length > 0) {
                                showDialog(DialogID.DLG_BACKUP_CONFIRM_OVERWRITE);
                            } else {
                                startPersonalDataBackup(mBackupFolder);
                            }
                        } else {
                            MyLogger.logE(CLASS_TAG, " can not get folder name");
                        }
                    }
                }).setNegativeButton(android.R.string.cancel, null).create();
        editor.addTextChangedListener(new TextWatcher() {

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                String inputName = s.toString().trim();
                if (inputName.length() <= 0 || inputName.matches(".*[/\\\\:*?\"<>|].*")) {
                    // characters not allowed
                    if (inputName.matches(".*[/\\\\:*?\"<>|].*")) {
                        Toast invalid = Toast.makeText(getApplicationContext(), R.string.invalid_char_prompt,
                                Toast.LENGTH_SHORT);
                        invalid.show();
                    }
                    Button botton = dialog.getButton(DialogInterface.BUTTON_POSITIVE);
                    if (botton != null) {
                        botton.setEnabled(false);
                    }
                } else {
                    Button botton = dialog.getButton(DialogInterface.BUTTON_POSITIVE);
                    if (botton != null) {
                        botton.setEnabled(true);
                    }
                }
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });
        return dialog;
    }

    protected String getProgressDlgMessage(final int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.backuping));
        builder.append("(");
        builder.append(ModuleType.getModuleStringFromType(this, type));
        builder.append(")");
        return builder.toString();
    }

    @Override
    protected void checkBackupState() {
        if (mBackupService != null) {
            int state = mBackupService.getState();
            switch (state) {
            case State.RUNNING:
                /* fall through */
            case State.PAUSE:
                BackupProgress p = mBackupService.getCurBackupProgress();
                Log.e(CLASS_TAG, "checkBackupState: Max = " + p.mMax + " curprogress = " + p.mCurNum);
                if (state == State.RUNNING) {
                    mProgressDialog.show();
                }
                if (p.mCurNum < p.mMax) {
                    String msg = getProgressDlgMessage(p.mType);
                    if (mProgressDialog != null) {
                        mProgressDialog.setMessage(msg);
                    }
                }
                if (mProgressDialog != null) {
                    mProgressDialog.setMax(p.mMax);
                    mProgressDialog.setProgress(p.mCurNum);
                }
                break;
            case State.FINISH:
                showBackupResult(mBackupService.getBackupResultType(), mBackupService.getBackupResult());
                break;
            default:
                super.checkBackupState();
                break;
            }
        }
    }

    @Override
    public void onConfigurationChanged(final Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e(CLASS_TAG, "onConfigurationChanged");
    }

    List<SimInfoManager.SimInfoRecord> mSimInfoList;
    int mSimCount = 0;

    private void getSimInfoList() {
        mSimInfoList = SimInfoManager.getInsertedSimInfoList(this);
        if (mSimInfoList != null) {
            for (SimInfoManager.SimInfoRecord simInfo : mSimInfoList) {
                MyLogger.logD(CLASS_TAG, "sim id  = " + simInfo.mSimInfoId + ", name = " + simInfo.mDisplayName
                        + ", slot = " + simInfo.mSimSlotId);
            }
        } else {
            MyLogger.logD(CLASS_TAG, "No SIM inserted!");
        }
        mSimCount = mSimInfoList.isEmpty() ? 0 : mSimInfoList.size();
        if (mSimInfoList.size() > 1) {
            Collections.sort(mSimInfoList, new Comparator<SimInfoManager.SimInfoRecord>() {
                public int compare(SimInfoManager.SimInfoRecord object1, SimInfoManager.SimInfoRecord object2) {
                    int left = object1.mSimSlotId;
                    int right = object2.mSimSlotId;
                    if (left < right) {
                        return -1;
                    } else {
                        return 1;
                    }
                }
            });
        }
        MyLogger.logD(CLASS_TAG, "SIM count = " + mSimCount);
    }

    private class PersonalDataBackupAdapter extends BaseAdapter {
        private ArrayList<PersonalItemData> mDataList;
        private int mLayoutId;
        private LayoutInflater mInflater;

        public PersonalDataBackupAdapter(Context context, ArrayList<PersonalItemData> list, int resource) {
            mDataList = list;
            mLayoutId = resource;
            mInflater = LayoutInflater.from(context);
        }

        public void changeData(ArrayList<PersonalItemData> list) {
            mDataList = list;
        }

        @Override
        public int getCount() {
            return mDataList.size();
        }

        @Override
        public Object getItem(final int position) {
            return mDataList.get(position);
        }

        @Override
        public long getItemId(final int position) {
            return mDataList.get(position).getType();
        }

        @Override
        public boolean isEnabled(int position) {
            if (mDataList == null) {
                return false;
            }
            final PersonalItemData item = mDataList.get(position);
            if (item != null) {
                if (item.getDataCount() > 0) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public View getView(final int position, final View convertView, final ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(mLayoutId, parent, false);
            }

            final PersonalItemData item = mDataList.get(position);
            final View content = view.findViewById(R.id.item_content);
            final View config = view.findViewById(R.id.item_config);
            final ImageView imgView = (ImageView) view.findViewById(R.id.item_image);
            final TextView textView = (TextView) view.findViewById(R.id.item_text);
            final CheckBox chxbox = (CheckBox) view.findViewById(R.id.item_checkbox);

            if (item.getType() == ModuleType.TYPE_CONTACT) {
                boolean isChecked = isItemCheckedByPosition(position);
                MyLogger.logD(CLASS_TAG, "contact config: positon + " + position + " is checked: " + isChecked);

                float alpha = isChecked ? ENABLE_ALPHA : DISABLE_ALPHA;
                config.setEnabled(isChecked);
                config.setAlpha(alpha);
                config.setVisibility(View.VISIBLE);
                config.setOnClickListener(new OnClickListener() {
                    public void onClick(View v) {
                        // contact config click
                        showContactConfigDialog();
                    }
                });
            } else {
                config.setVisibility(View.GONE);
                config.setOnClickListener(null);
            }
            
            content.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    if(chxbox.isEnabled()) {
                        revertItemCheckedByPosition(position);
                    }
                }
            });
            
            imgView.setImageResource(item.getIconId());
            textView.setText(item.getTextId());
            if(isItemCheckedByPosition(position)) {
                if(chxbox.isEnabled()) {
                    chxbox.setChecked(true);
                }
            } else {
                if (chxbox.isEnabled()) {
                    chxbox.setChecked(false);
                }
            }
            return view;
        }
    }

    public class PersonalDataBackupStatusListener extends NomalBackupStatusListener {
        @Override
        public void onBackupEnd(final BackupResultType resultCode, final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {
            final BackupResultType iResultCode = resultCode;
            final ArrayList<ResultEntity> iResultRecord = resultRecord;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        showBackupResult(iResultCode, iResultRecord);
                    }
                });
            }
        }

        @Override
        public void onComposerChanged(final Composer composer) {
            if (composer == null) {
                MyLogger.logE(CLASS_TAG, "onComposerChanged: error[composer is null]");
            }
            MyLogger.logI(CLASS_TAG,
                    "onComposerChanged: type = " + composer.getModuleType() + "Max = " + composer.getCount());

            final int count = composer.getCount();
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        String msg = getProgressDlgMessage(composer.getModuleType());
                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                            mProgressDialog.setMax(count);
                            mProgressDialog.setProgress(0);
                        }
                    }
                });
            }
        }
    }

    private class DeleteFolderTask extends AsyncTask<File[], String, Long> {
        private ProgressDialog mDeletingDialog;

        public DeleteFolderTask() {
            mDeletingDialog = new ProgressDialog(PersonalDataBackupActivity.this);
            mDeletingDialog.setCancelable(false);
            mDeletingDialog.setMessage(getString(R.string.delete_please_wait));
            mDeletingDialog.setIndeterminate(true);
        }

        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            if (mBackupFolder != null) {
                startPersonalDataBackup(mBackupFolder);
            }

            if (mDeletingDialog != null) {
                mDeletingDialog.dismiss();
            }
        }

        protected void onPreExecute() {
            if (mDeletingDialog != null) {
                mDeletingDialog.show();
            }
        }

        protected Long doInBackground(File[]... params) {
            File[] files = params[0];
            for (File file : files) {
                FileUtils.deleteFileOrFolder(file);
            }

            return null;
        }
    }

    /**
     * add for UI modify,check items counts
     */
/*    private class InitPersonData extends AsyncTask<Void, Void, Long> {

        private boolean mIsCanceled = false;
        private boolean mIsRunning = false;
        private boolean mDissmiss = false;
        private Context mContext;
        ArrayList<PersonalItemData> mInitPersonDataList;

        public InitPersonData(Context context) {
            mContext = context;
        }

        public void setCancel() {
            mIsCanceled = true;
            MyLogger.logD(CLASS_TAG, "InitPersonData : set cancel");
        }

        public boolean isRunning() {
            return mIsRunning;
        }
        
        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            synchronized(sLock) {
                if (!mIsCanceled && arg0 != -1) {
                    setButtonsEnable(true);
                    updatePersonData(mInitPersonDataList);
                    setProgressBarIndeterminateVisibility(false);
                    if (alertDialog != null && alertDialog.isShowing()) {
                        // mSimInfoAdapter = (SimInfoAdapter)
                        // alertDialog.getListView().getAdapter();
                            checkContactItem();
                        if(mDissmiss) {
                            alertDialog.dismiss();
                            MyLogger.logD(CLASS_TAG, "alertDialog.dismiss because contacts count is 0 !");
                        } else {
                            mSimInfoAdapter.notifyDataSetChanged();
                            MyLogger.logD(CLASS_TAG, "update mSimInfoAdapter");
                        }
                    }
                } else {
                    mInitPersonDataList.clear();
                    updatePersonData(mInitPersonDataList);
                    if(alertDialog != null && alertDialog.isShowing() && mDissmiss) {
                        alertDialog.dismiss();
                        MyLogger.logD(CLASS_TAG, "alertDialog.dismiss because here is no data !");
                    }
                    setEmptyViewInfo(R.string.no_data);
                    MyLogger.logD(CLASS_TAG, "no data and update UI");
                }
            }
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            // show progress and set title as "updating"
            synchronized (sLock) {
                if (!mIsCanceled) {
                    setProgressBarIndeterminateVisibility(true);
                    setTitle(R.string.updating);
                    if(mBackupItemDataList != null) {
                        MyLogger.logD(CLASS_TAG, "clear mBackupListAdapter");
                        mBackupListAdapter.changeData(new ArrayList());
                        mBackupListAdapter.notifyDataSetChanged();
                    }
                    setButtonsEnable(false);
                }
            }
        }

        @Override
        protected Long doInBackground(Void... params) {
            synchronized (sLock) {
                mInitPersonDataList = new ArrayList<PersonalItemData>();
                mComposerList = new ArrayList<Composer>();
                int types[] = new int[] { ModuleType.TYPE_CONTACT, ModuleType.TYPE_MESSAGE, ModuleType.TYPE_PICTURE,
                        ModuleType.TYPE_CALENDAR, ModuleType.TYPE_MUSIC, ModuleType.TYPE_NOTEBOOK };
                int num = types.length;
                if (setupComposer(types, mContext)) {
                    int contactCount = 0;
                    for (int i = 0; i < num; i++) {
                        int count = 0;
                        if (!mIsCanceled) {
                            if (mComposerList.get(i).getModuleType() == ModuleType.TYPE_CONTACT) {
                                mSimCountMap = new HashMap<String, Integer>();
                                    ArrayList<String> initContacts = getInitContacts();
                                    if(mContactCheckTypes == null){
                                        mContactCheckTypes = new boolean[initContacts.size()];
                                        for(int contactIndex = 0; contactIndex < mContactCheckTypes.length; contactIndex++) {
                                            mContactCheckTypes[contactIndex] = true;
                                        }
                                    }
                                int index = 0;
                                for (String mContacts : initContacts) {
                                    mComposerList.get(i).setParams(Arrays.asList(mContacts));
                                    mComposerList.get(i).init();
                                    count += mComposerList.get(i).getCount();
                                    initContactCheckTypes(index, mComposerList.get(i).getCount());
                                    index++;
                                    mComposerList.get(i).onEnd();
                                    mSimCountMap.put(mContacts, mComposerList.get(i).getCount());
                                    contactCount = count;
                                }
                                if(mOldContactCheckTypes != null) {
                                    mOldContactCheckTypes = null;
                                }
                            } else {
                                mComposerList.get(i).init();
                                count = mComposerList.get(i).getCount();
                                mComposerList.get(i).onEnd();
                            }
                            PersonalItemData item = new PersonalItemData(types[i], true, count);
                            mInitPersonDataList.add(item);
                        }
                    }
                    if (contactCount != 0 && mSimInfoAdapter != null && alertDialog != null && alertDialog.isShowing()) {
                        updateContactConfig();
                    } else {
                        mDissmiss = true;
                    }
                }
                if (getItemDataCount(mInitPersonDataList) == 0) {
                    mResule = -1;
                } else {
                    mResule = 0;
                }
                return mResule;
            }
        }

    }

    private void initContactCheckTypes(int index, int count) {
        Log.i(CLASS_TAG, "initContactCheckTypes : mContactCheckTypes index = " + index + " count = " + count);
        if(count > 0) {
            if(mOldContactCheckTypes != null && index < mOldContactCheckTypes.length) {
                mContactCheckTypes[index] = mOldContactCheckTypes[index];
            }
            //if the contact dialog is show, but onPause. General,the mTemp is null.
            if(mTemp != null &&  index < mTemp.length) {
                Log.i(CLASS_TAG, "initContactCheckTypes: mTemp != null! so the contact dialog interrupt by others" +
                " activity");
                mContactCheckTypes[index] = mTemp[index];
            }
        } else {
            mContactCheckTypes[index] = false;
        }
    }

    private ArrayList<String> getInitContacts() {
        getSimInfoList();
        ArrayList<String> mparams = new ArrayList<String>();
        mparams.add(ContactType.PHONE);
        if (mSimCount >= 1) {
            mparams.add(Long.toString(mSimInfoList.get(0).mSimSlotId));
        }
        if (mSimCount >= 2) {
            mparams.add(Long.toString(mSimInfoList.get(1).mSimSlotId));
        }
        return mparams;
    }

    private int getItemDataCount(List<PersonalItemData> dataList) {
        int mIndex = 0;
        for (PersonalItemData mPersonalItemData : dataList) {
            if (mPersonalItemData.getDataCount() != 0) {
                mIndex++;
            }
        }
        return mIndex;
    }
*/
/*    *//**
     * @param mInitPersonData
     *            update display info
     *//*
    private void updatePersonData(ArrayList<PersonalItemData> list) {
        Log.i(CLASS_TAG, "updatePersonData  is running !");
        if (mInitPersonData == null) {
            Log.w(CLASS_TAG, "updatePersonData  is null !!! !");
            return;
        }
        mBackupItemDataList = list;
        mBackupListAdapter.changeData(list);
        syncUnCheckedItems();
        mBackupListAdapter.notifyDataSetChanged();
        updateTitle();
        updateButtonState();
        Log.i(CLASS_TAG, "data is initialed, to checkBackupState");
    }*/

/*    private boolean setupComposer(int[] types, Context context) {
        boolean result = true;
        for (int type : types) {
            switch (type) {
            case ModuleType.TYPE_CONTACT:
                mComposerList.add(new ContactBackupComposer(context));
                break;
            case ModuleType.TYPE_CALENDAR:
                mComposerList.add(new CalendarBackupComposer(context));
                break;
            case ModuleType.TYPE_SMS:
                mComposerList.add(new SmsBackupComposer(context));
                break;
            case ModuleType.TYPE_MMS:
                mComposerList.add(new MmsBackupComposer(context));
                break;
            case ModuleType.TYPE_MESSAGE:
                mComposerList.add(new MessageBackupComposer(context));
                break;
            case ModuleType.TYPE_APP:
                mComposerList.add(new AppBackupComposer(context));
                break;
            case ModuleType.TYPE_PICTURE:
                mComposerList.add(new PictureBackupComposer(context));
                break;
            case ModuleType.TYPE_MUSIC:
                mComposerList.add(new MusicBackupComposer(context));
                break;
            case ModuleType.TYPE_NOTEBOOK:
                mComposerList.add(new NoteBookBackupComposer(context));
                break;
            default:
                result = false;
                break;
            }
        }
        return result;
    }
*/
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * add for Contact card dialog display
     */
    private class SimInfoAdapter extends BaseAdapter {
        private LayoutInflater mLayoutInflater = null;
        private ArrayList<ContactItemData> mData = null;
        private String[] mSelectInfo = null;
        private boolean[] mCheckStatus = null;
        private HashMap<Integer, Boolean> mChecked = null;

        SimInfoAdapter(Context context) {
        }

        public SimInfoAdapter(Context context, ArrayList<ContactItemData> list, boolean[] mTemp) {
            mLayoutInflater = LayoutInflater.from(context);
            this.mData = list;
            this.mCheckStatus = mTemp;
        }

        public void updataData(ArrayList<ContactItemData> list) {
            this.mData = list;
        }

        public int getCount() {
            return mData.size();
        }

        public Object getItem(int position) {
            return mData.get(position);
        }

        public long getItemId(int position) {
            return mData.get(position).getSimId();
        }

        public View getView(final int position, View convertView, ViewGroup parent) {
            ViewSimInfo mViewSimInfo = null;
            if (mData == null) {
                MyLogger.logD(CLASS_TAG, "warning no data !!");
                return null;
            }
            if (convertView == null) {
                mViewSimInfo = new ViewSimInfo();
                convertView = mLayoutInflater.inflate(R.layout.dialog_contacts_import, null);
                mViewSimInfo.mImg = (ImageView) convertView.findViewById(R.id.icon);
                mViewSimInfo.mSimName = (TextView) convertView.findViewById(R.id.simName);
                mViewSimInfo.mCheckbox = (CheckBox) convertView.findViewById(R.id.checkbox);
                convertView.setTag(mViewSimInfo);
            } else {
                mViewSimInfo = (ViewSimInfo) convertView.getTag();
            }
            mViewSimInfo.mImg.setBackgroundResource(mData.get(position).getIconId());
            mViewSimInfo.mSimName.setText(mData.get(position).getmContactName());
            mViewSimInfo.mCheckbox.setChecked(mData.get(position).isChecked());

            convertView.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    MyLogger.logD(CLASS_TAG, "DialogID.DLG_CONTACT_CONFIG: click position === " + position);
                    updateCheckStatus(position);
                }
            });

            return convertView;
        }
    }

    private void updateUnCheckedStatus(int position) {
        MyLogger.logD(CLASS_TAG, "updateUnCheckedStatus : false position = "+ position);
        mTemp[position] = false;
    }
    
    public void updateCheckStatus(final int position) {
        int count = mSimCount + 1;

        mContactItemData = mContactItemDataList.get(position);
        if (mContactItemData.isChecked()) {
            mContactItemData.setChecked(false);
            mTemp[position] = false;
        } else {
            mContactItemData.setChecked(true);
            mTemp[position] = true;
        }
        boolean isAllUnChecked = true;
        mSimInfoAdapter.notifyDataSetChanged();
        for (ContactItemData mItemData : mContactItemDataList) {
            if (mItemData.isChecked()) {
                MyLogger.logD(CLASS_TAG, "mItemData.isChecked == true");
                isAllUnChecked = false;
                break;
            }
        }
        if (isAllUnChecked) {
            alertDialog.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(false);
        } else {
            alertDialog.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(true);
        }
        MyLogger.logD(CLASS_TAG, "DialogID.DLG_CONTACT_CONFIG: updateCheckStatus ");

    }
    
    public void checkContactItem() {
        int index = 0;
        boolean isAllUnChecked = true;
        if(mContactItemDataList != null && mContactItemDataList.size()> 0 ) {
            for (ContactItemData mItemData : mContactItemDataList) {
                if (mItemData.isChecked()) {
                    index++;
                }
            }
            if (index == 0) {
                alertDialog.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(false);
                MyLogger.logD(CLASS_TAG, "checkContactItem setEnabled false ");
            } 
        }
    }

    private class ViewSimInfo {
        private ImageView mImg;
        private TextView mSimName;
        private CheckBox mCheckbox;
    }

    public void OnUnCheckedChanged() {
        super.OnUnCheckedChanged();
        MyLogger.logD(CLASS_TAG, "OnUnCheckedChanged and updateTitle");
        updateTitle();
    }

}
