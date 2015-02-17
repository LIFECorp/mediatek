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

package com.mediatek.mms.op02;

import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.drawable.Drawable;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;

import com.mediatek.mms.ext.IMmsConversation;
import com.mediatek.mms.ext.MmsConversationImpl;
import com.mediatek.mms.ext.IMmsConversationHost;
import com.mediatek.op02.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

import java.util.List;

public class Op02MmsConversationExt extends MmsConversationImpl{

    private static final String TAG = "Mms/Op02MmsConversationExt";

    private int mMenuSimSms = 0;
    
    public Op02MmsConversationExt(Context context) {
        super(context);
    }

    public void addOptionMenu(Menu menu, int base){
        Drawable sMenuIcon = 
            getResources().getDrawable(R.drawable.ic_menu_sim_sms);
        mMenuSimSms = base + 1;
        menu.add(0, mMenuSimSms, 0, getString(R.string.menu_sim_sms)).setIcon(
                    R.drawable.ic_menu_sim_sms);
        
        Xlog.d(TAG, "Add Menu: " + getString(R.string.menu_sim_sms));
        
        MenuItem item = menu.findItem(mMenuSimSms);
        List<SimInfoRecord> listSimInfo = SimInfoManager.getInsertedSimInfoList(this);
        if(listSimInfo == null || listSimInfo.isEmpty()){
            item.setEnabled(false);
            Xlog.d(TAG, "Menu: " + getString(R.string.menu_sim_sms) + " is disabled");
        }
        
        return;
    }

    public boolean onOptionsItemSelected(MenuItem item){
        if(item.getItemId() == mMenuSimSms){
            getHost().showSimSms();
        } else {
            return false;
        }
        return true;
    }
}

