package com.mediatek.settings.plugin;

import android.content.Context;
import android.content.Intent;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceGroup;
import android.preference.Preference.OnPreferenceChangeListener;
import android.provider.Settings.System;

import com.mediatek.op01.plugin.R;
import com.mediatek.settings.ext.DefaultMdmPermControlExt;
import com.mediatek.xlog.Xlog;

public class CMCCMdmPermControlExt extends DefaultMdmPermControlExt {
    private static final String TAG = "CMCCMdmPermControlExt";
    private static final int DM_BOOT_START_ENABLE_FALG = 1;
    private static final int DM_BOOT_START_DISABLE_FALG = 0;
    private static final int DEF_DM_BOOT_START_ENABLE_VALUE = 1;
    private CheckBoxPreference mPreference;
    private final Context mContext;
    public CMCCMdmPermControlExt(Context context) {
        super(context);
        // TODO Auto-generated constructor stub
        Xlog.i(TAG, "CMCCMdmPermControlExt...");
        mContext = context;
        //initial CheckBoxPreference
        mPreference = new CheckBoxPreference(context);
        mPreference.setEnabled(true);
        mPreference.setTitle(R.string.cmcc_dm_settings_title);
        mPreference.setChecked(getMdmPermCtrlSwitchFromDB());
        //initial summary
        if(getMdmPermCtrlSwitchFromDB()) {
            mPreference.setSummary(R.string.cmcc_dm_settings_enable_summary);
        } else {
            mPreference.setSummary(R.string.cmcc_dm_settings_disable_summary);
        }
        //add check listener
        mPreference.setOnPreferenceChangeListener(mPreferenceChangeListener);
    }
    public void addMdmPermCtrlPrf(PreferenceGroup prefGroup) {
        Xlog.i(TAG, "addMdmPermCtrlPrf.");
        if (prefGroup instanceof PreferenceGroup) {
            prefGroup.addPreference(mPreference);
        }
    }

    private OnPreferenceChangeListener mPreferenceChangeListener = new OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            boolean checked = ((Boolean) newValue).booleanValue();
            Xlog.i(TAG, "CMCCMdmPermControlExt checked :" + checked);
            if(checked) {
                mPreference.setSummary(R.string.cmcc_dm_settings_enable_summary);
                updateMdmPermCtrlSwitchToDB(DM_BOOT_START_ENABLE_FALG);
            } else {
                mPreference.setSummary(R.string.cmcc_dm_settings_disable_summary);
                updateMdmPermCtrlSwitchToDB(DM_BOOT_START_DISABLE_FALG);
            }
            getMdmPermCtrlSwitchFromDB();
            return true;
        }
    };
    
    private boolean getMdmPermCtrlSwitchFromDB() {
        int value = System.getInt(mContext.getContentResolver(), System.DM_BOOT_START_ENABLE_KEY, DEF_DM_BOOT_START_ENABLE_VALUE);
        Xlog.i(TAG, "Read from DB System.DM_BOOT_START_ENABLE_KEY is:" + value);
        if(value == 1) {
            return true;
        } else {
            return false;
        }
    }
    
    private void updateMdmPermCtrlSwitchToDB(int value) {
        Xlog.i(TAG, "Put DB System.DM_BOOT_START_ENABLE_KEY is:" + value);
        boolean isSuccess = false;
        if(value == 1) {
            isSuccess =  System.putInt(mContext.getContentResolver(), System.DM_BOOT_START_ENABLE_KEY, DM_BOOT_START_ENABLE_FALG );
            mPreference.setSummary(R.string.cmcc_dm_settings_enable_summary);
        } else {
            isSuccess =  System.putInt(mContext.getContentResolver(), System.DM_BOOT_START_ENABLE_KEY, DM_BOOT_START_DISABLE_FALG);
            mPreference.setSummary(R.string.cmcc_dm_settings_disable_summary);
        }
        Xlog.i(TAG, "Put DB System.DM_BOOT_START_ENABLE_KEY isSuccess:" + isSuccess);
    }
}
