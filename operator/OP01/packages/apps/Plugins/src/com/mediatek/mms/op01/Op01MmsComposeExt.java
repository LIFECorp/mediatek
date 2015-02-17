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
import android.text.InputFilter;
import android.text.Spanned;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.mms.ext.IMmsCompose;
import com.mediatek.mms.ext.MmsComposeImpl;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

/// M: New plugin API @{
import java.util.ArrayList;
import android.provider.Browser;
import android.content.DialogInterface;
import android.view.MenuItem;
import android.view.ContextMenu;
import android.app.AlertDialog;
/// @}

public class Op01MmsComposeExt extends MmsComposeImpl {
    private static final String TAG = "Mms/Op01MmsComposeExt";

    // TODO: Base id should be defined by Host
    public static final int MENU_SELECT_TEXT     = 101;     
    
    private Toast mExceedSubjectSizeToast                       =   null;
    private static final int DEFAULT_SUBJECT_LENGTH             =   40;

    public Op01MmsComposeExt(Context context) {
        super(context);
        Xlog.d(TAG,"context is " + context.getPackageName());
    }

    /**
     * This filter will constrain edits not to make the length of the text
     * greater than the specified length ( eg. 40 Bytes).
     */  
    class MyLengthFilter implements InputFilter {
        private final int mMax;
        public MyLengthFilter(Context context, int max) {
            Xlog.d(TAG,"MyLengthFilter:context is " + context.getPackageName());
            mMax = max;
            mExceedSubjectSizeToast = Toast.makeText(context, Op01MmsComposeExt.this.getString(R.string.exceed_subject_length_limitation),
                    Toast.LENGTH_SHORT);
        }

        private CharSequence getMaxByteSequence(CharSequence str, int keep) {
            String source = str.toString();
            int byteSize = source.getBytes().length;
            if (byteSize <= keep) {
                return str;
            } else {
                int charSize = source.length();
                while (charSize > 0) {
                    source = source.substring(0, source.length() - 1);
                    charSize--;
                    if (source.getBytes().length <= keep) {
                        break;
                    }
                }
                return source;
            }
        }
        
        //this is just the method code in LengthFilter, just add a Toast to show max length exceed.
        public CharSequence filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {

            int destOldLength = dest.toString().getBytes().length;
            int destReplaceLength = dest.subSequence(dstart, dend).toString().getBytes().length;
            CharSequence sourceSubString = source.subSequence(start, end); 
            int sourceReplaceLength = sourceSubString.toString().getBytes().length;
            int newLength =  destOldLength - destReplaceLength + sourceReplaceLength;
            if (newLength > mMax) {
                // need cut the new input charactors
                mExceedSubjectSizeToast.show();
                int keep = mMax - (destOldLength - destReplaceLength);
                if (keep <= 0) {
                    return ""; 
                } else {
                    return getMaxByteSequence(sourceSubString, keep);
                }
            } else {
                return null; // can replace
            }
        }
    }

/* this is comment feature now
    private final class ContextMenuClickListener implements MenuItem.OnMenuItemClickListener {
        public boolean onMenuItemClick(MenuItem item){
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

    public void addContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo, CharSequence text){
        if (!TextUtils.isEmpty(text)) {
            Xlog.i(TAG, "add select text menu");
            menu.add(0, MENU_SELECT_TEXT, 0, getString(R.string.select_text))
                    .setOnMenuItemClickListener(new ContextMenuClickListener());
        }
    }
*/
    public int getCapturePicMode() {
        return IMmsCompose.CAPTURE_PIC_AT_TEMP_FILE;
    }

    public boolean isAppendSender() {
        return true;
    }

    public void configSubjectEditor(EditText subjectEditor) {
        Context context = subjectEditor.getContext();
        subjectEditor.setFilters(new InputFilter[] { new MyLengthFilter(context, DEFAULT_SUBJECT_LENGTH) });
    }
    
    public boolean isAddMmsUrlToBookMark() {
        return true;
    }

    /// M: New plugin API @{
    int mMenuId = 0;
    ArrayList<String> mUrls = null;
    Context mContext;
    int mTitle;
    int mIcon;
    MenuClickListener l = new MenuClickListener();
    
    public void addMmsUrlToBookMark(Context context, ContextMenu menu, int menuId, ArrayList<String> urls, int titleString, int icon) {
        Xlog.d(TAG, "addMmsUrlToBookMark");
        menu.add(0, menuId, 0, titleString).setOnMenuItemClickListener(l);
        mMenuId = menuId;
        mUrls = urls;
        mTitle = titleString;
        mIcon = icon;
        mContext = context;
        return;
    }

    private final class MenuClickListener implements MenuItem.OnMenuItemClickListener {

        public MenuClickListener() {
        }

        @Override
        public boolean onMenuItemClick(MenuItem item) {
            if (item.getItemId() != mMenuId) {
                return false;
            }

            if (mUrls.size() == 1) {
                Browser.saveBookmark(mContext, null, mUrls.get(0));
            } else if (mUrls.size() > 1) {
                CharSequence[] items = new CharSequence[mUrls.size()];
                for (int i = 0; i < mUrls.size(); i++) {
                    items[i] = mUrls.get(i);
                }
                new AlertDialog.Builder(mContext)
                    .setTitle(mTitle)
                    .setIcon(mIcon)
                    .setItems(items, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Browser.saveBookmark(mContext, null, mUrls.get(which));
                            }
                        })
                    .show();
            }
            return true;
        }
    }
    /// @}
}

