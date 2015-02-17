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

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.TabHost.TabSpec;
import android.widget.TabWidget;
import android.widget.TextView;
    
import com.mediatek.settings.ext.DefaultDataUsageSummaryExt;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op01.plugin.R;
import com.mediatek.telephony.SimInfoManager;

public class DataUsageSummaryExt extends DefaultDataUsageSummaryExt {
    
    private static final String TAB_SIM_1 = "sim1";
    private static final String TAB_SIM_2 = "sim2";
    private static final String TAB_MOBILE = "mobile";
    private static final String GSETTINGS_PROVIDER = "com.google.settings";
    private static final String TAG = "SettingsMiscExt";
    private Context mContext;

    public DataUsageSummaryExt(Context context) {
        super(context);
        mContext = context;
    }

    public TabSpec customizeTabInfo(Activity activity, String tag, TabSpec tab, TabWidget tabWidget, String title) {
        SimInfoManager.SimInfoRecord simInfo = null;
        if (tag.equals(TAB_SIM_1)) {
            simInfo = SimInfoManager.getSimInfoBySlot(activity, 0);
        } else if (tag.equals(TAB_SIM_2)){
            simInfo = SimInfoManager.getSimInfoBySlot(activity, 1);
        } else if (tag.equals(TAB_MOBILE) && FeatureOption.MTK_DATAUSAGE_SUPPORT) {
            simInfo = SimInfoManager.getInsertedSimInfoList(activity).get(0);
        }
        if (simInfo != null) {
            int simColorResId = SimInfoManager.SimBackgroundLightRes[simInfo.mColor];
            
            LayoutInflater inflater =
                    (LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View tabIndicator = inflater.inflate(com.android.internal.R.layout.tab_indicator_holo,
                    tabWidget, // tab widget is the parent
                    false); // no inflate params
                    
            final TextView tv = (TextView) tabIndicator.findViewById(com.android.internal.R.id.title);
            tv.setText(title);
            tv.setBackgroundResource(simColorResId);
            tab.setIndicator(tabIndicator);
        }
        return tab;
    }
    
    public void customizeTextViewBackgroundResource(int simColor, TextView title) {
        title.setBackgroundResource(SimInfoManager.SimBackgroundLightRes[simColor]);
    }
}
