#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/cpu.h>
#include <linux/module.h>

void cookie(char *taskname, char *fname, off_t off, unsigned long pc, int mod)
{
	if (mod)
		trace_printk("task=%s, pc=%lx, fn=%s.ko, off=%lx,\n", taskname, pc, fname, off);
	else
		trace_printk("task=%s, pc=%lx, fn=%s, off=%lx\n", taskname, pc, fname, off);
}

void add_cookie(struct pt_regs *regs, int cpu)
{
	unsigned long pc;
	off_t off;

	if (regs == 0)
		return;

	pc = profile_pc(regs);

	if (user_mode(regs)) {
		struct mm_struct *mm;
		struct vm_area_struct *vma;
		struct path *ppath;

		mm = current->mm;
		for (vma = find_vma(mm, s.pc); vma; vma = vma->vm_next) {

			if (s.pc < vma->vm_start || s.pc >= vma->vm_end)
				continue;

			if (vma->vm_file) {
				ppath = &(vma->vm_file->f_path);
				off = (vma->vm_pgoff << PAGE_SHIFT) + s.pc - vma->vm_start;
				cookie(current->comm, pc, (char *)(ppath->dentry->d_name.name), off, 0);
			} else {
				/* must be an anonymous map */
				cookie(current->comm, pc, "nofile", pc, 0);
			}
			break;
		}
	} else {
		struct module *mod = __module_address(s.pc);
		if (mod) {
			s.off = s.pc - (unsigned long)mod->module_core;
			cookie(current->comm, pc, mod->name, off, 1);
		} else {
			s.off = 0;
			cookie(current->comm, pc, "vmlinux", off, 0);
		}
	}
}
