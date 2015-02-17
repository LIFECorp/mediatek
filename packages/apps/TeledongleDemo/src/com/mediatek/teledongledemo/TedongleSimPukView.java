package com.mediatek.teledongledemo;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;

public class TedongleSimPukView extends Activity {
	
	private final String LOG_TAG = "3GD-APK-TedongleSimPukView";
	private static final String ACTION_SIM_PUK_OK = "tedongle.intent.action.SIM_PUK_OK";
	private static final String ACTION_SIM_PUK_WRONG = "tedongle.intent.action.SIM_PUK_WRONG";
	private IntentFilter mIntentFilter;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.tedongle_sim_puk);	
	
	    mIntentFilter = new IntentFilter();
	    mIntentFilter.addAction(ACTION_SIM_PUK_OK);
	    mIntentFilter.addAction(ACTION_SIM_PUK_WRONG);
	    registerReceiver(mSimPukReceiver, mIntentFilter);
	}
	
	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mSimPukReceiver);
	}
	
	private BroadcastReceiver mSimPukReceiver = new BroadcastReceiver() {
	    @Override
	    public void onReceive(Context context, Intent intent) {
	        String action = intent.getAction();
	        Log.d(LOG_TAG, "mSimPukReceiver receive action=" + action);
	        // updated
	        if (action.equals(ACTION_SIM_PUK_OK)) {
	        	finish();
	        }
	    }
	};
}
