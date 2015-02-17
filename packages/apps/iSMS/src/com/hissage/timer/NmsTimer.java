package com.hissage.timer;

//import packages.
import java.text.SimpleDateFormat;
import java.util.Date;

import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.SystemClock;
import android.util.Log;
import android.widget.RemoteViews;

import com.hissage.R;
import com.hissage.db.NmsDBOpenHelper;
import com.hissage.db.NmsDBUtils;
import com.hissage.hpe.SDK;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.pushinfo.NmsPushInfo;
import com.hissage.service.NmsService;
import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.data.NmsConverter;
import com.hissage.util.log.NmsLog;
import com.hissage.util.statistics.NmsRegInfoStatistics;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;
import com.hissage.vcard.NmsVcardUtils;

public final class NmsTimer {
    private static final String TAG = "NmsTimer";

    public static class NmsTimerObj {
        public Intent intent = null;
        public PendingIntent sender = null;
        public boolean flag = false;
        public int index = 0;
    }

    public static final int NMS_TIMERID_REGISTRATION = 0;
    public static final int NMS_TIMERID_CONNECTION = 1;
    public static final int NMS_TIMERID_TRANSACTION = 2;
    public static final int NMS_TIMERID_HEART_BEAT = 3;
    public static final int NMS_TIMERID_CFG_SET = 4;
    public static final int NMS_TIMERID_FETCH = 5;
    public static final int NMS_TIMERID_VCARD = 6;
    public static final int NMS_TIMERID_TCP_1 = 7;
    public static final int NMS_TIMERID_TCP_2 = 8;
    public static final int NMS_TIMERID_TCP_3 = 9;
    public static final int NMS_TIMERID_IM_REFRESH_STATUS = 10;
    public static final int NMS_TIMERID_CONTACT_RESET_1 = 11;
    public static final int NMS_TIMERID_CONTACT_RESET_2 = 12;
    public static final int NMS_TIMERID_CONTACT_RESET_3 = 13;
    public static final int NMS_TIMERID_CONTACT_RESET_4 = 14;
    public static final int NMS_TIMERID_CONTACT_RESET_5 = 15;
    public static final int NMS_TIMERID_CONTACT_RESET_6 = 16;
    public static final int NMS_TIMERID_CONTACT_RESET_7 = 17;
    public static final int NMS_TIMERID_CONTACT_RESET_8 = 18;
    public static final int NMS_TIMERID_CONTACT_RESET_9 = 19;
    public static final int NMS_TIMERID_CONTACT_RESET_10 = 20;
    public static final int NMS_TIMERID_REG_NOC = 21;
    public static final int NMS_TIMERID_REG_SELF = 22;
    public static final int NMS_TIMERID_MAX = 23;

    public static final int NMS_TIMERID_CONTACT = 23;
    public static final int NMS_TIMERID_WAKEUP_ACTIVATE = 24;
    public static final int NMS_TIMERID_PN_REGISTER = 25;
    // M: Activation Statistics
    public static final int NMS_TIMERID_STATISTICS = 26;
    public static final int NMS_PRIVATE_UPGRADE = 27;
    public static final int NMS_TIMERID_REG_STATISTICS = 28;
    // public static final int NMS_TIMERID_ALL_MAX = 29;
    public static final int NMS_TIMERID_ALL_MAX = 1000;
    public static final int NMS_TIMERID_PUSH_INFO = 100;
    // M: Activation Statistics
    public static final String nmsTimerName[] = { "REGISTRATION", "CONNECTION", "TRANSACTION",
            "HEART_BEAT", "CFG_SET", "FETCH", "VCARD", "TCP_0", "TCP_1", "TCP_2", "RESET_1",
            "RESET_2", "RESET_3", "RESET_4", "RESET_5", "RESET_6", "RESET_7", "RESET_8", "RESET_9",
            "RESET_10", "NMS_TIMERID_REG_NOC", "NMS_TIMERID_REG_SELF", "NMS_TIMERID_ENGINE_MAX",
            "NMS_TIMERID_CONTACT", "NMS_TIMERID_UPGRADE", "NMS_TIMERID_PN_REGISTER",
            "NMS_TIMERID_STATISTICS", "NMS_PRIVATE_UPGRADE", "NMS_TIMERID_REG_STATISTICS",
            "NMS_TIMERID_ALL_MAX" };

    public static class AlarmRecevier extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String category = intent.getAction();
            int timerId = NmsConverter.string2Int(category);
            if (timerId >= NMS_TIMERID_REGISTRATION && timerId < NMS_TIMERID_MAX) {
                NmsWakeLock.NmsSetWakeupLock(NmsService.getInstance(), nmsTimerName[timerId]);
                engineadapter.get().nmsSendTimerMsgToEngine(
                        timerId | (((int) nmsTimerObj[timerId].index << 24)));
                nmsTimerObj[timerId].flag = false;
                NmsWakeLock.NmsReleaseWakeupLock(nmsTimerName[timerId]);
            } else if (timerId >= NMS_TIMERID_PUSH_INFO) { // push info message

                NmsPushInfo info = NmsDBUtils.getDataBaseInstance(NmsService.getInstance())
                        .getNmsSinglePushInfo(NmsDBOpenHelper.PUSH_INFO_MESSAGE,
                                String.valueOf(timerId - NMS_TIMERID_PUSH_INFO));
                if (info == null) {
                    return;
                }
                if ((engineadapter.get().nmsUIIsActivated() && info.pushMsgType == 0)
                        || info.pushInfoStatus == NmsPushInfo.PUSH_INFO_STATUS_ERROR) {
                    NmsLog.trace(NmsUpgradeManager.TAGPUSH,
                            " AlarmRecevier. onReceive adtivated pushInfoID:" + info.pushInfoID);
                    return;
                } else {
                    boolean isSend = sendNotification(info);
                    NmsLog.trace(NmsUpgradeManager.TAGPUSH,
                            " AlarmRecevier onReceive Notification  timerId." + timerId
                                    + "  isSend" + isSend);
                }

                ContentValues values = new ContentValues();
                SimpleDateFormat dateformat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                values.put(NmsDBOpenHelper.PUSH_INFO_OP_SHOWTIME,
                        dateformat.format(System.currentTimeMillis()).toString());
                
                values.put(NmsDBOpenHelper.PUSH_INFO_OPRATERTIME,
                        dateformat.format(System.currentTimeMillis()).toString());
                
                values.put(NmsDBOpenHelper.PUSH_INFO_STATUS, NmsPushInfo.PUSH_INFO_STATUS_SHOW);
                NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).updatePushInfoToDB(values,
                        info.pushInfoID);

            } else {
                if (timerId == NMS_TIMERID_CONTACT) {
                    NmsVcardUtils.notifyEngineContactChanged();
                } else if (timerId == NMS_TIMERID_WAKEUP_ACTIVATE) {
                    NmsLog.trace(NmsUpgradeManager.TAGPUSH,
                            " AlarmRecevier  onReceive  NMS_TIMERID_WAKEUP_ACTIVATE");
                    NmsUpgradeManager.handleTimerEvent();
                } else if (timerId == NMS_TIMERID_PN_REGISTER) {
                    SDK.onRegister(context);
                    // M: Activation Statistics
                } else if (timerId == NMS_TIMERID_STATISTICS) {
                    NmsStatistics.handleTimerEvent();
                } else if (timerId == NMS_TIMERID_REG_STATISTICS) {
                    NmsRegInfoStatistics.handleTimerEvent();
                }
                nmsTimerObj[timerId].flag = false;
            }
        }
    }

    private static boolean sendNotification(NmsPushInfo info) {
        try {
            NotificationManager nm = (NotificationManager) NmsService.getInstance()
                    .getSystemService(Context.NOTIFICATION_SERVICE);

            Notification notification = new Notification(R.drawable.notification_icon_thumb, " ",
                    System.currentTimeMillis());
            notification.flags = Notification.FLAG_AUTO_CANCEL;
            notification.defaults = Notification.DEFAULT_LIGHTS;

            Intent intent = new Intent();
            intent.setAction("com.hissage.pushinfo.NmsPushInfoBroadcast");
            intent.putExtra("pushInfoId", info.pushInfoID);
            NmsLog.trace(NmsUpgradeManager.TAGPUSH, " sendNotification info.pushInfoUrl."
                    + info.pushInfoUrl + ". info.pushMsgType." + info.pushMsgType + "  pushInfoID:"
                    + info.pushInfoID);

            if (info.pushMsgType == 0) {
                intent.putExtra("pushMsgType", 0);
            } else if ((info.pushMsgType == 1)) {
                intent.putExtra("pushMsgType", 1);
                if (info.pushInfoUrl != null) {
                    intent.putExtra("url", info.pushInfoUrl);
                } else {
                    return false;
                }
            } else {
                return false;
            }

            RemoteViews remoteView = new RemoteViews(NmsService.getInstance().getPackageName(),
                    R.layout.pushinfo_notification);
            Bitmap bitmap = BitmapFactory.decodeFile(NmsUpgradeManager.PUSH_INFO_PIC_PATH
                    + info.pushInfoID);
            if (bitmap == null) {
                remoteView.setImageViewResource(R.id.image, R.drawable.notification_icon);
            } else {
                remoteView.setImageViewBitmap(R.id.image, bitmap);
            }

            remoteView.setTextViewText(R.id.title, info.pushTitle);
            remoteView.setTextViewText(R.id.text, info.pushInfoContent);

            SimpleDateFormat dateformat = new SimpleDateFormat("MM-dd HH:mm");
            remoteView.setTextViewText(R.id.time,
                    dateformat.format(new Date(System.currentTimeMillis())).toString());
            notification.contentView = remoteView;
            PendingIntent contentIntent = PendingIntent.getBroadcast(NmsService.getInstance(),
                    info._id, intent, PendingIntent.FLAG_UPDATE_CURRENT);

            notification.contentIntent = contentIntent;
            notification.number++;
            nm.notify(info._id, notification);
            return true;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public static final int NMS_TIMER_RESULT_ERROR = -2;
    public static final int NMS_TIMER_RESULT_EXIST = -1;
    public static final int NMS_TIMER_RESULT_OK = 0;

    public static AlarmRecevier mReceiver = new AlarmRecevier();
    public static AlarmManager am;
    public static NmsTimerObj[] nmsTimerObj = null;

    public static NmsTimerObj getNmsTimerObj(int id) {
        if (nmsTimerObj == null || nmsTimerObj.length == 0) {
            nmsTimerObj = new NmsTimerObj[NMS_TIMERID_ALL_MAX];
            for (int i = 0; i < NMS_TIMERID_ALL_MAX; ++i) {
                nmsTimerObj[i] = new NmsTimerObj();
                nmsTimerObj[i].flag = false;
            }
        }
        return nmsTimerObj[id];
    }

    public static int NmsCreateTimer(int timerId, long seconds) {

        if ((timerId >= NMS_TIMERID_ALL_MAX || timerId < 0)) {
            NmsLog.trace(TAG, "Exception: create timer id outof TIMERID_MAX, id is " + timerId);
            return NMS_TIMER_RESULT_ERROR;
        }
        NmsTimerObj timer = getNmsTimerObj(timerId);
        if (timer.flag) {
            return NMS_TIMER_RESULT_EXIST;
        }
        timer.flag = true;
        timer.index++;
        String timerName = NmsConverter.int2String(timerId);
        NmsService.getInstance().registerReceiver(mReceiver, new IntentFilter(timerName));
        timer.intent = new Intent(timerName);
        timer.sender = PendingIntent.getBroadcast(NmsService.getInstance(), 0, timer.intent, 0);

        am = (AlarmManager) NmsService.getInstance().getSystemService(Context.ALARM_SERVICE);

        long firstTime = SystemClock.elapsedRealtime();

        if (timerId == NMS_TIMERID_WAKEUP_ACTIVATE) {
            firstTime += (seconds + 60) * 1000;
        } else {
            firstTime += seconds * 1000;
        }
        am.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, firstTime, timer.sender);

        return (timerId | (((int) timer.index << 24)));
    }

    public static int NmsRemoveTimer(int timerId) {
        if (timerId >= NMS_TIMERID_ALL_MAX || timerId < 0) {
            NmsLog.trace(TAG, "Exception: kill timer id outof TIMERID_MAX, id is " + timerId);
            return NMS_TIMER_RESULT_ERROR;
        }

        if (true != getNmsTimerObj(timerId).flag)
            return NMS_TIMER_RESULT_EXIST;
        am.cancel(getNmsTimerObj(timerId).sender);
        getNmsTimerObj(timerId).flag = false;

        return NMS_TIMER_RESULT_OK;
    }

    public static int NmsSetTimer(int id, long seconds) {
        try {
            if (seconds > 0) {
                return NmsCreateTimer(id, (int) seconds);
            } else {
                // This timer can't support 0 seconds, if set 0 then throw the
                // exception
                return NmsCreateTimer(id, 0);
            }
        } catch (Exception e) {
            NmsLog.error(TAG, "create timer id:" + id + "\texception:" + NmsLog.nmsGetStactTrace(e));
            return NMS_TIMER_RESULT_ERROR;
        }
    }

    public static int NmsKillTimer(int id) {
        if (id < NMS_TIMERID_ALL_MAX && id >= 0) {
            return NmsRemoveTimer(id);
        } else {
            NmsLog.trace(TAG, "Timer ID error, ID is :" + id);
            return NMS_TIMER_RESULT_ERROR;
        }
    }

    // retrieve current system time in seconds.
    public static int NmsGetSystemTime() {
        long millSeconds = System.currentTimeMillis();
        return (int) (millSeconds / 1000);
    }
}
