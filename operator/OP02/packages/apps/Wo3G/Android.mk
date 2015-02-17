# Copyright 2007-2008 The Android Open Source Project


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional


LOCAL_SRC_FILES := \
 	$(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := Wo3G
LOCAL_CERTIFICATE := shared
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/operator/app
LOCAL_DEX_PREOPT := false

ifeq ($(strip $(MTK_CIP_SUPPORT)), yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
endif

include $(BUILD_PACKAGE)
