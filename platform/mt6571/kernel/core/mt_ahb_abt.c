#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include "mach/irqs.h"
#include "mach/sync_write.h"
#include "mach/mt_ahb_abt.h"


#define AHBABT_DEBUG 0
#if(AHBABT_DEBUG == 1)
#define AHBABT_DEBUG_LEVEL KERN_NOTICE
#else
#define AHBABT_DEBUG_LEVEL KERN_INFO
#endif


static unsigned int g_ahbabt_init = 0;
static unsigned int g_ahbabt_addr[AHBABT_CNT];
static unsigned int g_ahbabt_cnt[AHBABT_CNT];
static unsigned int g_ahbabt_con;

#if 0
static struct mt_ahb_abt_driver {
	struct device_driver driver;
	const struct platform_device_id *id_table;
};

static struct mt_ahb_abt_driver mt_ahb_abt_drv = {
	.driver = {
		   .name = "mt_ahb_abt",
		   .bus = &platform_bus_type,
		   .owner = THIS_MODULE,
		   },
	.id_table = NULL,
};
#endif


int mt_ahb_abt_dump(char *buf)
{
	int i;
	char *ptr = buf;

    if (g_ahbabt_init == 0) {
        g_ahbabt_con = readl(AHBABT_CON);
	    for (i = 0; i < AHBABT_CNT; i++) {
	        g_ahbabt_addr[i] = readl(AHBABT_ADDR1 + (i * 4));
	        g_ahbabt_cnt[i] = readl(AHBABT_RDY_CNT1 + (i * 4));
	    }
	}

    ptr += sprintf(ptr, "\n");

    ptr += sprintf(ptr, "AHBABT_CON = 0x%x\n", g_ahbabt_con);
    ptr += sprintf(ptr, "abort_status: %s, mon_en: %s, abort_en: %s\n",
            (g_ahbabt_con & 0x80000000) ? "on" : "off",
            (g_ahbabt_con & 0x00000001) ? "yes" : "no",
            (g_ahbabt_con & 0x00000002) ? "yes" : "no");

	for (i = 0; i < AHBABT_CNT; i++) {
	    ptr += sprintf(ptr, "AHBABT_ADDR%d = 0x%x, AHBABT_CNT%d = 0x%x\n", i, g_ahbabt_addr[i], i, g_ahbabt_cnt[i]);
	}

	return 0;
}

/*
 * mt_ahb_abt_isr: AHB ABT Monitor interrupt service routine.
 * @irq: AHB ABT Monitor IRQ number
 * @dev_id:
 * Return IRQ returned code.
 */
static irqreturn_t mt_ahb_abt_isr(int irq, void *dev_id)
{
	//printk(KERN_INFO "AHB ABT Monitor ISR Start\n");

    while(1)  ;

	//printk(KERN_INFO "AHB ABT Monitor ISR END\n");

	return IRQ_HANDLED;
}

void mt_ahb_abt_start(void)
{
    printk(AHBABT_DEBUG_LEVEL "Clear AHB Abort Monitor\n");
    mt65xx_reg_sync_writel(0x3, AHBABT_CON_CLR);
    mt65xx_reg_sync_writel(0x1, AHBABT_MON_CLR);
    mt65xx_reg_sync_writel(0x3, AHBABT_ABOT_CLR);

    printk(AHBABT_DEBUG_LEVEL "Enable AHB Abort Monitor\n");
    //mt65xx_reg_sync_writel(0xCB7355, AHBABT_THRES);  // 100 ms
    mt65xx_reg_sync_writel(0x27A31840, AHBABT_THRES);  // 5 s
    mt65xx_reg_sync_writel(0x1, AHBABT_CON_SET);

    return;
}

static int mt_ahb_abt_probe(struct platform_device *dev)
{
    printk(AHBABT_DEBUG_LEVEL "AHB Abort Monitor probe.\n");
    mt_ahb_abt_start();
    return 0;
}

static int mt_ahb_abt_remove(struct platform_device *dev)
{
    printk(AHBABT_DEBUG_LEVEL "AHB Abort Monitor remove.\n");
    return 0;
}

static int mt_ahb_abt_suspend(struct platform_device *dev, pm_message_t state)
{
    printk(AHBABT_DEBUG_LEVEL "AHB Abort Monitor suspend.\n");
    return 0;
}

static int mt_ahb_abt_resume(struct platform_device *dev)
{
    printk(AHBABT_DEBUG_LEVEL "AHB Abort Monitor resume.\n");
    mt_ahb_abt_start();
    return 0;
}


static struct platform_device mt_ahb_abt_device = {
    .name   = "mt_ahb_abt",
    .id     = -1,
};

static struct platform_driver mt_ahb_abt_driver = {
    .probe      = mt_ahb_abt_probe,
    .remove     = mt_ahb_abt_remove,
    .suspend    = mt_ahb_abt_suspend,
    .resume     = mt_ahb_abt_resume,
    .driver     = {
        .name   = "mt_ahb_abt",
        .owner  = THIS_MODULE,
    },
};

/*
 * mt_ahb_abt_init: initialize driver.
 * Always return 0.
 */
static int __init mt_ahb_abt_init(void)
{
    int i;
	int ret;

    printk(AHBABT_DEBUG_LEVEL "Init AHB Abort Monitor\n");

#if 0
	ret = driver_register(&mt_ahb_abt_drv.driver);
	if (ret) {
		printk(KERN_ERR "Fail to register mt_ahb_abt_drv\n");
	}
#endif

    g_ahbabt_con = readl(AHBABT_CON);
    printk(KERN_NOTICE "AHBABT_CON = 0x%x\n", g_ahbabt_con);
    printk(KERN_NOTICE "abort_status: %s, mon_en: %s, abort_en: %s\n",
           (g_ahbabt_con & 0x80000000) ? "on" : "off",
           (g_ahbabt_con & 0x00000001) ? "yes" : "no",
           (g_ahbabt_con & 0x00000002) ? "yes" : "no");


	for (i = 0; i < AHBABT_CNT; i++) {
	    g_ahbabt_addr[i] = readl(AHBABT_ADDR1 + (i * 4));
	    g_ahbabt_cnt[i] = readl(AHBABT_RDY_CNT1 + (i * 4));
	    printk(KERN_NOTICE "AHBABT_ADDR%d = 0x%x\n, AHBABT_CNT%d = 0x%x\n", i, g_ahbabt_addr[i], i, g_ahbabt_cnt[i]);
	}
	g_ahbabt_init = 1;

    printk(AHBABT_DEBUG_LEVEL "Register platform AHB Abort Monitor device.\n");
    ret = platform_device_register(&mt_ahb_abt_device);
    if (ret) {
        printk(KERN_ERR "Fail to register mt_ahb_abt_device (%d)\n", ret);
        return ret;
    }

    printk(AHBABT_DEBUG_LEVEL "Register platform AHB Abort Monitor driver.\n");
    ret = platform_driver_register(&mt_ahb_abt_driver);
    if (ret) {
        printk(KERN_ERR "Fail to register mt_ahb_abt_driver (%d)\n", ret);
        return ret;
    }

    printk(AHBABT_DEBUG_LEVEL "Register AHB Abort Monitor IRQ\n");
    ret = request_irq(MT_AHBMON_IRQ_ID, mt_ahb_abt_isr, IRQF_TRIGGER_LOW, "AHB_ABT", NULL);
	if (ret) {
		printk(KERN_ERR "Fail to register mt_ahb_abt_isr (%d)\n", ret);
	}

	return 0;
}

arch_initcall(mt_ahb_abt_init);

