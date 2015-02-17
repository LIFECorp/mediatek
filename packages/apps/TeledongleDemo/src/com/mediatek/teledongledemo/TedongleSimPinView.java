package com.mediatek.teledongledemo;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.tedongle.TedongleManager;

public class TedongleSimPinView extends Activity {
		
	private final String LOG_TAG = "3GD-APK-TedongleSimPinView";
	private static final String ACTION_SIM_PIN_OK = "tedongle.intent.action.SIM_PIN_OK";
	private static final String ACTION_SIM_PIN_WRONG = "tedongle.intent.action.SIM_PIN_WRONG";
	private IntentFilter mIntentFilter;
	private TedongleManager mTedongleManager;

    /** SIM card state: Unknown. Signifies that the SIM is in transition
     *  between states. For example, when the user inputs the SIM pin
     *  under PIN_REQUIRED state, a query for sim status returns
     *  this state before turning to SIM_STATE_READY. */
    public static final int SIM_STATE_UNKNOWN = 0;
    /** SIM card state: no SIM card is available in the device */
    public static final int SIM_STATE_ABSENT = 1;
    /** SIM card state: Locked: requires the user's SIM PIN to unlock */
    public static final int SIM_STATE_PIN_REQUIRED = 2;
    /** SIM card state: Locked: requires the user's SIM PUK to unlock */
    public static final int SIM_STATE_PUK_REQUIRED = 3;
    /** SIM card state: Locked: requries a network PIN to unlock */
    public static final int SIM_STATE_NETWORK_LOCKED = 4;
    /** SIM card state: Ready */
    public static final int SIM_STATE_READY = 5;
    /** SIM card state: Not Ready */    
    public static final int SIM_STATE_NOT_READY = 6;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.tedongle_sim_pin);	
		mTedongleManager = TedongleManager.getDefault();
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(ACTION_SIM_PIN_OK);
        mIntentFilter.addAction(ACTION_SIM_PIN_WRONG);
        registerReceiver(mSimPinReceiver, mIntentFilter);
	}
	
	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mSimPinReceiver);
	}
	
    private BroadcastReceiver mSimPinReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(LOG_TAG, "mSimPinReceiver receive action=" + action);
            // updated
            if(action.equals(ACTION_SIM_PIN_OK)) {
            	finish();
            }else if(action.equals(ACTION_SIM_PIN_WRONG) 
            && SIM_STATE_PUK_REQUIRED == mTedongleManager.getSimState()) {
            	finish();
            }
        }
    };
}
