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

import android.content.Context;
import android.content.ContextWrapper;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.widget.EditText;
import android.content.Intent;

import android.os.Bundle;
import android.widget.TextView;
import com.mediatek.mms.ext.IMmsAttachmentEnhance;
import com.mediatek.mms.ext.MmsAttachmentEnhanceImpl;
//import com.mediatek.xlog.Xlog;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.SXlog;



public class Op01MmsAttachmentEnhanceExt extends MmsAttachmentEnhanceImpl {

     private static final String TAG = "Mms/Op01MmsAttachmentEnhanceExt";
    // private static final int MMS_SAVE_OTHER_ATTACHMENT = 0;
    // private static final int MMS_SAVE_ALL_ATTACHMENT = 1;
     private static final String MMS_SAVE_MODE = "savecontent" ;

    public Op01MmsAttachmentEnhanceExt(Context context) {
        super(context);
    }

     // It is a common function for check if it is plugin
     public boolean isSupportAttachmentEnhance() {
         return true;
     }

     //Set attachment name
     public void setAttachmentName(TextView text, int size) {
        if (size > 1) {
            text.setText(getString(R.string.multi_files));
        }
     }

    //Set save attachment intent according to smode
     public void setSaveAttachIntent(Intent i, int smode) {
        if (smode == MMS_SAVE_OTHER_ATTACHMENT ||
            smode == MMS_SAVE_ALL_ATTACHMENT) {
            Bundle data = new Bundle();
            data.putInt(MMS_SAVE_MODE,smode);
            i.putExtras(data);
        }
     }

    //get save attachment mode through intent
     public int getSaveAttachMode(Intent i) {
        int smode = -1;
        Bundle data = i.getExtras();
        if (data != null) {
            smode = data.getInt(MMS_SAVE_MODE);
        }
        return smode;
     }
}
