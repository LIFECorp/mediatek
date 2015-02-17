################################################################################
# prebuild DrmAssist-Recommended.apk
################################################################################

ifeq ($(strip $(MTK_PLAYREADY_SUPPORT)),yes)
#don't build this file to system/app

#LOCAL_PATH:= $(call my-dir)
#include $(CLEAR_VARS)

#LOCAL_MODULE := DrmAssist-Recommended
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := APPS
#LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
#LOCAL_MODULE_SUFFIX :=  $(COMMON_ANDROID_PACKAGE_SUFFIX)
#LOCAL_MODULE_PATH := $(TARGET_OUT_APPS)
#LOCAL_CERTIFICATE := PRESIGNED

#include $(BUILD_PREBUILT)
endif