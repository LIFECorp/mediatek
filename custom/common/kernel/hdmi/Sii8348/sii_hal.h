#if !defined(SII_HAL_H)
#define SII_HAL_H
#include <linux/kernel.h>

///if mhl chip CI2CA pin is pull up, the folloing should be defined. otherwise, please mask it.
#define SII_I2C_ADDR  (0x72)
#define HDMI_I2C_CHANNEL 3

#ifdef __cplusplus 
extern "C" { 
#endif

#ifndef	FALSE
#define	FALSE	false
#endif

#ifndef	TRUE
#define	TRUE	true
#endif

#define MHL_PRODUCT_NUM 8348
#define MHL_DRIVER_NAME "sii8348drv"
#define MHL_DEVICE_NAME "sii-8348"

#define CONFIG_DEBUG_DRIVER
#define RCP_INPUTDEV_SUPPORT
#define ENABLE_GEN2
#define MEDIA_DATA_TUNNEL_SUPPORT
#define ENABLE_EDID_INFO_PRINT
#define ENABLE_EDID_DEBUG_PRINT
#define ENABLE_DUMP_INFOFRAME

#ifdef __cplusplus
}
#endif  

#endif 
