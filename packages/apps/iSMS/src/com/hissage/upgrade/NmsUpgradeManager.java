package com.hissage.upgrade;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.URL;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.Enumeration;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.util.InetAddressUtils;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.CoreConnectionPNames;
import org.apache.http.protocol.HTTP;
import org.apache.http.util.EntityUtils;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONTokener;

import android.app.ActivityManager;
import android.content.ContentValues;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.telephony.CellLocation;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;
import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.db.NmsDBOpenHelper;
import com.hissage.db.NmsDBUtils;
import com.hissage.jni.engineadapter;
import com.hissage.jni.engineadapterforjni;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsFeatureSupport;
import com.hissage.pushinfo.NmsPushInfo;
import com.hissage.service.NmsService;
import com.hissage.timer.NmsTimer;
import com.hissage.util.log.NmsLog;
import com.hissage.util.preference.NmsPreferences;
import com.hissage.util.statistics.NmsPrivateStatistics;
import com.hissage.util.statistics.NmsStatistics;

public class NmsUpgradeManager {

    private final static class WakeUpType {
        static final public int ENone = 0;
        static final public int EPrompt = 1;
        static final public int ESlient = 2;

        static public String toString(int type) {
            if (type < ENone || type > EPrompt) {
                NmsLog.error(TAG, "invald state: " + type);
                return "error type";
            }
            String typeStrArray[] = { "none", "slient", "prompt" };
            return typeStrArray[type];
        }
    }

    static final public String PUSH_INFO_PIC_PATH = engineadapterforjni.getUserDataPath()
            + "/upgrade/";

    static private NmsUpgradeManager gSingleton = null;
    static final private String TAG = "Upgrade";
    static final public String TAGPUSH = "Push";
    static final private int REQ_MAX_TRY_TIME = 5;
    static final private int REQ_SELEEP_TIME = 15;

    static final private int NETWORK_TIMEOUT = 90;

    static final private long INIT_TIME_OUT = 90;

    static final private String INTERVAL_TIME_KEY_NAME = "nms_isms_upgrade_interval_time";
    static final private String LAST_TIME_KEY_NAME = "nms_isms_upgrade_update_time";

    static final private String NMS_ISMS_STARTUP_TIME_FIRST = "nms_isms_startup_time_first";

    static public boolean mUpgradeStarted = false;
    public static long ONE_DAY = (24 * 60 * 60);

    // M:Activation Statistics
    private long mUpgradeIntervalTime = ONE_DAY;
    private long mLastUpgradeTime = 0;

    private int mWakeupActivateType = WakeUpType.ENone;

    private NmsPushInfo[] pushInfoIDList = null;

    private NmsUpgradeManager() {
        readData();
        startTimer(INIT_TIME_OUT);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        NmsLog.error(TAG, "fatal error that finalize is call");
    }

    /* read data from permanent storage in android */
    private void readData() {
        long tmpIntervalTime = NmsPreferences.getLongValue(INTERVAL_TIME_KEY_NAME);
        long tmpLastUpdateTime = NmsPreferences.getLongValue(LAST_TIME_KEY_NAME);

        if (tmpIntervalTime > 0)
            mUpgradeIntervalTime = tmpIntervalTime;

        if (tmpLastUpdateTime > 0)
            mLastUpgradeTime = tmpLastUpdateTime;
    }

    /* save data from permanent storage in android */
    private void saveData() {
        NmsPreferences.setLongValue(INTERVAL_TIME_KEY_NAME, mUpgradeIntervalTime);
        NmsPreferences.setLongValue(LAST_TIME_KEY_NAME, mLastUpgradeTime);
    }

    private void doClearData() {
        mWakeupActivateType = WakeUpType.ENone;
    }

    public void startTimer(long delay) {

        NmsTimer.NmsKillTimer(NmsTimer.NMS_TIMERID_WAKEUP_ACTIVATE);
        NmsTimer.NmsSetTimer(NmsTimer.NMS_TIMERID_WAKEUP_ACTIVATE, delay);

        NmsLog.trace(TAG, "start timer in " + delay);
    }

    static private String getString(int id) {
        try {
            return NmsService.getInstance().getString(id);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return "";
    }

    private void handleNewVersionOk() {

        if (mWakeupActivateType == WakeUpType.EPrompt) {
            if (pushInfoIDList != null && pushInfoIDList.length > 0) {
                Arrays.sort(pushInfoIDList);
                int lenght = pushInfoIDList.length;
                for (int i = 0; i < lenght; i++) {
                    NmsPushInfo info = pushInfoIDList[i];
                    if (info.pushInfoPriority == 0) {
                        ContentValues values = new ContentValues();

                        values.put(NmsDBOpenHelper.PUSH_INFO_STATUS,
                                NmsPushInfo.PUSH_INFO_STATUS_DIABLE);
                        int ret = NmsDBUtils.getDataBaseInstance(NmsService.getInstance())
                                .updatePushInfoToDB(values, info.pushInfoID);
                        if (ret >= 0) {
                            NmsTimer.NmsRemoveTimer(ret + NmsTimer.NMS_TIMERID_PUSH_INFO);
                        }
                    } else {
                        NmsPushInfo getpushInfo = NmsDBUtils.getDataBaseInstance(
                                NmsService.getInstance()).getNmsSinglePushInfo(
                                NmsDBOpenHelper.PUSH_INFO_ISEXIST, info.pushInfoID);
                        if (getpushInfo == null) {
                            requestPushInfo(info.pushInfoID + "");
                            break;
                        }
                    }
                }
            }
        }
        handleFinished();
        return;
    }

    private void handleFinished() {

        NmsLog.trace(TAG, "handleFinished is call and current state: " + ", lastUpgradeTime: "
                + mLastUpgradeTime + ", intervalTime: " + mUpgradeIntervalTime);

        mLastUpgradeTime = System.currentTimeMillis() / 1000;
        startTimer(mUpgradeIntervalTime);
        saveData();
        doClearData();
    }

    private boolean isNetworkAvailable() {
        try {
            ConnectivityManager connectivity = (ConnectivityManager) NmsService.getInstance()
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            if (connectivity != null) {
                NetworkInfo info = connectivity.getActiveNetworkInfo();
                if (info != null && info.isConnected()) {
                    return true;
                }
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "isNetworkAvailable get Execption: " + e.toString());
            return false;
        }
        return false;
    }

    private void handleNetworkInavailable() {

        NmsLog.trace(TAG, "handleNetworkInavailable is call and current state: "
                + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                + mUpgradeIntervalTime);
        startTimer(mUpgradeIntervalTime);
    }

    private void handleError() {

        NmsLog.trace(TAG, "handleError is call and current state: " + ", lastUpgradeTime: "
                + mLastUpgradeTime + ", intervalTime: " + mUpgradeIntervalTime);
        handleFinished();
    }

    private void doStart(boolean downloadManually) {

        long curTime = System.currentTimeMillis() / 1000;

        if (mLastUpgradeTime > curTime) {
            NmsLog.warn(TAG, "mLastUpgradeTime: " + mLastUpgradeTime + " is bigger than curTime: "
                    + curTime + ", the user may reset the time previous");
            mLastUpgradeTime = 0;
        }

        if ((curTime - mLastUpgradeTime) < mUpgradeIntervalTime) {
            startTimer((mUpgradeIntervalTime - (curTime - mLastUpgradeTime)));
            return;
        }
        startTimer(0);

        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsFeatureSupport.NMS_MSG_FLAG_PRIVATE_MESSAGE) != 0) {
            upgradePrivateInfo();
        }
    }

    private static JSONArray getSoftwareList() {
        try {
            PackageManager pm = NmsService.getInstance().getPackageManager();
            List<ApplicationInfo> listAppcations = pm
                    .getInstalledApplications(PackageManager.GET_UNINSTALLED_PACKAGES);
            String ret = "";

            JSONArray array = new JSONArray();
            int lenght = listAppcations.size();
            for (int i = 0; i < lenght; i++) {
                String appName = (String) listAppcations.get(i).loadLabel(pm);
                array.put(appName);
            }
            ret = array.toString();
            return array;

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }

    ArrayList<NmsPushInfo> pushInfoLists;

    private JSONArray getPushInfoList() {
        pushInfoLists = NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).getPushInfoList(
                true);
        JSONArray array = new JSONArray();
        try {
            int size = pushInfoLists.size();
            for (int i = 0; i < size; i++) {
                NmsPushInfo push = pushInfoLists.get(i);
                JSONObject stoneObject = new JSONObject();
                stoneObject.put("pushInfoID", push.pushInfoID);
                stoneObject.put("pushInfoStatus", push.pushInfoStatus);
                stoneObject.put("infoOpratertime", push.infoOpratertime);
                stoneObject.put("infoShowTime",  push.infoOpShowTime);
                stoneObject.put("infoReceiveTime", push.infoReceiveTime);
                stoneObject.put("timeZone", getCurrentTimezoneOffset(push.timeZoneID));
                stoneObject.put("timeZoneID", push.timeZoneID);
                array.put(stoneObject);
            }
            String result = array.toString();
            NmsLog.trace(TAGPUSH, " getPushInfoList result " + result);
        } catch (Exception e) {
            return null;
        }
        return array;
    }
    
    
    private String getClientIp() {
     String ip = getWifiIP();
     if(ip ==null){
         ip=  getLocalIpAddress();
      }
     return ip;
    }
    
    public  String getLocalIpAddress(){ 
        
        try{ 
             for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) { 
                 NetworkInterface intf = en.nextElement();   
                    for (Enumeration<InetAddress> enumIpAddr = intf   
                            .getInetAddresses(); enumIpAddr.hasMoreElements();) {   
                        InetAddress inetAddress = enumIpAddr.nextElement();   
                        if (!inetAddress.isLoopbackAddress() && InetAddressUtils.isIPv4Address(inetAddress.getHostAddress())) {   
                             
                            return inetAddress.getHostAddress().toString();   
                        }   
                    }   
             } 
        }catch (SocketException e) { 
            // TODO: handle exception 
        } 
         
        return null;  
    } 
    
    
    public  String getWifiIP(){
        WifiManager wifiManager = (WifiManager)NmsService.getInstance().getSystemService(Context.WIFI_SERVICE);
        if (!wifiManager.isWifiEnabled()) {
            return null;
        }
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();     
        int ipAddress = wifiInfo.getIpAddress(); 
        if(ipAddress == 0){
            return null;
        }
        String ip = intToIp(ipAddress); 
        return ip;
    }
    private static String intToIp(int i) {     
        return (i & 0xFF ) + "." +     
      ((i >> 8 ) & 0xFF) + "." +     
      ((i >> 16 ) & 0xFF) + "." +     
      ( i >> 24 & 0xFF) ;
   }
    

    private static long getstartupTimeFirst() {

        long startupTimeFirst = NmsPreferences.getLongValue(NMS_ISMS_STARTUP_TIME_FIRST);
        if (startupTimeFirst == 0) {
            startupTimeFirst = System.currentTimeMillis();
            NmsPreferences.setLongValue(NMS_ISMS_STARTUP_TIME_FIRST, startupTimeFirst);
        }
        return startupTimeFirst;
    }

    private static String getPhoneManufacturer() {
        return android.os.Build.MANUFACTURER;
    }

    private static String getPhoneType() {
        return android.os.Build.MODEL;
    }

    private static String getCpuInfo() {
        return android.os.Build.CPU_ABI;
    }

    private static String getMemoryInfo() {
        long totalMemory = -1;
        long freeMemory = -1;
        FileReader r = null;
        BufferedReader bufferedRead = null;
        try {
            String str1 = "/proc/meminfo";
            String str2 = "";
            r = new FileReader(str1);
            bufferedRead = new BufferedReader(r, 8192);
            str2 = bufferedRead.readLine();
            String str4 = str2.substring(str2.length() - 9, str2.length() - 3);
            totalMemory = (long) Double.parseDouble(str4);

        } catch (Exception e) {
            NmsLog.warn(TAG, "getMemoryInfo get Execption for total memory: " + e.toString());
            totalMemory = -1;
        } finally {
            try {
                if (bufferedRead != null) {
                    bufferedRead.close();
                    bufferedRead = null;
                }
                if (r != null) {
                    r.close();
                    r = null;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        try {
            ActivityManager am = (ActivityManager) NmsService.getInstance().getSystemService(
                    Context.ACTIVITY_SERVICE);
            ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
            am.getMemoryInfo(memoryInfo);
            freeMemory = memoryInfo.availMem / 1000;
        } catch (Exception e) {
            NmsLog.warn(TAG, "getMemoryInfo get Execption for free memory: " + e.toString());
            freeMemory = -1;
        }

        return totalMemory + "|" + freeMemory;
    }

    private static String[] getNetworkStatus() {
        String status[] = { "unknown", "unknown" };
        try {
            ConnectivityManager manager = (ConnectivityManager) NmsService.getInstance()
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo info = manager.getActiveNetworkInfo();
            if (info != null) {
                if (ConnectivityManager.TYPE_WIFI == info.getType()) {
                    status[0] = info.getTypeName();
                    if (status[0] == null)
                        status[0] = "wifi";
                } else {
                    status[0] = info.getExtraInfo();
                    if (status[0] == null)
                        status[0] = "mobile";
                }

                status[1] = info.isRoaming() ? "1" : "0";
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "getNetworkStatus get Execption: " + e.toString());
        }
        return status;
    }

    private static JSONArray getImsiInfo() {
        String ret = engineadapter.get().nmsGetImsiList();
        JSONArray array = new JSONArray();
        try {
            String[] imsiList = ret.split("\\|");
            int lenght = imsiList.length;
            for (int i = 0; i < lenght; i++) {
                if (!TextUtils.isEmpty(imsiList[i])) {
                    array.put(imsiList[i]);
                    // array.put("310029012633417");
                }
            }
            String result = array.toString();
            NmsLog.trace(TAGPUSH, "..getImsiInfo..result: " + result);
        } catch (Exception e) {
            return null;
        }
        return array;
    }

    private static JSONArray getPhoneNumberInfo() {
        String ret = engineadapter.get().nmsGetPhoneNumberList();
        JSONArray array = new JSONArray();
        try {
            String[] numberList = ret.split("\\|");
            int length = numberList.length;
            for (int i = 0; i < length; i++) {
                if (!TextUtils.isEmpty(numberList[i])) {
                    array.put(numberList[i]);
                }
            }
        } catch (Exception e) {
            return null;
        }
        String result = array.toString();
        return array;
    }

    private static String getImeiInfo() {
        String ret = "unknown";
        try {
            ret = engineadapterforjni.getIMEI();
            if (ret == null)
                ret = "unknown";
        } catch (Exception e) {
            NmsLog.warn(TAG, "getImeiInfo get Execption: " + e.toString());
        }
        return ret;
    }

    private static String getDisplayInfo() {
        int width = -1;
        int height = -1;
        try {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager wmg = (WindowManager) NmsService.getInstance().getSystemService(
                    Context.WINDOW_SERVICE);
            wmg.getDefaultDisplay().getMetrics(dm);
            width = dm.widthPixels;
            height = dm.heightPixels;
        } catch (Exception e) {
            NmsLog.warn(TAG, "getNetworkStatus get Execption: " + e.toString());
            width = -1;
            height = -1;
        }

        return width + "|" + height;
    }

    private static String getPnType() {
        String ret = "unknown";
        try {
            ret = engineadapterforjni.nmsGetPNType();
            if (ret == null)
                ret = "unknown";
        } catch (Exception e) {
            NmsLog.warn(TAG, "getPnType get Execption: " + e.toString());
        }
        return ret;
    }

    private static String getCellLocationInfo() {
        try {
            TelephonyManager telephonyManager = (TelephonyManager) NmsService.getInstance()
                    .getSystemService(Context.TELEPHONY_SERVICE);

            if (telephonyManager == null) {
                NmsLog.warn(TAG, "getCellLocationInfo failed to get telephonyManager");
                return "";
            }
            CellLocation location = telephonyManager.getCellLocation();
            if (location == null) {
                NmsLog.warn(TAG, "getCellLocationInfo failed to get cellLocation");
                return "";
            }
            int networkType = telephonyManager.getNetworkType();

            if (location instanceof GsmCellLocation) {
                GsmCellLocation gsmCellLocation = (GsmCellLocation) telephonyManager
                        .getCellLocation();
                return String.format("gsm|%d|%d|%d", networkType, gsmCellLocation.getCid(),
                        gsmCellLocation.getLac());
            } else if (location instanceof CdmaCellLocation) {
                CdmaCellLocation cdmaCellLocation = (CdmaCellLocation) telephonyManager
                        .getCellLocation();
                return String.format("cdma|%d|%d|%d|%d", networkType,
                        cdmaCellLocation.getBaseStationId(), cdmaCellLocation.getSystemId(),
                        cdmaCellLocation.getNetworkId());
            } else {
                NmsLog.warn(TAG, "getCellLocationInfo unsupport type: " + networkType);
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "getCellLocationInfo get Execption: " + e.toString());
        }

        return "";
    }

    private static int getOsVersion() {
        return android.os.Build.VERSION.SDK_INT;
    }

    private static String getLanguage() {
        String ret = Locale.getDefault().getLanguage();
        return (ret != null) ? ret : "";
    }

    private static String getSmsServiceCenter() {

        try {
            final String smsUri = "content://sms/";
            final String serviceCenter = "service_center";
            Cursor cur = NmsService
                    .getInstance()
                    .getContentResolver()
                    .query(Uri.parse(smsUri), new String[] { serviceCenter },
                            "type = ? AND service_center IS NOT NULL", new String[] { "1" },
                            "date DESC LIMIT 1");

            if (cur == null) {
                NmsLog.warn(TAG, "get cursor is null");
                return "";
            }
            if (!cur.moveToFirst()) {
                cur.close();
                NmsLog.trace(TAG, "not inbox msg with service center in sms db");
                return "";
            }

            String ret = cur.getString(cur.getColumnIndex(serviceCenter));
            cur.close();

            if (ret == null)
                ret = "";
            return ret;

        } catch (Exception e) {
            NmsLog.warn(TAG, "getSmsServiceCenter get Execption: " + e.toString());
        }
        return "";
    }

    private void doUpgradePrivateInfo() {
        try {
            // TODO Auto-generated method stub
            String privateClickTime = null;
            String openPrivateMsg = null;
            String mPrivateContactCount = null;
            JSONObject data = new JSONObject();
            String mDataToUpgrage = NmsPrivateStatistics.getPrivateData();
            if (mDataToUpgrage == null) {
                NmsTimer.NmsKillTimer(NmsTimer.NMS_PRIVATE_UPGRADE);
                NmsTimer.NmsSetTimer(NmsTimer.NMS_PRIVATE_UPGRADE, ONE_DAY);
                NmsLog.trace(TAG, "get mDataToUpgrage is null");
                return;
            }
            data.put("phonenum", getPhoneNumberInfo());
            data.put("imsi", getImsiInfo());
            data.put("manufacturer", getPhoneManufacturer());
            data.put("device", engineadapter.get().nmsGetDevicelId());
            data.put("version", engineadapterforjni.getClientVersion());
            data.put("privateMsg", mDataToUpgrage);
            String sendStr = data.toString();
            NmsLog.trace(TAG, "get private data: " + data.toString());
            HttpPost httpPost = new HttpPost(NmsService.getInstance().getString(
                    R.string.STR_NMS_PRIVATE_UPGRADE_URL));
            StringEntity entity = new StringEntity(sendStr, HTTP.UTF_8);
            httpPost.setEntity(entity);
            HttpClient httpClient = new DefaultHttpClient();
            httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            HttpResponse httpResp = httpClient.execute(httpPost);
            if (httpResp.getStatusLine().getStatusCode() != 200) {
                NmsLog.warn(TAG, "get request reuslt form server err23456or: "
                        + httpResp.getStatusLine().getStatusCode());
            } else {
                NmsPrivateStatistics.clear();
            }
            NmsTimer.NmsKillTimer(NmsTimer.NMS_PRIVATE_UPGRADE);
            NmsTimer.NmsSetTimer(NmsTimer.NMS_PRIVATE_UPGRADE, ONE_DAY);
        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in upgradePrivateInfo " + e.toString());
            e.printStackTrace();
        }
    }

    public void upgradePrivateInfo() {
        try {
            new Thread(new Runnable() {

                @Override
                public void run() {
                    doUpgradePrivateInfo();
                }
            }).start();
        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in upgradePrivateInfo " + e.toString());
            e.printStackTrace();
        }
    }

    private boolean doRequestNewVersion() {
        try {
            JSONObject data = new JSONObject();
            data.put("channel", engineadapter.get().nmsGetChannelId());
            data.put("devId", engineadapter.get().nmsGetDevicelId());
            data.put("version", engineadapterforjni.getClientVersion());
            data.put("IsStandalone", 0);// 0:system , 1:standalone
            data.put("imsiList", getImsiInfo());
            data.put("numberList", getPhoneNumberInfo());
            data.put("mainImsiIndex", engineadapter.get().nmsGetMainImsiIndex());
            data.put("imei", getImeiInfo());
            data.put("pnType", getPnType());
            data.put("resolution", getDisplayInfo());

            data.put("manufacturer", getPhoneManufacturer());
            data.put("model", getPhoneType());
            data.put("processor", getCpuInfo());
            data.put("osid", getOsVersion());
            data.put("language", NmsCommonUtils.getLanguageCode(Locale.getDefault()));

            data.put("memory", getMemoryInfo());

            String netWorkStatus[] = getNetworkStatus();
            data.put("networkStatus", netWorkStatus[0]);
            data.put("roaming", netWorkStatus[1]);
            data.put("cellId", getCellLocationInfo());
            data.put("smsSrvCenter", getSmsServiceCenter());
            data.put("softwareList", getSoftwareList());

            data.put("activationStatus", engineadapter.get().nmsUIIsActivated() ? 2 : 1);
            data.put("clientIp", getClientIp());
            data.put("pushInfoList", getPushInfoList());

            data.put("startupTimeFirst", getstartupTimeFirst());
            // M: Activation Statistics
            JSONObject statisticsObject = NmsStatistics.toJasonData();
            if (statisticsObject != null) {
                data.put("statistics", statisticsObject);
            } else {
                data.put("statistics", new JSONObject());
            }
            String sendStr = data.toString();

            NmsLog.trace(TAG, "get request data: " + data.toString());

            HttpPost httpPost = new HttpPost(NmsService.getInstance().getString(
                    R.string.STR_NMS_PUSH_DEVICE_URL)
                    + "deviceInfo");
            httpPost.addHeader("protocolVersion",NmsService.getInstance().getString(
                    R.string.STR_NMS_PUSH_PROTOCOLVERSION));

            StringEntity entity = new StringEntity(sendStr, HTTP.UTF_8);
            httpPost.setEntity(entity);
            
            HttpClient httpClient = new DefaultHttpClient();
            httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            HttpResponse httpResp = httpClient.execute(httpPost);
            NmsLog.trace(TAGPUSH,
                    "..doRequestNewVersion..httpResp.getStatusLine().getStatusCode()  "
                            + httpResp.getStatusLine().getStatusCode());
            if (httpResp.getStatusLine().getStatusCode() != 200) {
                NmsLog.warn(TAG, "get request reuslt form server error: "
                        + httpResp.getStatusLine().getStatusCode());
                return false;
            }
            // M: Activation Statistics
            NmsStatistics.clear();
            doClearData();
            String resultStr = EntityUtils.toString(httpResp.getEntity(), HTTP.UTF_8);
            NmsLog.trace(TAG, "get request reuslt form server: " + resultStr);
            JSONTokener jsonParser = new JSONTokener(resultStr);
            JSONObject jsonResult = (JSONObject) jsonParser.nextValue();

            mUpgradeIntervalTime = jsonResult.getLong("pollingPeriod");

            // M:Activation Statistics
            if (mUpgradeIntervalTime <= 0) {
                NmsLog.error(TAG, "got invalid mUpgradeIntervalTime: " + mUpgradeIntervalTime);
                mUpgradeIntervalTime = ONE_DAY;
            }

            mWakeupActivateType = WakeUpType.ENone;
            if (!jsonResult.isNull("msgList")) {
                JSONArray msgList = jsonResult.optJSONArray("msgList");
                int size = msgList.length();
                pushInfoIDList = new NmsPushInfo[size];
                for (int i = 0; i < size; i++) {
                    JSONObject msgObject = msgList.optJSONObject(i);
                    NmsPushInfo pushInfo = new NmsPushInfo();
                    pushInfo.pushInfoID = msgObject.getString("pushInfoID");
                    pushInfo.pushInfoPriority = msgObject.getInt("msgPriority");
                    pushInfoIDList[i] = pushInfo;
                    NmsLog.trace(TAGPUSH, " jsonResult pushInfo.pushInfoID  " + pushInfo.pushInfoID
                            + "  pushInfoPriority:" + pushInfo.pushInfoPriority);
                }
                mWakeupActivateType = WakeUpType.EPrompt;
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in doRequestNewVersion " + e.toString());
            return false;
        }
        return true;
    }

    private void requestPushInfo(final String pushInfoID) {

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    HttpPost httpPost = new HttpPost(NmsService.getInstance().getString(
                            R.string.STR_NMS_PUSH_DEVICE_URL)
                            + "detailMessage");
                    StringEntity entity = new StringEntity(pushInfoID, HTTP.UTF_8);
                    httpPost.setEntity(entity);

                    HttpClient httpClient = new DefaultHttpClient();
                    httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
                            NETWORK_TIMEOUT * 1000);
                    httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT,
                            NETWORK_TIMEOUT * 1000);

                    HttpResponse httpResp = httpClient.execute(httpPost);

                    NmsLog.trace(TAGPUSH,
                            " requestPushInfo  httpResp.getStatusLine().getStatusCode()."
                                    + httpResp.getStatusLine().getStatusCode());

                    if (httpResp.getStatusLine().getStatusCode() != 200) {
                        return;
                    }
                    // M: Activation Statistics
                    String resultStr = EntityUtils.toString(httpResp.getEntity(), HTTP.UTF_8);

                    NmsLog.trace(TAG, "requestPushInfo form server: " + resultStr);
                    JSONTokener jsonParser = new JSONTokener(resultStr);
                    JSONObject jsonResult = (JSONObject) jsonParser.nextValue();
                    NmsPushInfo pushInfo = new NmsPushInfo();
                    pushInfo.pushInfoID = jsonResult.getString("pushInfoID");
                    pushInfo.pushInfoContent = jsonResult.getString("content");
                    pushInfo.pushTitle = jsonResult.getString("title");
                    if (!jsonResult.isNull("pushInfoUrl")) {
                        pushInfo.pushInfoUrl = jsonResult.getString("pushInfoUrl");
                    }
                    if (!jsonResult.isNull("picUrl")) {
                        pushInfo.pushPicUrl = jsonResult.getString("picUrl");
                    }
                    pushInfo.pushShowTime = jsonResult.getString("showTime");
                    pushInfo.randomRange = jsonResult.getInt("randomRange");
                    pushInfo.pushShowType = jsonResult.getInt("showType");// meiyong
                    pushInfo.pushInteractionType = jsonResult.getInt("interactionType");// meiyong
                    pushInfo.pushMsgType = jsonResult.getInt("msgType");
                    pushInfo.pushExpireTime = jsonResult.getString("expireTime");
                    pushInfo.pushInfoStatus = NmsPushInfo.PUSH_INFO_STATUS_DEFAULT;

                    if (pushInfo.pushInfoID == null || pushInfo.pushMsgType == -1) {
                        pushInfo.pushInfoStatus = NmsPushInfo.PUSH_INFO_STATUS_ERROR;
                    } else {
                        if (pushInfo.pushMsgType == 1 && pushInfo.pushInfoUrl == null) {
                            pushInfo.pushInfoStatus = NmsPushInfo.PUSH_INFO_STATUS_ERROR;
                        }
                    }
                    long currentTime = System.currentTimeMillis();
                    pushInfo.infoReceiveTime =new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",new Locale("English")).format(currentTime);
                    pushInfo.timeZoneID =TimeZone.getDefault().getID();
                    
                    NmsLog.trace(TAGPUSH, " requestPushInfo pushInfo.pushInfoID "
                            + pushInfo.pushInfoID + " pushTitle:" + pushInfo.pushTitle
                            + " pushInfo.pushMsgType:" + pushInfo.pushMsgType
                            + " pushInfo.timeZoneID:" + pushInfo.timeZoneID);
                    long ret = NmsDBUtils.getDataBaseInstance(NmsService.getInstance())
                            .insertPushInfoToDB(pushInfo);
                    long time = Long.parseLong(pushInfo.pushShowTime);
                    if (time <= 0) {
                        time = 0;
                    } else {
                        time = (long) ((time - System.currentTimeMillis()) / 1000 + (Math.random() * pushInfo.randomRange) * 60 * 60);
                    }
                    NmsLog.trace(TAGPUSH, " requestPushInfo time " + time
                            + " pushInfo.pushShowTime:" + pushInfo.pushShowTime + "  _id :" + ret
                            + "  Activated:" + engineadapter.get().nmsUIIsActivated()
                            + "  pushMsgType :" + pushInfo.pushMsgType);
                    if (ret >= 0) {
                        downLoadPic(pushInfo.pushPicUrl, pushInfo.pushInfoID);
                        if ((engineadapter.get().nmsUIIsActivated() && pushInfo.pushMsgType == 0)
                                || pushInfo.pushInfoStatus == NmsPushInfo.PUSH_INFO_STATUS_ERROR) {

                            ContentValues values = new ContentValues();
                            SimpleDateFormat dateformat = new SimpleDateFormat(
                                    "yyyy-MM-dd HH:mm:ss");
                            values.put(NmsDBOpenHelper.PUSH_INFO_OPRATERTIME,
                                    dateformat.format(System.currentTimeMillis())
                                            .toString());
                            values.put(NmsDBOpenHelper.PUSH_INFO_STATUS,
                                    NmsPushInfo.PUSH_INFO_STATUS_ERROR);
                            NmsDBUtils.getDataBaseInstance(NmsService.getInstance())
                                    .updatePushInfoToDB(values, pushInfo.pushInfoID);
                        } else {
                            int id = NmsTimer.NmsCreateTimer(
                                    (int) (ret + NmsTimer.NMS_TIMERID_PUSH_INFO), time > 0 ? time
                                            : 30);
                        }
                    } else {
                        NmsLog.warn(TAG, "get execption in requestPushInfo   ret" + ret);
                    }
                } catch (Exception e) {
                    NmsLog.warn(TAG, "get execption in requestPushInfo " + e.toString());
                }
            }
        }).start();

        int size = pushInfoLists.size();
        for (int i = 0; i < size; i++) {
            NmsPushInfo push = pushInfoLists.get(i);
            ContentValues values = new ContentValues();
            values.put(NmsDBOpenHelper.PUSH_INFO_STATUS, NmsPushInfo.PUSH_INFO_STATUS_DIABLE);
            NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).updatePushInfoToDB(values,
                    push.pushInfoID);
        }
    }
    
    public static String getCurrentTimezoneOffset(String timeZoneID) {
        TimeZone tz = TimeZone.getTimeZone(timeZoneID);
        Calendar cal = GregorianCalendar.getInstance(tz);
        int offsetInMillis = tz.getOffset(cal.getTimeInMillis());

        String offset = String.format("%02d%02d", Math.abs(offsetInMillis / 3600000),
                Math.abs((offsetInMillis / 60000) % 60));
        offset = (offsetInMillis >= 0 ? "+" : "-") + offset;

        return offset;
    }

    static long requestNewVersionLastTime;

    private void requestNewVersion() {
        if (System.currentTimeMillis() > requestNewVersionLastTime + 300 * 1000) {

            requestNewVersionLastTime = System.currentTimeMillis();
            NmsLog.trace(TAG, "doRequestNewVersion is call and current state: "
                    + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                    + mUpgradeIntervalTime);
            try {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            int tryTime = 0;
                            while (tryTime < REQ_MAX_TRY_TIME) {
                                if (gSingleton.doRequestNewVersion()) {
                                    break;
                                }
                                tryTime++;
                                Thread.sleep(REQ_SELEEP_TIME * 1000);
                            }

                            if (tryTime < REQ_MAX_TRY_TIME) {
                                gSingleton.handleNewVersionOk();
                            } else {
                                startTimer(mUpgradeIntervalTime);
                            }

                        } catch (Exception e) {
                            NmsLog.nmsPrintStackTrace(e);
                            gSingleton.handleError();
                        }
                    }

                }).start();

            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
                gSingleton.handleError();
            }
        } else {
            startTimer(mUpgradeIntervalTime);
        }
    }

    private static String toHexString(byte[] b) {
        char HEX_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
                'E', 'F' };
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (int i = 0; i < b.length; i++) {
            sb.append(HEX_DIGITS[(b[i] & 0xf0) >>> 4]);
            sb.append(HEX_DIGITS[b[i] & 0x0f]);
        }
        return sb.toString();
    }

    private static String getMd5(File file) {
        InputStream fis;
        byte[] buffer = new byte[1024 * 10];
        int numRead = 0;
        MessageDigest md5 = null;
        try {
            fis = new FileInputStream(file);
            md5 = MessageDigest.getInstance("MD5");
            while ((numRead = fis.read(buffer)) > 0) {
                md5.update(buffer, 0, numRead);
            }
            fis.close();
            return toHexString(md5.digest());
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return "";
    }

    static public void start(boolean downloadManually) {
        try {
            mUpgradeStarted = true;
            if (gSingleton == null)
                gSingleton = new NmsUpgradeManager();

            gSingleton.doStart(downloadManually);

        } catch (Exception e) {
            NmsLog.error(TAG, "start function get Exception: " + e.toString());
        }
    }

    static public void handleTimerEvent() {
        try {
            
            if (gSingleton == null) {
                NmsLog.error(TAG, "handleTimerEvent error for gSingleton is null");
                return;
            }

            if (!gSingleton.needToUpgrade()) {
                NmsLog.trace(TAG, "handleNetworkEvent is call not need to update");
                return;
            }
            if (!gSingleton.isNetworkAvailable()) {
                gSingleton.handleNetworkInavailable();
                return;
            }
            gSingleton.requestNewVersion();
        } catch (Exception e) {
            NmsLog.error(TAG, "handleTimerEvent function get Exception: " + e.toString());
        }
    }

    private boolean needToUpgrade() {
        long curTime = System.currentTimeMillis() / 1000;
        if ((curTime - mLastUpgradeTime) < mUpgradeIntervalTime) {
            startTimer((mUpgradeIntervalTime - (curTime - mLastUpgradeTime)));
            return false;
        } else {
            return true;
        }
    }

    static public synchronized void handleNetworkEvent() {
        try {
            if (gSingleton == null) {
                NmsLog.error(TAG, "handleNetworkEvent error for gSingleton is null");
                return;
            }
            NmsLog.trace(
                    TAG,
                    "handleNetworkEvent is call, state: +" + " and curTime: "
                            + System.currentTimeMillis() / 1000);
            if (!gSingleton.needToUpgrade()) {
                NmsLog.trace(TAG, "handleNetworkEvent is call not need to update");
                return;
            }

            // shezhi dingshi qi qidong
            gSingleton.startTimer(0);

        } catch (Exception e) {
            NmsLog.error(TAG, "handleNetworkEvent function get Exception: " + e.toString());
        }
    }

    private void downLoadPic(final String url, final String fileName) {
        NmsLog.trace(TAGPUSH, " downLoadPic  url:" + url);

        if (TextUtils.isEmpty(url) || !url.startsWith("http")) {
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                try {
                    Bitmap bm = BitmapFactory.decodeStream(getImageStream(url));
                    saveFileRunnable(bm, fileName);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();

    }

    public InputStream getImageStream(String path) throws Exception {
        URL url = new URL(path);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setConnectTimeout(5 * 1000);
        conn.setRequestMethod("GET");
        if (conn.getResponseCode() == HttpURLConnection.HTTP_OK) {
            return conn.getInputStream();
        }
        return null;
    }

    public void saveFile(Bitmap bm, String fileName) throws IOException {
        File dirFile = new File(PUSH_INFO_PIC_PATH);
        if (!dirFile.exists()) {
            dirFile.mkdir();
        }
        File myCaptureFile = new File(PUSH_INFO_PIC_PATH + fileName);
        BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(myCaptureFile));
        bm.compress(Bitmap.CompressFormat.JPEG, 80, bos);
        bos.flush();
        bos.close();
    }

    private void saveFileRunnable(final Bitmap bm, final String fileName) {

        new Thread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                try {
                    saveFile(bm, fileName);
                } catch (IOException e) {
                    e.printStackTrace();
                }

            }
        }).start();
    }

}
