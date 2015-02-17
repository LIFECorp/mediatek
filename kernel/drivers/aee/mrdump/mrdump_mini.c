#include <linux/mm.h>
#include <linux/aee.h>
#include <linux/elf.h>
#include <linux/kdebug.h>
#include <linux/mmc/sd_misc.h>
#include <linux/mrdump.h>

extern int emmc_ipanic_write(void *buf, int off, int len);
extern void* ipanic_next_write(void* (*next)(void *data, unsigned char *buffer, size_t sz_buf, int *size),
			       void *data, int off, size_t *sz_total);
/* 
 * ramdump mini 
 * implement ramdump mini prototype here 
 */
	
#ifdef MTK_EMMC_SUPPORT
/* data: indicate dump scope; buffer: dump to; sz_buf: buffer size; size: real size dumped */
static void* mrdump_memory_region(void *data, unsigned char *buffer, size_t sz_buf, int *size) {
	unsigned long sz_real;
	struct mrdump_mini_reg_desc *reg = (struct mrdump_mini_reg_desc*)data; 
	unsigned long start = reg->kstart;
	unsigned long end = reg->kend;
	unsigned long pos = reg->pos;

	if(pos > end || pos < start){
		printk(KERN_ALERT"%s: current offset(%ld) out of bounds [%ld, %ld]\n",
		       __func__, pos, start, end);
		return ERR_PTR(-1);
	}
	sz_real = (end - pos) > sz_buf ? sz_buf : (end - pos);
	memcpy(buffer, (void*)pos, sz_real);
	reg->pos += sz_real;
	*size = sz_real;
	return 0;
}

static char mrdump_mini_buf[MRDUMP_MINI_HEADER_SIZE];
static void __mrdump_mini_core(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, va_list ap)
{
	int i, n;
	unsigned long reg, start, end, size;
	size_t total = 0;
	loff_t offset = 0;
	struct mrdump_mini_header *hdr = (struct mrdump_mini_header *)mrdump_mini_buf;
	loff_t mem_offset = IPANIC_MRDUMP_OFFSET + MRDUMP_MINI_HEADER_SIZE;

	memset(mrdump_mini_buf, 0x0, MRDUMP_MINI_HEADER_SIZE);

	if (sizeof(struct mrdump_mini_header) > MRDUMP_MINI_HEADER_SIZE) {
		/* mrdump_mini_header is too large, write 0x0 headers to ipanic */
		printk(KERN_ALERT"mrdump_mini_header is too large(%d)\n", 
				sizeof(struct mrdump_mini_header));
		offset += MRDUMP_MINI_HEADER_SIZE;
		goto ipanic_write;
	}

	for(i = 0; i < ELF_NGREG; i++) {
		reg = regs->uregs[i];
		hdr->reg_desc[i].reg = reg;
		if (virt_addr_valid(reg)) {
			/* 
			 * ASSUMPION: memory is always in normal zone.
			 * 1) dump at most 32KB around valid kaddr
			 */
			/* align start address to PAGE_SIZE for gdb */
			start = round_down((reg - SZ_16K), PAGE_SIZE);
			end = start + SZ_32K;
			start = clamp(start, (unsigned long)PAGE_OFFSET, (unsigned long)high_memory);
			end = clamp(end, (unsigned long)PAGE_OFFSET, (unsigned long)high_memory) - 1;
			hdr->reg_desc[i].kstart = start;
			hdr->reg_desc[i].kend = end;
			hdr->reg_desc[i].pos = start;
			hdr->reg_desc[i].offset = offset;
			hdr->reg_desc[i].valid = 1;
			size = end - start + 1;
			ipanic_next_write(mrdump_memory_region, &hdr->reg_desc[i], mem_offset + offset, &total);
			offset += size;
		} else {
			hdr->reg_desc[i].kstart = 0;
			hdr->reg_desc[i].kend = 0;
			hdr->reg_desc[i].pos = 0;
			hdr->reg_desc[i].offset = 0;
			hdr->reg_desc[i].valid = 0;
		}
	}

ipanic_write:
	n = emmc_ipanic_write(mrdump_mini_buf, IPANIC_MRDUMP_OFFSET, ALIGN(MRDUMP_MINI_HEADER_SIZE, SZ_512));
	if (IS_ERR(ERR_PTR(n))) {
		printk(KERN_ERR"card_dump_func_write failed (%d), size: 0x%llx\n", 
				i, (unsigned long long)offset);
	}
}
#else //MTK_EMMC_SUPPORT
static void __mrdump_mini_core(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, va_list ap)
{
}
#endif //MTK_EMMC_SUPPORT

static void __mrdump_mini_create_oops_dump(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	__mrdump_mini_core(reboot_mode, regs, msg, ap);
	va_end(ap);
}

static int mrdump_mini_create_oops_dump(struct notifier_block *self, unsigned long cmd, void *ptr)
{
	struct die_args *dargs = (struct die_args *)ptr;
	smp_send_stop();
	__mrdump_mini_create_oops_dump(AEE_REBOOT_MODE_KERNEL_PANIC, dargs->regs, "kernel Oops");
	return NOTIFY_DONE;
}

static struct notifier_block mrdump_mini_oops_blk = {
	.notifier_call	= mrdump_mini_create_oops_dump,
};

static int __init mrdump_mini_init(void)
{
	register_die_notifier(&mrdump_mini_oops_blk);
	return 0;
}

late_initcall(mrdump_mini_init);
