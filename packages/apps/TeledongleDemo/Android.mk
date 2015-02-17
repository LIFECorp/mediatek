LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := TeledongleDemo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := tedongle-telephony

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

# Flag for LCA ram optimize.
ifeq (yes, strip$(MTK_LCA_RAM_OPTIMIZE))
    LOCAL_AAPT_FLAGS := --utf16
endif

include $(BUILD_PACKAGE)

# Build the test package
include $(call all-makefiles-under,$(LOCAL_PATH))
