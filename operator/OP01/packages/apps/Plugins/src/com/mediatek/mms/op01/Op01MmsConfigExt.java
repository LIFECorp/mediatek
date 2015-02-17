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


import android.content.Intent;
import android.net.Uri;
import android.provider.MediaStore;

import com.mediatek.mms.ext.MmsConfigImpl;
import com.mediatek.xlog.Xlog;

import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.Context;
/// M: ALPS00527989, Extend TextView URL handling @ {
import android.text.Spannable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.URLSpan;
import android.widget.TextView;
/// @}
/// M: New plugin API @{
import com.mediatek.op01.plugin.R;
import android.os.Handler;
import android.widget.Toast;
import android.os.Message;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.provider.Browser;
import android.net.Uri;
/// @}
/// M: ALPS00956607, not show modify button on recipients editor @{
import android.view.inputmethod.EditorInfo;
/// @}

import java.util.List;

public class Op01MmsConfigExt extends MmsConfigImpl {
    private static final String TAG = "Mms/Op01MmsConfigExt";
    
    private static final int SMS_TO_MMS_THRESHOLD = 11;
    private static final int MAX_CMCC_TEXT_LENGTH = 3100;
    private static final int RECIPIENTS_LIMIT = 50;
    private static final String UAPROFURL_OP01 = "http://218.249.47.94/Xianghe/MTK_Athens15_UAProfile.xml";
    private static final int SOCKET_TIMEOUT = 90 * 1000;
    private static final int SOCKET_SEND_TIMEOUT = 30 * 1000;
    private static final int[] OP01DEFAULTRETRYSCHEME = {
        0, 5 * 1000, 15 * 1000, 30 * 1000, 5 * 60 * 1000, 10 * 60 * 1000, 30 * 60 * 1000};
        
    public int getSmsToMmsTextThreshold() {
        return SMS_TO_MMS_THRESHOLD;
    }

    public void setSmsToMmsTextThreshold(int value){
        // TODO: currently, for operator, this value is not customized by configuration file. why?
        //SMS_TO_MMS_THRESHOLD = value;
    }

    public int getMaxTextLimit() {
        return MAX_CMCC_TEXT_LENGTH;
    }

    public void setMaxTextLimit(int value) {
        //do nothing
    }
    
    public int getMmsRecipientLimit() {
        return RECIPIENTS_LIMIT;
    }

    public void setMmsRecipientLimit(int value) {
        //do nothing
    }

    public String getUAProf(final String defaultUAP) {
        Xlog.d(TAG, "UaProfUrl = " + UAPROFURL_OP01);
        return UAPROFURL_OP01;
    }

    public int getHttpSocketTimeout() {
        Xlog.d(TAG, "get socket timeout: " + SOCKET_TIMEOUT);
        return SOCKET_TIMEOUT;
    }

    public void setHttpSocketTimeout(int socketTimeout) {
        Xlog.d(TAG, "set socket timeout: " + socketTimeout);
    }
    
    public boolean isEnableReportAllowed() {
        Xlog.d(TAG, "Enable ReportAllowed");
        return true;
    }

    public boolean isEnableMultiSmsSaveLocation() {
        Xlog.d(TAG, "Enable MultiSmsSaveLocation ");
        return true;
    }

    public boolean isEnableStorageFullToast() {
        Xlog.d(TAG, "Enable StorageFullToast");
        return true;
    }

    public boolean isEnableFolderMode() {
        Xlog.d(TAG, "Enable FolderMode");
        return true;
    }

    public boolean isEnableForwardWithSender() {
        Xlog.d(TAG, "Enable ForwardWithSender ");
        return true;
    }

    public boolean isEnableSIMLongSmsConcatenate() {
        Xlog.d(TAG, "Enable concatenate long sms in sim card status");
        return false;
    }

    public boolean isEnableDialogForUrl() {
        Xlog.d(TAG, "Enable ForwardWithSender ");
        return true;
    }

    public boolean isEnableStorageStatusDisp() {
        Xlog.d(TAG, "Enable display storage status ");
        return true;
    }
    
    public boolean isEnableAdjustFontSize() {
        Xlog.d(TAG, "Enable adjust font size");
        return true;
    }

    public boolean isEnableSmsValidityPeriod() {
        Xlog.d(TAG, "Enable sms validity period");
        return true;
    }
/*
    public Intent getCapturePictureIntent() {
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra("OP01", true);
        return intent;
    }
*/
    public boolean isShowUrlDialog() {
        Xlog.d(TAG, "Enable show dialog when open browser: " + true);
        return true;
    }

    public boolean isNotificationDialogEnabled() {
        Xlog.d(TAG, "isNotificationDialogEnabled");
        return true;
    }

    public int getMmsRetryPromptIndex() {
        Xlog.d(TAG, "getMmsRetryPromptIndex");
        return 4;
    }

    public int[] getMmsRetryScheme() {
        Xlog.d(TAG, "getMmsRetryScheme");
        return OP01DEFAULTRETRYSCHEME;
    }

    public void setSoSndTimeout(HttpParams params) {
        Xlog.d(TAG, "setSoSndTimeout");
        HttpConnectionParams.setSoSndTimeout(params, SOCKET_SEND_TIMEOUT);
        return;
    }

    public boolean isAllowRetryForPermanentFail() {
        Xlog.d(TAG, "setSoSndTimeout");
        return true;
    }

    public boolean isRetainRetryIndexWhenInCall() {
        Xlog.d(TAG, "isRetainRetryIndexWhenInCall: " + true);
        return true;
    }

    public boolean isMobileDataEnabled() {
        Xlog.d(TAG, "isMobileDataEnabled: " + true);
        return true;
    }

    public boolean isShowDraftIcon() {
        Xlog.d(TAG, "isShowDraftIcon: " + true);
        return true;
    }

    public boolean isSendExpiredResIfNotificationExpired() {
        Xlog.d(TAG, "isSendExpiredResIfNotificationExpired: " + false);
        return false;
    }

    public boolean isNeedExitComposerAfterForward() {
        Xlog.d(TAG, "isNeedExitComposerAfterForward: " + false);
        return false;
    }

    public Uri appendExtraQueryParameterForConversationDeleteAll(Uri uri) {
        Xlog.d(TAG, "appendExtraQueryParameterForConversationDeleteAll; groupDeleteParts: yes");
        return uri.buildUpon().appendQueryParameter("groupDeleteParts", "yes").build();
    }

    public void printMmsMemStat(Context context, String callerTag) {
        Xlog.d(TAG, callerTag + " call printMmsMemStat");
        String mmsProcName = "com.android.mms";
        int pid = 0;
        
        /* Find Mms process*/
        //Context context = getApplicationContext();
        ActivityManager activityManager = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);
        List<RunningAppProcessInfo> runningAppProcesses = activityManager.getRunningAppProcesses(); 
        //Map<Integer, String> pidMap = new TreeMap<Integer, String>(); 
        for (RunningAppProcessInfo runningAppProcessInfo : runningAppProcesses) { 
	        //pidMap.put(runningAppProcessInfo.pid, runningAppProcessInfo.processName); 
	        if (mmsProcName.equals(runningAppProcessInfo.processName)) {
                pid = runningAppProcessInfo.pid;
                printMmsMemStat(context, pid);
                break;
	        }
        }
    }

    private void printMmsMemStat(Context context, int pid) {
        Xlog.d(TAG, "printMmsMemStat " + pid);

        ActivityManager activityManager = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);
        int pids[] = new int[1];
	    pids[0] = pid;
        
        android.os.Debug.MemoryInfo[] memoryInfoArray = activityManager.getProcessMemoryInfo(pids);
        if (memoryInfoArray[0] == null) {
            Xlog.d(TAG, "getProcessMemoryInfo failed!");
            return;
        }
        Xlog.d(TAG, "Mms Mem: [PrivateDirty = " + memoryInfoArray[0].getTotalPrivateDirty() + "]");
    }

    /// M: ALPS00527989, Extend TextView URL handling @ {
    public void setExtendUrlSpan(TextView textView) {
        Xlog.d(TAG, "setExtendUrlSpan");

        CharSequence text = textView.getText();

        Spanned spanned = ((Spanned) text);
        URLSpan[] urlSpanArray = textView.getUrls();
        for(int i = 0; i < urlSpanArray.length; i++) {
            String url = urlSpanArray[i].getURL();
            Xlog.d(TAG, "find url:" + url);
            if (isWebUrl(url)) {
                URLSpan newurlSpan = new ExtendURLSpan(url);
                int spanStart = spanned.getSpanStart(urlSpanArray[i]);
                int spanEnd = spanned.getSpanEnd(urlSpanArray[i]);
                Spannable sp = (SpannableString)(text);
                ((SpannableString)(text)).removeSpan(urlSpanArray[i]);
                ((SpannableString)(text)).setSpan(newurlSpan, spanStart, spanEnd, Spanned.SPAN_INCLUSIVE_INCLUSIVE);
            }
        }
    }

    private boolean isWebUrl(String UrlString) {
        boolean isWebURL = false;
        Uri uri = Uri.parse(UrlString);

        String scheme = uri.getScheme();
        if (scheme != null) {
            isWebURL = scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https")
                    || scheme.equalsIgnoreCase("rtsp");
        }
        return isWebURL;
    }
    /// @}

    ///M: for cmcc, support reply message with the sim card received
    public boolean isSupportAutoSelectSimId() {
        return true;
    }

    ///M: for cmcc, async update wallpaper
    public boolean isSupportAsyncUpdateWallpaper() {
        return true;
    }

    /// M: ALPS00837193, query undelivered mms with non-permanent fail ones or not @{
    public Uri getUndeliveryMmsQueryUri(Uri defaultUri) {
        Xlog.d(TAG, "appendExtraQueryParameterForUndeliveryMms");
        Uri uri = Uri.parse("content://mms-sms/undelivered");
        return uri.buildUpon().appendQueryParameter("includeNonPermanent", "false").build();
    }
    /// @}

    /// M: New plugin API @{
    public void openUrl(Context context, String url) {
        Xlog.d(TAG, "openUrl, url=" + url);
        final String strUrl = url;
        final Context theContext = context;
        if (!url.startsWith("mailto:")) {
            AlertDialog.Builder b = new AlertDialog.Builder(theContext);
            b.setTitle(com.mediatek.internal.R.string.url_dialog_choice_title);
            b.setMessage(com.mediatek.internal.R.string.url_dialog_choice_message);
            b.setCancelable(true);
            b.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                public final void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            });
            b.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Uri theUri = Uri.parse(strUrl);
                    Intent intent = new Intent(Intent.ACTION_VIEW, theUri);
                    intent.putExtra(Browser.EXTRA_APPLICATION_ID, theContext.getPackageName());
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    theContext.startActivity(intent);
                }
            });
            b.show();
        } else {
            super.openUrl(theContext, url);
        }
    }
    /// @}

    /// M: ALPS00956607, not show modify button on recipients editor @{
    public void setRecipientsEditorOutAtts(EditorInfo outAttrs) {
        Xlog.d(TAG, "setRecipientsEditorOutAtts");
        outAttrs.imeOptions |= EditorInfo.IME_FLAG_NO_EXTRACT_UI;
    }
    /// @}

    ///M: add for fix alps01317511.
    public boolean isSupportMessagingNotificationProxy() {
        return true;
    }
}

