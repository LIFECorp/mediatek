package com.mediatek.dongle;

import android.app.ActivityManager;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Process; 
import android.os.Binder;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.Parcelable;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.tedongle.ServiceState;
import android.text.TextUtils;
import android.util.Log;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.io.DataOutputStream;
import java.io.IOException;
import android.util.Slog;
import android.os.SystemProperties;

import com.android.internal.tedongle.CallManager;
import com.android.internal.tedongle.TedonglePhoneNotifier;
import com.android.internal.tedongle.ITelephony;
import com.android.internal.tedongle.IccCard;
import com.android.internal.tedongle.Phone;
import com.android.internal.tedongle.PhoneBase;
import com.android.internal.tedongle.PhoneConstants;
import com.android.internal.tedongle.PhoneFactory;
import com.android.internal.tedongle.IccIoResult;
import com.android.internal.tedongle.IccUtils;
import com.android.internal.tedongle.CommandException;
import com.android.internal.tedongle.CommandException;
import com.android.internal.tedongle.IccFileHandler;
import com.android.internal.tedongle.PhoneProxy;
import com.android.internal.tedongle.IPhoneSubInfo;
import com.android.internal.tedongle.TelephonyProperties;
import com.android.internal.tedongle.ITedongle;
import android.os.SystemProperties;
import android.tedongle.SignalStrength;

//import com.android.internal.app.IBatteryStats;
//import com.android.server.am.BatteryStatsService;
import android.os.RemoteException;
import com.android.internal.tedongle.TelephonyIntents;
import android.content.pm.PackageManager;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import com.android.internal.tedongle.ITedongleStateListener;
import android.tedongle.TedongleStateListener;
import android.content.Intent;
import android.content.IntentFilter;

import android.app.PendingIntent;
import android.content.ComponentName;

public class TedongleService extends ITedongle.Stub {

    private static final boolean DBG = true ;
    private static final String LOG_TAG = "3GD-TedongleService";
    private Context mContext;
    private Phone mPhone;
	private boolean mIsDonglePluged = false;
    //private static SignalStrength mTegongleSignalstrength;
    private boolean mGetSingalStrength = false;
    //private final IBatteryStats mBatteryStats;
    
    private final BroadcastReceiver mReceiver = new TedongleReceiver();
    private boolean mIsAirplneMode = false;
    private Notification mNotification;
    private boolean mNotifyed = false;
    private SignalStrength mSignalStrength = new SignalStrength();

    private static final int TED_NOTIFICATION_ID = 991000;    

    private final ArrayList<IBinder> mRemoveList = new ArrayList<IBinder>();
    private final ArrayList<Record> mRecords = new ArrayList<Record>();

    private static class Record {

        IBinder binder;

        ITedongleStateListener callback;

        int callerUid;

        int events;
        
        @Override
        public String toString() {
            return "{ callerUid=" + callerUid +
                    " events=" + Integer.toHexString(events) + "}";
        }
    }



    protected static final int EVENT_TEDONGLE_RADIO_STATE_CHANGED               = 100;
    //tedongle signal strength
    protected static final int EVENT_TEDONGLE_SIGNAL_STRENGTH                   = 103;

    //3gdongle 
    protected static final int EVENT_TEDONGLE_MESSENGER_TEST                    = 104;
    protected Messenger replyMessenger = null;
    protected static final int EVENT_TEDONGLE_MANAGER = 120;
    // For async handler to identify request type
    protected Messenger replySimSetLockMSG = null;	
    protected Messenger replySimChangePin = null;
	private static final int MSG_ENABLE_ICC_PIN_COMPLETE = 121;
	private static final int MSG_CHANGE_ICC_PIN_COMPLETE = 122;
    private static final int MSG_ENABLE_ICC_PIN_COMPLETE_SERVICE = 123;
	private static final int MSG_CHANGE_ICC_PIN_COMPLETE_SERVICE = 124;
    private static final int MSG_ENABLE_ICC_PIN_COMPLETE_CLIENT = 125;
	private static final int MSG_CHANGE_ICC_PIN_COMPLETE_CLIENT = 126;


    /**
     * Handles client connections
     */
    private class AsyncServiceHandler extends Handler {

        AsyncServiceHandler(android.os.Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            Log.d(LOG_TAG, "handleMessage:" + msg.what);
            switch (msg.what) {
                case EVENT_TEDONGLE_RADIO_STATE_CHANGED:
                    updateDonglePlugState();
                    if (mIsDonglePluged && !mNotifyed) {
                        
                        Log.d(LOG_TAG, "The 3Gdongle is pluged");
                        notifyDonglePlug(true, mNotification);
                        mNotifyed = true;
                        Log.d(LOG_TAG, "install tedongleDemo apk!");
                        TeledongleDemoInstaller.install(mContext);
                        Intent it = new Intent();
                        it.setAction("com.mediatek.dongle.install");
                        mContext.sendBroadcast(it);
                    } else if (!mIsDonglePluged && mNotifyed) {
                    
                        Log.d(LOG_TAG, "The 3Gdongle is removed");
                        notifyDonglePlug(false, mNotification);
                        mNotifyed = false;
                        Log.d(LOG_TAG, "Uninstall tedongleDemo apk!");
                        TeledongleDemoInstaller.unInstall(mContext); 
                        Intent intent = new Intent();
                        intent.setAction("com.mediatek.dongle.uninstall");
                        mContext.sendBroadcast(intent);
                    }
                    break;
                 case EVENT_TEDONGLE_MESSENGER_TEST:
                    //3gdongle
                    Log.d(LOG_TAG, " 3344EVENT_TEDONGLE_MESSENGER_TEST");
                    replyMessenger = msg.replyTo;
                    Message mMessage = new Message();
                    mMessage.what = EVENT_TEDONGLE_MANAGER;
                    try {
                        replyMessenger.send(mMessage);
                    }catch (RemoteException e) {
                        //fjdsl
                    }
                    break;
                 case MSG_ENABLE_ICC_PIN_COMPLETE_SERVICE:
                    //3gdongle
                    Log.d(LOG_TAG, " 3344MSG_ENABLE_ICC_PIN_COMPLETE_SERVICE" + msg);
                    //replySimSetLockMSG = msg.replyTo;
                    //Message mMessage = new Message();
                    //mMessage.what = MSG_ENABLE_ICC_PIN_COMPLETE;
                    //try {
                    //    replySimSetLockMSG.send(mMessage);
                    //}catch (RemoteException e) {
                        //fjdsl
                    //} 
                    replySimSetLockMSG = msg.replyTo;
                    Message mMessageSER = Message.obtain(mAsyncServiceHandler, MSG_ENABLE_ICC_PIN_COMPLETE_CLIENT);
                    //mMessageSER.what = MSG_ENABLE_ICC_PIN_COMPLETE_CLIENT;
                    boolean bl = msg.arg1 == 1 ? true:false;
                    String password = Integer.toString(msg.arg2);
                    setIccLockEnabled(bl, password, mMessageSER);
                    break;
                 case MSG_ENABLE_ICC_PIN_COMPLETE_CLIENT:
                    Log.d(LOG_TAG, " 3344MSG_ENABLE_ICC_PIN_COMPLETE_CLIENT" + msg);
                    Message mMsg = new Message();
                    mMsg.what = MSG_ENABLE_ICC_PIN_COMPLETE;
                    //iccLockChanged(ar.exception == null);
                    //AsyncResult.forMessage(mMsg).exception = ar.exception;
                   //((Message)ar.userObj).sendToTarget();
                    //mMsg.obj = ar;
                    int mArg = 0;
                    if (ar.exception == null) {
                        Log.d(LOG_TAG, " 3344MSG...CLIENT ar.exception" );
                        mArg = 1;
                    }
                    mMsg.arg1 = mArg;
                    try {
                        replySimSetLockMSG.send(mMsg);
                    }catch (RemoteException e) {
                        //fjdsl
                    } 
                    break;
                 case MSG_CHANGE_ICC_PIN_COMPLETE_SERVICE :
                    //3gdongle
                    Log.d(LOG_TAG, " 3344MSG_CHANGE_ICC_PIN_COMPLETE_SERVICE" + msg);
                    replySimChangePin = msg.replyTo;
                    Message mChangePinSer = Message.obtain(mAsyncServiceHandler, MSG_CHANGE_ICC_PIN_COMPLETE_CLIENT);
                    //mMessageSER.what = MSG_ENABLE_ICC_PIN_COMPLETE_CLIENT;
                    String oldPassWord = Integer.toString(msg.arg1);
                    String newPassWord = Integer.toString(msg.arg2);
                    changeIccLockPassword(oldPassWord, newPassWord, mChangePinSer);
                    break;
                 case MSG_CHANGE_ICC_PIN_COMPLETE_CLIENT :
                    Log.d(LOG_TAG, " 3344MSG_CHANGE_ICC_PIN_COMPLETE_CLIENT" + msg);
                    Message mChangePinCli = new Message();
                    mChangePinCli.what = MSG_CHANGE_ICC_PIN_COMPLETE;
                    int mArg1 = 0;
                    if (ar.exception == null) {
                        Log.d(LOG_TAG, " 3344MSG...change pin CLIENT" );
                        mArg1= 1;
                    }
                    mChangePinCli.arg1 = mArg1;
                    try {
                        replySimChangePin.send(mChangePinCli);
                    }catch (RemoteException e) {
                        //fjdsl
                    } 
                    break;
                 default: {
                    Log.d(LOG_TAG, "tedongleservicehandler.handleMessage ignoring msg=" + msg);
                    break;
                }
            }
        }
    }
    private AsyncServiceHandler mAsyncServiceHandler;


     private class TedongleReceiver extends BroadcastReceiver {

         public void onReceive(Context context, Intent intent) {
    
             String action = intent.getAction();
             Log.d(LOG_TAG, "TedongleReceiver -----action=" + action);
             
             if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                 
                 boolean enabled = intent.getBooleanExtra("state", false);
                 Log.d(LOG_TAG, "TedongleReceiver ------ AIRPLANEMODE enabled=" + enabled);
                 mIsAirplneMode = enabled;
                 setRadio(!enabled);
    
             }
    
         }
    }


    /** The singleton instance. */
    private static TedongleService sInstance;
	/**
     * Initialize the singleton TedongleService instance.
     * This is only done once, at startup, from dongle.onCreate().
     */
    /* package */ static TedongleService init(Context mctext) {
	
        synchronized (TedongleService.class) {
            if (sInstance == null) {
                sInstance = new TedongleService(mctext);
                Log.d(LOG_TAG, "create TedongleService");
            } else {
                Log.d(LOG_TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
            return sInstance;
        }
    }
	
    private TedongleService(Context mct) {
	
		mContext = mct;
		PhoneFactory.makeDefaultPhones(mContext);
        mPhone = PhoneFactory.getDefaultPhone();

        //mBatteryStats = BatteryStatsService.getService();

        HandlerThread TedongleThread = new HandlerThread("TedongleService");
        TedongleThread.start();
        mAsyncServiceHandler = new AsyncServiceHandler(TedongleThread.getLooper());

        publish();
        initNotification();
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mReceiver, intentFilter);

        //unistall the teledongleDemoActivity
        //didn't check by PM...right???
        Log.d(LOG_TAG, "Uninstall tedongleDemo apk!");
        TeledongleDemoInstaller.unInstall(mContext); 
        
        ((PhoneProxy)mPhone).registerForRadioStateChanged(mAsyncServiceHandler, EVENT_TEDONGLE_RADIO_STATE_CHANGED, null);
        //mPhone.getActivePhone().mCM.registerForRadioStateChanged(mHandler, EVENT_TEDONGLE_RADIO_STATE_CHANGED, null);
    }
    
    private void initNotification () {

        mNotification = new Notification();
        mNotification.when = System.currentTimeMillis();
        mNotification.flags = Notification.FLAG_NO_CLEAR ;
        mNotification.icon = com.android.internal.R.drawable.stat_sys_warning;

        CharSequence title = "3Gdongle Pluged";
        CharSequence details = "";
        
        Intent intent = new Intent();
        ComponentName comp = new ComponentName("com.mediatek.teledongledemo","com.mediatek.teledongledemo.TeledongleDemoActivity");
        intent.setComponent(comp);
        intent.setAction("android.intent.action.VIEW");
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        mNotification.contentIntent = PendingIntent
        .getActivity(mContext, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);

        mNotification.tickerText = title;
        mNotification.setLatestEventInfo(mContext, title, details,mNotification.contentIntent);

    }

    private void publish() {
        if (DBG) Log.d(LOG_TAG, "publish: " + this);

        ServiceManager.addService("tedongleservice", this);
    }

    private void notifyDonglePlug(boolean visible, Notification notification) {
                
        NotificationManager notificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
    
        if (visible) {
            notificationManager.notify(TED_NOTIFICATION_ID, notification);
        } else {
            notificationManager.cancel(TED_NOTIFICATION_ID);
        }
        
    }
    
    private void updateDonglePlugState() {

	    String dongleplug = SystemProperties.get("dongle.pluged", "0");
		
		if (dongleplug.equals("1")) {
		    mIsDonglePluged = true;
		} else {
		    mIsDonglePluged = false;
		}
	}


    /**
     * Get a reference to handler. This is used by a client to establish
     * an AsyncChannel communication with TedongleService
     */
    public Messenger getTedongleServiceMessenger() {
        Log.d(LOG_TAG, "getTedongleServiceMessenger, pid:" + Binder.getCallingPid()
                + ", uid:" + Binder.getCallingUid());
        //enforceAccessPermission();
        //enforceChangePermission();
        return new Messenger(mAsyncServiceHandler);
    }
    
    public boolean setRadio(boolean turnOn) {

        if ((mPhone.getServiceState().getState() != ServiceState.STATE_POWER_OFF) != turnOn) {
            toggleRadioOnOff();
        }
        return true;
    }

	public void toggleRadioOnOff() {

        mPhone.setRadioPower(!isRadioOn());
 
    }
	
	public boolean isRadioOn() {
       
        return mPhone.getServiceState().getState() != ServiceState.STATE_POWER_OFF;
    }
	
	public boolean isDonglePluged() {
	
        updateDonglePlugState();
        return mIsDonglePluged;
        
    }

    public boolean isAirplaneMode() {

        return mIsAirplneMode;
    
    }
	
    public int enableApnType(String type) {

        return mPhone.enableApnType(type);
    }

    public int disableApnType(String type) {

        return mPhone.disableApnType(type);
    }

	public int getDataState() {
	
        return TedonglePhoneNotifier.convertDataState(mPhone.getDataConnectionState());
    }

    public int getDataActivity() {
	
        return TedonglePhoneNotifier.convertDataActivityState(mPhone.getDataActivityState());
    }
	
	public int getActivePhoneType() {
	
        return mPhone.getPhoneType();
    }

	public int getNetworkType() {
	
        return mPhone.getServiceState().getNetworkType();
    }
    
	/*public int getSimStat() {
	
        return ((PhoneProxy)mPhone).getSimIndicateState();
    }*/
    
    public void listen(ITedongleStateListener callback, int events) {
        int callerUid = UserHandle.getCallingUserId();
        int myUid = UserHandle.myUserId();
        
        if (DBG) {
            Log.d(LOG_TAG, " events=0x" + Integer.toHexString(events)
                + " myUid=" + myUid
                + " callerUid=" + callerUid);
        }

        if (events != 0) {

            synchronized (mRecords) {
                // register
                Record r = null;
                find_and_add: {
                    
                    IBinder b = callback.asBinder();
                    final int N = mRecords.size();
                    for (int i = 0; i < N; i++) {
                        r = mRecords.get(i);
                        if (b == r.binder) {
                            break find_and_add;
                        }
                    }
                    r = new Record();
                    r.binder = b;
                    r.callback = callback;
                    r.callerUid = callerUid;
                    mRecords.add(r);
                    if (DBG) Log.d(LOG_TAG, "listen: add new record=" + r);
                }
                
                r.events = events;


               if ((events & TedongleStateListener.LISTEN_SIGNAL_STRENGTH) != 0) {
                    try {
                        int gsmSignalStrength = mSignalStrength.getGsmSignalStrength();
                        r.callback.onSignalStrengthChanged((gsmSignalStrength == 99 ? -1
                                : gsmSignalStrength));
                    } catch (RemoteException ex) {
                        remove(r.binder);
                    }
               }
           }

        }
        
    }

    private void remove(IBinder binder) {
        synchronized (mRecords) {
            final int recordCount = mRecords.size();
            for (int i = 0; i < recordCount; i++) {
                if (mRecords.get(i).binder == binder) {
                    mRecords.remove(i);
                    return;
                }
            }
        }
    }

    private void handleRemoveListLocked() {
        if (mRemoveList.size() > 0) {
            for (IBinder b: mRemoveList) {
                remove(b);
            }
            mRemoveList.clear();
        }
    }
    public void NotifySignalStrength(SignalStrength SS) {
        
        synchronized (mRecords) {
            mSignalStrength = SS;
            for (Record r : mRecords) {
                if ((r.events & TedongleStateListener.LISTEN_SIGNAL_STRENGTHS) != 0) {
                    try {
                        r.callback.onSignalStrengthsChanged(new SignalStrength(SS));
                    } catch (RemoteException ex) {
                        mRemoveList.add(r.binder);
                    }
                }
                if ((r.events & TedongleStateListener.LISTEN_SIGNAL_STRENGTH) != 0) {
                    try {
                        int gsmSignalStrength = SS.getGsmSignalStrength();
                        r.callback.onSignalStrengthChanged((gsmSignalStrength == 99 ? -1
                                : gsmSignalStrength));
                    } catch (RemoteException ex) {
                        mRemoveList.add(r.binder);
                    }
                }
            }
            handleRemoveListLocked();
        }
        
    }



    private boolean checkNotifyPermission(String method) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.MODIFY_PHONE_STATE)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        String msg = "Modify Phone State Permission Denial: " + method + " from pid="
                + Binder.getCallingPid() + ", uid=" + Binder.getCallingUid();
        if (DBG) Log.d(LOG_TAG, msg);
        return false;
    }

    /*private void checkListenerPermission(int events) {
        if ((events & PhoneStateListener.LISTEN_CELL_LOCATION) != 0) {
            mContext.enforceCallingOrSelfPermission(
                    android.Manifest.permission.ACCESS_COARSE_LOCATION, null);

        }

        if ((events & PhoneStateListener.LISTEN_CELL_INFO) != 0) {
            mContext.enforceCallingOrSelfPermission(
                    android.Manifest.permission.ACCESS_COARSE_LOCATION, null);

        }

        if ((events & PHONE_STATE_PERMISSION_MASK) != 0) {
            mContext.enforceCallingOrSelfPermission(
                    android.Manifest.permission.READ_PHONE_STATE, null);
        }
    }*/

    ///TODO:the following should be applied.
    /*private void enforceAccessPermission() {
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.ACCESS_TEDONGLE_STATE,
                                                "TedongleService");
    }

    private void enforceChangePermission() {
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.CHANGE_TEDONGLE_STATE,
                                                "TedongleService");

    }

    private void enforceConnectivityInternalPermission() {
        mContext.enforceCallingOrSelfPermission(
                android.Manifest.permission.CONNECTIVITY_INTERNAL,
                "ConnectivityService");
    }*/

	public ServiceState getServiceState() {
	
	    return mPhone.getServiceState();
	}
	
	public SignalStrength getSignalStrength() {
	
	    return mPhone.getSignalStrength();
	}
	
	public boolean isSimReady() {
        //3gdonle for test!!!
	    return true;
	}
    public String getLine1Number() {
        
        return mPhone.getLine1Number();
    }

    public String getSubscriberId() {
        
        return mPhone.getSubscriberId();
    }

    public boolean supplyPin(String pin) {
        //enforceModifyPermission();
        final UnlockSim checkSimPin = new UnlockSim(mPhone.getIccCard());
        checkSimPin.start();
        return checkSimPin.unlockSim(null, pin);
    }

    public boolean supplyPuk(String puk, String pin) {
        //enforceModifyPermission();
        final UnlockSim checkSimPuk = new UnlockSim(mPhone.getIccCard());
        checkSimPuk.start();
        return checkSimPuk.unlockSim(puk, pin);
    }

    /**
     * Helper thread to turn async call to {@link SimCard#supplyPin} into
     * a synchronous one.
     */
    private static class UnlockSim extends Thread {

        private final IccCard mSimCard;

        private boolean mDone = false;
        private boolean mResult = false;

        // For replies from SimCard interface
        private Handler mHandler;

        // For async handler to identify request type
        private static final int SUPPLY_PIN_COMPLETE = 100;

        public UnlockSim(IccCard simCard) {
            mSimCard = simCard;
        }

        @Override
        public void run() {
            Looper.prepare();
            synchronized (UnlockSim.this) {
                mHandler = new Handler() {
                    @Override
                    public void handleMessage(Message msg) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        switch (msg.what) {
                            case SUPPLY_PIN_COMPLETE:
                                Log.d(LOG_TAG, "SUPPLY_PIN_COMPLETE");
                                synchronized (UnlockSim.this) {
                                    mResult = (ar.exception == null);
                                    mDone = true;
                                    UnlockSim.this.notifyAll();
                                }
                                break;
                        }
                    }
                };
                UnlockSim.this.notifyAll();
            }
            Looper.loop();
        }

        /*
         * Use PIN or PUK to unlock SIM card
         *
         * If PUK is null, unlock SIM card with PIN
         *
         * If PUK is not null, unlock SIM card with PUK and set PIN code
         */
        synchronized boolean unlockSim(String puk, String pin) {

            while (mHandler == null) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
            Message callback = Message.obtain(mHandler, SUPPLY_PIN_COMPLETE);

            if (puk == null) {
                mSimCard.supplyPin(pin, callback);
            } else {
                mSimCard.supplyPuk(puk, pin, callback);
            }

            while (!mDone) {
                try {
                    Log.d(LOG_TAG, "wait for done");
                    wait();
                } catch (InterruptedException e) {
                    // Restore the interrupted status
                    Thread.currentThread().interrupt();
                }
            }
            Log.d(LOG_TAG, "done");
            return mResult;
        }
    }

    public boolean getIccLockEnabled() {
        /* defaults to true, if ICC is absent */
        Boolean retValue = ((PhoneProxy)mPhone).getIccCard().getIccLockEnabled();
        return retValue;
    }

    public void setIccLockEnabled(boolean enabled, String password, Message onComplete) {
        //3gdongle
        Log.d(LOG_TAG, "setIccLockEnabled :" + " enabled:"+enabled+ " password:"+password
        + " message:"+onComplete);
        ((PhoneProxy)mPhone).getIccCard().setIccLockEnabled(enabled, password, onComplete);
    }

    public void changeIccLockPassword(String oldPassword, String newPassword, Message onComplete) {
        ((PhoneProxy)mPhone).getIccCard().changeIccLockPassword(oldPassword, newPassword, onComplete);
    }
    
	public void dispose() {
        //mPhone.getActivePhone().mCM.unregisterForRadioStateChanged(mHandler);
        ((PhoneProxy)mPhone).unregisterForRadioStateChanged(mAsyncServiceHandler);
        mPhone.dispose();
    }
}
