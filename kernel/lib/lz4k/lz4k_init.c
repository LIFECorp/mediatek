#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>

int lz4k_compress(const unsigned char *src, size_t src_len,
			unsigned char *dst, size_t *dst_len, void *wrkmem);
int lz4k_decompress_safe(const unsigned char *src, size_t src_len,
			unsigned char *dst, size_t *dst_len);


/* Set ZRAM hooks */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 4, 69)) && defined(CONFIG_ZRAM)
extern void zram_set_hooks(void *compress_func, void *decompress_func);
#endif
static int __init lz4k_init(void)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 4, 69)) && defined(CONFIG_ZRAM)
	zram_set_hooks(&lz4k_compress, &lz4k_decompress_safe);
#endif
	return 0;
}

static void __exit lz4k_exit(void)
{
	printk(KERN_INFO "Bye LZ4K!\n");
}

module_init(lz4k_init);
module_exit(lz4k_exit);
