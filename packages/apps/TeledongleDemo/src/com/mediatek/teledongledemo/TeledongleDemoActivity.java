package com.mediatek.teledongledemo;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View.OnClickListener;
import android.widget.AdapterView.OnItemClickListener;
import com.mediatek.basic.Util.SimState;

import android.preference.*;

import android.view.View;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.tedongle.TedongleManager;
import android.tedongle.SignalStrength;
import com.android.internal.tedongle.ITedongleStateListener;
import android.tedongle.TedongleStateListener;
import android.tedongle.ServiceState;

import android.os.RemoteException;
import android.os.PowerManager.WakeLock;
import android.provider.Settings;
import android.widget.Checkable;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.Toast;
import android.content.res.Resources;
import com.mediatek.basic.Util;
import android.content.DialogInterface;
import android.widget.Toast;
import android.widget.CheckBox;

import android.net.ConnectivityManager;
import android.content.BroadcastReceiver;
import android.app.ActivityManager;

class SimItem {
    public String mName = null;
    public String mNumber = null;
    
    
	  /** UNKNOWN, invalid value */
    public static final int SIM_INDICATOR_UNKNOWN = -1;
    /** ABSENT, no SIM/USIM card inserted for this phone */
    public static final int SIM_INDICATOR_ABSENT = 0;
    /** RADIOOFF,  has SIM/USIM inserted but not in use . */
    public static final int SIM_INDICATOR_RADIOOFF = 1;
    /** LOCKED,  has SIM/USIM inserted and the SIM/USIM has been locked. */
    public static final int SIM_INDICATOR_LOCKED = 2;
	  /** INVALID : has SIM/USIM inserted and not be locked but failed to register to the network. */
    public static final int SIM_INDICATOR_INVALID = 3; 
	  /** SEARCHING : has SIM/USIM inserted and SIM/USIM state is Ready and is searching for network. */
    public static final int SIM_INDICATOR_SEARCHING = 4; 
	  /** NORMAL = has SIM/USIM inserted and in normal service(not roaming and has no data connection). */
    public static final int SIM_INDICATOR_NORMAL = 5; 
    /** ROAMING : has SIM/USIM inserted and in roaming service(has no data connection). */	
    public static final int SIM_INDICATOR_ROAMING = 6; 
	  /** CONNECTED : has SIM/USIM inserted and in normal service(not roaming) and data connected. */
    public static final int SIM_INDICATOR_CONNECTED = 7; 
	  /** ROAMINGCONNECTED = has SIM/USIM inserted and in roaming service(not roaming) and data connected.*/
    public static final int SIM_INDICATOR_ROAMINGCONNECTED = 8;
    
    /**
     * Normal operation condition, the phone is registered
     * with an operator either in home network or in roaming.
     */
    public static final int STATE_IN_SERVICE = 0;

    /**
     * 3gdongle is not registered with any operator, the phone
     * can be currently searching a new operator to register to, or not
     * searching to registration at all, or registration is denied, or radio
     * signal is not available.
     */
    public static final int STATE_OUT_OF_SERVICE = 1;

    /**
     * The phone is registered and locked.  Only emergency numbers are allowed. {@more}
     */
    public static final int STATE_EMERGENCY_ONLY = 2;

    /**
     * Radio of telephony is explicitly powered off.
     */
    public static final int STATE_POWER_OFF = 3;
    
    public SimItem(String name, String number) {
        mName = name;
        mNumber = number;
    }
}

public class TeledongleDemoActivity extends PreferenceActivity implements
			Preference.OnPreferenceClickListener {
	
	protected String LOG_TAG = "3GD-APK-DemoAct";
    protected final int EVENT_TEDONGLE_RADIO_STATE_CHANGED               = 100;
    protected final int EVENT_TEDONGLE_SIM_PIN_REQUIRE					 = 101;
    protected final int EVENT_TEDONGLE_SIM_PUK_REQUIRE 					 = 102;
    
    private static final int PROGRESS_DIALOG = 1000;
    private IntentFilter mIntentFilter;
    private CheckBox mCheckBox;
    private TedongleRadioEnablePre mSwitchPreference;
    private PreferenceScreen mSetupSimLockPre;
    private PreferenceScreen mDongleSupportListPre;
    private PreferenceScreen mTedongleSimInfoPre;
    private boolean radioEnable = true;
    private TedongleManager mTedongleManager;
    private PreferenceScreen mTedongleInfoPreferenceScreen;
	protected WakeLock mWakeLock;
    private IconPreferenceScreen mSignalStrengthPreference;
    private ConnectivityManager mConnService;
    
    private SignalStrength mSignalStrength;
    public int mSimState = SIM_STATE_UNKNOWN;
    //whether the sim unlock success or fail
    public boolean mSimunlock = true;
    private int mServiceState;
    private int mDataState;
    private String mNetWorkTypeName;
    
    private static final String TEDONGLE_INFO = "tedongle_network";
    private static final String TEDONGLE_RADIO_INFO = "tedongle_radio_information";
    private static final String TEDONGLE_RADIO_ENABLE ="tedongle_radio_enable";
    private static final String TEDONGLE_SIGNAL_STRENGTH="tedongle_signal_strength";
    private static final String TEDONGLE_SIM_STATUS="tedongle_siminfo";
    private static final String TEDONGLE_SETUP_SIM_LOCK="tedongle_sim_lock";
    private static final String TEDONGLE_SUPPORT_LIST="support_list";
  
    public static final String ACTION_SIM_STATE_CHANGED = "tedongle.intent.action.SIM_STATE_CHANGED";
    public static final String ACTION_DONGLE_INSTALL = "com.mediatek.dongle.install";
    public static final String ACTION_DONGLE_UNINSTALL = "com.mediatek.dongle.uninstall";
    public static final String ACTION_ANY_DATA_CONNECTION_STATE_CHANGED = "tedongle.intent.action.ANY_DATA_STATE";
	public static final String ACTION_CLOSE_WAITING = "com.mediatek.dongle.waiting";


	/** Data connection state: Unknown.  Used before we know the state.
	   * @hide
	   */
    public static final int DATA_UNKNOWN        = -1;
    /** Data connection state: Disconnected. IP traffic not available. */
    public static final int DATA_DISCONNECTED   = 0;
    /** Data connection state: Currently setting up a data connection. */
    public static final int DATA_CONNECTING     = 1;
    /** Data connection state: Connected. IP traffic should be available. */
    public static final int DATA_CONNECTED      = 2;
    /** Data connection state: Suspended. The connection is up, but IP
     * traffic is temporarily unavailable. For example, in a 2G network,
     * data activity may be suspended when a voice call arrives. */
    public static final int DATA_SUSPENDED      = 3;

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
    
    
    private static final int PIN_RESULT_OK = 1;
    private static final int PIN_RESULT_NOT_OK = 2;
    private static final int PUK_RESULT_OK = 3;
    private static final int PUK_RESULT_NOT_OK = 4;    
    public enum DataState {
        CONNECTED, CONNECTING, DISCONNECTED, SUSPENDED;
    };
    
	Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) { 
            switch (msg.what) {
				case EVENT_TEDONGLE_RADIO_STATE_CHANGED:
					break;
				case EVENT_TEDONGLE_SIM_PIN_REQUIRE:
					StartSimPinActivity();
					break;
				case EVENT_TEDONGLE_SIM_PUK_REQUIRE:
					StartSimPukActivity();
					break;
			}
		}
			
	};
	
    // Receiver to handle different actions
    private BroadcastReceiver mtedongleReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(LOG_TAG, "mtedongleReceiver receive action=" + action);
            // updated
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean airplaneMode = intent.getBooleanExtra("state", false);
				setPreferenceState(!airplaneMode);
				mSwitchPreference.setEnabled(!airplaneMode);
            } else if (action.equals(ACTION_SIM_STATE_CHANGED)) {
            	//change the preference ui
            	//makeToast("STATE_CHANGED");
            	checkSimState();
            } else if (action.equals(ACTION_DONGLE_UNINSTALL)) {
	            //makeToast("ACTION_DONGLE_UNINSTALL");
                StopTedonleDemo();
            }else if (action.equals(ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                DataState state = Enum.valueOf(DataState.class,
                        intent.getStringExtra("state"));
                switch (state) {
                	case DISCONNECTED:
                    	Log.d(LOG_TAG, "121212DISCONNECTED");						
                    	break;
                	case CONNECTED:
                    	Log.d(LOG_TAG, "121212CONNECTED");						
						StopWaitingActivity();
                    	break;
                	case CONNECTING:
						StartWaitingActivity();
                    	Log.d(LOG_TAG, "121212CONNECTING");
                    	break;
						default:
							break;
                }
                	
            }
        }
    };
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		initIntentFilter();
		mTedongleManager = TedongleManager.getDefault();
		
        
        ///M: initilize connectivity manager for consistent_UI
        mConnService = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);

		try {
            if (mTedongleManager != null) {
        		mDataState = mTedongleManager.getDataState();
                mServiceState = mTedongleManager.getServiceState().getState();
                mSignalStrength = mTedongleManager.getSignalStrength();
                mSimState = mTedongleManager.getSimState();
                Log.d(LOG_TAG, "mServiceState = " + mServiceState + "mDataState = " + mDataState +
                           " mSignalStrength = " + mSignalStrength.getLevel() +
                           " mSimState =" + mSimState);
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "mTeledongleManager exception");
        } 
        registerReceiver(mtedongleReceiver, mIntentFilter);

		InitUi();
		
		PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
		mWakeLock = powerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK,
				"Tedongle");
		Log.d(LOG_TAG, "onCreate end...");
	}
	
	public void setPreferenceState(boolean flag){
		radioEnable = flag;
		mSignalStrengthPreference.setEnabled(flag);		
		updateSignalStrength();
    	mSwitchPreference.setChecked(flag);
    	mTedongleInfoPreferenceScreen.setEnabled(flag);
    	mSetupSimLockPre.setEnabled(flag);
    	mTedongleSimInfoPre.setEnabled(flag);
	}
	//just for test!!!
	public void makeToast(String str) {
		Toast.makeText(this, str, Toast.LENGTH_SHORT).show();
	}
	
	public void checkSimState() {
		//3gdongle get sim state dongle state
        mSimState = mTedongleManager.getSimState();
		Log.d(LOG_TAG, " mSimState :" + mSimState);
		switch (mSimState) {
			case SIM_STATE_UNKNOWN:
			case SIM_STATE_ABSENT:
			case SIM_STATE_NOT_READY:
				setPreferenceState(false);
				mSwitchPreference.setEnabled(false);
            	//makeToast("sim state is:" + mSimState);
            	break;
			case SIM_STATE_READY:
				setPreferenceState(true);
				mSwitchPreference.setEnabled(true);
            	//makeToast("sim state is:" + mSimState);
            	break;
			case SIM_STATE_PIN_REQUIRED:
				final Builder builder = new AlertDialog.Builder(this);
				//builder.setIcon(R.drawable.ic_default_user);
				//builder.setTitle("PIN REQUIRED");
				builder.setMessage(R.string.dialog_title_pin);
				builder.setNegativeButton(R.string.dialog_no, new DialogInterface.OnClickListener(){
					public void onClick(DialogInterface dialog, int which) {
						//ProgressDialog m_Dialog = ProgressDialog.show(this, "3333", "4444", true);
						//DisablePreferenceUiForPinPuk();
						System.exit(0);
					}
				}).setPositiveButton(R.string.dialog_yes, new DialogInterface.OnClickListener(){
					@Override
					public void onClick(DialogInterface dialog, int whcih ) {
						//makeToast("yes click!");
						//StartSimPinActivity();
						mHandler.sendMessage(mHandler.obtainMessage(EVENT_TEDONGLE_SIM_PIN_REQUIRE));
					}
				});
				builder.setCancelable(false);
				builder.create().show();
				break;
			case SIM_STATE_PUK_REQUIRED:
				final Builder mbuilder = new AlertDialog.Builder(this);
				//mbuilder.setIcon(R.drawable.ic_default_user);
				//mbuilder.setTitle("PUK REQUIRED");
				mbuilder.setMessage(R.string.dialog_title_puk);
				mbuilder.setNegativeButton(R.string.dialog_no, new DialogInterface.OnClickListener(){
					public void onClick(DialogInterface dialog, int which) {
						//ProgressDialog m_Dialog = ProgressDialog.show(this, "3333", "4444", true);
						//DisablePreferenceUiForPinPuk();
						System.exit(0);
					}
				}).setPositiveButton(R.string.dialog_yes, new DialogInterface.OnClickListener(){
					@Override
					public void onClick(DialogInterface dialog, int whcih ) {
						mHandler.sendMessage(mHandler.obtainMessage(EVENT_TEDONGLE_SIM_PUK_REQUIRE));
					}
				});
				mbuilder.create().show();
				break;
				
		}
	}

	public void StopTedonleDemo(){
		ActivityManager am =(ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
		am.forceStopPackage("com.mediatek.teledongledemo");
	}
 	public void StopWaitingActivity() {
		Intent intent = new Intent();
		intent.setAction(ACTION_CLOSE_WAITING);
		sendBroadcast(intent);		
	}   
	public void StartWaitingActivity() {
		Intent intent = new Intent(this, WaitingActivity.class);
		startActivity(intent);
	}
	
	public void StartSimPinActivity() {
		Intent intent = new Intent(this, TedongleSimPinView.class);
		//startActivity(intent);
        startActivityForResult(intent, PIN_RESULT_OK);
	}
	
	public void StartSimPukActivity() {
		Intent intent = new Intent(this, TedongleSimPukView.class);
		//startActivity(intent);
        startActivityForResult(intent, PUK_RESULT_OK);
	}
	

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
        	case PIN_RESULT_OK:
        		
        		break;
        	case PUK_RESULT_OK:
        		
        		break;
        }
    }
	public void InitUi() {
		addPreferencesFromResource(R.layout.activity_teledongle_demo);
		mTedongleInfoPreferenceScreen = (PreferenceScreen)findPreference(TEDONGLE_INFO);
		mTedongleInfoPreferenceScreen.setOnPreferenceClickListener(this);
		
		mSwitchPreference = (TedongleRadioEnablePre)findPreference(TEDONGLE_RADIO_ENABLE);
		mSwitchPreference.setOnPreferenceClickListener(this);
		mSwitchPreference.setUiActivity(this);

		radioEnable  = mSwitchPreference.isChecked();
		Log.d(LOG_TAG, "INITUI radioEnable1:" + radioEnable + " dongleinsert:" + isDonglePluged()
				+ " simready:" + isSimReady());
		mSwitchPreference.setChecked((isDonglePluged() && isSimReady()));
		
		mSignalStrengthPreference = (IconPreferenceScreen)findPreference(TEDONGLE_SIGNAL_STRENGTH);
		
		mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_0));
		mSignalStrengthPreference.setOnPreferenceClickListener(this);
		
		mTedongleSimInfoPre = (PreferenceScreen)findPreference(TEDONGLE_SIM_STATUS);
		mTedongleSimInfoPre.setOnPreferenceClickListener(this);
		
		mSetupSimLockPre = (PreferenceScreen)findPreference(TEDONGLE_SETUP_SIM_LOCK);
		mSetupSimLockPre.setOnPreferenceClickListener(this);
		
		mDongleSupportListPre = (PreferenceScreen)findPreference(TEDONGLE_SUPPORT_LIST);
		mDongleSupportListPre.setOnPreferenceClickListener(this);
		
		//checkSimState();
	}
    
    /**
     * @return is airplane mode or all sim card is set on radio off
     * 
     */
    private boolean isRadioOff() {
        boolean isAllRadioOff = (Settings.System.getInt(getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, -1) == 1)
                || radioEnable;
        Log.d(LOG_TAG, "isAllRadioOff=" + isAllRadioOff);
        return isAllRadioOff;
    }
    
	@Override
	public void onPause(){
		super.onPause();
		mWakeLock.release();
	}

	@Override
	public void onResume() {
		super.onResume();
		mWakeLock.acquire();

		Log.d(LOG_TAG, "onResume()...");		
        updateDataState();
        updateNetworkType();
		if(mDataState != DATA_CONNECTED && mSimState != SIM_STATE_PIN_REQUIRED 
			&& mSimState != SIM_STATE_PUK_REQUIRED){
			StartWaitingActivity();
		}
        mServiceState = mTedongleManager.getServiceState().getState();
        mSignalStrength = mTedongleManager.getSignalStrength();
        updateSignalStrength();
        mTedongleManager.listen(mPhoneStateListener,
        		TedongleStateListener.LISTEN_DATA_CONNECTION_STATE
                        | TedongleStateListener.LISTEN_SIGNAL_STRENGTHS
                        | TedongleStateListener.LISTEN_SERVICE_STATE);

	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mtedongleReceiver);
	}
	
    private void initIntentFilter() {
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mIntentFilter.addAction(ACTION_SIM_STATE_CHANGED);
        mIntentFilter.addAction(ACTION_DONGLE_UNINSTALL);
        mIntentFilter.addAction(ACTION_DONGLE_INSTALL);
        mIntentFilter.addAction(ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
    }
      
    private boolean isSimLocked() {
        boolean isLocked = false;
        /*try {          
        	isLocked = mTedongleManager.isSimLocked();
        } catch (Exception e) {
            Log.d(LOG_TAG, "[e = " + e + "]");
        }*/
        return isLocked;
    }
    
    private boolean isDonglePluged() {
    	return mTedongleManager.isDonglePluged();
    }
    
    private boolean isSimReady() {
    	return mTedongleManager.isSimReady();
    }
    
    private boolean setRadio(boolean value) {
    	return mTedongleManager.setRadio(value);
    }
          
    public void switchPreferenceValue(Preference preference, boolean value) {   
    		Log.d(LOG_TAG, "value :" + value);
			setPreferenceState(value);
    		setRadio(value);
    }
    
    public boolean getRadioState(){
    	 return radioEnable;
    }
    
    @Override
    public boolean onPreferenceClick(Preference preference) {
    	//add ---
    	//operatePreference(preference);
    	//
    	return false;
    }
    public boolean OnPreferenceTreeClick(PreferenceScreen preferenceScreen,
    		Preference preference){
    	Log.d(LOG_TAG, "OnPreferenceTreeClick-->" + preference.getKey());
    	//operatePreference(preference);
    	if (preference.getKey().equals(TEDONGLE_INFO)) {
    		Intent mIntent = new Intent(this, TedongleInfo.class);
    		mIntent.putExtra("radioState", radioEnable);
    		startActivity(mIntent);
    		return true;
    	} else if(preference.getKey().equals(TEDONGLE_SIM_STATUS)) {
    		Intent mIntent = new Intent(this, TedongleSimInfo.class);
    		mIntent.putExtra("radioState", radioEnable);
    		startActivity(mIntent);
    		return true;
    	} else if(preference.getKey().equals(TEDONGLE_SETUP_SIM_LOCK)) {
    		Intent mIntent = new Intent(this, TedongleSimLockSet.class);
    		mIntent.putExtra("radioState", radioEnable);
    		startActivity(mIntent);
    		return true;
    	}
    	return false;
    }
    
    public boolean onPreferenceChange(Preference preference, Object objValue) {
    	Log.d(LOG_TAG, "onPreferenceChange-->" + String.valueOf(preference.getKey()));
	    if (preference == mSwitchPreference) {
	    	Log.d(LOG_TAG, "onPreferenceChange-->" + String.valueOf(objValue));
			return true;
	   }
	   return false;
    }
    
    void updateSignalStrength() {
        Log.d(LOG_TAG, "updateSignalStrength()");
        // TODO PhoneStateIntentReceiver is deprecated and PhoneStateListener
        // should probably used instead.
        
        
        //3gdongle for signal
        //makeToast(" mServiceState :" + mServiceState);
        // not loaded in some versions of the code (e.g., zaku)
        if (mSignalStrengthPreference != null) {
            if ((ServiceState.STATE_OUT_OF_SERVICE == mServiceState)
                    || (ServiceState.STATE_POWER_OFF == mServiceState)
                    || (radioEnable == false)) {
				//makeToast(" radioEnable :" + radioEnable);
            	Log.d(LOG_TAG, "ServiceState is Not ready, set signalStrength 0");
                mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_0));
				return;
			} 
            
            int iconLevel = mSignalStrength.getLevel();
			Log.d(LOG_TAG,"iconLevel "+ iconLevel);
			//makeToast("iconLevel :" + iconLevel);
			if(iconLevel == 0){
				mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_0));
			}else if(iconLevel == 1){
				mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_1));
			}else if(iconLevel == 2){
				mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_2));
			}else if(iconLevel == 3){
				mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_3));
			}else if(iconLevel == 4){
				mSignalStrengthPreference.setIcon(getResources().getDrawable(R.drawable.ic_wifi_signal_4));
			}
        }
    }
    
    private void updateNetworkType() {
        // Whether EDGE, UMTS, etc...
        mNetWorkTypeName = mTedongleManager.getNetworkTypeName();
        Log.d(LOG_TAG,"getNetworkTypeName "+ mNetWorkTypeName);
    }

    private void updateDataState() {
        mDataState = mTedongleManager.getDataState();
        Log.d(LOG_TAG,"getNetworkTypeName "+ mDataState);
    }

    
    private TedongleStateListener mPhoneStateListener = new TedongleStateListener() {

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            Log.d(LOG_TAG, "onDataConnectionStateChanged");
            updateDataState();
            updateNetworkType();
        }

        @Override
        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
        	    mSignalStrength = signalStrength;
                Log.d(LOG_TAG, "SignalStrengthsChanged mSignalStrength : " + mSignalStrength);
                updateSignalStrength();
            
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
                mServiceState = serviceState.getState();
                Log.d(LOG_TAG, "ServiceStateChanged mServiceState : " + mServiceState);
                updateSignalStrength();
        }
    };
    
    
}
