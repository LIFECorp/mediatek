package com.mediatek.rejection;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.ServiceManager;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op01.plugin.R;
import com.mediatek.phone.callrejection.CallRejectSetting;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;


public class CallrejectionTest extends
        ActivityInstrumentationTestCase2<CallRejectSetting> {

    public static final String TAG = "CallrejectionTest";
    private static final int TWO_SECONDS = 2000;
    private static final int FIVE_SECONDS = 5000;
    private static final int THIRTY_SECONDS = 30000;

    private Instrumentation mInst;
    private Solo mSolo;
    private Context mContext;
    private Activity mActivity;
    private ActionBar mActionBar;
    private ImageView mKillIcon;
    private ImageView mKillAll;
    private ListView mListView;
    private LayoutInflater mInflater;

    public CallrejectionTest() {
        super("com.mediatek.phone.callrejection", CallRejectSetting.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
    }
	
    @Override
    public void tearDown()throws Exception{
        Log.d(TAG, "SettingsTest tearDown:");
        super.tearDown();
		try{
            mSolo.finishOpenedActivities();
		} catch(Exception e){
		// ignore
        }
    }

    private String findSelect(String deleteButton) {
        String newString = null;

        for(int i = 0; i < 200; i++) {
            newString = mActivity.getString(R.string.selected_item_count, i);
            Log.d(TAG, "newString:" + newString);
            if (mSolo.searchText(newString) && i < 200) {
                return newString;
            } else if (i > 199) {
                newString = null;
            }
        }
        return newString;
    }

    private void addFromInput() {
        Log.d(TAG, "addFromInput");
        String rejectList = mActivity.getString(R.string.voice_call_reject_list_title);
        if (mSolo.searchText(rejectList)) {
            Log.d(TAG, "addFromInput click reject list:" + rejectList);
            mSolo.clickOnText(rejectList);
            mSolo.sleep(FIVE_SECONDS);
            //search add button
            String addButton = mActivity.getString(R.string.call_reject_list_add);
            if (mSolo.searchText(addButton)) {
                Log.d(TAG, "addFromInput click add button:" + addButton);
                mSolo.clickOnText(addButton);
                mSolo.sleep(FIVE_SECONDS);
                Log.d(TAG, "addFromInput input reject number: 10086");
                mSolo.enterText(0, "10086");
                mSolo.sleep(FIVE_SECONDS);

                String saveText = mActivity.getString(R.string.save);
                if (mSolo.searchText(saveText)) {
                    Log.d(TAG, "addFromInput click the confirm:" + saveText);
                    mSolo.clickOnText(saveText);
                    mSolo.sleep(FIVE_SECONDS);
                    Log.d(TAG, "addFromInput back to CallRejectSetting");
                    mSolo.goBack();
                    mSolo.sleep(FIVE_SECONDS);
                }
            }
        }
    }

    private void addFromContacts() {
        Log.d(TAG, "addFromContacts");
        String rejectList = mActivity.getString(R.string.voice_call_reject_list_title);
        if (mSolo.searchText(rejectList)) {
            Log.d(TAG, "addFromContacts click reject list:" + rejectList);
            mSolo.clickOnText(rejectList);
            mSolo.sleep(FIVE_SECONDS);

            //search add button
            String addButton = mActivity.getString(R.string.call_reject_list_add);
            if (mSolo.searchText(addButton)) {
                Log.d(TAG, "addFromContacts click add button:" + addButton);
                mSolo.clickOnText(addButton);
                mSolo.sleep(FIVE_SECONDS);

                mSolo.sleep(FIVE_SECONDS);
                Log.d(TAG, "addFromContacts click image button");
                mSolo.clickOnImage(0);
                String[] fromWhere = mActivity.getResources().getStringArray(R.array.call_reject_dialog_array);
                /*
                if (mSolo.searchText(fromWhere[0])) {
                    Log.d(TAG, "addFromContacts: select add from contacts" + fromWhere[0]);
                    mSolo.clickOnText(fromWhere[0]);
                    mSolo.sleep(TWO_SECONDS);

                    Log.d(TAG, "addFromContacts:DOWN and ENTER");
                    mSolo.sendKey(Solo.DOWN);
                    mSolo.sleep(FIVE_SECONDS);
                    mSolo.sendKey(Solo.ENTER);
                    mSolo.sleep(FIVE_SECONDS);
                    //mSolo.clickOnImage(0);
                    //Log.d(TAG, "addFromContacts: select add from contacts 5");
                    //mSolo.clickInList(6);
                    //mSolo.clickOnCheckBox(0);
                    String okButton = mActivity.getString(android.R.string.ok);
                    if (mSolo.searchText(okButton)) {
                        Log.d(TAG, "addFromContacts find delete button ok" + okButton);
                        mSolo.clickOnText(okButton);
                        mSolo.sleep(FIVE_SECONDS);
                        Log.d(TAG, "addFromContacts back to CallRejectSetting1");
                        mSolo.goBack();
                        return;
                    } 

                    mSolo.sleep(TWO_SECONDS);
                    Log.d(TAG, "addFromContacts back to CallRejectSetting2");
                    mSolo.goBack();
                    mSolo.sleep(TWO_SECONDS);
                    mSolo.goBack();
                    return;	
               }*/
            }	
        }

        mSolo.sleep(FIVE_SECONDS);
        Log.d(TAG, "addFromContacts back to CallRejectSetting3");
        mSolo.goBack();
        mSolo.sleep(FIVE_SECONDS);
        mSolo.goBack();
    }

    private void addFromCalllogs() {
        Log.d(TAG, "addFromCalllogs");
        String rejectList = mActivity.getString(R.string.voice_call_reject_list_title);
        if (mSolo.searchText(rejectList)) {
            Log.d(TAG, "addFromCalllogs click reject list:" + rejectList);
            mSolo.clickOnText(rejectList);
            mSolo.sleep(FIVE_SECONDS);

            //search add button
            String addButton = mActivity.getString(R.string.call_reject_list_add);
            if (mSolo.searchText(addButton)) {
                Log.d(TAG, "addFromCalllogs click add button:" + addButton);
                mSolo.clickOnText(addButton);
                mSolo.sleep(FIVE_SECONDS);

                mSolo.sleep(FIVE_SECONDS);
                Log.d(TAG, "addFromCalllogs click image button");
                mSolo.clickOnImage(0);
                String[] fromWhere = mActivity.getResources().getStringArray(R.array.call_reject_dialog_array);
/*
if (mSolo.searchText(fromWhere[1])) {
	Log.d(TAG, "addFromContacts: select add from contacts" + fromWhere[1]);
	mSolo.clickOnText(fromWhere[1]);
	mSolo.sleep(TWO_SECONDS);
    
	Log.d(TAG, "addFromContacts: click image 0 and image 2");
	Solo tempSolo = new Solo(mInst, getActivity());
	tempSolo.clickOnImage(0);
	tempSolo.sleep(FIVE_SECONDS);
	tempSolo.clickOnImage(3);
		
	Log.d(TAG, "addFromContacts back to CallRejectSetting2");
	mSolo.sleep(TWO_SECONDS);
	mSolo.goBack();
	mSolo.sleep(TWO_SECONDS);
	mSolo.goBack();
	return;	
}*/
            }
        }

        mSolo.sleep(TWO_SECONDS);
        Log.d(TAG, "addFromContacts back to CallRejectSetting3");
        mSolo.goBack();
        mSolo.sleep(TWO_SECONDS);
        mSolo.goBack();
    }
    
    private void modeSwtich(String modeText) {
        Log.d(TAG, "modeSwtich:" + modeText);
        String[] callRejectModeArray = mActivity.getResources().getStringArray(R.array.call_reject_mode_entries);

        //mode 0
        Log.d(TAG, "modeSwtich:click0: " + modeText);
        mSolo.clickOnText(modeText);
        mSolo.sleep(TWO_SECONDS);
        Log.d(TAG, "modeSwtich:mode select 0:" + callRejectModeArray[0]);
        mSolo.clickOnText(callRejectModeArray[0]);
        mSolo.sleep(FIVE_SECONDS);

        //mode 1
        Log.d(TAG, "modeSwtich:click1: " + modeText);
        mSolo.clickOnText(modeText);
        mSolo.sleep(FIVE_SECONDS);
        Log.d(TAG, "modeSwtich:mode select 1:" + callRejectModeArray[1]);
        mSolo.clickOnText(callRejectModeArray[1]);
        String okText = mActivity.getString(android.R.string.ok);
        if (mSolo.searchText(okText)) {
            Log.d(TAG, "modeSwtich:mode select 1, then confirm ok button");
            mSolo.clickOnText(okText);
            mSolo.sleep(FIVE_SECONDS);
        }
        
        //mode 2
        Log.d(TAG, "modeSwtich:click2: " + modeText);
        mSolo.clickOnText(modeText);
        mSolo.sleep(FIVE_SECONDS);
        Log.d(TAG, "modeSwtich:mode select 2:" + callRejectModeArray[2]);
        mSolo.clickOnText(callRejectModeArray[2]);
        mSolo.sleep(FIVE_SECONDS);
    }


    public void test01VoiceRejectModeSwitch() {
        Log.d(TAG, "test01VoiceRejectModeSwitch");
        String modeText = mActivity.getString(R.string.voice_call_reject_mode_title);
        if (mSolo.searchText(modeText)) {
            modeSwtich(modeText);
        }
        
    }


    public void test02VideoRejectModeSwitch() {
        Log.d(TAG, "test01VideoRejectModeSwitch");
        if (!FeatureOption.MTK_VT3G324M_SUPPORT) return;
        String modeText = mActivity.getString(R.string.video_call_reject_mode_title);
        Log.d(TAG, "test01VideoRejectModeSwitch ok");
        if (mSolo.searchText(modeText)) {
            modeSwtich(modeText);
        }
        
    }
	

    public void test03VoiceRejectListAdd() {
        Log.d(TAG, "test03VoiceRejectListAdd");
        addFromInput();
        addFromContacts();
        //addFromCalllogs();
    }
	
    public void test04VoiceRejectListDelete() {
        Log.d(TAG, "test04VoiceRejectListDelete");
        String rejectList = mActivity.getString(R.string.voice_call_reject_list_title);
        if (mSolo.searchText(rejectList)) {
            Log.d(TAG, "test04VoiceRejectListDelete click reject list:" + rejectList);
            mSolo.clickOnText(rejectList);
            mSolo.sleep(FIVE_SECONDS);

            //search delete button
            String deleteButton = mActivity.getString(R.string.call_reject_list_delete);
            if (mSolo.searchText(deleteButton)) {
                Log.d(TAG, "test04VoiceRejectListDelete click delete btn:" + deleteButton);
                mSolo.clickOnText(deleteButton);
                mSolo.sleep(FIVE_SECONDS);

                String selectText = mActivity.getString(R.string.selected_item_count);
                String selectTextNew = findSelect(selectText);
                if (mSolo.searchText(selectTextNew)) {
                    //Log.d(TAG, "test04VoiceRejectListDelete click action bar:" + selectTextNew);
                    //mSolo.clickOnView(findView(R.id.select_items));
 
                    Log.d(TAG, "test04VoiceRejectListDelete click action bar:" + selectTextNew);
                    mSolo.clickOnText(selectTextNew);
                    mSolo.sleep(FIVE_SECONDS);
                    //select_all  select_clear

                    String selectAll = mActivity.getString(R.string.select_all);
                    String selectClear = mActivity.getString(R.string.select_clear);

                    if (mSolo.searchText(selectAll)) {
                        Log.d(TAG, "test04VoiceRejectListDelete click action bar:" + selectText);
                        mSolo.clickOnText(selectAll);
                    } else {
                        Log.d(TAG, "test04VoiceRejectListDelete go back modify list");
                        mSolo.goBack();
                    }
                    String okButton = mActivity.getString(android.R.string.ok);
                    if (mSolo.searchText(okButton)) {
                        Log.d(TAG, "test04VoiceRejectListDelete find delete button ok" + okButton);
                        mSolo.clickOnText(okButton);
                        mSolo.sleep(FIVE_SECONDS);
                        Log.d(TAG, "test04VoiceRejectListDelete go to CallRejectSetting1");
                        mSolo.goBack();
                        return;
                    } 
                }
			}
       }
    }

    public void test05VideoRejectListAdd() {
            Log.d(TAG, "test05VideoRejectListAdd");
            if (!FeatureOption.MTK_VT3G324M_SUPPORT) return;
            String rejectList = mActivity.getString(R.string.video_call_reject_list_title);
            if (mSolo.searchText(rejectList)) {
                Log.d(TAG, "test05VideoRejectListAdd click reject list:" + rejectList);
                mSolo.clickOnText(rejectList);
                mSolo.sleep(FIVE_SECONDS);

                //search add button
                String addButton = mActivity.getString(R.string.call_reject_list_add);
                if (mSolo.searchText(addButton)) {
                    Log.d(TAG, "test05VideoRejectListAdd click add button:" + addButton);
                    mSolo.clickOnText(addButton);
                    mSolo.sleep(FIVE_SECONDS);
                    Log.d(TAG, "test05VideoRejectListAdd input reject number: 10086");
                    mSolo.enterText(0, "10086");
                    mSolo.sleep(FIVE_SECONDS);

                    String saveText = mActivity.getString(R.string.save);
                    if (mSolo.searchText(saveText)) {
                        Log.d(TAG, "test05VideoRejectListAdd click the confirm:" + saveText);
                        mSolo.clickOnText(saveText);
                        mSolo.sleep(FIVE_SECONDS);
                        Log.d(TAG, "test05VideoRejectListAdd toback to CallRejectSetting");
                        mSolo.goBack();
                    }
                 }
             }   
        }

        public void test06VideoRejectListDelete() {
            Log.d(TAG, "test06VideoRejectListDelete");
            if (!FeatureOption.MTK_VT3G324M_SUPPORT) return;
            String rejectList = mActivity.getString(R.string.video_call_reject_list_title);
            if (mSolo.searchText(rejectList)) {
                Log.d(TAG, "test06VideoRejectListDelete click reject list:" + rejectList);
                mSolo.clickOnText(rejectList);
                mSolo.sleep(FIVE_SECONDS);

                //search delete button
                String deleteButton = mActivity.getString(R.string.call_reject_list_delete);
                if (mSolo.searchText(deleteButton)) {
                    Log.d(TAG, "test06VideoRejectListDelete click delete btn:" + deleteButton);
                    mSolo.clickOnText(deleteButton);
                    mSolo.sleep(FIVE_SECONDS);

                    String selectText = mActivity.getString(R.string.selected_item_count);
                    String selectTextNew = findSelect(selectText);
					Log.d(TAG, "test06VideoRejectListDelete click select:" + selectTextNew);
                    if (mSolo.searchText(selectTextNew)) {
                        //Log.d(TAG, "test04VoiceRejectListDelete click action bar:" + selectTextNew);
                        //mSolo.clickOnView(findView(R.id.select_items));

                        Log.d(TAG, "test06VideoRejectListDelete click action bar:" + selectTextNew);
                        mSolo.clickOnText(selectTextNew);
                        mSolo.sleep(FIVE_SECONDS);
                        //select_all  select_clear

                        String selectAll = mActivity.getString(R.string.select_all);
                        String selectClear = mActivity.getString(R.string.select_clear);

                        if (mSolo.searchText(selectAll)) {
                            Log.d(TAG, "test06VideoRejectListDelete click action bar:" + selectText);
                            mSolo.clickOnText(selectAll);
                            String okButton = mActivity.getString(android.R.string.ok);
                            if (mSolo.searchText(okButton)) {
                                Log.d(TAG, "test06VideoRejectListDelete find delete button ok" + okButton);
                                mSolo.clickOnText(okButton);
                                mSolo.sleep(FIVE_SECONDS);
                                Log.d(TAG, "test06VideoRejectListDelete go to CallRejectSetting1");
                                mSolo.goBack();
                                mSolo.sleep(FIVE_SECONDS);
                                return;
                            }    
                        } 
                    } 
            }
        }
    }
}
