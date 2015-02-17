#ifndef __CCCI_DFO_H__
#define __CCCI_DFO_H__

#include "ccci_core.h"
#include "ccci_debug.h"

#define MD1_EN (1<<0)
#define MD2_EN (1<<1)
#define MD5_EN (1<<4)

#define MD1_SETTING_ACTIVE	(1<<0)
#define MD2_SETTING_ACTIVE	(1<<1)

#define MD5_SETTING_ACTIVE	(1<<4)

#define MD_2G_FLAG (1<<0)
#define MD_WG_FLAG ((1<<1)|(1<<0))
#define MD_TG_FLAG ((1<<2)|(1<<0))
#define MD_LWG_FLAG ((1<<3)|(1<<1)|(1<<0))
#define MD_LTG_FLAG ((1<<3)|(1<<2)|(1<<0))

#define MD1_MEM_SIZE (96*1024*1024) // MD ROM+RAM size
#define MD1_SMEM_SIZE (8*1024) // share memory size
#define MD2_MEM_SIZE (22*1024*1024)
#define MD2_SMEM_SIZE (2*1024*1024)

typedef enum {
	md_type_invalid = 0,
	modem_2g = 1,
	modem_3g,
	modem_wg,
	modem_tg,
    modem_lwg,
    modem_ltg,
    MAX_IMG_NUM = modem_ltg // this enum starts from 1
} MD_LOAD_TYPE;

struct dfo_item
{
	char name[32];
	int  value;
};

// image name/path
#define MOEDM_IMAGE_NAME "modem.img"
#define DSP_IMAGE_NAME "DSP_ROM"
#define CONFIG_MODEM_FIRMWARE_PATH "/etc/firmware/"
#define IMG_ERR_STR_LEN 64

// image header constants
#define MD_HEADER_MAGIC_NO "CHECK_HEADER"
#define MD_HEADER_VER_NO    (2)
#define VER_2G_STR  "2G"
#define VER_3G_STR  "3G"
#define VER_WG_STR   "WG"
#define VER_TG_STR   "TG"
#define VER_LWG_STR   "LWG"
#define VER_LTG_STR   "LTG"
#define VER_INVALID_STR  "INVALID"
#define DEBUG_STR   "Debug"
#define RELEASE_STR  "Release"
#define INVALID_STR  "INVALID"
#define AP_PLATFORM_LEN 16

struct md_check_header {
	u8 check_header[12];	    /* magic number is "CHECK_HEADER"*/
	u32 header_verno;	        /* header structure version number */
	u32 product_ver;	        /* 0x0:invalid; 0x1:debug version; 0x2:release version */
	u32 image_type;	            /* 0x0:invalid; 0x1:2G modem; 0x2: 3G modem */
	u8 platform[16];	        /* MT6573_S01 or MT6573_S02 */
	u8 build_time[64];	        /* build time string */
	u8 build_ver[64];	        /* project version, ex:11A_MD.W11.28 */
	u8 bind_sys_id;	            /* bind to md sys id, MD SYS1: 1, MD SYS2: 2 */
	u8 ext_attr;                /* no shrink: 0, shrink: 1*/
	u8 reserved[2];             /* for reserved */
	u32 mem_size;       		/* md ROM/RAM image size requested by md */
	u32 md_img_size;            /* md image size, exclude head size*/
	u32 reserved_info;          /* for reserved */
	u32 size;	                /* the size of this structure */
} __attribute__ ((packed));

int ccci_load_firmware(struct ccci_modem *md, MD_IMG_TYPE img_type, char img_err_str[IMG_ERR_STR_LEN]);
char *ccci_get_md_info_str(struct ccci_modem *md);
int ccci_init_security(struct ccci_modem *md);
void ccci_reload_md_type(struct ccci_modem *md, int type);

#endif //__CCCI_DFO_H__