/*
 * ANT Stack
 *
 * Copyright 2011 Dynastream Innovations
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

/**
 * Defines constants to be used by the ANT Radio Service to ANT System Service interface (IAntHal).
 */
public class AntHalDefine
{
    // ANT HAL Results
    /** Operation was successful. */
    public static final int ANT_HAL_RESULT_SUCCESS              = 1;

    /** Operation failed for an unknown reason. */
    public static final int ANT_HAL_RESULT_FAIL_UNKNOWN         = -1;

    /** Operation failed as it is invalid. */
    public static final int ANT_HAL_RESULT_FAIL_INVALID_REQUEST = -2;

    /** Operation failed as ANT is not enabled. */
    public static final int ANT_HAL_RESULT_FAIL_NOT_ENABLED     = -3;

    /** Operation failed as this device does not support ANT. */
    public static final int ANT_HAL_RESULT_FAIL_NOT_SUPPORTED   = -4;

    /** Operation failed as the ANT hardware is in use. */
    public static final int ANT_HAL_RESULT_FAIL_RESOURCE_IN_USE = -5;

    /**
     * Gets the human-readable value of an ANT_HAL_RESULT_X value.
     *
     * @param value the ANT_HAL_RESULT_X value
     * @return the string representation.
     */
    public static String getAntHalResultString(int value)
    {
        String string = null;

        switch(value)
        {
            case ANT_HAL_RESULT_SUCCESS:
                string = "SUCCESS";
                break;
            case ANT_HAL_RESULT_FAIL_UNKNOWN:
                string = "FAIL: UNKNOWN";
                break;
            case ANT_HAL_RESULT_FAIL_INVALID_REQUEST:
                string = "FAIL: INVALID REQUEST";
                break;
            case ANT_HAL_RESULT_FAIL_NOT_ENABLED:
                string = "FAIL: NOT ENABLED";
                break;
            case ANT_HAL_RESULT_FAIL_NOT_SUPPORTED:
                string = "FAIL: NOT SUPPORTED";
                break;
            case ANT_HAL_RESULT_FAIL_RESOURCE_IN_USE:
                string = "FAIL: RESOURCE IN USE";
                break;
        }

        return string;
    }


    // ANT HAL State

    /** State not yet initialised. */
    public static final int ANT_HAL_STATE_UNKNOWN               = 0;

    /** Powering on the ANT hardware and initialising the transport to it. */
    public static final int ANT_HAL_STATE_ENABLING              = 1;

    /** ANT is ON. */
    public static final int ANT_HAL_STATE_ENABLED               = 2;

    /** Closing the transport and powering down the ANT hardware. */
    public static final int ANT_HAL_STATE_DISABLING             = 3;

    /** ANT is OFF. */
    public static final int ANT_HAL_STATE_DISABLED              = 4;

    /** ANT is not supported on this device. */
    public static final int ANT_HAL_STATE_NOT_SUPPORTED         = 5;

    /** ANT system service is not installed. */
    public static final int ANT_HAL_STATE_SERVICE_NOT_INSTALLED = 6;

    /** There is no connection to the ANT system service (not bound). */
    public static final int ANT_HAL_STATE_SERVICE_NOT_CONNECTED = 7;

    /** ANT hardware was reset, low level is in the process of re-enabling. */
    public static final int ANT_HAL_STATE_RESETTING             = 8;

    /** ANT low level has finished re-enabling the hardware. */
    public static final int ANT_HAL_STATE_RESET                 = 9;

    /**
     * Gets the human-readable value of an ANT_HAL_STATE_X value.
     *
     * @param state the ANT_HAL_STATE_X value
     * @return the string representation.
     */
    public static String getAntHalStateString(int state)
    {
        String string = null;

        switch(state)
        {
            case ANT_HAL_STATE_UNKNOWN:
                string = "STATE UNKNOWN";
                break;
            case ANT_HAL_STATE_ENABLING:
                string = "ENABLING";
                break;
            case ANT_HAL_STATE_ENABLED:
                string = "ENABLED";
                break;
            case ANT_HAL_STATE_DISABLING:
                string = "DISABLING";
                break;
            case ANT_HAL_STATE_DISABLED:
                string = "DISABLED";
                break;
            case ANT_HAL_STATE_NOT_SUPPORTED:
                string = "ANT NOT SUPPORTED";
                break;
            case ANT_HAL_STATE_SERVICE_NOT_INSTALLED:
                string = "ANT HAL SERVICE NOT INSTALLED";
                break;
            case ANT_HAL_STATE_SERVICE_NOT_CONNECTED:
                string = "ANT HAL SERVICE NOT CONNECTED";
                break;
            case ANT_HAL_STATE_RESETTING:
                string = "RESETTING";
                break;
            case ANT_HAL_STATE_RESET:
                string = "RESET";
                break;
        }

        return string;
    }
}
