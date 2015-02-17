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

package com.mediatek.op.sms;

import com.mediatek.common.sms.IDupSmsFilterExt;
import android.content.Context;
import com.mediatek.xlog.Xlog;
import java.util.Arrays;

public class DupSmsFilterExtOP01 extends DupSmsFilterExt {

    private static String TAG = "DupSmsFilterExtOP01";

    public DupSmsFilterExtOP01(Context context, int simId) {
        super(context, simId);
        Xlog.d(TAG, "call OP01 constructor");
    }

    protected boolean isDupSms(byte[] newPdu, byte[] oldPdu) {
        Xlog.d(TAG, "call OP01 isDupSms");

        // no SC
        if (newPdu.length != oldPdu.length) {
            Xlog.d(TAG, "OP01 DupSms, different length");
            return false;
        }
        int newPduLen = newPdu[0] & 0xff;
        byte[] newPduByte = new byte[newPdu.length - newPduLen -1];
        System.arraycopy(newPdu, newPduLen + 1, newPduByte, 0, newPduByte.length);
        int oldPduLen = oldPdu[0] & 0xff;
        byte[] oldPduByte = new byte[oldPdu.length - oldPduLen -1];
        System.arraycopy(oldPdu, oldPduLen + 1, oldPduByte, 0, oldPduByte.length);

        if (Arrays.equals(newPduByte, oldPduByte)) {
            Xlog.d(TAG, "find a OP01 duplicated sms");
            return true;
        }
        Xlog.d(TAG, "not a OP01 dup sms");
        return false;
    }
}

