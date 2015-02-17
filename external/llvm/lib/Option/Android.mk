LOCAL_PATH:= $(call my-dir)

option_SRC_FILES := \
  Arg.cpp \
  ArgList.cpp \
  OptTable.cpp \
  Option.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(option_SRC_FILES)

LOCAL_MODULE:= libLLVMOption

LOCAL_MODULE_TAGS := optional

include $(LLVM_HOST_BUILD_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(option_SRC_FILES)

LOCAL_MODULE:= libLLVMOption

LOCAL_MODULE_TAGS := optional

include $(LLVM_DEVICE_BUILD_MK)
include $(BUILD_STATIC_LIBRARY)
