/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.antenna;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;
import java.util.HashSet;

public class AntennaActivity extends Activity {
    private static final String TAG = "AntennaTest";
    private static final int MSG_QUERY_SUPPORT_ANTENNA_MODE = 0;
    private static final int MSG_QUERY_ANTENNA_MODE = 1;
    private static final int MSG_SET_ANTENNA_MODE = 2;

    private HashSet<Integer> mSupportModes = new HashSet<Integer>();
    private GeminiPhone mGeminiPhone = null;
    private Phone mPhone = null;
    private Spinner mSpinner = null;
    private Toast mToast = null;
    private int mCurrentPos = 0;

    private final OnItemSelectedListener mItemSelectedListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> arg0, View arg1, int pos, long arg3) {
            if (mCurrentPos == arg0.getSelectedItemPosition()) {
                return;
            }
            mCurrentPos = arg0.getSelectedItemPosition();

            if (mSupportModes.contains(pos)) {
                setMode(pos);
            } else {
                showToast("Not supported.");
                queryCurrentMode();
            }
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
            // Do nothing
        }
    };

    private final Handler mCommandHander = new Handler() {
        @Override
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult;
            switch (msg.what) {
            case MSG_QUERY_SUPPORT_ANTENNA_MODE:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.exception == null && asyncResult.result != null) {
                    final String[] result = (String[]) asyncResult.result;
                    Xlog.i(TAG, "Get support mode " + result[0]);

                    // query result is like: +ERXPATH: (0,1,2)
                    int start = result[0].indexOf('(', 0);
                    int end = result[0].indexOf(')', 0);
                    if (start >= 0 && start + 1 < end) {
                        String[] modes = result[0].substring(start + 1, end).split(",");
                        for (int i = 0; i < modes.length; i++) {
                            try {
                                mSupportModes.add(Integer.parseInt(modes[i].trim()));
                            } catch (NumberFormatException e) {
                            }
                        }
                    }

                    queryCurrentMode();
                } else {
                    showToast("Query antenna mode failed.");
                }
                break;
            case MSG_QUERY_ANTENNA_MODE:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.exception == null && asyncResult.result != null) {
                    final String[] result = (String[]) asyncResult.result;
                    Xlog.i(TAG, "Get mode " + result[0]);

                    // query result is like: +ERXPATH: 1
                    int mode = -1;
                    try {
                        mode = Integer.valueOf(result[0].substring("+ERXPATH:".length()).trim());
                    } catch (NumberFormatException e) {
                    }

                    if (mode < 0 || mode >= mSpinner.getCount()) {
                        showToast("Modem returned invalid mode: " + result);
                        return;
                    }

                    mCurrentPos = mode;
                    mSpinner.setSelection(mode);
                    mSpinner.setEnabled(true);
                } else {
                    showToast("Query antenna mode failed.");
                }
                break;
            case MSG_SET_ANTENNA_MODE:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {
                    showToast("Set successful.");
                } else {
                    showToast("Set failed.");
                }
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.antena_test);

        mSpinner = (Spinner) findViewById(R.id.spinner_antenna);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,
                getResources().getStringArray(R.array.antenna_modes));
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(adapter);
        mSpinner.setOnItemSelectedListener(mItemSelectedListener);
        mSpinner.setEnabled(false);

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone)PhoneFactory.getDefaultPhone();
        } else {
            mPhone = (Phone)PhoneFactory.getDefaultPhone();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        querySupportMode();
    }

    private void queryCurrentMode() {
        sendCommand(new String[] {"AT+ERXPATH?", "+ERXPATH:"}, MSG_QUERY_ANTENNA_MODE);
    }

    private void querySupportMode() {
        sendCommand(new String[] {"AT+ERXPATH=?", "+ERXPATH:"}, MSG_QUERY_SUPPORT_ANTENNA_MODE);
    }

    private void setMode(int mode) {
        Xlog.i(TAG, "Set mode " + mode);
        sendCommand(new String[] {"AT+ERXPATH=" + mode, ""}, MSG_SET_ANTENNA_MODE);
    }

    private void sendCommand(String[] command, int msg) {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone.invokeOemRilRequestStringsGemini(command,
                    mCommandHander.obtainMessage(msg), PhoneConstants.GEMINI_SIM_1);
        } else {
            mPhone.invokeOemRilRequestStrings(command, mCommandHander.obtainMessage(msg));
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
