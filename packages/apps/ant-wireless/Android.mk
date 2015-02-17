$(warning [MTK_ANT] feature_option=$(MTK_ANT_SUPPORT))
ifeq ($(MTK_ANT_SUPPORT),yes)

BOARD_ANT_WIRELESS_DEVICE = vfs-prerelease

# Use the folloing include to make our test apk.
include $(call all-subdir-makefiles)

endif