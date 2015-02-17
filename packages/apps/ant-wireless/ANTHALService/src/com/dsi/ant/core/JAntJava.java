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
package com.dsi.ant.core;

import android.util.Log;

import com.dsi.ant.server.AntHalDefine;


/** Class for providing connection with JANTNative.cpp module */

public class JAntJava
{
    private static final boolean debug = false;
    private static final String TAG = "JAntJava";
    private static ICallback mCallback = null;


    /** Static constructor */
    static
    {
        try
        {
            System.load("libantradio.so");
        }
        catch (Exception e)
        {
            Log.e("JANT", "Exception during nativeJANT_ClassInitNative (" + e.toString() + ")");
        }
    }

    public interface ICallback
    {
        void ANTRxMessage(byte[] RxMessage);
        void ANTStateChange(int NewState);
    }
    /*******************************************************************************
     *
     * Class Methods
     *
     *******************************************************************************/

    public JAntStatus create(ICallback callback)
    {
        JAntStatus jAntStatus;

        try
        {
            if (debug)
                Log.d(TAG, "Calling nativeJAnt_Create");
            int AntStatus = nativeJAnt_Create();
            jAntStatus = JAntUtils.getEnumConst(JAntStatus.class, AntStatus);

            // Record the caller's callback if create was successful
            if (JAntStatus.SUCCESS == jAntStatus)
            {
                if (debug)
                    Log.d(TAG, "create: nativeJAnt_Create returned success");
                mCallback = callback;
            }
            else
            {
                Log.e(TAG, "create: nativeJAnt_Create failed " + jAntStatus);
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "create: exception during nativeJAnt_Create (" + e.toString() + ")");
            jAntStatus = JAntStatus.FAILED;
        }

        return jAntStatus;
    }

    public JAntStatus destroy()
    {
        if (debug)
            Log.d(TAG, "destroy: entered");
        JAntStatus jAntStatus;

        try
        {
            int AntStatus = nativeJAnt_Destroy();
            jAntStatus = JAntUtils.getEnumConst(JAntStatus.class, AntStatus);
            if (JAntStatus.SUCCESS == jAntStatus)
            {
                if (debug)
                    Log.d(TAG, "destroy: nativeJAnt_Destroy returned success");
                mCallback = null;
            }
            else
            {
                Log.e(TAG, "destroy: nativeJAnt_Destroy failed " + jAntStatus);
            }
        }
        catch (Exception e)
        {
            Log.e(TAG, "destroy: exception during nativeJAnt_Destroy (" + e.toString() + ")");
            jAntStatus = JAntStatus.FAILED;
        }

        if (debug)
            Log.d(TAG, "destroy: exiting");

        return jAntStatus;
    }

    public JAntStatus enable()
    {
        if (debug)
            Log.d(TAG, "enable: entered");

        JAntStatus jAntStatus;

        try
        {
            int AntStatus = nativeJAnt_Enable();
            jAntStatus = JAntUtils.getEnumConst(JAntStatus.class, AntStatus);
            if (debug)
                Log.d(TAG, "After nativeJAnt_Enable, status = " + jAntStatus.toString());
        }
        catch (Exception e)
        {
            Log.e(TAG, "enable: exception during nativeJAnt_enable (" + e.toString() + ")");
            jAntStatus = JAntStatus.FAILED;
        }

        if (debug)
            Log.d(TAG, "enable: exiting");

        return jAntStatus;
    }


    public JAntStatus disable()
    {
        if (debug)
            Log.d(TAG, "disable: entered");

        JAntStatus jAntStatus;

        try
        {
            int status = nativeJAnt_Disable();
            jAntStatus = JAntUtils.getEnumConst(JAntStatus.class, status);
            if (debug)
                Log.d(TAG, "After nativeJAnt_Disable, status = " + jAntStatus.toString());
        }
        catch (Exception e)
        {
            Log.e(TAG, "disable: exception during nativeJAnt_Disable (" + e.toString() + ")");
            jAntStatus = JAntStatus.FAILED;
        }

        if (debug)
            Log.d(TAG, "disable: exiting");

        return jAntStatus;
    }

    public int getRadioEnabledStatus()
    {
        if (debug) Log.d(TAG, "getRadioEnabledStatus: entered");
        int retStatus;
        try
        {
            retStatus = nativeJAnt_GetRadioEnabledStatus();
            if (debug) Log.i(TAG, "Got ANT status as " + retStatus);
        }
        catch (Exception e)
        {
            Log.e(TAG, "getRadioEnabledStatus: exception during call (" + e.toString() + ")");
            retStatus = AntHalDefine.ANT_HAL_STATE_UNKNOWN;
        }
        if (debug) Log.d(TAG, "getRadioEnabledStatus: exiting");
        return retStatus;
    }

    public JAntStatus ANTTxMessage(byte[] message)
    {
        if (debug)
            Log.d(TAG, "ANTTxMessage: entered");
        JAntStatus jAntStatus;

        try
        {
            int AntStatus = nativeJAnt_TxMessage(message);
            jAntStatus = JAntUtils.getEnumConst(JAntStatus.class, AntStatus);
            if (debug)
                Log.d(TAG, "After nativeJAnt_ANTTxMessage, status = " + jAntStatus.toString());
        }
        catch (Exception e)
        {
            Log.e(TAG, "ANTTxMessage: exception during nativeJAnt_ANTTxMessage (" + e.toString() + ")");
            jAntStatus = JAntStatus.FAILED;
        }

        if (debug)
            Log.d(TAG, "ANTTxMessage: exiting");
        return jAntStatus;
    }

    public JAntStatus hardReset()
    {
        if (debug)
            Log.d(TAG, "hardReset: entered");

        JAntStatus jAntStatus;

        try
        {
            int status = nativeJAnt_HardReset();
            jAntStatus = JAntUtils.getEnumConst(JAntStatus.class, status);
            if (debug)
                Log.d(TAG, "After nativeJAnt_HardReset, status = " + jAntStatus.toString());
        }
        catch (Exception e)
        {
            Log.e(TAG, "hardReset: exception during nativeJAnt_HardReset, (" + e.toString() + ")");
            jAntStatus = JAntStatus.FAILED;
        }

        if (debug)
            Log.d(TAG, "hardReset:: exiting");

        return jAntStatus;
    }

    /* --------------------------------
     *            NATIVE PART
     * --------------------------------
     */
    /* RBTL Calls */
    private static native int    nativeJAnt_Create();
    private static native int    nativeJAnt_Destroy();
    private static native int    nativeJAnt_Enable();
    private static native int    nativeJAnt_Disable();
    private static native int    nativeJAnt_GetRadioEnabledStatus();
    private static native int    nativeJAnt_TxMessage(byte[] message);
    private static native int    nativeJAnt_HardReset();


    /*    ----------------------------------------------
     *    Callbacks from the JAntNative.cpp module
     *    ----------------------------------------------
     */
    public static void nativeCb_AntRxMessage(byte[] RxMessage)
    {
        if (debug)
            Log.d(TAG, "nativeCb_AntRxMessage: calling callback");

        if (mCallback != null)
        {
            mCallback.ANTRxMessage(RxMessage);
        }
        else
        {
            Log.e(TAG, "nativeCb_AntRxMessage: callback is null");
        }
    }

    public static void nativeCb_AntStateChange(int NewState)
    {
        if (debug)
            Log.d(TAG, "nativeCb_AntStateChange: calling callback");

        if (mCallback != null)
        {
            mCallback.ANTStateChange(NewState);
        }
        else
        {
            Log.e(TAG, "nativeCb_AntStateChange: callback is null");
        }
    }

} /* End class */

