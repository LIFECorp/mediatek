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
package com.dsi.ant.server.startup;

import com.dsi.ant.server.AntService;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * This class will receive BOOT_COMPLETED, and start the ANT HAL Service running forever.
 */
public class BootCompletedReceiver extends BroadcastReceiver 
{
    /** The debug log tag */
    public static final String TAG = "BootCompletedReceiver";
    
    @Override
    public void onReceive(final Context context, final Intent intent) 
    {
        // just make sure we are getting the right intent (better safe than sorry)
        if(Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) 
        {
            AntService.startService(context);
        }
        else
        {
            Log.w(TAG, "Received unexpected intent " + intent.toString());
        }
    }
}
