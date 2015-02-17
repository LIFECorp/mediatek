package com.mediatek.configurecheck2;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.ConnectivityManager;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;

import com.android.internal.telephony.gemini.MTKPhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.gsm.GSMPhone;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.worldphone.ModemSwitchHandler;

import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.ArrayList;
import java.util.List;

class CheckNetworkMode extends CheckItemBase {
     private static final String TAG = " ProtocolItem CheckNetWorkMode";
     private boolean mAsyncDone = true;
     private boolean mNeedNofity = false;

     private final Handler mNetworkQueryHandler = new Handler() {
          public final void handleMessage(Message msg) {
            CTSCLog.d(TAG, "Receive msg form network mode query");
            mAsyncDone = true;
            AsyncResult ar = (AsyncResult) msg.obj;
              if (ar.exception == null) {
                  int type = ((int[]) ar.result)[0];
                  CTSCLog.d(TAG, "Get Preferred Type " + type);
                  switch (type) {
                      case 0: // TD-SCDMA preferred
                        setValue(R.string.value_NM_TD_SCDMA_Preferred);
                        if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_PREFERED_CONFIG)) {
                            mResult = check_result.RIGHT;
                        } else {
                           mResult = check_result.WRONG;
                        }
                          break;
                      case 1: // GSM only
                        setValue("GSM only");
                        mResult = check_result.WRONG;
                          break;
                      case 2: // TD-SCDMA only
                        setValue(R.string.value_NM_TD_SCDMA_Only);
                        if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_ONLY) ||
                            getKey().equals(CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG)) {
                            mResult = check_result.RIGHT;
                        } else {
                            mResult = check_result.WRONG;
                        }
                          break;
                      case 3: // GSM/TD-SCDMA(auto)
                        setValue(R.string.value_NM_TD_DUAL_MODE);                   
                        if (getKey().equals(CheckItemKeySet.CI_DUAL_MODE_CONFIG) ||
                            getKey().equals(CheckItemKeySet.CI_DUAL_MODE_CHECK)) {
                            mResult = check_result.RIGHT;
                        } else {                        
                            mResult = check_result.WRONG;
                        }
                          break;
                      default:
                          break;
                  }
              } else {
                 setValue("Query failed");                
              }
              if (mNeedNofity) {
                sendBroadcast();
              }
          }
      };
      
      private final Handler mNetworkSetHandler = new Handler() {
          public final void handleMessage(Message msg) {
            CTSCLog.i(TAG, "Receive msg form network mode set");  
            if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_PREFERED_CONFIG)) {
                setValue(R.string.value_NM_TD_SCDMA_Preferred);
                setNote(R.string.note_NM_TD_SCDMA_Preferred);
            } else if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG)) {
                setValue(R.string.value_NM_TD_SCDMA_Only);
                setNote(R.string.note_NM_TD_SCDMA_Only);
            } else if (getKey().equals(CheckItemKeySet.CI_DUAL_MODE_CONFIG)){
                setValue(R.string.value_NM_TD_DUAL_MODE);
                setNote(R.string.note_NM_TD_DUAL_MODE);
            }
            CTSCLog.d(TAG, "update network mode done refresh");
            mResult = check_result.RIGHT;
            sendBroadcast();
          }
      };
    /*
     * set title and note in constructor function
     */
    CheckNetworkMode (Context c, String key) {
        super(c, key);
        
        if (key.equals(CheckItemKeySet.CI_TDWCDMA_ONLY)) {
            setTitle(R.string.title_Network_Mode);
            setNote(getContext().getString(R.string.note_NM_TD_SCDMA_Only) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            setProperty(PROPERTY_AUTO_CHECK);
        } else if(key.equals(CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG)){
            setTitle(R.string.title_Network_Mode);
            setNote(getContext().getString(R.string.note_NM_TD_SCDMA_Only) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else if(key.equals(CheckItemKeySet.CI_TDWCDMA_PREFERED_CONFIG)){
            setTitle(R.string.title_Network_Mode);
            setNote(R.string.note_NM_TD_SCDMA_Preferred);
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else if(key.equals(CheckItemKeySet.CI_DUAL_MODE_CONFIG) ||
                  key.equals(CheckItemKeySet.CI_DUAL_MODE_CHECK)){
            setTitle(R.string.title_Network_Mode);
            if (key.equals(CheckItemKeySet.CI_DUAL_MODE_CHECK)) {
                setProperty(PROPERTY_AUTO_CHECK); 
                setNote(getContext().getString(R.string.note_NM_TD_DUAL_MODE) + getContext().getString(R.string.SOP_self_check) + 
                       getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            } else {
                setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
                setNote(getContext().getString(R.string.note_NM_TD_DUAL_MODE) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            }
        }        
    }

    public boolean onCheck() {
        getNetworkMode();
        return true;
    }
    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */ 
        if (!mAsyncDone) {
            mResult = check_result.UNKNOWN;
            mNeedNofity = true;
            setValue(R.string.ctsc_querying);
            return mResult;
        }
        mNeedNofity = false;
        CTSCLog.d(TAG, "getCheckResult mResult = " + mResult);
        if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_ONLY)) {           
            return mResult;
        } else {
            return mResult;
        }        
    }
    
    public boolean onReset() {
        /*
                * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        setNetWorkMode();  
        return true;
    }
    
    private void getNetworkMode() {
        Phone mPhone = null;
        CTSCLog.i(TAG, "getNetworkMode");
        mAsyncDone = false;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mGeminiPhone.getPhonebyId(PhoneConstants.GEMINI_SIM_1).getPreferredNetworkType(
                    mNetworkQueryHandler.obtainMessage());
        } else {
             mPhone = PhoneFactory.getDefaultPhone();
             mPhone.getPreferredNetworkType(mNetworkQueryHandler.obtainMessage());
        }
    }  

    private void setNetWorkMode() {
        Phone mPhone = null;
        Message msg = null;
        GeminiPhone mGeminiPhone = null;
        CTSCLog.i(TAG, "setNetworkMode");
        setValue("Modifing...");
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_PREFERED_CONFIG)) {
                mGeminiPhone.setPreferredNetworkTypeGemini(0, mNetworkSetHandler.obtainMessage(), PhoneConstants.GEMINI_SIM_1);                            
            } else if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG)) {
               mGeminiPhone.setPreferredNetworkTypeGemini(2, mNetworkSetHandler.obtainMessage(), PhoneConstants.GEMINI_SIM_1);
            } else if (getKey().equals(CheckItemKeySet.CI_DUAL_MODE_CONFIG)){
                mGeminiPhone.setPreferredNetworkTypeGemini(3, mNetworkSetHandler.obtainMessage(), PhoneConstants.GEMINI_SIM_1);
            }
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
            if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_PREFERED_CONFIG)) {
                mPhone.setPreferredNetworkType(0, mNetworkSetHandler.obtainMessage());                            
            } else if (getKey().equals(CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG)) {
                mPhone.setPreferredNetworkType(2, mNetworkSetHandler.obtainMessage());
            } else if (getKey().equals(CheckItemKeySet.CI_DUAL_MODE_CONFIG)){
                mPhone.setPreferredNetworkType(3, mNetworkSetHandler.obtainMessage());
            }
        }
    }
}


class CheckGPRSProtocol extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckGPRSProtol";
    private boolean needRefresh = false;
    
    private Context getEMContext() {
        Context eMContext = null;
        try {
            eMContext = getContext().createPackageContext(
                    "com.mediatek.engineermode", Context.CONTEXT_IGNORE_SECURITY);
        } catch (NameNotFoundException e) {
            // TODO: handle exception
            e.printStackTrace();
        }
        if (null == eMContext) {
            throw new NullPointerException("eMContext=" + eMContext);
        }
        return eMContext;
    }
    
    private final Handler mResponseHander = new Handler() {
          public final void handleMessage(Message msg) {
            CTSCLog.i(TAG, "Receive msg form GPRS always attached continue  set"); 
            setValue(R.string.value_GPRS_attach_continue);
            mResult = check_result.RIGHT;
            sendBroadcast();
         }
    };
          
    CheckGPRSProtocol (Context c, String key) {
        super(c, key);
        
        if (key.equals(CheckItemKeySet.CI_GPRS_ON)) {
            setTitle(R.string.title_GPRS_ALWAYS_ATTACH);
            setNote(getContext().getString(R.string.note_GPRS_always_on) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            setProperty(PROPERTY_AUTO_CHECK);
        } else if (key.equals(CheckItemKeySet.CI_GPRS_CONFIG)){
            setTitle(R.string.title_GPRS_ALWAYS_ATTACH);
            setNote(getContext().getString(R.string.note_GPRS_always_on) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else if (key.equals(CheckItemKeySet.CI_GPRS_ATTACH_CONTINUE)){ 
            setTitle(R.string.title_GPRS_ALWAYS_ATTACH_CONTINUE);
            setNote(getContext().getString(R.string.note_GPRS_attach_continue) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }        
    }
    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */
        CTSCLog.i(TAG, "getCheckResult");
        String resultString;
        String resultImg;
        String note = null;

        int gprsAttachType = SystemProperties.getInt(
                "persist.radio.gprs.attach.type", 1);  
        CTSCLog.d(TAG, "get gprs mode gprsAttachType =" + gprsAttachType);
        
        if (getKey().equals(CheckItemKeySet.CI_GPRS_ON) ||
            getKey().equals(CheckItemKeySet.CI_GPRS_CONFIG)) {//yaling check how to get continue info
            if (gprsAttachType == 1) {
                setValue(R.string.value_GPRS_always_on);
                mResult = check_result.RIGHT;
            } else {
                setValue(R.string.value_GPRS_not_always_on);
                mResult = check_result.WRONG;               
            }
            CTSCLog.d(TAG, "getCheckResult mResult = " + mResult);            
            return mResult;
        } else if(getKey().equals(CheckItemKeySet.CI_GPRS_ATTACH_CONTINUE)) {
            SharedPreferences preference = getEMContext().getSharedPreferences("com.mtk.GPRS", 0);
            int attachMode = preference.getInt("ATTACH_MODE", -1);
            CTSCLog.d(TAG, "yaling test attachmode = " + attachMode);
            
            if (attachMode == 1) {
                setValue(R.string.value_GPRS_attach_continue);
                mResult = check_result.RIGHT;
            } else if (attachMode == 0){
                setValue(R.string.value_GPRS_when_needed_continue);
                mResult = check_result.WRONG;
            } else {
                setValue(R.string.value_GPRS_not_to_specify);
                mResult = check_result.WRONG;
            }
        }
        CTSCLog.d(TAG, "getCheckResult2 mResult = " + mResult);                   
        return mResult;
    } 
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "getReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        SystemProperties.set("persist.radio.gprs.attach.type", "1");
        
        if(getKey().equals(CheckItemKeySet.CI_GPRS_ATTACH_CONTINUE)) {
            String cmdStr[] = { "AT+EGTYPE=1,1", "" };
            Phone mPhone = PhoneFactory.getDefaultPhone();
            mPhone.invokeOemRilRequestStrings(cmdStr, mResponseHander
                      .obtainMessage());
            
            setValue("setting...");
            mResult = check_result.UNKNOWN;

            SharedPreferences preference = getEMContext().getSharedPreferences("com.mtk.GPRS", 0);
            SharedPreferences.Editor edit = preference.edit();

            edit.putInt("ATTACH_MODE", 1);
            edit.commit();
        } else {
            setValue(R.string.value_GPRS_always_on);
            mResult = check_result.RIGHT;
        }
        return true;
    }
}

class CheckCFU extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckCFU";
    private boolean mAsyncDone = true;

    private final Handler mSetModemATHander = new Handler() {
        public final void handleMessage(Message msg) {
            CTSCLog.i(TAG, "Receive msg form CFU set");  
            mAsyncDone = true;
            setValue(R.string.value_CFU_always_not_query);
            mResult = check_result.RIGHT;
            sendBroadcast();
        }
    };
    
    CheckCFU (Context c, String key) {
        super(c, key);
        
        if (key.equals(CheckItemKeySet.CI_CFU)) {
            setTitle(R.string.title_CFU);
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setTitle(R.string.title_CFU);
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } 
        setNote(getContext().getString(R.string.note_CFU_off) + 
            getContext().getString(R.string.SOP_REFER) + 
            getContext().getString(R.string.SOP_Protocol));
    }

    public boolean onCheck() {  
        CTSCLog.i(TAG, "onCheck");
        String cfuSetting = SystemProperties.get(PhoneConstants.CFU_QUERY_TYPE_PROP,
                PhoneConstants.CFU_QUERY_TYPE_DEF_VALUE);
        CTSCLog.i(TAG, "cfuSetting = " + cfuSetting);
        
        if (cfuSetting.equals("0")) {
            setValue(R.string.value_CFU_default);
            mResult = check_result.RIGHT;
        } else if (cfuSetting.equals("1")) {
            setValue(R.string.value_CFU_always_not_query);
            mResult = check_result.RIGHT;
        } else if (cfuSetting.equals("2")) {
            setValue(R.string.value_CFU_always_query);
            mResult = check_result.WRONG;
        } else {
            setValue("CFU query failed");
            mResult = check_result.WRONG;
        }
        
        return true;
    }
    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */
        if (!mAsyncDone) {
            mResult = check_result.UNKNOWN;
            setValue(R.string.ctsc_querying);
            return mResult;
        }
        CTSCLog.i(TAG, "getCheckResult mResult = " + mResult);
        return mResult;
    }
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        setCFU();             
        return true;
    }

    private void setCFU() {
        mAsyncDone = false;
        String cmdString[] = new String[2];
        cmdString[0] = "AT+ESSP=1";
        cmdString[1] = "";
        CTSCLog.i(TAG, "setCFU");

        Phone mPhone = PhoneFactory.getDefaultPhone();
        mPhone.invokeOemRilRequestStrings(cmdString, mSetModemATHander.obtainMessage());
    }
}

class CheckCTAFTA extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckCTAFTA";
    private boolean mAsyncDone = true;
    private boolean mNeedNofity = false;
    
    private final Handler mModemATHander = new Handler() {
        public final void handleMessage(Message msg) {  
            mAsyncDone = true;
            CTSCLog.i(TAG, "recieve msg from query CTAFTA ");
            boolean isJB2 = (Build.VERSION.SDK_INT >= 17);
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar.exception == null) {
                String data[] = (String[]) ar.result;

                if (null != data) {
                    CTSCLog.i(TAG, "data length is " + data.length);
                } else {
                    CTSCLog.i(TAG, "The returned data is wrong.");
                }
                int i = 0;
                for (String str : data) {
                    i++;
                }
                if (data[0].length() > 6) {
                    String mode = data[0].substring(7, data[0].length());
                    if (mode.length() >= 3) {
                        String subMode = mode.substring(0, 1);
                        String subCtaMode = mode.substring(2, mode.length());
                        CTSCLog.d(TAG, "subMode is " + subMode);
                        CTSCLog.d(TAG, "subCtaMode is " + subCtaMode);
                        if ("0".equals(subMode)) {
                            setValue(R.string.value_modem_none);
                            if (getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
                                mResult = check_result.WRONG;
                            } else {
                                mResult = check_result.RIGHT;
                            }
                        } else if ("1".equals(subMode)) {
                            if (isJB2) {
                                setValue(R.string.value_modem_Integrity_off);
                            } else {
                                setValue(R.string.value_modem_CTA);
                            }
                            if (getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
                              mResult = check_result.RIGHT;
                            } else {
                               mResult = check_result.WRONG;
                            }                                  
                        } else if ("2".equals(subMode)) {
                            setValue(R.string.value_modem_FTA);
                            mResult = check_result.WRONG;
                        } else if ("3".equals(subMode)) {
                            setValue(R.string.value_modem_IOT);
                            if (getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
                                mResult = check_result.WRONG;
                            } else {
                                mResult = check_result.RIGHT;
                            }
                        } else if ("4".equals(subMode)) {
                            setValue(R.string.value_modem_Operator);
                            if (getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
                                mResult = check_result.WRONG;
                            } else {
                                mResult = check_result.RIGHT;
                            }
                        } else if (isJB2 && "5".equals(subMode)) {
                            setValue(R.string.value_modem_Factory);
                            if (getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
                                mResult = check_result.WRONG;
                            } else {
                                mResult = check_result.RIGHT;
                            }
                        }                        
                    } else {
                        setValue("Query failed");
                        mResult = check_result.UNKNOWN;
                    }
                } else {
                    setValue("Query failed");
                    mResult = check_result.UNKNOWN;
                }
            } else {
                setValue("Query failed");
                mResult = check_result.UNKNOWN;
            } 
            if (mNeedNofity) {                 
                sendBroadcast();
            }
        }
    };

    private final Handler mSetModemATHander = new Handler() {
        public final void handleMessage(Message msg) {
            CTSCLog.i(TAG, "Receive msg form Mode set");  
            if(getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
                setValue(R.string.value_modem_Integrity_off);
            } else if(getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_OFF)) {
                setValue(R.string.value_modem_none);
            }
            mResult = check_result.RIGHT;
            sendBroadcast();
        }
    };

    CheckCTAFTA (Context c, String key) {
        super(c, key);
        if (key.equals(CheckItemKeySet.CI_CTAFTA)) {
            setTitle(R.string.title_CTA_FTA);
            setNote(getContext().getString(R.string.note_CTA_FTA_off) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol));
            setProperty(PROPERTY_AUTO_CHECK);
        } else if (key.equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)){ 
            setTitle(R.string.title_integrity_check_off);
            setNote(getContext().getString(R.string.note_integrity_check_off) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else {
            setTitle(R.string.title_CTA_FTA);
            setNote(getContext().getString(R.string.note_CTA_FTA_off) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }
    }

    public boolean onCheck() {
        getCTAFTA();
        return true;
    }
    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */ 
       if (!mAsyncDone) {
            mResult = check_result.UNKNOWN;
            mNeedNofity = true;
            setValue(R.string.ctsc_querying);
            return mResult;
        }
        mNeedNofity = false;
        CTSCLog.i(TAG, "getCheckResult mResult = " + mResult);
        return mResult;
    } 
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onResult");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        if (getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_OFF)) {
            setCTAFTA("0");
        } else if(getKey().equals(CheckItemKeySet.CI_CTAFTA_CONFIG_ON)) {
            setCTAFTA("1");
        }
        return true;
    }
   
    private void getCTAFTA() {
        String cmd[] = new String[2];
        cmd[0] = "AT+EPCT?";
        cmd[1] = "+EPCT:";
        CTSCLog.i(TAG, "getCTAFTA");
        mAsyncDone = false;
        Phone mPhone = PhoneFactory.getDefaultPhone();
        mPhone.invokeOemRilRequestStrings(cmd, mModemATHander
                .obtainMessage());
    }
    
    private void setCTAFTA(String str) {
        Phone mPhone = PhoneFactory.getDefaultPhone();
        String cmd[] = new String[2];
        cmd[0] = "AT+EPCT=" + str;
        cmd[1] = "";
        CTSCLog.i(TAG, "setCTAFTA");
        mPhone.invokeOemRilRequestStrings(cmd, mSetModemATHander
                .obtainMessage());
    }
}

class CheckDataConnect extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckDataConnect";
    private boolean mhasSim = true;
    private boolean mReseting = false;
    
    CheckDataConnect (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_Data_Connection);
        
        if (key.equals(CheckItemKeySet.CI_DATA_CONNECT_OFF)) {            
            setNote(getContext().getString(R.string.note_DC_off) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol));
            setProperty(PROPERTY_AUTO_CHECK);
        } else if (key.equals(CheckItemKeySet.CI_DATA_CONNECT_ON)) {
            setNote(getContext().getString(R.string.note_DC_on) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_IOT));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else if (key.equals(CheckItemKeySet.CI_DATA_CONNECT_ON_DM)) {
            setNote(getContext().getString(R.string.note_DC_on) + 
                    getContext().getString(R.string.SOP_REFER) + 
                    getContext().getString(R.string.SOP_DM));
                setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else if (key.equals(CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)) {
            setNote(getContext().getString(R.string.note_DC_off) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } else if (key.equals(CheckItemKeySet.CI_DATA_CONNECT_CHECK)) {
            //if (FeatureOption.MTK_GEMINI_SUPPORT) {// default is off
                setNote(getContext().getString(R.string.note_DC_off) + 
                    getContext().getString(R.string.SOP_REFER) + 
                    getContext().getString(R.string.SOP_Protocol));
            //} else {// default is on
            //    setNote(getContext().getString(R.string.note_DC_on) + 
            //    getContext().getString(R.string.SOP_REFER) + 
            //    getContext().getString(R.string.SOP_Protocol));
            //}
            setProperty(PROPERTY_AUTO_CHECK);
        }
        
        List<SimInfoRecord> mSimList = SimInfoManager.getInsertedSimInfoList(getContext());
        CTSCLog.i(TAG, "mSimList size : "+mSimList.size());

        if (mSimList.size() == 0) {
            setProperty(PROPERTY_AUTO_CHECK);
            setValue(R.string.value_SIM);
            mResult = check_result.UNKNOWN;
            mhasSim = false;
        } 
    }

    public boolean onCheck() {
        CTSCLog.d(TAG, "OnCheck mHasSim = " + mhasSim);
        if (mReseting) {
            setValue(R.string.ctsc_querying);
            return true;
        }
        if (mhasSim) {
            ConnectivityManager cm = (ConnectivityManager)getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            
            boolean dataEnable = cm.getMobileDataEnabled();            
            if (!dataEnable) {               
                setValue(R.string.value_DC_off);
                if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON)
                        || getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON_DM)) {
                    mResult = check_result.WRONG;
                } else if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_OFF) || 
                        getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)) {
                    mResult = check_result.RIGHT;
                } else if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_CHECK)) {
                    //if (FeatureOption.MTK_GEMINI_SUPPORT) {

                        mResult = check_result.RIGHT;
                    //} else {
                    //    mResult = check_result.WRONG;
                    //}
                }
            } else {  
                setValue(R.string.value_DC_on);
                if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON)
                        || getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON_DM)) {
                    mResult = check_result.RIGHT;
                } else if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_OFF) || 
                        getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)) {
                    mResult = check_result.WRONG;
                } else if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_CHECK)) {
                    //if (FeatureOption.MTK_GEMINI_SUPPORT) {

                        mResult = check_result.WRONG;
                    //} else {
                    //    mResult = check_result.RIGHT;
                    //}
                }
            }
            CTSCLog.i(TAG, "onCheck data enable = " +dataEnable + " mResult = " + mResult);
        }

        return true;
    }

    
    public check_result getCheckResult() {
        /*
         * implement check function here
         */ 
        CTSCLog.i(TAG, "getCheckResult mResult = " + mResult);
        return mResult;
    } 


    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        ConnectivityManager cm = (ConnectivityManager)getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)) {
            cm.setMobileDataEnabled(false);
            setValue(R.string.value_DC_off);
        } else if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON)
                || getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON_DM)) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                Intent intent = new Intent(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
                List<SimInfoRecord> mSimList = SimInfoManager.getInsertedSimInfoList(getContext());
                CTSCLog.i(TAG, "mSimList size : "+mSimList.size());
                SimInfoRecord siminfo = SimInfoManager.getSimInfoBySlot(getContext(), mSimList.get(0).mSimSlotId);
                intent.putExtra(PhoneConstants.MULTI_SIM_ID_KEY, siminfo.mSimInfoId); 
                getContext().sendBroadcast(intent);
            } else {
                  cm.setMobileDataEnabled(true);
            }
            
            setValue(R.string.value_DC_on);
        }         
       // mResult = check_result.RIGHT;
        mReseting = true;
        new Handler().postDelayed(new Runnable() {
             public void run() {
                CTSCLog.d(TAG, "data connect send set refresh");
                sendBroadcast();
                mReseting = false;
                mResult = check_result.RIGHT;
                if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)) {           
                    setValue(R.string.value_DC_off);
                } else if (getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON)
                        || getKey().equals(CheckItemKeySet.CI_DATA_CONNECT_ON_DM)) {
                    setValue(R.string.value_DC_on);
                }
           }
        }, 2000);
        
        return true;
    }
}

class CheckDataRoam extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckDataRoam";
    private boolean mhasSim = true;
    
    CheckDataRoam (Context c, String key) {
        super(c, key);
        
        if (key.equals(CheckItemKeySet.CI_DATA_ROAM)) {
            setTitle(R.string.title_Data_ROAM);
            setNote(getContext().getString(R.string.note_DR_on) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_IOT));
            setProperty(PROPERTY_AUTO_CHECK);
        } else if (key.equals(CheckItemKeySet.CI_DATA_ROAM_CONFIG)) {
            setTitle(R.string.title_Data_ROAM);
            setNote(getContext().getString(R.string.note_DR_on) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_IOT));
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }else if (key.equals(CheckItemKeySet.CI_DATA_ROAM_OFF_CONFIG)){
            setTitle(R.string.title_Data_ROAM);
            setNote(R.string.note_DC_off);
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        } 

        List<SimInfoRecord> mSimList = SimInfoManager.getInsertedSimInfoList(getContext());
        CTSCLog.i(TAG, "mSimList size : "+mSimList.size());

        if (mSimList.size() == 0) {
            setProperty(PROPERTY_AUTO_CHECK);
            setValue(R.string.value_SIM);
            mResult = check_result.UNKNOWN;
            mhasSim = false;
        } 
    }
        

    public boolean onCheck() {
        if (mhasSim) {
        Phone mPhone =  PhoneFactory.getDefaultPhone();
        boolean dataRoamEnable = mPhone.getDataRoamingEnabled();   
        if (getKey().equals(CheckItemKeySet.CI_DATA_ROAM) ||
            getKey().equals(CheckItemKeySet.CI_DATA_ROAM_CONFIG)) {
            if (!dataRoamEnable) { 
                mResult = check_result.WRONG;
                setValue(R.string.value_DR_off);
            } else { 
                mResult = check_result.RIGHT;
                setValue(R.string.value_DR_on);
            }                
        } else if (getKey().equals(CheckItemKeySet.CI_DATA_ROAM_OFF_CONFIG)) {
            if (!dataRoamEnable) { 
                setValue(R.string.value_DR_off);
                mResult = check_result.RIGHT;
            } else { 
                mResult = check_result.WRONG;
                setValue(R.string.value_DR_on);
            }         
        }  
        
        CTSCLog.d(TAG, "data roam Enable = " + dataRoamEnable + " mResult = " + mResult);
        }        
        return true;
    }

    public check_result getCheckResult() {
        /*
         * implement check function here
         */ 
        CTSCLog.d(TAG, "get check result mResult = " + mResult);
        return mResult;
    } 

    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }

        List<SimInfoRecord> mSimList = SimInfoManager.getInsertedSimInfoList(getContext());
        if (getKey().equals(CheckItemKeySet.CI_DATA_ROAM_CONFIG)) {

            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                try {
                    TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
                    if (telephonyManagerEx != null) {
                        telephonyManagerEx.setDataRoamingEnabled(true, mSimList.get(0).mSimSlotId);
                    }
                } catch (RemoteException e) {
                    CTSCLog.d(TAG,"iTelephony exception");                        
                }
                
            } else {
                Phone mPhone =  PhoneFactory.getDefaultPhone();            
                mPhone.setDataRoamingEnabled(true);
            }
            setValue(R.string.value_DR_on);
        } else if (getKey().equals(CheckItemKeySet.CI_DATA_ROAM_OFF_CONFIG)) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                try {
                    TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
                    if (telephonyManagerEx != null) {
                        telephonyManagerEx.setDataRoamingEnabled(
                                false, mSimList.get(0).mSimSlotId);
                    }
                } catch (RemoteException e) {
                    CTSCLog.d(TAG,"iTelephony exception");                     
                }
                
            } else {
                Phone mPhone =  PhoneFactory.getDefaultPhone();
                mPhone.setDataRoamingEnabled(false);
            }
            setValue(R.string.value_DR_off);
        }
        mResult = check_result.RIGHT;
        return true;
    }
}

class CheckPLMN extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckPLMN";
    private boolean mAsyncDone = true;
    private boolean mNeedNofity = false;

    private final Handler mNetworkSelectionModeHandler = new Handler() {
        public final void handleMessage(Message msg) {
            CTSCLog.i(TAG, "Receive msg form network slection mode");
           
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar.exception == null) {
                int auto = ((int[]) ar.result)[0];
                CTSCLog.d(TAG, "Get Selection Type " + auto);
                if(auto == 0) {
                    setValue(R.string.value_PLMN_auto_select);
                    mResult = check_result.RIGHT;
                } else {
                    setValue(R.string.value_PLMN_manual_select);
                    mResult = check_result.WRONG;
                }   
            } else {
                setValue("PLMN Query failed");
                mResult = check_result.UNKNOWN;
            }
            mAsyncDone = true;
            if (mNeedNofity) {
                sendBroadcast();
            }                
        }
    };

    
    private final Handler mSetNetworkSelectionModeHander = new Handler() {
        public final void handleMessage(Message msg) {
            CTSCLog.i(TAG, "Receive msg form Mode set");  
            if(getKey().equals(CheckItemKeySet.CI_PLMN_DEFAULT_CONFIG)) {
                setValue(R.string.value_PLMN_auto_select);
            }            
            mResult = check_result.RIGHT;
            sendBroadcast();
        }
    };
     
    CheckPLMN (Context c, String key) {
        super(c, key);
        
        if (key.equals(CheckItemKeySet.CI_PLMN_DEFAULT)) {
            setTitle(R.string.title_PLMN);
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setTitle(R.string.title_PLMN);
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }
        setNote(getContext().getString(R.string.note_PLMN_auto_select) + 
            getContext().getString(R.string.SOP_REFER) + 
            getContext().getString(R.string.SOP_Protocol) );
    }

    public boolean onCheck() {
        List<SimInfoRecord> mSimList = SimInfoManager.getInsertedSimInfoList(getContext());
        CTSCLog.i(TAG, "mSimList size : "+mSimList.size());

        if (mSimList.size() == 0) {
            setProperty(PROPERTY_AUTO_CHECK);
            setValue(R.string.value_SIM);
            mResult = check_result.UNKNOWN;
        } else {
            getNetworkSelectionMode();
        }
        return true;
    }


    public check_result getCheckResult() {
        /*
         * implement check function here
         */ 
        CTSCLog.i(TAG, "getCheckResult");
        if (!mAsyncDone) {
             mResult = check_result.UNKNOWN;
             mNeedNofity = true;
             setValue(R.string.ctsc_querying);
             return mResult;
        }
        mNeedNofity = false;
        return mResult;
    } 

    @Override
    public boolean onReset() {
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }
        setNetWorkSelectionMode();
        return true;
    }

    public void getNetworkSelectionMode() {
        CTSCLog.i(TAG, "getNetworkSelectionMode");
        Phone mPhone = null;
        mAsyncDone = false;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mPhone = mGeminiPhone.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
        ((PhoneBase)((PhoneProxy)mPhone).getActivePhone()).mCi.
            getNetworkSelectionMode(mNetworkSelectionModeHandler.obtainMessage());
     }

     private void setNetWorkSelectionMode() {
         Phone mPhone = null;
         if (FeatureOption.MTK_GEMINI_SUPPORT) {
             GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
             mPhone = mGeminiPhone.getDefaultPhone();
         } else {
             mPhone = PhoneFactory.getDefaultPhone();
         }
         ((PhoneBase)((PhoneProxy)mPhone).getActivePhone()).mCi.
             setNetworkSelectionModeAutomatic(mSetNetworkSelectionModeHander.obtainMessage());
     }
}

class CheckDTSUPPORT extends CheckItemBase {
    
    private static final String TAG = " ProtocolItem CheckDTSUPPORT";
    
    CheckDTSUPPORT (Context c, String key) {
        super(c, key);
        setTitle(R.string.title_GEMINI_SUPPORT);
        if (FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_DT_SUPPORT) {
            setValue(R.string.value_GEMINI_DT_SUPPORT);
        } else if (FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_DT_SUPPORT == false) {
            setValue(R.string.value_GEMINI_SINGLE_SUPPORT);     
        } else if (FeatureOption.MTK_GEMINI_SUPPORT == false) {
            setValue(R.string.value_SINGLE_SD_SUPPORT);
        }
        setNote(getContext().getString(R.string.note_GEMINI_SUPPORT) + 
            getContext().getString(R.string.SOP_REFER) + 
            getContext().getString(R.string.SOP_FieldTest));
    }
}

class CheckSIMSlot extends CheckItemBase {
    private static final String TAG = " ProtocolItem CheckSIMSlot";
    
    CheckSIMSlot (Context c, String key) {
        super(c, key);

        List<SimInfoRecord> mSimList = SimInfoManager.getInsertedSimInfoList(getContext());
        CTSCLog.i(TAG, "mSimList size : " + mSimList.size());
        setTitle(R.string.title_SIM_STATE);                    
        if (key.equals(CheckItemKeySet.CI_DUAL_SIM_CHECK)) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                if (mSimList.size() == 2) {
                    setValue(R.string.value_DUAL_SIM_STATE);
                    mResult = check_result.RIGHT;
                } else {
                    setValue(R.string.note_DUAL_SIM_STATE);
                    mResult = check_result.WRONG;
                }            
            } else {
                if (mSimList.size() == 0) {            
                    setValue(R.string.value_SIM);
                    mResult = check_result.WRONG;
                } else {
                    setValue(R.string.value_SIM_STATE);
                    mResult = check_result.RIGHT;
                }
            }
            setNote(getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_FieldTest));
        } else if (key.equals(CheckItemKeySet.CI_SIM_3G_CHECK)) {
            if (FeatureOption.MTK_GEMINI_SUPPORT && mSimList.size() == 2) {
                setValue(getContext().getString(R.string.note_SINGLE_SIM_CHECK));
                mResult = check_result.WRONG;
            } else if (mSimList.size() == 0) {
                setValue(getContext().getString(R.string.note_SIM_3G_CHECK));
                mResult = check_result.WRONG;
            } else {
                int slot3G;
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
                    slot3G = mGeminiPhone.get3GCapabilitySIM();
                } else {
                    Phone mPhone = PhoneFactory.getDefaultPhone();
                    slot3G = mPhone.get3GCapabilitySIM();
                }
                CTSCLog.d(TAG, "is3G slot = " + slot3G);
                if (mSimList.get(0).mSimSlotId == slot3G) {
                    
                    CTSCLog.d(TAG, "is3G result = true");
                    setValue(getContext().getString(R.string.value_SIM_3G));
                    mResult = check_result.RIGHT;
                } else {
                    setValue(getContext().getString(R.string.note_SIM_3G_CHECK));
                    mResult = check_result.WRONG;
                }
            }            
            setNote(getContext().getString(R.string.note_SIM_3G_CHECK) + 
                getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_LOCAL_TEST));                
        }
        setProperty(PROPERTY_AUTO_CHECK);
    }

    public check_result getCheckResult() {
        /*
         * implement check function here
         */             
        return mResult;
    } 
    
    public boolean onReset() {
        /*
         * implement your reset function here
         */
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            //On no, this instance is check only, not allow auto config.
            return false;
        }                
        return true;
    }
}

class CheckModemSwitch extends CheckItemBase {
    private static final String TAG = "CheckModemSwitch";
    
    CheckModemSwitch(Context c, String key) {
        super(c, key);
        
        setProperty(PROPERTY_AUTO_CHECK | PROPERTY_AUTO_CONFG);
        setTitle(R.string.modem_switch_title);
        setNote(getContext().getString(R.string.modem_switch_note) + getContext().getString(R.string.SOP_REFER) + 
                getContext().getString(R.string.SOP_Protocol) + "" + getContext().getString(R.string.SOP_PhoneCard));
    }

    public boolean onCheck() {
        CTSCLog.d(TAG, " oncheck");
        int modemType = ModemSwitchHandler.getModem();
        CTSCLog.d(TAG, "Get modem type: " + modemType);

        if (modemType == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
            setValue(R.string.modem_switch_fdd);
            mResult = check_result.WRONG;
        } else if (modemType == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
            setValue(R.string.modem_switch_tdd);
            mResult = check_result.RIGHT;
        } else {
            setValue(R.string.ctsc_error);
            mResult = check_result.WRONG;
            CTSCLog.e(TAG, "Query Modem type failed: " + modemType);
        }

        if (Settings.Global.getInt(getContext().getContentResolver(),
                Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 1) == 1) {
            setValue(R.string.modem_switch_auto);
            mResult = check_result.WRONG;
        }

        return true;
    }

    public check_result getCheckResult() {          
        return mResult;
    } 
    
    public boolean onReset() {
        CTSCLog.i(TAG, "onReset");
        boolean result = false;
        CTSCLog.d(TAG, "Need to set modem type to TDD...");
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            (((GeminiPhone)(MTKPhoneFactory.getDefaultPhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL);
        } else {
            PhoneProxy proxyPhone = (PhoneProxy)(MTKPhoneFactory.getDefaultPhone());
            (((GSMPhone)(proxyPhone.getActivePhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL);
        }
        if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
            ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
        }
        if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
            result = true;
        }
        CTSCLog.d(TAG, "Set modem type result: " + result);

        return true;
    }
}
