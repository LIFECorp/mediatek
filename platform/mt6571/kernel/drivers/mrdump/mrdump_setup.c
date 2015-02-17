#include <asm/page.h>
#include <asm/io.h>
#include <mach/mt_reg_base.h>
#include <linux/memblock.h>
#include <linux/mrdump.h>

#define MRDUMP_CB_ADDR 0x81F00000
#define MRDUMP_CB_SIZE 0x1000

static void mrdump_hw_enable(bool enabled)
{
}

static void mrdump_reboot(void)
{
	while (1)
		cpu_relax();
}

static const struct mrdump_platform mrdump_mt6572_platform = {
	.hw_enable = mrdump_hw_enable,
	.reboot = mrdump_reboot
};

void mrdump_reserve_memory(void)
{
	struct mrdump_control_block *cblock = NULL;

	memblock_reserve(MRDUMP_CB_ADDR, MRDUMP_CB_SIZE);
	cblock = (struct mrdump_control_block *)__va(MRDUMP_CB_ADDR);

	mrdump_platform_init(cblock, &mrdump_mt6572_platform);
}
