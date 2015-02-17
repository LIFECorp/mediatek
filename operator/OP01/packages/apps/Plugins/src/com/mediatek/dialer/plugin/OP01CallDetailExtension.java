package com.mediatek.dialer.plugin;

import android.provider.CallLog.Calls;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.mediatek.dialer.ext.CallDetailExtension;
import com.mediatek.dialer.ext.DialerPluginDefault;

public class OP01CallDetailExtension extends CallDetailExtension {
    private static final String TAG = "OP01CallDetailExtension";
    
    @Override
    public String getCommand() {
        return DialerPluginDefault.COMMD_FOR_OP01;
    }
    
    @Override
    public void setTextView(int callType, TextView durationView, String formatDuration, String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return;
        }
        if (callType == Calls.MISSED_TYPE || callType == Calls.VOICEMAIL_TYPE) {
            durationView.setVisibility(View.GONE);
        }
    }
    
    @Override
    public boolean isNeedAutoRejectedMenu(boolean isAutoRejectedFilterMode, String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        Log.i(TAG, "isAutoRejectedFilterMode : " + isAutoRejectedFilterMode);
        return isAutoRejectedFilterMode;
    }
    
    @Override
    public String setChar(boolean notSPChar, String str, String spChar, int charType, boolean secondSelection, String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return null;
        }
        if (notSPChar) {
            Log.i(TAG, "str : " + str);
            return str;
        } else if (secondSelection) {
            String result = spChar + "!=" + charType;
            Log.i(TAG, "result : " + result);
            return result;
        } else {
            return null;
        }
    }

}
