#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/memory.h>
#include <linux/memblock.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <mach/emi_mpu.h>

#define CONFIG_MTKPASR_MINDIESIZE_PFN		(0x10000)	/* 256MB */
#define MTKPASR_LESSRESTRICT_PFNS		(0x40000)	/* 1GB */

//#define NO_UART_CONSOLE
#ifndef NO_UART_CONSOLE
#define PRINT(len, string, args...) 	printk(KERN_EMERG string,##args)
#else
unsigned char mtkpasr_log_buf[4096];
static int log_stored = 0;
#define PRINT(len, string, args...) 	do { sprintf(mtkpasr_log_buf + log_stored,string,##args); log_stored += len; } while (0)
#endif

/* Struct for parsing rank information (SW view) */
struct view_rank {
	unsigned long start_pfn;	/* The 1st pfn */
	unsigned long end_pfn;		/* The pfn after the last valid one */
	unsigned long bank_pfn_size;	/* Bank size in PFN */
	unsigned long valid_channel;	/* Channels: 0x00000101 means there are 2 valid channels - 1st & 2nd (MAX: 4 channels) */
};
static struct view_rank rank_info[MAX_RANKS];

/* Basic DRAM configuration */
static struct basic_dram_setting pasrdpd;
/* PASR/DPD imposed start rank */
static int mtkpasr_start_rank;
/* MAX Banksize in pfns (from SW view) */
unsigned long pasrbank_pfns = 0;
/* We can't guarantee HIGHMEM zone is bank alignment, so we need another variable to represent it. */
static unsigned long mtkpasr_pfn_start = 0;
static unsigned long mtkpasr_pfn_end = 0;
/* Segment mask */
static unsigned long valid_segment = 0x0;

/* Set pageblock's mobility */
extern void set_pageblock_mobility(struct page *page, int mobility);

/* From dram_overclock.c */
extern bool pasr_is_valid(void)__attribute__((weak));
/* To confirm PASR is valid */
static inline bool could_do_mtkpasr(void)
{
	if (pasr_is_valid) {
		return pasr_is_valid();
	}

	return true;
}

extern void acquire_dram_setting(struct basic_dram_setting *pasrdpd)__attribute__((weak));

/* 
 * Parse DRAM setting - transform DRAM setting to temporary bank structure.
 */
static void __meminit parse_dram_setting(unsigned long hint)
{
	int channel_num, chan, rank, check_segment_num;
	unsigned long valid_channel;
	unsigned long check_rank_size, rank_pfn, start_pfn = ARCH_PFN_OFFSET;

	PRINT(29, "ARCH_PFN_OFFSET [0x%8x]\n",ARCH_PFN_OFFSET);

	if (acquire_dram_setting) {
		acquire_dram_setting(&pasrdpd);
		channel_num = pasrdpd.channel_nr;
		/* By ranks */
		for (rank = 0; rank < MAX_RANKS; ++rank) {
			rank_pfn = 0;
			rank_info[rank].valid_channel = 0x0; 
			valid_channel = 0x1;
			check_rank_size = 0x0;
			check_segment_num = 0x0;
			for (chan = 0; chan < channel_num; ++chan) {
				if (pasrdpd.channel[chan].rank[rank].valid_rank) {
					rank_pfn += (pasrdpd.channel[chan].rank[rank].rank_size << (27 - PAGE_SHIFT));
					rank_info[rank].valid_channel |= valid_channel;
					/* Sanity check for rank size */
					if (!check_rank_size) {
						check_rank_size = pasrdpd.channel[chan].rank[rank].rank_size;
					} else {
						/* We only support ranks with equal size */
						if (check_rank_size != pasrdpd.channel[chan].rank[rank].rank_size) {
							BUG();
						}
					}
					/* Sanity check for segment number */
					if (!check_segment_num) {
						check_segment_num = pasrdpd.channel[chan].rank[rank].segment_nr;
					} else {
						/* We only support ranks with equal segment number */
						if (check_segment_num != pasrdpd.channel[chan].rank[rank].segment_nr) {
							BUG();
						}
					}
				}
				valid_channel <<= 8;
			}
			/* Have we found a valid rank */
			if (check_rank_size != 0 && check_segment_num != 0) {
				rank_info[rank].start_pfn = start_pfn;
				rank_info[rank].end_pfn = start_pfn + rank_pfn;
				rank_info[rank].bank_pfn_size = rank_pfn/check_segment_num;
				start_pfn = rank_info[rank].end_pfn;
				PRINT(96, "Rank[%d] start_pfn[%8lu] end_pfn[%8lu] bank_pfn_size[%8lu] valid_channel[0x%-8lx]\n",
						rank, rank_info[rank].start_pfn, rank_info[rank].end_pfn, 
						rank_info[rank].bank_pfn_size, rank_info[rank].valid_channel);
			} else {
				rank_info[rank].start_pfn = ARCH_PFN_OFFSET;
				rank_info[rank].end_pfn = ARCH_PFN_OFFSET;
				rank_info[rank].bank_pfn_size = 0;
				rank_info[rank].valid_channel = 0x0;
			}
		}
	} else {
		/* Single channel, dual ranks, 8 segments per rank - Get a hint from system */
		rank_pfn = (hint + CONFIG_MTKPASR_MINDIESIZE_PFN - 1) & ~(CONFIG_MTKPASR_MINDIESIZE_PFN - 1);
		rank_pfn >>= 1;
		for (rank = 0; rank < 2; ++rank) {
			rank_info[rank].start_pfn = start_pfn;
			rank_info[rank].end_pfn = start_pfn + rank_pfn;
			rank_info[rank].bank_pfn_size = rank_pfn >> 3;
			rank_info[rank].valid_channel = 0x1;
			start_pfn = rank_info[rank].end_pfn;
			PRINT(96, "Rank[%d] start_pfn[%8lu] end_pfn[%8lu] bank_pfn_size[%8lu] valid_channel[0x%-8lx]\n",
					rank, rank_info[rank].start_pfn, rank_info[rank].end_pfn, 
					rank_info[rank].bank_pfn_size, rank_info[rank].valid_channel);
		}
		/* Reset remaining ranks */
		for (; rank < MAX_RANKS; ++rank) {
			rank_info[rank].start_pfn = ARCH_PFN_OFFSET;
			rank_info[rank].end_pfn = ARCH_PFN_OFFSET;
			rank_info[rank].bank_pfn_size = 0;
			rank_info[rank].valid_channel = 0x0;
		}
	}
}

/* Check whether it is a valid rank */
static bool __init is_valid_rank(int rank)
{
	/* Check start/end pfn */
	if (rank_info[rank].start_pfn == rank_info[rank].end_pfn) {
		return false;
	}

	/* Check valid_channel */
	if (rank_info[rank].valid_channel == 0x0) {
		return false;
	}

	return true;
}

#if 0
/* Show memblock */
void show_memblock(void)
{
	struct memblock_region *reg;
	phys_addr_t start;
	phys_addr_t end;

	for_each_memblock(memory, reg) {
		start = reg->base;
		end = start + reg->size;
                printk(KERN_EMERG"[PHY layout]kernel   :   0x%08llx - 0x%08llx (0x%08llx)\n",
				(unsigned long long)start,
				(unsigned long long)end - 1,
				(unsigned long long)reg->size);
	}
	
	for_each_memblock(reserved, reg) {
		start = reg->base;
		end = start + reg->size;
                printk(KERN_EMERG"[PHY layout]reserved   :   0x%08llx - 0x%08llx (0x%08llx)\n",
				(unsigned long long)start,
				(unsigned long long)end - 1,
				(unsigned long long)reg->size);

	}
}
#endif

#define PFN_TO_PADDR(x)	(phys_addr_t)(x << PAGE_SHIFT)

/* Fill valid_segment */
static void __init mark_valid_segment(phys_addr_t start, phys_addr_t end)
{
	int num_segment, rank;
	unsigned long spfn, epfn;

	num_segment = 0;
	for (rank = 0; rank < MAX_RANKS; ++rank) {
		spfn = ((start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) >> PAGE_SHIFT;		/* Round-up */
		epfn = (end & ~(PAGE_SIZE - 1)) >> PAGE_SHIFT;					/* Round-down */
		if (is_valid_rank(rank)) {
			spfn = max(spfn, rank_info[rank].start_pfn);
			if (rank_info[rank].end_pfn > spfn) {
				spfn = (spfn + rank_info[rank].bank_pfn_size - 1) & ~(rank_info[rank].bank_pfn_size - 1);
				epfn = min(epfn, rank_info[rank].end_pfn);
				while (epfn >= (spfn + rank_info[rank].bank_pfn_size)) {
					valid_segment |= (1 << ((spfn - rank_info[rank].start_pfn) / rank_info[rank].bank_pfn_size + num_segment));
					spfn += rank_info[rank].bank_pfn_size;
				}
			}
			num_segment += (rank_info[rank].end_pfn - rank_info[rank].start_pfn) / rank_info[rank].bank_pfn_size;
		}
	}
}

/* Set page mobility to MIGRATE_MTKPASR */
static void __init set_page_mobility_mtkpasr(phys_addr_t start, phys_addr_t end)
{
	int rank;
	unsigned long spfn, epfn, espfn, pfn;
	struct page *page;

	for (rank = 0; rank < MAX_RANKS; ++rank) {
		spfn = ((start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) >> PAGE_SHIFT;		/* Round-up */
		epfn = (end & ~(PAGE_SIZE - 1)) >> PAGE_SHIFT;					/* Round-down */
		if (is_valid_rank(rank)) {
			spfn = max(spfn, rank_info[rank].start_pfn);
			if (rank_info[rank].end_pfn > spfn) {
				spfn = (spfn + rank_info[rank].bank_pfn_size - 1) & ~(rank_info[rank].bank_pfn_size - 1);
				epfn = min(epfn, rank_info[rank].end_pfn);
				espfn = spfn + rank_info[rank].bank_pfn_size;
				while (epfn >= espfn) {
					/* Set page mobility to MIGRATE_MTKPASR */
					for (pfn = spfn; pfn < espfn; pfn++) {
						/* If invalid */
						if (!early_pfn_valid(pfn))
							continue;		
						/* Set it as MIGRATE_MTKPASR */
						page = pfn_to_page(pfn);
						if(!(pfn & (pageblock_nr_pages - 1)))
							set_pageblock_mobility(page, MIGRATE_MTKPASR);
					}
					spfn += rank_info[rank].bank_pfn_size;
					espfn = spfn + rank_info[rank].bank_pfn_size;
				}
			}
		}
	}
}

/* Fill valid_segment & set page mobility */
static void __init construct_mtkpasr_range(void)
{
	phys_addr_t vstart, vend;
	struct memblock_region *reg;
	phys_addr_t start;
	phys_addr_t end;

	/* memblock should be sorted! */
	for_each_memblock(memory, reg) {
		vstart = PFN_TO_PADDR(mtkpasr_pfn_start);
		vend = PFN_TO_PADDR(mtkpasr_pfn_end);
		start = reg->base;
		end = start + reg->size;
		/* Intersect */
		if (end > vstart && start < vend) {
			vstart = max(start, vstart);
			vend = min(end, vend);
			/* Mark valid segment */
			mark_valid_segment(vstart, vend);			
			/* Set page mobility */
			set_page_mobility_mtkpasr(vstart, vend);
		}
	}

	/* Last bank - TODO */
}

/* 
 * We will set an offset on which active PASR will be imposed. 
 * This is done by setting those pages as MIGRATE_MTKPASR type.
 * It only takes effect on HIGHMEM zone now! 
 */
void __meminit init_mtkpasr_range(struct zone *zone)
{
	struct pglist_data *pgdat;
	int rank;
	unsigned long start_pfn;
	unsigned long end_pfn;
	unsigned long pfn_bank_alignment = 0;

	/* Check whether our platform supports PASR */
	if (!could_do_mtkpasr()) {
		return;
	}

	/* Indicate node */
	pgdat = zone->zone_pgdat;

	/* Parsing DRAM setting */
	parse_dram_setting(pgdat->node_spanned_pages);

#ifdef CONFIG_HIGHMEM
	/* Start from HIGHMEM zone if we have CONFIG_HIGHMEM defined. */
	zone = zone + ZONE_HIGHMEM;
#else
	zone = zone + ZONE_NORMAL;
#endif
	
	/* Sanity check - Is this zone empty? */
	if (!populated_zone(zone)) {
		return;
	}

	/* Mark the start pfn */
	start_pfn = zone->zone_start_pfn;
	end_pfn = start_pfn + zone->spanned_pages;

	/* Indicate the beginning pfn of PASR/DPD */
#ifdef CONFIG_HIGHMEM
	start_pfn += (pgdat->node_spanned_pages >> 4);
#else
	start_pfn += (pgdat->node_spanned_pages >> 2);
#endif

	/* Find out which rank "start_pfn" belongs to */
	for (rank = 0; rank < MAX_RANKS; ++rank) {
		if (start_pfn < rank_info[rank].end_pfn && start_pfn >= rank_info[rank].start_pfn) {
			mtkpasr_start_rank = rank;
			pfn_bank_alignment = rank_info[rank].bank_pfn_size;
			break;
		}
	}

	/* Sanity check */
	if (!pfn_bank_alignment) {
		return;
	}

	/* 1st attempted bank size */
	pasrbank_pfns = pfn_bank_alignment;

	/* Round up to bank alignment */
	start_pfn = (start_pfn + pfn_bank_alignment - 1) & ~(pfn_bank_alignment - 1);

	/* Find out which rank "end_pfn" belongs to */
	for (; rank < MAX_RANKS; ++rank) {
		if (end_pfn <= rank_info[rank].end_pfn && end_pfn > rank_info[rank].start_pfn) {
			pfn_bank_alignment = rank_info[rank].bank_pfn_size;
			break;
		}
	}

	/* Determine the final bank size */
	if (pasrbank_pfns < pfn_bank_alignment) {
		pasrbank_pfns = pfn_bank_alignment;
	}

#ifdef CONFIG_MTKPASR_NO_LASTBANK
	/* Align MTKPASR end pfn to bank alignment! (Round Down) */
	end_pfn = end_pfn & ~(pfn_bank_alignment - 1);
#endif

	/* Find out MTKPASR Start/End PFN */
	mtkpasr_pfn_start = start_pfn;
	mtkpasr_pfn_end	= end_pfn;

	/* Round up mtkpasr_pfn_end (a little tricky, affected by CONFIG_MTKPASR_NO_LASTBANK) */
	mtkpasr_pfn_end = (mtkpasr_pfn_end + pfn_bank_alignment - 1) & ~(pfn_bank_alignment - 1);

	/* Fix up - allow holes existing in the PASR range */
	construct_mtkpasr_range();
	
	PRINT(138, "[MTKPASR] @@@@@@ Start_pfn[%8lu] End_pfn[%8lu] (MTKPASR) start_pfn[%8lu] end_pfn[%8lu] Valid_segment[0x%8lx] @@@@@@\n", 
			start_pfn, end_pfn, mtkpasr_pfn_start, mtkpasr_pfn_end, valid_segment);
}

/* 
 * Helper of constructing Memory (Virtual) Rank & Bank Information -
 *
 * start_pfn 	  - Pfn of the 1st page in that pasr range (Should be bank-aligned)
 * end_pfn   	  - Pfn of the one after the last page in that pasr range (Should be bank-aligned)
 * 		    (A hole may exist between end_pfn & bank-aligned(last_valid_pfn))
 * banks_per_rank - Number of banks in a rank
 *
 * Return    - The number of memory (virtual) banks, -1 means no valid range for PASR
 */
int __init compute_valid_pasr_range(unsigned long *start_pfn, unsigned long *end_pfn, int *num_ranks)
{
	int num_banks, rank, seg_num;
	unsigned long vseg = valid_segment;
	bool contain_rank;
	
	/* Check whether our platform supports PASR */
	if (!could_do_mtkpasr()) {
		return -1;
	}

	/* Set PASR/DPD range */
	*start_pfn = mtkpasr_pfn_start;
	*end_pfn = mtkpasr_pfn_end;

	/* Compute number of banks & ranks*/
	num_banks = 0;
	*num_ranks = 0;
	for (rank = 0; rank < MAX_RANKS; ++rank) {
		if (is_valid_rank(rank)) {
			contain_rank = true;
			seg_num = (rank_info[rank].end_pfn - rank_info[rank].start_pfn) / rank_info[rank].bank_pfn_size;
			while (seg_num--) {
				if (vseg & 0x1) {
					num_banks++;
				} else {
					contain_rank = false;
				}
				vseg >>= 1;
			}
			if (contain_rank) {
				*num_ranks += 1;
			}
		}
	}

	/* No valid banks */
	if (num_banks == 0) {
		return -1;
	}

	return num_banks;
}

/*
 * Give bank, this function will return its (start_pfn, end_pfn) and corresponding rank 
 * ("fully == true" means we should identify whether whole bank's rank is in a PASRDPD-imposed range)
 */
int __init query_bank_information(int bank, unsigned long *spfn, unsigned long *epfn, bool fully)
{
	int seg_num = 0, rank, num_segment = 0;
	unsigned long vseg = valid_segment, valid_mask;

	/* Reset */
	*spfn = 0;
	*epfn = 0;

	/* Which segment */
	do {
		if (vseg & 0x1) {
			if (!bank) {
				/* Found! */
				break;
			}
			bank--;
		}
		vseg >>= 1;
		seg_num++;
	} while (seg_num < BITS_PER_LONG);

	/* Sanity check */
	if (seg_num == BITS_PER_LONG) {
		return -1;
	}

	/* Which rank */
	vseg = valid_segment;
	for (rank = 0; rank < MAX_RANKS; ++rank) {
		if (is_valid_rank(rank)) {
			num_segment = (rank_info[rank].end_pfn - rank_info[rank].start_pfn) / rank_info[rank].bank_pfn_size;
			if (seg_num < num_segment) {
				*spfn = rank_info[rank].start_pfn + seg_num * rank_info[rank].bank_pfn_size;
				*epfn = *spfn + rank_info[rank].bank_pfn_size;
				break;
			}
			seg_num -= num_segment;
			vseg >>= num_segment;
		}
	}

	/* Sanity check */
	if (rank == MAX_RANKS) {
		return -1;
	}

	/* Should acquire rank information according to "rank" */
	if (fully) {
		valid_mask = (1 << num_segment) - 1;
		if ((vseg & valid_mask) == valid_mask) {
			return rank;	
		}
	}

	return -1;
}

/*
 * Translate sw bank to physical dram segment.
 * This will output different translation results depends on what dram model our platform uses.
 * non-interleaving(1-channel) vs. interleaving(n-channel, n > 1)
 *
 * Now it only supports full-interleaving translation.
 */
u32 __init pasr_bank_to_segment(unsigned long start_pfn, unsigned long end_pfn)
{
	int num_segment, rank;

	num_segment = 0;
	for (rank = 0; rank < MAX_RANKS; ++rank) {
		if (is_valid_rank(rank)) {
			if (rank_info[rank].start_pfn <= start_pfn && rank_info[rank].end_pfn >= end_pfn) {
				break;
			}
			num_segment += (rank_info[rank].end_pfn - rank_info[rank].start_pfn) / rank_info[rank].bank_pfn_size;
		}
	}

	return ((start_pfn - rank_info[rank].start_pfn) / rank_info[rank].bank_pfn_size + num_segment);

	/*
	 *  Symmetric Interleaving
	 *  segment = (start_pfn - CONFIG_MEMPHYS_OFFSET) / pasrbank_pfns + dram_segment_offset_ch0;
	 *  // Dual-Channel   (n+n)
	 *  return segment | (segment << 8);
	 *  // Triple-Channel (n+n+n)
	 *  return segment | (segment << 8) | (segment << 16);
	 *  // Quad-Channel   (n+n+n+n)
	 *  return segment | (segment << 8) | (segment << 16) | (segment << 24);	 
	 */
}
