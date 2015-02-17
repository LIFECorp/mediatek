package com.hissage.util.statistics;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import org.json.JSONException;
import org.json.JSONObject;

import android.text.TextUtils;

import com.hissage.config.NmsCommonUtils;
import com.hissage.jni.engineadapter;
import com.hissage.jni.engineadapterforjni;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.pn.HpnsApplication;
import com.hissage.timer.NmsTimer;
import com.hissage.util.http.NmsHttpTask;
import com.hissage.util.log.NmsLog;

public class NmsRegInfoStatistics implements NmsHttpTask.HttpTaskListener {

    /* must correspond to the definition in nmsPlatform.h */
    public static final int NMS_REGS_START = 0;
    public static final int NMS_REGS_RECV_NOC_NUMBER = 1;
    public static final int NMS_REGS_SEND_NOC_SMS = 2;
    public static final int NMS_REGS_RECV_NOC_SMS_RESP = 3;
    public static final int NMS_REGS_SHOW_INPUT_DLG = 4;
    public static final int NMS_REGS_SEND_SELF_REG = 5;
    public static final int NMS_REGS_RECV_SELF_REG_RESP = 6;
    public static final int NMS_REGS_FAILED = 7;
    public static final int NMS_REGS_SUCCEED = 8;

    private static final String LOG_TAG = "NmsRegInfoStatistics";

    private static final String URL = "http://ismsi.hissage.com/webservice/client/registerProcess.php";
    private static final int RETRY_COUNT = 3;
    private static final int FAILED_RETRY_TIME = 15 * 60;

    private static NmsRegInfoStatistics mSingleton = null;

    private String[] mImsi = null;

    private String mImei = null;

    private String mVersion = null;
    private String mManufacturer = null;
    private String mModel = null;

    private boolean mIsStandalone = false;
    private int mMarketChannel = -1;

    private ArrayList<StatisticsInfo> mFailedInfo = null;

    private final class StatisticsInfo {
        public String imsi = null;
        public int type = -1;
        public String val = null;
        public String time = null;

        public StatisticsInfo(String aImsi, int aType, String aVal, String aTime) {
            imsi = aImsi;
            type = aType;
            val = aVal;
            time = aTime;
        }
    }

    private NmsRegInfoStatistics() {

        mImsi = new String[2];
        mImsi[0] = NmsPlatformAdapter.getInstance(HpnsApplication.mGlobalContext).getImsi(0);
        mImsi[1] = NmsPlatformAdapter.getInstance(HpnsApplication.mGlobalContext).getImsi(1);

        mImei = checkEmptyString(engineadapterforjni.getIMEI());
        mVersion = checkEmptyString(engineadapterforjni.getClientVersion());
        mManufacturer = checkEmptyString(android.os.Build.MANUFACTURER);
        mModel = checkEmptyString(android.os.Build.MODEL);

        mIsStandalone = (engineadapterforjni.nmsCheckIsWCP2DbExist() == 0);

        mMarketChannel = 0;

        mFailedInfo = new ArrayList<StatisticsInfo>();
    }

    private static String checkEmptyString(String str) {
        if (TextUtils.isEmpty(str))
            return "unknown";
        return str;
    }

    private static void doInit() {
        if (mSingleton != null)
            return;

        synchronized (NmsRegInfoStatistics.class) {
            if (mSingleton != null)
                return;

            mSingleton = new NmsRegInfoStatistics();
        }
    }

    private void postToServer(String imsi, int type, String val, String time) throws JSONException {

        JSONObject data = new JSONObject();
        data.put("imsi", imsi);
        data.put("actType", type);
        data.put("imei", mImei);
        data.put("manufacturer", mManufacturer);
        data.put("model", mModel);
        data.put("version", mVersion);
        data.put("IsStandalone", mIsStandalone ? 1 : 0);
        data.put("marketChannel", mMarketChannel);
        data.put("channel", engineadapter.get().nmsGetChannelId());
        data.put("devId", engineadapter.get().nmsGetDevicelId());
        data.put("language", NmsCommonUtils.getLanguageCode(Locale.getDefault()));

        data.put("client_time", time);

        if (type == NMS_REGS_RECV_NOC_SMS_RESP) {
            if (TextUtils.isEmpty(val)) {
                NmsLog.error(LOG_TAG, "invalid val of NMS_REGS_RECV_NOC_SMS_RESP");
                data.put("nocReplyContent", "invalid");
            } else {
                data.put("nocReplyContent", val);
            }
        } else if (type == NMS_REGS_SEND_SELF_REG) {
            if (TextUtils.isEmpty(val)) {
                NmsLog.error(LOG_TAG, "invalid val of NMS_REGS_SEND_SELF_REG");
                data.put("inputNumber", "invalid");
            } else {
                data.put("inputNumber", val);
            }
        }

        NmsHttpTask task = new NmsHttpTask(this, RETRY_COUNT, URL, data.toString(), null);
        task.setClientParam(new StatisticsInfo(imsi, type, val, time));
        task.startTask(false);
    }

    private String getImsiString(int simId) {
        if (simId == -1) {
            if (TextUtils.isEmpty(mImsi[0]))
                return mImsi[1];

            if (TextUtils.isEmpty(mImsi[1]))
                return mImsi[0];

            return mImsi[0] + "|" + mImsi[1];
        }

        return mImsi[NmsPlatformAdapter.getInstance(HpnsApplication.mGlobalContext)
                .getSlotIdBySimId(simId)];
    }

    private synchronized void doSendNewStatistics(int simId, int type, String val, String time)
            throws JSONException {
        for (StatisticsInfo info : mFailedInfo) {
            postToServer(info.imsi, info.type, info.val, info.time);
        }

        String imsi = getImsiString(simId);
        NmsLog.trace(LOG_TAG, String.format(
                "send regInfo time: %s, imsi: %s, type: %d, val: %s to server", time, imsi, type,
                val));
        postToServer(imsi, type, val, time);
    }

    private synchronized void doHandleTimerEvent() throws JSONException {
        for (StatisticsInfo info : mFailedInfo) {
            postToServer(info.imsi, info.type, info.val, info.time);
        }
    }

    @Override
    public void onProgress(NmsHttpTask httpTask, int downloadByte) {
    }

    @Override
    public void onResult(NmsHttpTask httpTask) {
        StatisticsInfo obj = (StatisticsInfo) httpTask.getClientParam();

        if (!httpTask.isTaskSucceed()) {
            synchronized (this) {

                mFailedInfo.add(obj);

                NmsTimer.NmsKillTimer(NmsTimer.NMS_TIMERID_REG_STATISTICS);
                NmsTimer.NmsSetTimer(NmsTimer.NMS_TIMERID_REG_STATISTICS, FAILED_RETRY_TIME);

                NmsLog.trace(LOG_TAG,
                        String.format("send imsi: %s, type: %d failed", obj.imsi, obj.type));
            }
        } else {
            NmsLog.trace(LOG_TAG,
                    String.format("send imsi: %s, type: %d succeed", obj.imsi, obj.type));
        }
    }

    public static void sendNewStatistics(int simId, int type, String val) {

        try {
            doInit();
            String time = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS").format(new Date());
            mSingleton.doSendNewStatistics(simId, type, val, time);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
        }
    }

    public static void handleTimerEvent() {
        try {
            mSingleton.doHandleTimerEvent();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
        }
    }
}
