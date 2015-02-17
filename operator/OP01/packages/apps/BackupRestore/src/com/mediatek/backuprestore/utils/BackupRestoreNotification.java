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

package com.mediatek.backuprestore.utils;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import com.mediatek.backuprestore.R;

public class BackupRestoreNotification {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupRestoreNotification";
    public static final int NOTIFY_NEW_DETECTION = 1;
    public static final int NOTIFY_BACKUPING = 2;
    public static final int NOTIFY_RESTOREING = 3;

    public static final int FP_NEW_DETECTION_NOTIFY_TYPE_DEAFAULT = 0;
    public static final int FP_NEW_DETECTION_NOTIFY_TYPE_LIST = 1;
    public static final String FP_NEW_DETECTION_INTENT_LIST = "com.mediatek.backuprestore.intent.MainActivity";
    public static final String BACKUP_PERSONALDATA_INTENT = "com.mediatek.backuprestore.intent.PersonalDataBackupActivity";
    public static final String BACKUP_APPLICATION_INTENT = "com.mediatek.backuprestore.intent.AppBackupActivity";
    public static final String RESTORE_PERSONALDATA_INTENT = "com.mediatek.backuprestore.intent.PersonalDataRestoreActivity";
    public static final String RESTORE_APPLICATION_INTENT = "com.mediatek.backuprestore.intent.AppRestoreActivity";

    private Notification.Builder mNotification;
    private int mNotificationType;
    private Context mNotificationContext;
    private NotificationManager mNotificationManager;
    private int mMaxPercent = 100;
    private static BackupRestoreNotification sBackupRestoreNotification;

    /**
     * Constructor function.
     * 
     * @param context
     *            environment context
     */
    private BackupRestoreNotification(Context context) {
        mNotificationContext = context;
        mNotification = null;
        mNotificationManager = (NotificationManager) mNotificationContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
    }

    public static BackupRestoreNotification getInstance(Context context) {
        if (sBackupRestoreNotification == null) {
            sBackupRestoreNotification = new BackupRestoreNotification(context);
        }
        return sBackupRestoreNotification;
    }

    public void setMaxPercent(int maxPercent) {
        mMaxPercent = maxPercent;
        MyLogger.logD(CLASS_TAG, "BackupRestoreNotification : MaxPercent = "+mMaxPercent);
        
    }

    public void initBackupNotification(int type, int currentProgress) {
        if (mMaxPercent == 0) {
            return;
        }
        mNotificationType = NOTIFY_BACKUPING;
        CharSequence contentTitle = mNotificationContext.getText(R.string.notification_backup_title);
        String intentFilter = null;
        if (type == ModuleType.TYPE_APP) {
            intentFilter = BACKUP_APPLICATION_INTENT;
        } else {
            intentFilter = BACKUP_PERSONALDATA_INTENT;
        }
        setNotificationProgress(R.drawable.ic_backuprestore_notify, contentTitle, currentProgress, intentFilter);
    }

    public void initRestoreNotification(int type, int currentProgress) {
        if (mMaxPercent == 0) {
            return;
        }
        mNotificationType = NOTIFY_RESTOREING;
        CharSequence contentTitle = mNotificationContext.getText(R.string.notification_restore_title);
        String intentFilter = null;
        if (type == ModuleType.TYPE_APP) {
            intentFilter = RESTORE_APPLICATION_INTENT;
        } else {
            intentFilter = RESTORE_PERSONALDATA_INTENT;
        }
        setNotificationProgress(R.drawable.ic_backuprestore_notify, contentTitle, currentProgress, intentFilter);
    }

    private void setNotificationProgress(int iconDrawableId, CharSequence contentTitle, int currentProgress,
            String intentFilter) {
        if (mNotification == null) {
            mNotification = new Notification.Builder(mNotificationContext);
            if (mNotification == null) {
                return;
            }
        }
        mNotification.setAutoCancel(true).setOngoing(true).setContentTitle(contentTitle).setSmallIcon(iconDrawableId)
                .setWhen(System.currentTimeMillis()).setContentIntent(getPendingIntenActivity(intentFilter));
        mNotification.setProgress(mMaxPercent, currentProgress, false);
    }

    private PendingIntent getPendingIntenActivity(String intentFilter) {

        Intent notificationIntent = new Intent(intentFilter);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent contentIntent = PendingIntent.getActivity(mNotificationContext, 0,
                notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        return contentIntent;
    }

    public void clearNotification() {
        if (mNotification != null) {
            mNotificationManager.cancel(mNotificationType);
            MyLogger.logD(CLASS_TAG, "clearNotification mNotificationType = " +mNotificationType);
            mNotification = null;
        }	
        return;
    }
    
    public Notification getNotification() {
        if(mNotification != null) {
            return mNotification.getNotification();
        }
        return null;
    }
    
    public void updateNotification(int moduleType, int currentProgress) {
        if (mNotification != null) {
            mNotification.setProgress(mMaxPercent, currentProgress, false);
            MyLogger.logD(CLASS_TAG, "updateNotification and update currentProgress = " + currentProgress + ",moduleType = "+moduleType);
        }
    }

    /*public boolean updateAppNotification(int moduleType, int currentProgress) {
        int currentCount = 0;
        if(currentProgress - currentCount > 2){
            if (mNotification != null) {
                mNotification.setProgress(mMaxPercent, currentProgress, false);
                MyLogger.logD(CLASS_TAG, "updateNotification and update currentProgress = " + currentProgress);
            }
            
        }
        
        return false;
    }*/

    public void LastNotification(int moduleType) {
        // when composer onEnd update progressBar and the progressBarvalue is max
        if(mNotification != null) {
            mNotification.setProgress(mMaxPercent, mMaxPercent, false);
            MyLogger.logD(CLASS_TAG, "LastNotification and update progressbar maxValue");
        }
    }

}
