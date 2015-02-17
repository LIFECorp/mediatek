# Copyright 2006 The Android Open Source Project
ifeq ($(MTK_EXTERNAL_DONGLE_SUPPORT),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	rild.c


LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libril_dongle 

ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm


LOCAL_CFLAGS := -DRIL_SHLIB

LOCAL_MODULE:= rild_dongle
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endif #(($(MTK_EXTERNAL_DONGLE_SUPPORT),yes)

ifeq ($(MTK_EXTERNAL_DONGLE_SUPPORT),yes)
# For radiooptions binary
# =======================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	radiooptions.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := \

LOCAL_MODULE:= radiooptions_dongle
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)

endif #(($(MTK_EXTERNAL_DONGLE_SUPPORT),yes)