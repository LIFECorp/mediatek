package com.mediatek.dongle;

import android.app.Application;
import android.os.UserHandle;
import android.util.Log;

public class dongle extends Application{
	public static final String TAG = "DONGLE";
	TedongleService mTedongleService;
	
	public dongle() {
	}
	
	@Override
	public void onCreate() {
		super.onCreate();
		if (UserHandle.myUserId() == 0) {
			Log.d(TAG, "22222222 for debug");
			mTedongleService = TedongleService.init(this);
		}

		
	}
}