package com.mediatek.ppl;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.storage.IMountService;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.SmsManagerEx;
import com.mediatek.telephony.TelephonyManagerEx;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

public class PlatformManager {
    protected static final String TAG = "PPL/PlatformManager";
    private final Context mContext;
    private final IPplAgent mAgent;
    private final SmsManagerEx mSmsManager;
    private final IMountService mMountService;
    private final ConnectivityManager mConnectivityManager;
    private final WakeLock mWakeLock;

    public static final int SIM_NUMBER;

    static {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                SIM_NUMBER = 4;
            } else if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                SIM_NUMBER = 3;
            } else {
                SIM_NUMBER = 2;
            }
        } else {
            SIM_NUMBER = 1;
        }
    }

    public PlatformManager(Context context) {
        mContext = context;
        IBinder binder = ServiceManager.getService("PPLAgent");
        if (binder == null) {
            throw new Error("Failed to get PPLAgent");
        }
        mAgent = IPplAgent.Stub.asInterface(binder);
        if (mAgent == null) {
            throw new Error("mAgent is null!");
        }
        mSmsManager = SmsManagerEx.getDefault();
        mMountService = IMountService.Stub.asInterface(ServiceManager.getService("mount"));
        mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "PPL_WAKE_LOCK");
    }

    public IPplAgent getPPLAgent() {
        return mAgent;
    }

    public void sendTextMessage(String destinationAddress, long id, String text, Intent sentIntent, int simId) {
        Log.d(TAG, "sendTextMessage(" + destinationAddress + ", " + id + ", " + text + ", " + simId + ")");
        ArrayList<String> segments = divideMessage(text);
        ArrayList<PendingIntent> pis = new ArrayList<PendingIntent>(segments.size());
        final int total = segments.size();
        for (int i = 0; i < total; ++i) {
            Intent intent = new Intent(sentIntent);
            Uri.Builder builder = new Uri.Builder();
            builder.authority(MessageManager.SMS_PENDING_INTENT_DATA_AUTH)
                    .scheme(MessageManager.SMS_PENDING_INTENT_DATA_SCHEME).appendPath(Long.toString(id))
                    .appendPath(Integer.toString(total)).appendPath(Integer.toString(i));
            Log.d(TAG, "sendTextMessage: uri string is " + builder.toString());
            intent.setData(builder.build());

            byte type = intent.getByteExtra(PendingMessage.KEY_TYPE, MessageManager.Type.INVALID);
            String number = intent.getStringExtra(PendingMessage.KEY_NUMBER);
            Log.d(TAG, "id is " + id + ", type is " + type + ", number is " + number);

            PendingIntent pi = PendingIntent.getBroadcast(mContext, 0, intent, PendingIntent.FLAG_ONE_SHOT);
            pis.add(pi);
        }
        sendMultipartTextMessage(destinationAddress, null, segments, pis, simId);
    }

    protected ArrayList<String> divideMessage(String text) {
        return mSmsManager.divideMessage(text);
    }

    protected void sendMultipartTextMessage(String destinationAddress, String scAddress, ArrayList<String> parts,
            ArrayList<PendingIntent> sentIntents, int simId) {
        mSmsManager.sendMultipartTextMessage(destinationAddress, scAddress, parts, sentIntents, null, simId);
    }

    public boolean isUsbMassStorageEnabled() {
        try {
            return mMountService.isUsbMassStorageEnabled();
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }

    public void setMobileDataEnabled(boolean enable) {
        mConnectivityManager.setMobileDataEnabled(enable);
    }
    
    public void acquireWakeLock() {
        mWakeLock.acquire();
    }
    
    public void releaseWakeLock() {
        mWakeLock.release();
    }

    /**
     * TelephonyManager may be unavailable in certain circumstance such as data encrypting process of the phone.
     * @return  Whether TelephonyManager is available.
     */
    @SuppressWarnings("unused")
    public static boolean isTelephonyReady(Context context){
        boolean teleEnable = true;

        try {
            TelephonyManagerEx telephonyManagerEx = new TelephonyManagerEx(context);
            if (telephonyManagerEx == null) {
                teleEnable = false;
                Log.e(TAG, "TelephonyManagerEx is null");
            }
            telephonyManagerEx.hasIccCard(0);

        } catch (Throwable t) {
            Log.e(TAG, "TelephonyManager(Ex) is not ready");
            teleEnable = false;
        }

        return teleEnable;
    }


   public static List<HashMap<String, Object>> buildSimInfo(Context context) {
        List<SimInfoRecord> simItem =  SimInfoManager.getInsertedSimInfoList(context);
        Log.i(TAG, "simItem: " + simItem.size());

        // sort the unordered list
        Collections.sort(simItem, new SimInfoComparable());

        List<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
        for (int i = 0; i < simItem.size(); i++) {
            HashMap<String, Object> map = new HashMap<String, Object>();

            int indicator = getSimIndicator(context, simItem.get(i).mSimSlotId);
            if (indicator == PhoneConstants.SIM_INDICATOR_UNKNOWN ||
                    indicator == PhoneConstants.SIM_INDICATOR_NORMAL) {
                indicator = -1;
            }

            map.put("ItemImage", indicator);
            map.put("ItemTitle", simItem.get(i).mDisplayName);
            map.put("Color", simItem.get(i).mColor);

            Log.i(TAG, "mSimSlotId: " + simItem.get(i).mSimSlotId);
            Log.i(TAG, "mDisplayName: " + simItem.get(i).mDisplayName);
            Log.i(TAG, "mColor: " + simItem.get(i).mColor);

            data.add(map);
        }

        return data;
    }

    private static class SimInfoComparable implements Comparator<SimInfoRecord> {
        @Override
         public int compare(SimInfoRecord sim1, SimInfoRecord sim2) {
            return sim1.mSimSlotId - sim2.mSimSlotId;
         }
     }

    private static int getSimIndicator(Context context, int slotId) {
        if (isAllRadioOff(context)) {
            Log.i(TAG, "isAllRadioOff=" + isAllRadioOff(context) + "slotId=" + slotId);
            return PhoneConstants.SIM_INDICATOR_RADIOOFF;
        }

        ITelephonyEx iTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        int indicator = PhoneConstants.SIM_INDICATOR_UNKNOWN;
        if (iTelephonyEx != null) {
            try {
                indicator = iTelephonyEx.getSimIndicatorState(slotId);
                Log.i(TAG, "indicator is " + indicator);
            } catch (RemoteException e) {
                Log.i(TAG, "RemoteException");
            } catch (NullPointerException ex) {
                Log.i(TAG, "NullPointerException");
            }
        }
        return indicator;
    }

    /**
     * Get the pic from framework according to sim indicator state 
     * @param state sim indicator state
     * @return the pic res from mediatek framework
     */
    public static int getStatusResource(int state) {
        int resId;
        switch (state) {
        case PhoneConstants.SIM_INDICATOR_RADIOOFF:
            resId = com.mediatek.internal.R.drawable.sim_radio_off;
            break;
        case PhoneConstants.SIM_INDICATOR_LOCKED:
            resId = com.mediatek.internal.R.drawable.sim_locked;
            break;
        case PhoneConstants.SIM_INDICATOR_INVALID:
            resId = com.mediatek.internal.R.drawable.sim_invalid;
            break;
        case PhoneConstants.SIM_INDICATOR_SEARCHING:
            resId = com.mediatek.internal.R.drawable.sim_searching;
            break;
        case PhoneConstants.SIM_INDICATOR_ROAMING:
            resId = com.mediatek.internal.R.drawable.sim_roaming;
            break;
        case PhoneConstants.SIM_INDICATOR_CONNECTED:
            resId = com.mediatek.internal.R.drawable.sim_connected;
            break;
        case PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
            resId = com.mediatek.internal.R.drawable.sim_roaming_connected;
            break;
        default:
            resId = PhoneConstants.SIM_INDICATOR_UNKNOWN;
            break;
        }
        return resId;
    }

    /**
     * Get sim color resources
     * @param colorId sim color id
     * @return the color resource 
     */
    public static int getSimColorResource(int colorId) {
        int bgColor = -1;
        if ((colorId >= 0) && (colorId < SimInfoManager.SimBackgroundDarkRes.length)) {
            bgColor = SimInfoManager.SimBackgroundDarkRes[colorId];
        } else if (colorId == SimInfoManager.SimBackgroundDarkRes.length) {
            bgColor = com.mediatek.internal.R.drawable.sim_background_sip;
        }

        return bgColor;
    }

    private static boolean isAllRadioOff(Context context) {
        int airMode = Settings.Global.getInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1);
        int dualMode = Settings.System.getInt(context.getContentResolver(), Settings.System.DUAL_SIM_MODE_SETTING, -1);
        return airMode == 1 || dualMode == 0;
    }

}
