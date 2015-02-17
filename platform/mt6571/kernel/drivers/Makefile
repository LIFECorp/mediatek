include $(MTK_PATH_BUILD)/common.mk
#Fix-me: modified for early porting
#$(call all-subdir-src-or-makefile)
obj-y   += uart/
obj-y   += gpio/
obj-y   += pwm/
ifneq	($(MTK_GPU_SUPPORT),no)
obj-y   += gpu/
endif
obj-y   += auxadc/
obj-y   += btcvsd/
obj-y   += pmic_wrap/
obj-y   += power/
obj-y   += m4u/
obj-y   += smi/
obj-y   += usb20/
obj-y   += usb11/
obj-y   += mmc-host/
obj-y   += sound/
ifneq ($(MTK_EMMC_SUPPORT),yes)
obj-y   += nand/
endif
obj-y   += keypad/
obj-y   += ../../../common/kernel/drivers/video/
obj-y   += video/
ifneq ($(strip $(FPGA_EARLY_PORTING)),yes)
obj-y   += touchpanel/
obj-y   += spi/
obj-$(CONFIG_MTK_LDVT)   += ldvt/
obj-y   += mmc-host/
endif
obj-y   += rtc/

obj-$(CONFIG_MTK_VIDEOCODEC_DRIVER)   += videocodec/
obj-y   += i2c/
ifneq ($(MTK_PROJECT), mt6571fpga_ldvt)
obj-y   += dispsys/
endif
obj-y   += dual_ccci/
obj-y   += accdet/
obj-y   += vibrator/
obj-y   += leds/
obj-y   += wdt/
obj-y	+= devinfo/
obj-y	+= masp/
obj-y	+= devapc/
obj-$(CONFIG_THERMAL) += thermal/
obj-$(CONFIG_MTK_ETM) += mt_etm/
include $(MTK_PATH_BUILD)/common.mk
obj-y   += $(call all-subdirs-with-makefile,cpu_stress)

ifeq ($(HAVE_CMMB_FEATURE),yes)
obj-m += cmmbdrv/
endif
obj-y   += btif/
ifneq ($(CONFIG_MTK_COMBO),)
ifneq ($(filter CONSYS_6571,$(MTK_COMBO_CHIP)),)
obj-y   += wmt/
endif
endif
obj-y   += pmt/
obj-y   += mrdump/

obj-y += $(call all-subdirs-with-makefile,met)

ifeq ($(MTK_NFC_SUPPORT),yes)
obj-y	+= nfc/
endif
