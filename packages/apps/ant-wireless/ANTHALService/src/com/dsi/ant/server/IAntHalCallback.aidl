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

/**
 * Specifies the interface system service uses to communicate with the ANT Radio Service.
 * The ANT Radio Service must register an instance of the callback with the system service.
 *
 * @version 1.0.1
 */
interface IAntHalCallback
{
    /**
     * Triggered when the ANT enabled state has changed.
     *
     * @param state The new (ANT_HAL_STATE_X) state of the system service, either enabling, enabled, disabling or disabled.
     */
    void antHalStateChanged(int state);
    
    /**
     * Triggered when an ANT message has been received.  Always a single ANT packet.
     *
     * @param message The raw ANT packet.
     *
     *  The format is
     *   II JJ ------ 
     *   ^          ^
     *  | ANT Packet |
     *
     *   where:   II     is the 1 byte size of the ANT message (0-249)  
     *            JJ     is the 1 byte ID of the ANT message (1-255, 0 is invalid)
     *            ------ is the data of the ANT message (0-249 bytes of data)
     *
     * The sync byte (header) and checksum byte (footer) for each ANT packet were removed by the system service if required.
     */
    void antHalRxMessage(in byte[] message);
}
