libmcld_STATIC_LIBRARIES += \
  libmcldCodeGen \
  libmcldTarget \
  libmcldLDVariant \
  libmcldMC \
  libmcldObject \
  libmcldFragment \
  libmcldScript \
  libmcldCore \
  libmcldSupport \
  libmcldADTGraphLite \
  libmcldADT \
  libmcldLD

libmcld_arm_STATIC_LIBRARIES += \
  libmcldARMTarget \
  libmcldARMInfo

libmcld_mips_STATIC_LIBRARIES += \
  libmcldMipsTarget \
  libmcldMipsInfo

libmcld_x86_STATIC_LIBRARIES += \
  libmcldX86Target \
  libmcldX86Info

#=====================================================================
# Host Shared Library libmcld
#=====================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libmcld
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_WHOLE_STATIC_LIBRARIES += \
  $(libmcld_arm_STATIC_LIBRARIES) \
  $(libmcld_mips_STATIC_LIBRARIES) \
  $(libmcld_x86_STATIC_LIBRARIES) \
  $(libmcld_STATIC_LIBRARIES)

LOCAL_SHARED_LIBRARIES := libLLVM

include $(MCLD_HOST_BUILD_MK)
include $(BUILD_HOST_SHARED_LIBRARY)

#=====================================================================
# Host Shared Library libmcld
#=====================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libmcld
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

libmcld_target_STATIC_LIBRARIES := $(libmcld_$(TARGET_ARCH)_STATIC_LIBRARIES)

ifeq ($(libmcld_target_STATIC_LIBRARIES),)
  $(error Unsupported TARGET_ARCH $(TARGET_ARCH))
endif

LOCAL_WHOLE_STATIC_LIBRARIES += \
  $(libmcld_target_STATIC_LIBRARIES) \
  $(libmcld_STATIC_LIBRARIES)

LOCAL_SHARED_LIBRARIES := libLLVM libstlport

include $(MCLD_DEVICE_BUILD_MK)
include $(BUILD_SHARED_LIBRARY)
