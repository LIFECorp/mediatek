ifeq ($(MTK_EMMC_SUPPORT), yes)
##### EMMC META library #####
BUILD_CLR_EMMC := true

ifeq ($(BUILD_CLR_EMMC), true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := meta_clr_emmc.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_STATIC_LIBRARIES := libstorageutil
LOCAL_CFLAGS += -DUSE_EXT4
LOCAL_C_INCLUDES += system/extras/ext4_utils
LOCAL_STATIC_LIBRARIES += libext4_utils_static
LOCAL_MODULE := libmeta_clr_emmc
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)
endif
endif

ifeq ($(MTK_EMMC_SUPPORT), no)
##### NAND META library #####
BUILD_CLR_NAND := true

ifeq ($(BUILD_CLR_NAND), true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := meta_clr_nand.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/meta/common/inc
LOCAL_SHARED_LIBRARIES := libcutils libc
LOCAL_STATIC_LIBRARIES := libstorageutil
#LOCAL_CFLAGS += -DUSE_EXT4
#LOCAL_C_INCLUDES += system/extras/ext4_utils
#LOCAL_STATIC_LIBRARIES += libext4_utils libz
LOCAL_MODULE := libmeta_clr_emmc
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)
endif
endif
