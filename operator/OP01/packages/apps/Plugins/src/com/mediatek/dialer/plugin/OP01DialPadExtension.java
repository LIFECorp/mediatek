package com.mediatek.dialer.plugin;

import android.util.Log;

import com.mediatek.dialer.ext.DialPadExtension;
import com.mediatek.dialer.ext.DialerPluginDefault;

public class OP01DialPadExtension extends DialPadExtension {
    private static final String TAG = "OP01DialPadExtension";
    @Override
    public String changeChar(String string, String string2, String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return null;
        }
        Log.i(TAG, "[changeChar] string : " + string + " | string2 : " + string2);
        return string2;
    }

}
