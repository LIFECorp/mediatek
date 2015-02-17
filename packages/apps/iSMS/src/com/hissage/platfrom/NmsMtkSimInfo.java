package com.hissage.platfrom;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

import android.content.Context;
import android.provider.Telephony.SIMInfo;

import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsMtkSimInfo extends NmsPlatformBase {

    private static final String TAG = "NmsMtkSimInfo";
    protected Class<?> mSystemSimManager = null;
    protected Class<?> mSystemSimInfoClass = null;
    protected Class<?> mMessageUtilsClass = null;
    private static final String MANAGER_CLASS_PATH = "com.mediatek.telephony.SimInfoManagerAdp";
    private static final String INFO_CLASS_PATH = "com.mediatek.encapsulation.android.telephony.EncapsulatedSimInfoManager";
    private static final String MESSAGE_UTILS_CLASS_PATH = "com.android.mms.ui.MessageUtils";
    private Context mMMSContext = null;

    protected Class<?> mEncapsulatedFeatureOptionClass = null;
    private static final String ENCAPSULATED_FEATURE_OPTION_CLASS_PATH = "com.mediatek.encapsulation.com.mediatek.common.featureoption.EncapsulatedFeatureOption";
    private boolean MTK_GEMINI_SUPPORT = false;

    public NmsMtkSimInfo(Context context) {
        super(context);
        try {
            mMMSContext = context.createPackageContext("com.android.mms",
                    Context.CONTEXT_IGNORE_SECURITY | Context.CONTEXT_INCLUDE_CODE);
        } catch (Exception e) {
            NmsLog.warn(TAG, e.toString());
        }
        try {

            mSystemSimManager = Class.forName(MANAGER_CLASS_PATH, true,
                    mMMSContext.getClassLoader());
            mSystemSimInfoClass = Class.forName(INFO_CLASS_PATH, true,
                    mMMSContext.getClassLoader());
            mPlatfromMode = NMS_INTEGRATION_MODE;
        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.warn(TAG, e.toString());
        }
    }

    public int getColor(Context context, long simId) {
 if (mSystemSimInfoClass != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
            		int realColor = -1;
                int color = -1;
                Method method = mSystemSimInfoClass.getMethod("getSIMInfoById", Context.class,
                        long.class);

                Field field = mSystemSimInfoClass.getField("mSimBackgroundRes");
                field.setAccessible(true);

                Method method1 = mSystemSimInfoClass.getMethod("getSimBackgroundRes");

                Object simInfo = (Object) method.invoke(mSystemSimInfoClass, context, simId);
                if (simInfo != null) {
                    color = (Integer) field.get(simInfo);
                    realColor = (Integer)method1.invoke(simInfo);
                }
                
                NmsLog.trace(TAG, "getColor, return " + simId + ", platfrom mode: "
                        + getModeString() + ", color:" + color + ", realColor: " + realColor);
                return realColor;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return 0;
            }
        } else {
            return 0;
        }

    }


    public String getName(Context context, long simId) {
        if (mSystemSimInfoClass != null && NMS_INTEGRATION_MODE == mPlatfromMode
                && mSystemSimManager != null) {
            try {
                String name = null;
                Method method = mSystemSimManager.getMethod("getSimInfoByIdAdp", Context.class,
                        long.class);
                if (method != null) {
                    SIMInfo info = (SIMInfo) method.invoke(null, context, simId);
                    name = info.mDisplayName;
                }
                NmsLog.trace(TAG, "getName, return " + simId + ", platfrom mode: "
                        + getModeString());
                return name;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return null;
            }
        } else {
            return null;
        }
    }

    public int getSlotIdBySimId(Context context, long simId) {
        if (mSystemSimInfoClass != null && NMS_INTEGRATION_MODE == mPlatfromMode
                && mSystemSimManager != null) {
            try {
                int slotId = -2;
                Method method = mSystemSimManager.getMethod("getSimInfoByIdAdp", Context.class,
                        long.class);
                if (method != null) {
                    SIMInfo info = (SIMInfo) method.invoke(null, context, simId);
                    slotId = info.mSlot;
                }
                NmsLog.trace(TAG, "getSlotIdBySimId, return: " + slotId + ", platfrom mode: "
                        + getModeString());
                return slotId;
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
                return 0;
            }
        } else {
            return 0;
        }
    }

    public long getSimIdBySlot(Context context, int soltId) {
        if (mSystemSimInfoClass != null && NMS_INTEGRATION_MODE == mPlatfromMode
                && mSystemSimManager != null) {
            try {
                long simId = NmsConsts.INVALID_SIM_ID;

                Method method = mSystemSimManager.getMethod("getSimInfoBySlotAdp", Context.class,
                        int.class);
                if (method != null) {
                    SIMInfo info = (SIMInfo) method.invoke(null, context, soltId);
                    simId = info.mSimId;
                }
                NmsLog.trace(TAG, "getSimIdBySlot, return " + simId + ", platfrom mode: "
                        + getModeString());
                return simId;
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
                return 0;
            }
        } else {
            if (soltId == 0) {
                return getGoogleDefaultSimId(context);
            } else {
                return 0;
            }
        }
    }

    public CharSequence getSimIndicator(int simId) {
        if (mMMSContext != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                if (mMessageUtilsClass == null) {
                    mMessageUtilsClass = Class.forName(MESSAGE_UTILS_CLASS_PATH, true,
                            mMMSContext.getClassLoader());
                }

                if (mMessageUtilsClass != null) {
                    Method method = mMessageUtilsClass.getMethod("getSimInfoSync", Context.class,
                            int.class);
                    return (CharSequence) method.invoke(null, mMMSContext, simId);
                }
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
                return null;
            }
        }
        return null;
    }

    public boolean isMtkGeminiSupport() {
//        if (mMMSContext != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
//            try {
//                if (mEncapsulatedFeatureOptionClass != null) {
//                    return MTK_GEMINI_SUPPORT;
//                } else {
//                    if (mEncapsulatedFeatureOptionClass == null) {
//                        mEncapsulatedFeatureOptionClass = Class.forName(
//                                ENCAPSULATED_FEATURE_OPTION_CLASS_PATH, true,
//                                mMMSContext.getClassLoader());
//                    }
//
//                    if (mEncapsulatedFeatureOptionClass != null) {
//                        Field field0 = mEncapsulatedFeatureOptionClass
//                                .getField("MTK_GEMINI_SUPPORT");
//                        MTK_GEMINI_SUPPORT = (Boolean) field0.get(mEncapsulatedFeatureOptionClass);
//                        NmsLog.trace(TAG, "iSMS MTK_GEMINI_SUPPORT: " + MTK_GEMINI_SUPPORT);
//                        return MTK_GEMINI_SUPPORT;
//                    }
//                }
//
//            } catch (Exception e) {
//                mEncapsulatedFeatureOptionClass = null;
//                NmsLog.error(TAG, "getMtkGeminiSupport: " + e.toString());
//            }
//        }
        return true;
    }
}
