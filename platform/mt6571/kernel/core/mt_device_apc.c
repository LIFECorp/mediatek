#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include "mach/sync_write.h"
#include "mach/mt_device_apc.h"


DEFINE_SPINLOCK(g_mt_devapc_lock);
EXPORT_SYMBOL(g_mt_devapc_lock);


/*
 * mt_devapc_set_permission: set module permission on device apc.
 * @module: the moudle to specify permission
 * @domain_num: domain index number
 * @permission_control: specified permission
 * no return value.
 */
void mt_devapc_set_permission(unsigned int module, DEVAPC_DOM domain_num, DEVAPC_ATTR permission_control)
{
    unsigned long irq_flag;
    unsigned int base;
    unsigned int clr_bit = 0x3 << ((module % 16) * 2);
    unsigned int set_bit = permission_control << ((module % 16) * 2);

    if( module >= DEVAPC_DEVICE_NUMBER )
    {
        printk(KERN_WARNING, "[DEVAPC] ERROR, device number %d exceeds the max number!\n", module);
        return;
    }

    if (E_DOM_AP == domain_num)
    {
        base = DEVAPC_D0_APC_0 + (module / 16) * 4;
    }
    else if (E_DOM_MD == domain_num)
    {
        base = DEVAPC_D1_APC_0 + (module / 16) * 4;
    }
    else if (E_DOM_CONN == domain_num)
    {
        base = DEVAPC_D2_APC_0 + (module / 16) * 4;
    }
    else
    {
        printk(KERN_WARNING, "[DEVAPC] ERROR, domain number %d exceeds the max number!\n", domain_num);
        return;
    }
    
    spin_lock_irqsave(&g_mt_devapc_lock, irq_flag);

    mt65xx_reg_sync_writel(readl(base) & ~clr_bit, base);
    mt65xx_reg_sync_writel(readl(base) | set_bit, base);

    spin_unlock_irqrestore(&g_mt_devapc_lock, irq_flag);
    
    return;
}
EXPORT_SYMBOL(mt_devapc_set_permission);

