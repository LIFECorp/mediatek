/*
 * ANT Stack
 *
 * Copyright 2009 Dynastream Innovations
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.dsi.ant.server;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.app.Service;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.os.SystemProperties;

import com.dsi.ant.core.*;

import com.dsi.ant.server.AntHalDefine;
import com.dsi.ant.server.IAntHal;
import com.dsi.ant.server.IAntHalCallback;
import com.dsi.ant.server.Version;

public class AntService extends Service
{
    private static final String TAG = "AntHalService";

    private static final boolean DEBUG = false;

    public static final String ANT_SERVICE = "AntService";

    /**
     * Allows the application to directly configure the ANT radio through the
     * proxy service. Malicious applications may prevent other ANT applications
     * from connecting to ANT devices
     */
    public static final String ANT_ADMIN_PERMISSION = "com.dsi.ant.permission.ANT_ADMIN";

    /**
     * Request that ANT be enabled
     */
    public static final String ACTION_REQUEST_ENABLE = "com.dsi.ant.server.action.REQUEST_ENABLE";

    /**
     * Request that ANT be disabled
     */
    public static final String ACTION_REQUEST_DISABLE = "com.dsi.ant.server.action.REQUEST_DISABLE";

    private JAntJava mJAnt = null;

    private boolean mInitialized = false;

    /**
     * Flag for if Bluetooth needs to be enabled for ANT to enable
     */
    private boolean mRequiresBluetoothOn = false;

    /**
     * Flag which specifies if we are waiting for an ANT enable intent
     */
    private boolean mEnablePending = false;

    private Object mChangeAntPowerState_LOCK = new Object();
    private static Object sAntHalServiceDestroy_LOCK = new Object();

    IAntHalCallback mCallback;

    /**
     * Receives Bluetooth State Changed intent and sends {@link ACTION_REQUEST_ENABLE}
     * and {@link ACTION_REQUEST_DISABLE} accordingly
     */
    private final StateChangedReceiver mStateChangedReceiver = new StateChangedReceiver();

    /**
     * Receives {@link ACTION_REQUEST_ENABLE} and {@link ACTION_REQUEST_DISABLE}
     * intents to enable and disable ANT.
     */
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (mRequiresBluetoothOn) {
                String action = intent.getAction();
                if (ACTION_REQUEST_ENABLE.equals(action)) {
                    if (mEnablePending) {
                        asyncSetAntPowerState(true);
                        mEnablePending = false;
                    }
                } else if (ACTION_REQUEST_DISABLE.equals(action)) {
                    if (mEnablePending) {
                        mEnablePending = false;
                    } else {
                        asyncSetAntPowerState(false);
                    }
                }
            }
        }
    };

    /**
     * Checks if Bluetooth needs to be turned on for ANT to enable
     */
    private boolean requiresBluetoothOn() {
        return isQcomPlatform();
    }

    /**
     * Checks if the current platform is QCom
     */
    private boolean isQcomPlatform()
    {
        if ((SystemProperties.get("ro.board.platform").equals("msm8974"))
                || (SystemProperties.get("ro.board.platform").equals("msm8610"))
                || (SystemProperties.get("ro.board.platform").equals("msm8226"))) {

            return true;
        }
        return false;
    }

    public static boolean startService(Context context)
    {
        return ( null != context.startService(new Intent(IAntHal.class.getName())) );
    }

    /**
     * Calls back the registered callback with the change to the new state 
     * @param state the {@link AntHalDefine} state
     */
    private void setState(int state)
    {
        synchronized(mChangeAntPowerState_LOCK) {
            if(DEBUG) Log.i(TAG, "Setting ANT State = "+ state +" / "+ AntHalDefine.getAntHalStateString(state));

            if (mCallback != null)
            {
                try
                {
                    if(DEBUG) Log.d(TAG, "Calling status changed callback "+ mCallback.toString());

                    mCallback.antHalStateChanged(state);
                }
                catch (RemoteException e)
                {
                    // Don't do anything as this is a problem in the application

                    if(DEBUG) Log.e(TAG, "ANT HAL State Changed callback failure in application", e);
                }
            }
            else
            {
                if(DEBUG) Log.d(TAG, "Calling status changed callback is null");
            }
        }
    }

    /**
     * Requests to change the state
     * @param state The desired state to change to
     * @return An {@link AntHalDefine} result
     */
    private int doSetAntState(int state)
    {
        synchronized(mChangeAntPowerState_LOCK) {
            int result = AntHalDefine.ANT_HAL_RESULT_FAIL_INVALID_REQUEST;

            switch(state)
            {
                case AntHalDefine.ANT_HAL_STATE_ENABLED:
                {
                    result = AntHalDefine.ANT_HAL_RESULT_FAIL_NOT_ENABLED;

                    boolean waitForBluetoothToEnable = false;

                    if (mRequiresBluetoothOn) {

                        // Try to turn on BT if it is not enabled.
                        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

                        if (bluetoothAdapter != null) {

                            // run with permissions of ANTHALService
                            long callingPid = Binder.clearCallingIdentity();

                            if (!bluetoothAdapter.isEnabled()) {

                                waitForBluetoothToEnable = true;
                                mEnablePending = true;
                                
                                boolean isEnabling = bluetoothAdapter.enable();

                                // if enabling adapter has begun, return
                                // success.
                                if (isEnabling) {
                                    result = AntHalDefine.ANT_HAL_RESULT_SUCCESS;
                                    // StateChangedReceiver will receive
                                    // enabled status and then enable ANT
                                } else {
                                    mEnablePending = false;
                                }
                            }

                            Binder.restoreCallingIdentity(callingPid);
                        }
                    }

                    if (!waitForBluetoothToEnable) {
                        result = asyncSetAntPowerState(true);
                    }
                    break;
                }
                case AntHalDefine.ANT_HAL_STATE_DISABLED:
                {
                    result = asyncSetAntPowerState(false);
                    break;
                }
                case AntHalDefine.ANT_HAL_STATE_RESET:
                {
                    result = doHardReset();
                    break;
                }
            }
            
            return result;
        }
    }

    /**
     * Queries the native code for state
     * @return An {@link AntHalDefine} state
     */
    private int doGetAntState()
    {
        if(DEBUG) Log.v(TAG, "doGetAntState start");

        int retState = mJAnt.getRadioEnabledStatus(); // ANT state is native state
        
        if(DEBUG) Log.i(TAG, "Get ANT State = "+ retState +" / "+ AntHalDefine.getAntHalStateString(retState));

        return retState;
    }

    /**
     * Perform a power change if required.
     * Tries to use changeAntWirelessState() in {@link BluetoothService}. If it does not exist then
     * it defaults to a native enable() or disable() call
     * @param state true for enable, false for disable
     * @return {@link AntHalDefine#ANT_HAL_RESULT_SUCCESS} when the request is successfully sent,
     * false otherwise
     */
    private int asyncSetAntPowerState(final boolean state)
    {
        int result = AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;

        synchronized (mChangeAntPowerState_LOCK) {
            // Check we are not already in/transitioning to the state we want
            int currentState = doGetAntState();

            if (state) {
                if ((AntHalDefine.ANT_HAL_STATE_ENABLED == currentState)
                        || (AntHalDefine.ANT_HAL_STATE_ENABLING == currentState)) {
                    if (DEBUG) {
                        Log.d(TAG, "Enable request ignored as already enabled/enabling");
                    }

                    return AntHalDefine.ANT_HAL_RESULT_SUCCESS;
                } else if (AntHalDefine.ANT_HAL_STATE_DISABLING == currentState) {
                    Log.w(TAG, "Enable request ignored as already disabling");

                    return AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;
                }
            } else {
                if ((AntHalDefine.ANT_HAL_STATE_DISABLED == currentState)
                        || (AntHalDefine.ANT_HAL_STATE_DISABLING == currentState)) {
                    if (DEBUG) {
                        Log.d(TAG, "Disable request ignored as already disabled/disabling");
                    }

                    return AntHalDefine.ANT_HAL_RESULT_SUCCESS;
                } else if (AntHalDefine.ANT_HAL_STATE_ENABLING == currentState) {
                    Log.w(TAG, "Disable request ignored as already enabling");

                    return AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;
                }
            }

            if (state) {
                result = enableBackground();
            } else {
                result = disableBackground();
            }
        }

        return result;
    }

    /**
     * Calls enable on the native libantradio.so
     * @return {@link AntHalDefine#ANT_HAL_RESULT_SUCCESS} when successful, or
     * {@link AntHalDefine#ANT_HAL_RESULT_FAIL_UNKNOWN} if unsuccessful
     */
    private int enableBlocking()
    {
        int ret = AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;
        synchronized(sAntHalServiceDestroy_LOCK)
        {
            if (mJAnt != null)
            {
                if(JAntStatus.SUCCESS == mJAnt.enable())
                {
                    if(DEBUG) Log.v(TAG, "Enable call: Success");
                    ret = AntHalDefine.ANT_HAL_RESULT_SUCCESS;
                }
                else
                {
                    if(DEBUG) Log.v(TAG, "Enable call: Failure");
                }
            }
        }
        return ret;
    }

    /**
     * Calls disable on the native libantradio.so
     * @return {@link AntHalDefine#ANT_HAL_RESULT_SUCCESS} when successful, or
     * {@link AntHalDefine#ANT_HAL_RESULT_FAIL_UNKNOWN} if unsuccessful
     */
    private int disableBlocking()
    {
        int ret = AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;
        synchronized(sAntHalServiceDestroy_LOCK)
        {
            if (mJAnt != null)
            {
                if(JAntStatus.SUCCESS == mJAnt.disable())
                {
                    if(DEBUG) Log.v(TAG, "Disable callback end: Success");
                    ret = AntHalDefine.ANT_HAL_RESULT_SUCCESS;
                }
                else
                {
                    if (DEBUG) Log.v(TAG, "Disable callback end: Failure");
                }
            }
        }
        return ret;
    }

    /**
     * Post an enable runnable.
     */
    private int enableBackground()
    {
        if(DEBUG) Log.v(TAG, "Enable start");

        if (DEBUG) Log.d(TAG, "Enable: enabling the radio");

        // TODO use handler to post runnable rather than creating a new thread.
        new Thread(new Runnable() {
            public void run() {
                enableBlocking();
            }
        }).start();

        if(DEBUG) Log.v(TAG, "Enable call end: Successfully called");
        return AntHalDefine.ANT_HAL_RESULT_SUCCESS;
    }

    /**
     * Post a disable runnable.
     */
    private int disableBackground()
    {
        if(DEBUG) Log.v(TAG, "Disable start");

        // TODO use handler to post runnable rather than creating a new thread.
        new Thread(new Runnable() {
            public void run() {
                disableBlocking();
            }
        }).start();

        if(DEBUG) Log.v(TAG, "Disable call end: Success");
        return AntHalDefine.ANT_HAL_RESULT_SUCCESS;
    }

    private int doANTTxMessage(byte[] message)
    {
        if(DEBUG) Log.v(TAG, "ANT Tx Message start");

        if(message == null)
        {
            Log.e(TAG, "ANTTxMessage invalid message: message is null");
            return AntHalDefine.ANT_HAL_RESULT_FAIL_INVALID_REQUEST;
        }

        int result = AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;

        JAntStatus status = mJAnt.ANTTxMessage(message);

        if(JAntStatus.SUCCESS == status)
        {
            if (DEBUG) Log.d (TAG, "mJAnt.ANTTxMessage returned status: " + status.toString());

            result = AntHalDefine.ANT_HAL_RESULT_SUCCESS;
        }
        else
        {
            if (DEBUG) Log.w( TAG, "mJAnt.ANTTxMessage returned status: " + status.toString() );

            if(JAntStatus.FAILED_BT_NOT_INITIALIZED == status)
            {
                result = AntHalDefine.ANT_HAL_RESULT_FAIL_NOT_ENABLED;
            }
            else if(JAntStatus.NOT_SUPPORTED == status)
            {
                result = AntHalDefine.ANT_HAL_RESULT_FAIL_NOT_SUPPORTED;
            }
            else if(JAntStatus.INVALID_PARM == status)
            {
                result = AntHalDefine.ANT_HAL_RESULT_FAIL_INVALID_REQUEST;
            }
        }

        if (DEBUG) Log.v(TAG, "ANTTxMessage: Result = "+ result);

        if(DEBUG) Log.v(TAG, "ANT Tx Message end");

        return result;
    }

    private int doRegisterAntHalCallback(IAntHalCallback callback)
    {
        if(DEBUG) Log.i(TAG, "Registering callback: "+ callback.toString());

        mCallback = callback;

        return AntHalDefine.ANT_HAL_RESULT_SUCCESS;
    }

    private int doUnregisterAntHalCallback(IAntHalCallback callback)
    {
        if(DEBUG) Log.i(TAG, "UNRegistering callback: "+ callback.toString());

        int result = AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;

        if(mCallback.asBinder() == callback.asBinder())
        {
            mCallback = null;
            result = AntHalDefine.ANT_HAL_RESULT_SUCCESS;
        }

        return result;
    }

    private int doGetServiceLibraryVersionCode()
    {
        return Version.ANT_HAL_LIBRARY_VERSION_CODE;
    }

    private String doGetServiceLibraryVersionName()
    {
        return Version.ANT_HAL_LIBRARY_VERSION_NAME;
    }

    private int doHardReset()
    {
        int ret = AntHalDefine.ANT_HAL_RESULT_FAIL_UNKNOWN;
        synchronized(sAntHalServiceDestroy_LOCK)
        {
            if (mJAnt != null)
            {
                if(JAntStatus.SUCCESS == mJAnt.hardReset())
                {
                    if(DEBUG) Log.v(TAG, "Hard Reset end: Success");
                    ret = AntHalDefine.ANT_HAL_RESULT_SUCCESS;
                }
                else
                {
                    if (DEBUG) Log.v(TAG, "Hard Reset end: Failure");
                }
            }
        }
        return ret;
    }

    // ----------------------------------------------------------------------------------------- IAntHal

    private final IAntHal.Stub mHalBinder = new IAntHal.Stub()
    {
        public int setAntState(int state)
        {
            return doSetAntState(state);
        }

        public int getAntState()
        {
            return doGetAntState();
        }

        public int ANTTxMessage(byte[] message)
        {
            return doANTTxMessage(message);
        }

        // Call these in onServiceConnected and when unbinding
        public int registerAntHalCallback(IAntHalCallback callback)
        {
            return doRegisterAntHalCallback(callback);
        }

        public int unregisterAntHalCallback(IAntHalCallback callback)
        {
            return doUnregisterAntHalCallback(callback);
        }

        public int getServiceLibraryVersionCode()
        {
            return doGetServiceLibraryVersionCode();
        }

        public String getServiceLibraryVersionName()
        {
            return doGetServiceLibraryVersionName();
        }
    }; // new IAntHal.Stub()

    // -------------------------------------------------------------------------------------- Service

    @Override
    public void onCreate()
    {
        if (DEBUG) Log.d(TAG, "onCreate() entered");

        super.onCreate();
        
        if(null != mJAnt)
        {
            // This somehow happens when quickly starting/stopping an application.
            if (DEBUG) Log.e(TAG, "LAST JAnt HCI Interface object not destroyed");
        }
        // create a single new JAnt HCI Interface instance
        mJAnt = new JAntJava();
        mRequiresBluetoothOn = requiresBluetoothOn();
        JAntStatus createResult = mJAnt.create(mJAntCallback);

        if (createResult == JAntStatus.SUCCESS)
        {
        	mInitialized = true;

        	if (DEBUG) Log.d(TAG, "JAntJava create success");
        }
        else
        {
            mInitialized = false;

            if (DEBUG) Log.e(TAG, "JAntJava create failed: " + createResult);
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_REQUEST_ENABLE);
        filter.addAction(ACTION_REQUEST_DISABLE);
        registerReceiver(mReceiver, filter);
        
        if (mRequiresBluetoothOn) {
            IntentFilter stateChangedFilter = new IntentFilter();
            stateChangedFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            registerReceiver(mStateChangedReceiver, stateChangedFilter);
        }
    }

    @Override
    public void onDestroy()
    {
        if (DEBUG) Log.d(TAG, "onDestroy() entered");

        try
        {
        	synchronized(sAntHalServiceDestroy_LOCK)
            {
                if(null != mJAnt)
                {
                    int result = disableBlocking();
                    if (DEBUG) Log.d(TAG, "onDestroy: disable result is: " + AntHalDefine.getAntHalResultString(result));
                    
                    mJAnt.destroy();
                    mJAnt = null;
                }
            }

            mCallback = null;
        }
        finally
        {
            super.onDestroy();
        }

        if (mRequiresBluetoothOn) {
            unregisterReceiver(mStateChangedReceiver);
        }

        unregisterReceiver(mReceiver);
    }

    @Override
    public IBinder onBind(Intent intent)
    {
        if (DEBUG) Log.d(TAG, "onBind() entered");

        IBinder binder = null;

        if (mInitialized)
        {
            if(intent.getAction().equals(IAntHal.class.getName()))
            {
                if (DEBUG) Log.i(TAG, "Bind: IAntHal");

                binder = mHalBinder;
            }
        }

        // As someone has started using us, make sure we run "forever" like we
        // are a system service.
        startService(this);

        return binder;
    }

    @Override
    public boolean onUnbind(Intent intent)
    {
        if (DEBUG) Log.d(TAG, "onUnbind() entered");

        mCallback = null;

        return super.onUnbind(intent);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        if (DEBUG) Log.d(TAG, "onStartCommand() entered");

        if (!mInitialized)
        {
            if (DEBUG) Log.e(TAG, "not initialized, stopping self");
            stopSelf();
        }
        return START_NOT_STICKY;
    }

    // ----------------------------------------------------------------------------------------- JAnt Callbacks

    private JAntJava.ICallback mJAntCallback = new JAntJava.ICallback()
    {
        public synchronized void ANTRxMessage( byte[] message)
        {
            if(null != mCallback)
            {
                try
                {
                    mCallback.antHalRxMessage(message);
                }
                catch (RemoteException e)
                {
                    // Don't do anything as this is a problem in the application
                    if(DEBUG) Log.e(TAG, "ANT HAL Rx Message callback failure in application", e);
                }
            }
            else
            {
                Log.w(TAG, "JAnt callback called after service has been destroyed");
            }
        }

        public synchronized void ANTStateChange(int NewState)
        {
            if (DEBUG) Log.i(TAG, "ANTStateChange callback to " + NewState);
            
            setState(NewState);
        }
    };
}

