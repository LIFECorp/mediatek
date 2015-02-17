#include <linux/kernel.h>
#include <linux/module.h>


struct met_api_tbl {
	int (*met_tag_start) (unsigned int class_id, const char *name);
	int (*met_tag_end) (unsigned int class_id, const char *name);
	int (*met_tag_oneshot) (unsigned int class_id, const char *name, unsigned int value);
	int (*met_tag_dump) (unsigned int class_id, const char *name, void *data, unsigned int length);
	int (*met_tag_disable) (unsigned int class_id);
	int (*met_tag_enable) (unsigned int class_id);
	int (*met_set_dump_buffer) (int size);
	int (*met_save_dump_buffer) (const char *pathname);
	int (*met_save_log) (const char *pathname);
};

struct met_api_tbl met_ext_api;
EXPORT_SYMBOL(met_ext_api);

int met_tag_start(unsigned int class_id, const char *name)
{
	if (met_ext_api.met_tag_start) {
		return met_ext_api.met_tag_start(class_id, name);
	}
	return 0;
}

int met_tag_end(unsigned int class_id, const char *name)
{
	if (met_ext_api.met_tag_end) {
		return met_ext_api.met_tag_end(class_id, name);
	}
	return 0;
}

int met_tag_oneshot(unsigned int class_id, const char *name, unsigned int value)
{
	//trace_printk("8181888\n");
	if (met_ext_api.met_tag_oneshot) {
		return met_ext_api.met_tag_oneshot(class_id, name, value);
	}
	//else {
	//	trace_printk("7171777\n");
	//}
	return 0;
}

int met_tag_dump(unsigned int class_id, const char *name, void *data, unsigned int length)
{
	if (met_ext_api.met_tag_dump) {
		return met_ext_api.met_tag_dump(class_id, name, data, length);
	}
	return 0;
}

int met_tag_disable(unsigned int class_id)
{
	if (met_ext_api.met_tag_disable) {
		return met_ext_api.met_tag_disable(class_id);
	}
	return 0;
}

int met_tag_enable(unsigned int class_id)
{
	if (met_ext_api.met_tag_enable) {
		return met_ext_api.met_tag_enable(class_id);
	}
	return 0;
}

int met_set_dump_buffer(int size)
{
	if (met_ext_api.met_set_dump_buffer) {
		return met_ext_api.met_set_dump_buffer(size);
	}
	return 0;
}

int met_save_dump_buffer(const char *pathname)
{
	if (met_ext_api.met_save_dump_buffer) {
		return met_ext_api.met_save_dump_buffer(pathname);
	}
	return 0;
}

int met_save_log(const char *pathname)
{
	if (met_ext_api.met_save_log) {
		return met_ext_api.met_save_log(pathname);
	}
	return 0;
}

EXPORT_SYMBOL(met_tag_start);
EXPORT_SYMBOL(met_tag_end);
EXPORT_SYMBOL(met_tag_oneshot);
EXPORT_SYMBOL(met_tag_dump);
EXPORT_SYMBOL(met_tag_disable);
EXPORT_SYMBOL(met_tag_enable);
EXPORT_SYMBOL(met_set_dump_buffer);
EXPORT_SYMBOL(met_save_dump_buffer);
EXPORT_SYMBOL(met_save_log);

