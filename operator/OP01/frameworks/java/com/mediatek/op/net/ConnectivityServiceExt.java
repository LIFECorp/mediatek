
package com.mediatek.common.op.net;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;

import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import android.os.IBinder;
import android.os.PowerManager;
import android.os.ServiceManager;
import android.os.RemoteException;

import android.provider.Settings;
import android.provider.Telephony.SIMInfo;

import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

import android.net.ConnectivityManager;
import android.net.NetworkUtils;
import android.net.wifi.IWifiManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.net.IConnectivityServiceExt;
import com.mediatek.op.net.DefaultConnectivityServiceExt;
import com.mediatek.xlog.Xlog;
import android.net.ConnectivityManager;
import com.mediatek.telephony.TelephonyManagerEx;

import java.util.List;

/**
 * Interface that defines all methos which are implemented in ConnectivityService
 */
 
 /** {@hide} */
public class ConnectivityServiceExt extends DefaultConnectivityServiceExt
{
    private static final String MTK_TAG = "CDS/ConnectivityServiceExt";
    
    private BroadcastReceiver mReceiver;
    private Object mSynchronizedObject;

    private static final String ACTION_PS_STATE_RESUMED = "com.mtk.ACTION_PS_STATE_RESUMED";
    private static final String ACTION_CMCC_MUSIC_RETRY = "android.intent.action.EMMRRS_PS_RESUME";
    //[RB release workaround]
    public static String ACTION_SET_PACKETS_FLUSH = "com.android.internal.ACTION_SET_PACKETS_FLUSH";
    //[RB release workaround]

    private PhoneStateListener mDataStateListener;
    private PhoneStateListener mDataStateListener2;
    private int mPsNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    private Context mContext;
        
    public void init(Context context){
        mContext = context;
                        
        IntentFilter filter =
            new IntentFilter(ACTION_PS_STATE_RESUMED);
        filter.addAction(ACTION_CMCC_MUSIC_RETRY);
        //[RB release workaround]
        filter.addAction(ACTION_SET_PACKETS_FLUSH);
        //[RB release workaround]

        mReceiver = new ConnectivityServiceReceiver();
        Intent intent = mContext.registerReceiver(mReceiver, filter);
                
        mSynchronizedObject = new Object();

        if(FeatureOption.MTK_GEMINI_SUPPORT){
            TelephonyManagerEx mTelephonyManagerEx = new TelephonyManagerEx(mContext);
            if(mTelephonyManagerEx == null) return;
            mDataStateListener  = new DataStateListener(PhoneConstants.GEMINI_SIM_1);
            mDataStateListener2 = new DataStateListener(PhoneConstants.GEMINI_SIM_2);
            mTelephonyManagerEx.listen(mDataStateListener, PhoneStateListener.LISTEN_DATA_CONNECTION_STATE, PhoneConstants.GEMINI_SIM_1);
            mTelephonyManagerEx.listen(mDataStateListener2,PhoneStateListener.LISTEN_DATA_CONNECTION_STATE, PhoneConstants.GEMINI_SIM_2);
        }else{
            TelephonyManager mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            if(mTelephonyManager == null) return;
            mDataStateListener  = new DataStateListener(PhoneConstants.GEMINI_SIM_1);
            mTelephonyManager.listen(mDataStateListener, PhoneStateListener.LISTEN_DATA_CONNECTION_STATE);
        }
        
        Xlog.d(MTK_TAG, "Init done in ConnectivityServiceExt");
        
    }
    
    public boolean isDefaultFailover(int netType, int activeDefaultNetwork) {
        if(ConnectivityManager.TYPE_WIFI != netType || ConnectivityManager.TYPE_WIFI != activeDefaultNetwork){
           return true;
        }
        
        return false;
    }

    private long getDataConnectionIdAndTurnoff() {

        long id = -1;
        ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm == null) {
            return id;
        }

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            id = Settings.System.getLong(mContext.getContentResolver(),
                    Settings.System.GPRS_CONNECTION_SIM_SETTING,
                    Settings.System.DEFAULT_SIM_NOT_SET);
        } else if(cm.getMobileDataEnabled()) {
            id = 1;
        }

        // turn it off
        if (id > 0) {
            cm.setMobileDataEnabled(false);
        }

        Xlog.d(MTK_TAG, "getLastDataconnectionSimId" + id);
        // Save it into settings.java; 
        Settings.System.putLong(mContext.getContentResolver(),
            Settings.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
            id);

        return id;
    }

    public boolean isUserPrompt(){
        boolean isCTSMode = FeatureOption.OP01_CTS_COMPATIBLE;
        if (isCTSMode) {
            Xlog.d(MTK_TAG, "CTS mode, no datadialog");
            return false;
        }

        boolean dataAvailable = isPsDataAvailable();
        Xlog.d(MTK_TAG, "dataAvailable: " + dataAvailable);
        if (!dataAvailable) {
            return false;
        }

        long id = getDataConnectionIdAndTurnoff();
        IBinder binder = ServiceManager.getService(Context.WIFI_SERVICE);
        final IWifiManager wifiService = IWifiManager.Stub.asInterface(binder);
        boolean hasConnectableAP = false;
        try{
            hasConnectableAP = wifiService.hasConnectableAp();
        } catch (RemoteException e) {
            Xlog.d(MTK_TAG, "hasConnectableAp failed!");
        }
        Xlog.d(MTK_TAG, "hasConnectableAP: " + hasConnectableAP + " LastSimId: " + id);

        if(!hasConnectableAP) {
            // Show the Data Dialog here
            Intent i = new Intent(TelephonyIntents.ACTION_WIFI_FAILOVER_GPRS_DIALOG);
            i.putExtra("simId", id);
            mContext.sendBroadcast(i);
            Xlog.d(MTK_TAG, "Send ACTION_WIFI_FAILOVER_GPRS_DIALOG intent");
        }

        // WIFI module will setup DIALOG later
        return true;
    }

    public boolean isControlBySetting(int netType, int radio) {
        Xlog.d(MTK_TAG, "isControlBySetting: netType=" + netType + " readio=" + radio);
        if (radio == ConnectivityManager.TYPE_MOBILE 
             && (netType != ConnectivityManager.TYPE_MOBILE_MMS)) {
             return true;
        }

        return false;
    }
    
    private boolean isPsDataAvailable() {

        try {
            
            //Check radio is on
            ITelephony phone = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
            if (phone == null || !phone.isRadioOn()) {
                return false;
            }

            TelephonyManagerEx tme = TelephonyManagerEx.getDefault();
            if (tme == null) {
                return false;
            }

            boolean isSIM1Insert = tme.hasIccCard(PhoneConstants.GEMINI_SIM_1);
            boolean isSIM2Insert = false;
            if(FeatureOption.MTK_GEMINI_SUPPORT){
               isSIM2Insert = tme.hasIccCard(PhoneConstants.GEMINI_SIM_2);
            }

            if(!isSIM1Insert && !isSIM2Insert) {
               return false;
            }            
        } catch (RemoteException e) {
            Xlog.e(MTK_TAG, "Connect to phone service error!");
            return false;
        }
        
        int airplanMode = Settings.System.getInt(mContext.getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0);
        Xlog.v(MTK_TAG, "airplanMode" + airplanMode);
        if (airplanMode == 1) return false;
        
        return true;
    }

    private void resetSocketConnection(){
        ActivityManager am = (ActivityManager)mContext.getSystemService(Context.ACTIVITY_SERVICE);
        String  mmPackageName = mContext.getResources().getString(com.mediatek.internal.R.string.config_mm_package_name);

        Xlog.d(MTK_TAG, "Try to resetSocketConnection");

        if(mmPackageName == null || mmPackageName.length() == 0){
           Xlog.e(MTK_TAG, "[Error] No MM package name");
        }
            
        if (am != null) {
            List<ActivityManager.RunningAppProcessInfo> appList = am.getRunningAppProcesses();
            if (appList != null) {
                for (ActivityManager.RunningAppProcessInfo info : appList) {
                    if (info.processName.startsWith(mmPackageName)) {
                        Xlog.i(MTK_TAG, "3G to 2G and MM is running with uid " + info.uid);
                        NetworkUtils.resetConnectionByUid(info.uid);
                        break;
                    }
                }
            }
        }
    }

    private void retryMMusic(){
        ActivityManager am = (ActivityManager)mContext.getSystemService(Context.ACTIVITY_SERVICE);
        if (am != null) {
            List<ActivityManager.RunningAppProcessInfo> appList = am.getRunningAppProcesses();
            if (appList != null) {
                for (ActivityManager.RunningAppProcessInfo info : appList) {
                    if (info.processName.startsWith("cmccwm.mobilemusic")) {
                        Xlog.i(MTK_TAG, "Mobile Music uid " + info.uid);
                        NetworkUtils.resetConnectionByUidErrNum(info.uid, 102);  // error 102: ENETRESET
                    } else if (info.processName.startsWith("com.aspire.mm")) {
                        Xlog.i(MTK_TAG, "Retry MM with uid " + info.uid);
                        NetworkUtils.resetConnectionByUid(info.uid);
                    }
                }
            }
        }
    }

    //[RB release workaround]
    private void retryMms(){ 
        Xlog.d(MTK_TAG, "retryMms()");

        ActivityManager am = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<RunningAppProcessInfo> processInfoList = am.getRunningAppProcesses();
        if (processInfoList == null) {
            return;
        }

        for (RunningAppProcessInfo info : processInfoList) {
            if (info.processName.equals("com.android.mms")) {
                Xlog.d(MTK_TAG, "retryMms(),info.uid:" + info.uid);
                //NetworkUtils.resetConnectionByUidErrNum(info.uid, 0);

                String rbReleaseSetting = android.os.SystemProperties.get("debug.rb.release", "true");
                if (rbReleaseSetting == null || "true".equals(rbReleaseSetting)) {
                    Xlog.d(MTK_TAG, "retryMms(),info.uid:" + info.uid +
                           ",rbReleaseSetting:" + rbReleaseSetting);
                    NetworkUtils.resetConnectionByUidErrNum(info.uid, 0);
                }
                break;
            }
        }
    }
    //[RB release workaround]

    private class ConnectivityServiceReceiver extends BroadcastReceiver {
    
        public void onReceive(Context context, Intent intent) {
            if (intent == null) return;
            String action = intent.getAction();
            Xlog.d(MTK_TAG,"received intent ==> " + action);
            
            synchronized(mSynchronizedObject){
              if (ACTION_PS_STATE_RESUMED.equals(action)) {
                  resetSocketConnection();
              } else if (ACTION_CMCC_MUSIC_RETRY.equals(action)) {
                  retryMMusic();
              } else if (ACTION_SET_PACKETS_FLUSH.equals(action)) {
                  //[RB release workaround]
                  retryMms();
              }
            }
        }
    }
    
    private class DataStateListener extends PhoneStateListener {
        int mSimID = PhoneConstants.GEMINI_SIM_1;
        
        public DataStateListener(int simID){
            mSimID = simID;
        }

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            Xlog.d(MTK_TAG, "data state:" + state + ":" + " nw type:" + networkType + "/"+ mPsNetworkType + " simId:" + mSimID);
                        
            // Only handle the active data connection
            if(state == TelephonyManager.DATA_CONNECTED){
               if( (mPsNetworkType > TelephonyManager.NETWORK_TYPE_EDGE  && networkType < TelephonyManager.NETWORK_TYPE_UMTS && networkType > TelephonyManager.NETWORK_TYPE_UNKNOWN) ||
                   (mPsNetworkType != TelephonyManager.NETWORK_TYPE_UNKNOWN && mPsNetworkType < TelephonyManager.NETWORK_TYPE_UMTS && networkType > TelephonyManager.NETWORK_TYPE_EDGE)){
                    // resetSocketConnection();
                    Xlog.d(MTK_TAG, "Send ps resumed from connectivityservice");
                    Intent intent = new Intent(ACTION_PS_STATE_RESUMED);
                    mContext.sendBroadcast(intent);
               }
               mPsNetworkType = networkType;
            }
        }
    }
}
