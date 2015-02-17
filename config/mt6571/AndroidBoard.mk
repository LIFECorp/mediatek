LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := mtk-kpd.kcm
include $(BUILD_KEY_CHAR_MAP)

##################################
$(call config-custom-folder,modem:modem)

##### INSTALL MODEM FIRMWARE #####
ifeq ($(strip $(MTK_ENABLE_MD1)),yes)

MD1_IMAGE_2G := $(wildcard $(LOCAL_PATH)/modem/modem_1_2g_n.img)
MD1_IMAGE_WG := $(wildcard $(LOCAL_PATH)/modem/modem_1_wg_n.img)
MD1_IMAGE_TG := $(wildcard $(LOCAL_PATH)/modem/modem_1_tg_n.img)

ifneq ($(strip $(MD1_IMAGE_2G)),)
include $(CLEAR_VARS)
LOCAL_MODULE := modem_1_2g_n.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := catcher_filter_1_2g_n.bin
LOCAL_MODULE_CLASS := ETC 
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
endif # MD1_IMAGE_2G

ifneq ($(strip $(MD1_IMAGE_WG)),)
include $(CLEAR_VARS)
LOCAL_MODULE := modem_1_wg_n.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := catcher_filter_1_wg_n.bin
LOCAL_MODULE_CLASS := ETC 
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
endif # MD1_IMAGE_WG

ifneq ($(strip $(MD1_IMAGE_TG)),)
include $(CLEAR_VARS)
LOCAL_MODULE := modem_1_tg_n.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := catcher_filter_1_tg_n.bin
LOCAL_MODULE_CLASS := ETC 
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
endif # MD1_IMAGE_TG

########INSTALL MODEM_DATABASE########
# for Modem database
ifeq ($(strip $(MTK_INCLUDE_MODEM_DB_IN_IMAGE)), yes)
  ifeq ($(filter generic banyan_addon banyan_addon_x86,$(PROJECT)),)
    MD1_DATABASE_FILE1 := $(foreach m,$(CUSTOM_MODEM),$(if $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_1_2g_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_1_2g_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomApp_*_1_2g_*)))
    $(info 2G database file of md1 = $(MD1_DATABASE_FILE1))
    MD1_DATABASE_FILENAME1 := $(notdir $(MD1_DATABASE_FILE1))
    ifneq ($(strip $(MD1_IMAGE_2G)),)
        $(TARGET_OUT_ETC)/firmware/modem_1_2g_n.img:$(PRODUCT_OUT)/system/etc/mddb/$(MD1_DATABASE_FILENAME1)
        $(eval $(call copy-one-file,$(LOCAL_PATH)/modem/$(MD1_DATABASE_FILENAME1),$(PRODUCT_OUT)/system/etc/mddb/$(MD1_DATABASE_FILENAME1)))
    endif

    MD1_DATABASE_FILE2 := $(foreach m,$(CUSTOM_MODEM),$(if $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_1_wg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_1_wg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomApp_*_1_wg_*)))
    $(info WG database file of md1 = $(MD1_DATABASE_FILE2)) 
    MD1_DATABASE_FILENAME2 := $(notdir $(MD1_DATABASE_FILE2))
    ifneq ($(strip $(MD1_IMAGE_WG)),)
        $(TARGET_OUT_ETC)/firmware/modem_1_wg_n.img:$(PRODUCT_OUT)/system/etc/mddb/$(MD1_DATABASE_FILENAME2)
        $(eval $(call copy-one-file,$(LOCAL_PATH)/modem/$(MD1_DATABASE_FILENAME2),$(PRODUCT_OUT)/system/etc/mddb/$(MD1_DATABASE_FILENAME2)))
    endif

    MD1_DATABASE_FILE3 := $(foreach m,$(CUSTOM_MODEM),$(if $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_1_tg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_1_tg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomApp_*_1_tg_*)))
    $(info TG database file of md1 = $(MD1_DATABASE_FILE3)) 
    MD1_DATABASE_FILENAME3 := $(notdir $(MD1_DATABASE_FILE3))
    ifneq ($(strip $(MD1_IMAGE_TG)),)
        $(TARGET_OUT_ETC)/firmware/modem_1_tg_n.img:$(PRODUCT_OUT)/system/etc/mddb/$(MD1_DATABASE_FILENAME3)
        $(eval $(call copy-one-file,$(LOCAL_PATH)/modem/$(MD1_DATABASE_FILENAME3),$(PRODUCT_OUT)/system/etc/mddb/$(MD1_DATABASE_FILENAME3)))
    endif
  endif
endif

endif # MTK_ENABLE_MD1=yes

ifeq ($(strip $(MTK_ENABLE_MD2)),yes)

MD2_IMAGE_2G := $(wildcard $(LOCAL_PATH)/modem/modem_2_2g_n.img)
MD2_IMAGE_WG := $(wildcard $(LOCAL_PATH)/modem/modem_2_wg_n.img)
MD2_IMAGE_TG := $(wildcard $(LOCAL_PATH)/modem/modem_2_tg_n.img)

ifneq ($(strip $(MD2_IMAGE_2G)),)
include $(CLEAR_VARS)
LOCAL_MODULE := modem_2_2g_n.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := catcher_filter_2_2g_n.bin
LOCAL_MODULE_CLASS := ETC 
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
endif # MD2_IMAGE_2G

ifneq ($(strip $(MD2_IMAGE_WG)),)
include $(CLEAR_VARS)
LOCAL_MODULE := modem_2_wg_n.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := catcher_filter_2_wg_n.bin
LOCAL_MODULE_CLASS := ETC 
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
endif # MD2_IMAGE_WG

ifneq ($(strip $(MD2_IMAGE_TG)),)
include $(CLEAR_VARS)
LOCAL_MODULE := modem_2_tg_n.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := catcher_filter_2_tg_n.bin
LOCAL_MODULE_CLASS := ETC 
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
endif # MD2_IMAGE_TG

########INSTALL MODEM_DATABASE########
# for Modem database
ifeq ($(strip $(MTK_INCLUDE_MODEM_DB_IN_IMAGE)), yes)
  ifeq ($(filter generic banyan_addon banyan_addon_x86,$(PROJECT)),)
    MD2_DATABASE_FILE1 := $(foreach m,$(CUSTOM_MODEM),$(if $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_2_2g_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_2_2g_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomApp_*_2_2g_*)))
    $(info 2G database file of md2 = $(MD2_DATABASE_FILE1))
    MD2_DATABASE_FILENAME1 := $(notdir $(MD2_DATABASE_FILE1))
    ifneq ($(strip $(MD2_IMAGE_2G)),)
        $(TARGET_OUT_ETC)/firmware/modem_2_2g_n.img:$(PRODUCT_OUT)/system/etc/mddb/$(MD2_DATABASE_FILENAME1)
        $(eval $(call copy-one-file,$(LOCAL_PATH)/modem/$(MD2_DATABASE_FILENAME1),$(PRODUCT_OUT)/system/etc/mddb/$(MD2_DATABASE_FILENAME1)))
    endif

    MD2_DATABASE_FILE2 := $(foreach m,$(CUSTOM_MODEM),$(if $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_2_wg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_2_wg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomApp_*_2_wg_*)))
    $(info WG database file of md2 = $(MD2_DATABASE_FILE2)) 
    MD2_DATABASE_FILENAME2 := $(notdir $(MD2_DATABASE_FILE2))
    ifneq ($(strip $(MD2_IMAGE_WG)),)
        $(TARGET_OUT_ETC)/firmware/modem_2_wg_n.img:$(PRODUCT_OUT)/system/etc/mddb/$(MD2_DATABASE_FILENAME2)
        $(eval $(call copy-one-file,$(LOCAL_PATH)/modem/$(MD2_DATABASE_FILENAME2),$(PRODUCT_OUT)/system/etc/mddb/$(MD2_DATABASE_FILENAME2)))
    endif

    MD2_DATABASE_FILE3 := $(foreach m,$(CUSTOM_MODEM),$(if $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_2_tg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomAppSrcP_*_2_tg_*), \
       $(wildcard mediatek/custom/common/modem/$(strip $(m))/BPLGUInfoCustomApp_*_2_tg_*)))
    $(info TG database file of md2 = $(MD2_DATABASE_FILE3)) 
    MD2_DATABASE_FILENAME3 := $(notdir $(MD2_DATABASE_FILE3))
    ifneq ($(strip $(MD2_IMAGE_TG)),)
        $(TARGET_OUT_ETC)/firmware/modem_2_tg_n.img:$(PRODUCT_OUT)/system/etc/mddb/$(MD2_DATABASE_FILENAME3)
        $(eval $(call copy-one-file,$(LOCAL_PATH)/modem/$(MD2_DATABASE_FILENAME3),$(PRODUCT_OUT)/system/etc/mddb/$(MD2_DATABASE_FILENAME3)))
    endif
  endif
endif
#############################################

endif # MTK_ENABLE_MD2=yes


