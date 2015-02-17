package com.mediatek.teledongledemo;

import android.preference.*;
import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import android.content.res.TypedArray;
import android.widget.Checkable;
import android.widget.CompoundButton;
import android.widget.Switch;

public class TedongleRadioEnablePre extends SwitchPreference{
	
    private static final String TAG = "3GD-APK-TedongleRadioEnablePre";
    private TeledongleDemoActivity mTeledongleDemoActivity;
    
    public void setUiActivity(TeledongleDemoActivity mTedongle ){
    	this.mTeledongleDemoActivity = mTedongle;
    }
    public TedongleRadioEnablePre(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public TedongleRadioEnablePre(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public TedongleRadioEnablePre(Context context) {
        super(context);
    }

    @Override
    protected void onClick() {
        Log.d(TAG,"onClick()");
    }
    
    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
			
			@Override
			public boolean onPreferenceChange(Preference preference, Object newValue) {
				// TODO Auto-generated method stub
		        Log.d(TAG,"onPreferenceChange+++");
		        boolean value = Boolean.parseBoolean(String.valueOf(newValue));
				mTeledongleDemoActivity.switchPreferenceValue(TedongleRadioEnablePre.this, value );
				return true;
			}
		});   	
        
       
    }
}
