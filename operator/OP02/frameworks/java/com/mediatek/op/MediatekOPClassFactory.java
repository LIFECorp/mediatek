package com.mediatek.op;

import java.util.HashMap;
import java.util.Map;


import android.os.SystemClock;
import android.util.Log;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.bootanim.IBootAnimExt;
import com.mediatek.common.sms.IWapPushFwkExt;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.telephony.ITelephonyExt;
import com.mediatek.common.telephony.ITelephonyProviderExt;
import com.mediatek.common.telephony.IGsmDCTExt;

public final class MediatekOPClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekOPClassFactory";

    // mediatek-op.jar public interface map used for interface class matching.
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(IBootAnimExt.class, "com.mediatek.op.bootanim.BootAnimExtOP02");
        opInterfaceMap.put(ITelephonyExt.class, "com.mediatek.op.telephony.TelephonyExtOP02");
        opInterfaceMap.put(ITelephonyProviderExt.class, "com.mediatek.op.telephony.TelephonyProviderExtOP02");
        opInterfaceMap.put(IGsmDCTExt.class, "com.mediatek.op.telephony.GsmDCTExtOP02");
        ///For CU feature,don't show ECC when can't call 120
        opInterfaceMap.put(IServiceStateExt.class, "com.mediatek.op.telephony.ServiceStateExtOP02");
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
