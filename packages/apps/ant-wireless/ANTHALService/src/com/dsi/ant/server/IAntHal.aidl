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

/*
 *  !! Do not modify this file !!
 *
 * To update the interface, create a new AIDL and allow the service to bind with
 * it, along with any previous AIDL's.
 */

package com.dsi.ant.server;

import com.dsi.ant.server.IAntHalCallback;

/**
 * Specifies the interface the ANT Radio Service uses to communicate with the system service.
 *
 * @version 1.0.1
 */
interface IAntHal 
{
    /**
     * Powers on/off the ANT chip.
     * 
     * @param state The desired ANT state, either ANT_HAL_STATE_ENABLED or ANT_HAL_STATE_DISABLED.
     *
     * @return ANT_HAL_RESULT_SUCCESS if the request was forwarded to the hardware layer.
     * The IAntHalCallback sends ANT_HAL_STATE_X through antHalStateCallback()
     * to indicate when the state has changed.
     */
    int setAntState(int state);
    
    /**
     * Gets the enabled status (ANT_HAL_STATE_X) of the ANT hardware, either enabling, enabled, disabling or disabled.
     * 
     * @return The current state of the ANT HAL service.
     */
    int getAntState();
    
    
    /**
     * Sends raw data to the ANT hardware.  May be multiple ANT packets.
     *
     * @param message The data to be forwarded to the hardware, plus a header giving the 2 byte little endian length.
     *
     *  The format is
     *   LL LL II JJ ------ [... II JJ ------   ] 
     *  | Len | ANT Packet |    | ANT Packet N |
     *
     *   where:   LL LL  is the 2 byte (little endian) total length of the following ANT packet(s)
     *            II     is the 1 byte size of the ANT message (0-249)  
     *            JJ     is the 1 byte ID of the ANT message (1-255, 0 is invalid)
     *            ------ is the data of the ANT message (0-249 bytes of data)
     *
     *
     * The sync byte (header) and checksum byte (footer) for each ANT packet are added by the system service if required.
     * 
     * @return ANT_HAL_RESULT_SUCCESS if the request was forwarded to the hardware layer.
     */
    int ANTTxMessage(in byte[] message);
    
    
    /**
     * Set the callback to be used for updates from the ANT system service.
     *
     * @param callback The instance of an IAntHalCallback to use.
     * 
     * @return ANT_HAL_RESULT_SUCCESS if the callback was set.
     */
    int registerAntHalCallback(IAntHalCallback callback);
    
    /**
     * Stop receiving updates from the ANT system service on the specified callback.
     *
     * @param callback The instance of an IAntHalCallback to remove.
     * 
     * @return ANT_HAL_RESULT_SUCCESS if the callback was removed.
     */
    int unregisterAntHalCallback(IAntHalCallback callback);
    
    
    /**
     * Gets the version code of the (latest) interface version provided by the system service.
     * This allows the ANT Radio Service to only request functionality provided by the current system service.
     * 
     * @return The version number.
     */
    int getServiceLibraryVersionCode();
    
    
    /**
     * Gets the human-readable version name of the interface (latest) version provided by the system service.
     * 
     * @return The current state of the ANT HAL service.
     */
    String getServiceLibraryVersionName();
}
