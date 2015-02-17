ifeq ($(CONFIG_MTK_COMBO), y)
ccflags-y += -I$(src)/../../linux/include \
						 -I$(src)/../../linux/pri/include \
						 -I$(src)/../../core/include \
						 -I$(src)/../../include \
						 -I$(src)/../ \
						 -I$(src)/../../../../common_detect
						 
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/eemcs/
ccflags-y += -I$(MTK_ROOT)/kernel/drivers/conn_md/include

obj-y += osal.o \
         bgw_desense.o \
         stp_chrdev_bt.o \
         stp_chrdev_gps.o \
         wmt_chrdev_wifi.o \
         wmt_idc.o

endif
