## ==> build this lib only when cmmb feature is yes
ifeq ($(HAVE_CMMB_FEATURE),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
       CmmbHelper.cpp \
       ftm_cmmb_impl.cpp \
       meta_cmmb.c 

LOCAL_C_INCLUDES := $(KERNEL_HEADERS) \
	$(MTK_PATH_SOURCE)/external/meta/common/inc \
        $(LOCAL_PATH)/include 

## ==>  Innofidei Chip used
ifneq (,$(findstring Innofidei,$(MTK_CMMB_CHIP)))
LOCAL_SRC_FILES += \
        innofidei/InnoAppDriver.cpp \
        innofidei/CmmbAdapterInno.cpp 

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/innofidei 

LOCAL_CFLAGS := -DCMMB_CHIP_INNO 
# siano chip used
else    
LOCAL_SRC_FILES += \
       siano/CmmbAdapterSiano.cpp \
       siano/siano_appdriver_new/SmsAdrLiteLinux.c \
       siano/hostlib/SmsHostlibLiteCmmb.cpp \
       siano/hostlib/SmsHostLibLiteCommon.cpp \
       siano/hostlib/SmsHostUtils.c \
       siano/osw/linux/Osw_Event.c \
       siano/osw/linux/Osw_FileSystem.c \
       siano/osw/linux/Osw_Init.c \
       siano/osw/linux/Osw_Memory.c \
       siano/osw/linux/Osw_Mutex.c \
       siano/osw/linux/Osw_Task.c \
       siano/osw/linux/Osw_Time.c 

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/siano/hostlib \
        $(LOCAL_PATH)/siano/osw/include \
        $(LOCAL_PATH)/siano/osw/linux \
        $(LOCAL_PATH)/siano/siano_appdriver_new
endif  
## <== Innofidei Chip used

LOCAL_MODULE:= libmeta_cmmb

LOCAL_SHARED_LIBRARIES := 


include $(BUILD_STATIC_LIBRARY)

endif
## <== build this lib only when CMCC compile option is yes
