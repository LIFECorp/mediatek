LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_pq.cpp 

LOCAL_C_INCLUDES += \
        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include \
        $(MTK_PATH_PLATFORM)/../../hardware/dpframework/inc \
        $(MTK_PATH_PLATFORM)/kernel/drivers/dispsys

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \

LOCAL_MODULE:= pq

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))