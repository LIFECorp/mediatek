package com.mediatek.teledongledemo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class TedongleSupportListActivity extends Activity{

	private TextView mTextView;
	private final String LOG_TAG="TedongleSupportListActivity";
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.tedongle_support_list);
	}

	@Override
	public void onResume() {
		super.onResume();
	}

	@Override
	public void onPause() {
		super.onPause();
	}
	
}
