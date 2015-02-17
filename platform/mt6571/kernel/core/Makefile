#
# Makefile for the linux kernel.
#
ifeq ($(MTK_INTERNAL), yes)
ifeq ($(MTK_USE_RESERVED_EXT_MEM), yes)
KBUILD_CFLAGS += -DMTK_USE_RESERVED_EXT_MEM
endif
endif

obj-y := core.o irq.o timer.o mt_dbg.o
obj-y += mt_devs.o mt_gpt.o mt_boot.o board.o mt_pm_init.o mt_fixup.o
obj-y += ca7_timer.o
obj-y += mt_ahb_abt.o
obj-y += mt_reg_dump.o
ifeq ($(CONFIG_SMP),y)
obj-y += mt-smp.o hotplug.o mt-headsmp.o mt_hotplug.o mt_hotplug_mechanism.o
ccflags-y += -I$(srctree)
endif
obj-y += eint.o mt_freqhopping.o
obj-y += mt_device_apc.o
obj-$(CONFIG_MT65XX_TRACER) += pmu_v7.o mon_interface.o mt_mon.o mt_emi_bm.o
obj-y += emi_bwl.o emi_mpu.o
obj-y += camera_isp.o
obj-y += camera_pipe_mgr.o
#obj-y += camera_eis.o
#obj-y += camera_fdvt.o
obj-y += mt_pm_ldo.o
obj-y += innercache.o mt_cache_dump.o

obj-$(CONFIG_MTK_GDMA) += mt_dma.o

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power \
             -I$(MTK_PATH_SOURCE)/drivers/wdk
obj-y += mt_spm.o mt_spm_pcm.o mt_spm_suspend.o mt_spm_dpidle.o mt_spm_mcdi.o mt_spm_sodi.o mt_spm_mtcmos.o mt_spm_mcdi_ldvt.o spm_fs.o mt_spm_wdt.o mt_spm_normal.o
obj-y += mt_sleep.o
obj-y += mt_cpufreq.o
ifneq ($(MTK_GPU_SUPPORT), no)
ccflags-y += -DMTK_GPU_SUPPORT
endif
obj-y += mt_gpufreq.o
obj-y += mt_cpu_ss.o
obj-y += mt_dormant.o cpu_dormant.o
obj-y += mt_dcm.o mt_idle.o
obj-y += mt_clkmgr.o mt_clkmgr_common.o
obj-y += mt_golden_setting.o
obj-y += mt_ptp.o
obj-y += sbchk_base.o
obj-$(CONFIG_MTK_CIRQ) += mt_cirq.o
obj-y += mtk_ccci_helper.o
obj-y += mssv.o

ifneq ($(CONFIG_MTK_CCCI_DEVICES),y)
ccflags-y  += -D__USING_DUMMY_CCCI_API__
endif

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power \
             -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/include \
             -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/platform \
             -I$(MTK_PATH_PLATFORM)/drivers/devinfo \
             -I$(MTK_PATH_SOURCE)/drivers/wdk

# Hibernation
AFLAGS_swsusp.o	:= -DTEXT_OFFSET=$(TEXT_OFFSET)
obj-$(CONFIG_HIBERNATION) += cpu_hibernate.o swsusp.o

ifneq ($(MTK_TABLET_HARDWARE), )
ccflags-y += -DMTK_TABLET_PLATFORM=\"$(shell echo $(MTK_TABLET_HARDWARE) | tr a-z A-Z )\"
endif


# SLT Cases
#obj-y += slt_maxpower.o slt_fp0_maxpower_asm.o slt_memcpyL2.o slt_fp3_memcpyL2_asm.o slt_vfp_func.o slt_fp4_vfp_func_asm.o slt_fdct.o slt_fp7_fdct_asm.o slt_imdct.o slt_fp8_imdct_asm.o
#

#KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon)
#KT: modify for slt vfp test
KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon-vfpv4)
