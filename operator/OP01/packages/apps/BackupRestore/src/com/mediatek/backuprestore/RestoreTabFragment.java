package com.mediatek.backuprestore;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.backuprestore.SDCardReceiver.OnSDCardStatusChangedListener;
import com.mediatek.backuprestore.utils.BackupFilePreview;
import com.mediatek.backuprestore.utils.BackupFileScanner;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.MessageID;
import com.mediatek.backuprestore.utils.FileUtils;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.OldBackupFilePreview;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

public class RestoreTabFragment extends PreferenceFragment {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreTabFragment";
    private static final int START_ACTION_MODE_DELAY_TIME = 500;
    private static final String STATE_DELETE_MODE = "deleteMode";
    private static final String STATE_CHECKED_ITEMS = "checkedItems";

    private ListView mListView;
    private BackupFileScanner mFileScanner;
    private Handler mHandler;
    private ProgressDialog mLoadingDialog;
    private ActionMode mDeleteActionMode;
    private DeleteActionMode mActionModeListener;
    OnSDCardStatusChangedListener mSDCardListener;
    private boolean mIsActive = false;
    TextView mEmptyView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(CLASS_TAG, "RestoreTabFragment: onCreate");
        addPreferencesFromResource(R.xml.restore_tab_preference);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        Log.i(CLASS_TAG, "RestoreTabFragment: onAttach");
    }

    @Override
    public void onActivityCreated(final Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.i(CLASS_TAG, "RestoreTabFragment: onActivityCreated");
        init();
        if (savedInstanceState != null) {
            boolean isActionMode = savedInstanceState.getBoolean(STATE_DELETE_MODE, false);
            if (isActionMode) {
                mHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        mDeleteActionMode = getActivity().startActionMode(mActionModeListener);
                        mActionModeListener.restoreState(savedInstanceState);
                    }
                }, START_ACTION_MODE_DELAY_TIME);
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(CLASS_TAG, "RestoreTabFragment: onDestroy");
        if (mFileScanner != null) {
            mFileScanner.setHandler(null);
        }
        unRegisteSDCardListener();
    }

    public void onPause() {
        super.onPause();
        Log.i(CLASS_TAG, "RestoreTabFragment: onPasue");
        if (mFileScanner != null) {
            mFileScanner.quitScan();
        }
        if (mLoadingDialog != null) {
            mLoadingDialog.cancel();
            MyLogger.logV(CLASS_TAG,"mFileScanner is canle and mLoadingDialog need dismiss");
        }
        mIsActive = false;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "RestoreTabFragment: onResume");
        mIsActive = true;
        // refresh
        if (SDCardUtils.isSdCardAvailable()) {
            startScanFiles();
        } else {
            mEmptyView.setText(R.string.no_data);
            mEmptyView.setVisibility(View.VISIBLE);
            PreferenceScreen ps = getPreferenceScreen();
            ps.removeAll();
            if(mActionModeListener != null && mDeleteActionMode != null){
                Log.i(CLASS_TAG, "RestoreTabFragment: onResume and mDeleteActionMode need finish");
                mDeleteActionMode.finish();
            }
            Log.i(CLASS_TAG, "RestoreTabFragment: onResume and no data test = " + mEmptyView.getText());
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        Log.i(CLASS_TAG, "RestoreTabFragment: onDetach");
    }

    public void onSaveInstanceState(final Bundle outState) {
        super.onSaveInstanceState(outState);
        if (mDeleteActionMode != null) {
            outState.putBoolean(STATE_DELETE_MODE, true);
            mActionModeListener.saveState(outState);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference instanceof RestoreCheckBoxPreference) {
            RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
            if (mDeleteActionMode == null) {
                Intent intent = p.getIntent();
                String fileName = intent.getStringExtra(Constants.FILENAME);
                File file = new File(fileName);
                if (file.exists()) {
                    startActivity(intent);
                } else {
                    Toast.makeText(getActivity(), R.string.file_no_exist_and_update, Toast.LENGTH_SHORT);
                }
            } else if (mActionModeListener != null) {
                mActionModeListener.setItemChecked(p, !p.isChecked());
            }
        }
        return true;
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.i(CLASS_TAG, "RestoreTabFragment: onStart");
    }

    private void init() {
        initHandler();
        initListView(getView());
        initLoadingDialog();
        registerSDCardListener();
    }

    private void unRegisteSDCardListener() {
        if (mSDCardListener != null) {
            SDCardReceiver receiver = SDCardReceiver.getInstance();
            receiver.unRegisterOnSDCardChangedListener(mSDCardListener);
        }
    }

    private void registerSDCardListener() {
        mSDCardListener = new OnSDCardStatusChangedListener() {
            @Override
            public void onSDCardStatusChanged(final boolean mount) {
                if (mIsActive) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mIsActive) {
                                if (mount) {
                                    startScanFiles();
                                }
                                int resId = mount ? R.string.sdcard_swap_insert : R.string.sdcard_swap_remove;
                                Toast.makeText(getActivity(), resId, Toast.LENGTH_SHORT).show();
                            }
                            if (!mount) {
                                PreferenceScreen ps = getPreferenceScreen();
                                ps.removeAll();
                                if(mActionModeListener != null && mDeleteActionMode != null){
                                    Log.i(CLASS_TAG, "RestoreTabFragment: sdCard Umount and mDeleteActionMode need finish");
                                    mDeleteActionMode.finish();
                                }
                            }
                        }
                    });
                }
            }
        };

        SDCardReceiver receiver = SDCardReceiver.getInstance();
        receiver.registerOnSDCardChangedListener(mSDCardListener);
    }

    private void initLoadingDialog() {
        mLoadingDialog = new ProgressDialog(getActivity());
        mLoadingDialog.setCancelable(false);
        mLoadingDialog.setMessage(getString(R.string.loading_please_wait));
        mLoadingDialog.setIndeterminate(true);
    }

    private void initListView(View root) {
        View view = root.findViewById(android.R.id.list);
        if (view != null && view instanceof ListView) {
            mListView = (ListView) view;
            //mEmptyView.setVisibility(View.GONE);
            Log.i(CLASS_TAG, "RestoreTabFragment: setEmptyView");
            mEmptyView = (TextView) getView().findViewById(android.R.id.empty);
            mEmptyView.setTextAppearance(getActivity(), android.R.attr.textAppearanceMedium);
            mListView.setEmptyView(mEmptyView);
            mActionModeListener = new DeleteActionMode();
            mListView.setOnItemLongClickListener(new OnItemLongClickListener() {
                @Override
                public boolean onItemLongClick(AdapterView<?> listView, View view, int position, long id) {
                    mDeleteActionMode = getActivity().startActionMode(mActionModeListener);
                    showCheckBox(true);
                    mActionModeListener.onPreferenceItemClick(getPreferenceScreen(), position);
                    return true;
                }
            });
        }
    }

    @SuppressWarnings("unchecked")
    private void startDeleteItems(final HashSet<String> deleteItemIds) {
        PreferenceScreen ps = getPreferenceScreen();
        int count = ps.getPreferenceCount();
        HashSet<File> files = new HashSet<File>();
        for (int position = 0; position < count; position++) {
            Preference preference = ps.getPreference(position);
            if (preference != null && preference instanceof RestoreCheckBoxPreference) {
                RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                String key = p.getKey();
                if (deleteItemIds.contains(key)) {
                    files.add(p.getAccociateFile());
                }
            }
        }
        DeleteCheckItemTask deleteTask = new DeleteCheckItemTask();
        deleteTask.execute(files);
    }

    private void initHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(final Message msg) {
                switch (msg.what) {

                case MessageID.SCANNER_FINISH:
                    if (getActivity() != null) {
                        if (SDCardUtils.getStoragePath() != null && msg.obj != null) {
                            addScanResultsAsPreferences(msg.obj);
                        } else {
                            PreferenceScreen ps = getPreferenceScreen();
                            // clear the old items last scan
                            ps.removeAll();
                            MyLogger.logV(CLASS_TAG,
                                    "SCANNER_FINISH and data is null and PreferenceScreen clear PreferenceCount = "
                                            + ps.getPreferenceCount());
                            mEmptyView.setText(R.string.no_data);
                            mEmptyView.setVisibility(View.VISIBLE);
                        }
                    }
                    if (mLoadingDialog != null) {
                        mLoadingDialog.cancel();
                        MyLogger.logV(CLASS_TAG,"hanlde msg FINISH --- mLoadingDialog cancel");
                    }
                    break;

                default:
                    break;
                }
            }

        };
    }
    
    private boolean isNoData(String storagePath) {
        // TODO Auto-generated method stub
        return false;
    }

    private void startScanFiles() {
        if (!mIsActive) {
            MyLogger.logV(CLASS_TAG, "no need to scan files as mIsActive is false");
            return;
        }
        if (mEmptyView != null) {
            mEmptyView.setVisibility(View.GONE);
        }
        if (!mLoadingDialog.isShowing()) {
            mLoadingDialog.show();
        }
        if (mFileScanner == null) {
            mFileScanner = new BackupFileScanner(getActivity(), mHandler);
        } else {
            mFileScanner.setHandler(mHandler);
        }
        if (!mFileScanner.isScanning()) {
            MyLogger.logD(CLASS_TAG, "RestoreTabFragment: startScanFiles");
            mFileScanner.startScan();
        } else {
            MyLogger.logD(CLASS_TAG, "don't need to startScanFiles mFileScanner is running");
        }
    }

    @SuppressWarnings("unchecked")
    private void addScanResultsAsPreferences(Object obj) {

        PreferenceScreen ps = getPreferenceScreen();

        // clear the old items last scan
        ps.removeAll();

        HashMap<String, List<?>> map = (HashMap<String, List<?>> )obj;

        // personal data
        List<BackupFilePreview> items = (List<BackupFilePreview>) map.get(Constants.SCAN_RESULT_KEY_PERSONAL_DATA);
        if (items != null && !items.isEmpty()) {
            addPreferenceCategory(ps, R.string.backup_personal_data_history);
            for (BackupFilePreview item : items) {
                addRestoreCheckBoxPreference(ps, item, "personal data");
            }
        }
        // old backup data
        List<OldBackupFilePreview> itemsOlds = (List<OldBackupFilePreview>) map.get(Constants.SCAN_RESULT_KEY_OLD_DATA);
        if (itemsOlds != null && !itemsOlds.isEmpty()) {
            // addPreferenceCategory(ps, R.string.backup_personal_data_history);
            for (OldBackupFilePreview itemsOld : itemsOlds) {
                addRestoreCheckBoxPreference(ps, itemsOld, "old data");
                MyLogger.logD(CLASS_TAG, "addScanResultsAsPreferences: old data having add");
            }
        }

        // app data
        items = (List<BackupFilePreview>) map.get(Constants.SCAN_RESULT_KEY_APP_DATA);
        if (items != null && !items.isEmpty()) {
            addPreferenceCategory(ps, R.string.backup_app_data_history);
            for (BackupFilePreview item : items) {
                addRestoreCheckBoxPreference(ps, item, "app");
            }
        }

        if (mDeleteActionMode != null && mActionModeListener != null) {
            mActionModeListener.confirmSyncCheckedPositons();
        }
    }

    private void addPreferenceCategory(PreferenceScreen ps, int titleID) {
        PreferenceCategory category = new PreferenceCategory(getActivity());
        category.setTitle(titleID);
        ps.addPreference(category);
    }

    private <T> void addRestoreCheckBoxPreference(PreferenceScreen ps, T items, String type) {
        long size = 0;
        String fileName = null;
        File file = null;
        String path = null;
        if (items == null || type == null) {
            MyLogger.logE(CLASS_TAG, "addRestoreCheckBoxPreference: Error!");
            return;
        }
        RestoreCheckBoxPreference preference = new RestoreCheckBoxPreference(getActivity());
        if (type.equals("app")) {
            preference.setTitle(R.string.backup_app_data_preference_title);
            BackupFilePreview item = (BackupFilePreview) items;
            size = item.getFileSize();
            file = item.getFile();
            path = item.getFile().getAbsolutePath();
        } else if (type.equals("personal data")) {
            BackupFilePreview item = (BackupFilePreview) items;
            fileName = item.getFileName();
            preference.setTitle(fileName);
            size = item.getFileSize();
            file = item.getFile();
            path = item.getFile().getAbsolutePath();
        } else if (type.equals("old data")) {
            OldBackupFilePreview item = (OldBackupFilePreview) items;
            fileName = item.getFileName();
            int index = fileName.lastIndexOf(".");
            fileName = fileName.substring(0, index);
            preference.setTitle(fileName);
            size = item.getFileSize();
            file = item.getFile();
            path = item.getFile().getAbsolutePath();
        }
        MyLogger.logI(CLASS_TAG, "addRestoreCheckBoxPreference: type is " + type + " fileName = " + fileName);
        StringBuilder builder = new StringBuilder(getString(R.string.backup_data));
        builder.append(" ");
        builder.append(FileUtils.getDisplaySize(size, getActivity()));
        preference.setSummary(builder.toString());
        if (mDeleteActionMode != null) {
            preference.showCheckbox(true);
        }
        preference.setAccociateFile(file);

        Intent intent = new Intent();
        if (type.equals("app")) {
            intent.setClass(getActivity(), AppRestoreActivity.class);
        } else {
            intent.setClass(getActivity(), PersonalDataRestoreActivity.class);
        }
        intent.putExtra(Constants.FILENAME, path);
        preference.setIntent(intent);
        ps.addPreference(preference);
        mEmptyView.setVisibility(View.GONE);
    }

    private void showCheckBox(boolean bShow) {
        PreferenceScreen ps = getPreferenceScreen();
        int count = ps.getPreferenceCount();
        for (int position = 0; position < count; position++) {
            Preference p = ps.getPreference(position);
            if (p instanceof RestoreCheckBoxPreference) {
                ((RestoreCheckBoxPreference) p).showCheckbox(bShow);
            }
        }
    }

    class DeleteActionMode implements ActionMode.Callback {

        private int mCheckedCount;
        private HashSet<String> mCheckedItemIds;
        private ActionMode mMode;

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.select_all:
                setAllItemChecked(true);
                break;

            case R.id.cancel_select:
                setAllItemChecked(false);
                break;

            case R.id.delete:
                if (mCheckedCount == 0) {
                    Toast.makeText(getActivity(), R.string.no_item_selected, Toast.LENGTH_SHORT).show();
                } else {
                    startDeleteItems(mCheckedItemIds);
                    mode.finish();
                }
                break;

            default:
                break;
            }
            return false;
        }

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            mMode = mode;
            mListView.setLongClickable(false);
            MenuInflater inflater = getActivity().getMenuInflater();
            inflater.inflate(R.menu.multi_select_menu, menu);
            mCheckedItemIds = new HashSet<String>();
            setAllItemChecked(false);
            showCheckBox(true);
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            mCheckedItemIds = null;
            mCheckedCount = 0;
            mDeleteActionMode = null;
            mListView.setLongClickable(true);
            showCheckBox(false);
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return false;
        }

        private void updateTitle() {
            StringBuilder builder = new StringBuilder();
            builder.append(mCheckedCount);
            builder.append(" ");
            builder.append(getString(R.string.selected));
            mMode.setTitle(builder.toString());
        }

        public void onPreferenceItemClick(PreferenceScreen ps, final int position) {
            Preference preference = ps.getPreference(position);
            if (preference instanceof RestoreCheckBoxPreference) {
                RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                boolean toChecked = !p.isChecked();
                p.setChecked(toChecked);
                String key = p.getAccociateFile().getAbsolutePath();
                if (toChecked) {
                    mCheckedItemIds.add(key);
                    mCheckedCount++;
                } else {
                    mCheckedItemIds.remove(key);
                    mCheckedCount--;
                }
                updateTitle();
            }
        }

        public void setItemChecked(final RestoreCheckBoxPreference p, final boolean checked) {
            if (p.isChecked() != checked) {
                p.setChecked(checked);
                String key = p.getKey();
                if (checked) {
                    mCheckedItemIds.add(key);
                    mCheckedCount++;
                } else {
                    mCheckedItemIds.remove(key);
                    mCheckedCount--;
                }
            }
            updateTitle();
        }

        private void setAllItemChecked(boolean checked) {
            PreferenceScreen ps = getPreferenceScreen();

            mCheckedCount = 0;
            mCheckedItemIds.clear();
            int count = ps.getPreferenceCount();
            for (int position = 0; position < count; position++) {
                Preference preference = ps.getPreference(position);
                if (preference instanceof RestoreCheckBoxPreference) {
                    RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                    p.setChecked(checked);
                    if (checked) {
                        mCheckedItemIds.add(p.getAccociateFile().getAbsolutePath());
                        mCheckedCount++;
                    }
                }
            }
            updateTitle();
        }

        /**
         * after refreshed, must sync witch mCheckedItemIds;
         */
        public void confirmSyncCheckedPositons() {
            mCheckedCount = 0;

            HashSet<String> tempCheckedIds = new HashSet<String>();
            PreferenceScreen ps = getPreferenceScreen();
            int count = ps.getPreferenceCount();
            for (int position = 0; position < count; position++) {
                Preference preference = ps.getPreference(position);
                if (preference instanceof RestoreCheckBoxPreference) {
                    RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                    String key = p.getAccociateFile().getAbsolutePath();
                    if (mCheckedItemIds.contains(key)) {
                        tempCheckedIds.add(key);
                        p.setChecked(true);
                        mCheckedCount++;
                    }
                }
            }
            mCheckedItemIds.clear();
            mCheckedItemIds = tempCheckedIds;
            updateTitle();
        }

        public void saveState(final Bundle outState) {
            ArrayList<String> list = new ArrayList<String>();
            for (String item : mCheckedItemIds) {
                list.add(item);
            }
            outState.putStringArrayList(STATE_CHECKED_ITEMS, list);
        }

        public void restoreState(Bundle state) {
            ArrayList<String> list = state.getStringArrayList(STATE_CHECKED_ITEMS);
            if (list != null && !list.isEmpty()) {
                for (String item : list) {
                    mCheckedItemIds.add(item);
                }
            }
            PreferenceScreen ps = getPreferenceScreen();
            if (ps.getPreferenceCount() > 0) {
                confirmSyncCheckedPositons();
            }
        }
    }

    private class DeleteCheckItemTask extends AsyncTask<HashSet<File>, String, Long> {

        private ProgressDialog mDeletingDialog;

        public DeleteCheckItemTask() {
            mDeletingDialog = new ProgressDialog(getActivity());
            mDeletingDialog.setCancelable(false);
            mDeletingDialog.setMessage(getString(R.string.delete_please_wait));
            mDeletingDialog.setIndeterminate(true);
        }

        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            startScanFiles();
            Activity activity = getActivity();
            if (activity != null && mDeletingDialog != null) {
                mDeletingDialog.dismiss();
            }
        }

        @Override
        protected void onPreExecute() {
            Activity activity = getActivity();
            if (activity != null && mDeletingDialog != null) {
                mDeletingDialog.show();
            }
        }

        @Override
        protected Long doInBackground(HashSet<File>... params) {
            HashSet<File> deleteFiles = params[0];
            for (File file : deleteFiles) {
                FileUtils.deleteFileOrFolder(file);
            }
            return null;
        }
    }
}
