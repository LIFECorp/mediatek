package com.mediatek.keyguard.plugin;

import android.content.Context;
import android.os.RemoteException;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.keyguard.ext.DefaultEmergencyButtonExt;
import com.mediatek.xlog.Xlog;

public class OP02EmergencyButtonExt extends DefaultEmergencyButtonExt {

    private String TAG = "OP02EmergencyButtonExt";

    @Override
    public boolean isSimInService(boolean[] isServiceSupportEcc, int slotId) {
        ITelephony tmex = ITelephony.Stub.asInterface(android.os.ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        boolean isSimReady = false;
        boolean isServiceSupport = false;
        if (null != tmex) {
            for (int i = PhoneConstants.GEMINI_SIM_1; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
                try {
                    Log.i(TAG, "tmex.getSimState(i)   = " + tmex.getSimState(i)  + "    i =  " +i );
                    if (TelephonyManager.SIM_STATE_READY ==  tmex.getSimState(i)) {
                        isSimReady =  true;
                        break;
                    }
                } catch (RemoteException e) {
                    Xlog.i(TAG, "tmex.isSimInsert(i) has RemoteException");
                    return false;
                }
            }
        }
        if (false == isSimReady) {
            return false;
        }
        for (int i = 0; i < isServiceSupportEcc.length; i++) {
            if (isServiceSupportEcc[i]) {
                isServiceSupport = true;
                break;
            }
        }
        return isSimReady && isServiceSupport;
    }
}
