#ifndef _SRC_MAP_H_
#define _SRC_MAP_H_

#include <linux/mm.h>

#define INVALID_COOKIE ~0UL
#define NO_COOKIE 0UL

//#define MET_DEBUG_MCOOKIE

#ifdef MET_DEBUG_MCOOKIE

#define dbg_mcookie_tprintk(fmt, a...) \
		trace_printk("[DEBUG_MCOOKIE]%s, %d: "fmt, \
			__FUNCTION__, __LINE__ , ##a)

#define dbg_mcookie_printk(fmt, a...) \
		printk("[DEBUG_MCOOKIE]%s, %d: "fmt, \
			__FUNCTION__, __LINE__ , ##a)

#else // MET_DEBUG_MCOOKIE

#define dbg_mcookie_tprintk(fmt, a...) \
		no_printk(fmt, ##a)

#define dbg_mcookie_printk(fmt, a...) \
		no_printk(fmt, ##a)

#endif // MET_DEBUG_MCOOKIE

struct mm_struct *take_tasks_mm(struct task_struct *task);
void release_mm(struct mm_struct *mm);
unsigned long lookup_dcookie(struct mm_struct *mm, unsigned long addr, off_t *offset);
int src_map_start(void);
void src_map_stop(void);
void src_map_stop_dcookie(void);

void mark_done(int cpu);

/* support met cookie */
unsigned int get_mcookie(const char *name);
int dump_mcookie(char *buf, unsigned int size);
int init_met_cookie(void);
int uninit_met_cookie(void);

#endif //_SRC_MAP_H_
