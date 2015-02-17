# limitations under the License.
#

# This makefile shows how to build your own shared library that can be
# shipped on the system of a phone, and included additional examples of
# including JNI code with the library and writing client applications against it.

LOCAL_PATH := $(call my-dir)

# MediaTek op library.
# =============================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := mediatek-op

LOCAL_SRC_FILES := $(call all-java-files-under,java)

LOCAL_JAVA_LIBRARIES := mediatek-framework telephony-common

# Disable DEX pre-optimization for dynamic OP substitution.
LOCAL_DEX_PREOPT := false

# Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/framework
endif

include $(BUILD_JAVA_LIBRARY)

ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
# Define symlink path (i.e. /system/framework/mediatek-op.jar).
SYMLINKS := $(TARGET_OUT_JAVA_LIBRARIES)/mediatek-op.jar

# Target that makes the symlink after mediatek-op is built.
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink $@ -> ../../custom/framework/$(notdir $<)"
	mkdir -p $(dir $@)
	rm -rf $@
	$(hide) ln -sf ../../custom/framework/$(notdir $<) $@

# We need this so that the installed files could be picked up based on the
# local module name.
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(SYMLINKS)
endif
