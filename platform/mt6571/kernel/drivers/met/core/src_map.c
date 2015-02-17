#include <linux/dcache.h>
#include <linux/types.h>
#include <asm-generic/errno.h>
#include <linux/dcookies.h>
#include <linux/profile.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#include <linux/slab.h>

#include "src_map.h"
#include "buffer.h"
#include "util.h"

#define MET_NAME_BUF_SIZE 	32
#define MET_HASH_SIZE		32

static cpumask_var_t marked_cpus;
static struct dcookie_user *dcookie = NULL;
static unsigned int *met_crc32_table;
struct met_cookie_struct {
	char name[MET_NAME_BUF_SIZE];
	unsigned int mcookie;
	struct list_head hash_list;
};
static DEFINE_MUTEX(mcookie_mutex);
static struct kmem_cache *met_cookie_cache __read_mostly;
static struct list_head *met_cookie_hashtable __read_mostly;
static size_t met_hash_size __read_mostly;

static DEFINE_SPINLOCK(task_mortuary);
static DEFINE_MUTEX(die_mutex);
static LIST_HEAD(dying_tasks);
static LIST_HEAD(dead_tasks);

static inline unsigned long met_cookie_value(struct met_cookie_struct *mcs)
{
	return (unsigned long)mcs->mcookie;
}

static size_t met_cookie_hash(unsigned long mcookie)
{
	return (mcookie >> L1_CACHE_SHIFT) & (met_hash_size - 1);
}

static struct met_cookie_struct *find_mcookie(unsigned int mcookie)
{
	struct met_cookie_struct *found = NULL;
	struct met_cookie_struct *mcs;
	struct list_head *pos;
	struct list_head *list;

	list = met_cookie_hashtable + met_cookie_hash(mcookie);

	dbg_mcookie_tprintk("hashtable is 0x%x\n",
			(unsigned int)met_cookie_hashtable);
	dbg_mcookie_tprintk("mcookie is 0x%x, off is %d\n",
			mcookie,
			met_cookie_hash(mcookie));

	list_for_each(pos, list) {
		mcs = list_entry(pos, struct met_cookie_struct, hash_list);
		if (met_cookie_value(mcs) == mcookie) {
			found = mcs;
			break;
		}
	}

	return found;
}

static void hash_mcookie(struct met_cookie_struct *mcs)
{
	struct list_head *list = met_cookie_hashtable +
				met_cookie_hash(met_cookie_value(mcs));
	list_add(&mcs->hash_list, list);
}

static struct met_cookie_struct *alloc_mcookie(unsigned int mcookie,
						const char *name)
{
	int length;
	struct met_cookie_struct *mcs;

	if (name == NULL)
		return NULL;

	dbg_mcookie_tprintk("mcookie is 0x%x, name is %s\n",
			mcookie,
			name);

	mcs = kmem_cache_alloc(met_cookie_cache, GFP_KERNEL);
	if (mcs == NULL)
		return NULL;

	length = strlen(name);
	mcs->mcookie = mcookie;
	memset(mcs->name, 0, MET_NAME_BUF_SIZE);
	if (length < MET_NAME_BUF_SIZE)
		memcpy(mcs->name, name, length);
	else
		memcpy(mcs->name, name, MET_NAME_BUF_SIZE);

	hash_mcookie(mcs);

	return mcs;
}

static void free_mcookie(struct met_cookie_struct *mcs)
{
	kmem_cache_free(met_cookie_cache, mcs);
}

static unsigned int mcookie_gen(const char *data)
{
	register unsigned long crc;
	const unsigned char *block = data;
	int i, length;

	if (data == NULL)
		return 0;

	length = strlen(data);
	crc = 0xFFFFFFFF;
	for (i = 0; i < length; i++) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ met_crc32_table[(crc ^ *block++) & 0xFF];
	}

	return (crc ^ 0xFFFFFFFF);
}

unsigned int get_mcookie(const char *name)
{
	unsigned int err;
	unsigned int key;
	struct met_cookie_struct *mcs;

	dbg_mcookie_tprintk("name is %s\n", name);

	err = 0;
	if (name == NULL)
		return err;

	key = mcookie_gen(name);

	dbg_mcookie_tprintk("key is 0x%x\n", key);

	if (key == 0)
		return err;

	mutex_lock(&mcookie_mutex);
	mcs = find_mcookie(key);
	if (mcs == NULL) {
		mcs = alloc_mcookie(key, name);
		if (mcs == NULL) {
			mutex_unlock(&mcookie_mutex);
			return err;
		}
	}
	mutex_unlock(&mcookie_mutex);

	return (unsigned int)mcs->mcookie;
}

int dump_mcookie(char *buf, unsigned int size)
{
	struct met_cookie_struct *mcs;
	struct list_head *pos;
	struct list_head *pos2;
	struct list_head *list;
	int i;
	unsigned int total_size;

	if (buf == NULL)
		return -1;

	total_size = 0;
	memset(buf, 0, size);
	mutex_lock(&mcookie_mutex);
	for (i=0; i<met_hash_size; ++i) {
		list = met_cookie_hashtable + i;

		dbg_mcookie_printk("hashtable is 0x%x\n",
				(unsigned int)met_cookie_hashtable);
		dbg_mcookie_printk("off is %d\n",
				i);
		dbg_mcookie_printk("list is 0x%x\n",
				(unsigned int)list);

		list_for_each_safe(pos, pos2, list) {
			mcs = list_entry(pos,
					struct met_cookie_struct,
					hash_list);
			/* total_size = total_size +
					"name" +
					space +
					"0xffff ffff" +
					newline */
			if (total_size +
				(strlen(mcs->name) + 1 + 2 + 8 + 1) > size)
				break;
			total_size += sprintf(buf + total_size, "%s 0x%x\n",
				mcs->name,
				mcs->mcookie);
			dbg_mcookie_printk("total_size is %d, "
				"name is %s, "
				"mcookie is 0x%x\n",
					total_size,
					mcs->name,
					(unsigned int)mcs->mcookie);
		}
	}
	mutex_unlock(&mcookie_mutex);

	return total_size;
}

int init_met_cookie(void)
{
	int i, j;
	unsigned int crc, poly;
	struct list_head *d;

	// build CRC32 table
	poly = 0xdeadbeef;
	met_crc32_table = (unsigned int *)kmalloc(256 * sizeof(unsigned int),
							GFP_KERNEL);
	if (met_crc32_table == NULL)
		return -ENOMEM;
	for (i = 0; i < 256; i++) {
		crc = i;
		for (j = 8; j > 0; j--) {
			if (crc & 1) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc >>= 1;
			}
		}
		met_crc32_table[i] = crc;
	}

	mutex_lock(&mcookie_mutex);
	met_cookie_cache = kmem_cache_create("met_cookie_cache",
		sizeof(struct met_cookie_struct),
		0, 0, NULL);
	if (met_cookie_cache == NULL)
		return -ENOMEM;

	met_hash_size = MET_HASH_SIZE;
	met_cookie_hashtable = kmalloc(met_hash_size *
				sizeof(struct list_head),
					GFP_KERNEL);
	if (met_cookie_hashtable == NULL)
		return -ENOMEM;
	/* And initialize the newly allocated array */
	d = met_cookie_hashtable;
	i = met_hash_size;
	do {
		INIT_LIST_HEAD(d);
		d++;
		i--;
	} while (i);
	mutex_unlock(&mcookie_mutex);

	return 0;
}

int uninit_met_cookie(void)
{
	struct list_head *list;
	struct list_head *pos;
	struct list_head *pos2;
	struct met_cookie_struct *mcs;
	size_t i;

	kfree(met_crc32_table);
	met_crc32_table = NULL;

	mutex_lock(&mcookie_mutex);
	for (i = 0; i < met_hash_size; ++i) {
		list = met_cookie_hashtable + i;
		list_for_each_safe(pos, pos2, list) {
			mcs = list_entry(pos, struct met_cookie_struct, hash_list);
			list_del(&mcs->hash_list);
			free_mcookie(mcs);
		}
	}
	kfree(met_cookie_hashtable);
	kmem_cache_destroy(met_cookie_cache);
	mutex_unlock(&mcookie_mutex);

	return 0;
}

/* Move tasks along towards death. Any tasks on dead_tasks
 * will definitely have no remaining references in any
 * CPU buffers at this point, because we use two lists,
 * and to have reached the list, it must have gone through
 * one full sync already.
 */
static void process_task_mortuary(void)
{
	unsigned long flags;
	LIST_HEAD(local_dead_tasks);
	struct task_struct *task;
	struct task_struct *ttask;

	spin_lock_irqsave(&task_mortuary, flags);

	list_splice_init(&dead_tasks, &local_dead_tasks);
	list_splice_init(&dying_tasks, &dead_tasks);

	spin_unlock_irqrestore(&task_mortuary, flags);

	list_for_each_entry_safe(task, ttask, &local_dead_tasks, tasks) {
		list_del(&task->tasks);
		free_task(task);
	}
}

void mark_done(int cpu)
{
	int i;

	mutex_lock(&die_mutex);
	cpumask_set_cpu(cpu, marked_cpus);

	for_each_online_cpu(i) {
		if (!cpumask_test_cpu(i, marked_cpus)) {
			mutex_unlock(&die_mutex);
			return;
		}
	}
	/* All CPUs have been processed at least once,
	 * we can process the mortuary once
	 */
	process_task_mortuary();

	cpumask_clear(marked_cpus);
	mutex_unlock(&die_mutex);
}

static void free_all_tasks(void)
{
	/* make sure we don't leak task structs */
	process_task_mortuary();
	process_task_mortuary();
}

/* Take ownership of the task struct and place it on the
 * list for processing. Only after two full buffer syncs
 * does the task eventually get freed, because by then
 * we are sure we will not reference it again.
 * Can be invoked from softirq via RCU callback due to
 * call_rcu() of the task struct, hence the _irqsave.
 */
static int task_free_notify(struct notifier_block *self, unsigned long val, void *data)
{
	unsigned long flags;
	struct task_struct *task = data;
	spin_lock_irqsave(&task_mortuary, flags);
	list_add(&task->tasks, &dying_tasks);
	spin_unlock_irqrestore(&task_mortuary, flags);
	return NOTIFY_OK;
}

static int task_exit_notify(struct notifier_block *self, unsigned long val, void *data)
{
//	struct task_struct *task = data;
	sync_all_samples();
	return 0;
}

static int munmap_notify(struct notifier_block *self, unsigned long val, void *data)
{
//	struct task_struct *task = data;
	sync_all_samples();
	return 0;
}

static int module_load_notify(struct notifier_block *self, unsigned long val, void *data)
{
	return 0;
}

static struct notifier_block task_free_nb = {
	.notifier_call	= task_free_notify,
};

static struct notifier_block task_exit_nb = {
	.notifier_call	= task_exit_notify,
};

static struct notifier_block munmap_nb = {
	.notifier_call	= munmap_notify,
};

static struct notifier_block module_load_nb = {
	.notifier_call = module_load_notify,
};

struct mm_struct *take_tasks_mm(struct task_struct *task)
{
	struct mm_struct *mm = get_task_mm(task);
	if (mm)
		down_read(&mm->mmap_sem);
	return mm;
}

void release_mm(struct mm_struct *mm)
{
	if (!mm)
		return;
	up_read(&mm->mmap_sem);
	mmput(mm);
}

static inline unsigned long fast_get_dcookie(struct path *path)
{
	unsigned long cookie = 0;

	if (path->dentry->d_flags & DCACHE_COOKIE)
		return (unsigned long)path->dentry;
	get_dcookie(path, &cookie);
	return cookie;
}

unsigned long lookup_dcookie(struct mm_struct *mm, unsigned long addr, off_t *offset)
{
	unsigned long cookie = NO_COOKIE;
	struct vm_area_struct *vma;

	for (vma = find_vma(mm, addr); vma; vma = vma->vm_next) {

		if (addr < vma->vm_start || addr >= vma->vm_end)
			continue;

		if (vma->vm_file) {
			cookie = fast_get_dcookie(&vma->vm_file->f_path);
			*offset = (vma->vm_pgoff << PAGE_SHIFT) + addr -
			           vma->vm_start;
		} else {
			/* must be an anonymous map */
			*offset = addr;
		}

		break;
	}

	if (!vma)
		cookie = INVALID_COOKIE;

	return cookie;
}

int src_map_start(void)
{
	dcookie = dcookie_register();

	if (!zalloc_cpumask_var(&marked_cpus, GFP_KERNEL))
		return -ENOMEM;

	task_handoff_register(&task_free_nb);
	profile_event_register(PROFILE_TASK_EXIT, &task_exit_nb);
	profile_event_register(PROFILE_MUNMAP, &munmap_nb);
	register_module_notifier(&module_load_nb);

	return 0;
}

void src_map_stop(void)
{
	unregister_module_notifier(&module_load_nb);
	profile_event_unregister(PROFILE_MUNMAP, &munmap_nb);
	profile_event_unregister(PROFILE_TASK_EXIT, &task_exit_nb);
	task_handoff_unregister(&task_free_nb);

	free_all_tasks();
	free_cpumask_var(marked_cpus);
}

void src_map_stop_dcookie(void)
{
	if (dcookie != NULL) {
		dcookie_unregister(dcookie);
		dcookie	= NULL;
	}
}
