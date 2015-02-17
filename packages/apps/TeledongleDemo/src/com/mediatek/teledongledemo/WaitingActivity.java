package com.mediatek.teledongledemo;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.widget.Toast;

public class WaitingActivity extends Activity {
	private IntentFilter mIntentFilter;
	public static final String ACTION_CLOSE_WAITING = "com.mediatek.dongle.waiting";
	public static final String ACTION_DONGLE_UNINSTALL = "com.mediatek.dongle.uninstall";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.waiting_activity);
		mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(ACTION_CLOSE_WAITING);
		registerReceiver(mWaitingReceiver, mIntentFilter);
    }
        private BroadcastReceiver mWaitingReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(ACTION_CLOSE_WAITING)){
				//makeToast("finish ACTION_CLOSE_WAITING");
                finish();

			}//else if(action.equals(ACTION_DONGLE_UNINSTALL)) {
				//makeToast("ACTION_DONGLE_UNINSTALL");
               // finish();
            //}
        }
    };
		public void makeToast(String str) {
		Toast.makeText(this, str, Toast.LENGTH_SHORT).show();
	}
}
