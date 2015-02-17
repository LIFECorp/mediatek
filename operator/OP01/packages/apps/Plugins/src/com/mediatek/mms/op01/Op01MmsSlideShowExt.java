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
import android.widget.TextView;
import android.widget.VideoView;

import com.mediatek.mms.ext.IMmsSlideShow;
import com.mediatek.mms.ext.MmsSlideShowImpl;

//import com.mediatek.xlog.Xlog;
//import com.mediatek.xlog.SXlog;
//import android.view.Menu;
//import android.view.MenuInflater;
//import android.view.MenuItem;
//import android.content.ContextWrapper;
//import com.mediatek.op01.plugin.R;
//import android.provider.Telephony.Mms;
//import android.provider.Telephony.SIMInfo;
//import android.provider.Telephony.Sms;
//import java.util.List;
//import com.mediatek.mms.ext.IMmsSlideShowHost;
//import android.widget.ScrollView;
//import android.view.View;
//import android.view.ContextMenu.ContextMenuInfo;
//import android.view.ContextMenu;


public class Op01MmsSlideShowExt extends MmsSlideShowImpl {
    private static final String TAG = "Mms/Op01MmsSlideShowExt";

    // TODO: Base id should be defined by Host
    public static final int MENU_SELECT_TEXT     = 101;

    public Op01MmsSlideShowExt(Context context) {
        super(context);
    }

/* this is common feature now
    private final class ContextMenuClickListener implements MenuItem.OnMenuItemClickListener {
        public boolean onMenuItemClick(MenuItem item) {
            switch(item.getItemId()) {
                case MENU_SELECT_TEXT:
                    getHost().showSelectText(item);
                    break;
        
                default:
                    return false;
            }
        
            return true;
        
        }
    }

    public void addContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo, CharSequence text) {
        menu.add(0, MENU_SELECT_TEXT, 0, getString(R.string.select_text))
            .setOnMenuItemClickListener(new ContextMenuClickListener());
        return;
    }
*/
    public int getInitialPlayState() {
        return IMmsSlideShow.PLAY_AS_START;
    }
    
    public void configVideoView(VideoView view) {
        // seek to first ms to got the thumbnail.
        view.seekTo(1);
        return;
    }

    public void configTextView(Context activityContext, TextView view) {
        float size =  Op01MmsUtils.getTextSize(activityContext);
        view.setTextSize(size);
    }

}

