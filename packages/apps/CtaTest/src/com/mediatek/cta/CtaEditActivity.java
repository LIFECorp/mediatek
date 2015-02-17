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

package com.mediatek.cta;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.app.LoaderManager.LoaderCallbacks;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.MediaRecorder;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.CallLog;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.provider.Telephony;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Mms;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CursorAdapter;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.SimpleCursorAdapter;
import android.widget.SimpleCursorAdapter.ViewBinder;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import com.mediatek.cta.R;
import com.mediatek.cta.camera.Camera;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Date;
import java.text.SimpleDateFormat;

public class CtaEditActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CTA";
    public static final String TYPE = "type";
    public static final String TEXT1 = "text1";
    public static final String ID = "id";

    public static final int TYPE_CONTACTS = 0;
    public static final int TYPE_CALL_LOG = 1;
    public static final int TYPE_SMS = 2;
    public static final int TYPE_MMS = 3;

    private int mType;
    private int mId;
    private SimpleCursorAdapter mAdapter;
    private TextView mLabel;
    private EditText mEditor;
    private Button mButtonWrite;
    private Button mButtonDelete;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cta_edit);
        mLabel = (TextView) findViewById(R.id.cta_label_1);
        mEditor = (EditText) findViewById(R.id.cta_editor_1);
        mButtonWrite = (Button) findViewById(R.id.cta_button_write);
        mButtonDelete = (Button) findViewById(R.id.cta_button_delete);
        mButtonWrite.setOnClickListener(this);
        mButtonDelete.setOnClickListener(this);

        mType = getIntent().getIntExtra(TYPE, TYPE_CONTACTS);
        mEditor.setText(getIntent().getStringExtra(TEXT1));
        mId = getIntent().getIntExtra(ID, 0);

        switch (mType) {
        case TYPE_CONTACTS:
            mLabel.setText("Name:");
            break;
        case TYPE_CALL_LOG:
            mLabel.setText("Phone Number:");
            break;
        case TYPE_SMS:
            mLabel.setText("Content:");
            break;
        case TYPE_MMS:
            mLabel.setText("Subject:");
            break;
        default:
            return;
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonWrite) {
            ContentValues values = new ContentValues();
            String text = mEditor.getText().toString();
            try {
                switch (mType) {
                case TYPE_CONTACTS:
                    values.put(StructuredName.GIVEN_NAME, "");
                    values.put(StructuredName.FAMILY_NAME, "");
                    values.put(StructuredName.PREFIX, "");
                    values.put(StructuredName.MIDDLE_NAME, "");
                    values.put(StructuredName.SUFFIX, "");
                    values.put(StructuredName.DISPLAY_NAME, text);
                    getContentResolver().update(Data.CONTENT_URI, values,
                            Data.CONTACT_ID + " = ? AND " + Data.MIMETYPE + " = ?",
                            new String[] {String.valueOf(mId), StructuredName.CONTENT_ITEM_TYPE});
                    break;
                case TYPE_CALL_LOG:
                    values.put(CallLog.Calls.NUMBER, text);
                    getContentResolver().update(CallLog.Calls.CONTENT_URI, values,
                            CallLog.Calls._ID + " = ?", new String[] {String.valueOf(mId)});
                    break;
                case TYPE_SMS:
                    values.put(Sms.BODY, text);
                    getContentResolver().update(Sms.CONTENT_URI, values,
                            Sms._ID + " = ?", new String[] {String.valueOf(mId)});
                    break;
                case TYPE_MMS:
                    values.put(Mms.SUBJECT, text);
                    getContentResolver().update(Mms.CONTENT_URI, values,
                            Sms._ID + " = ?", new String[] {String.valueOf(mId)});
                    break;
                default:
                    return;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            finish();
        }
        if (v == mButtonDelete) {
            switch (mType) {
            case TYPE_CONTACTS:
                getContentResolver().delete(Data.CONTENT_URI,
                        Data.CONTACT_ID + " = ?", new String[] {String.valueOf(mId)});
                getContentResolver().delete(RawContacts.CONTENT_URI,
                        RawContacts.CONTACT_ID + " = ?", new String[] {String.valueOf(mId)});
                getContentResolver().delete(Contacts.CONTENT_URI,
                        Contacts._ID + " = ?", new String[] {String.valueOf(mId)});
                break;
            case TYPE_CALL_LOG:
                getContentResolver().delete(CallLog.Calls.CONTENT_URI,
                        CallLog.Calls._ID + " = ?", new String[] {String.valueOf(mId)});
                break;
            case TYPE_SMS:
                getContentResolver().delete(Sms.CONTENT_URI,
                        Sms._ID + " = ?", new String[] {String.valueOf(mId)});
                break;
            case TYPE_MMS:
                getContentResolver().delete(Mms.CONTENT_URI,
                        Mms._ID + " = ?", new String[] {String.valueOf(mId)});
                break;
            default:
            }
            finish();
        }
    }
}
