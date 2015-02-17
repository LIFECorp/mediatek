package com.mediatek.backuprestore;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.backuprestore.BackupEngine.BackupResultType;
import com.mediatek.backuprestore.BackupService.BackupProgress;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.modules.AppBackupComposer;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.DialogID;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.BackupRestoreNotification;
import com.mediatek.backuprestore.utils.ModuleType;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class AppBackupActivity extends AbstractBackupActivity {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AppBackupActivity";
    private List<AppSnippet> mData = new ArrayList<AppSnippet>();
    private List<AppSnippet> mOriginData = new ArrayList<AppSnippet>();
    private AppBackupAdapter mAdapter;
    private InitDataTask mInitDataTask;
    private boolean mIsDataInitialed = false;
    private AppBackupStatusListener mBackupStatusListener = new AppBackupStatusListener();
    private boolean mIsCheckedBackupStatus = false;
    private Bundle mSettingData;
    private static SharedPreferences sSharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        super.onCreate(savedInstanceState);
        setProgressBarIndeterminateVisibility(false);
        Log.v(CLASS_TAG, "onCreate");
        setRequestCode(Constants.RESULT_APP_DATA);
        if (savedInstanceState != null) {
            mSettingData = savedInstanceState.getBundle(Constants.DATA_TITLE);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        // update
        if (mInitDataTask == null || !mInitDataTask.isRunning()) {
            mInitDataTask = new InitDataTask(this);
            mInitDataTask.execute();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mInitDataTask != null) {
            mInitDataTask.setCancel();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuItem m = menu.add(Menu.NONE, Menu.FIRST + 1, 0, getResources().getString(R.string.settings));
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case Menu.FIRST + 1:
            Intent intent = new Intent(this, SettingsAppActivity.class);
            if (mSettingData != null) {
                intent.putExtras(mSettingData);
            } else {
                sSharedPreferences = getInstance(this);
                mSettingData = new Bundle();
                mSettingData.putString(Constants.DATE, sSharedPreferences.getString((Constants.DATA_TITLE),null));
                intent.putExtras(mSettingData);
            }
            
            startActivityForResult(intent, 0);
            Log.v(CLASS_TAG, "onOptionsItemSelected startSettings from menu mSettingData = "+mSettingData.getString(Constants.DATE));
            break;
        default:
            break;
        }
        return false;
    }

    @Override
    public BaseAdapter initBackupAdapter() {
        mAdapter = new AppBackupAdapter(this, mData, R.layout.app_item);
        return mAdapter;
    }

    @Override
    public void startBackup() {
        Log.v(CLASS_TAG, "startBackup");
        mOriginData = mData;
        startService();
        if (mBackupService != null) {
            ArrayList<Integer> backupList = new ArrayList<Integer>();
            backupList.add(ModuleType.TYPE_APP);
            mBackupService.setBackupModelList(backupList);

            boolean needBackupData = getSettingInfo();
            Log.v(CLASS_TAG, "~~~~ startBackup needBackupData = " + needBackupData);
            ArrayList<String> list = getSelectedPackageNameList();
            mBackupService.setBackupItemParam(ModuleType.TYPE_APP, list);
            mBackupService.setBackupAppData(needBackupData);
            String appPath = SDCardUtils.getAppsBackupPath();
            MyLogger.logD(CLASS_TAG, "backup path is: " + appPath);
            boolean ret = mBackupService.startBackup(appPath);
            if (ret) {
                showProgress();
                mProgressDialog.setProgress(0);
                mProgressDialog.setMax(list.size());
                String packageName = list.get(0);
                AppSnippet appSnippet = getAppSnippetByPackageName(packageName);
                String msg = formatProgressDialogMsg(appSnippet);
                mProgressDialog.setMessage(msg);
            } else {
                showDialog(DialogID.DLG_SDCARD_FULL);
                stopService();
            }
        }
    }

    private boolean getSettingInfo() {
        if (mSettingData != null) {
            String title = mSettingData.getString(Constants.DATE);
            Log.v(CLASS_TAG, "~~ getSettingInfo title = " + title);
            if (title.equals(Constants.APP_AND_DATA)) {
                return true;
            }
        } else {
            sSharedPreferences = getInstance(this);
            if (sSharedPreferences.getString(Constants.DATA_TITLE, null).equals(Constants.APP_AND_DATA)) {
                return true;
            }
        }
        return false;
    }

    protected void afterServiceConnected() {
        MyLogger.logD(CLASS_TAG, "afterServiceConnected, to checkBackupState");
        checkBackupState();
    }

    private ArrayList<String> getSelectedPackageNameList() {
        ArrayList<String> list = new ArrayList<String>();
        int count = mAdapter.getCount();
        for (int position = 0; position < count; position++) {
            AppSnippet item = (AppSnippet) getItemByPosition(position);
            if (isItemCheckedByPosition(position)) {
                list.add(item.getPackageName());
            }
        }
        return list;
    }

    @Override
    public void onCheckedCountChanged() {
        super.onCheckedCountChanged();
        updateTitle();
    }

    private void updateData(ArrayList<AppSnippet> list) {
        if (list == null) {
            MyLogger.logE(CLASS_TAG, "updateData, list is null");
            return;
        }
        mData = list;
        mAdapter.changeData(list);
        syncUnCheckedItems();
        mAdapter.notifyDataSetChanged();
        updateTitle();
        updateButtonState();
        mIsDataInitialed = true;
        MyLogger.logD(CLASS_TAG, "data is initialed, to checkBackupState");
        checkBackupState();
    }

    private AppSnippet getAppSnippetByPackageName(String packageName) {

        AppSnippet result = null;
        for (AppSnippet item : mData) {
            if (item.getPackageName().equalsIgnoreCase(packageName)) {
                result = item;
                break;
            }
        }
        return result;
    }

    private String formatProgressDialogMsg(AppSnippet item) {
        StringBuilder builder = new StringBuilder(getString(R.string.backuping));
        if (item != null) {
            builder.append("(").append(item.getName()).append(")");
        }
        return builder.toString();
    }

    public void updateTitle() {
        StringBuilder sb = new StringBuilder();
        sb.append(getString(R.string.backup_app));
        int totalNum = mAdapter.getCount();
        int selectNum = this.getSelectedPackageNameList().size();
        sb.append("(" + selectNum + "/" + totalNum + ")");
        this.setTitle(sb.toString());
    }

    @Override
    protected void checkBackupState() {
        if (mIsCheckedBackupStatus) {
            MyLogger.logD(CLASS_TAG, "can not checkBackupState, as it has been checked");
            return;
        }
        if (!mIsDataInitialed) {
            MyLogger.logD(CLASS_TAG, "can not checkBackupState, wait data to initialed");
            return;
        }
        MyLogger.logD(CLASS_TAG, "to checkBackupState");
        mIsCheckedBackupStatus = true;
        if (mBackupService != null) {
            int state = mBackupService.getState();
            MyLogger.logD(CLASS_TAG, "checkBackupState: state = " + state);
            switch (state) {
            case State.RUNNING:
            case State.PAUSE:
                ArrayList<String> params = mBackupService.getBackupItemParam(ModuleType.TYPE_APP);
                BackupProgress p = mBackupService.getCurBackupProgress();
                Log.e(CLASS_TAG, CLASS_TAG + "checkBackupState: Max = " + p.mMax + " curprogress = " + p.mCurNum);

                if (state == State.RUNNING) {
                    mProgressDialog.show();
                }
                if (p.mCurNum < p.mMax) {
                    String packageName = params.get(p.mCurNum);
                    String msg = formatProgressDialogMsg(getAppSnippetByPackageName(packageName));
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
                showBackupResult(mBackupService.getBackupResultType(), mBackupService.getAppBackupResult());
                break;
            default:
                super.checkBackupState();
                break;
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;
        switch (id) {
        case DialogID.DLG_RESULT:
            final DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(final DialogInterface dialog, final int which) {
                    stopService();
                }
            };
            dialog = ResultDialog.createResultDlg(this, R.string.backup_result, args, listener);
            break;

        case DialogID.DLG_LOADING:
            ProgressDialog progressDlg = new ProgressDialog(this);
            progressDlg.setCancelable(false);
            progressDlg.setMessage(getString(R.string.loading_please_wait));
            progressDlg.setIndeterminate(true);
            dialog = progressDlg;
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
                ListAdapter adapter = ResultDialog.createAppResultAdapter(mData, this, args,
                        ResultDialog.RESULT_TYPE_BACKUP);
                view.setAdapter(adapter);
            }
            break;
        default:
            super.onPrepareDialog(id, dialog, args);
            break;
        }
    }

    protected void showBackupResult(final BackupResultType result, final ArrayList<ResultEntity> appResultRecord) {

        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }

        if (mCancelDlg != null && mCancelDlg.isShowing()) {
            mCancelDlg.dismiss();
        }

        if (result != BackupResultType.Cancel) {
            Bundle args = new Bundle();
            args.putParcelableArrayList("result", appResultRecord);
            ListAdapter adapter = ResultDialog.createAppResultAdapter(mOriginData, this, args,
                    ResultDialog.RESULT_TYPE_BACKUP);
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(false).setTitle(R.string.backup_result)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                        public void onClick(final DialogInterface dialog, final int which) {
                            if (mBackupService != null) {
                                mBackupService.reset();
                            }
                            stopService();
                        }
                    }).setAdapter(adapter, null).create();
            dialog.show();
        } else {
            stopService();
        }
    }

    private class InitDataTask extends AsyncTask<Void, Void, Long> {

        List<ApplicationInfo> mAppInfoList;
        ArrayList<AppSnippet> mAppDataList;
        private boolean mIsCanceled = false;
        private boolean mIsRunning = false;
        private boolean mIsSetting = false;
        private Context mContext = null;

        public InitDataTask(Context context) {
            mContext = context;
        }

        public void setCancel() {
            mIsCanceled = true;
            MyLogger.logD(CLASS_TAG, "FilePreviewTask: set cancel");
        }

        public boolean isRunning() {
            return mIsRunning;
        }

        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            if (!mIsCanceled) {
                setButtonsEnable(true);
                updateUnCheckedStates(mAppDataList, Constants.RESULT_APP_DATA);
                updateData(mAppDataList);
                setOnBackupStatusListener(mBackupStatusListener);
                setProgressBarIndeterminateVisibility(false);
            }

            if (mIsSetting) {
                Intent intent = new Intent(mContext, SettingsAppActivity.class);
                if (mSettingData != null) {
                    intent.putExtras(mSettingData);
                }
                startActivityForResult(intent, 0);
            }

        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            // show progress and set title as "updating"
            if (!mIsCanceled) {
                setProgressBarIndeterminateVisibility(true);
                setTitle(R.string.updating);
                setButtonsEnable(false);
            }
        }

        @Override
        protected Long doInBackground(Void... arg0) {
            mAppInfoList = AppBackupComposer.getUserAppInfoList(AppBackupActivity.this);
            PackageManager pm = getPackageManager();
            mAppDataList = new ArrayList<AppSnippet>();
            for (ApplicationInfo info : mAppInfoList) {
                if (!mIsCanceled) {
                    Drawable icon = info.loadIcon(pm);
                    CharSequence name = info.loadLabel(pm);
                    AppSnippet snippet = new AppSnippet(icon, name, info.packageName);
                    mAppDataList.add(snippet);
                }
            }
            if (!mIsCanceled) {
                Collections.sort(mAppDataList, new Comparator<AppSnippet>() {
                    public int compare(AppSnippet object1, AppSnippet object2) {
                        String left = new StringBuilder(object1.getName()).toString();
                        String right = new StringBuilder(object2.getName()).toString();
                        if (left != null && right != null) {
                            return left.compareTo(right);
                        }
                        return 0;
                    }
                });
            }
            sSharedPreferences = getInstance(mContext);
            Log.d(CLASS_TAG,
                    "~~ doInBackground  ~mSettingData == " + mSettingData
                            + ", sSharedPreferences.getString(Constants.DATA_TITLE, null) = "
                            + sSharedPreferences.getString(Constants.DATA_TITLE, null));
            if (sSharedPreferences.getString(Constants.DATA_TITLE, null) == null && mSettingData == null) {
                mIsSetting = true;
                Log.d(CLASS_TAG, "~~ doInBackground  ~mIsSetting == " + mIsSetting);
            }
            return null;
        }
    }

    private class AppBackupAdapter extends BaseAdapter {

        private List<AppSnippet> mList;
        private int mLayoutId;
        private LayoutInflater mInflater;

        public AppBackupAdapter(Context context, List<AppSnippet> list, int resource) {
            mList = list;
            mLayoutId = resource;
            mInflater = LayoutInflater.from(context);
        }

        public void changeData(List<AppSnippet> list) {
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
            return mList.get(position).getPackageName().hashCode();
        }

        public View getView(final int position, View convertView, ViewGroup parent) {
            if (mList == null) {
                return null;
            }
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(mLayoutId, parent, false);
            }
            final AppSnippet item = mList.get(position);
            ImageView imgView = (ImageView) view.findViewById(R.id.item_image);
            TextView textView = (TextView) view.findViewById(R.id.item_text);
            CheckBox checkbox = (CheckBox) view.findViewById(R.id.item_checkbox);
            imgView.setBackgroundDrawable(item.getIcon());
            textView.setText(item.getName());
            checkbox.setChecked(isItemCheckedByPosition(position));
            return view;
        }
    }

    private class AppBackupStatusListener extends NomalBackupStatusListener {

        @Override
        public void onComposerChanged(final Composer composer) {
            if (composer == null) {
                MyLogger.logE(CLASS_TAG,
                        "onComposerChasetProgressBarIndeterminateVisibility(false);nged: error[composer is null]");
            }
            MyLogger.logI(CLASS_TAG,
                    "onComposerChanged: type = " + composer.getModuleType() + "Max = " + composer.getCount());
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        ArrayList<String> params = mBackupService.getBackupItemParam(ModuleType.TYPE_APP);
                        if (params != null && !params.isEmpty()) {
                            String packageName = params.get(0);
                            MyLogger.logV(CLASS_TAG, "onComposerChanged, first packageName is " + packageName);
                            String msg = formatProgressDialogMsg(getAppSnippetByPackageName(packageName));
                            if (mProgressDialog != null) {
                                mProgressDialog.setMessage(msg);
                                mProgressDialog.setMax(composer.getCount());
                                mProgressDialog.setProgress(0);
                            }
                        }
                    }
                });
            }
        }

        @Override
        public void onProgressChanged(final Composer composer, final int progress) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                            if (progress < composer.getCount()) {
                                ArrayList<String> params = mBackupService.getBackupItemParam(ModuleType.TYPE_APP);
                                String packageName = params.get(progress);
                                MyLogger.logV(CLASS_TAG, "onComposerChanged: the " + progress + "  packageName is "
                                        + packageName);
                                String msg = formatProgressDialogMsg(getAppSnippetByPackageName(packageName));
                                if (mProgressDialog != null) {
                                    mProgressDialog.setMessage(msg);
                                }
                            }
                        }
                    }
                });
            }
        }

        @Override
        public void onBackupEnd(final BackupResultType resultCode, final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        showBackupResult(resultCode, appResultRecord);
                    }
                });
            }
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.i(CLASS_TAG, "onActivityResult requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = "
                + data);
        if (resultCode == Activity.RESULT_OK && data != null) {
            mSettingData = data.getExtras();
            Log.i(CLASS_TAG, "onActivityResult mSettingData = " + mSettingData);
            if(mSettingData != null){
                Log.i(CLASS_TAG, "onActivityResult mSettingData = " + mSettingData.getString(Constants.DATE));
            }
        } else {
            Log.w(CLASS_TAG, "Intent data is null !!!");
        }

    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        if (mSettingData != null) {
            outState.putBundle(Constants.DATA_TITLE, mSettingData);
        }
    }

    public static SharedPreferences getInstance(Context context) {
        if (sSharedPreferences == null) {
            sSharedPreferences = context.getSharedPreferences(Constants.SETTINGINFO, Activity.MODE_PRIVATE);
        }
        return sSharedPreferences;
    }
}
