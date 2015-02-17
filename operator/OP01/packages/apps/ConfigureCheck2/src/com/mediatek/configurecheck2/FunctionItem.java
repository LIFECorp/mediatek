package com.mediatek.configurecheck2;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import android.telephony.TelephonyManager;

//APN
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.os.Handler;
import android.os.Bundle;
import android.os.Message;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import java.util.*;
import android.os.SystemProperties;
import com.android.internal.telephony.TelephonyProperties;

// SUPL
import com.mediatek.common.agps.*;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

// BT
import android.bluetooth.BluetoothAdapter;

// MMS Roaming
import android.content.ComponentName;
import android.content.Intent;

class CheckLabAPN extends CheckItemBase {
    private List<SimInfoRecord> mSimInfoList = null;
    private Set<Integer> mSimNeedSetAPN = new HashSet<Integer>();

    private final static int CHECK_BASE = 0x000;
    private final static int CHECK_MMS = 0x001;
    private final static int CHECK_TYPE = 0x002;

    private final static String TAG = "CheckLabAPN";

    private int mCheckProperty = CHECK_BASE;
        
    CheckLabAPN (Context c, String key) {
        super(c, key);

        setTitle(R.string.apn_check_title);

        StringBuilder noteStr = new StringBuilder();
        noteStr.append(getContext().getString(R.string.apn_name) + "\n");
        noteStr.append(getContext().getString(R.string.apn_apn_lab) + "labwap3" + "\n");
        noteStr.append(getContext().getString(R.string.apn_proxy_lab) + "\n");
        noteStr.append(getContext().getString(R.string.apn_port_lab) + "\n");
        if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)) {
            mCheckProperty = mCheckProperty | CHECK_MMS;
            noteStr.append(getContext().getString(R.string.apn_mms) + "\n");
            noteStr.append(getContext().getString(R.string.apn_mms_proxy) + "\n");
            noteStr.append(getContext().getString(R.string.apn_mms_port) + "\n");
        } else if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_TYPE)) {
            mCheckProperty = mCheckProperty | CHECK_TYPE;
            noteStr.append(getContext().getString(R.string.apn_type) + "\n");
        } else if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
            mCheckProperty = mCheckProperty | CHECK_MMS;
            mCheckProperty = mCheckProperty | CHECK_TYPE;
            noteStr.append(getContext().getString(R.string.apn_mms) + "\n");
            noteStr.append(getContext().getString(R.string.apn_mms_proxy) + "\n");
            noteStr.append(getContext().getString(R.string.apn_mms_port) + "\n");
            noteStr.append(getContext().getString(R.string.apn_type) + "\n");
        } else {
            mCheckProperty = CHECK_BASE;
        }

        if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)) {
            noteStr.append(getContext().getString(R.string.agps_doc) + "\n");
            noteStr.append(getContext().getString(R.string.card_doc));
        } else if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
            noteStr.append(getContext().getString(R.string.iot_doc));
        }

        setNote(noteStr.toString());
        setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        mSimInfoList = SimInfoManager.getInsertedSimInfoList(c);

        if (mSimInfoList.isEmpty()) {
            setProperty(PROPERTY_AUTO_CHECK);
        }
    }
    
    public check_result getCheckResult() {
        if (!isCheckable()) {
            return super.getCheckResult();
        }

        if (mSimInfoList.isEmpty()) {
            setValue(R.string.string_sim);
            return check_result.UNKNOWN;
        }

        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            StringBuilder valueStr = new StringBuilder();
            check_result result = check_result.RIGHT;
            
            for (SimInfoRecord simInfo : mSimInfoList) {
                int simNo = simInfo.mSimSlotId + 1;
                Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                            null, null, null, null);

                if (cursor_APN == null || !cursor_APN.moveToNext()) {
                    if (cursor_APN != null) {
                        cursor_APN.close();
                    }
                    valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_setted));
                    result = check_result.WRONG;
                    mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                } else {
                    String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));
                    String curAPNProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PROXY));
                    String curAPNPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PORT));
                    String curAPNMMS = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSC));
                    String curAPNType = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.TYPE));
                    String curAPNMMSProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPROXY));
                    String curAPNMMSPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPORT));

                    cursor_APN.close();
                    
                    if (curAPN.equalsIgnoreCase("labwap3") && curAPNProxy.equalsIgnoreCase("192.168.230.8") &&
                                curAPNPort.equalsIgnoreCase("9028")) {
                        if ((mCheckProperty & CHECK_MMS) != 0) {
                            if (false == curAPNMMS.equalsIgnoreCase("http://218.206.176.175:8181/was") ||
                                false == curAPNMMSProxy.equalsIgnoreCase("192.168.230.8") ||
                                false == curAPNMMSPort.equalsIgnoreCase("9028") ) {
                                result = check_result.WRONG;
                                mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                                if(valueStr.toString().length() != 0) {
                                    valueStr.append("\n");
                                }
                                valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                            }
                        }

                        if ((mCheckProperty & CHECK_TYPE) != 0) {
                            if (null == curAPNType) {
                                result = check_result.WRONG;
                                mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                                if(valueStr.toString().length() != 0) {
                                    valueStr.append("\n");
                                }
                                valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                            } else if (false == curAPNType.equals("default,mms,net")) {
                                result = check_result.WRONG;
                                mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                                if(valueStr.toString().length() != 0) {
                                    valueStr.append("\n");
                                }
                                valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                            }
                        }
                    } else {
                        result = check_result.WRONG;
                        mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                        if(valueStr.toString().length() != 0) {
                            valueStr.append("\n");
                        }
                        valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                    }
                }
            }

            setValue(valueStr.toString());
            return result;
        } else {
            Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers/preferapn"),
                                            null, null, null, null);
            if (cursor_APN == null || !cursor_APN.moveToNext()) {
                if (cursor_APN != null) {
                    cursor_APN.close();
                }
            
                setValue(R.string.apn_not_setted);
                return check_result.WRONG;
            } else {
                String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));
                String curAPNProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PROXY));
                String curAPNPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PORT));
                String curAPNMMS = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSC));
                String curAPNType = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.TYPE));
                String curAPNMMSProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPROXY));
                String curAPNMMSPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPORT));
                    
                cursor_APN.close();

                if (curAPN.equalsIgnoreCase("labwap3") && curAPNProxy.equalsIgnoreCase("192.168.230.8") &&
                                curAPNPort.equalsIgnoreCase("9028")) {
                    if ((mCheckProperty & CHECK_MMS) != 0) {
                        if (false == curAPNMMS.equalsIgnoreCase("http://218.206.176.175:8181/was") ||
                                false == curAPNMMSProxy.equalsIgnoreCase("192.168.230.8") ||
                                false == curAPNMMSPort.equalsIgnoreCase("9028") ){
                            setValue(R.string.apn_not_correct);
                            return check_result.WRONG;
                        }
                    }

                    if ((mCheckProperty & CHECK_TYPE) != 0) {
                        if (null == curAPNType) {
                            setValue(R.string.apn_not_correct);
                            return check_result.WRONG;
                        } else if (false == curAPNType.equals("default,mms,net")) {
                            setValue(R.string.apn_not_correct);
                            return check_result.WRONG;
                        }
                    }

                    setValue("");
                    return check_result.RIGHT;
                } else {
                    return check_result.WRONG;
                }
            }
        }

    }    

    public boolean onReset() {
        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            for (Integer simId : mSimNeedSetAPN) {
                String where = "apn=\"labwap3\"";

                CTSCLog.v(TAG, "where = " + where);
                int simNo = simId.intValue() + 1;
                Uri uri= Uri.parse("content://telephony/carriers_sim" + simNo);
                Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "numeric"}, where, null, null);

                APNBuilder apnBuilder = new APNBuilder(getContext(), simId.intValue(), "labwap3", "labwap3");
                
                if ((cursor != null) && (true == cursor.moveToFirst())) {
                    cursor.moveToFirst();
                    int index = cursor.getColumnIndex("_id");
                    String apnId = cursor.getString(index);

                    ContentValues values = new ContentValues();
                    values.put(Telephony.Carriers.MCC, apnBuilder.getMCC());
                    values.put(Telephony.Carriers.MNC, apnBuilder.getMNC());
                    values.put(Telephony.Carriers.NUMERIC, apnBuilder.getSimOperator());

                    values.put(Telephony.Carriers.NAME, "labwap3");
                    values.put(Telephony.Carriers.PROXY, "192.168.230.8");
                    values.put(Telephony.Carriers.PORT, "9028");

                    if ((mCheckProperty & CHECK_MMS) != 0) {
                        values.put(Telephony.Carriers.MMSC, "http://218.206.176.175:8181/was");
                        values.put(Telephony.Carriers.MMSPROXY, "192.168.230.8");
                        values.put(Telephony.Carriers.MMSPORT, "9028");
                    }

                    if ((mCheckProperty & CHECK_TYPE) != 0) {
                        values.put(Telephony.Carriers.TYPE, "default,mms,net");
                    }

                    String whereUpdate = "_id=\"" + apnId + "\"";

                    getContext().getContentResolver().update(
                                Uri.parse("content://telephony/carriers_sim" + simNo),
                                values, 
                                whereUpdate, 
                                null);
                    
                    ContentValues values_prefer = new ContentValues();
                    values_prefer.put("apn_id", apnId);

                    getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                    values_prefer, 
                                    null, 
                                    null);

                } else {
                    apnBuilder.setProxy("192.168.230.8")
                              .setPort("9028");
                    if ((mCheckProperty & CHECK_MMS) != 0) {
                        apnBuilder.setMMSC("http://218.206.176.175:8181/was");
                        apnBuilder.setMMSProxy("192.168.230.8");
                        apnBuilder.setMMSPort("9028");
                    }

                    if ((mCheckProperty & CHECK_TYPE) != 0) {
                        apnBuilder.setApnType("default,mms,net");
                    }
                              
                    apnBuilder.build()
                              .setAsCurrent();
                }

                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            String where = "apn=\"labwap3\"";
            Uri uri = Uri.parse("content://telephony/carriers");
            Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "numeric"}, where, null, null);

            APNBuilder apnBuilder = new APNBuilder(getContext(), "labwap3", "labwap3");

            if ((cursor != null) && (true == cursor.moveToFirst())) {
                cursor.moveToFirst();

                int index = cursor.getColumnIndex("_id");
                String apnId = cursor.getString(index);

                ContentValues values = new ContentValues();
                values.put(Telephony.Carriers.MCC, apnBuilder.getMCC());
                values.put(Telephony.Carriers.MNC, apnBuilder.getMNC());
                values.put(Telephony.Carriers.NUMERIC, apnBuilder.getSimOperator());

                values.put(Telephony.Carriers.NAME, "labwap3");
                values.put(Telephony.Carriers.PROXY, "192.168.230.8");
                values.put(Telephony.Carriers.PORT, "9028");

                if ((mCheckProperty & CHECK_MMS) != 0) {
                    values.put(Telephony.Carriers.MMSC, "http://218.206.176.175:8181/was");
                    values.put(Telephony.Carriers.MMSPROXY, "192.168.230.8");
                    values.put(Telephony.Carriers.MMSPORT, "9028");
                }

                if ((mCheckProperty & CHECK_TYPE) != 0) {
                    values.put(Telephony.Carriers.TYPE, "default,mms,net");
                }

                String whereUpdate = "_id=\"" + apnId + "\"";

                getContext().getContentResolver().update(
                                    uri,
                                    values, 
                                    whereUpdate, 
                                    null);

                ContentValues values_prefer = new ContentValues();
                values_prefer.put("apn_id", apnId);

                getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers/preferapn"),
                                    values_prefer, 
                                    null, 
                                    null);

                cursor.close();

            } else {            
                if (cursor != null) {
                    cursor.close();
                }
                apnBuilder.setProxy("192.168.230.8")
                          .setPort("9028");
                if ((mCheckProperty & CHECK_MMS) != 0) {
                    apnBuilder.setMMSC("http://218.206.176.175:8181/was");
                    apnBuilder.setMMSProxy("192.168.230.8");
                    apnBuilder.setMMSPort("9028");
                }

                if ((mCheckProperty & CHECK_TYPE) != 0) {
                    apnBuilder.setApnType("default,mms,net");
                }
                              
                apnBuilder.build()
                          .setAsCurrent();
            }
        }

        return true;
    }

}

class CheckSUPL extends CheckItemBase {
    
    CheckSUPL (Context c, String key) {
        super(c, key);

        setTitle(R.string.supl_check_title);

        if (key.equals(CheckItemKeySet.CI_SUPL_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }

        StringBuilder suplStr = new StringBuilder();
        suplStr.append(getContext().getString(R.string.supl_address) + "\n");
        suplStr.append(getContext().getString(R.string.supl_port) + "\n");
        suplStr.append(getContext().getString(R.string.supl_tls) + "\n");
        suplStr.append(getContext().getString(R.string.supl_net_request) + "\n");
        suplStr.append(getContext().getString(R.string.supl_log_to_file) + "\n");
        suplStr.append(getContext().getString(R.string.agps_doc));

        setNote(suplStr.toString());
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }
    
    public check_result getCheckResult() {
        
        MtkAgpsProfileManager profileMgr = new MtkAgpsProfileManager();
        profileMgr.updateAgpsProfile("/etc/agps_profiles_conf.xml");
        
        MtkAgpsManager agpsMgr = (MtkAgpsManager)getContext().getSystemService(Context.MTK_AGPS_SERVICE);

        if(agpsMgr == null) {
            return check_result.UNKNOWN;
        }

        MtkAgpsConfig config = agpsMgr.getConfig();
        if (1 != config.niEnable || 1 != config.supl2file) {
            return check_result.WRONG;
        }

        MtkAgpsProfile profile = agpsMgr.getProfile();
        
        if (profile.addr.equalsIgnoreCase("218.206.176.50") &&  
            profile.port == 7275 &&
            profile.tls == 1) {
            return check_result.RIGHT;
        } else {
            return check_result.WRONG;
        }
        
    }    
    
    private boolean mHasReset = false;
    public boolean onReset() {
        mHasReset = true;
        
        return setSUPL(getContext());
    }

    private boolean setSUPL(Context context) {
        MtkAgpsProfileManager profileMgr = new MtkAgpsProfileManager();
        profileMgr.updateAgpsProfile("/etc/agps_profiles_conf.xml");
        
        MtkAgpsManager agpsMgr = (MtkAgpsManager)context.getSystemService(Context.MTK_AGPS_SERVICE);

        if(agpsMgr == null) {
            return false;
        }

        // set net work request
        MtkAgpsConfig config = agpsMgr.getConfig();
        config.niEnable = 1;
        config.supl2file = 1;
        agpsMgr.setConfig(config);

        // look for cmcc lab profile
        for (MtkAgpsProfile profile: profileMgr.getAllProfile()) {
            if(profile.name.equalsIgnoreCase("Lab - CMCC")) {
                agpsMgr.setProfile(profile);
                return true;
            }
        }

        MtkAgpsProfile profile = agpsMgr.getProfile();
        profile.addr = "218.206.176.50";
        profile.port = 7275;
        profile.tls = 1;
        agpsMgr.setProfile(profile);

        return true;
    }

}


class APNBuilder {
    private Context mContext = null;
    private APN mAPN = null;
    private Uri mUri = null;
    private int mSlot = -1;

    public APNBuilder(Context context, String apnName, String apn) {
        mContext = context;

        mAPN = new APN();
        mAPN.name = apnName;
        mAPN.apn = apn;
    }

    public APNBuilder(Context context, int slotId, String apnName, String apn) {
        mContext = context;

        mAPN = new APN();
        mAPN.name = apnName;
        mAPN.apn = apn;

        mSlot = slotId;
    }

    // for gemini
    public APNBuilder build() {
        ContentValues values = new ContentValues();
        setParams(values);

        if(mSlot == -1) {
            mUri = mContext.getContentResolver().insert(
                                 Uri.parse("content://telephony/carriers"),
                                 values
                                 );
        } else {
            int simNo = mSlot + 1;
            mUri = mContext.getContentResolver().insert(
                                     Uri.parse("content://telephony/carriers_sim" + simNo),
                                     values
                                     );
        }

        return this;
    }

    private void setParams(ContentValues values) {
        values.put(Telephony.Carriers.NAME, mAPN.name);
        values.put(Telephony.Carriers.APN, mAPN.apn);
        
        values.put(Telephony.Carriers.MCC, getMCC());
        values.put(Telephony.Carriers.MNC, getMNC());
        values.put(Telephony.Carriers.NUMERIC, getSimOperator());

        if(mAPN.proxy != null) {
            values.put(Telephony.Carriers.PROXY, mAPN.proxy);
        }

        if(mAPN.port != null) {
            values.put(Telephony.Carriers.PORT, mAPN.port);
        }

        if(mAPN.uName != null) {
            values.put(Telephony.Carriers.USER, mAPN.uName);
        }

        if(mAPN.pwd != null) {
            values.put(Telephony.Carriers.PASSWORD, mAPN.pwd);
        }

        if(mAPN.server != null) {
            values.put(Telephony.Carriers.SERVER, mAPN.server);
        }

        if(mAPN.mmsc != null) {
            values.put(Telephony.Carriers.MMSC, mAPN.mmsc);
        }

        if(mAPN.mmsproxy != null) {
            values.put(Telephony.Carriers.MMSPROXY, mAPN.mmsproxy);
        }

        if(mAPN.mmsport != null) {
            values.put(Telephony.Carriers.MMSPORT, mAPN.mmsport);
        }

        // TODO: check if type is String
        if(mAPN.authtype != null) {
            values.put(Telephony.Carriers.AUTH_TYPE, mAPN.authtype);
        }

        if(mAPN.apntype != null) {
            values.put(Telephony.Carriers.TYPE, mAPN.apntype);
        }
    }

    public void setAsCurrent() {
        if (mUri == null) {
            return;
        }
        Cursor cur = mContext.getContentResolver().query(mUri, null, null, null, null);
        cur.moveToFirst();
        int index = cur.getColumnIndex("_id");
        String apnId = cur.getString(index);
        cur.close();

        ContentValues values = new ContentValues();
        values.put("apn_id", apnId);
        if (mSlot == -1) {
            mContext.getContentResolver().update(
                                Uri.parse("content://telephony/carriers/preferapn"), 
                                values, 
                                null, 
                                null);
        } else {
            int simNo = mSlot + 1;
            mContext.getContentResolver().update(
                                Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                values, 
                                null, 
                                null);
        }
    }

    public APNBuilder setProxy(String proxy) {
        mAPN.proxy = proxy;

        return this;
    }

    public APNBuilder setPort(String port) {
        mAPN.port = port;

        return this;
    }

    public APNBuilder setUserName(String name) {
        mAPN.uName = name;

        return this;
    }

    public APNBuilder setPassword(String pwd) {
        mAPN.pwd = pwd;

        return this;
    }

    public APNBuilder setServer(String server) {
        mAPN.server = server;

        return this;
    }

    public APNBuilder setMMSC(String mmsc) {
        mAPN.mmsc = mmsc;

        return this;
    }

    public APNBuilder setMMSProxy(String mmsproxy) {
        mAPN.mmsproxy = mmsproxy;

        return this;
    }

    public APNBuilder setMMSPort(String mmsport) {
        mAPN.mmsport = mmsport;

        return this;
    }

    public APNBuilder setAuthType(String authtype) {
        mAPN.authtype = authtype;

        return this;
    }

    public APNBuilder setApnType(String apntype) {
        mAPN.apntype = apntype;

        return this;
    }

    String getMCC() {
        return getSimOperator().substring(0,3);
    }

    String getMNC() {
        return getSimOperator().substring(3);
    }

    String getSimOperator() {
        String oprator = null;
        
        switch(mSlot) {
            case -1:
            case 0:
                oprator = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
                break;
            case 1:
                oprator = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
                break;
            default:
                break;
        }

        return oprator;
    }

    class APN {
        String name;
        String apn;
        String proxy;
        String port;
        String uName;
        String pwd;
        String server;
        String mmsc;
        String mmsproxy;
        String mmsport;
        String mcc;
        String mnc;
        String authtype;
        String apntype;
    }
}

class CheckSMSC extends CheckItemBase {
    private boolean mSyncDone = false;
    private boolean mNeedNotify = false;

    private List<SimInfoRecord> mSimInfoList = null;

    private static final int MSG_GET_SMSC_DONE = 1;
    private static final int MSG_SET_SMSC_DONE = 2;
    
    private final Handler mSMSCHandler = new Handler() {
        public final void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_GET_SMSC_DONE:
                    Bundle bd = (Bundle)msg.obj;
                    if(FeatureOption.MTK_GEMINI_SUPPORT == true) {

                        mResult = check_result.RIGHT;
                        StringBuilder smscStr = new StringBuilder();
                        for (SimInfoRecord simInfo : mSimInfoList) {
                            String curStr = bd.getString("SMSC" + simInfo.mSimSlotId);

                            CTSCLog.v("CheckSMSC", "SMSC" + simInfo.mSimSlotId + " " + curStr);
                            if (false == curStr.equalsIgnoreCase("+8613800100569")) {
                                if(smscStr.toString().length() != 0) {
                                    smscStr.append("\n");
                                }
                                smscStr.append("Sim" + (simInfo.mSimSlotId+1) + getContext().getString(R.string.smsc_not_correct));
                                mResult = check_result.WRONG;
                            }
                        }
                        setValue(smscStr.toString());
                        
                    } else {
                        String smscStr = bd.getString("SMSC");
                    
                        if (smscStr.equalsIgnoreCase("+8613800100569")) {
                            setValue("");
                            mResult = check_result.RIGHT;
                        } else {
                            setValue(R.string.smsc_not_correct);
                            mResult = check_result.WRONG;
                        }

                    }
                    
                    mSyncDone = true;
                    if(true == mNeedNotify) {
                        sendBroadcast();
                    }
                    break;
                case MSG_SET_SMSC_DONE:
                    CTSCLog.v("SMSC", " set done");
                    sendBroadcast();
                    break;
                default:
                    return;
            }
        }
    };
    
    CheckSMSC(Context c, String key) {
        super(c, key);

        setTitle(R.string.smsc_check_title);

        if (key.equals(CheckItemKeySet.CI_SMSC_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }

        mSimInfoList = SimInfoManager.getInsertedSimInfoList(c);

        if (mSimInfoList.isEmpty()) {
            setProperty(PROPERTY_AUTO_CHECK);
        }

        StringBuilder smscNote = new StringBuilder();
        smscNote.append(getContext().getString(R.string.smsc_number));
        smscNote.append("\n" + getContext().getString(R.string.agps_doc));
        
        setNote(smscNote.toString());
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }

    public boolean onCheck() {
        if (mSimInfoList.isEmpty()) {
                setValue(R.string.string_sim);
                return false;
        }
        
        new Thread(new Runnable() {
                public void run() {
                    TelephonyManagerEx telephonyMgr = TelephonyManagerEx.getDefault();
                            
                    if(FeatureOption.MTK_GEMINI_SUPPORT == true) {
                        Bundle bd = new Bundle();
                            
                        for (SimInfoRecord simInfo : mSimInfoList) {
                            int slotId = simInfo.mSimSlotId;
                            String smscStr = telephonyMgr.getScAddress(slotId);
                            bd.putString("SMSC" + slotId, smscStr);

                            CTSCLog.v("CheckSMSC", "Get Result SMSC" + slotId + " " + smscStr);
                        }
                        
                        mSMSCHandler.sendMessage(mSMSCHandler.obtainMessage(MSG_GET_SMSC_DONE, 0, 0, bd));
                    } else {
                        String smscStr = telephonyMgr.getScAddress(0);
                        Bundle bd = new Bundle();
                        bd.putString("SMSC", smscStr);
                        mSMSCHandler.sendMessage(mSMSCHandler.obtainMessage(MSG_GET_SMSC_DONE, 0, 0, bd));
                    }
                }
            }).start();
        mSyncDone = false;

        return super.onCheck();
    }
    
    public check_result getCheckResult() {
        
        if (!isCheckable()) {
            return super.getCheckResult();
        }

        if (mSimInfoList.isEmpty()) {
                setValue(R.string.string_sim);
                return check_result.UNKNOWN;
        }

        if (mSyncDone == false) {
            mResult = check_result.UNKNOWN;
            mNeedNotify = true;
            setValue(R.string.string_checking);
            return mResult;
        }

        mNeedNotify = false;
        return mResult;
    }    

    public boolean onReset() {
        if(!isCheckable()) {
            return false;
        }

        new Thread(new Runnable() {
            public void run() {
                TelephonyManagerEx telephonyMgr = TelephonyManagerEx.getDefault();
        
                if(FeatureOption.MTK_GEMINI_SUPPORT == true) {
                    for (SimInfoRecord simInfo : mSimInfoList) {
                        telephonyMgr.setScAddress("+8613800100569", simInfo.mSimSlotId);
                    }
                } else {
                    telephonyMgr.setScAddress("+8613800100569", 0);
                }
                
                mSMSCHandler.sendEmptyMessage(MSG_SET_SMSC_DONE);
            }
        }).start();
        
        return true;
    }
}

class CheckCurAPN extends CheckItemBase {
    private List<SimInfoRecord> mSimInfoList = null;
    private Set<Integer> mSimNeedSetAPN = new HashSet<Integer>();
    private String checkAPN = null;
    
    // numeric list  key for gemini
    //copy from ApnUtils.java (file of apn in Settings app)
    private static final String NUMERIC_LIST[] = {
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC,
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2,
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_3,
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_4
     };
    
    CheckCurAPN(Context c, String key) {
        super(c, key);

        setTitle(R.string.apn_check_title);

        if (key.equals(CheckItemKeySet.CI_CMWAP_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
            checkAPN = "cmwap";
            setNote(R.string.apn_cmwap);
            StringBuilder apn_cmwap = new StringBuilder();
            apn_cmwap.append(getContext().getString(R.string.apn_cmwap));
            apn_cmwap.append("\n" + getContext().getString(R.string.internet_doc));
        
            setNote(apn_cmwap.toString());
        } else if (key.equals(CheckItemKeySet.CI_CMWAP_AUTO_CONFG) || key.equals(CheckItemKeySet.CI_CMWAP)){
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
            checkAPN = "cmwap";
            StringBuilder apn_cmwap = new StringBuilder();
            apn_cmwap.append(getContext().getString(R.string.apn_cmwap));
            apn_cmwap.append("\n" + getContext().getString(R.string.internet_doc));
        
            setNote(apn_cmwap.toString());
        } else if (key.equals(CheckItemKeySet.CI_CMNET_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
            checkAPN = "cmnet";
            StringBuilder apn_cmnet = new StringBuilder();
            apn_cmnet.append(getContext().getString(R.string.apn_cmnet));
            apn_cmnet.append("\n" + getContext().getString(R.string.streaming_doc));
        
            setNote(apn_cmnet.toString());
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
            checkAPN = "cmnet";
            StringBuilder apn_cmnet = new StringBuilder();
            apn_cmnet.append(getContext().getString(R.string.apn_cmnet));
            apn_cmnet.append("\n" + getContext().getString(R.string.streaming_doc));
        
            setNote(apn_cmnet.toString());
        }

        mSimInfoList = SimInfoManager.getInsertedSimInfoList(c);

        if (mSimInfoList.isEmpty()) {
            setProperty(PROPERTY_AUTO_CHECK);
        }
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }

    public check_result getCheckResult() {
        if (!isCheckable()) {
            return super.getCheckResult();
        }

        if (mSimInfoList.isEmpty()) {
            setValue(R.string.string_sim);
            return check_result.UNKNOWN;
        }

        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            StringBuilder valueStr = new StringBuilder();
            check_result result = check_result.RIGHT;
            
            for (SimInfoRecord simInfo : mSimInfoList) {
                int simNo = simInfo.mSimSlotId + 1;
                Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                            null, null, null, null);

                if (cursor_APN == null || !cursor_APN.moveToNext()) {
                    if (cursor_APN != null) {
                        cursor_APN.close();
                    }

                    if(valueStr.toString().length() != 0) {
                        valueStr.append("\n");
                    }                    
                    valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_setted));
                    result = check_result.WRONG;
                    mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                } else {
                    String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));

                    cursor_APN.close();
                    
                    if (curAPN.equalsIgnoreCase(checkAPN)) {

                    } else {
                        result = check_result.WRONG;
                        if(valueStr.toString().length() != 0) {
                            valueStr.append("\n");
                        }
                        valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                        mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                    }
                }
            }

            setValue(valueStr.toString());
            return result;
        } else {
            Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers/preferapn"),
                                            null, null, null, null);
            if (cursor_APN == null || !cursor_APN.moveToNext()) {
                if (cursor_APN != null) {
                    cursor_APN.close();
                }
            
                setValue(R.string.apn_not_setted);
                return check_result.WRONG;
            } else {
                String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));
                cursor_APN.close();

                if (curAPN.equalsIgnoreCase(checkAPN)) {
                    setValue("");
                    return check_result.RIGHT;
                } else {
                    setValue(R.string.apn_not_correct);
                    return check_result.WRONG;
                }
            }
        }
    }

    public boolean onReset() {
        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            for (Integer simSlotId : mSimNeedSetAPN) {
                String numeric = SystemProperties.get(NUMERIC_LIST[simSlotId.intValue()], "-1");
                String where = "apn=\"" + checkAPN + "\"" + " and numeric=\"" + numeric + "\"";
                CTSCLog.v("CheckCurAPN", "where = " + where);
                
                int simNo = simSlotId.intValue() + 1;
                Uri uri= Uri.parse("content://telephony/carriers_sim" + simNo);
                Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "name"}, where, null, null);

                if ((cursor != null) && (true == cursor.moveToFirst())) {
                    cursor.moveToFirst();
                    int index = cursor.getColumnIndex("_id");
                    String apnId = cursor.getString(index);
                    CTSCLog.v("CheckCurAPN", "set apnId = " + apnId);

                    ContentValues values = new ContentValues();
                    values.put("apn_id", apnId);

                    getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                    values, 
                                    null, 
                                    null);
                }

                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            CTSCLog.v("CheckCurAPN","Not support GEMINI");
            String numeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
            String where = "apn=\"" + checkAPN + "\"" + " and numeric=\"" + numeric + "\"";
            CTSCLog.v("CheckCurAPN", "where = " + where);
            
            Uri uri= Uri.parse("content://telephony/carriers");
            Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "name"}, where, null, null);

            if ((cursor != null) && (true == cursor.moveToFirst())) {
                    cursor.moveToFirst();
                    int index = cursor.getColumnIndex("_id");
                    String apnId = cursor.getString(index);
                    CTSCLog.v("CheckCurAPN", "set apnId = " + apnId);
                    cursor.close();

                    ContentValues values = new ContentValues();
                    values.put("apn_id", apnId);

                    getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers/preferapn"),
                                    values, 
                                    null, 
                                    null);
            }
        }
        
        return true;
    }
}

class CheckBT extends CheckItemBase {
    private BluetoothAdapter mAdapter = null;
    
    CheckBT(Context c, String key) {
        super(c, key);

        setTitle(R.string.bt_name_check_title);

        mAdapter = BluetoothAdapter.getDefaultAdapter();

        StringBuilder sb = new StringBuilder();
        sb.append(getContext().getString(R.string.bt_cmcc_req));
        //sb.append("\n");
        //sb.append(getContext().getString(R.string.bt_doc));

        setNote(sb.toString());
    }

    public boolean onCheck() {
        if (mAdapter.isEnabled()) {
            StringBuilder sb = new StringBuilder();
            sb.append(getContext().getString(R.string.bt_name_current));
            sb.append(mAdapter.getName());
            
            setValue(sb.toString());
        } else {
            setValue(R.string.bt_open_device);
        }

        return true;
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }

    public check_result getCheckResult() {
        return super.getCheckResult();
    }

}

class CheckMMSRoaming extends CheckItemBase {
    CheckMMSRoaming(Context c, String key) {
        super(c, key);

        setProperty(PROPERTY_AUTO_FWD);
        setTitle(R.string.mms_roaming_title);

        StringBuilder sb = new StringBuilder();
        sb.append(getContext().getString(R.string.mms_roaming_setting));
        sb.append("\n");
        sb.append(getContext().getString(R.string.iot_doc));
        
        setNote(sb.toString());        
    }

    public Intent getForwardIntent() {
        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.android.mms", 
                "com.android.mms.ui.BootActivity"));

        if (getContext().getPackageManager().resolveActivity(intent, 0) != null) {
            return intent;
        } else {
            throw new RuntimeException(
                    "Can't find such activity: com.android.mms, " +
                    "com.android.mms.ui.BootActivity");
        }
    }
}


