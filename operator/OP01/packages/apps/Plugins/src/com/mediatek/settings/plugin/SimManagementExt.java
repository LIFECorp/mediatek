package com.mediatek.settings.plugin;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.preference.DialogPreference;

import android.provider.Settings;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.settings.ext.DefaultSimManagementExt;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class SimManagementExt extends DefaultSimManagementExt {
    
    private static final String TAG = "SimManagementExt";
    
    private static final String KEY_3G_SERVICE_SETTING = "3g_service_settings";
    private static final String KEY_AUTO_WAP_PUSH = "wap_push_settings";
    private static final String KEY_SIM_STATUS = "status_info";
    
    private Context mContext;
    PreferenceFragment mPrefFragment;
    private AlertDialog mAlertDlg;
    private ProgressDialog mWaitDlg;
    private boolean mIsDataSwitchWaiting = false;
    private int mToCloseSlot = -1;
    private static final String KEY_VOICE_CALL_SIM_SETTING = "voice_call_sim_setting";
	
    public static final int[] sSIMColors = new int[] {
        0xFF0000DD, /* blue  */
        0XFFDF8326, /*orange*/
        0xFF00DD00, /*green */
        0XFFAA66CC  /*purple*/
    };
    private static final int DATA_SWITCH_TIME_OUT_MSG = 2000;
    private static final int DATA_SWITCH_TIME_OUT_TIME = 10000;

    //Timeout handler
    private Handler mTimerHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (DATA_SWITCH_TIME_OUT_MSG == msg.what) {
                
                Xlog.i(TAG, "reveive time out msg...");
                if (mIsDataSwitchWaiting) {
                    
                    mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
                    mWaitDlg.dismiss();
                    mIsDataSwitchWaiting = false;
                }
            }
        }
    };
    
    /**
     * update the preference screen of sim management
     * @param parent parent preference
     */
    public SimManagementExt(Context context) {
        super();
        mContext = context;
    }
    
    public void updateSimManagementPref(PreferenceGroup parent) {
        
        Xlog.d(TAG,"updateSimManagementPref()");
        PreferenceScreen pref3GService = null;
        PreferenceScreen prefWapPush = null;
        PreferenceScreen prefStatus = null;
        if (parent != null) {
            pref3GService = (PreferenceScreen)parent.findPreference(KEY_3G_SERVICE_SETTING);
            prefWapPush = (PreferenceScreen)parent.findPreference(KEY_AUTO_WAP_PUSH);
            prefStatus = (PreferenceScreen)parent.findPreference(KEY_SIM_STATUS);
        }
        if (pref3GService != null) {
            Xlog.d(TAG,"updateSimManagementPref()---remove pref3GService");
            parent.removePreference(pref3GService);
        }
        if (prefWapPush != null) {
            Xlog.d(TAG,"updateSimManagementPref()---remove prefWapPush");
            parent.removePreference(prefWapPush);
        }
        if (prefStatus != null) {
            Xlog.d(TAG,"updateSimManagementPref()---remove prefStatus");
            parent.removePreference(prefStatus);
        }
    }
    public void updateSimEditorPref(PreferenceFragment pref) {
        pref.getPreferenceScreen().removePreference(pref.findPreference("sim_color"));
    }
    public void dealWithDataConnChanged(Intent intent, boolean isResumed) {
        
        Xlog.d(TAG, "dealWithDataConnChanged: mToClosedSimCard is " + mToCloseSlot);
        //remove confrm dialog
        long curConSimId = Settings.System.getLong(mContext.getContentResolver(),
                Settings.System.GPRS_CONNECTION_SIM_SETTING,
                Settings.System.DEFAULT_SIM_NOT_SET);
        
        if (mToCloseSlot >= 0) {
            
            long toCloseSimId = getSimIdBySlot(mToCloseSlot);
            Xlog.i(TAG, "dealWithDataConnChanged: toCloseSimId is " + toCloseSimId);
            Xlog.i(TAG, "dealWithDataConnChanged: curConSimId is " + curConSimId);
            
            if (toCloseSimId != curConSimId) {
                if(mAlertDlg != null && mAlertDlg.isShowing() && isResumed) {
                    Xlog.d(TAG, "dealWithDataConnChanged dismiss AlertDlg"); 
                    mAlertDlg.dismiss();
                    mToCloseSlot = -1;
                }
            }
        }
        //remove waiting dialog
        if (intent != null) {
            String apnTypeList = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
            PhoneConstants.DataState state = getMobileDataState(intent);
            
            if ((state == PhoneConstants.DataState.CONNECTED)
                    || (state == PhoneConstants.DataState.DISCONNECTED)) {
                    
                if ((PhoneConstants.APN_TYPE_DEFAULT.equals(apnTypeList))) {
                    
                    if (mIsDataSwitchWaiting) {
                        
                        mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
                        mWaitDlg.dismiss();
                        mIsDataSwitchWaiting = false;
                    }
            
                }
            }
        }
    }
    
    public void updateDefaultSIMSummary(DialogPreference pref, Long simId) {
        if (simId == Settings.System.SMS_SIM_SETTING_AUTO){
            pref.setSummary(mContext.getString(R.string.gemini_default_sim_auto));
        }
    }

    public void customizeSmsChoiceArray(List<String> listItem) {
        listItem.add(mContext.getString(R.string.gemini_default_sim_auto));
    }

    public void customizeSmsChoiceValueArray(List<Long> listItemValue) {
        listItemValue.add(Settings.System.SMS_SIM_SETTING_AUTO);
    } 

    public void showChangeDataConnDialog(PreferenceFragment prefFragment, boolean isResumed) {
        
        Xlog.d(TAG, "showChangeDataConnDialog");
        mPrefFragment = prefFragment;
        
        if(mToCloseSlot >= 0 && SimInfoManager.getInsertedSimInfoList(mContext).size() > 1) {
            long curConSimId = Settings.System.getLong(mContext.getContentResolver(),
                    Settings.System.GPRS_CONNECTION_SIM_SETTING,
                    Settings.System.DEFAULT_SIM_NOT_SET);
            
            long toCloseSimId = getSimIdBySlot(mToCloseSlot);
            Xlog.d(TAG, "toCloseSimId= " + toCloseSimId + "curConSimId= " + curConSimId);            
            if (mAlertDlg != null && mAlertDlg.isShowing()) {
                Xlog.d(TAG, "mAlertDlg.isShowing(), return");
                return;
            }
            
            if (toCloseSimId == curConSimId && isResumed) {
                
                Builder builder = new AlertDialog.Builder(prefFragment.getActivity());
                mAlertDlg = getChangeDataConnDialog(builder);
                mAlertDlg.show();
            } 
        }

    }
    
    public void setToClosedSimSlot(int simSlot) {
        Xlog.d(TAG, "setToClosedSimSlot = " + simSlot);
        mToCloseSlot = simSlot;
        if(mToCloseSlot >= 0 && SimInfoManager.getInsertedSimInfoList(mContext).size() > 1) {
            long curVoiceSimId = Settings.System.getLong(mContext.getContentResolver(),
                    Settings.System.VOICE_CALL_SIM_SETTING,
                    Settings.System.DEFAULT_SIM_NOT_SET);
            
            long toCloseSimId = getSimIdBySlot(mToCloseSlot);
            
            if (toCloseSimId == curVoiceSimId) {
                long simId = getSimIdBySlot(1 - mToCloseSlot);
                switchVoiceCallDefaultSim(simId);
            }
        }
    }

    private AlertDialog getChangeDataConnDialog(Builder builder) {
        
        Xlog.d(TAG, "getChangeDataConnDialog");
        
        SimInfoManager.SimInfoRecord currentSiminfo = 
               SimInfoManager.getSimInfoBySlot(mContext, mToCloseSlot);
        
        SimInfoManager.SimInfoRecord anotherSiminfo = 
               SimInfoManager.getSimInfoBySlot(mContext, 1 - mToCloseSlot);
        
        String currentSimName = currentSiminfo.mDisplayName;
        String anotherSimName = anotherSiminfo.mDisplayName;
        
        int currentSimColor = sSIMColors[currentSiminfo.mColor];
        int anotherSimColor = sSIMColors[anotherSiminfo.mColor];
        
        Xlog.d(TAG, "mToClosedSimCard = " + mToCloseSlot);
        
        String message = String.format(currentSimName +
               mContext.getString(R.string.change_data_conn_message) +
               anotherSimName + 
               "?");
        
        int currentSimStartIdx = message.indexOf(currentSimName);
        int currentSimEndIdx = currentSimStartIdx + currentSimName.length();
        
        int anotherSimStartIdx = currentSimEndIdx
                + (mContext.getString(R.string.change_data_conn_message)).length();
        int anotherSimEndIdx = anotherSimStartIdx + anotherSimName.length();
                    
        SpannableStringBuilder style = new SpannableStringBuilder(message);
        
        style.setSpan(new BackgroundColorSpan(currentSimColor), currentSimStartIdx,
               currentSimEndIdx, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        
        style.setSpan(new BackgroundColorSpan(anotherSimColor), anotherSimStartIdx,
               anotherSimEndIdx, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                     
        builder.setTitle(mContext.getString(R.string.change_data_conn_title));
        builder.setMessage(style);
        
        builder.setPositiveButton(android.R.string.yes, 
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Xlog.d(TAG, "Perform On click ok"); 
                           long simId = getSimIdBySlot(1 - mToCloseSlot);
                           Xlog.d(TAG, "Auto Switch GPRS Sim id = " + simId);
                           mToCloseSlot = -1;
                           switchGprsDefautSIM(simId);
                       }
                       });
        
        builder.setNegativeButton(android.R.string.cancel,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Xlog.d(TAG, "Perform On click cancel"); 
                           mToCloseSlot = -1;
                       }
                       });
        
        return builder.create();
        
    }
    
    private long getSimIdBySlot(int slotId) {
        Xlog.d(TAG, "SlotId = " + slotId);
        if(slotId < 0 || slotId > 1) {
            return -1;
        }
        SimInfoManager.SimInfoRecord simInfo = SimInfoManager.getSimInfoBySlot(mContext, slotId);
        long simId = -1;
        if (simInfo != null) {
            simId = simInfo.mSimInfoId;
        }
        Xlog.d(TAG, "GetSimIdBySlot: Sim id = " + simId 
                + "sim Slot = " + slotId);
        return simId;
    }
    
    private void switchGprsDefautSIM(long simid) {
        
        Xlog.d(TAG, "switchGprsDefautSIM() with simid=" + simid);
        
        if (simid < 0) {
            return;
        }
        long curConSimId = Settings.System.getLong(mContext.getContentResolver(),
            Settings.System.GPRS_CONNECTION_SIM_SETTING,
            Settings.System.DEFAULT_SIM_NOT_SET);
        Xlog.d(TAG,"curConSimId=" + curConSimId);
        
        if (simid == curConSimId) {
            return;
        }
        Intent intent = new Intent(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
        intent.putExtra("simid", simid);
        mContext.sendBroadcast(intent); 
        showDataConnWaitDialog();
        
    }
    
    private void switchVoiceCallDefaultSim(long simid) {
        Xlog.d(TAG, "switchVoiceCallDefaultSim() with simid=" + simid);
        if(simid < 0) {
            return;
        }
        Settings.System.putLong(mContext.getContentResolver(),
                Settings.System.VOICE_CALL_SIM_SETTING, simid);
        Intent intent = new Intent(
                Intent.ACTION_VOICE_CALL_DEFAULT_SIM_CHANGED);
        intent.putExtra("simid", simid);
        mContext.sendBroadcast(intent);
        Xlog.d(TAG, "send broadcast voice call change with simid="
                + simid);

    }

    private void showDataConnWaitDialog() {
        
        mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
        mTimerHandler.sendEmptyMessageDelayed(DATA_SWITCH_TIME_OUT_MSG,
                DATA_SWITCH_TIME_OUT_TIME);
        
        mWaitDlg = new ProgressDialog(mPrefFragment.getActivity());
        mWaitDlg.setMessage(mContext.getString(R.string.change_data_conn_progress_message));
        mWaitDlg.setIndeterminate(true);
        mWaitDlg.setCancelable(false);
        mWaitDlg.show();
        
        mIsDataSwitchWaiting = true;
    }
    
    private PhoneConstants.DataState getMobileDataState(Intent intent) {
        
        String str = intent.getStringExtra(PhoneConstants.STATE_KEY);
        
        if (str != null) {
            return Enum.valueOf(PhoneConstants.DataState.class, str);
        } else {
            return PhoneConstants.DataState.DISCONNECTED;
        }
    }
}
