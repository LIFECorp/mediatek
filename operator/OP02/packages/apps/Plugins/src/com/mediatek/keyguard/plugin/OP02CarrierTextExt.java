package com.mediatek.keyguard.plugin;

import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.ServiceState;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.keyguard.ext.DefaultCarrierTextExt;
import com.mediatek.xlog.Xlog;

public class OP02CarrierTextExt extends DefaultCarrierTextExt {
    public static final String TAG = "OP02CarrierTextExt";

    @Override
    public CharSequence getTextForSimMissing(CharSequence simMessage, CharSequence original, int simId) {
        Bundle bd = null;
        try {
            ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(ServiceManager.checkService("phoneEx"));
            if (phoneEx != null) {
                bd = phoneEx.getServiceState(simId);
                ServiceState ss = ServiceState.newFromBundle(bd);
                Xlog.i(TAG, " ss.isEmergencyOnly()=" + ss.isEmergencyOnly()
                        + " for simId=" + simId);
                if (ss.isEmergencyOnly()) {
                    return simMessage;
                }
            }
        } catch (RemoteException e) {
            Xlog.i(TAG, "getServiceState error e:" + e.getMessage());
        }
        return super.getTextForSimMissing(simMessage, original, simId);
    }
}
