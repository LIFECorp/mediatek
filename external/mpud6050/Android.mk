#
# InvenSense MPU6050 Daemon
#
ifeq ($(CUSTOM_KERNEL_GYROSCOPE), mpu6050gy-new)

LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libmllitd.so libmplmpd.so

include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_EXECUTABLES := mpud6050
#LOCAL_MODULE_TAGS := user

include $(BUILD_MULTI_PREBUILT)

endif
