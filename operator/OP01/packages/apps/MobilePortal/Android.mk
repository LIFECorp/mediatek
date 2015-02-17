LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
 	$(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := MobilePortal
LOCAL_CERTIFICATE := shared

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/operator/app
LOCAL_DEX_PREOPT:= false

include $(BUILD_PACKAGE)