package com.mediatek.op.telephony;

import android.util.Log;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.content.Intent;
import android.content.Context;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.featureoption.FeatureOption;
import android.telephony.SignalStrength;
import android.content.res.Resources;

public class ServiceStateExtOP01 extends ServiceStateExt {
    private static final String ACTION_PS_RESUME = "com.mtk.ACTION_PS_STATE_RESUMED";
    private static final String ACTION_EMMRRS_PS_RESUME_INDICATOR = "android.intent.action.EMMRRS_PS_RESUME";
    private Context mContext;

    public ServiceStateExtOP01() {
    }

    public ServiceStateExtOP01(Context context) {
        mContext = context;
    }

    public void onPollStateDone(ServiceState oldSS, ServiceState newSS,
        int gprsState, int newGprsState, int psNetworkType, int newPsNetworkType) 
    {
        if ((newGprsState == ServiceState.STATE_IN_SERVICE && gprsState == ServiceState.STATE_OUT_OF_SERVICE) ||
            (psNetworkType >= TelephonyManager.NETWORK_TYPE_UMTS &&
             newPsNetworkType > TelephonyManager.NETWORK_TYPE_UNKNOWN &&
             newPsNetworkType <= TelephonyManager.NETWORK_TYPE_EDGE) ||
             (psNetworkType <= TelephonyManager.NETWORK_TYPE_EDGE &&
             psNetworkType > TelephonyManager.NETWORK_TYPE_UNKNOWN &&
             newPsNetworkType >= TelephonyManager.NETWORK_TYPE_UMTS)) {
            //this is a workaround for MM. when 3G->2G or 2G->3G RAU, PS temporary unavailable
            //it will trigger TCP delay retry and make MM is resumed slowly
            //PS status is recovered from unknown to in service
            //we could trigger MM retry mechanism by socket timeout

            log("PS resumed and broadcast resume intent");
            Intent intent = new Intent(ACTION_PS_RESUME);
            mContext.sendBroadcast(intent);
        }
    }

    public String onUpdateSpnDisplay(String plmn, int radioTechnology) {
        if (radioTechnology > 2 && plmn != Resources.getSystem().getText(com.android.internal.R.string.lockscreen_carrier_default).toString())
            plmn = plmn + " 3G";

        log("Current PLMN: " + plmn);
        return plmn;
    }
    
    public int mapGsmSignalLevel(int asu,int GsmRscpQdbm){
        int level;

        if (asu <= 2 || asu == 99) level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (asu >= 12) level = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (asu >= 8)  level = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (asu >= 5)  level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else level = SignalStrength.SIGNAL_STRENGTH_POOR;

        return level;
    }

    public int mapGsmSignalDbm(int GsmRscpQdbm,int asu){
        int dBm;

        if(GsmRscpQdbm < 0)
            dBm = GsmRscpQdbm / 4; // Return raw value for TDD 3G network.
        else
            dBm = -113 + (2 * asu);        
		
        return dBm;
    }		

    public boolean isBroadcastEmmrrsPsResume(int value) {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (value == 1) {
                Intent intent = new Intent(ACTION_EMMRRS_PS_RESUME_INDICATOR);
                mContext.sendBroadcast(intent);
                return true;
            }
        }
        return false;
    }

    public boolean needEMMRRS() {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            return true;
        } else {
            return false;
        }
    }

    public boolean needSpnRuleShowPlmnOnly() {
        return true;
    }
}
