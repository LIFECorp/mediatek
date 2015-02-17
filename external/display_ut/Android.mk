LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES = \
    display_session_test.cpp

LOCAL_C_INCLUDES += \
 	$(MTK_PATH_SOURCE)/kernel/include \
 	$(MTK_PATH_SOURCE)/kernel/drivers/video \
  $(TOPDIR)/kernel/include \
  $(TOPDIR)/system/core/include \

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := session_test

LOCAL_SHARED_LIBRARIES := libcutils libc  libion libsync   libutils \
    libbinder \
    libgui \
    libpng \
    libz 
include $(BUILD_EXECUTABLE)
