package com.mediatek.configurecheck2;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
//IMEI
import android.telephony.TelephonyManager;

//DM
import android.os.ServiceManager;
import com.mediatek.common.dm.DmAgent;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import com.mediatek.custom.CustomProperties;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;


/**
 * 
 * @author mtk80202
 *      this is a demo for developer how to add new configure check item.
 *      please write your owner configure checker here by extends CheckItemBase 
 */

class CheckIMEI extends CheckItemBase {

    private String mImei = null;
    
    CheckIMEI(Context c, String key) {
        super(c, key);
        /* No check + No auto configration */

        setTitle(R.string.imei_title);
        setNote(R.string.imei_note);
    }
    
    public boolean onCheck() {
        TelephonyManager tm = (TelephonyManager) getContext().getSystemService(
                Context.TELEPHONY_SERVICE);
        TelephonyManagerEx tmEx = TelephonyManagerEx.getDefault();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (!getKey().equals(CheckItemKeySet.CI_IMEI_IN_DM)) {
                //if is Gemini phone, need to query two IMEI, except
                //CI_IMEI_IN_DM which only need to query the default IMEI
                String nextLine = System.getProperty("line.separator");
                String sim1IMEI = tmEx.getDeviceId(PhoneConstants.GEMINI_SIM_1);
                CTSCLog.e("IMEI", "sim1IMEI: " + sim1IMEI);
                String sim2IMEI = tmEx.getDeviceId(PhoneConstants.GEMINI_SIM_2);
                CTSCLog.e("IMEI", "sim2IMEI: " + sim2IMEI);
                
                StringBuilder sb = new StringBuilder(getContext().getString(R.string.imei_slot1));
                if (null == sim1IMEI) {
                    sb.append("null");
                } else {
                    sb.append(sim1IMEI);
                }
                sb.append(nextLine).append(getContext().getString(R.string.imei_slot2));
                if (null == sim2IMEI) {
                    sb.append("null");
                } else {
                    sb.append(sim2IMEI);  
                }
                
                mImei = sb.toString();
                setValue(mImei);
                return true;
            }
        }
        
        mImei = tm.getDeviceId();
        CTSCLog.e("IMEI", "IMEI: " + mImei);
        
        if (mImei == null) {
            setValue("null");
        } else {
            setValue(mImei);
        }
        return true;
    }

    public check_result getCheckResult() {
        
        mResult = super.getCheckResult();
        if (mImei == null && (check_result.UNKNOWN == mResult)) {
            mResult = check_result.WRONG;
        }
        return mResult;
    }
}

class CheckDMState extends CheckItemBase {

    CheckDMState(Context c, String key) {
        super(c, key);

        setTitle(R.string.smsreg_state_title);
        if (key.equals(CheckItemKeySet.CI_DMSTATE_CHECK_ONLY)) {
            setNote(R.string.smsreg_note_off);
            //setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setNote(R.string.smsreg_note_on);
            setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        }
    }

    private DmAgent getDmAgent() {
        IBinder binder = ServiceManager.getService("DmAgent");
        return DmAgent.Stub.asInterface(binder);
    }

    public check_result getCheckResult() {

        check_result result = check_result.UNKNOWN;
        DmAgent dma = getDmAgent();
        if (dma == null) {
            CTSCLog.e("DMCheck", "DM agent is null ");
            setValue(R.string.smsreg_err_no_agent);
            return result;
        }

        try {
            byte[] data = dma.getRegisterSwitch();
            int cta = 0;
            if (data != null) {
                cta = Integer.parseInt(new String(data));
                CTSCLog.e("DMCheck", "dm sting " + new String(data));
            }
            
                if(cta == 1) {
                    setValue(R.string.ctsc_enabled);
                } else {
                    setValue(R.string.ctsc_disabled);
                }
                
                if (getKey().equals(CheckItemKeySet.CI_DMSTATE_AUTO_CONFG)) {
                    result = (cta == 1) ? check_result.RIGHT : check_result.WRONG;
                } else {
                result = super.getCheckResult();                    
                }
                CTSCLog.e("DMCheck", "dm result" + result);

        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }
        CTSCLog.e("DMCheck", "result = " + result);
        return result;
    }

    public boolean onReset() {

        if (!isConfigurable()) {
            return false;
        }

        DmAgent dma = getDmAgent();
        if (null == dma) {
            return false;
        }

        String data = new String("1");
        try {
            dma.setRegisterSwitch(data.getBytes());
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return true;
    }
}

class DMHelpUtil {
    public static final String SMS_NUMBER = "smsNumber";
    public static final String SMS_PORT = "smsPort";
    public static final String DM_SERVER = "Addr";

    public static String querySmsRegProvider(Context c, String name) {
        StringBuilder sb = new StringBuilder("");
        Uri dmSmsregUri = Uri.parse("content://com.mediatek.providers.smsreg");
        try {
            Cursor cursor = c.getContentResolver().query(dmSmsregUri, null,
                    null, null, null);
            while (cursor.moveToNext()) {
                sb.append(cursor.getString(cursor.getColumnIndex(name)));
            }
            cursor.close();
        } catch (NullPointerException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
        CTSCLog.e("DMCheck", "DM query Key:" + name + ". result:" + sb.toString());
        return sb.toString();
    }

    public static String queryDMProvider(Context c, String type, String name) {
        StringBuilder sb = new StringBuilder("");
        Uri dmServerUri = Uri.parse("content://com.mediatek.providers.mediatekdm/" + type);

        try {
            Cursor cursor = c.getContentResolver().query(dmServerUri, null,
                    null, null, null);
            while (cursor.moveToNext()) {
                sb.append(cursor.getString(cursor.getColumnIndex(name)));
            }
            cursor.close();
        } catch (NullPointerException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
        CTSCLog.e("DMCheck", "DM query type:" + type + ". Key:" + name + ". result:" + sb.toString());
        return sb.toString();
    }

    public static Intent getForwardDMIntent(Context context) {

        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.mediatek.engineermode", 
                "com.mediatek.engineermode.devicemgr.DeviceMgr"));

        if (context.getPackageManager().resolveActivity(intent, 0) != null) {
            return intent;
        } else {
            throw new RuntimeException(
                    "Can't find such activity: com.mediatek.engineermode, " +
                    "com.mediatek.engineermode.devicemgr.DeviceMgr");
        }
    }
}

class CheckDMAuto extends CheckItemBase {
    
    private boolean mLab;
    String mSmsNumber;
    String mSmsPort;
    String mDmServer;
    
    CheckDMAuto(Context c, String key) {
        super(c, key);

        setTitle(R.string.smsreg_info_title);
        //setNote(R.sting.smsreg_info_note);
        //setProperty(PROPERTY_AUTO_CHECK);
    }
    
    public boolean onCheck() {
        mSmsNumber = DMHelpUtil.querySmsRegProvider(getContext(), DMHelpUtil.SMS_NUMBER);
        mSmsPort   = DMHelpUtil.querySmsRegProvider(getContext(), DMHelpUtil.SMS_PORT);
        mDmServer  = DMHelpUtil.queryDMProvider(getContext(), "OMSAcc", DMHelpUtil.DM_SERVER);
        
        setNote(getContext().getString(R.string.smsreg_number) + ":" + mSmsNumber + "\n" + 
                getContext().getString(R.string.smsreg_port_title) + ":" + mSmsPort + "\n" + 
                getContext().getString(R.string.dmsrv_title) + ":" + mDmServer + "\n" +
                getContext().getString(R.string.smsreg_info_note));
        
        return true;
    }

    public check_result getCheckResult() {
        
        if (!super.getCheckResult().equals(check_result.UNKNOWN)) {
            return mResult;
        }
        
        if ((mDmServer.equals("http://218.206.176.97") || mDmServer.startsWith("http://218.206.176.97:")) &&
             mSmsNumber.endsWith("1065840409")) {
            mLab = true;
            setValue(R.string.smsreg_info_lab);
           } else if (mSmsNumber.endsWith("10654040") && 
                      (mDmServer.equals("http://dm.monternet.com") || 
                       mDmServer.startsWith("http://dm.monternet.com:"))){
            mLab = false;
            setValue(R.string.smsreg_info_curr);
        } else {
            mResult = check_result.WRONG;
            setValue(R.string.smsreg_info_conflict);
        }
        
           if (!mSmsPort.equals("16998")) {
            mResult = check_result.WRONG;
            setValue(R.string.smsreg_info_port_wrong);
        }
        return mResult;
    }
}

class CheckSMSNumber extends CheckItemBase {

    boolean bLab;

    CheckSMSNumber(Context c, String key) {
        super(c, key);

        setTitle(R.string.smsreg_number);

        setProperty(PROPERTY_AUTO_FWD);
        setProperty(PROPERTY_AUTO_CHECK);
        if (key.equals(CheckItemKeySet.CI_SMSNUMBER_LAB)) {
            setNote(R.string.smsreg_number_lab);
            bLab = true;
        } else {
            bLab = false;
            setNote(R.string.smsreg_number_cur);
        }
    }
    
    public boolean onCheck() {
        String smsNumber = DMHelpUtil.querySmsRegProvider(getContext(),
                DMHelpUtil.SMS_NUMBER);
        setValue(smsNumber);
        if (bLab) {
            mResult = smsNumber.equals("1065840409") ? check_result.RIGHT
                    : check_result.WRONG;
        } else {
            mResult = smsNumber.equals("10654040") ? check_result.RIGHT
                    : check_result.WRONG;
        }
        return true;
    }

    public check_result getCheckResult() {
        
        return mResult;
    }

    public Intent getForwardIntent() {
        return DMHelpUtil.getForwardDMIntent(getContext());
    }
}

class CheckSMSPort extends CheckItemBase {

    boolean bLab;

    CheckSMSPort(Context c, String key) {
        super(c, key);

        setTitle(R.string.smsreg_port_title);
        setNote(R.string.smsreg_port_note);
        setProperty(PROPERTY_AUTO_CHECK);
        setProperty(PROPERTY_AUTO_FWD);
    }
    
    public boolean onCheck() {
        String smsPort = DMHelpUtil.querySmsRegProvider(getContext(), DMHelpUtil.SMS_PORT);
        setValue(smsPort);
        mResult = smsPort.equals("16998") ? check_result.RIGHT : check_result.WRONG;
        return true;
    }

    public check_result getCheckResult() {
        
        return mResult;
    }

    public Intent getForwardIntent() {
        return DMHelpUtil.getForwardDMIntent(getContext());
    }
}

class CheckDMServer extends CheckItemBase {

    boolean bLab;

    CheckDMServer(Context c, String key) {
        super(c, key);
        setTitle(R.string.dmsrv_title);
        setProperty(PROPERTY_AUTO_CHECK);
        setProperty(PROPERTY_AUTO_FWD);

        if (key.equals(CheckItemKeySet.CI_DMSERVER_LAB)) {
            bLab = true;
            setNote(R.string.dmsrv_lab);
        } else {
            bLab = false;
            setNote(R.string.dmsrv_cur);
        }
    }
    
    public boolean onCheck() {
        
        String dmServer = DMHelpUtil.queryDMProvider(getContext(), "OMSAcc", DMHelpUtil.DM_SERVER);
        setValue(dmServer);
        if (bLab) {
            mResult = (dmServer.equals("http://218.206.176.97") || dmServer
                    .startsWith("http://218.206.176.97:")) ? check_result.RIGHT
                    : check_result.WRONG;
        } else {
            mResult = (dmServer.equals("http://dm.monternet.com") || dmServer
                    .startsWith("http://dm.monternet.com:")) ? check_result.RIGHT
                    : check_result.WRONG;
        }
        return true;
    }

    public check_result getCheckResult() {

        return mResult;
    }

    public Intent getForwardIntent() {
        return DMHelpUtil.getForwardDMIntent(getContext());
    }

}

class CheckDMManufacturer extends CheckItemBase {

    CheckDMManufacturer(Context c, String key) {
        super(c, key);

        setTitle(R.string.dm_manu_title);
        setNote(R.string.dm_manu_note);
        setProperty(PROPERTY_CLEAR);
    }
    
    public boolean onCheck() {
        //String dmManu = DMHelpUtil.querySmsRegProvider(getContext(), "manufacturer");
        String dmManu = CustomProperties.getString(CustomProperties.MODULE_DM, 
                    CustomProperties.MANUFACTURER, "MTK1");
        setValue(dmManu);
        return true;
    }

    public check_result getCheckResult() {
        check_result result = super.getCheckResult();

        if (mValue.equals("MTK1") && result == check_result.UNKNOWN) {
            mResult = check_result.WRONG;
        }

        return mResult;
    }
}

class CheckDMProduct extends CheckItemBase {

    CheckDMProduct(Context c, String key) {
        super(c, key);

        setTitle(R.string.dm_product_title);
        setNote(R.string.dm_product_note);
        setProperty(PROPERTY_CLEAR);
    }
    
    public boolean onCheck() {
        //String dmManu = DMHelpUtil.querySmsRegProvider(getContext(), "product");
        String dmManu = CustomProperties.getString(CustomProperties.MODULE_DM, 
                CustomProperties.MODEL, "MTK");
        setValue(dmManu);
        return true;
    }

    public check_result getCheckResult() {
        check_result result = super.getCheckResult();
        
        if (mValue.equals("MTK") &&  result == check_result.UNKNOWN) {
            mResult = check_result.WRONG;
        }

        return mResult;
    }

}

class CheckDMVersion extends CheckItemBase {

    CheckDMVersion(Context c, String key) {
        super(c, key);

        setTitle(R.string.dm_version_title);
        setNote(R.string.dm_version_note);
        setProperty(PROPERTY_CLEAR);
    }
    
    public boolean onCheck() {
        String dmVersion = DMHelpUtil.querySmsRegProvider(getContext(), "version");
        setValue(dmVersion);
        return true;
    }

    public check_result getCheckResult() {
        return super.getCheckResult();
    }
}

class CheckRoot extends CheckItemBase {

    private final String PROPERTY_RO_SECURE = "ro.secure";
    private String isUser;

    CheckRoot(Context c, String key) {
        super(c, key);

        setTitle(R.string.root_title);
        setNote(R.string.root_note);
        setProperty(PROPERTY_AUTO_CHECK);
    }
    
    public boolean onCheck() {
        isUser = SystemProperties.get(PROPERTY_RO_SECURE);
        CTSCLog.e("RootCheck", "Current state is:" + isUser);
        if (isUser.equals("1")) {
            SystemProperties.set(PROPERTY_RO_SECURE, "0");
            if (SystemProperties.get(PROPERTY_RO_SECURE).equals("0")) {
                CTSCLog.e("RootCheck", "Try to change state:" + SystemProperties.get(PROPERTY_RO_SECURE));
                SystemProperties.set(PROPERTY_RO_SECURE, "1");
                mResult = check_result.RIGHT;
                setValue(R.string.ctsc_yes);
            } else {
                mResult = check_result.WRONG;
                setValue(R.string.ctsc_no);
            }

        } else {
            mResult = check_result.WRONG;
            setValue(R.string.root_already);
        }
        return true;
    }

    public check_result getCheckResult() {

        return mResult;
    }

}

class CheckIVSR extends CheckItemBase {

    CheckIVSR(Context c, String key) {
        super(c, key);

        setTitle(R.string.ivsr_title);
        setNote(R.string.ivsr_note);
        if (key.equals(CheckItemKeySet.CI_IVSR_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }
    }
    
    public boolean onCheck() {
        long ivsr = Settings.System.getLong(getContext().getContentResolver(),
                Settings.System.IVSR_SETTING,
                Settings.System.IVSR_SETTING_DISABLE);

        if (ivsr == Settings.System.IVSR_SETTING_ENABLE) {
            mResult = check_result.RIGHT;
            setValue(R.string.ctsc_enabled);
        } else {
            mResult = check_result.WRONG;
            setValue(R.string.ctsc_disabled);
        }

        return true;
    }
    
    public check_result getCheckResult() {

        return mResult;
    }

    @Override
    public boolean onReset() {

        return Settings.System.putLong(getContext().getContentResolver(),
                Settings.System.IVSR_SETTING,
                Settings.System.IVSR_SETTING_ENABLE);

    }
}

class CheckWIFISleepPolicy extends CheckItemBase {

    CheckWIFISleepPolicy(Context c, String key) {
        super(c, key);

        if (key.equals(CheckItemKeySet.CI_WIFI_NEVERKEEP_ONLYCHECK)) {
            setTitle(R.string.wifi_sleep_title);
            setNote(R.string.wifi_no_sleep_note);
            setProperty(PROPERTY_AUTO_CHECK);
        } else if (key.equals(CheckItemKeySet.CI_WIFI_NEVERKEEP)) {
            setTitle(R.string.wifi_sleep_title);
            setNote(R.string.wifi_no_sleep_note);
            setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        } else {
        setTitle(R.string.wifi_sleep_title);
        setNote(R.string.wifi_sleep_note);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }
    }

    public boolean onCheck() {

        try {
            int sleep = Settings.Global.getInt(getContext()
                    .getContentResolver(), Settings.Global.WIFI_SLEEP_POLICY);
            
            if (Settings.Global.WIFI_SLEEP_POLICY_NEVER == sleep) {
                setValue(R.string.wifi_sleep_no);
                mResult = getKey().equals(CheckItemKeySet.CI_WIFI_ALWAYKEEP) ?
                        check_result.RIGHT : check_result.WRONG;
            } else if (Settings.Global.WIFI_SLEEP_POLICY_DEFAULT == sleep) {
                setValue(R.string.wifi_sleep_yes);
                mResult = getKey().equals(CheckItemKeySet.CI_WIFI_ALWAYKEEP) ?
                        check_result.WRONG : check_result.RIGHT;
            } else {
                setValue(R.string.wifi_sleep_charge);
                mResult = check_result.WRONG;
            }
            
        } catch (SettingNotFoundException e) {
            e.printStackTrace();
        }
        return true;
    }

    public check_result getCheckResult() {
        
        return mResult;
    }

    @Override
    public boolean onReset() {

        if (getKey().equals(CheckItemKeySet.CI_WIFI_ALWAYKEEP)) {
        return Settings.Global.putInt(getContext().getContentResolver(),
                Settings.Global.WIFI_SLEEP_POLICY,
                Settings.Global.WIFI_SLEEP_POLICY_NEVER);
        } else {
            return Settings.Global.putInt(getContext().getContentResolver(),
                    Settings.Global.WIFI_SLEEP_POLICY,
                    Settings.Global.WIFI_SLEEP_POLICY_DEFAULT);
        }
    }
}


class CheckUA extends CheckItemBase {

    CheckUA(Context c, String key) {
        super(c, key);

        setNote(R.string.ua_note);
        setProperty(PROPERTY_CLEAR);
    }

    private String getUAString(String name, String type) {
        String str = null;
        try {
            str = CustomProperties.getString(name, type);
        } catch (java.lang.NoClassDefFoundError e) {
            e.printStackTrace();
        }
        CTSCLog.e("CheckUA", "name:" + name + "type:" + type + "str:" + str);
        return str;
    }

    public check_result getCheckResult() {

        check_result result = super.getCheckResult();
        String Value;

        if (getKey().equals(CheckItemKeySet.CI_UA_BROWSER)) {
            setValue(getUAString("browser", "UserAgent"));
            setTitle(R.string.browser_ua_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_BROWSERURL)) {
            setValue(getUAString("browser", "UAProfileURL"));
            setTitle(R.string.browser_ua_url_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_MMS)) {
            setValue(getUAString("mms", "UserAgent"));
            setTitle(R.string.mms_ua_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_MMSURL)) {
            setValue(getUAString("mms", "UAProfileURL"));
            setTitle(R.string.mms_ua_url_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_HTTP)) {
            setValue(getUAString("http_streaming", "UserAgent"));
            setTitle(R.string.http_ua_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_HTTPURL)) {
            setValue(getUAString("http_streaming", "UAProfileURL"));
            setTitle(R.string.http_ua_url_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_RTSP)) {
            setValue(getUAString("rtsp_streaming", "UserAgent"));
            setTitle(R.string.rtsp_ua_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_RTSPURL)) {
            setValue(getUAString("rtsp_streaming", "UAProfileURL"));
            setTitle(R.string.rtsp_ua_url_title);
        } else if (getKey().equals(CheckItemKeySet.CI_UA_CMMB)) {
            setValue(getUAString("cmmb", "UserAgent"));
            setTitle(R.string.cmmb_ua_title);
        } else {
            setValue(R.string.ctsc_error);
            setTitle(R.string.ctsc_unknown);
        }

        return result;
    }

}

class CheckGprsMode extends CheckItemBase {

    boolean mCallPref;
    
    CheckGprsMode(Context c, String key) {
        super(c, key);

        setTitle(R.string.gprs_mode_title);
        if (key.equals(CheckItemKeySet.CI_GPRS_CALL_PREF)) {
            setNote(R.string.gprs_call_pref_note);
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setNote(R.string.gprs_data_pref_note);
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
    }
    }
    
    public boolean onCheck() {
        int mode = Settings.System.getInt(getContext().getContentResolver(),
                Settings.System.GPRS_TRANSFER_SETTING,
                Settings.System.GPRS_TRANSFER_SETTING_DEFAULT);

        if (1 == mode) {
            mCallPref = true;
            setValue(R.string.gprs_call_preferred);            
        } else {
            mCallPref = false;
            setValue(R.string.gprs_data_preferred);
        }
        
        if (getKey().equals(CheckItemKeySet.CI_GPRS_CALL_PREF)) {
            mResult = mCallPref ? check_result.RIGHT : check_result.WRONG;
        } else {
            mResult = mCallPref ? check_result.WRONG : check_result.RIGHT;
        }
        return true;
    }

    public check_result getCheckResult() {

        return mResult;
    }

    @Override
    public boolean onReset() {

        return Settings.System.putInt(getContext().getContentResolver(),
                Settings.System.GPRS_TRANSFER_SETTING, 0);

    }
}


class CheckMTKLogger extends CheckItemBase {

    CheckMTKLogger(Context c, String key) {
        super(c, key);

        setProperty(PROPERTY_AUTO_FWD);

        if (key.equals(CheckItemKeySet.CI_LOGGER_ON)) {
            setTitle(R.string.logger_on_title);
            setNote(R.string.logger_on_note);
        } else if (key.equals(CheckItemKeySet.CI_LOGGER_OFF)) {
            setTitle(R.string.logger_off_title);
            setNote(R.string.logger_off_note);
        } else if (key.equals(CheckItemKeySet.CI_TAGLOG_OFF)) {
            setTitle(R.string.taglog_off_title);
            setNote(R.string.taglog_off_note);
        } else {
            throw new IllegalArgumentException("Error key = " + key);
        }
    }

    public Intent getForwardIntent() {

        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.mediatek.mtklogger", "com.mediatek.mtklogger.MainActivity"));
        intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);

        if (getContext().getPackageManager().resolveActivity(intent, 0) != null) {
            return intent;
        } else {
            throw new RuntimeException(
                    "Can't find such activity: com.mediatek.mtklogger, " +
                    "com.mediatek.mtklogger.MainActivity");
        }
    }
}

class CheckCTIA extends CheckItemBase {

    CheckCTIA(Context c, String key) {
        super(c, key);

        setProperty(PROPERTY_AUTO_FWD);
        setTitle(R.string.ctia_title);
        setNote(R.string.ctia_note);
    }


    public Intent getForwardIntent() {

        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.mediatek.engineermode", 
                "com.mediatek.engineermode.wifi.WifiTestSetting"));

        if (getContext().getPackageManager().resolveActivity(intent, 0) != null) {
            return intent;
        } else {
            throw new RuntimeException(
                    "Can't find such activity: com.mediatek.engineermode, " +
                    "com.mediatek.engineermode.wifi.WifiTestSetting");
        }
    }
}

class CheckSN extends CheckItemBase {

    CheckSN(Context c, String key) {
        super(c, key);
        setTitle(R.string.sn_title);
        setNote(R.string.sn_note);
    }
    
    public boolean onCheck() {
        
        StringBuilder sResult = new StringBuilder("");
        InputStream inputstream = null;
        BufferedReader bufferedreader = null;
        
        try {
            Process proc = Runtime.getRuntime().exec("cat /sys/class/android_usb/android0/iSerial");
            inputstream = proc.getInputStream();
            InputStreamReader inputstreamreader = new InputStreamReader(inputstream);
            bufferedreader = new BufferedReader(inputstreamreader);
            
            if (proc.waitFor() == 0) {
                String line;
                while ((line = bufferedreader.readLine()) != null) {
                    CTSCLog.i("SN", "read line " + line);
                    sResult.append(line);
                }
            } else {
                CTSCLog.i("SN", "exit value = " + proc.exitValue() + "|| get:sb-- " + sResult);
            }
                
        } catch (InterruptedException e) {
            CTSCLog.i("SN", "exe fail " + e.toString() + "|| get:sb-- " + sResult);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
             if (null != bufferedreader) {
                 try {
                     bufferedreader.close();
                 } catch (IOException e) {
                    CTSCLog.w("SN", "close IOException: " + e.getMessage());
                }
            }
        }
          
        setValue(sResult.toString());
        return true;
    }

    public check_result getCheckResult() {
        return super.getCheckResult();  
    }    
}

class CheckATCI extends CheckItemBase {

    CheckATCI(Context c, String key) {
        super(c, key);
        
        setProperty(PROPERTY_AUTO_CHECK);
        setTitle(R.string.atci_title);
        setNote(R.string.atci_note);
    }
    
    public boolean onCheck() {
        CTSCLog.i("CheckATCI", "onCheck()");
        boolean hasAtcidProc = false;
        //boolean hasAtciServiceProc = false;
        Process proc = null;
        InputStream inputstream = null;
        InputStreamReader inputstreamreader = null;
        BufferedReader bufferedreader = null;
        
        try {
            proc = Runtime.getRuntime().exec("ps atcid");
            inputstream = proc.getInputStream();
            inputstreamreader = new InputStreamReader(inputstream);
            bufferedreader = new BufferedReader(inputstreamreader);
            if (proc.waitFor() == 0) {
                String line;
                while ((line = bufferedreader.readLine()) != null) {
                    if (line.contains("atcid")) {
                        CTSCLog.i("CheckATCI", "line contains atcid, line: " + line);
                        hasAtcidProc = true; 
                        break;
                    }
                }
            } else {
                CTSCLog.i("CheckATCI", "exit value = " + proc.exitValue());
            }
            
            //proc = Runtime.getRuntime().exec("ps atci_service");
            //inputstream = proc.getInputStream();
            //inputstreamreader = new InputStreamReader(inputstream);
            //bufferedreader = new BufferedReader(inputstreamreader);
            //if (proc.waitFor() == 0) {
                //String line;
                //while ((line = bufferedreader.readLine()) != null) {
                    //if (line.contains("atci_service")) {
                        //CTSCLog.i("CheckATCI", "line contains atci_service, line: " + line);
                        //hasAtciServiceProc = true; 
                        //break;
                    //}
                //}
            //} else {
                //CTSCLog.i("CheckATCI", "exit value = " + proc.exitValue());
            //}
        } catch (InterruptedException e) {
            CTSCLog.i("CheckATCI", "exe fail " + e.toString());
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
             if (null != bufferedreader) {
                 try {
                     bufferedreader.close();
                 } catch (IOException e) {
                    CTSCLog.w("CheckATCI", "close IOException: " + e.getMessage());
                }
            }
        }
        
        //if (hasAtcidProc && hasAtciServiceProc) {
            //CTSCLog.i("CheckATCI", "atcid and atci_service exist!");
        if (hasAtcidProc) {
            setValue(R.string.atci_value_on);
            mResult = check_result.RIGHT;
        } else {
            setValue(R.string.atci_value_off);
            mResult = check_result.WRONG;
        }
        
        return true;
    }

    public check_result getCheckResult() {        
        return mResult;  
    }
    
}

class CheckBattery extends CheckItemBase {

    IntentFilter mIF;
    int mLevel = -1;
    
    private BroadcastReceiver mIR = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action  = intent.getAction();
            if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                int newlevel = intent.getIntExtra("level", 200);
                if (mLevel != newlevel) {
                    mLevel = newlevel;
                    sendBroadcast();
                } else {
                    CTSCLog.i("checkBattery", "no change, return" );
                    return;
                }
                CTSCLog.i("checkBattery", "level = " + mLevel);
                setValue(String.valueOf(mLevel) + "%");
            }
        }

    };
    
    public void finalize() {
        getContext().unregisterReceiver(mIR);
    }
    
    CheckBattery(Context c, String key) {
        super(c, key);
        
        mIF = new IntentFilter();
        mIF.addAction(Intent.ACTION_BATTERY_CHANGED);
        getContext().registerReceiver(mIR, mIF);
        
        setProperty(PROPERTY_AUTO_CHECK);
        setTitle(R.string.battery_title);
        setNote(R.string.battery_note);        
    }
    
    public check_result getCheckResult() {
        
         if (mLevel < 0) {
            setValue(R.string.ctsc_querying);
            return check_result.UNKNOWN;
         } else if (mLevel < 90) {
             return check_result.WRONG;
         } else {
             return check_result.RIGHT;         
         }
    }
    
}

class CheckBuildType extends CheckItemBase {

    private final String PROPERTY_RO_BUILD_TYPE = "ro.build.type";
        
    CheckBuildType(Context c, String key) {
        super(c, key);
        setTitle(R.string.buildtype_title);
        setNote(R.string.buildtype_note);
        setProperty(PROPERTY_AUTO_CHECK);
    }
    
    public boolean onCheck() {
        String type = SystemProperties.get(PROPERTY_RO_BUILD_TYPE);
        setValue(type + " mode");
        mResult = type.equals("user") ? check_result.RIGHT : check_result.WRONG;
        return true;
    }
    
    public check_result getCheckResult() {        
        return mResult;        
    }
}

class CheckTargetMode extends CheckItemBase {
    CheckTargetMode(Context c, String key) {
        super(c, key);
        setTitle(R.string.targetmode_title);
        setNote(R.string.targetmode_note);        
    }
    
    public boolean onCheck() {
        Uri customerServiceUri = Uri.parse("content://com.mediatek.cmcc.provider/phoneinfo");
        StringBuilder sb = new StringBuilder("");
        try {
            Cursor cursor = getContext().getContentResolver().query(customerServiceUri, null, null, null, null);
            while (cursor.moveToNext()) {
                sb.append(cursor.getString(cursor.getColumnIndex("phone_model")));
            }
            cursor.close();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
        setValue(sb.toString());
        return true;
    }
}

class CheckTargetVersion extends CheckItemBase {
    
    private final String PROPERTY_RO_BUILD_VERSION = "ro.build.version.release";
    
    CheckTargetVersion(Context c, String key) {
        super(c, key);
        setTitle(R.string.targetversion_title);
        setNote(R.string.targetversion_note);        
    }
    
    public boolean onCheck() {
        setValue(SystemProperties.get(PROPERTY_RO_BUILD_VERSION));
        return true;
    }
}

class CheckVendorApk extends CheckItemBase {
    
    final private String[] mApkArray = {
        "MobileMarket",
        "MobileVideo",
        "CMCCMobileMusic",
        "PIMClient",
        "GameHall",
        "WLANClient"
    };    
    
    CheckVendorApk(Context c, String key) {
        super(c, key);
        setTitle(R.string.vendor_title);
        setNote(R.string.vendor_note);
        setProperty(PROPERTY_AUTO_CHECK);
    }
    
    public boolean onCheck() {
        int size = mApkArray.length;
        
        setValue(R.string.vendor_value_normal);
        mResult = check_result.RIGHT;
        CTSCLog.i("CheckVendor", String.valueOf(size));
        for (int i = 0; i < size; i++) {
            if (!apkExist(mApkArray[i])) {
                mResult = check_result.WRONG;
                setValue(R.string.vendor_value_abnormal);
                break;
            }
        }
                    
        return true;
    }
    
    private boolean apkExist(String apkName) {
        StringBuilder apkDir = new StringBuilder("system/vendor/operator/app/");
        apkDir.append(apkName);
        apkDir.append(".apk");
        
        CTSCLog.i("CheckVendor", "apkExist:" + apkDir.toString());
        File apkFile = new File(apkDir.toString());
        if (!apkFile.exists()) {
            return false;
        }        
        return true;
    }
}

class CheckFR extends CheckItemBase {

    CheckFR(Context c, String key) {
        super(c, key);
        setTitle(R.string.rf_title);
        setNote(R.string.rf_note);
    }
}

class CheckBaseFunc extends CheckItemBase {
    
    CheckBaseFunc(Context c, String key) {
        super(c, key);
        setTitle(R.string.basefunc_title);
        setNote(R.string.basefunc_note);
    }
}

class CheckRestore extends CheckItemBase {
    CheckRestore(Context c, String key) {
        super(c, key);
        setTitle(R.string.restore_title);
        setNote(R.string.restore_note);
    }
}

//Yongmao
class CheckCalData extends CheckItemBase {

    CheckCalData(Context c, String key) {
        super(c, key);
        setTitle(R.string.title_calibration_data);
        setNote(R.string.note_calibration_data);
        setProperty(PROPERTY_CLEAR);
    }
}

class CheckApkExist extends CheckItemBase {

    CheckApkExist(Context c, String key) {
        super(c, key);
        setProperty(PROPERTY_CLEAR);
        
        if (key.equals(CheckItemKeySet.CI_APK_CALCULATOR)) {
            setTitle(R.string.title_apk_calculator);
        } else if (key.equals(CheckItemKeySet.CI_APK_STOPWATCH)){
            setTitle(R.string.title_apk_stopwatch);
        } else if (key.equals(CheckItemKeySet.CI_APK_NOTEBOOK)){
            setTitle(R.string.title_apk_notebook);
        } else if (key.equals(CheckItemKeySet.CI_APK_BACKUP)){
            setTitle(R.string.title_apk_backup);
        } else if (key.equals(CheckItemKeySet.CI_APK_CALENDAR)){
            setTitle(R.string.title_apk_calendar);
        } else if (key.equals(CheckItemKeySet.CI_APK_OFFICE)){
            setTitle(R.string.title_apk_office);
            setNote(R.string.note_apk_office);
        } else {
            throw new IllegalArgumentException("Error key = " + key);
        }
    }

    public check_result getCheckResult() {
        CTSCLog.i("CheckApkExist", "CheckApkExist");
        
        check_result result = super.getCheckResult();
        if (getKey().equals(CheckItemKeySet.CI_APK_OFFICE)) {
            return result;
        }
        if (check_result.UNKNOWN == result) {
            if (getKey().equals(CheckItemKeySet.CI_APK_CALCULATOR)) {
                if (apkExist(R.string.apk_calculator_name)) {
                    setProperty(PROPERTY_AUTO_CHECK);
                    setNote(R.string.note_apk_calculator_exist);
                    setValue(R.string.value_apk_calculator_exist);
                    result = check_result.RIGHT;
                } else {
                    setNote(R.string.note_mtk_calculator_not_exist);
                }
            } else if (getKey().equals(CheckItemKeySet.CI_APK_STOPWATCH)){
                boolean isJB2 = (Build.VERSION.SDK_INT >= 17);
                if (isJB2) {
                    if (apkExist(R.string.apk_deskclock_name)) {
                        setProperty(PROPERTY_AUTO_CHECK);
                        setNote(R.string.note_apk_stopwatch_exist);
                        setValue(R.string.value_apk_deskclock_exist);
                        result = check_result.RIGHT;
                    } else {
                        setNote(R.string.note_deskclock_not_exist);
                    }
                } else {
                    if (apkExist(R.string.apk_stopwatch_name)) {
                        setProperty(PROPERTY_AUTO_CHECK);
                        setNote(R.string.note_apk_stopwatch_exist);
                        setValue(R.string.value_apk_stopwatch_exist);
                        result = check_result.RIGHT;
                    } else {
                        setNote(R.string.note_mtk_stopwatch_not_exist);
                    }
                }
            } else if (getKey().equals(CheckItemKeySet.CI_APK_NOTEBOOK)) {
                if (apkExist(R.string.apk_notebook_name)) {
                    setProperty(PROPERTY_AUTO_CHECK);
                    setNote(R.string.note_apk_notebook_exist);
                    setValue(R.string.value_apk_notebook_exist);
                    result = check_result.RIGHT;
                } else {
                    setNote(R.string.note_mtk_notebook_not_exist);
                }
            } else if (getKey().equals(CheckItemKeySet.CI_APK_BACKUP)) {
                if (apkExist(R.string.apk_backup_name)) {
                    setProperty(PROPERTY_AUTO_CHECK);
                    setNote(R.string.note_apk_backup_exist);
                    setValue(R.string.value_apk_backup_exist);
                    result = check_result.RIGHT;
                } else {
                    setNote(R.string.note_mtk_backup_not_exist);
                }
            } else if (getKey().equals(CheckItemKeySet.CI_APK_CALENDAR)) {
                if (apkExist(R.string.apk_calendar_name)) {
                    setProperty(PROPERTY_AUTO_CHECK);
                    setNote(R.string.note_apk_calendar_exist);
                    setValue(R.string.value_apk_calendar_exist);
                    result = check_result.RIGHT;
                } else {
                    setNote(R.string.note_calendar_not_exist);
                }
            } else {
                throw new IllegalArgumentException("Error key = " + getKey());
            }
        } else {
            if (getKey().equals(CheckItemKeySet.CI_APK_CALCULATOR)) {
                setNote(R.string.note_apk_calculator_exist);
            } else if (getKey().equals(CheckItemKeySet.CI_APK_STOPWATCH)){
                setNote(R.string.note_apk_stopwatch_exist);
            } else if (getKey().equals(CheckItemKeySet.CI_APK_NOTEBOOK)){
                setNote(R.string.note_apk_notebook_exist);
            } else if (getKey().equals(CheckItemKeySet.CI_APK_BACKUP)){
                setNote(R.string.note_apk_backup_exist);
            } else if (getKey().equals(CheckItemKeySet.CI_APK_CALENDAR)){
                setNote(R.string.note_apk_calendar_exist);
            } else {
                throw new IllegalArgumentException("Error key = " + getKey());
            }
        }
        
        return result;
    }
    
    private boolean apkExist(int apkNameId) {
        StringBuilder apkDir = new StringBuilder("system/app/");
        apkDir.append(getContext().getString(apkNameId));
        apkDir.append(".apk");
        
        File apkFile = new File(apkDir.toString());
        if (!apkFile.exists()) {
            return false;
        }
        
        return true;
    }
}

class Check24hChargeProtect extends CheckItemBase {

    Check24hChargeProtect(Context c, String key) {
        super(c, key);
        setTitle(R.string.title_24h_protect);
        setNote(R.string.note_24h_disable);
        setProperty(PROPERTY_CLEAR);
    }
}

class CheckHSUPA extends CheckItemBase {
    private static final String TAG = "CheckHSUPA";
    private boolean mAsyncDone = true;
    private boolean mNeedNofity = false;
    
    private static final int EVENT_HSPA_INFO = 1;
    private static final String QUERY_CMD = "AT+EHSM?";
    private static final String RESPONSE_CMD = "+EHSM: ";

    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    
    private static final int MODE_HSUPA_OFF_1 = 0;
    private static final int MODE_HSUPA_OFF_2 = 1;
    private static final int MODE_HSUPA_ON_1 = 2;
    private static final int MODE_HSUPA_ON_2 = 3;

    private Handler mATCmdHander = new Handler() {
        public void handleMessage(Message msg) {
            CTSCLog.d(TAG, "Receive msg from HSUPA info query");
            if (msg.what == EVENT_HSPA_INFO) {
                mAsyncDone = true;
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar != null && ar.exception == null) {
                    handleQuery((String[]) ar.result);
                } else {
                    setValue("Query failed");
                }
            }
            
            if (mNeedNofity) {
                sendBroadcast();
            }
        }
    };
    
    private void handleQuery(String[] result) {
        if (null == result || result.length <= 0) {
            setValue("Query failed");
            return;
        }
        
        CTSCLog.d(TAG, "Modem return: " + result[0]);
        String[] mode = result[0].substring(RESPONSE_CMD.length(), result[0].length()).split(",");
        if (null == mode || mode.length <= 0) {
            setValue("Query failed");
            return;
        }
        
        int modeInt = -1;
        try {
            modeInt = Integer.parseInt(mode[0]);
        } catch (NumberFormatException e) {
            CTSCLog.d(TAG, "Modem return invalid mode: " + mode[0]);
            return;
        }
        switch (modeInt) {
        case MODE_HSUPA_OFF_1:
        case MODE_HSUPA_OFF_2:
            setValue(R.string.value_hsupa_off);
            mResult = check_result.WRONG;
            break;
            
        case MODE_HSUPA_ON_1:
        case MODE_HSUPA_ON_2:
            setValue(R.string.value_hsupa_on);
            mResult = check_result.RIGHT;
            break;

        default:
            setValue("Query failed");
            break;
        }

    }

    CheckHSUPA(Context c, String key) {
        super(c, key);
        setTitle(R.string.hsupa_title);
        setNote(R.string.hsupa_note);
        setProperty(PROPERTY_AUTO_CHECK); 
    }

    public boolean onCheck() {
        getHSUPAInfo();
        return true;
    }
   
    public check_result getCheckResult() {
        if (!mAsyncDone) {
            mResult = check_result.UNKNOWN;
            mNeedNofity = true;
            setValue(R.string.ctsc_querying);
        } else {
            mNeedNofity = false;
        }
        CTSCLog.d(TAG, "getCheckResult mResult = " + mResult);       
        return mResult;       
    }
   
    private void getHSUPAInfo() {
        CTSCLog.i(TAG, "getHSUPAInfo");
        mAsyncDone = false;
        String[] cmd = new String[2];
        cmd[0] = QUERY_CMD;
        cmd[1] = RESPONSE_CMD;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mGeminiPhone.invokeOemRilRequestStringsGemini(cmd,
                    mATCmdHander.obtainMessage(EVENT_HSPA_INFO), PhoneConstants.GEMINI_SIM_1);
        } else {
            mPhone = (Phone) PhoneFactory.getDefaultPhone();
            mPhone.invokeOemRilRequestStrings(cmd, mATCmdHander.obtainMessage(EVENT_HSPA_INFO));
        }
    }  
}

class CheckWlanSSID extends CheckItemBase {

    CheckWlanSSID(Context c, String key) {
        super(c, key);
        setTitle(R.string.title_wlan_ssid);
        setNote(R.string.note_wlan_ssid);
        setProperty(PROPERTY_CLEAR);
    }
    
    public check_result getCheckResult() {
        check_result result = super.getCheckResult();
        
        WifiManager wifi = (WifiManager) getContext().getSystemService(Context.WIFI_SERVICE);
        WifiConfiguration config = wifi.getWifiApConfiguration();
        String ssid = config == null ? null : config.SSID;
        
        if (ssid == null && !wifi.isWifiEnabled()) {
            setValue(R.string.value_please_open_wlan);
        } else {
            setValue(ssid);
        }

        return result;   
    }
}

class CheckWlanMacAddr extends CheckItemBase {

    CheckWlanMacAddr(Context c, String key) {
        super(c, key);
        setTitle(R.string.title_wlan_mac_addr);
        setNote(R.string.note_wlan_mac_addr);
        setProperty(PROPERTY_CLEAR);
    }
    
    public check_result getCheckResult() {
        check_result result = super.getCheckResult();
        
        WifiManager wifi = (WifiManager) getContext().getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifi.getConnectionInfo(); 
        String macAddress = wifiInfo == null ? null : wifiInfo.getMacAddress();
        
        if (macAddress == null && !wifi.isWifiEnabled()) {
            setValue(R.string.value_please_open_wlan);
        } else {
            setValue(macAddress);
        }

        return result;   
    }
}
