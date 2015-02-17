#ifndef __CUST_KEY_H__
#define __CUST_KEY_H__

#include<cust_kpd.h>

#define MT65XX_META_KEY		42	/* KEY_2 */
//#define MT65XX_PMIC_RST_KEY	23
#define MT_CAMERA_KEY 		10

#ifdef MT65XX_RECOVERY_KEY
#define MT65XX_BOOT_MENU_KEY       MT65XX_RECOVERY_KEY     /* KEY_VOLUMEUP */
#else
#define MT65XX_BOOT_MENU_KEY       23     /* KEY_VOLUMEUP */
#endif

#define MT65XX_MENU_SELECT_KEY     MT65XX_BOOT_MENU_KEY

#ifdef MT65XX_FACTORY_KEY
#define MT65XX_MENU_OK_KEY         MT65XX_FACTORY_KEY     /* KEY_VOLUMEDOWN */
#else
#define MT65XX_MENU_OK_KEY         32     /* KEY_VOLUMEDOWN */
#endif

#endif /* __CUST_KEY_H__ */
