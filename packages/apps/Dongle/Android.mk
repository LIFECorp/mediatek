LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#ifeq ($(MTK_BT_SUPPORT), yes)
### generate AndroidManifest.xml
##$(warning $(LOCAL_PATH)/build/blueangel.py])
##PY_RES := $(shell python $(LOCAL_PATH)/build/blueangel.py)
##endif

LOCAL_SRC_FILES := $(call all-java-files-under, src)



LOCAL_PACKAGE_NAME := Dongle
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
#LOCAL_STATIC_JAVA_LIBRARIES := com.android.phone.common 

#LOCAL_JAVA_LIBRARIES := telephony-common
#LOCAL_JAVA_LIBRARIES += mediatek-framework
#LOCAL_JAVA_LIBRARIES += mediatek-common

LOCAL_JAVA_LIBRARIES := tedongle-telephony

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

# Flag for LCA ram optimize.
ifeq (yes, strip$(MTK_LCA_RAM_OPTIMIZE))
    LOCAL_AAPT_FLAGS := --utf16
endif

include $(BUILD_PACKAGE)

# Build the test package
include $(call all-makefiles-under,$(LOCAL_PATH))
