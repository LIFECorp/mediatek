package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.op01.plugin.R;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

/**
 * M: OP01 implementation of Plug-in definition of Status bar.
 */
public class Op01StatusBarPlugin extends DefaultStatusBarPlugin {
    public Op01StatusBarPlugin(Context context) {
        super(context);
    }

    public Resources getPluginResources() {
        return this.getResources();
    }

    public String getSignalStrengthDescription(int level) {
        return getString(AccessibilityContentDescriptions.PHONE_SIGNAL_STRENGTH[level]);
    }

    public boolean isHspaDataDistinguishable() {
        return false;
    }

    public int[] getDataActivityIconList(int simColor, boolean showSimIndicator) {
    	if (showSimIndicator) {
    		return TelephonyIcons.DATA_ACTIVITY_S[simColor];
    	} else {
    		return TelephonyIcons.DATA_ACTIVITY;
    	}
    }

    public boolean supportDataTypeAlwaysDisplayWhileOn() {
        return true;
    }

    public boolean supportDisableWifiAtAirplaneMode() {
        return true;
    }

    public String get3gDisabledWarningString() {
        return null;
    }
}
