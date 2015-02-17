package com.mediatek.op.telephony;

import android.content.Context;
import android.content.res.Resources;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.telephony.TelephonyManagerEx;

/// M: The OP02 implementation of ServiceState.
public class ServiceStateExtOP02 extends ServiceStateExt {
    public ServiceStateExtOP02() {
    }

    public ServiceStateExtOP02(Context context) {
    }

    @Override
    public String getEccPlmnValue() {
        int state = 0;
        boolean hasNoSimCard = true;
        for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            state = TelephonyManagerEx.getDefault().getSimState(i);
            if (state == TelephonyManager.SIM_STATE_READY) {
                return Resources.getSystem().getText(com.android.internal.R.string.emergency_calls_only).toString();
            }
            hasNoSimCard = hasNoSimCard && (state == TelephonyManager.SIM_STATE_ABSENT);
        }
        if (hasNoSimCard) {
            return Resources.getSystem().getText(com.android.internal.R.string.lockscreen_missing_sim_message_short).toString();
        }
        return Resources.getSystem().getText(com.android.internal.R.string.lockscreen_carrier_default).toString();
    }
}
