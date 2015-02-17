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

import android.content.ContentValues;
import android.content.Context;
import android.provider.Telephony.Sms.Inbox;
import android.telephony.SmsMessage;

import com.android.internal.telephony.SmsHeader;

import com.mediatek.mms.ext.SmsReceiverImpl;
import com.mediatek.xlog.Xlog;

public class Op01SmsReceiverExt extends SmsReceiverImpl {
    private static final String TAG = "Mms/Op01SmsReceiverExt";

    public Op01SmsReceiverExt(Context context) {
        super(context);
    }

    public void extractSmsBody(SmsMessage[] msgs, SmsMessage sms, ContentValues values) {
        Xlog.d(TAG, "Op01SmsReceiverExt.extractSmsBody");

        int pduCount = msgs.length;
        boolean hasMissedSegments = checkConcateRef(sms.getUserDataHeader(), pduCount);

        Xlog.d(TAG, "pduCount=" + pduCount);
        
        if (hasMissedSegments) {
            int totalParts = sms.getUserDataHeader().concatRef.msgCount;
            Xlog.v(TAG, "[fake process missed segment(s) " + pduCount + "/" + totalParts);
            String messageBody = handleMissedParts(msgs);
            if (messageBody != null) {
                values.put(Inbox.BODY, messageBody);
            }
        } else {
            if (pduCount == 1) {
                // There is only one part, so grab the body directly.
                values.put(Inbox.BODY, sms.getDisplayMessageBody());
            } else {
                // Build up the body from the parts.
                StringBuilder body = new StringBuilder();
                for (int i = 0; i < pduCount; i++) {
                    SmsMessage msg;
                    msg = msgs[i];
                    body.append(msg.getDisplayMessageBody());
                }
                values.put(Inbox.BODY, body.toString());
            }
        }
    }

    private boolean checkConcateRef(SmsHeader udh, int actualPartsNum) {
        if (udh == null || udh.concatRef == null) {
            Xlog.d(TAG, "[fake not concate message");
            return false;
        } else {
            int totalPartsNum = udh.concatRef.msgCount;
            if (totalPartsNum > actualPartsNum) {
                Xlog.d(TAG, "[fake missed segment(s) "
                        + (totalPartsNum - actualPartsNum));
                return true;
            }
        }
        
        return false;
    }

    private String handleMissedParts(SmsMessage[] parts) {
        if (parts == null || parts.length <= 0) {
            Xlog.e(TAG, "[fake invalid message array");
            return null;
        }
        
        SmsHeader udh = parts[0].getUserDataHeader();
        int totalPartsNum = udh.concatRef.msgCount;
        //int actualPartsNum = parts.length;
        
        String[] fakeContents = new String[totalPartsNum];
        for (SmsMessage msg : parts) {
            int seq = msg.getUserDataHeader().concatRef.seqNumber;
            Xlog.d(TAG, "[fake add segment " + seq);
            fakeContents[seq - 1] = msg.getDisplayMessageBody();
        }
        for (int i = 0; i < fakeContents.length; ++i) {
            if (fakeContents[i] == null) {
                Xlog.d(TAG, "[fake replace segment " + (i + 1));
                fakeContents[i] = "(...)";
            }
        }
        
        StringBuilder body = new StringBuilder();
        for (String s : fakeContents) {
            body.append(s);
        }
        return body.toString();
    }
}
