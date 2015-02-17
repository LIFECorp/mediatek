################################################################################
# prebuild 04040404040404040404040404040404.tlbin
################################################################################

ifeq ($(strip $(MTK_PLAYREADY_SUPPORT)),yes)
LOCAL_PATH:= $(call my-dir)
#ALL_DEFAULT_INSTALLED_MODULES + = 04040404040404040404040404040404.tlbin
include $(CLEAR_VARS)
LOCAL_MODULE := 04040404040404040404040404040404.tlbin
LOCAL_MODULE_CLASS := ETC

LOCAL_SRC_FILES := release/04040404040404040404040404040404.tlbin
#ifeq ($(TEE_MODE),Debug)
    #LOCAL_SRC_FILES := debug/04040404040404040404040404040404.tlbin
#else 
    #LOCAL_SRC_FILES := release/04040404040404040404040404040404.tlbin
#endif

LOCAL_MODULE_PATH := $(TARGET_OUT_APPS)/mcRegistry

include $(BUILD_PREBUILT)
endif
