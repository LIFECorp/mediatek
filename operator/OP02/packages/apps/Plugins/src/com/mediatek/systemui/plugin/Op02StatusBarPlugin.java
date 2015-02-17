package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;

import com.mediatek.op02.plugin.R;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

/**
 * M: OP02 implementation of Plug-in definition of Status bar.
 */
public class Op02StatusBarPlugin extends DefaultStatusBarPlugin {

    public Op02StatusBarPlugin(Context context) {
        super(context);
    }

    public Resources getPluginResources() {
        return this.getResources();
    }

    public int getSignalStrengthNullIconGemini(int slotId) {
        switch (slotId) {
        case 0:
            return R.drawable.stat_sys_gemini_signal_null_sim1;
        case 1:
            return R.drawable.stat_sys_gemini_signal_null_sim2;
        case 2:
            return R.drawable.stat_sys_gemini_signal_null_sim3;
        case 3:
            return R.drawable.stat_sys_gemini_signal_null_sim4;
        default:
            return -1;
        }
    }

    public int getSignalIndicatorIconGemini(int slotId) {
        switch (slotId) {
        case 0:
            return R.drawable.stat_sys_gemini_signal_indicator_sim1;
        case 1:
            return R.drawable.stat_sys_gemini_signal_indicator_sim2;
        case 2:
            return R.drawable.stat_sys_gemini_signal_indicator_sim3;
        case 3:
            return R.drawable.stat_sys_gemini_signal_indicator_sim4;
        default:
            return -1;
        }
    }

    public int[] getDataTypeIconListGemini(boolean roaming, DataType dataType) {
        int[] iconList = null;
        if (roaming) {
            iconList = TelephonyIcons.DATA_ROAM[dataType.getTypeId()];
        }
        return iconList;
    }
    
    public int getDataNetworkTypeIconGemini(NetworkType networkType, int simColorId) {
        int typeId = networkType.getTypeId();
        if (typeId >= 0 && typeId <= 1) {
            return TelephonyIcons.NETWORK_TYPE[typeId][simColorId];
        }
        return -1;
    }

    public String get3gDisabledWarningString() {
        return getString(R.string.gemini_3g_disable_warning);
    }

    public boolean getMobileGroupVisible() {
        return true;	
    }
}
