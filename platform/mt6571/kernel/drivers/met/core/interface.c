#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include "interface.h"
#include "sampler.h"
#include "util.h"

#include "met_drv.h"

#include "version.h"

static volatile int run = -1;
static volatile int sample_rate = 1000;	/* Default: 1000 Hz */
int hrtimer_expire;		/* in ns */
int timer_expire;		/* in jiffies */
unsigned int ctrl_flags = 0;

static void calc_timer_value(int rate)
{
	sample_rate = rate;

	if (rate == 0) {
		hrtimer_expire = 0;
		timer_expire = 0;
		return;
	}

	hrtimer_expire = 1000000000 / rate;

	/* Case 1: hrtimer < 1 OS tick, timer_expire = 1 OS tick */
	if (rate > HZ)
		timer_expire = 1;
	/* Case 2: hrtimer > 1 OS tick, timer_expire is hrtimer + 1 OS tick */
	else
		timer_expire = (HZ / rate) + 1;

	/* xprintk("JBK HZ=%d, hrtimer_expire=%d ns, timer_expire=%d ticks\n", */
	/* HZ, hrtimer_expire, timer_expire); */
}

int met_parse_num(const char *str, unsigned int *value, int len)
{
	unsigned int i;

	if (len <= 0)
		return -1;

	if ((len > 2) && ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))) {
		for (i = 2; i < len; i++) {
			if (!(((str[i] >= '0') && (str[i] <= '9'))
			      || ((str[i] >= 'a') && (str[i] <= 'f'))
			      || ((str[i] >= 'A') && (str[i] <= 'F')))) {
				return -1;
			}
		}
		if (sscanf(str, "%x", value) == 0)
			return -1;
	} else {
		for (i = 0; i < len; i++) {
			if (!((str[i] >= '0') && (str[i] <= '9')))
				return -1;
		}
		if (sscanf(str, "%d", value) == 0)
			return -1;
	}

	return 0;
}

#ifdef CONFIG_CPU_FREQ
#include "power.h"
volatile int do_dvfs = 0;
#endif

LIST_HEAD(met_list);
static struct kobject *kobj_pmu;
static struct kobject *kobj_bus;

static ssize_t ver_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(ver, 0444, ver_show, NULL);

static ssize_t cpu0_id_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(cpu0_id, 0444, cpu0_id_show, NULL);

static ssize_t plf_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(plf, 0444, plf_show, NULL);

static ssize_t core_topology_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(core_topology, 0444, core_topology_show, NULL);

static ssize_t devices_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(devices, 0444, devices_show, NULL);

static ssize_t ctrl_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t ctrl_store(struct device *dev, struct device_attribute *attr, const char *buf,
			  size_t count);
static DEVICE_ATTR(ctrl, 0644, ctrl_show, ctrl_store);

static ssize_t spr_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t spr_store(struct device *dev, struct device_attribute *attr, const char *buf,
			 size_t count);
static DEVICE_ATTR(sample_rate, 0644, spr_show, spr_store);

static ssize_t run_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t run_store(struct device *dev, struct device_attribute *attr, const char *buf,
			 size_t count);
static DEVICE_ATTR(run, 0644, run_show, run_store);

#ifdef CONFIG_CPU_FREQ
static ssize_t dvfs_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t dvfs_store(struct device *dev, struct device_attribute *attr, const char *buf,
			  size_t count);
static DEVICE_ATTR(dvfs, 0644, dvfs_show, dvfs_store);
#endif


static const struct file_operations met_file_ops = {
	.owner = THIS_MODULE
};

struct miscdevice met_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "met",
	.mode = 0666,
	.fops = &met_file_ops
};

static int met_run(void)
{
	sampler_start();
	return 0;
}

static void met_stop(void)
{
	sampler_stop();
}

static ssize_t ver_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	mutex_lock(&dev->mutex);
	i = snprintf(buf, PAGE_SIZE, "%s\n", MET_BACKEND_VERSION);
	mutex_unlock(&dev->mutex);
	return i;
}

static void read_cpu_id(void *data)
{
	int *value = (int *)data;
	/* Read Main ID Register */
	asm volatile ("mrc p15, 0, %0, c0, c0, 0":"=r" (*value));
	*value = (*value & 0xffff) >> 4;	/* primary part number */
}

static ssize_t cpu0_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int value;

	if (smp_call_function_single(0, read_cpu_id, &value, 1) == 0)
		return snprintf(buf, PAGE_SIZE, "%x\n", value);
	else
		return -EINVAL;
}

static ssize_t devices_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int len, total_len = 0;
	struct metdevice *c = NULL;
	mutex_lock(&dev->mutex);
	list_for_each_entry(c, &met_list, list) {
		len = 0;
		if (c->type == MET_TYPE_PMU)
			len = snprintf(buf, PAGE_SIZE - total_len, "pmu/%s:0\n", c->name);
		else if (c->type == MET_TYPE_BUS)
			len = snprintf(buf, PAGE_SIZE - total_len, "bus/%s:0\n", c->name);
		else if (c->type == MET_TYPE_MISC)
			len = snprintf(buf, PAGE_SIZE - total_len, "misc/%s:0\n", c->name);

		if (c->process_argument)
			buf[len - 2]++;

		buf += len;
		total_len += len;
	}

	mutex_unlock(&dev->mutex);
	return total_len;
}

static char met_platform[16] = "none";
static ssize_t plf_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	mutex_lock(&dev->mutex);
	i = snprintf(buf, PAGE_SIZE, "%s\n", met_platform);
	mutex_unlock(&dev->mutex);
	return i;
}

static char met_topology[64] = "none";
static ssize_t core_topology_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	mutex_lock(&dev->mutex);
	i = snprintf(buf, PAGE_SIZE, "%s\n", met_topology);
	mutex_unlock(&dev->mutex);
	return i;
}

static ssize_t ctrl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ctrl_flags);
}

static ssize_t ctrl_store(struct device *dev, struct device_attribute *attr, const char *buf,
			  size_t count)
{
	unsigned int value;
	if (met_parse_num(buf, &value, count) < 0)
		return -EINVAL;

	ctrl_flags = value;
	return count;
}

static ssize_t spr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	mutex_lock(&dev->mutex);
	i = snprintf(buf, PAGE_SIZE, "%d\n", sample_rate);
	mutex_unlock(&dev->mutex);
	return i;
}

static ssize_t spr_store(struct device *dev, struct device_attribute *attr, const char *buf,
			 size_t count)
{
	int value;
	struct metdevice *c = NULL;

	mutex_lock(&dev->mutex);

	if ((run == 1) || (count == 0) || (buf == NULL)) {
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}
	if (sscanf(buf, "%d", &value) != 1) {
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}

	if ((value < 0) || (value > 10000)) {
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}

	calc_timer_value(value);

	list_for_each_entry(c, &met_list, list) {
		if (c->polling_interval > 0)
			c->polling_count_reload = ((c->polling_interval * sample_rate) - 1) / 1000;
		else
			c->polling_count_reload = 0;
	}

	mutex_unlock(&dev->mutex);

	return count;
}

static ssize_t run_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	mutex_lock(&dev->mutex);
	i = snprintf(buf, PAGE_SIZE, "%d\n", run);
	mutex_unlock(&dev->mutex);
	return i;
}

static ssize_t run_store(struct device *dev, struct device_attribute *attr, const char *buf,
			 size_t count)
{
	int value;

	mutex_lock(&dev->mutex);

	if ((count == 0) || (buf == NULL)) {
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}
	if (sscanf(buf, "%d", &value) != 1) {
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}

	switch (value) {
	case 1:
		if (run != 1) {
			run = 1;
			met_run();
		}
		break;
	case 0:
		if (run != 0) {
			if (run == 1) {
				met_stop();
#ifdef MET_USER_EVENT_SUPPORT
				met_save_dump_buffer("/data/trace.dump");
#endif
				run = 0;
			} else
				/* run == -1 */
				run = 0;
		}
		break;
	case -1:
		if (run != -1) {
			if (run == 1)
				met_stop();

			run = -1;
		}
		break;
	default:
		mutex_unlock(&dev->mutex);
		return -EINVAL;
	}

	mutex_unlock(&dev->mutex);

	return count;
}

#ifdef CONFIG_CPU_FREQ
static ssize_t dvfs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	i = snprintf(buf, PAGE_SIZE, "%d\n", do_dvfs);
	return i;
}

static ssize_t dvfs_store(struct device *dev, struct device_attribute *attr, const char *buf,
			  size_t count)
{
	int value;

	if ((count == 0) || (buf == NULL))
		return -EINVAL;
	if (sscanf(buf, "%u", &value) != 1)
		return -EINVAL;

	switch (value) {
	case 1:
		do_dvfs = 1;
		force_power_log(POWER_LOG_ALL);
		break;
	case 0:
		do_dvfs = 0;
		break;
	default:
		return -EINVAL;
	}

	return count;
}
#endif

static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	return snprintf(buf, PAGE_SIZE, "%d\n", c->mode);
}

static ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
			  size_t n)
{
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	if (sscanf(buf, "%d", &(c->mode)) != 1)
		return -EINVAL;

	return n;
}

static struct kobj_attribute mode_attr = __ATTR(mode, 0644, mode_show, mode_store);

static ssize_t polling_interval_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int interval = 1;
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	if (c->polling_interval)
		interval = c->polling_interval;

	return snprintf(buf, PAGE_SIZE, "%d\n", interval);
}

static ssize_t polling_interval_store(struct kobject *kobj, struct kobj_attribute *attr,
				      const char *buf, size_t n)
{
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	if (sscanf(buf, "%d", &(c->polling_interval)) != 1)
		return -EINVAL;

	if (c->polling_interval > 0)
		c->polling_count_reload = ((c->polling_interval * sample_rate) - 1) / 1000;
	else
		c->polling_count_reload = 0;

	return n;
}

static struct kobj_attribute polling_interval_attr =
__ATTR(polling_ms, 0644, polling_interval_show, polling_interval_store);

static ssize_t header_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	if ((c->mode) && (c->print_header))
		return c->print_header(buf, PAGE_SIZE);

	return 0;
}

static struct kobj_attribute header_attr = __ATTR(header, 0444, header_show, NULL);

static ssize_t help_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	if (c->print_help)
		return c->print_help(buf, PAGE_SIZE);

	return 0;
}

static struct kobj_attribute help_attr = __ATTR(help, 0444, help_show, NULL);

static ssize_t argu_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
			  size_t n)
{
	int ret = 0;
	struct metdevice *c = NULL;
	list_for_each_entry(c, &met_list, list) {
		if (c->kobj == kobj)
			break;
	}
	if (c == NULL)
		return -ENOENT;

	if (c->process_argument)
		ret = c->process_argument(buf, (int)n);

	if (ret != 0)
		return -EINVAL;

	return n;
}

static struct kobj_attribute argu_attr = __ATTR(argu, 0244, NULL, argu_store);

int met_register(struct metdevice *met)
{
	int ret, cpu;
	struct metdevice *c;

	list_for_each_entry(c, &met_list, list) {
		if (!strcmp(c->name, met->name))
			return -EEXIST;
	}

	INIT_LIST_HEAD(&met->list);

	/* Allocate timer count for per CPU */
	met->polling_count = alloc_percpu(int);
	if (met->polling_count == NULL)
		return -EINVAL;

	for_each_possible_cpu(cpu)
		*(per_cpu_ptr(met->polling_count, cpu)) = 0;

	if (met->polling_interval > 0) {
		ret = ((met->polling_interval * sample_rate) - 1) / 1000;
		met->polling_count_reload = ret;
	} else
		met->polling_count_reload = 0;

	met->kobj = NULL;

	if (met->type == MET_TYPE_BUS)
		met->kobj = kobject_create_and_add(met->name, kobj_bus);
	else if (met->type == MET_TYPE_PMU)
		met->kobj = kobject_create_and_add(met->name, kobj_pmu);
	else {
		ret = -EINVAL;
		goto err_out;
	}

	if (met->kobj == NULL) {
		ret = -EINVAL;
		goto err_out;
	}

	if (met->create_subfs) {
		ret = met->create_subfs(met->kobj);
		if (ret)
			goto err_out;
	}

	ret = sysfs_create_file(met->kobj, &mode_attr.attr);
	if (ret)
		goto err_out;

	ret = sysfs_create_file(met->kobj, &polling_interval_attr.attr);
	if (ret)
		goto err_out;

	if (met->print_header) {
		ret = sysfs_create_file(met->kobj, &header_attr.attr);
		if (ret)
			goto err_out;
	}

	if (met->print_help) {
		ret = sysfs_create_file(met->kobj, &help_attr.attr);
		if (ret)
			goto err_out;
	}

	if (met->process_argument) {
		ret = sysfs_create_file(met->kobj, &argu_attr.attr);
		if (ret)
			goto err_out;
	}

	list_add(&met->list, &met_list);
	return 0;

 err_out:

	if (met->polling_count)
		free_percpu(met->polling_count);

	if (met->kobj)
		kobject_del(met->kobj);

	return ret;
}

int met_deregister(struct metdevice *met)
{
	struct metdevice *c = NULL;

	list_for_each_entry(c, &met_list, list) {
		if (c == met)
			break;
	}
	if (c != met)
		return -ENOENT;

	if (met->print_header)
		sysfs_remove_file(met->kobj, &header_attr.attr);

	if (met->print_help)
		sysfs_remove_file(met->kobj, &help_attr.attr);

	if (met->process_argument)
		sysfs_remove_file(met->kobj, &argu_attr.attr);

	sysfs_remove_file(met->kobj, &polling_interval_attr.attr);
	sysfs_remove_file(met->kobj, &mode_attr.attr);

	if (met->delete_subfs)
		met->delete_subfs();

	kobject_del(met->kobj);

	if (met->polling_count)
		free_percpu(met->polling_count);

	list_del(&met->list);
	return 0;
}

int met_set_platform(const char *plf_name, int flag)
{
	int ret;

	if (flag) {
		ret = device_create_file(met_device.this_device, &dev_attr_plf);
		if (ret != 0) {
			pr_err("can not create device file: plf\n");
			return ret;
		}
		strncpy(met_platform, plf_name, 16);
	} else
		device_remove_file(met_device.this_device, &dev_attr_plf);

	return 0;
}

int met_set_topology(const char *topology_name, int flag)
{
	int ret;

	if (flag) {
		ret = device_create_file(met_device.this_device, &dev_attr_core_topology);
		if (ret != 0) {
			pr_err("can not create device file: topology\n");
			return ret;
		}
		strncpy(met_topology, topology_name, strlen(topology_name));
	} else {
		device_remove_file(met_device.this_device, &dev_attr_core_topology);
	}
	return 0;
}
EXPORT_SYMBOL(met_register);
EXPORT_SYMBOL(met_deregister);
EXPORT_SYMBOL(met_set_platform);
EXPORT_SYMBOL(met_set_topology);

#include "buffer.h"
#include "met_struct.h"

void force_sample(void *unused)
{
	int cpu;
	unsigned long long stamp;
	struct metdevice *c;
	struct met_cpu_struct *met_cpu_ptr;

	if ((run != 1) || (sample_rate == 0))
		return;

	/* to avoid met tag is coming after __met_hrtimer_stop and before run=-1 */
	met_cpu_ptr = &__get_cpu_var(met_cpu);
	if (0 == met_cpu_ptr->work_enabled)
		return;

	cpu = smp_processor_id();

	if (cpu == 0) {
		stamp = cpu_clock(cpu);

		list_for_each_entry(c, &met_list, list) {
			if (c->cpu_related == 0) {
				if ((c->mode != 0) && (c->tagged_polling != NULL))
					c->tagged_polling(stamp, 0);
			}
		}
	}
}

int fs_reg(void)
{
	int ret = 0;

	calc_timer_value(sample_rate);

	ret = misc_register(&met_device);
	if (ret != 0) {
		pr_err("misc register failed\n");
		return ret;
	}

	ret = device_create_file(met_device.this_device, &dev_attr_run);
	if (ret != 0) {
		pr_err("can not create device file: run\n");
		return ret;
	}
#ifdef CONFIG_CPU_FREQ
	ret = device_create_file(met_device.this_device, &dev_attr_dvfs);
	if (ret != 0) {
		pr_err("can not create device file: dvfs\n");
		return ret;
	}
#endif
	ret = device_create_file(met_device.this_device, &dev_attr_ver);
	if (ret != 0) {
		pr_err("can not create device file: ver\n");
		return ret;
	}

	ret = device_create_file(met_device.this_device, &dev_attr_cpu0_id);
	if (ret != 0) {
		pr_err("can not create device file: cpu0_id\n");
		return ret;
	}

	ret = device_create_file(met_device.this_device, &dev_attr_devices);
	if (ret != 0) {
		pr_err("can not create device file: devices\n");
		return ret;
	}

	ret = device_create_file(met_device.this_device, &dev_attr_ctrl);
	if (ret != 0) {
		pr_err("can not create device file: ctrl\n");
		return ret;
	}

	ret = device_create_file(met_device.this_device, &dev_attr_sample_rate);
	if (ret != 0) {
		pr_err("can not create device file: sample_rate\n");
		return ret;
	}

	kobj_pmu = kobject_create_and_add("pmu", &met_device.this_device->kobj);
	if (kobj_pmu == NULL) {
		pr_err("can not create kobject: kobj_pmu\n");
		return ret;
	}

	kobj_bus = kobject_create_and_add("bus", &met_device.this_device->kobj);
	if (kobj_bus == NULL) {
		pr_err("can not create kobject: kobj_bus\n");
		return ret;
	}

	met_register(&met_cookie);
	met_register(&met_cpupmu);

#ifdef MET_USER_EVENT_SUPPORT
	tag_reg((struct file_operations * const) met_device.fops, &met_device.this_device->kobj);
#endif

	met_register(&met_stat);
	return ret;
}

void fs_unreg(void)
{
	if (run == 1)
		met_stop();

	run = -1;

	met_deregister(&met_stat);

#ifdef MET_USER_EVENT_SUPPORT
	tag_unreg();
#endif

	met_deregister(&met_cpupmu);
	met_deregister(&met_cookie);

	kobject_del(kobj_pmu);
	kobject_del(kobj_bus);
	device_remove_file(met_device.this_device, &dev_attr_run);
#ifdef CONFIG_CPU_FREQ
	device_remove_file(met_device.this_device, &dev_attr_dvfs);
#endif
	device_remove_file(met_device.this_device, &dev_attr_ver);
	device_remove_file(met_device.this_device, &dev_attr_cpu0_id);
	device_remove_file(met_device.this_device, &dev_attr_devices);
	device_remove_file(met_device.this_device, &dev_attr_sample_rate);
	device_remove_file(met_device.this_device, &dev_attr_ctrl);

	misc_deregister(&met_device);
}

unsigned int get_ctrl_flags(void)
{
	return ctrl_flags;
}
