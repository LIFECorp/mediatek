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

package com.mediatek.cmcc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.text.TextUtils;
//import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class ManagerSetActivity extends Activity {

    private EditText mNameText;
    private EditText mNumberText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.manager_set_activity);
        //try {
        SharedPreferences preferences = getSharedPreferences("user", ManagerSetActivity.MODE_PRIVATE);
        String name = preferences.getString("name", "");
        String number = preferences.getString("number", "");

        mNameText = (EditText) findViewById(R.id.EditTextSetName);
        mNumberText = (EditText) findViewById(R.id.EditTextSetNumber);

        mNameText.setText(name);
        mNumberText.setText(number);

        View.OnClickListener listenner = new View.OnClickListener() {

            public void onClick(View v) {
                // TODO Auto-generated method stub
                switch (v.getId()) {
                case R.id.ButtonSetSave:
                    onSaveButtonClicked();
                    break;
                case R.id.ButtonSetCancel:
                    ManagerSetActivity.this.finish();
                    break;
                default:
                    break;
                }
            }
        };

        Button saveButton = (Button) findViewById(R.id.ButtonSetSave);
        saveButton.setOnClickListener(listenner);

        Button cancelButton = (Button) findViewById(R.id.ButtonSetCancel);
        cancelButton.setOnClickListener(listenner);
        //} catch (Exception e) {
            //e.printStackTrace();
        //}
        
    }

    private void onSaveButtonClicked() {
        //try {
        String number = mNumberText.getText().toString().trim();
        String name = mNameText.getText().toString().trim();
        if (TextUtils.isEmpty(number) || TextUtils.isEmpty(name)) {
            new AlertDialog.Builder(this).setTitle(R.string.app_name).setMessage(R.string.set_number_or_name_empty)
                            .setPositiveButton(R.string.str_ok, null).show();
            return;
        }
        SharedPreferences preferences = getSharedPreferences("user", Context.MODE_WORLD_READABLE);
        Editor edit = preferences.edit();
        edit.putString("name", name);
        edit.putString("number", number);
        edit.commit();
        Toast.makeText(this, R.string.save_success, Toast.LENGTH_LONG).show();
        this.finish();
        //} catch (Exception e) {
            //e.printStackTrace();
            //Log.v("ManagerSetActivity", "onSaveButtonClicked save prefs exception!");
        //}
    }

}
