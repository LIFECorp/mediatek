/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.settings.plugin;

import android.content.Context;

import com.mediatek.settings.ext.DefaultDataUsageSummaryExt;
import com.mediatek.op02.plugin.R;
import com.mediatek.xlog.Xlog;

public class OP02DataUsageSummaryExt extends DefaultDataUsageSummaryExt {

    private static final String TAG = "OP02DataUsageSummaryExt";

    private static final String TAG_BG_DATA_SWITCH = "bgDataSwitch";
    private static final String TAG_BG_DATA_SUMMARY = "bgDataSummary";
    private static final String TAG_BG_DATA_APP_DIALOG_TITLE = "bgDataDialogTitle";
    private static final String TAG_BG_DATA_APP_DIALOG_MESSAGE = "bgDataDialogMessage";
    private static final String TAG_BG_DATA_MENU_DIALOG_MESSAGE = "bgDataMenuDialogMessage";
    private static final String TAG_BG_DATA_RESTRICT_DENY_MESSAGE = "bgDataRestrictDenyMessage";

    private Context mContext;

    public OP02DataUsageSummaryExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public String customizeBackgroundString(String defStr, String tag) {
        String cuBgDataStr = defStr;
        if (tag == TAG_BG_DATA_SWITCH) {
             cuBgDataStr = mContext.getString(
                    R.string.data_background_restrict_CU);
        }
        else if (tag == TAG_BG_DATA_SUMMARY) {
            cuBgDataStr = mContext.getString(
                    R.string.data_usage_app_restrict_background_summary_CU);
        }
        else if (tag == TAG_BG_DATA_APP_DIALOG_TITLE) {
            cuBgDataStr = mContext.getString(
                    R.string.data_usage_app_restrict_dialog_title_CU);
        }
        else if (tag == TAG_BG_DATA_APP_DIALOG_MESSAGE) {
            cuBgDataStr = mContext.getString(
                    R.string.data_usage_app_restrict_dialog_CU);
        }
        else if (tag == TAG_BG_DATA_MENU_DIALOG_MESSAGE) {
            cuBgDataStr = mContext.getString(
                    R.string.data_usage_restrict_background_CU);
        }
        else if (tag == TAG_BG_DATA_RESTRICT_DENY_MESSAGE) {
            cuBgDataStr = mContext.getString(
                    R.string.data_usage_restrict_denied_dialog_CU);
        }
        Xlog.d(TAG, "cuBgDataStr: " + cuBgDataStr);
        return cuBgDataStr;
    }
}
