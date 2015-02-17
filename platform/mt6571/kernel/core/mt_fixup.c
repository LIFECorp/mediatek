#include <linux/init.h>
#include <linux/string.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <mach/devs.h>
#include <mach/mt_boot.h>
#include <mach/memory.h>
#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <asm/page.h>
#include <linux/export.h>
#include <mach/mtk_memcfg.h>
#include <mach/mtk_ccci_helper.h>
#include <mach/dfo_boot.h>
#include <mach/dfo_boot_default.h>

/*=======================================================================*/
/* Commandline filter                                                    */
/* This function is used to filter undesired command passed from LK      */
/*=======================================================================*/
static void cmdline_filter(struct tag *cmdline_tag, char *default_cmdline)
{
	const char *undesired_cmds[] = {
	                             "console=",
                                     "root=",
			             };

	int i;
	int ck_f = 0;
	char *cs,*ce;

	cs = cmdline_tag->u.cmdline.cmdline;
	ce = cs;
	while((__u32)ce < (__u32)tag_next(cmdline_tag)) {

	    while(*cs == ' ' || *cs == '\0') {
	    	cs++;
	    	ce = cs;
	    }

	    if (*ce == ' ' || *ce == '\0') {
	    	for (i = 0; i < sizeof(undesired_cmds)/sizeof(char *); i++){
	    	    if (memcmp(cs, undesired_cmds[i], strlen(undesired_cmds[i])) == 0) {
			ck_f = 1;
                        break;
                    }    		
	    	}

                if(ck_f == 0){
	    	        *ce = '\0';
         	        //Append to the default command line
        	        strcat(default_cmdline, " ");
	    	        strcat(default_cmdline, cs);
	    	    }
		ck_f = 0;
	    	cs = ce + 1;
	    }
	    ce++;
	}
	if (strlen(default_cmdline) >= COMMAND_LINE_SIZE)
	{
		panic("Command line length is too long.\n\r");
	}
}

#define MAX_NR_MODEM 2
static unsigned long modem_start_addr_list[MAX_NR_MODEM] = { 0x0, 0x0, };

#ifdef __USING_DUMMY_CCCI_API__
static unsigned int modem_size_list[1] = {24 * 1024 * 1024}; // 72 modem is 24MB
static unsigned int get_nr_modem(void)
{
    return 1;
}
static unsigned int *get_modem_size_list(void)
{
    return modem_size_list;
}
#else
extern unsigned int get_nr_modem(void);
extern unsigned int *get_modem_size_list(void);
#endif

unsigned long *get_modem_start_addr_list(void)
{
    return modem_start_addr_list;
}
EXPORT_SYMBOL(get_modem_start_addr_list);

static unsigned int get_modem_size(void)
{
    int i, nr_modem;
    unsigned int size = 0, *modem_size_list;
    modem_size_list = get_modem_size_list();
    nr_modem = get_nr_modem();
    if (modem_size_list) {
        for (i = 0; i < nr_modem; i++) {
            size += modem_size_list[i];
        }
        return size;
    } else {
        return 0;
    }
}
#if defined(CONFIG_MTK_FB)
char temp_command_line[1024] = {0};
extern unsigned int DISP_GetVRamSizeBoot(char* cmdline);
#define RESERVED_MEM_SIZE_FOR_FB (DISP_GetVRamSizeBoot((char*)&temp_command_line))
extern void   mtkfb_set_lcm_inited(bool isLcmInited);
#else
#define RESERVED_MEM_SIZE_FOR_FB (0x400000)
#endif
#ifndef CONFIG_RESERVED_MEM_SIZE_FOR_PMEM
#define CONFIG_RESERVED_MEM_SIZE_FOR_PMEM 	1
#endif
#define TOTAL_RESERVED_MEM_SIZE (RESERVED_MEM_SIZE_FOR_PMEM+RESERVED_MEM_SIZE_FOR_FB) // Size reserved at end of DRAM
#define MAX_PFN        ((max_pfn << PAGE_SHIFT) + PHYS_OFFSET)
#define PMEM_MM_START  (pmem_start)
#define PMEM_MM_SIZE   (RESERVED_MEM_SIZE_FOR_PMEM)
#define FB_START       (PMEM_MM_START + PMEM_MM_SIZE)
#define FB_SIZE        (RESERVED_MEM_SIZE_FOR_FB)

/*
 * CONNSYS FW must occupy the beginning of DRAM on MT6572 which is always 0x80000000
 * The size is 1MB
 * This offset is done by LK so we won't need to "fixup" for CONNSYS here
 */
#define CONNSYS_START  (DRAM_PHYS_ADDR_START)       // beginning of DRAM
#define RESERVED_MEM_SIZE_FOR_CONNSYS (0x100000)    // 1MB
#define CONNSYS_SIZE   (RESERVED_MEM_SIZE_FOR_CONNSYS)

/*
 * The memory size reserved for PMEM
 *
 * The size could be varied in different solutions.
 * The size is set in mt65xx_fixup function.
 * - MT6572 in develop (before M4U) should be 0x1700000
 * - MT6572 SQC should be 0x0
 */
#define RESERVED_MEM_SIZE_FOR_PMEM (0x0)
unsigned long pmem_start=0x0; // Init at fixup

extern int parse_tag_videofb_fixup(const struct tag *tags);
extern int parse_tag_devinfo_data_fixup(const struct tag *tags);
extern void adjust_kernel_cmd_line_setting_for_console(char *u_boot_cmd_line, char *kernel_cmd_line);
#if defined(CONFIG_FIQ_DEBUGGER)
extern void fiq_uart_fixup(int uart_port);
extern struct platform_device mt_fiq_debugger;
#endif
extern int use_bl_fb;
unsigned int get_max_DRAM_size(void);
static unsigned long actual_dram = 0;   // Memory size that is passed down from boot loader, not limited by configurations.
static unsigned long avail_dram = 0;    // Memory size that is managed by kernel. Valid after mt_fixup().

/*
 * is_pmem_range
 * Input
 *   base: buffer base physical address
 *   size: buffer len in byte
 * Return
 *   1: buffer is located in pmem address range
 *   0: buffer is out of pmem address range
 */
int is_pmem_range(unsigned long *base, unsigned long size)
{
        unsigned long start = (unsigned long)base;
        unsigned long end = start + size;

        //printk("[PMEM] start=0x%p,end=0x%p,size=%d\n", start, end, size);
        //printk("[PMEM] PMEM_MM_START=0x%p,PMEM_MM_SIZE=%d\n", PMEM_MM_START, PMEM_MM_SIZE);

        if (start < PMEM_MM_START)
                return 0;
        if (end >= PMEM_MM_START + PMEM_MM_SIZE)
                return 0;

        return 1;
}
EXPORT_SYMBOL(is_pmem_range);


// return the size of memory from start pfn to max pfn,
// _NOTE_
// the memory area may be discontinuous
unsigned int mtk_get_memory_size (void)
{
    return (MAX_PFN) - (PHYS_OFFSET);
}

// return the physical DRAM size (limited by configuration)
unsigned int mtk_get_max_DRAM_size(void)
{
        return avail_dram + RESERVED_MEM_SIZE_FOR_CONNSYS;
}

// return the physical DRAM size (passed down from Boot Loader)
unsigned int get_actual_DRAM_size(void)
{
        return actual_dram;
}
EXPORT_SYMBOL(get_actual_DRAM_size);

// return the starting phys addr that AP processor accesses
unsigned int get_phys_offset(void)
{
	return PHYS_OFFSET;
}
EXPORT_SYMBOL(get_phys_offset);

// return maximum phys addr that AP processor accesses
unsigned int get_max_phys_addr(void)
{
    // MT6572 reserved 1MB for CONNSYS via PHYS_OFFSET,
    // and avail_dram already subtracted such value.
    return PHYS_OFFSET + avail_dram;
}
EXPORT_SYMBOL(get_max_phys_addr);

unsigned int get_fb_start(void)
{
    return FB_START;
}
EXPORT_SYMBOL(get_fb_start);

unsigned int get_fb_size(void)
{
    return FB_SIZE;
}
EXPORT_SYMBOL(get_fb_size);

#include <asm/sections.h>
void get_text_region (unsigned int *s, unsigned int *e)
{
    *s = (unsigned int)_text, *e=(unsigned int)_etext ;
}
EXPORT_SYMBOL(get_text_region) ;

void __init mt_fixup(struct tag *tags, char **cmdline, struct meminfo *mi)
{
    struct tag *cmdline_tag = NULL;
    struct tag *reserved_mem_bank_tag = NULL;
    struct tag *none_tag = NULL;
    char *br_ptr = NULL;

    /* 
     * In dual modem model, modem's memory is splitted from normal memory, 
     * so we do not take modem size (rear-end-modems) from max_limit_size.
     */
    unsigned long max_limit_size = CONFIG_MAX_DRAM_SIZE_SUPPORT - (PHYS_OFFSET - DRAM_PHYS_ADDR_START);
    actual_dram = 0;
    avail_dram = 0;

    printk(KERN_ALERT"Load default dfo data...\n");
    parse_ccci_dfo_setting(&dfo_boot_default, DFO_BOOT_COUNT);

#if defined(CONFIG_MTK_FB)
    // Filter command line to get reserved FB mem size
    struct tag *temp_tags = tags;
    for (; temp_tags->hdr.size; temp_tags = tag_next(temp_tags))
    {
        if(temp_tags->hdr.tag == ATAG_CMDLINE)
            cmdline_filter(temp_tags, (char*)&temp_command_line);
    }
#endif

    for (; tags->hdr.size; tags = tag_next(tags)) {
        if (tags->hdr.tag == ATAG_MEM) {
            actual_dram += tags->u.mem.size;

            /*
             * Modify the memory tag to limit available memory to
             * CONFIG_MAX_DRAM_SIZE_SUPPORT
             */
            if (max_limit_size > 0) {
                if (max_limit_size >= tags->u.mem.size) {
                    max_limit_size -= tags->u.mem.size;
                    avail_dram += tags->u.mem.size;
                } else {
                    tags->u.mem.size = max_limit_size;
                    avail_dram += max_limit_size;
                    // we are full. accept no further banks by setting 
                    // max_limit_size to 0
                    max_limit_size = 0;
                }
                // By Keene:
                // remove this check to avoid calcuate pmem size before we know all dram size
                // Assuming the minimum size of memory bank is 256MB
                //if (tags->u.mem.size >= (TOTAL_RESERVED_MEM_SIZE)) {
                reserved_mem_bank_tag = tags;
                //}
            } else {
                tags->u.mem.size = 0;
            }
        }
        else if (tags->hdr.tag == ATAG_CMDLINE) {
            cmdline_tag = tags;
        } else if (tags->hdr.tag == ATAG_BOOT) {
            g_boot_mode = tags->u.boot.bootmode;
        } else if (tags->hdr.tag == ATAG_VIDEOLFB) {
            parse_tag_videofb_fixup(tags);
        } else if (tags->hdr.tag == ATAG_DEVINFO_DATA){
            parse_tag_devinfo_data_fixup(tags);
        } else if(tags->hdr.tag == ATAG_META_COM) {
          g_meta_com_type = tags->u.meta_com.meta_com_type;
          g_meta_com_id = tags->u.meta_com.meta_com_id;
        } else if (tags->hdr.tag == ATAG_DFO_DATA) {
            parse_ccci_dfo_setting(&tags->u.dfo_data, DFO_BOOT_COUNT);
        }
    }

    if ((g_boot_mode == META_BOOT) || (g_boot_mode == ADVMETA_BOOT)) {
        /* 
         * Always use default dfo setting in META mode.
         * We can fix abnormal dfo setting this way.
         */
        printk(KERN_ALERT"(META mode) Load default dfo data...\n");
        parse_ccci_dfo_setting(&dfo_boot_default, DFO_BOOT_COUNT);
    }

    /*
     * If the maximum memory size configured in kernel
     * is smaller than the actual size (passed from BL)
     * Still limit the maximum memory size but use the FB
     * initialized by BL
     */
    if (actual_dram >= (CONFIG_MAX_DRAM_SIZE_SUPPORT)) {
        use_bl_fb++;
    }

    /*
     * Setup PMEM, FB reserve area
     * Reserve memory in the last bank.
     * Connsys is already reserved in LK, so that atag starting address is already offseted.
     */
    if (reserved_mem_bank_tag) {
        // Others (PMEM, FB) are at the end
        reserved_mem_bank_tag->u.mem.size -= ((__u32)TOTAL_RESERVED_MEM_SIZE);
        pmem_start = reserved_mem_bank_tag->u.mem.start + reserved_mem_bank_tag->u.mem.size;
    }
    else // we should always have reserved memory
        BUG();
    
    // Physical Mem Layout Debug info 
    printk(KERN_ALERT
    "[Phy Layout] Avail. DRAM Size : 0x%08lx\n"
    "[Phy Layout]               FB : 0x%08lx-0x%08lx (0x%08lx)\n"
    "[Phy Layout]             PMEM : 0x%08lx-0x%08lx (0x%08lx)\n"
    "[Phy Layout]          CONNSYS : 0x%08lx-0x%08lx (0x%08lx)\n",
    avail_dram,
    FB_START,FB_START+FB_SIZE,FB_SIZE,
    PMEM_MM_START, PMEM_MM_START+PMEM_MM_SIZE, PMEM_MM_SIZE,
    CONNSYS_START, CONNSYS_START+CONNSYS_SIZE, CONNSYS_SIZE    
    );

    /* 
     * fixup memory tags for dual modem model
     * assumptions:
     * 1) modem start addresses should be 32MiB aligned
     */
    unsigned int nr_modem = 0, i = 0;
    unsigned int max_avail_addr = 0;
    unsigned int modem_start_addr = 0;
    unsigned int hole_start_addr = 0;
    unsigned int hole_size = 0;
    unsigned int *modem_size_list = 0;

    nr_modem = get_nr_modem();
    modem_size_list = get_modem_size_list();
    if(tags->hdr.tag == ATAG_NONE) {
        // Reserve Mem for Modem
        for (i = 0; i < nr_modem; i++) {
            /* sanity test */
            if (modem_size_list[i]) {
                printk(KERN_ALERT"fixup for modem [%d], size = 0x%08x\n", i,
                        modem_size_list[i]);
            } else {
                printk(KERN_ALERT"[Error]skip empty modem [%d]\n", i);
                continue;
            }
            printk(KERN_ALERT
                    "reserved_mem_bank_tag start = 0x%08x, "
                    "reserved_mem_bank_tag size = 0x%08x, "
                    "TOTAL_RESERVED_MEM_SIZE = 0x%08x\n",
                    reserved_mem_bank_tag->u.mem.start,
                    reserved_mem_bank_tag->u.mem.size,
                    TOTAL_RESERVED_MEM_SIZE);
            /* find out start address for modem */
            max_avail_addr = reserved_mem_bank_tag->u.mem.start + 
                reserved_mem_bank_tag->u.mem.size;
            modem_start_addr = 
                round_down((max_avail_addr - 
                            modem_size_list[i]), 0x2000000);
            /* sanity test */
            if (modem_size_list[i] > reserved_mem_bank_tag->u.mem.size) {
                printk(KERN_ALERT
                        "[Error]skip modem [%d] fixup: "
                        "modem size too large: 0x%08x, "
                        "reserved_mem_bank_tag->u.mem.size: 0x%08x\n", 
                        i,
                        modem_size_list[i],
                        reserved_mem_bank_tag->u.mem.size);
                continue;
            }
            if (modem_start_addr < reserved_mem_bank_tag->u.mem.start) {
                printk(KERN_ALERT
                        "[Error]skip modem [%d] fixup: "
                        "modem start addr crosses memory bank boundary: 0x%08x, "
                        "reserved_mem_bank_tag->u.mem.start: 0x%08x\n", 
                        i,
                        modem_start_addr,
                        reserved_mem_bank_tag->u.mem.start);
                continue;
            }
            printk(KERN_ALERT"modem fixup sanity test pass\n");
            modem_start_addr_list[i] = modem_start_addr;
            hole_start_addr = modem_start_addr + modem_size_list[i];
            hole_size = max_avail_addr - hole_start_addr;
            printk(KERN_ALERT
                    "max_avail_addr = 0x%08x, "
                    "modem_start_addr_list[%d] = 0x%08x, "
                    "hole_start_addr = 0x%08x, hole_size = 0x%08x\n", 
                    max_avail_addr, i, modem_start_addr, 
                    hole_start_addr, hole_size);
            /* shrink reserved_mem_bank */
            reserved_mem_bank_tag->u.mem.size -= 
                (max_avail_addr - modem_start_addr);
            printk(KERN_ALERT
                    "reserved_mem_bank: start = 0x%08x, size = 0x%08x\n", 
                    reserved_mem_bank_tag->u.mem.start, 
                    reserved_mem_bank_tag->u.mem.size);
            /* setup a new memory tag */
            tags->hdr.tag = ATAG_MEM;
            tags->hdr.size = tag_size(tag_mem32);
            tags->u.mem.start = hole_start_addr;
            tags->u.mem.size = hole_size;
            /* do next tag */
            tags = tag_next(tags);
        }
        tags->hdr.tag = ATAG_NONE; // mark the end of the tag list
        tags->hdr.size = 0; 
    }

    printk(KERN_ALERT
        "[Phy Layout]  get_max_DRAM_size() : 0x%08lx\n"
        "[Phy Layout]    get_phys_offset() : 0x%08lx\n"
        "[Phy Layout]  get_max_phys_addr() : 0x%08lx\n",
        get_max_DRAM_size(), 
        get_phys_offset(),
        get_max_phys_addr()
        );

    if(tags->hdr.tag == ATAG_NONE)
        none_tag = tags;
    if (cmdline_tag != NULL) {
#ifdef CONFIG_FIQ_DEBUGGER
        char *console_ptr;
        int uart_port;
#endif
        // This function may modify ttyMT3 to ttyMT0 if needed
        adjust_kernel_cmd_line_setting_for_console(cmdline_tag->u.cmdline.cmdline, *cmdline);
#ifdef CONFIG_FIQ_DEBUGGER
        if ((console_ptr=strstr(*cmdline, "ttyMT")) != 0)
        {
            uart_port = console_ptr[5] - '0';
            if (uart_port >= CFG_DEV_UART_NR)
                uart_port = -1;

            fiq_uart_fixup(uart_port);
        }
#endif
	/*FIXME mark for porting*/
        cmdline_filter(cmdline_tag, *cmdline);
	if ((br_ptr = strstr(*cmdline, "boot_reason=")) != 0) {
		/* get boot reason */
		g_boot_reason = br_ptr[12] - '0';
	}

        /* Use the default cmdline */
        memcpy((void*)cmdline_tag,
                (void*)tag_next(cmdline_tag),
                /* ATAG_NONE actual size */
                (uint32_t)(none_tag) - (uint32_t)(tag_next(cmdline_tag)) + 8);
    }
}

