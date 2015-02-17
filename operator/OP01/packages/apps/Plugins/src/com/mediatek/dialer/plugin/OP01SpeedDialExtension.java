package com.mediatek.dialer.plugin;

import android.util.Log;
import android.view.View;

import com.mediatek.dialer.ext.DialerPluginDefault;
import com.mediatek.dialer.ext.SpeedDialExtension;

public class OP01SpeedDialExtension extends SpeedDialExtension {
    private static final String TAG = "OP01SpeedDialExtension";
    
    @Override
    public void setView(View view, int viewId, boolean mPrefNumContactState, int sdNumber, String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return ;
        }
        Log.i(TAG, "setView viewId : " + viewId + " | sdNumber : " + sdNumber);
        if (!mPrefNumContactState && viewId == sdNumber) {
            view.setVisibility(View.GONE);
            Log.i(TAG, "[setView] view is gone");
        } else if (viewId == sdNumber) {
            view.setVisibility(View.VISIBLE);
            Log.i(TAG, "[setView] view is visible");
        }
    }
    
    @Override
    public int setAddPosition(int mAddPosition, boolean mNeedRemovePosition, String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return mAddPosition;
        }
        if (mNeedRemovePosition) {
            return -1;
        }
        return mAddPosition;
    }

    @Override
    public boolean showSpeedInputDialog(String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        Log.i(TAG, "[showSpeedInputDialog");
        return true;
    }
    
    @Override
    public boolean needClearSharedPreferences(String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        Log.i(TAG, "[needClearSharedPreferences");
        return false;
    }
   
    public boolean clearPrefStateIfNecessary(String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        Log.i(TAG, "SpeedDialManageActivity: [clearPrefStateIfNecessary()]");
        return false;
    }

    public boolean needCheckContacts(String commd) {
        if (! DialerPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return true;
        }
        Log.i(TAG, "SpeedDialManageActivity: [needCheckContacts()]");
        return false;
    }

}
