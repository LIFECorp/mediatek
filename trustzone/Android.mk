LOCAL_PATH := $(call my-dir)
include $(call all-makefiles-under,$(LOCAL_PATH))

include $(call all-makefiles-under,$(LOCAL_PATH)/vendor/*)
