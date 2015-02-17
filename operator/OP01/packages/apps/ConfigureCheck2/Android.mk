LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := ConfigureCheck2

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := CustomProperties
LOCAL_JAVA_LIBRARIES += mediatek-framework telephony-common

include $(call all-makefiles-under,$(LOCAL_PATH))
include $(BUILD_PACKAGE)
