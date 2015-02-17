ifeq ($(strip $(MTK_JAZZ)),yes)

LOCAL_PATH := $(call my-dir)
MCLD_ROOT_PATH := $(LOCAL_PATH)
# For mcld.mk
LLVM_ROOT_PATH := mediatek/external/llvm
MCLD_ENABLE_ASSERTION := false

include $(CLEAR_VARS)

# MCLinker Libraries
subdirs := \
  lib/ADT \
  lib/ADT/GraphLite \
  lib/CodeGen \
  lib/Core \
  lib/Fragment \
  lib/LD \
  lib/MC \
  lib/Object \
  lib/Script \
  lib/Support \
  lib/Target

# ARM Code Generation Libraries
subdirs += \
  lib/Target/ARM \
  lib/Target/ARM/TargetInfo

# MIPS Code Generation Libraries
subdirs += \
  lib/Target/Mips \
  lib/Target/Mips/TargetInfo

# X86 Code Generation Libraries
subdirs += \
  lib/Target/X86 \
  lib/Target/X86/TargetInfo

include $(MCLD_ROOT_PATH)/mcld.mk
include $(MCLD_ROOT_PATH)/mcld-shared.mk
include $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk, $(subdirs)))

endif  # Jazz