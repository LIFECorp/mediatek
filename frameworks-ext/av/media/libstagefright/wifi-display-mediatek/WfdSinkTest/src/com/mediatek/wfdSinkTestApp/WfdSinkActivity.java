/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.mediatek.wfdsinktest;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class WfdSinkActivity extends Activity
{
    final static String TAG = "WfdSinkActivity";
    final static int GENERIC_INPUT_TYPE_ID_TOUCH_DOWN = 0;
    final static int GENERIC_INPUT_TYPE_ID_TOUCH_UP = 1;
    final static int GENERIC_INPUT_TYPE_ID_TOUCH_MOVE = 2;
    
    private float mPosX;
    private float mPosY;

    private float mLastTouchX;
    private float mLastTouchY;

    private ScaleGestureDetector mScaleDetector;
    private float mScaleFactor = 1.f;
    private boolean mConnected = false;
    
    static WfdSinkActivity sInstance = null;
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        sInstance = this;
        setContentView(R.layout.activity_main);
        
        TextView myTextField = (TextView)findViewById(R.id.myTextField1);
        myTextField.setText(jniTest());  
        
        final Button button = (Button) findViewById(R.id.connect);
        final EditText editText1=(EditText)findViewById(R.id.ip1);  
        final EditText editText2=(EditText)findViewById(R.id.ip2);  
        final EditText editText3=(EditText)findViewById(R.id.ip3);  
        final EditText editText4=(EditText)findViewById(R.id.ip4);
        final EditText editTextPort=(EditText)findViewById(R.id.port); 

        editText1.setText("192");
        editText2.setText("168");
        editText3.setText("0");
        editText4.setText("141");
        editTextPort.setText("10000");       
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                StringBuilder ipAddr = new StringBuilder();
                ipAddr.append(editText1.getText())
                    .append("." + editText2.getText())
                    .append("." + editText3.getText())
                    .append("." + editText4.getText());
                
                String strPort = editTextPort.getText().toString();
                int port =  Integer.parseInt(strPort);
                if (startWfdSink(ipAddr.toString(), port)) { 
                    mConnected = true;
                    Intent intent = new Intent();
                    intent.setClass(WfdSinkActivity.this, UibcActivity.class);
                    startActivity(intent);
                }
            }
        });       		
    }

    public native String jniTest();
    public native boolean startWfdSink(String srcAddr, int port);
    public native void stopWfdSink();
/*
    public native void sendGenericTouchEvent(String eventDesc);
    public native void sendGenericKeyEvent(String eventDesc);
    public native void sendGenericZoomEvent(String eventDesc);
    public native void sendGenericScaleEvent(String eventDesc);
    public native void sendGenericRotateEvent(String eventDesc);
    public native void sendGenericVendorEvent(String eventDesc);
*/
    static public boolean startWfdSink_Navive(String srcAddr, int port) {
        if (sInstance != null) {
            return sInstance.startWfdSink(srcAddr, port);
        }
        return false;
    }
    
    static public void stopWfdSink_Navive() {
        if (sInstance != null) {
            sInstance.stopWfdSink();
        }
    }
    
    // format: "typeId(1/2), number of pointers, pointer Id1, X coordnate, Y coordnate, , pointer Id2, X coordnate, Y coordnate,..."
    static public void sendGenericTouchEvent_Navive(String eventDesc) {
        if (sInstance != null) {
            //sInstance.sendGenericTouchEvent(eventDesc);
        }
    }
    
    // format: "typeId(3/4), Key code 1(0x0000), Key code 2(0x0000)"
    static public void sendGenericKeyEvent_Navive(String eventDesc) {
        if (sInstance != null) {
            //sInstance.sendGenericKeyEvent(eventDesc);
        }
    }
    
    // format: "typeId(5),  X coordnate, Y coordnate, integer part, fraction part"
    static public void sendGenericZoomEvent_Navive(String eventDesc) {
        if (sInstance != null) {
            //sInstance.sendGenericZoomEvent(eventDesc);
        }
    }
    
    // format: "typeId(6/7),  amount to scroll"
    static public void sendGenericScaleEvent_Navive(String eventDesc) {
        if (sInstance != null) {
            //sInstance.sendGenericScaleEvent(eventDesc);
        }
    }
    
    // format: "typeId(8),  integer part, fraction part"
    static public void sendGenericRotateEvent_Navive(String eventDesc) {
        if (sInstance != null) {
            //sInstance.sendGenericRotateEvent(eventDesc);
        }
    }
    
    // format: "typeId(255),  integer part, fraction part"
    static public void sendGenericVendorEvent_Navive(String eventDesc) {
        if (sInstance != null) {
            //sInstance.sendGenericVendorEvent(eventDesc);
        }
    }
    
    static {
        System.loadLibrary("wfdsink");
    }
}
