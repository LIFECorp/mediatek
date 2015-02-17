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

package com.mediatek.woreader;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.Toast;
import android.util.Log;

public class WoReader extends Activity {
    private static final String TAG = "WoReader";
    private static final String url = "http://iread.wo.com.cn";
    private static final int REQUEST_SETTINGS_CODE = 1;
    private NetworkInfo info = null;	

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        info = ((ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE)).getActiveNetworkInfo();
        if(info!=null && info.isConnected()) {
            if(info.getTypeName().equalsIgnoreCase("mobile")) {
                String apn = info.getExtraInfo();
                if(apn!=null && (apn.equalsIgnoreCase("3gnet") || apn.equalsIgnoreCase("3gwap") || apn.equalsIgnoreCase("uninet") || apn.equalsIgnoreCase("uniwap"))) {
                    launchBrowser();
                } else {
                    new AlertDialog.Builder(this)
                        .setTitle(R.string.alertTitle)
                        .setMessage(R.string.setupCUMsg)
                        .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                Intent i = new Intent();
                                //i.setAction("android.intent.action.MAIN");	
                                i.setAction("android.settings.GEMINI_MANAGEMENT");
                                //i.setClassName("com.android.settings","com.android.settings.WirelessSettings");
                                startActivityForResult(i, REQUEST_SETTINGS_CODE);
                           }
                        })
                        .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                finish();
                            }
                        })
                        .setOnKeyListener(new DialogInterface.OnKeyListener() {
                            public boolean onKey(DialogInterface dialog, int KeyCode, KeyEvent event) {
                                switch(KeyCode) {
                                    case KeyEvent.KEYCODE_BACK:
                                        finish();
                                        break;
                                    default:
                                        break;
                                }
                                return true;
                            }	
                        })
                        .show();
                }
            } else {	//use wifi network.
                showDialog(getString(R.string.requireCUMsg));   
            }
        } else {
            showDialog(getString(R.string.noNetworkMsg));
        }
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        switch (requestCode) {
            case REQUEST_SETTINGS_CODE:
                info = ((ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE)).getActiveNetworkInfo();
                if(info == null) {	
                    Toast.makeText(this, R.string.noNetworkMsg, Toast.LENGTH_SHORT).show();
                    finish();
                    return;
                }
                String apn = info.getExtraInfo();
                if(apn!=null && (apn.equalsIgnoreCase("3gnet") || apn.equalsIgnoreCase("3gwap") || apn.equalsIgnoreCase("uninet") || apn.equalsIgnoreCase("uniwap"))) {
                    launchBrowser();
                } else {
                    Toast.makeText(this,R.string.requireCUMsg,Toast.LENGTH_SHORT).show();
                    finish();
                }
                break;
            default:
                break;
        }
    }

    private void launchBrowser() {
        Uri uri = Uri.parse(url);
        Intent i = new Intent(Intent.ACTION_VIEW, uri);
        i.setClassName("com.android.browser", "com.android.browser.BrowserActivity");
        try {
            startActivity(i);
        } catch (ActivityNotFoundException e) {
            // Some third party app can disable Browser, so we should catch it. And since
            // user disable Browser, we should do nothing.
            Log.d(TAG, "WoReader, no activity found to handle uri", e);
        }
        finish();
    }

    public void showDialog(String msg) {
    new AlertDialog.Builder(this)
        .setTitle(R.string.alertTitle)
        .setMessage(msg)
        .setPositiveButton(R.string.ok1,	new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        })
        .setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int KeyCode, KeyEvent event) {
                switch(KeyCode) {
                    case KeyEvent.KEYCODE_BACK:
                        finish();
                        break;
                    default:
                    break;
                }
                return true;
            }
        })
        .show();
    }

}
