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
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.wfdsinktest.WfdSinkActivity;

public class UibcActivity extends Activity implements SensorEventListener {
    final static String TAG = "uibcActivity";
    final static int GENERIC_INPUT_TYPE_ID_TOUCH_DOWN = 0;
    final static int GENERIC_INPUT_TYPE_ID_TOUCH_UP = 1;
    final static int GENERIC_INPUT_TYPE_ID_TOUCH_MOVE = 2;
    final static int GENERIC_INPUT_TYPE_ID_ZOOM = 5;

    private float mPosX;
    private float mPosY;

    private float mLastTouchX;
    private float mLastTouchY;

    private float[] mAccelerometer_values;
    private float[] mMagnitude_values;

    private ScaleGestureDetector mScaleDetector;
    private float mScaleFactor = 1.f;
    private SensorManager mSensorManager;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        TextView tv = new TextView(this);
        tv.setText("Start rendering...");
        setContentView(tv);

        mScaleDetector = new ScaleGestureDetector(this, new ScaleListener());
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
    }

    @Override
    protected void onResume() {
        super.onResume();

        if(!mSensorManager.registerListener(this, mSensorManager
            .getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_UI)) {
            Log.w(TAG, "ACCELEROMETER sensor not found!");
            mSensorManager.unregisterListener(this);
            return;
        }

        if(!mSensorManager.registerListener(this, mSensorManager
            .getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD), SensorManager.SENSOR_DELAY_UI)) {
            Log.w(TAG, "MAGNETIC sensor not found!");
            mSensorManager.unregisterListener(this);
        }
        if(!mSensorManager.registerListener(this, mSensorManager
            .getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_UI)) {
            Log.w(TAG, "TYPE_ROTATION_VECTOR sensor not found!");
            mSensorManager.unregisterListener(this);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mSensorManager.unregisterListener(this);
        finish();
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        // Let the ScaleGestureDetector inspect all events.
        mScaleDetector.onTouchEvent(ev);
        final int action = ev.getAction();
        Log.d(TAG, "onTouchEvent action=" + action);

        StringBuilder eventDesc = new StringBuilder();
        
        switch (action & MotionEvent.ACTION_MASK) {
        case MotionEvent.ACTION_DOWN: 
            eventDesc.append(String.valueOf(GENERIC_INPUT_TYPE_ID_TOUCH_DOWN))
                .append(",");
            eventDesc.append(getTouchEventDesc(ev));
            Log.d(TAG, "onTouchEvent MotionEvent.ACTION_DOWN eventDesc=" + eventDesc);
            WfdSinkActivity.sendGenericTouchEvent_Navive(eventDesc.toString());

            // Test key event
            WfdSinkActivity.sendGenericKeyEvent_Navive("3, 0x0041, 0x0000");
            WfdSinkActivity.sendGenericKeyEvent_Navive("4, 0x0041, 0x0000");

            // Test rotate event
            WfdSinkActivity.sendGenericRotateEvent_Navive("8, 0, 128"); // -180 degree
            WfdSinkActivity.sendGenericRotateEvent_Navive("8, 1, 62"); // +180 degree

            // Test scroll event
            // format: "typeId, unit, direction, amount to scroll"
            WfdSinkActivity.sendGenericScaleEvent_Navive("6, 0, 0, 15"); // vertically scroll down for 15 pixels
            WfdSinkActivity.sendGenericScaleEvent_Navive("6, 0, 1, 15"); // vertically scroll up for 15 pixels
            WfdSinkActivity.sendGenericScaleEvent_Navive("7, 0, 0, 30"); // horizontally scroll right for 30 pixels
            WfdSinkActivity.sendGenericScaleEvent_Navive("7, 0, 1, 30"); // horizontally scroll left for 30 pixels
            break;
            
        case MotionEvent.ACTION_POINTER_UP:
            eventDesc.append(String.valueOf(GENERIC_INPUT_TYPE_ID_TOUCH_UP))
                .append("," + getTouchEventDesc(ev));
            WfdSinkActivity.sendGenericTouchEvent_Navive(eventDesc.toString());
            break;
            
        case MotionEvent.ACTION_MOVE: 
            // Only move if the ScaleGestureDetector isn't processing a gesture.
            if (!mScaleDetector.isInProgress()) {

            }
            eventDesc.append(String.valueOf(GENERIC_INPUT_TYPE_ID_TOUCH_MOVE))
                .append("," + getTouchEventDesc(ev));
            WfdSinkActivity.sendGenericTouchEvent_Navive(eventDesc.toString());
            break;

        default: 
            break;
        }

        return true;
    }

    private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            mScaleFactor *= detector.getScaleFactor();

            // Don't let the object get too small or too large.
            mScaleFactor = Math.max(0.1f, Math.min(mScaleFactor, 10.0f));
            Log.d(TAG, "onScale mScaleFactor=" + mScaleFactor);

            double fractionalPart = mScaleFactor % 1;
            double integralPart = mScaleFactor - fractionalPart;
            int i_integralPart = (int)integralPart;
            double uibcFractionalPart = fractionalPart * 256;
            int i_uibcFractionalPart = (int)uibcFractionalPart;

            StringBuilder eventDesc = new StringBuilder();
            eventDesc.append(String.valueOf(GENERIC_INPUT_TYPE_ID_ZOOM))
                .append("," + String.valueOf((int)(detector.getFocusX())))
                .append("," + String.valueOf((int)(detector.getFocusY())))
                .append("," + String.valueOf(i_integralPart))
                .append("," + String.valueOf(i_uibcFractionalPart));
            Log.d(TAG, "onScale eventDesc=" + eventDesc.toString());
            WfdSinkActivity.sendGenericZoomEvent_Navive(eventDesc.toString());

            return true;
        }
    }

    private String getTouchEventDesc(MotionEvent ev) {
        final int pointerCount = ev.getPointerCount();
        StringBuilder eventDesc = new StringBuilder();
        eventDesc.append(String.valueOf(pointerCount))
            .append(",");
        for (int p = 0; p < pointerCount; p++) {
            eventDesc.append(String.valueOf(ev.getPointerId(p)))
                .append("," + String.valueOf((int)(ev.getXPrecision() * ev.getX(p))))
                .append("," + String.valueOf((int)(ev.getYPrecision() * ev.getY(p))))
                .append(",");
        }
        return eventDesc.toString();
    }


    @Override
    public void onSensorChanged(SensorEvent event) {
        float[] eventValues;
        Log.d(TAG, "onSensorChanged event.sensor.getType()=" + event.sensor.getType());
        switch (event.sensor.getType()) {
        case Sensor.TYPE_ACCELEROMETER:
            mAccelerometer_values = (float[]) event.values.clone();
            break;
        case Sensor.TYPE_MAGNETIC_FIELD:
            mMagnitude_values = (float[]) event.values.clone();
            break;
        case Sensor.TYPE_ROTATION_VECTOR:
            StringBuilder sensorInfo = new StringBuilder();
            eventValues = (float[]) event.values.clone();
            for (int i = 0; i < eventValues.length; i++)
                sensorInfo.append("-values[" + i + "] = " + eventValues[i] + "\n");
            Log.d(TAG, "onSensorChanged TYPE_ROTATION_VECTOR eventValues=" + sensorInfo.toString());
            break;
        default:
            break;
        }

        if (mMagnitude_values != null && mAccelerometer_values != null) {
            float[] R = new float[9];
            float[] values = new float[3];
            SensorManager.getRotationMatrix(R, null, 
                mAccelerometer_values, mMagnitude_values);

            SensorManager.getOrientation(R, values);
            StringBuilder sensorInfo = new StringBuilder();
            for (int i = 0; i < values.length; i++) {
                sensorInfo.append("-values[" + i + "] = " + values[i] + "\n");
            }
            Log.d(TAG, "onSensorChanged sensorInfo=" + sensorInfo.toString());
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {}

    @Override
    public void onDestroy() {
        super.onDestroy();
        WfdSinkActivity.stopWfdSink_Navive();
    }
}
