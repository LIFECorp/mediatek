package com.mediatek.backuprestore;

import java.util.ArrayList;
import java.util.List;

import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.MyLogger;
import com.mediatek.backuprestore.utils.Constants.LogTag;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.ListAdapter;
import android.widget.ListView;



public class SettingsAppActivity extends PreferenceActivity {
    
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/SettingsAppActivity";
    List<SettingData> mSettingDataList;
    String mTitle = null;;
    Boolean mChecked;
    private Bundle mSettingData;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.pref_settings);
        Log.d(CLASS_TAG, "~~ onCreate ~");
        if (savedInstanceState != null) {
            mSettingData = savedInstanceState;
        } else {
            Intent intent = getIntent();
            mSettingData = intent.getExtras();
            Log.d(CLASS_TAG, "~~ onCreate ~ and savedInstanceState = null mSettingData = "+mSettingData);
        }
        mTitle = Constants.APP_AND_DATA;
        mSettingDataList = initData(mSettingData);
        addResultsAsPreferences(mSettingDataList);
    }

    private void addResultsAsPreferences(List<SettingData> mSettingDataList) {
        PreferenceScreen ps = getPreferenceScreen();
        if(mSettingDataList != null && mSettingDataList.size() > 0){
            for(SettingData mSettingData : mSettingDataList){
                addRadioButtonPreferences(ps,mSettingData);
            }
            
        }
    }

    private void addRadioButtonPreferences(PreferenceScreen ps, SettingData mSettingData) {
        AppBackupRadioButtonPreference radioButtonPreference = new AppBackupRadioButtonPreference(this);
        if(mSettingData != null){
            radioButtonPreference.setTitle(mSettingData.getTitle());
            if(mSettingData.getSummary() != null){
                radioButtonPreference.setSummary(mSettingData.getSummary());
                radioButtonPreference.setKey(Constants.APP_AND_DATA);
                ps.setKey(Constants.APP_AND_DATA);
            } else {
                radioButtonPreference.setKey(Constants.ONLY_APP);
                ps.setKey(Constants.ONLY_APP);
            }
            radioButtonPreference.setChecked(mSettingData.isChecked());
        }
        ps.addPreference(radioButtonPreference);
    }

    private List<SettingData> initData(Bundle savedInstanceState) {
        SharedPreferences sharedPreferences = getSharedPreferences(Constants.SETTINGINFO, Activity.MODE_PRIVATE);
        String title = null;
        String summary = null;
        String key = null;
        boolean checked = true;
        if (savedInstanceState != null) {
            key = savedInstanceState.getString(Constants.DATE);
            
            if (key != null && !key.equals( Constants.APP_AND_DATA)) {
                checked = false;
            }
            mTitle = key;
        }
        
        Log.d(CLASS_TAG, "~initData checked = "+checked +", key = "+ key);
        List<SettingData> mSettingDataList = new ArrayList<SettingData>();
        title = getResources().getString(R.string.app_data);
        summary = getResources().getString(R.string.only_app);
        SettingData mAppAndData = new SettingData(getResources().getString(R.string.app_data), getResources()
                .getString(R.string.appdata_summary), checked, Constants.APP_AND_DATA);
        SettingData mAppData = new SettingData(getResources().getString(R.string.only_app), null, !checked,
                Constants.ONLY_APP);
        mSettingDataList.add(mAppAndData);
        mSettingDataList.add(mAppData);
        return mSettingDataList;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if(preference != null && preference instanceof AppBackupRadioButtonPreference){
            AppBackupRadioButtonPreference radioButtonPreference = (AppBackupRadioButtonPreference) preference;
            radioButtonPreference.setChecked(true);

            for (int i = 0; i < preferenceScreen.getPreferenceCount(); i++) {
                if (preferenceScreen.getPreference(i).getKey() != radioButtonPreference.getKey()
                        && preferenceScreen.getPreference(i) instanceof AppBackupRadioButtonPreference) {
                    AppBackupRadioButtonPreference appBackupRadioButtonPreference = (AppBackupRadioButtonPreference) preferenceScreen
                            .getPreference(i);
                    appBackupRadioButtonPreference.setChecked(!radioButtonPreference.isChecked());
                }
            }
            mTitle = radioButtonPreference.getKey();
            mChecked = radioButtonPreference.isChecked();
            Log.d(CLASS_TAG, "~onPreferenceTreeClick~  ~title = "+mTitle +", checked = "+mChecked);
            mSettingData = new Bundle();
            mSettingData.putString(Constants.DATE, mTitle);
            Intent data = new Intent();
            data.putExtras(mSettingData);
            setResult(RESULT_OK, data);
            this.finish();
            
        }
        return true;
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString(Constants.DATE, mTitle);
        Log.d(CLASS_TAG, "~~ onSaveInstanceState ~title = "+mTitle +", checked = "+mChecked+ ", mSettingData = "+mSettingData);
    }
    
    @Override
    protected void onStop() {
        super.onStop();
        SharedPreferences.Editor editor = AppBackupActivity.getInstance(this).edit();
        editor.putString(Constants.DATA_TITLE, mTitle);
        editor.commit();
        Log.d(CLASS_TAG, "~~ onStop ~ ");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
            mSettingData = new Bundle();
            mSettingData.putString(Constants.DATE, mTitle);
            Log.d(CLASS_TAG, "~~ onKeyDown  ~ mSettingData = " + mSettingData.getString(Constants.DATE));
            Intent data = new Intent();
            data.putExtras(mSettingData);
            setResult(RESULT_OK, data);
        }
        return super.onKeyDown(keyCode, event);
    }

}
