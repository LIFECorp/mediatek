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

/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.cmcc;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.util.Log;

public class CustomerServiceProvider extends ContentProvider {
    private static final String TAG = "CustomerServiceProvider";
    private static final String AUTHORITY = "com.mediatek.cmcc.provider";
    private static final String PATH = "phoneinfo";
    private static final int PHONE_INFO = 1;

    private static final String[] COLUMNS_NAME = {
        "phone_model", "service_number", "service_web"
    };

    private UriMatcher mUriMatcher = null;

    @Override
    public boolean onCreate() {
        Log.i(TAG, "CustomerServiceProvider onCreate()");
        mUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        mUriMatcher.addURI(AUTHORITY, PATH, PHONE_INFO);

        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        Log.i(TAG, "CustomerServiceProvider query()");
        switch (mUriMatcher.match(uri)) {
            case PHONE_INFO:
                return qurerPhoneInfo();
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    @Override
    public String getType(Uri uri) {
        /*
        switch (mUriMatcher.match(uri)) {
            case PHONE_INFO:
                return "vnd.android.cursor.dir/vnd.mediatek.phoneinfo";
            default:
                throw new IllegalArgumentException("Unknown URL" + uri);
        }*/
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
       return null;
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        return 0;
    }

    private MatrixCursor qurerPhoneInfo() {
        Log.i(TAG, "CustomerServiceProvider qurerPhoneInfo()");

        String[] rowItems = new String[COLUMNS_NAME.length];
        rowItems[0] = getContext().getResources().getString(R.string.phone_type_content);
        rowItems[1] = getContext().getResources().getString(R.string.after_service_phone_num);
        rowItems[2] = getContext().getResources().getString(R.string.web_address);

        MatrixCursor cur = new MatrixCursor(COLUMNS_NAME);
        cur.addRow(rowItems);
        return cur;
    }

    
    /**
     * Use CustomerServiceProvider
     * Method one by activity:
     * String[] mInfo = new String[3];
       Uri uri = Uri.parse("content://com.mediatek.cmcc.provider/phoneinfo");
       Cursor cur = managedQuery(uri, null, null, null, null); 
       if (cur != null && cur.moveToFirst()) {
           do {
               mInfo[0] = cur.getString(cur.getColumnIndex("phone_model"));
               mInfo[1] = cur.getString(cur.getColumnIndex("service_number"));
               mInfo[2] = cur.getString(cur.getColumnIndex("service_web"));
           } while (cur.moveToNext());
       }

     * Method two needs to close cusor:
     * String[] mInfo = new String[3];
       Uri uri = Uri.parse("content://com.mediatek.cmcc.provider/phoneinfo");
       Cursor cursor = getContentResolver().query(uri, null, null, null, null);
       if (cur != null && cur.moveToFirst()) {
           do {
               mInfo[0] = cur.getString(cur.getColumnIndex("phone_model"));
               mInfo[1] = cur.getString(cur.getColumnIndex("service_number"));
               mInfo[2] = cur.getString(cur.getColumnIndex("service_web"));
           } while (cur.moveToNext());
       }
       cursor.close();
     */
}
