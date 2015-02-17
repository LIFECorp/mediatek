package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.Preference;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;

import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

public class ApnSettingsExt extends DefaultApnSettingsExt {
    
    private static final String TAG = "OP01ApnSettingsExt";
    private Context mContext;
    public ApnSettingsExt(Context context) {
        super();
        mContext = context;
    }
    
    public boolean isAllowEditPresetApn(String type, String apn, String numeric, int sourcetype) {
         return true;
    }

    /**
     * get the apn type.
     * @param defResId The default resource id for host
     * @param iTether identify is tethering or not for orange
     */
    public String[] getApnTypeArray(Context context, int defResId,boolean isTether) {
        Xlog.d(TAG,"getApnTypeArray : cmcc array");
        return mContext.getResources().getStringArray(R.array.apn_type_cmcc);
    }
}

