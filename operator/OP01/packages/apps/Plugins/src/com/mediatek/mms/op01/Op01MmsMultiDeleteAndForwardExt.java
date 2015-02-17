/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.mms.op01;

import android.database.Cursor;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.drawable.Drawable;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.mms.ext.MmsMultiDeleteAndForwardImpl;
import com.mediatek.mms.ext.IMmsMultiDeleteAndForwardHost;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import java.util.List;

public class Op01MmsMultiDeleteAndForwardExt extends MmsMultiDeleteAndForwardImpl {
    private static final String TAG = "Mms/Op01MmsMultiDeleteAndForwardExt";

    private int mMenuForward = 0;
    private MenuItem item = null;
    
    //add for multi-forward
    private Map<Long, BodyandAddress> mBodyandAddressItem;
    class BodyandAddress{
        String mAddress;
        String mBody;
        int mBoxType;
        public BodyandAddress(String mAddress, String mBody, int boxType) {
            super();
            this.mAddress = mAddress;
            this.mBody = mBody;
            this.mBoxType = boxType;
        }  
    }
    
    public void setBodyandAddress(Cursor mcursor,int mColumnSmsAddress,int mColumnSmsBody,
            int mColumnBoxId,String mtype,long msgId) {
        if (mtype.equals("mms")) {  
        } else {
            String mAddress = mcursor.getString(mColumnSmsAddress);
            String mBody    = mcursor.getString(mColumnSmsBody);
            int boxType = mcursor.getInt(mColumnBoxId);
            Xlog.d(TAG,"initListMap mAddress = "+mAddress +"mBody"+mBody + ", boxid = " + boxType);
            BodyandAddress  ba = new BodyandAddress(mAddress,mBody, boxType);
            mBodyandAddressItem.put(msgId, ba);
        }
    }
    
    public  String getBody(long id) {
        if (mBodyandAddressItem != null && mBodyandAddressItem.size() > 0) {
            return mBodyandAddressItem.get(id).mBody;
        } else {
            return null;
        }
    }
    
    public  String getAddress(long id) {
        if (mBodyandAddressItem != null && mBodyandAddressItem.size() > 0) {
            return mBodyandAddressItem.get(id).mAddress;
        } else {
            return null;
        }
    }

    public int getBoxType(long id) {
        if (mBodyandAddressItem != null && mBodyandAddressItem.size() > 0) {
            return mBodyandAddressItem.get(id).mBoxType;
        } else {
            return -1;
        }
    }

    public void initBodyandAddress() {
        mBodyandAddressItem = new HashMap<Long, BodyandAddress>();
    }
    
    public void clearBodyandAddressList() {
        if (mBodyandAddressItem != null) {
            mBodyandAddressItem.clear();
        }
    }
    
    public Op01MmsMultiDeleteAndForwardExt(Context context) {
        super(context);
    }

    public boolean onMultiforwardItemSelected() {
        Xlog.d(TAG,"onMultiforwardItemSelected  ");
        getHost().prepareToForwardMessage();
        return true;
    }
    
    public void onNewBodyandAddressForTest() {
        BodyandAddress  ba = new BodyandAddress("10086","test",0);
    }
}

