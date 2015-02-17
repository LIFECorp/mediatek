package com.mediatek.contacts.plugin;

import java.util.HashMap;
import java.util.List;
import java.util.Collections;
import java.util.Comparator;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.app.AlertDialog;
import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.Context;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import android.view.MenuItem;
import android.view.Menu;

import com.mediatek.op01.plugin.R;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;

public class OP01ContactListExtension extends ContactListExtension {
    private static final String TAG = "OP01ContactListExtension";
    private Context mContext = null;
    private static Context mContextHost = null;
    private static final int MENU_SIM_STORAGE = 9999;   
    
    @Override
    public String getCommand() {
        return ContactPluginDefault.COMMD_FOR_OP01;
    }
    
    @Override
    public void setMenuItem(MenuItem blockVoiceCallmenu, boolean mOptionsMenuOptions, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return ;
        }
        Log.i(TAG, "[setMenuItem]");
        blockVoiceCallmenu.setVisible(false);
       
    }
    
    @Override
    public void registerHostContext(Context context, Bundle args, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return ;
        }
        
        mContextHost = context;
        try {
            mContext = context.createPackageContext("com.mediatek.op01.plugin", Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch(NameNotFoundException e) {
            Log.d(TAG, "no com.mediatek.op01.plugin packages");
        }
    }

    @Override
    public void addOptionsMenu(Menu menu, Bundle args, String commd) {
        Log.i(TAG, "addOptionsMenu"); 
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return ;
        }

        MenuItem item = menu.findItem(MENU_SIM_STORAGE);
        List<SimInfoRecord> simInfos = SimInfoManager.getInsertedSimInfoList(mContext);
        if (item == null && simInfos != null && simInfos.size() > 0) {
            String string = mContext.getResources().getString(R.string.look_simstorage);
            menu.add(0, MENU_SIM_STORAGE, 0, string).setOnMenuItemClickListener(
                new MenuItem.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        ShowSimCardStorageInfoTask.showSimCardStorageInfo(mContext);
                        return true;
                    }
            });
        }
    }

    @Override
    public void setLookSimStorageMenuVisible(MenuItem lookSimStorageMenu, boolean flag, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return ;
        }
        Log.i(TAG, "PeopleActivity: [setLookSimStorageMenuVisible()]"); 
        if (flag) {
            lookSimStorageMenu.setVisible(true);
        } else {
            lookSimStorageMenu.setVisible(false);
        }
    }

    @Override
    public String getReplaceString(final String src, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return null;
        }
        Log.i(TAG, "AbstractStartSIMService: [getReplaceString()]");
        return src.replace(PhoneNumberUtils.PAUSE, 'p').replace(PhoneNumberUtils.WAIT, 'w');
    }

    public static class ShowSimCardStorageInfoTask extends AsyncTask<Void, Void, Void> {
        private static ShowSimCardStorageInfoTask sInstance = null;
        private boolean mIsCancelled = false;
        private boolean mIsException = false;
        private String mDlgContent = null;
        private Context mContext = null;
        private static HashMap<Integer, Integer> sSurplugMap = new HashMap<Integer, Integer>();

        public static void showSimCardStorageInfo(Context context) {
            Log.i(TAG, "[ShowSimCardStorageInfoTask]_beg");
            if (sInstance != null) {
                sInstance.cancel();
                sInstance = null;
            }
            sInstance = new ShowSimCardStorageInfoTask(context);
            sInstance.execute();
            Log.i(TAG, "[ShowSimCardStorageInfoTask]_end");
        }

        public ShowSimCardStorageInfoTask(Context context) {
            mContext = context;
            Log.i(TAG, "[ShowSimCardStorageInfoTask] onCreate()");
        }

        @Override
        protected Void doInBackground(Void... args) {
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: doInBackground_beg");
            sSurplugMap.clear();
            List<SimInfoRecord> simInfos = getSortedInsertedSimInfoList(SimInfoManager.getInsertedSimInfoList(mContext));
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: simInfos.size = " + simInfos.size());
            if (!mIsCancelled && (simInfos != null) && simInfos.size() > 0) {
                StringBuilder build = new StringBuilder();
                int simId = 0;
                for (SimInfoRecord simInfo : simInfos) {
                    if (simId > 0) {
                        build.append("\n\n");
                    }
                    simId++;
                    int[] storageInfos = null;
                    Log.i(TAG, "[ShowSimCardStorageInfoTask] simName = " + simInfo.mDisplayName
                            + "; simSlot = " + simInfo.mSimSlotId + "; simId = " + simInfo.mSimInfoId);
                    build.append(simInfo.mDisplayName);
                    build.append(":\n");
                    try {
                        ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(ServiceManager
                              .checkService("phoneEx"));
                        if (!mIsCancelled && phoneEx != null) {
                            storageInfos = phoneEx.getAdnStorageInfo(simInfo.mSimSlotId);
                            if (storageInfos == null) {
                                mIsException = true;
                                Log.i(TAG, " storageInfos is null");
                                return null;
                            }
                            Log.i(TAG, "[ShowSimCardStorageInfoTask] infos: "
                                    + storageInfos.toString());
                        } else {
                            Log.i(TAG, "[ShowSimCardStorageInfoTask]: phone = null");
                            mIsException = true;
                            return null;
                        }
                    } catch (RemoteException ex) {
                        Log.i(TAG, "[ShowSimCardStorageInfoTask]_exception: " + ex);
                        mIsException = true;
                        return null;
                    }
                    Log.i(TAG, "slotId:" + simInfo.mSimSlotId + "||storage:"
                            + (storageInfos == null ? "NULL" : storageInfos[1]) + "||used:"
                            + (storageInfos == null ? "NULL" : storageInfos[0]));
                    if (storageInfos != null && storageInfos[1] > 0) {
                        sSurplugMap.put(simInfo.mSimSlotId, storageInfos[1] - storageInfos[0]);
                    }
                    build.append(mContext.getResources().getString(R.string.dlg_simstorage_content,
                            storageInfos[1], storageInfos[0]));
                    if (mIsCancelled) {
                        return null;
                    }
                }
                mDlgContent = build.toString();
            }
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: doInBackground_end");
            return null;
        }

        public void cancel() {
            super.cancel(true);
            mIsCancelled = true;
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: mIsCancelled = true");
        }

        @Override
        protected void onPostExecute(Void v) {
            if (mContextHost instanceof Activity) {
                Log.i(TAG, "[onPostExecute]: activity find");
                Activity activity = (Activity) mContextHost;
                if (activity.isFinishing()) {
                    Log.i(TAG, "[onPostExecute]: activity finish");
                    return;
                }
            }
        
            Drawable icon = mContext.getResources().getDrawable(R.drawable.ic_menu_look_simstorage_holo_light);
            String string = mContext.getResources().getString(R.string.look_simstorage);
            sInstance = null;
            if (!mIsCancelled && !mIsException) {
                new AlertDialog.Builder(mContextHost).setIcon(icon).setTitle(string).setMessage(mDlgContent).setPositiveButton(
                       android.R.string.ok, null).setCancelable(true).create().show();
            }
            mIsCancelled = false;
            mIsException = false;
        }

        public List<SimInfoRecord> getSortedInsertedSimInfoList(List<SimInfoRecord> ls) {
            Collections.sort(ls, new Comparator<SimInfoRecord>() {
                @Override
                public int compare(SimInfoRecord arg0, SimInfoRecord arg1) {
                    return (arg0.mSimSlotId - arg1.mSimSlotId);
                }
            });
            return ls;
        }
    }

    @Override
    public int getMultiChoiceLimitCount(String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            Log.i(TAG, "[getMultiChoiceLimitCount] commd: " + commd);
            return 1000;
        }
        Log.i(TAG, "[getMultiChoiceLimitCount]");
        return 5000;
    }
}
