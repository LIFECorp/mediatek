
ifeq ($(MTK_WLAN_SUPPORT),yes)


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libsysutils libcutils

LOCAL_SRC_FILES:= loader.c

LOCAL_MODULE:= wlan_loader

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif
