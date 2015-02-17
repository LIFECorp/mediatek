package com.mediatek.configurecheck2;

import com.mediatek.configurecheck2.R;
import com.mediatek.common.featureoption.FeatureOption;

import android.content.Context;

public class ProviderFactory {
    /**
     * @Prompt Add your provider in this provider factory
     */
    public static CheckItemProviderBase getProvider(Context c, String category) {

        if (category.equals(c.getString(R.string.one_button_check))) {
            return new OneBtnCheckProvider(c);

        } else if (category.equals(c.getString(R.string.wc_ttcn_3g_6210))) {
            return new TTCN3GProvider(c);

        } else if (category.equals(c.getString(R.string.wc_anritsu_23g_8470))) {
            return new Anritsu23GProvider(c);

        } else if (category.equals(c.getString(R.string.wc_ttcn_23g_6210))) {
            return new TTCN23GProvider(c);

        } else if (category.equals(c.getString(R.string.wc_rrm_6200))) {
            return new RRMProvider(c);

        } else if (category.equals(c.getString(R.string.wc_rf_6010_8960))) {
            return new RFProvider(c);

        } else if (category.equals(c.getString(R.string.wc_network_iot))) {
            return new NetworkIOTProvider(c);

        } else if (category.equals(c.getString(R.string.wc_basic_communication))) {
            return new BasicCommunProvider(c);

        } else if (category.equals(c.getString(R.string.wc_phone_card_compatible))) {
            return new PhoneCardCompatProvider(c);

        } else if (category.equals(c.getString(R.string.sa_local_func))) {
            return new LocalFuncProvider(c);

        } else if (category.equals(c.getString(R.string.sa_broadband_internet))) {
            return new BroadbandNetProvider(c);

        } else if (category.equals(c.getString(R.string.sa_video_tel))) {
            return new VideoTelProvider(c);

        } else if (category.equals(c.getString(R.string.sa_streaming))) {
            return new StreamingProvider(c);

        } else if (category.equals(c.getString(R.string.sa_dm_lab_net))) {
            return new DMLabNetProvider(c);

        } else if (category.equals(c.getString(R.string.sa_dm_real_net))) {
            return new DMRealNetProvider(c);

        //} else if (category.equals(c.getString(R.string.sa_mobile_tv))) {
            //return new MobileTVProvider(c);

        } else if (category.equals(c.getString(R.string.sa_agps))) {
            return new AgpsProvider(c);

        } else if (category.equals(c.getString(R.string.sa_wlan))) {
            return new WlanProvider(c);

        } else if (category.equals(c.getString(R.string.sr_mtbf))) {
            return new MTBFProvider(c);

        } else if (category.equals(c.getString(R.string.sr_local_perf))) {
            return new LocalPerfProvider(c);

        } else if (category.equals(c.getString(R.string.hr_power_consumption))) {
            return new PowerConsumpProvider(c);

        } else if (category.equals(c.getString(R.string.fd_field_test))) {
            return new FieldTestProvider(c);

        } else if (category.equals(c.getString(R.string.MTBF))) {
            return new MTKMTBFProvider(c);
        } else {
            throw new RuntimeException("No such category !!! ");
        }
    }
}

/*
 * Add check item key here !
 * CI_[item name]_[characteristic]
 */
final class CheckItemKeySet {
	public static final String CI_GPRS_CHECK_ONLY = "GPRSCheckOnly";
	public static final String CI_GPRS_AUTO_CONFG = "GPRSConfgOn";
	
	public static final String CI_APN = "APNConfg";
	
    // yaling
    public static final String CI_TDWCDMA_ONLY = "TDWCDMAOnly";
    public static final String CI_TDWCDMA_ONLY_CONFIG = "TDWCDMAOnlyConfig";
    public static final String CI_TDWCDMA_PREFERED_CONFIG = "TDWCDMAPreferredConfig";
    public static final String CI_DUAL_MODE_CONFIG = "DualModeConfig";
    public static final String CI_DUAL_MODE_CHECK = "DualModeCheck";
    public static final String CI_GPRS_ON = "GPRSOn";
    public static final String CI_GPRS_CONFIG = "GPRSalwaysattachConfig";
    public static final String CI_GPRS_ATTACH_CONTINUE = "GPRS always attach continue";
    public static final String CI_GPRS_ATTACH_CHECK = "GPRS always attach continue check";
    public static final String CI_CFU = "CFUcheck";
    public static final String CI_CFU_CONFIG = "CFUConfigOff";
    public static final String CI_CTAFTA = "CTAFTAcheck";
    public static final String CI_CTAFTA_CONFIG_OFF = "CTAFTAConfigOff";
    public static final String CI_CTAFTA_CONFIG_ON = "CTAFTAConfigIntegrityCheckOn";
    public static final String CI_DATA_CONNECT_CHECK = "DataConnectCheck";
    public static final String CI_DATA_CONNECT_OFF = "DataConnectOff";
    public static final String CI_DATA_CONNECT_OFF_CONFIG = "DataConnectOffConfig";
    public static final String CI_DATA_CONNECT_ON = "DataConnectOn";
    public static final String CI_DATA_CONNECT_ON_DM = "DataConnectOnDm";
    public static final String CI_PLMN_DEFAULT = "PLMNDefault";
    public static final String CI_PLMN_DEFAULT_CONFIG = "PLMNDefaultConfig";
    public static final String CI_DT = "CheckDualtalk";
    public static final String CI_DUAL_SIM_CHECK = "CheckDualSIM";
    public static final String CI_SIM_3G_CHECK = "CheckSingleSIM3G";
    public static final String CI_DATA_ROAM = "CheckDataRoam";
    public static final String CI_DATA_ROAM_CONFIG = "CheckDataRoamConfig";
    public static final String CI_DATA_ROAM_OFF_CONFIG = "CheckDataRoamoffConfig";
    // ruoyao
    public static final String CI_RESTORE = "Restore";
	public static final String CI_IMEI_IN_DM = "IMEIInDm";
	public static final String CI_IMEI = "IMEI";
	public static final String CI_DMSTATE_CHECK_ONLY = "DMCheckOnly";
	public static final String CI_DMSTATE_AUTO_CONFG = "DMAutoConfgOn";
	public static final String CI_BATTERY = "Battery";
	public static final String CI_VENDOR_APK = "VendorApk";
	public static final String CI_SMSNUMBER_LAB = "SMSNumberLab";
	public static final String CI_SMSNUMBER_CURR = "SMSNumberCurr";
	public static final String CI_SMSPORT = "SMSPort";
	public static final String CI_DMSERVER_LAB = "DMServerLab";
	public static final String CI_DMSERVER_CURR = "DMServerCurr";
	public static final String CI_DMMANU = "DMManufacturer";
	public static final String CI_DM_AUTO_CHECK = "DMAutoCheck";
	public static final String CI_DMPRODUCT = "DMProduct";
    public static final String CI_DMVERSION = "DMVersion";
    public static final String CI_BUILD_TYPE = "BuildType";
    public static final String CI_FR = "FR";
    public static final String CI_HSUPA = "HSUPA";
    public static final String CI_BASE_FUNC = "BaseFunc";
	
    public static final String CI_TARGET_VERSION = "TargetVersion";
    public static final String CI_TARGET_MODE = "TargetMode";
	public static final String CI_ROOT = "Root";
	public static final String CI_IVSR = "IVSR";
	public static final String CI_IVSR_CHECK_ONLY = "IVSRCheckOnly";
	public static final String CI_ATCI = "ATCI";
	public static final String CI_SERIAL_NUMBER = "SerialNumber";
    public static final String CI_LOGGER_ON = "LoggerOn";
    public static final String CI_LOGGER_OFF = "LoggerOff";
    public static final String CI_TAGLOG_OFF = "Taglogoff";
	public static final String CI_WIFI_ALWAYKEEP = "WifiAlwaysKeep";
	public static final String CI_WIFI_NEVERKEEP = "WifiNeverKeep";
	public static final String CI_WIFI_NEVERKEEP_ONLYCHECK = "WifiNeverKeepOnlyCheck";
	public static final String CI_CTIA_ENABLE = "CTIAEnable";
	public static final String CI_GPRS_DATA_PREF = "GprsDataPreffer";
	public static final String CI_GPRS_CALL_PREF = "GprsCallPreffer";
	
	public static final String CI_UA_BROWSER = "UABrowser";
	public static final String CI_UA_BROWSERURL = "UABrowserURL";
	public static final String CI_UA_MMS = "UAMms";
	public static final String CI_UA_MMSURL = "UAMmsURL";
	public static final String CI_UA_HTTP = "UAHttp";
	public static final String CI_UA_HTTPURL = "UAHttpURL";
	public static final String CI_UA_RTSP = "UARtsp";
    public static final String CI_UA_RTSPURL = "UARtspURL";
    public static final String CI_UA_CMMB = "UACmmb";

    //boli
    public static final String CI_SUPL = "SUPL";
    public static final String CI_SUPL_CHECK_ONLY = "SUPLCheckOnly";
    public static final String CI_SUPL_AUTO_CONFG = "SUPLAutoConfig";

    public static final String CI_LABAPN = "LabAPN";
    public static final String CI_LABAPN_CHECK_MMS = "LabAPNCheckMMS";
    public static final String CI_LABAPN_CHECK_TYPE = "LabAPNCheckType";
    public static final String CI_LABAPN_CHECK_MMS_TYPE = "LabAPNCheckMMSAndType";

    public static final String CI_SMSC = "SMSC";
    public static final String CI_SMSC_CHECK_ONLY = "SMSCCheckOnly";
    public static final String CI_SMSC_AUTO_CONFG = "SMSCAutoConfig";

    public static final String CI_CMWAP = "CMWAP";
    public static final String CI_CMWAP_CHECK_ONLY = "CMWAPCheckOnly";
    public static final String CI_CMWAP_AUTO_CONFG = "CMWAPAutoConfig";

    public static final String CI_CMNET = "CMNET";
    public static final String CI_CMNET_CHECK_ONLY = "CMNETCheckOnly";
    public static final String CI_CMNET_AUTO_CONFG = "CMNETAutoConfig";

    public static final String CI_BT = "BT";
    public static final String CI_BT_CHECK_ONLY = "BTCheckOnly";
    public static final String CI_BT_AUTO_CONFG = "BTAutoConfig";

    public static final String CI_MMS_ROAMING = "MMSRoaming";

    //Yongmao
    public static final String CI_CALIBRATION_DATA = "CalibrationData";
    public static final String CI_APK_CALCULATOR = "Calculator";
    public static final String CI_APK_STOPWATCH = "Stopwatch";
    public static final String CI_APK_OFFICE = "Office";
    public static final String CI_APK_NOTEBOOK = "Notebook";
    public static final String CI_APK_BACKUP = "Backup";
    public static final String CI_APK_CALENDAR = "Calendar";
    public static final String CI_24H_CHARGE_PROTECT = "24hChargeProtect";
    public static final String CI_WLAN_SSID = "WlanSSID";
    public static final String CI_WLAN_MAC_ADDR = "WlanMacAddress";
    public static final String CI_MODEM_SWITCH = "ModemSwitch";

   //2.2
    public static final String CI_BJ_DATA_TIME = "CheckBJTime";
    public static final String CI_SCREEN_ON_SLEEP = "checkscreenonsleep";
    public static final String CI_SCREEN_ON_UNLOCK = "checkscreenonunlock";
    public static final String CI_SCREEN_ROTATE = "checkscreenrotate";
    public static final String CI_BROWSER_URL = "checkUrl";
    public static final String CI_SHORTCUT = "checkshortcut";
    public static final String CI_DEFAULTSTORAGE = "checkDefaultStorage";
    public static final String CI_DEFAULTIME = "checkdefaultime";
    public static final String CI_WIFI_CONTROL = "checkwificontrol";
    public static final String CI_MANUL_CHECK = "checkManualCheck";
    public static final String CI_DISABLED_RED_SCREEN = "disabledRedScreen";
    public static final String CI_ADD_Mo_CALL_LOG = "addMoCallLog";
}

/**
 * @Prompt Add your owner provider below, then put it into ProviderFactory
 */
class OneBtnCheckProvider extends CheckItemProviderBase {
    OneBtnCheckProvider(Context c) {
        mArrayItems.add(new CheckCalData(c, CheckItemKeySet.CI_CALIBRATION_DATA));
        mArrayItems.add(new CheckRestore(c, CheckItemKeySet.CI_RESTORE));
        mArrayItems.add(new CheckBuildType(c, CheckItemKeySet.CI_BUILD_TYPE));
        mArrayItems.add(new CheckTargetMode(c, CheckItemKeySet.CI_TARGET_MODE));
        mArrayItems.add(new CheckTargetVersion(c, CheckItemKeySet.CI_TARGET_VERSION));
        mArrayItems.add(new CheckIMEI(c, CheckItemKeySet.CI_IMEI));
        mArrayItems.add(new CheckGprsMode(c, CheckItemKeySet.CI_GPRS_CALL_PREF));
        mArrayItems.add(new CheckDMState(c, CheckItemKeySet.CI_DMSTATE_CHECK_ONLY));
        mArrayItems.add(new CheckATCI(c, CheckItemKeySet.CI_ATCI));
        mArrayItems.add(new CheckBattery(c, CheckItemKeySet.CI_BATTERY));
        //mArrayItems.add(new CheckWIFISleepPolicy(c, CheckItemKeySet.CI_WIFI_NEVERKEEP_ONLYCHECK));
        mArrayItems.add(new CheckSN(c, CheckItemKeySet.CI_SERIAL_NUMBER));
        mArrayItems.add(new CheckIVSR(c, CheckItemKeySet.CI_IVSR_CHECK_ONLY));
        mArrayItems.add(new CheckVendorApk(c, CheckItemKeySet.CI_VENDOR_APK));
        mArrayItems.add(new CheckDMAuto(c, CheckItemKeySet.CI_DM_AUTO_CHECK));
        mArrayItems.add(new CheckDMManufacturer(c, CheckItemKeySet.CI_DMMANU));
        mArrayItems.add(new CheckDMProduct(c, CheckItemKeySet.CI_DMPRODUCT));
        mArrayItems.add(new CheckDMVersion(c, CheckItemKeySet.CI_DMVERSION));   
        //mArrayItems.add(new CheckFR(c, CheckItemKeySet.CI_FR));
        mArrayItems.add(new CheckHSUPA(c, CheckItemKeySet.CI_HSUPA));
        mArrayItems.add(new CheckBaseFunc(c, CheckItemKeySet.CI_BASE_FUNC));
        mArrayItems.add(new CheckRoot(c, CheckItemKeySet.CI_ROOT));

        // yaling
        mArrayItems.add(new CheckNetworkMode(c, CheckItemKeySet.CI_DUAL_MODE_CHECK));
        mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_ON));
        mArrayItems.add(new CheckCFU(c, CheckItemKeySet.CI_CFU));
        mArrayItems.add(new CheckCTAFTA(c, CheckItemKeySet.CI_CTAFTA));
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_CHECK));
        mArrayItems.add(new CheckPLMN(c, CheckItemKeySet.CI_PLMN_DEFAULT));

        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_BROWSER));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_BROWSERURL));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_MMS));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_MMSURL));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_HTTP));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_HTTPURL));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_RTSP));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_RTSPURL));
        //mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_CMMB));

        //Yongmao
        
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_CALCULATOR));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_STOPWATCH));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_NOTEBOOK));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_BACKUP));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_CALENDAR));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_OFFICE));
        mArrayItems.add(new CheckWlanMacAddr(c, CheckItemKeySet.CI_WLAN_MAC_ADDR));
        mArrayItems.add(new CheckWlanSSID(c, CheckItemKeySet.CI_WLAN_SSID));

        // Bo
        mArrayItems.add(new CheckBT(c, CheckItemKeySet.CI_CMNET_CHECK_ONLY));
    }
}

class TTCN3GProvider extends CheckItemProviderBase {
    TTCN3GProvider(Context c) {
        mArrayItems.add(new CheckNetworkMode(c, CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG));
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)); 
        mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_CONFIG));
        if (FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_DT_SUPPORT == false) {
           mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_ATTACH_CONTINUE));
        }
        mArrayItems.add(new CheckCFU(c, CheckItemKeySet.CI_CFU_CONFIG)); 
        mArrayItems.add(new CheckCTAFTA(c, CheckItemKeySet.CI_CTAFTA_CONFIG_OFF)); 
        mArrayItems.add(new CheckPLMN(c, CheckItemKeySet.CI_PLMN_DEFAULT_CONFIG));
        if (FeatureOption.MTK_WORLD_PHONE) {
           mArrayItems.add(new CheckModemSwitch(c, CheckItemKeySet.CI_MODEM_SWITCH));
        }
    }
}

class Anritsu23GProvider extends CheckItemProviderBase {
    Anritsu23GProvider(Context c) {
        mArrayItems.add(new CheckNetworkMode(c, CheckItemKeySet.CI_DUAL_MODE_CONFIG));
        mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_CONFIG));
        if (FeatureOption.MTK_WORLD_PHONE) {
           mArrayItems.add(new CheckModemSwitch(c, CheckItemKeySet.CI_MODEM_SWITCH));
        }
    }
}

class TTCN23GProvider extends CheckItemProviderBase {
    TTCN23GProvider(Context c) {
        mArrayItems.add(new CheckNetworkMode(c, CheckItemKeySet.CI_DUAL_MODE_CONFIG));
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)); 
        mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_CONFIG));
        if (FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_DT_SUPPORT == false) {
            mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_ATTACH_CONTINUE));
        }
        mArrayItems.add(new CheckCFU(c, CheckItemKeySet.CI_CFU_CONFIG)); 
        mArrayItems.add(new CheckCTAFTA(c, CheckItemKeySet.CI_CTAFTA_CONFIG_OFF)); 
        mArrayItems.add(new CheckPLMN(c, CheckItemKeySet.CI_PLMN_DEFAULT_CONFIG));
        if (FeatureOption.MTK_WORLD_PHONE) {
           mArrayItems.add(new CheckModemSwitch(c, CheckItemKeySet.CI_MODEM_SWITCH));
        }
    }
}

class RRMProvider extends CheckItemProviderBase {
    RRMProvider(Context c) {
        mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_CONFIG));
        if (FeatureOption.MTK_WORLD_PHONE) {
           mArrayItems.add(new CheckModemSwitch(c, CheckItemKeySet.CI_MODEM_SWITCH));
        }
    }
}

class RFProvider extends CheckItemProviderBase {
    RFProvider(Context c) {
        mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_CONFIG));
        if (FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_DT_SUPPORT == false) {
            mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_ATTACH_CONTINUE));
        }
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_OFF_CONFIG)); 
        
        mArrayItems.add(new CheckCFU(c, CheckItemKeySet.CI_CFU_CONFIG)); 
        mArrayItems.add(new CheckCTAFTA(c, CheckItemKeySet.CI_CTAFTA_CONFIG_ON)); 
	}
}

class NetworkIOTProvider extends CheckItemProviderBase {
    NetworkIOTProvider(Context c) {
        mArrayItems.add(new CheckLabAPN(c, CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE));
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_ON));
        mArrayItems.add(new CheckDataRoam(c, CheckItemKeySet.CI_DATA_ROAM_CONFIG));
        mArrayItems.add(new CheckMMSRoaming(c, CheckItemKeySet.CI_MMS_ROAMING));
        if (FeatureOption.MTK_WORLD_PHONE) {
           mArrayItems.add(new CheckModemSwitch(c, CheckItemKeySet.CI_MODEM_SWITCH));
        }
    }
}

class BasicCommunProvider extends CheckItemProviderBase {
    BasicCommunProvider(Context c) {
        mArrayItems.add(new CheckGprsMode(c, CheckItemKeySet.CI_GPRS_DATA_PREF));
	}
}

class PhoneCardCompatProvider extends CheckItemProviderBase {
    PhoneCardCompatProvider(Context c) {
         mArrayItems.add(new CheckGPRSProtocol(c, CheckItemKeySet.CI_GPRS_CONFIG));
         mArrayItems.add(new CheckLabAPN(c, CheckItemKeySet.CI_LABAPN_CHECK_MMS));
        if (FeatureOption.MTK_WORLD_PHONE) {
           mArrayItems.add(new CheckModemSwitch(c, CheckItemKeySet.CI_MODEM_SWITCH));
        }
    }
}

class LocalFuncProvider extends CheckItemProviderBase {
    LocalFuncProvider(Context c) {
        //Yongmao
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_CALCULATOR));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_STOPWATCH));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_NOTEBOOK));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_BACKUP));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_CALENDAR));
        mArrayItems.add(new CheckApkExist(c, CheckItemKeySet.CI_APK_OFFICE));


	}
}

class BroadbandNetProvider extends CheckItemProviderBase {
    BroadbandNetProvider(Context c) {
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_BROWSER));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_BROWSERURL));
        mArrayItems.add(new CheckCurAPN(c, CheckItemKeySet.CI_CMNET));
    }
}

class VideoTelProvider extends CheckItemProviderBase {
    VideoTelProvider(Context c) {
		
	}
}

class StreamingProvider extends CheckItemProviderBase {
    StreamingProvider(Context c) {
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_HTTP));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_HTTPURL));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_RTSP));
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_RTSPURL));
        mArrayItems.add(new CheckCurAPN(c, CheckItemKeySet.CI_CMWAP));
    }
}

class DMLabNetProvider extends CheckItemProviderBase {
    DMLabNetProvider(Context c) {
        mArrayItems.add(new CheckIMEI(c, CheckItemKeySet.CI_IMEI_IN_DM));
        mArrayItems.add(new CheckDMState(c, CheckItemKeySet.CI_DMSTATE_AUTO_CONFG));
        mArrayItems.add(new CheckSMSNumber(c, CheckItemKeySet.CI_SMSNUMBER_LAB));
        mArrayItems.add(new CheckSMSPort(c, CheckItemKeySet.CI_SMSPORT));
        mArrayItems.add(new CheckDMServer(c, CheckItemKeySet.CI_DMSERVER_LAB));
        mArrayItems.add(new CheckDMManufacturer(c, CheckItemKeySet.CI_DMMANU));
        mArrayItems.add(new CheckDMProduct(c, CheckItemKeySet.CI_DMPRODUCT));
        mArrayItems.add(new CheckDMVersion(c, CheckItemKeySet.CI_DMVERSION));
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_ON_DM));  
    }
}

class DMRealNetProvider extends CheckItemProviderBase {
    DMRealNetProvider(Context c) {
        mArrayItems.add(new CheckIMEI(c, CheckItemKeySet.CI_IMEI_IN_DM));
        mArrayItems.add(new CheckDMState(c, CheckItemKeySet.CI_DMSTATE_AUTO_CONFG));
        mArrayItems.add(new CheckSMSNumber(c, CheckItemKeySet.CI_SMSNUMBER_CURR));
        mArrayItems.add(new CheckSMSPort(c, CheckItemKeySet.CI_SMSPORT));
        mArrayItems.add(new CheckDMServer(c, CheckItemKeySet.CI_DMSERVER_CURR));
        mArrayItems.add(new CheckDMManufacturer(c, CheckItemKeySet.CI_DMMANU));
        mArrayItems.add(new CheckDMProduct(c, CheckItemKeySet.CI_DMPRODUCT));
        mArrayItems.add(new CheckDMVersion(c, CheckItemKeySet.CI_DMVERSION));
        mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_ON_DM));
	}
}

class MobileTVProvider extends CheckItemProviderBase {
    MobileTVProvider(Context c) {
        mArrayItems.add(new CheckUA(c, CheckItemKeySet.CI_UA_CMMB));
	}
}

class AgpsProvider extends CheckItemProviderBase {
    AgpsProvider(Context c) {
            mArrayItems.add(new CheckLabAPN(c, CheckItemKeySet.CI_LABAPN_CHECK_MMS));
            mArrayItems.add(new CheckSUPL(c, CheckItemKeySet.CI_SUPL));
            mArrayItems.add(new CheckSMSC(c, CheckItemKeySet.CI_SMSC));
	}
}

class WlanProvider extends CheckItemProviderBase {
    WlanProvider(Context c) {
        mArrayItems.add(new CheckWIFISleepPolicy(c, CheckItemKeySet.CI_WIFI_ALWAYKEEP));
        mArrayItems.add(new CheckCTIA(c, CheckItemKeySet.CI_CTIA_ENABLE));
	}
}

class MTBFProvider extends CheckItemProviderBase {
    MTBFProvider(Context c) {
        //mArrayItems.add(new CheckRoot(c, CheckItemKeySet.CI_ROOT));
        mArrayItems.add(new CheckIVSR(c, CheckItemKeySet.CI_IVSR));
        mArrayItems.add(new CheckMTKLogger(c, CheckItemKeySet.CI_TAGLOG_OFF));
        mArrayItems.add(new CheckSN(c, CheckItemKeySet.CI_SERIAL_NUMBER));

        //Yongmao
        mArrayItems.add(new Check24hChargeProtect(c,
                CheckItemKeySet.CI_24H_CHARGE_PROTECT));
	}
}

class LocalPerfProvider extends CheckItemProviderBase {
    LocalPerfProvider(Context c) {
        mArrayItems.add(new CheckBuildType(c, CheckItemKeySet.CI_BUILD_TYPE));
        //yaling
        mArrayItems.add(new CheckSIMSlot(c, CheckItemKeySet.CI_SIM_3G_CHECK));
    }
}

class PowerConsumpProvider extends CheckItemProviderBase {
    PowerConsumpProvider(Context c) {
        mArrayItems.add(new CheckMTKLogger(c, CheckItemKeySet.CI_LOGGER_OFF));
        mArrayItems.add(new CheckWIFISleepPolicy(c, CheckItemKeySet.CI_WIFI_NEVERKEEP));
        mArrayItems.add(new CheckNetworkMode(c, CheckItemKeySet.CI_TDWCDMA_ONLY_CONFIG));

	}
}

class FieldTestProvider extends CheckItemProviderBase {
    FieldTestProvider(Context c) {
         mArrayItems.add(new CheckNetworkMode(c, CheckItemKeySet.CI_DUAL_MODE_CONFIG));
         mArrayItems.add(new CheckMTKLogger(c, CheckItemKeySet.CI_LOGGER_ON));
         mArrayItems.add(new CheckMTKLogger(c, CheckItemKeySet.CI_TAGLOG_OFF));
         mArrayItems.add(new CheckDTSUPPORT(c, CheckItemKeySet.CI_DT));
         mArrayItems.add(new CheckSIMSlot(c, CheckItemKeySet.CI_DUAL_SIM_CHECK));
    }
}

//class MTKMTBFProvider extends CheckItemProviderBase {
//    MTKMTBFProvider(Context c) {         
//         mArrayItems.add(new CheckBJTime(c, CheckItemKeySet.CI_BJ_DATA_TIME));
//         mArrayItems.add(new CheckScreenOn(c, CheckItemKeySet.CI_SCREEN_ON_SLEEP));
//         mArrayItems.add(new CheckScreenOn(c, CheckItemKeySet.CI_SCREEN_ON_UNLOCK));
//         mArrayItems.add(new CheckScreenRotate(c, CheckItemKeySet.CI_SCREEN_ROTATE));
//         mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_ON));
//         mArrayItems.add(new CheckCurAPN(c, CheckItemKeySet.CI_CMWAP));
//         mArrayItems.add(new CheckWIFIControl(c, CheckItemKeySet.CI_WIFI_NEVERKEEP));
//         mArrayItems.add(new CheckUrl(c, CheckItemKeySet.CI_BROWSER_URL));
//         mArrayItems.add(new CheckShortcut(c, CheckItemKeySet.CI_SHORTCUT));
//        // mArrayItems.add(new CheckDefaultStorage(c, CheckItemKeySet.CI_DEFAULTSTORAGE));
//         mArrayItems.add(new CheckDefaultIME(c, CheckItemKeySet.CI_DEFAULTIME));
//         mArrayItems.add(new CheckDefaultIME(c, CheckItemKeySet.CI_MANUL_CHECK));
//         mArrayItems.add(new CheckDualSIMAsk(c, CheckItemKeySet.CI_MANUL_CHECK));
//         mArrayItems.add(new CheckWebFont(c, CheckItemKeySet.CI_MANUL_CHECK));
//         mArrayItems.add(new CheckLogger(c, CheckItemKeySet.CI_MANUL_CHECK));
//         //mArrayItems.add(new CheckRedScreenOff(c, CheckItemKeySet.CI_MANUL_CHECK));
//         mArrayItems.add(new CheckMMSSetting(c, CheckItemKeySet.CI_MANUL_CHECK));
//         mArrayItems.add(new CheckEmailSetting(c, CheckItemKeySet.CI_MANUL_CHECK));
//         mArrayItems.add(new CheckDefaultStorageSetting(c, CheckItemKeySet.CI_MANUL_CHECK));         
//         mArrayItems.add(new CheckRedScreen(c, CheckItemKeySet.CI_DISABLED_RED_SCREEN));
//    }  
//}


