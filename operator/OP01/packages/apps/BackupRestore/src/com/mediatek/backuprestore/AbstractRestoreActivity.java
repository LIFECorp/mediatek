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
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListAdapter;
import android.widget.RadioButton;
import android.widget.SimpleAdapter;

import com.mediatek.backuprestore.CheckedListActivity.OnCheckedCountChangedListener;
import com.mediatek.backuprestore.RestoreService.OnRestoreStatusListener;
import com.mediatek.backuprestore.RestoreService.RestoreBinder;
import com.mediatek.backuprestore.ResultDialog.ResultEntity;
import com.mediatek.backuprestore.SDCardReceiver.OnSDCardStatusChangedListener;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.utils.Constants.DialogID;
import com.mediatek.backuprestore.utils.Constants.State;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.SDCardUtils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public abstract class AbstractRestoreActivity extends CheckedListActivity implements OnCheckedCountChangedListener {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AbstractRestoreActivity";

    private static final String SELECT_INDEX = "index";
    private static final String TITLE = "title";
    private static final String INFO = "info";
    protected ArrayList<String> mUnCheckedList = new ArrayList<String>();
    protected Handler mHandler;
    BaseAdapter mAdapter;
    private Button mBtRestore = null;
    private Button mBtSelect = null;
    private ProgressDialog mProgressDialog;
    protected RestoreBinder mRestoreService;
    OnRestoreStatusListener mRestoreListener;
    OnSDCardStatusChangedListener mSDCardListener;
    // / M: add for notice dialog (import/replace)
    private List<HashMap<String, String>> mListHashMap;
    private HashMap<String, String> mMap;
    private SimpleAdapter mSimpleAdapter;
    private RadioButton mRadioButton = null;
    private int mSelectedIndex = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        super.onCreate(savedInstanceState);
        if (savedInstanceState != null) {
            restoreIndex(savedInstanceState);
        }
        setProgressBarIndeterminateVisibility(false);
        setContentView(R.layout.restore);
        init();

        /*
         * bind Restore Service when activity onCreate, and unBind Service when
         * activity onDestroy
         */
        this.bindService();
        MyLogger.logI(CLASS_TAG, " onCreate");
    }

    private void restoreIndex(Bundle savedInstanceState) {
        mSelectedIndex = savedInstanceState.getInt(SELECT_INDEX);
    }

    // / M: add for notice dialog (import/replace)
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(SELECT_INDEX, mSelectedIndex);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        MyLogger.logI(CLASS_TAG, " onDestroy");
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
        unRegisteSDCardListener();

        // when startService when to Restore and stopService when onDestroy if
        // the service in IDLE
        if (mRestoreService != null && mRestoreService.getState() == State.INIT) {
            this.stopService();
        }

        // set listener to null avoid some special case when restart after
        // configure changed
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(null);
        }
        this.unBindService();
        mHandler = null;
    }

    @Override
    public void onPause() {
        super.onPause();
        MyLogger.logI(CLASS_TAG, " onPasue");
    }

    @Override
    public void onStop() {
        super.onStop();
        MyLogger.logI(CLASS_TAG, " onStop");
    }

    @Override
    protected void onStart() {
        MyLogger.logI(CLASS_TAG, " onStart");
        super.onStart();
    }

    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        MyLogger.logI(CLASS_TAG, " onConfigurationChanged");
    }

    @Override
    public void onCheckedCountChanged() {
        mAdapter.notifyDataSetChanged();
        updateButtonState();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU && event.isLongPress()) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void init() {
        registerOnCheckedCountChangedListener(this);
        registerSDCardListener();
        initButton();
        initHandler();
        initDetailList();
        createProgressDlg();
    }

    protected Dialog onCreateDialog(int id) {
        return onCreateDialog(id, null);
    }

    protected Dialog onCreateDialog(int id, Bundle args) {
        Dialog dialog = null;
        MyLogger.logI(CLASS_TAG, " oncreateDialog, id = " + id);
        switch (id) {

        case DialogID.DLG_RESTORE_CONFIRM:
            dialog = new AlertDialog.Builder(this).setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.notice)
                    .setSingleChoiceItems(getAdapter(), mSelectedIndex, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            mSelectedIndex = which;
                            mSimpleAdapter.notifyDataSetChanged();
                        }
                    }).setNegativeButton(android.R.string.cancel, null)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            MyLogger.logI(CLASS_TAG, " to Restore" + mSelectedIndex);
                            startRestore(mSelectedIndex);
                        }
                    }).setCancelable(false).create();
            break;

        case DialogID.DLG_SDCARD_REMOVED:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.error).setIcon(android.R.drawable.ic_dialog_alert)
                    .setMessage(R.string.sdcard_removed)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            if (mRestoreService != null) {
                                mRestoreService.reset();
                            }
                            stopService();
                            AbstractRestoreActivity.this.finish();
                        }
                    }).setCancelable(false).create();
            break;

        case DialogID.DLG_SDCARD_FULL:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.error).setIcon(android.R.drawable.ic_dialog_alert)
                    .setMessage(R.string.sdcard_is_full)
                    .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            if (mRestoreService != null) {
                                mRestoreService.cancelRestore();
                                mRestoreService.reset();
                            }
                        }
                    }).setCancelable(false).create();
            break;

        default:
            break;
        }
        return dialog;
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
            public void onSDCardStatusChanged(boolean mount) {
                if (!mount) {
                    mRestoreService.cancelRestore();
                    mRestoreService.reset();
                    AbstractRestoreActivity.this.finish();
                }
            }
        };

        SDCardReceiver receiver = SDCardReceiver.getInstance();
        receiver.registerOnSDCardChangedListener(mSDCardListener);
    }

    private void initHandler() {
        mHandler = new Handler();
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

    private void resetSelectButton() {
        mHandler.postDelayed(new Runnable() {

            @Override
            public void run() {
                mBtSelect.setOnClickListener(mOnClickSelect);
                setButtonStatus(true);
            }
        }, 500);
    }

    private void initButton() {
        mBtSelect = (Button) findViewById(R.id.restore_bt_select);
        mBtSelect.setOnClickListener(mOnClickSelect);

        mBtRestore = (Button) findViewById(R.id.restore_bt_restore);
        mBtRestore.setOnClickListener(new Button.OnClickListener() {

            public void onClick(View v) {
                mBtSelect.setOnClickListener(null);
                MyLogger.logI(CLASS_TAG, "getCheckedCount() == " + getCheckedCount());
                if (getCheckedCount() > 0) {
                    setButtonStatus(false);
                    showDialogModule(DialogID.DLG_RESTORE_CONFIRM);
                }
                resetSelectButton();
            }
        });
    }

    protected void updateButtonState() {
        if (getCount() == 0) {
            setButtonsEnable(false);
            return;
        }

        if (isAllChecked(false)) {
            mBtRestore.setEnabled(false);
            mBtSelect.setText(R.string.selectall);
        } else {
            mBtRestore.setEnabled(true);
            if (isAllChecked(true)) {
                mBtSelect.setText(R.string.unselectall);
            } else {
                mBtSelect.setText(R.string.selectall);
            }
        }
    }

    protected void setButtonsEnable(boolean enabled) {
        if (mBtSelect != null) {
            mBtSelect.setEnabled(enabled);
        }
        if (mBtRestore != null) {
            mBtRestore.setEnabled(enabled);
        }
    }

    private void initDetailList() {
        mAdapter = initAdapter();
        setListAdapter(mAdapter);
        this.updateButtonState();
    }

    protected abstract BaseAdapter initAdapter();

    protected void notifyListItemCheckedChanged() {
        if (mAdapter != null) {
            mAdapter.notifyDataSetChanged();
        }
        updateButtonState();
    }

    protected ProgressDialog createProgressDlg() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setMessage(getString(R.string.restoring));
            mProgressDialog.setCancelable(false);
        }
        return mProgressDialog;
    }

    protected void showProgressDialog() {
        if (mProgressDialog != null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.show();
    }

    protected void setProgressDialogMax(int max) {
        if (mProgressDialog != null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setMax(max);
    }

    protected void setProgressDialogProgress(int value) {
        if (mProgressDialog != null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setProgress(value);
    }

    protected void setProgressDialogMessage(CharSequence message) {
        if (mProgressDialog != null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setMessage(message);
    }

    protected boolean isProgressDialogShowing() {
        if (mProgressDialog != null) {
            return mProgressDialog.isShowing();
        }

        return false;
    }

    protected void dismissProgressDialog() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }

    protected boolean isCanStartRestore() {
        if (mRestoreService == null) {
            MyLogger.logE(CLASS_TAG, " isCanStartRestore : mRestoreService is null");
            return false;
        }

        if (mRestoreService.getState() != State.INIT) {
            MyLogger.logE(CLASS_TAG, " isCanStartRestore :Can not to start Restore. Restore Service is ruuning");
            return false;
        }
        return true;
    }

    protected boolean errChecked() {
        boolean ret = false;
        String path = SDCardUtils.getStoragePath();
        if (path == null) {
            // no sdcard
            ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        showDialog(DialogID.DLG_SDCARD_REMOVED, null);
                    }
                });
            }
        } else if (SDCardUtils.getAvailableSize(path) <= 512) {
            // no space
            // ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        showDialog(DialogID.DLG_SDCARD_FULL, null);
                    }
                });
            }
        }
        return ret;
    }

    public void setOnRestoreStatusListener(OnRestoreStatusListener listener) {
        mRestoreListener = listener;
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(mRestoreListener);
        }
    }

    protected abstract void afterServiceConnected();

    protected abstract void startRestore(int command);

    protected abstract void startRestore();

    protected void showDialogModule(int dialogId) {
        showDialog(dialogId);
    }

    private void bindService() {
        getApplicationContext().bindService(new Intent(this, RestoreService.class), mServiceCon,
                Service.BIND_AUTO_CREATE);
    }

    private void unBindService() {
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(null);
        }
        getApplicationContext().unbindService(mServiceCon);
    }

    protected void startService() {
        startService(new Intent(this, RestoreService.class));
    }

    protected void stopService() {
        if (mRestoreService != null) {
            mRestoreService.reset();
        }
        stopService(new Intent(this, RestoreService.class));
    }

    ServiceConnection mServiceCon = new ServiceConnection() {

        public void onServiceConnected(ComponentName name, IBinder service) {
            mRestoreService = (RestoreBinder) service;
            if (mRestoreService != null) {
                mRestoreService.setOnRestoreChangedListner(mRestoreListener);
                afterServiceConnected();
            }
            MyLogger.logI(CLASS_TAG, " onServiceConnected");
        }

        public void onServiceDisconnected(ComponentName name) {
            mRestoreService = null;
            MyLogger.logI(CLASS_TAG, " onServiceDisconnected");
        }
    };

    public class NormalRestoreStatusListener implements OnRestoreStatusListener {

        public void onComposerChanged(final int type, final int max) {
            Log.i(CLASS_TAG, "onComposerChanged");
        }

        public void onProgressChanged(Composer composer, final int progress) {
            Log.i(CLASS_TAG, "onProgressChange, p = " + progress);
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                        }
                    }
                });
            }
        }

        public void onRestoreEnd(boolean bSuccess, final ArrayList<ResultEntity> resultRecord) {
            Log.i(CLASS_TAG, "onRestoreEnd");
        }

        public void onRestoreErr(IOException e) {
            Log.i(CLASS_TAG, "onRestoreErr");
            if (errChecked()) {
                if (mRestoreService != null && mRestoreService.getState() != State.INIT
                        && mRestoreService.getState() != State.FINISH) {
                    mRestoreService.pauseRestore();
                }
            } else {
                if (mRestoreService != null) {
                    mRestoreService.cancelRestore();
                    mRestoreService.reset();
                }
            }
        }
    }

    // / M: add for notice dialog (import/replace)
    private List<HashMap<String, String>> getDate() {
        mListHashMap = new ArrayList<HashMap<String, String>>();
        mMap = new HashMap<String, String>();
        mMap.put(TITLE, this.getResources().getString(R.string.import_data));
        mMap.put(INFO, this.getResources().getString(R.string.import_info));

        mListHashMap.add(mMap);

        mMap = new HashMap<String, String>();
        mMap.put(TITLE, this.getResources().getString(R.string.replace_data));
        mMap.put(INFO, this.getResources().getString(R.string.replace_info));
        mListHashMap.add(mMap);
        return mListHashMap;
    }

    // / M: add for notice dialog (import/replace)
    private ListAdapter getAdapter() {
        mSimpleAdapter = new SimpleAdapter(this, getDate(), R.layout.dialog_restore, new String[] { TITLE, INFO },
                new int[] { R.id.title, R.id.summary }) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                // TODO Auto-generated method stub
                View view = super.getView(position, convertView, parent);
                mRadioButton = (RadioButton) view.findViewById(R.id.radiobutton);
                mRadioButton.setOnClickListener(null);
                mRadioButton.setClickable(false);
                if (mSelectedIndex == position) {
                    mRadioButton.setChecked(true);
                } else {
                    mRadioButton.setChecked(false);
                }
                return view;
            }
        };
        return mSimpleAdapter;
    }
}
