package com.mediatek.op.telephony;

public class TelephonyExtOP02 extends TelephonyExt {
    public boolean isDefaultDataOn() {
        return true;
    }

    public boolean isAutoSwitchDataToEnabledSim() {
        return true;
    }

    public boolean isDefaultEnable3GSIMDataWhenNewSIMInserted() {
        return true;
    }
}
