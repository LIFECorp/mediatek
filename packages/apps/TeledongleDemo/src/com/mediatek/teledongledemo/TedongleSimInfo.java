package com.mediatek.teledongledemo;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Bundle;
//import andorid.tedongle.TedongleManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.content.BroadcastReceiver;
import android.util.Log;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.text.TextUtils;
import android.content.res.Resources;

import android.tedongle.ServiceState;
import android.tedongle.TedongleStateListener;
import android.tedongle.TedongleManager;
import android.tedongle.SignalStrength;
import com.android.internal.tedongle.ITedongleStateListener;


public class TedongleSimInfo extends PreferenceActivity {
	private final String LOG_TAG = "3GD-APK-SimInfo";
    private TedongleManager mTeledongleManager;
	private boolean mIsAirplneMode = false;
    private static Resources sRes;
    private int mServiceState;
    private IntentFilter mIntentFilter;
    private static String sUnknown;
	
    private static final String KEY_NETWORK_TYPE = "network_type";
    private static final String KEY_SIM_NUMBVER = "sim_number";
    private static final String KEY_SERVICE_STATE = "service_state";
    private static final String KEY_ROAMING_STATE = "roaming_state";
    private static final String KEY_OPERATOR_NAME = "operator_name";
    private static final String KEY_SIM_IMSI = "sim_imsi";
    
    private TedongleStateListener mPhoneStateListener = new TedongleStateListener() {

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            Log.d(LOG_TAG, "onDataConnectionStateChanged");
            //updateDataState();
            //updateNetworkType();
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
                mServiceState = serviceState.getState();
                Log.d(LOG_TAG, "ServiceStateChanged mServiceState : " + mServiceState);
                updateServiceState(serviceState);
        }
    };
    
     private BroadcastReceiver mTedongleReceiver = new BroadcastReceiver() {
    	@Override
        public void onReceive(Context context, Intent intent) {
   
            String action = intent.getAction();
            Log.d(LOG_TAG, "TedongleReceiver -----action=" + action);            
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                
                boolean enabled = intent.getBooleanExtra("state", false);
                Log.d(LOG_TAG, "TedongleReceiver ------ AIRPLANEMODE enabled=" + enabled);
                mIsAirplneMode = enabled;
                mTeledongleManager.setRadio(!enabled);
            }
        }
    };
    
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//mTeledongleManager = (TedongleManager)getSystemService("tedongleservice");
		//mTeledongleManager = new TedongleManager(this);
		mTeledongleManager = TedongleManager.getDefault();
		addPreferencesFromResource(R.xml.sim_status);
        sRes = getResources();
        sUnknown = sRes.getString(R.string.device_info_default);
        updateSiminfo();
        updateNetworkType();
        ServiceState serviceState = mTeledongleManager.getServiceState();
        updateServiceState(serviceState);
        mTeledongleManager.listen(mPhoneStateListener,
        		TedongleStateListener.LISTEN_DATA_CONNECTION_STATE
                        | TedongleStateListener.LISTEN_SIGNAL_STRENGTHS
                        | TedongleStateListener.LISTEN_SERVICE_STATE);
        
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mTedongleReceiver, mIntentFilter);
        Log.d(LOG_TAG, "oncreate end...");
	}
	
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    private void setSummaryText(String preference, String text) {
        if (TextUtils.isEmpty(text)) {
            text = this.getResources().getString(R.string.device_info_default);
        }
        // some preferences may be missing
        Preference p = findPreference(preference);
        if (p != null) {
            p.setSummary(text);
        }
    }
    
    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mTedongleReceiver);
    }  
     

    private void updateSiminfo() {

        Log.d(LOG_TAG, "updateSiminfo " );
        //Sim number
        String sim_number = mTeledongleManager.getLine1Number();
        setSummaryText(KEY_SIM_NUMBVER, sim_number);
        //sim imsi
        String sim_imsi = mTeledongleManager.getSubscriberId();
        setSummaryText(KEY_SIM_IMSI, sim_imsi);
    }
    
    private void updateNetworkType() {
        // Whether EDGE, UMTS, etc...
        String netWorkTypeName = mTeledongleManager.getNetworkTypeName();
        Log.d(LOG_TAG,"getNetworkTypeName:"+ netWorkTypeName);
        Preference p = findPreference(KEY_NETWORK_TYPE);
        if (p != null) {
            p.setSummary((netWorkTypeName.equals("UNKNOWN")) ? sUnknown
                    : netWorkTypeName);
        }
    }
    private void updateServiceState(ServiceState serviceState) {

        int state = serviceState.getState();
        String display = sRes.getString(R.string.radioInfo_unknown);

        switch (state) {
        case ServiceState.STATE_IN_SERVICE:
            display = sRes.getString(R.string.radioInfo_service_in);
            break;
        case ServiceState.STATE_OUT_OF_SERVICE:
            display = sRes.getString(R.string.radioInfo_service_out);
            break;
        case ServiceState.STATE_EMERGENCY_ONLY:
            display = sRes.getString(R.string.radioInfo_service_emergency);
            break;
        case ServiceState.STATE_POWER_OFF:
            display = sRes.getString(R.string.radioInfo_service_off);
            break;
        default:
            break;
        }

        setSummaryText(KEY_SERVICE_STATE, display);

        if (serviceState.getRoaming()) {
            setSummaryText(KEY_ROAMING_STATE, sRes
                    .getString(R.string.radioInfo_roaming_in));
        } else {
            setSummaryText(KEY_ROAMING_STATE, sRes
                    .getString(R.string.radioInfo_roaming_not));
        }
        setSummaryText(KEY_OPERATOR_NAME, serviceState.getOperatorAlphaLong());
    }

}
