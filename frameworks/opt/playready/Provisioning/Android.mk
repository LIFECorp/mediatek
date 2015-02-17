################################################################################
# prebuild bgroupcert.dat
################################################################################

ifeq ($(strip $(MTK_PLAYREADY_SUPPORT)),yes)
LOCAL_PATH:= $(call my-dir)

#ALL_DEFAULT_INSTALLED_MODULES + = bgroupcert.dat
include $(CLEAR_VARS)
LOCAL_MODULE := bgroupcert.dat
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/DxDrm/Credentials/PlayReady
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

################################################################################
# prebuild devcerttemplate.dat
################################################################################
#ALL_DEFAULT_INSTALLED_MODULES + = devcerttemplate.dat
include $(CLEAR_VARS)
LOCAL_MODULE := devcerttemplate.dat
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/DxDrm/Credentials/PlayReady
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

################################################################################
# prebuild priv.dat
################################################################################
#ALL_DEFAULT_INSTALLED_MODULES + = priv.dat
include $(CLEAR_VARS)
LOCAL_MODULE := priv.dat
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/DxDrm/Credentials/PlayReady
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

################################################################################
# prebuild zgpriv.dat
################################################################################
#ALL_DEFAULT_INSTALLED_MODULES + = zgpriv.dat
include $(CLEAR_VARS)
LOCAL_MODULE := zgpriv.dat
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/DxDrm/Credentials/PlayReady
#LOCAL_STRIP_MODULE := true

include $(BUILD_PREBUILT)

endif
