/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/xlog.h>
#include <mach/irqs.h>
#include <mach/eint.h>
#include <linux/workqueue.h>
#include <linux/usb/gadget.h>


#ifndef CONFIG_MTK_FPGA
#include <cust_gpio_usage.h>
#endif
#include "mach/emi_mpu.h"

#include <linux/mu3d/hal/mu3d_hal_osal.h>
#include "musb_core.h"

extern struct musb *_mu3d_musb;

typedef enum
{
    CABLE_MODE_CHRG_ONLY = 0,
    CABLE_MODE_NORMAL,
    CABLE_MODE_HOST_ONLY,
    CABLE_MODE_MAX
} CABLE_MODE;

/* Battery relative fucntion */
typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    // AC : 450mA~1A
    STANDARD_CHARGER,       // AC : ~1A
} CHARGER_TYPE;

extern void wake_up_bat(void);
extern CHARGER_TYPE mt_charger_type_detection(void);
extern bool upmu_is_chr_det(void);
extern void BATTERY_SetUSBState(int usb_state);
extern void upmu_interrupt_chrdet_int_en(u32 val);
extern u32 upmu_get_rgs_chrdet(void);

unsigned int cable_mode = CABLE_MODE_NORMAL;

/* ================================ */
/* connect and disconnect functions */
/* ================================ */
void connection_work(struct work_struct *data)
{
	struct musb *musb = container_of(to_delayed_work(data), struct musb, connection_work);
	static bool is_on = true;
	bool is_usb_cable = usb_cable_connected();

	os_printk(K_INFO, "%s musb %s, cable %s\n", __func__, (is_on?"ON":"OFF"), (is_usb_cable?"IN":"OUT"));

	if ((is_usb_cable == true) && (is_on == false) && (musb->usb_mode == CABLE_MODE_NORMAL)) {

		is_on = true;

		if (!wake_lock_active(&musb->usb_wakelock))
			wake_lock(&musb->usb_wakelock);

		/* FIXME: Should use usb_udc_start() & usb_gadget_connect(), like usb_udc_softconn_store().
		 * But have no time to think how to handle. However i think it is the correct way.*/
		musb_start(musb);

		os_printk(K_INFO, "%s ----Connect----\n", __func__);
	} else if (((is_usb_cable == false) && (is_on == true)) || (musb->usb_mode != CABLE_MODE_NORMAL)) {

		is_on = false;

		/*FIXME: we should use usb_gadget_disconnect() & usb_udc_stop().  like usb_udc_softconn_store().
		 * But have no time to think how to handle. However i think it is the correct way.*/
		musb_stop(musb);

		if (wake_lock_active(&musb->usb_wakelock))
			wake_unlock(&musb->usb_wakelock);

		os_printk(K_INFO, "%s ----Disconnect----\n", __func__);
	} else {
		/* This if-elseif is to set wakelock when booting with USB cable.
		 * Because battery driver does _NOT_ notify at this codition.*/
		if( (is_usb_cable == true) && !wake_lock_active(&musb->usb_wakelock)) {
			os_printk(K_INFO, "%s Boot wakelock\n", __func__);
			wake_lock(&musb->usb_wakelock);
		} else if( (is_usb_cable == false) && wake_lock_active(&musb->usb_wakelock)) {
			os_printk(K_INFO, "%s Boot unwakelock\n", __func__);
			wake_unlock(&musb->usb_wakelock);
		}

		os_printk(K_INFO, "%s directly return\n", __func__);
	}
}

bool mt_usb_is_device(void)
{
	os_printk(K_INFO, "%s\n", __func__);
#ifdef NEVER
	if(!mtk_musb){
		DBG(0,"mtk_musb is NULL\n");
		return false; // don't do charger detection when usb is not ready
	} else {
		DBG(4,"is_host=%d\n",mtk_musb->is_host);
	}
	return !mtk_musb->is_host;
#endif /* NEVER */
	return true;
}

bool mt_usb_is_ready(void)
{
	os_printk(K_INFO, "USB is ready or not\n");
#ifdef NEVER
	if(!mtk_musb || !mtk_musb->is_ready)
		return false;
	else
		return true;
#endif /* NEVER */
	return true;
}

void mt_usb_connect(void)
{
	struct delayed_work *work;

	os_printk(K_INFO, "%s+\n", __func__);
	if(_mu3d_musb) {
		work = &_mu3d_musb->connection_work;

		//if(!cancel_delayed_work(work))
		//	flush_workqueue(_mu3d_musb->wq);

		queue_delayed_work(_mu3d_musb->wq, work, 0);
	} else {
		os_printk(K_INFO, "%s musb_musb not ready\n", __func__);
	}
	os_printk(K_INFO, "%s-\n", __func__);
}
EXPORT_SYMBOL_GPL(mt_usb_connect);

void mt_usb_disconnect(void)
{
	struct delayed_work *work;

	os_printk(K_INFO, "%s+\n", __func__);

	if(_mu3d_musb) {
		work = &_mu3d_musb->connection_work;

		//if(!cancel_delayed_work(work))
		//	flush_workqueue(_mu3d_musb->wq);

		queue_delayed_work(_mu3d_musb->wq, work, 0);
	} else {
		os_printk(K_INFO, "%s musb_musb not ready\n", __func__);
	}
	os_printk(K_INFO, "%s-\n", __func__);
}
EXPORT_SYMBOL_GPL(mt_usb_disconnect);

bool usb_cable_connected(void)
{
#ifdef CONFIG_POWER_EXT
	CHARGER_TYPE chg_type = mt_charger_type_detection();
	os_printk(K_INFO, "%s ext-chrdet=%d type=%d\n", __func__, upmu_get_rgs_chrdet(), chg_type);
	if (upmu_get_rgs_chrdet() && (chg_type == STANDARD_HOST))
#else
	os_printk(K_INFO, "%s chrdet=%d\n", __func__, upmu_is_chr_det());
	if (upmu_is_chr_det())
#endif
	{
		return true;
	} else {
		return false;
	}
	return true;
}
EXPORT_SYMBOL_GPL(usb_cable_connected);

#ifdef NEVER
void musb_platform_reset(struct musb *musb)
{
	u16 swrst = 0;
	void __iomem	*mbase = musb->mregs;
	swrst = musb_readw(mbase,MUSB_SWRST);
	swrst |= (MUSB_SWRST_DISUSBRESET | MUSB_SWRST_SWRST);
	musb_writew(mbase, MUSB_SWRST,swrst);
}
#endif /* NEVER */

void usb_check_connect(void)
{
	os_printk(K_INFO, "usb_check_connect\n");

#ifndef CONFIG_MTK_FPGA
	if (usb_cable_connected())
		mt_usb_connect();
#endif

}

void musb_sync_with_bat(struct musb *musb, int usb_state)
{
	os_printk(K_INFO, "musb_sync_with_bat\n");

#ifndef CONFIG_MTK_FPGA
	BATTERY_SetUSBState(usb_state);
	wake_up_bat();
#endif

}

/*--FOR INSTANT POWER ON USAGE--------------------------------------------------*/
static inline struct musb *dev_to_musb(struct device *dev)
{
	return dev_get_drvdata(dev);
}

const char* const usb_mode_str[CABLE_MODE_MAX] = {"CHRG_ONLY", "NORMAL", "HOST_ONLY"};

ssize_t musb_cmode_show(struct device* dev, struct device_attribute *attr, char *buf)
{
	if (!dev) {
		os_printk(K_ERR, "dev is null!!\n");
		return 0;
	}
	return sprintf(buf, "%s\n", usb_mode_str[cable_mode]);
}

ssize_t musb_cmode_store(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int cmode;
	struct musb	*musb = dev_to_musb(dev);

	if (!dev) {
		os_printk(K_ERR, "dev is null!!\n");
		return count;
	} else if (1 == sscanf(buf, "%d", &cmode)) {
		os_printk(K_INFO, "%s %s --> %s\n", __func__, usb_mode_str[cable_mode], usb_mode_str[cmode]);

		if (cmode >= CABLE_MODE_MAX)
			cmode = CABLE_MODE_NORMAL;

		if (cable_mode != cmode) {
			if (cmode == CABLE_MODE_CHRG_ONLY) { // IPO shutdown, disable USB
				if(musb) {
					musb->usb_mode = CABLE_MODE_CHRG_ONLY;
					mt_usb_disconnect();
				}
			} else if (cmode == CABLE_MODE_HOST_ONLY) {
				if(musb) {
					musb->usb_mode = CABLE_MODE_HOST_ONLY;
					mt_usb_disconnect();
				}
			} else { // IPO bootup, enable USB
				if(musb) {
					musb->usb_mode = CABLE_MODE_NORMAL;
					mt_usb_connect();
				}
			}
			cable_mode = cmode;
		}
	}
	return count;
}

#ifdef NEVER
#ifdef CONFIG_MTK_FPGA
static struct i2c_client *usb_i2c_client = NULL;
static const struct i2c_device_id usb_i2c_id[] = {{"mtk-usb",0},{}};

static struct i2c_board_info __initdata usb_i2c_dev = { I2C_BOARD_INFO("mtk-usb", 0x60)};


void USB_PHY_Write_Register8(UINT8 var,  UINT8 addr)
{
	char buffer[2];
	buffer[0] = addr;
	buffer[1] = var;
	i2c_master_send(usb_i2c_client, &buffer, 2);
}

UINT8 USB_PHY_Read_Register8(UINT8 addr)
{
	UINT8 var;
	i2c_master_send(usb_i2c_client, &addr, 1);
	i2c_master_recv(usb_i2c_client, &var, 1);
	return var;
}

static int usb_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	printk("[MUSB]usb_i2c_probe, start\n");

	usb_i2c_client = client;

	//disable usb mac suspend
	DRV_WriteReg8(USB_SIF_BASE + 0x86a, 0x00);

	//usb phy initial sequence
	USB_PHY_Write_Register8(0x00, 0xFF);
	USB_PHY_Write_Register8(0x04, 0x61);
	USB_PHY_Write_Register8(0x00, 0x68);
	USB_PHY_Write_Register8(0x00, 0x6a);
	USB_PHY_Write_Register8(0x6e, 0x00);
	USB_PHY_Write_Register8(0x0c, 0x1b);
	USB_PHY_Write_Register8(0x44, 0x08);
	USB_PHY_Write_Register8(0x55, 0x11);
	USB_PHY_Write_Register8(0x68, 0x1a);


	printk("[MUSB]addr: 0xFF, value: %x\n", USB_PHY_Read_Register8(0xFF));
	printk("[MUSB]addr: 0x61, value: %x\n", USB_PHY_Read_Register8(0x61));
	printk("[MUSB]addr: 0x68, value: %x\n", USB_PHY_Read_Register8(0x68));
	printk("[MUSB]addr: 0x6a, value: %x\n", USB_PHY_Read_Register8(0x6a));
	printk("[MUSB]addr: 0x00, value: %x\n", USB_PHY_Read_Register8(0x00));
	printk("[MUSB]addr: 0x1b, value: %x\n", USB_PHY_Read_Register8(0x1b));
	printk("[MUSB]addr: 0x08, value: %x\n", USB_PHY_Read_Register8(0x08));
	printk("[MUSB]addr: 0x11, value: %x\n", USB_PHY_Read_Register8(0x11));
	printk("[MUSB]addr: 0x1a, value: %x\n", USB_PHY_Read_Register8(0x1a));


	printk("[MUSB]usb_i2c_probe, end\n");
    return 0;

}

static int usb_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-usb");
    return 0;
}

static int usb_i2c_remove(struct i2c_client *client) {return 0;}


struct i2c_driver usb_i2c_driver = {
    .probe = usb_i2c_probe,
    .remove = usb_i2c_remove,
    .detect = usb_i2c_detect,
    .driver = {
    	.name = "mtk-usb",
    },
    .id_table = usb_i2c_id,
};

int add_usb_i2c_driver()
{
	i2c_register_board_info(0, &usb_i2c_dev, 1);
	if(i2c_add_driver(&usb_i2c_driver)!=0)
	{
		printk("[MUSB]usb_i2c_driver initialization failed!!\n");
		return -1;
	}
	else
	{
		printk("[MUSB]usb_i2c_driver initialization succeed!!\n");
	}
	return 0;
}
#endif //End of CONFIG_MTK_FPGA
#endif /* NEVER */
