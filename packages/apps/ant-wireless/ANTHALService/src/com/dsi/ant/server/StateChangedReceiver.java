/*
 Copyright 2013 Dynastream Innovations

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

package com.dsi.ant.server;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class StateChangedReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        // Bluetooth State Changed
        if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(intent.getAction())) {

            int bluetoothState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0);

            if (BluetoothAdapter.STATE_OFF == bluetoothState) {
                Intent antIntent = new Intent(AntService.ACTION_REQUEST_DISABLE);
                context.sendBroadcast(antIntent, AntService.ANT_ADMIN_PERMISSION);
            } else if (BluetoothAdapter.STATE_ON == bluetoothState) {
                Intent antIntent = new Intent(AntService.ACTION_REQUEST_ENABLE);
                context.sendBroadcast(antIntent, AntService.ANT_ADMIN_PERMISSION);
            }
        } // else if <<Another device dependency>> state change

    }
}
