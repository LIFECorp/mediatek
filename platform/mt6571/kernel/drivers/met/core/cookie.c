#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/irq_regs.h>

#include "met_drv.h"

static const char header[] =
	"# cookie: task_name,PC,cookie_name,offset\n"
	"met-info [000] 0.0: cookie_header: task_name,PC,cookie_name,offset\n";

void cookie(char *taskname, unsigned long pc, char *fname, off_t off, int mod)
{
	if (mod)
		trace_printk("%s,%lx,%s.ko,%lx\n", taskname, pc, fname, off);
	else
		trace_printk("%s,%lx,%s,%lx\n", taskname, pc, fname, off);
}

void met_cookie_polling(unsigned long long stamp, int cpu)
{
	struct pt_regs *regs;
	unsigned long pc;
	off_t off;

	regs = get_irq_regs();

	if (regs == 0)
		return;

	pc = profile_pc(regs);

	if (user_mode(regs)) {
		struct mm_struct *mm;
		struct vm_area_struct *vma;
		struct path *ppath;

		mm = current->mm;
		for (vma = find_vma(mm, pc); vma; vma = vma->vm_next) {

			if (pc < vma->vm_start || pc >= vma->vm_end)
				continue;

			if (vma->vm_file) {
				ppath = &(vma->vm_file->f_path);

				if (vma->vm_flags & VM_DENYWRITE)
					off = pc;
				else
					off = (vma->vm_pgoff << PAGE_SHIFT) + pc - vma->vm_start;

				cookie(current->comm, pc, (char *)(ppath->dentry->d_name.name), off, 0);
			} else {
				/* must be an anonymous map */
				cookie(current->comm, pc, "nofile", pc, 0);
			}
			break;
		}
	} else {
		struct module *mod = __module_address(pc);
		if (mod) {
			off = pc - (unsigned long)mod->module_core;
			cookie(current->comm, pc, mod->name, off, 1);
		} else {
			cookie(current->comm, pc, "vmlinux", pc, 0);
		}
	}
}


static void met_cookie_start(void)
{
	return;
}

static void met_cookie_stop(void)
{
	return;
}

static const char help[] = "  --cookie                              enable sampling task and PC\n";

static int met_cookie_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static int met_cookie_print_header(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, header);
}

struct metdevice met_cookie = {
	.name = "cookie",
	.type = MET_TYPE_PMU,
	.cpu_related = 1,
	.start = met_cookie_start,
	.stop = met_cookie_stop,
	.polling_interval = 1,
	.timed_polling = met_cookie_polling,
	.tagged_polling = met_cookie_polling,
	.print_help = met_cookie_print_help,
	.print_header = met_cookie_print_header,
};
