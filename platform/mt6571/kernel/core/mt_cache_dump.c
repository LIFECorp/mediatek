#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/trace_seq.h>
#include <linux/ftrace_event.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <asm/tlbflush.h>
#include <linux/cpu.h>
#include <linux/smp.h>

#include <linux/xlog.h>
extern void __inner_clean_dcache_L1(void);
extern void __inner_clean_dcache_L2(void);
extern void __enable_dcache(void);
extern void __disable_dcache(void);
extern void __enable_icache(void);
extern void __disable_icache(void);
DEFINE_SPINLOCK(tlb_dump_lock);

#define tlb_dump_debug(fmt, args...)

#define tlb_dump_msg(fmt, args...) xlog_printk(ANDROID_LOG_INFO, "TLB/RAMDUMP", fmt, ##args);

struct tlb_ram_conf
{
    unsigned int way;
    unsigned int idx;
    unsigned int data[3];
};

static phys_addr_t arm_virt_to_phy(unsigned int vaddr)
{
	    phys_addr_t paddr = 0xFFFFFFFF;

	    /*
	     * VA to PA translation with priviledge read permission check
	     */
	    asm volatile(
	        "mcr    p15, 0, %1, c7, c8, 0\n"
	        "mrc    p15, 0, %0, c7, c4, 0\n"
	        : "=r" (paddr)
	        : "r" (vaddr)
	        :);

	    if(0 == (paddr & 0xFFFFFF80))
	        return 0;
	    else
	        return (paddr & 0xFFFFF000) | ((unsigned int)vaddr & 0x0FFF);
}

static int tlb_ram_get_data(struct tlb_ram_conf *tlbr)
{
    unsigned int tlb_tag;

    if(tlbr->way >= 2 || tlbr->idx > 0xFF)
    {
        tlbr->data[0] = 0;
        tlbr->data[1] = 0;
        tlbr->data[2] = 0;
        return -1;
    }

    tlb_tag = (tlbr->way << 31) | tlbr->idx;
    asm volatile(
        "MCR p15, 3, %0, c15, c4, 2\n"
        "isb\n"
        "MRC p15, 3, %1, c15, c0, 0\n"
        "MRC p15, 3, %2, c15, c0, 1\n"
        "MRC p15, 3, %3, c15, c0, 2\n"
        : "+r" (tlb_tag),"+r" (tlbr->data[0]),"+r" (tlbr->data[1]), "+r" (tlbr->data[2])
        :
        : "cc"
        );

    return 0;
}



unsigned int get_tlb_ram_pa(unsigned int in_paddr)
{

    /*  Cortex?-A7 MPCore, TRM
     *
     *  TLB Data Read Operation Register Write-only MCR p15, 3, <Rd>, c15, c4, 2 Index/Way
     *
     *  Table 6-7 TLB Data Read Operation Register location encoding
     *  Bit-field of Rd Description
     *  [31] TLB way
     *  [30:8] Unused
     *  [7:0] TLB index
     *        TLB index[7:0] Format
     *        0-127 Main TLB RAM, see Main TLB RAM
     *        128-159 Walk cache RAM, see Walk cache RAM on page 6-14
     *        160-191 IPA cache RAM, see IPA cache RAM on page 6-15
     *        192-255 Unused
     */
    struct tlb_ram_conf tlb_r_conf;
    struct tlb_ram_conf *tlbr = &tlb_r_conf;

    unsigned int max_tlb_way = 2;
    unsigned int max_tlb_idx = 192;
    unsigned int asid;
    unsigned int vmid;
    unsigned int vaddr, paddr;
    unsigned long flags;
    unsigned int ret = 0xFFFFFFFF;
    unsigned int nGb = 0;
    spin_lock_irqsave(&tlb_dump_lock, flags);

    __disable_dcache();

    /*
     * MRC p15, 0, <Rt>, c0, c1, 7 ; Read ID_MMFR3 into Rt
     * Maintenance broadcast, bits[15:12]
     * Indicates whether Cache, TLB and branch predictor operations are broadcast. Permitted values are:
     * 0b0000 Cache, TLB and branch predictor operations only affect local structures.
     * 0b0001 Cache and branch predictor operations affect structures according to shareability and
     * defined behavior of instructions. TLB operations only affect local structures.
     * 0b0010 Cache, TLB and branch predictor operations affect structures according to shareability
     * and defined behavior of instructions.
     */

    for (tlbr->idx = 0; tlbr->idx < max_tlb_idx; tlbr->idx++)
    {
        for (tlbr->way = 0; tlbr->way < max_tlb_way; tlbr->way++)
        {
            if(tlb_ram_get_data(tlbr))
            {
                continue;
            }

            if(tlbr->data[0] & 0x1) /* data is valid, dump it*/
            {

                tlb_dump_msg("way:%d, index = %d, data0=0x%08x, data1=0x%08x, data2=0x%08x\n", tlbr->way, tlbr->idx, tlbr->data[0], tlbr->data[1], tlbr->data[2]);

                if(tlbr->idx < 128)
                {
                    /* Main TLB RAM
                     * [68:41] PA Physical Address.
                     * [33:26] ASID Address Space Identifier.
                     * [25:18] VMID Virtual Machine Identifier.
                     * [17:5] VA Virtual address.
                     */
                    asid = ((tlbr->data[1] & 0x3) << 6) | ((tlbr->data[0] & 0xFC000000) >> 26);
                    vmid = (tlbr->data[0] >> 18) & 0xFF;
                    vaddr = (tlbr->data[0] >> 5) & 0x1FFFF;
                    paddr = ((tlbr->data[2] & 0x1F) << 23) | ((tlbr->data[1] >> 9) &0x7FFFFF);
                    nGb = (tlbr->data[1] >> 2) &0x1;

                    if((in_paddr & 0xFFFFF000) == (paddr << 12)) /* 4kb size */
                    {
                        tlb_dump_msg("way:%d, index = %d, data0=0x%08x, data1=0x%08x, data2=0x%08x\n", tlbr->way, tlbr->idx, tlbr->data[0], tlbr->data[1], tlbr->data[2]);
                        tlb_dump_msg("Main TLBX: asid=0x%x nGb=0x%x, vmid=0x%x, pa=0x%x, va=0x%x\n", asid, nGb, vmid, paddr, vaddr);
                        ret = asid;
                    }
                }
#if 0 //only main tlb, if someone need to dump other. please fix it.
                else if(tlbr->idx < 160)
                {
                    /* Walk cache RAM
                     * [77:48] PA The physical address of the level 3 (LPAE) or level 2 (VMSAv7) table
                     * [47:41] VA Virtual address
                     * [33:26] ASID Indicates the Address Space Identifier
                     * [25:18] VMID Virtual Machine Identifier
                     */
                    asid = ((tlbr->data[1] & 0x3) << 6) | ((tlbr->data[0] & 0xFC000000) >> 26);
                    vmid = (tlbr->data[0] >> 18) & 0xFF;
                    vaddr = (tlbr->data[1] >> 9) & 0x7F;
                    paddr = ((tlbr->data[2] & 0x3FFF) << 16) | ((tlbr->data[1] >> 16) & 0xFFFF);
                    nGb = (tlbr->data[1] >> 2) &0x1;
                    ret = asid;
                    tlb_dump_msg("Walk cache RAM: asid=0x%x, vmid=0x%x, pa=0x%x, va=0x%x\n", asid, vmid, paddr, vaddr);
                }
                else
                {
                    /* IPA cache RAM
                     * [58:31] PA Physical address
                     * [25:18] VMID Virtual Machine Identifier
                     */
                    vmid = (tlbr->data[0] >> 18) & 0xFF;
                    paddr = ((tlbr->data[2] & 0x7FFFFFF) << 1) | ((tlbr->data[0] >> 31) & 0x1);
                    tlb_dump_msg("IPA cache RAM: vmid=0x%x, pa=0x%x\n", vmid, paddr);
                }
#endif
            }

        }
    }
    __enable_dcache();
    spin_unlock_irqrestore(&tlb_dump_lock, flags);
    return ret;

}

void dump_tlb_ram()
{
    /*  Cortex™-A7 MPCore, TRM
     *
     *  TLB Data Read Operation Register Write-only MCR p15, 3, <Rd>, c15, c4, 2 Index/Way
     *
     *  Table 6-7 TLB Data Read Operation Register location encoding
     *  Bit-field of Rd Description
     *  [31] TLB way
     *  [30:8] Unused
     *  [7:0] TLB index
     *        TLB index[7:0] Format
     *        0-127 Main TLB RAM, see Main TLB RAM
     *        128-159 Walk cache RAM, see Walk cache RAM on page 6-14
     *        160-191 IPA cache RAM, see IPA cache RAM on page 6-15
     *        192-255 Unused
     */
    struct tlb_ram_conf tlb_r_conf;
    struct tlb_ram_conf *tlbr = &tlb_r_conf;
    unsigned int max_tlb_way = 2;
    unsigned int max_tlb_idx = 192;

    unsigned int asid;
    unsigned int vmid;
    unsigned int vaddr, paddr;
    unsigned long flags;
    unsigned int id_mmfr3 = 0;
    spin_lock_irqsave(&tlb_dump_lock, flags);

    __disable_dcache();

    /*
     * MRC p15, 0, <Rt>, c0, c1, 7 ; Read ID_MMFR3 into Rt
     * Maintenance broadcast, bits[15:12]
     * Indicates whether Cache, TLB and branch predictor operations are broadcast. Permitted values are:
     * 0b0000 Cache, TLB and branch predictor operations only affect local structures.
     * 0b0001 Cache and branch predictor operations affect structures according to shareability and
     * defined behavior of instructions. TLB operations only affect local structures.
     * 0b0010 Cache, TLB and branch predictor operations affect structures according to shareability
     * and defined behavior of instructions.
     */

    asm volatile(
        "MRC p15, 0, %0, c0, c1, 7\n"
        : "+r" (id_mmfr3)
        :
        : "cc"
        );

    tlb_dump_msg("id_mmfr3 = 0x%08x, Maintenance broadcast, bits[15:12]=0x%x\n", id_mmfr3, (id_mmfr3 >> 12) & 0xF);


    for (tlbr->idx = 0; tlbr->idx < max_tlb_idx; tlbr->idx++)
    {
        for (tlbr->way = 0; tlbr->way < max_tlb_way; tlbr->way++)
        {
            if(tlb_ram_get_data(tlbr))
            {
                continue;
            }

            if(tlbr->data[0] & 0x1) /* data is valid, dump it*/
            {
                tlb_dump_msg("way:%d, index = %d, data0=0x%08x, data1=0x%08x, data2=0x%08x\n", tlbr->way, tlbr->idx, tlbr->data[0], tlbr->data[1], tlbr->data[2]);

                if(tlbr->idx < 128)
                {
                    /* Main TLB RAM
                     * [68:41] PA Physical Address.
                     * [33:26] ASID Address Space Identifier.
                     * [25:18] VMID Virtual Machine Identifier.
                     * [17:5] VA Virtual address.
                     */
                    asid = ((tlbr->data[1] & 0x3) << 6) | ((tlbr->data[0] & 0xFC000000) >> 26);
                    vmid = (tlbr->data[0] >> 18) & 0xFF;
                    vaddr = (tlbr->data[0] >> 5) & 0x1FFFF;
                    paddr = ((tlbr->data[2] & 0x1F) << 23) | ((tlbr->data[1] >> 9) &0x7FFFFF);
                    tlb_dump_msg("Main TLB: asid=0x%x, vmid=0x%x, pa=0x%x, va=0x%x\n", asid, vmid, paddr, vaddr);
                }
                else if(tlbr->idx < 160)
                {
                    /* Walk cache RAM
                     * [77:48] PA The physical address of the level 3 (LPAE) or level 2 (VMSAv7) table
                     * [47:41] VA Virtual address
                     * [33:26] ASID Indicates the Address Space Identifier
                     * [25:18] VMID Virtual Machine Identifier
                     */
                    asid = ((tlbr->data[1] & 0x3) << 6) | ((tlbr->data[0] & 0xFC000000) >> 26);
                    vmid = (tlbr->data[0] >> 18) & 0xFF;
                    vaddr = (tlbr->data[1] >> 9) & 0x7F;
                    paddr = ((tlbr->data[2] & 0x3FFF) << 16) | ((tlbr->data[1] >> 16) & 0xFFFF);
                    tlb_dump_msg("Walk cache RAM: asid=0x%x, vmid=0x%x, pa=0x%x, va=0x%x\n", asid, vmid, paddr, vaddr);

                }
                else
                {
                    /* IPA cache RAM
                     * [58:31] PA Physical address
                     * [25:18] VMID Virtual Machine Identifier
                     */
                    vmid = (tlbr->data[0] >> 18) & 0xFF;
                    paddr = ((tlbr->data[2] & 0x7FFFFFF) << 1) | ((tlbr->data[0] >> 31) & 0x1);
                    tlb_dump_msg("IPA cache RAM: vmid=0x%x, pa=0x%x\n", vmid, paddr);
                }
            }

        }
    }
    __enable_dcache();

    spin_unlock_irqrestore(&tlb_dump_lock, flags);

}

static int paddr_is_in_range(unsigned int paddr, unsigned int p_size, unsigned int v_start, unsigned int v_end)
{
    unsigned int v_addr = v_start & 0xFFFFF000;
    unsigned int tmp_addr;

    while(v_addr <= v_end)
    {
        tmp_addr = (unsigned int)arm_virt_to_phy(v_addr);

        if(paddr <= tmp_addr && tmp_addr <= (paddr + p_size))
        {
            tlb_dump_msg("va(0x%08x)=pa(0x%08x) is in paddr=0x%08x, size=0x%x\n", v_addr, tmp_addr, paddr, p_size);
            return true;
        }
        v_addr += 0x1000;
    }
    return false;
}

int range_in_tlb_ram(unsigned long in_asid, unsigned long in_start, unsigned long in_end)
{
    struct tlb_ram_conf tlb_r_conf;
    struct tlb_ram_conf *tlbr = &tlb_r_conf;
    unsigned long asid;
    unsigned long vmid, size;
    unsigned long vaddr, paddr, mask;
    unsigned long flags;
    int ret = false;

    unsigned int max_tlb_way = 2;
    unsigned int max_tlb_idx = 128; /* check main tlb only */

    spin_lock_irqsave(&tlb_dump_lock, flags);

    __disable_dcache();

    for (tlbr->idx = 0; tlbr->idx < max_tlb_idx; tlbr->idx++)
    {
        for (tlbr->way = 0; tlbr->way < max_tlb_way; tlbr->way++)
        {
            if(tlb_ram_get_data(tlbr))
            {
                continue;
            }

            /* Main TLB RAM
             * [68:41] PA Physical Address.
             * [33:26] ASID Address Space Identifier.
             * [25:18] VMID Virtual Machine Identifier.
             * [17:5] VA Virtual address.
             * [3:1] Size This field indicates the VMSA v7 or LPAE TLB RAM size.
             * VMSA v7:
             * 0b000 4KB.
             * 0b010 64KB.
             * 0b100 1MB.
             * 0b110 16MB.
             */
            asid = ((tlbr->data[1] & 0x3) << 6) | ((tlbr->data[0] & 0xFC000000) >> 26);
            //tlb_dump_debug("Main TLB: asid=0x%x, vmid=0x%x, pa=0x%x, va=0x%x\n", asid, vmid, paddr, vaddr);
#if 0 //check all ?
            if((in_asid & 0xFF) == asid)
#endif
            {
                //tlb_dump_debug("ta->ta_vma->vm_mm->context.id = 0x%x\n", ta->ta_vma->vm_mm->context.id);
                //tlb_dump_debug("ta->ta_start = 0x%08x\n", ta->ta_start);
                //tlb_dump_debug("ta->ta_end   = 0x%08x\n", ta->ta_end);

                //tlb_dump_debug("way:%d, index = %d, data0=0x%08x, data1=0x%08x, data2=0x%08x\n",
                //tlbr->way, tlbr->idx, tlbr->data[0], tlbr->data[1], tlbr->data[2]);

                vmid = (tlbr->data[0] >> 18) & 0xFF;
                vaddr = (tlbr->data[0] >> 5) & 0x1FFFF;
                paddr = ((tlbr->data[2] & 0x1F) << 23) | ((tlbr->data[1] >> 9) &0x7FFFFF);
                size = (tlbr->data[0] >> 1) & 0x7;

                /* real size*/
                size = 0x1 << (12 + 2 * size); /* 1 << 12 is 4KB, 1 << 16 is 64KB, 1 << 20 is 1MB, 1 << 24 is 16MB */

                vaddr = vaddr << 14;
                //tlb_dump_debug("Main TLB: asid=0x%x, vmid=0x%x, pa=0x%x, va[31:19]=0x%x size=0x%08x\n", asid, vmid, paddr, vaddr, size);

                mask = 0xFFF80000;

                if(((vaddr & mask) < (in_start & mask)) ||
                   ((vaddr & mask) > (in_end & mask)))
                {
                    continue;
                }

                if(paddr_is_in_range(paddr << 12, size, in_start, in_end))
                {
                    tlb_dump_msg("way:%d, index = %d, data0=0x%08x, data1=0x%08x, data2=0x%08x\n",
                                   tlbr->way, tlbr->idx, tlbr->data[0], tlbr->data[1], tlbr->data[2]);
                    tlb_dump_msg("input: asid=0x%lu, start=0x%08lu, end=0x%08lu\n", in_asid, in_start, in_end);
                    tlb_dump_msg("Main TLB: asid=0x%lu, vmid=0x%lu, pa=0x%lu, va[31:19]=0x%08lu size=0x%08lu\n", asid, vmid, paddr, vaddr, size);
                    ret = true;
                }
            }
        }
    }

    __enable_dcache();

    spin_unlock_irqrestore(&tlb_dump_lock, flags);
    return ret;
}

void dump_data_cache_L1(){
    unsigned int cache_tag;
    unsigned int cache_level = 0;
    unsigned int cache_way;
    unsigned int cache_set;
    unsigned int elem_idx;
    unsigned int cache_data1=0,cache_data2=0;
    unsigned int tag_address;
    unsigned int sec;
    unsigned int tag_MOESI;
    unsigned int dirty_MOESI;
    unsigned int outer_memory_att;
    unsigned int data_cache_size = 64;
    unsigned int max_cache_set = 0x80;
    unsigned int max_cache_way = 0x4;

    __inner_clean_dcache_L1();
    __disable_dcache();
    printk("[CPU%d] Dump L1 D cache...\n",smp_processor_id());
    printk("ADDRESS\tSEC\tSET\tWAY\tMOESI\t00\t04\t08\t0C\t10\t14\t18\t1C\t20\t24\t28\t2C\t30\t34\t38\t3C\n");
    for (cache_set = 0; cache_set < max_cache_set; cache_set++)
    {
        printk("cache_set:%d\n",cache_set);
        for (cache_way = 0; cache_way < max_cache_way; cache_way++)
        {
            printk("cache_way:%d\n",cache_way);
            cache_tag = (cache_way << 30) | (cache_set << 6);
            asm volatile(
                "MCR p15, 2, %0, c0, c0, 0\n"
                "MCR p15, 3, %1, c15, c2, 0\n"
                "MRC p15, 3, %2, c15, c0, 0\n"
                "MRC p15, 3, %3, c15, c0, 1\n"
                : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1), "+r" (cache_data2)
                :
                : "cc"
                );
            tag_address = (cache_data2 & 0x1FFFFFFF) << 11;
            sec = (cache_data2 & 0x20000000)>>29;
            tag_MOESI = (cache_data2 & 0xC0000000)>>30;
            dirty_MOESI = cache_data1 & 0x3;
            outer_memory_att = cache_data1 & 0x1C;

            printk("%x\t %x\t %x\t %x\t %x:%x\t",tag_address+cache_set*data_cache_size,sec,cache_set,cache_way,tag_MOESI,dirty_MOESI);
            for (elem_idx = 0; elem_idx < 8; elem_idx++)
            {
                cache_tag = (cache_way << 30) | (cache_set << 6) | (elem_idx << 3);
                asm volatile(
                    "MCR p15, 2, %0, c0, c0, 0\n"
                    "MCR p15, 3, %1, c15, c4, 0\n"
                    "MRC p15, 3, %2, c15, c0, 0\n"
                    "MRC p15, 3, %3, c15, c0, 1\n"
                    : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1), "+r" (cache_data2)

                    :
                    : "cc"
                    );
                printk("%x\t %x\t",cache_data1,cache_data2);
            }
            printk("\n");
        }
    }
    __enable_dcache();
}

void dump_data_cache_L2(void){
    unsigned int cache_tag;
    unsigned int cache_level = 2;
    unsigned int cache_way;
    unsigned int cache_set;
    unsigned int elem_idx;
    unsigned int cache_data1=0,cache_data2=0;
    unsigned int tag_address;
    unsigned int sec;
    unsigned int tag_MOESI;
    unsigned int dirty_MOESI;
    unsigned int outer_memory_att;
    unsigned int data_cache_size = 64;
    unsigned int max_cache_set = 0x800;
    unsigned int max_cache_way = 0x8;

    __inner_clean_dcache_L2();
    __disable_dcache();
    printk("[CPU%d] Dump L2 D cache...\n",smp_processor_id());
    printk("ADDRESS\tSEC\tSET\tWAY\tMOESI\t00\t04\t08\t0C\t10\t14\t18\t1C\t20\t24\t28\t2C\t30\t34\t38\t3C\n");

    for (cache_set = 0; cache_set < max_cache_set; cache_set++)
    {
        printk("cache_set:%d\n",cache_set);
        for (cache_way = 0; cache_way < max_cache_way; cache_way++)
        {
            printk("cache_way:%d\n",cache_way);
            cache_tag = (cache_way << 30) | (cache_set << 6);
            asm volatile(
                "MCR p15, 2, %0, c0, c0, 0\n"
                "MCR p15, 3, %1, c15, c2, 0\n"
                "MRC p15, 3, %2, c15, c0, 0\n"
                "MRC p15, 3, %3, c15, c0, 1\n"
                : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1), "+r" (cache_data2)
                :
                : "cc"
                );
            tag_address = (cache_data2 & 0x1FFFFFFF) << 11;
            sec = (cache_data2 & 0x20000000)>>29;
            tag_MOESI = (cache_data2 & 0xC0000000)>>30;
            dirty_MOESI = cache_data1 & 0x3;
            outer_memory_att = cache_data1 & 0x1C;

            printk("%x\t %x\t %x\t %x\t %x:%x\t",tag_address+cache_set*data_cache_size,sec,cache_set,cache_way,tag_MOESI,dirty_MOESI);
            for (elem_idx = 0; elem_idx < 8; elem_idx++)
            {
                cache_tag = (cache_way << 30) | (cache_set << 6) | (elem_idx << 3);
                asm volatile(
                    "MCR p15, 2, %0, c0, c0, 0\n"
                    "MCR p15, 3, %1, c15, c4, 0\n"
                    "MRC p15, 3, %2, c15, c0, 0\n"
                    "MRC p15, 3, %3, c15, c0, 1\n"
                    : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1), "+r" (cache_data2)

                    :
                    : "cc"
                    );
                printk("%x\t %x\t",cache_data1,cache_data2);
            }
            printk("\n");
        }
    }
    __enable_dcache();
}

unsigned long mt_icache_dump(unsigned int cache_way, unsigned int cache_set, unsigned int elem_idx)
{
    unsigned int cache_level = 1;
    unsigned int cache_data1 = 0, cache_data2 = 0;
    unsigned int cache_tag;
    __disable_icache();

    cache_tag = (cache_way << 31) | (cache_set << 5) | (elem_idx << 2);
    dsb();
    asm volatile(
        "MCR p15, 2, %0, c0, c0, 0\n"
        "isb\n"
        "MCR p15, 3, %1, c15, c4, 1\n"
        "isb\n"
        "MRC p15, 3, %2, c15, c0, 0\n"
        "MRC p15, 3, %3, c15, c0, 1\n"
        : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1), "+r" (cache_data2)
        :
        : "cc"
        );
    dsb();
    cache_data1 &= ~(0x3 << 16);
    cache_data2 &= ~(0x3 << 16);
    //printk("%x%x\t",cache_data1,cache_data2);

    __enable_icache();

    return cache_data1 << 16 | cache_data2;
}



void dump_inst_cache(const char *lvl, unsigned long v_addr)
{
    /* dump a line of addr */
    unsigned int cache_level = 1;
    unsigned int tag, way, set, idx;
    unsigned int tag_address, cache_data1 = 0;
    unsigned long p_addr;
    char str[sizeof(" 12345678") * 8 + 1];

    memset(str, ' ', sizeof(str));
    str[sizeof(str) - 1] = '\0';

    p_addr = __virt_to_phys(v_addr);

    printk("%sdump icache. virtual addr:%lu physical addr:%lu\n", lvl, v_addr, p_addr);

    //tag = (p_addr >> 12) << 12;
    set = (p_addr >> 5) & (0x200 - 1);
    idx = (p_addr >> 2) & 0x7;

    for (way = 0; way < 2; way++)
    {
        tag = (way << 31) | (set << 5);
        dsb();
        asm volatile(
            "MCR p15, 2, %0, c0, c0, 0\n"
            "isb\n"
            "MCR p15, 3, %1, c15, c2, 1\n"
            "isb\n"
            "MRC p15, 3, %2, c15, c0, 0\n"
            : "+r" (cache_level),"+r" (tag),"+r" (cache_data1)
            :
            : "cc"
            );

        dsb();

        tag_address =  (cache_data1 & 0xFFFFFFF) << 12;

        if(((p_addr >> 12) << 12) == tag_address)
        {


            for(idx = 0; idx < 8; idx++)
            {
                sprintf(str + idx * 9, " %08lx", mt_icache_dump(way, set, idx));
            }

            printk("%stag:%x way:%x set:0x%x valid:%x sec:%x %s\n",
                   lvl,
                   tag_address,                        //address
                   way,                                //way
                   set,                                //set
                   (cache_data1 & 0x40000000) >> 30,   //valid
                   (cache_data1 & 0x10000000) >> 28,   //security
                   (cache_data1 & 0x20000000) >> 29 ? "arm" : "thumb"   //arm state
                );

            printk("%s%x   %s\n",
                   lvl,
                   tag_address | (set << 5) , //address
                   str);

        }
    }

}


void dump_inst_cache_all(){
    unsigned int cache_tag;
    unsigned int cache_level = 1;
    unsigned int cache_way;
    unsigned int cache_set;
    unsigned int elem_idx;
    unsigned int cache_data1 = 0, cache_data2 = 0, cache_data3 = 0;
    unsigned int valid;
    unsigned int tag_address;
    unsigned int sec;
    unsigned int arm_state;
    unsigned int inst_cache_size = 32;

    __disable_icache();

    printk("[CPU%d] Dump L1 I cache...\n",smp_processor_id());
    printk("ADDRESS\tSEC\tSET\tWAY\tVALID\tarm_state\t00\t04\t08\t0C\t10\t14\t18\t1C\n");
    for (cache_set = 0; cache_set < 512; cache_set++)
    {
        for (cache_way = 0; cache_way < 2; cache_way++)
        {
            cache_tag = (cache_way << 31) | (cache_set << 5);
            asm volatile(
                "MCR p15, 2, %0, c0, c0, 0\n"
                "isb\n"
                "MCR p15, 3, %1, c15, c2, 1\n"
                "isb\n"
                "MRC p15, 3, %2, c15, c0, 0\n"
                "MRC p15, 3, %3, c15, c0, 1\n"
                "MRC p15, 3, %4, c15, c0, 2\n"
                : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1) ,"+r" (cache_data2) ,"+r" (cache_data3)
                :
                : "cc"
                );
            valid = (cache_data1 & 0x40000000) >> 30;
            arm_state = (cache_data1 & 0x20000000) >> 29;
            sec = (cache_data1 & 0x10000000) >> 28;
            tag_address =  (cache_data1 & 0xFFFFFFF) << 12;
            if (!valid)
                continue;
            printk("%x\t%x\t%x\t%x\t%x\t%x\t",tag_address+cache_set*inst_cache_size,sec,cache_set,cache_way,valid,arm_state);
            for (elem_idx = 0; elem_idx < 8; elem_idx++)
            {
                cache_tag = (cache_way << 31) | (cache_set << 5) | (elem_idx << 2);
                asm volatile(
                    "MCR p15, 2, %0, c0, c0, 0\n"
                    "isb\n"
                    "MCR p15, 3, %1, c15, c4, 1\n"
                    "isb\n"
                    "MRC p15, 3, %2, c15, c0, 0\n"
                    "MRC p15, 3, %3, c15, c0, 1\n"
                    : "+r" (cache_level),"+r" (cache_tag),"+r" (cache_data1), "+r" (cache_data2)
                    :
                    : "cc"
                    );
                cache_data1 &= ~(0x3 << 16);
                cache_data2 &= ~(0x3 << 16);
                printk("%x%x\t",cache_data1,cache_data2);
            }
            printk("\n");
        }
    }

    __enable_icache();
}


/*
 *
 * Return 0.
 */
int mt_cache_dump_init(void)
{

    return 0;
}

/*
 * mt_cache_dump_deinit:
 * Return 0.
 */
int mt_cache_dump_deinit(void)
{
    return 0;
}

static ssize_t cache_dump_store(struct device_driver *driver, const char *buf,
                                size_t count)
{
    char *p = (char *)buf;
    unsigned int num;

    num = simple_strtoul(p, &p, 10);
    get_online_cpus();
    switch(num){
    case 1:
        on_each_cpu(dump_data_cache_L1, NULL, true);
        dump_data_cache_L2();
        break;
    case 2:
        on_each_cpu(dump_inst_cache_all, NULL, true);
        break;
    case 3:
        on_each_cpu(dump_inst_cache_all, NULL, true);
        on_each_cpu(dump_data_cache_L1, NULL, true);
        dump_data_cache_L2();
        break;
    case 4:
        on_each_cpu(dump_tlb_ram, NULL, true);
        break;
    }
    put_online_cpus();
    return count;
}
static ssize_t cache_dump_show(struct device_driver *driver, char *buf)
{
    int len = 0;
    char *p = buf;
    p +=  sprintf(p, "1:dump L1 D cache and L2 unified cache\n");
    p +=  sprintf(p, "2:dump I cache\n");
    p +=  sprintf(p, "3:dump I D cache\n");
    p +=  sprintf(p, "4:dump TLB ram\n");

    len = p - buf;
    return len;
}

DRIVER_ATTR(cache_dump, 0644, cache_dump_show, cache_dump_store);

static struct device_driver mt_cache_dump_drv =
{
    .name = "mt_cache_dump",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};



/*
 * mt_mon_mod_init: module init function
 */
static int __init mt_cache_dump_mod_init(void)
{
    int ret;

    /* register driver and create sysfs files */
    ret = driver_register(&mt_cache_dump_drv);

    if (ret) {
        printk("fail to register mt_cache_dump_drv\n");
        return ret;
    }
    ret = driver_create_file(&mt_cache_dump_drv, &driver_attr_cache_dump);

    if (ret) {
        printk("fail to create mt_cache_dump sysfs files\n");

        return ret;
    }


    return 0;
}

/*
 * mt_cache_dump_mod_exit: module exit function
 */
static void __exit mt_cache_dump_mod_exit(void)
{
}
module_init(mt_cache_dump_mod_init);
module_exit(mt_cache_dump_mod_exit);

