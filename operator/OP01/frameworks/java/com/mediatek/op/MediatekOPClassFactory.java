
package com.mediatek.op;

import java.util.HashMap;
import java.util.Map;

import android.os.SystemClock;
import android.util.Log;

import com.mediatek.common.net.IConnectivityServiceExt;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.telephony.IPhoneNumberExt;
import com.mediatek.common.telephony.IGsmDCTExt;
import com.mediatek.common.util.IPatterns;
import com.mediatek.common.audioprofile.IAudioProfileExtension;
import com.mediatek.common.bootanim.IBootAnimExt;
import com.mediatek.common.media.IOmaSettingHelper;
import com.mediatek.common.media.IAudioServiceExt;
import com.mediatek.common.sms.IDupSmsFilterExt;

/// M: Account sync start
import com.mediatek.common.accountsync.ISyncManagerExt;
/// M: Account sync feature end @}


public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOP01ClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(IPatterns.class, "com.mediatek.op.util.OP01Patterns");
        opInterfaceMap.put(IWifiFwkExt.class, "com.mediatek.op.wifi.WifiFwkExtOP01");
        opInterfaceMap.put(IBootAnimExt.class, "com.mediatek.op.bootanim.BootAnimExtOP01");
        opInterfaceMap.put(IServiceStateExt.class, "com.mediatek.op.telephony.ServiceStateExtOP01");
        opInterfaceMap.put(IPhoneNumberExt.class, "com.mediatek.op.telephony.PhoneNumberExtOP01");
        opInterfaceMap.put(IGsmDCTExt.class, "com.mediatek.op.telephony.GsmDCTExtOP01");

        opInterfaceMap.put(IAudioServiceExt.class, "com.mediatek.op.media.AudioServiceExtOP01");
        opInterfaceMap.put(IOmaSettingHelper.class, "com.mediatek.op.media.OmaSettingHelperOP01");
        opInterfaceMap.put(IConnectivityServiceExt.class, "com.mediatek.common.op.net.ConnectivityServiceExt");
        opInterfaceMap.put(IAudioProfileExtension.class, "com.mediatek.op.audioprofile.AudioProfileExtensionOP01");
        opInterfaceMap.put(IAudioProfileExtension.IDefaultProfileStatesGetter.class,
            "com.mediatek.op.audioprofile.DefaultProfileStatesGetterOP01");
        opInterfaceMap.put(ISyncManagerExt.class,
                "com.mediatek.op.accountsync.SyncManagerOPExt");
        opInterfaceMap.put(IDupSmsFilterExt.class, "com.mediatek.op.sms.DupSmsFilterExtOP01");
    }


    /**
     * Get the op specific class name.
     * 
     * 
     */
    public static String getOpIfClassName(Class<?> clazz) {
        String ifClassName = null;

        if (opInterfaceMap.containsKey(clazz)) {
            ifClassName = opInterfaceMap.get(clazz);
        }

        return ifClassName;
    }
}
