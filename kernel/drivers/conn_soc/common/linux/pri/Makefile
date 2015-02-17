ifeq ($(CONFIG_MTK_COMBO), y)
ccflags-y += -I$(src)/../../linux/include \
						 -I$(src)/../../linux/pri/include \
						 -I$(src)/../../core/include \
						 -I$(src)/../../include \
						 -I$(src)/../ \
						 -I$(src)/../../../../btif/inc \
						 -I$(src)/../../../../common_detect

ccflags-y += -I$(MTK_ROOT)/kernel/drivers/eemcs/
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/conn_md/include

obj-y += stp_btif.o \
         stp_dbg.o \
         stp_exp.o \
         wmt_dev.o \
         wmt_exp.o

endif
