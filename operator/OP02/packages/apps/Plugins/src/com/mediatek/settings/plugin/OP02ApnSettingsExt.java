package com.mediatek.settings.plugin;

import android.preference.Preference;
import android.preference.PreferenceGroup;

import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.xlog.Xlog;

public class OP02ApnSettingsExt extends DefaultApnSettingsExt {

    private static final String TAG = "OP02ApnSettingsExt";
    private static final String CU_NUMERIC = "46001";

    @Override
    public boolean isAllowEditPresetApn(String type, String apn, String numeric, int sourcetype) {
        return (!numeric.equals(CU_NUMERIC) || sourcetype != 0);
    }
}

