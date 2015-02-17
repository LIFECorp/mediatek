#ifndef _MTK_DEVICE_APC_HW_H
#define _MTK_DEVICE_APC_HW_H

#include "mt_reg_base.h"


/*
 * Define hardware registers.
 */
/* DEVAPC_AO Registers*/
#define DEVAPC_D0_APC_0             (DEVICE_APC_AO_BASE + 0x000)
#define DEVAPC_D0_APC_1             (DEVICE_APC_AO_BASE + 0x004)
#define DEVAPC_D0_APC_2             (DEVICE_APC_AO_BASE + 0x008)
#define DEVAPC_D0_APC_3             (DEVICE_APC_AO_BASE + 0x00C)

#define DEVAPC_D1_APC_0             (DEVICE_APC_AO_BASE + 0x100)
#define DEVAPC_D1_APC_1             (DEVICE_APC_AO_BASE + 0x104)
#define DEVAPC_D1_APC_2             (DEVICE_APC_AO_BASE + 0x108)
#define DEVAPC_D1_APC_3             (DEVICE_APC_AO_BASE + 0x10C)

#define DEVAPC_D2_APC_0             (DEVICE_APC_AO_BASE + 0x200)
#define DEVAPC_D2_APC_1             (DEVICE_APC_AO_BASE + 0x204)
#define DEVAPC_D2_APC_2             (DEVICE_APC_AO_BASE + 0x208)
#define DEVAPC_D2_APC_3             (DEVICE_APC_AO_BASE + 0x20C)

#define DEVAPC_APC_CON              (DEVICE_APC_AO_BASE + 0xF00)
#define DEVAPC_APC_LOCK0            (DEVICE_APC_AO_BASE + 0xF04)
#define DEVAPC_APC_LOCK1            (DEVICE_APC_AO_BASE + 0xF08)

/* DEVAPC Registers*/
#define DEVAPC_D0_VIO_MASK_0        (DEVICE_APC_BASE + 0x000)
#define DEVAPC_D0_VIO_MASK_1        (DEVICE_APC_BASE + 0x004)
#define DEVAPC_D0_VIO_STA_0         (DEVICE_APC_BASE + 0x400)
#define DEVAPC_D0_VIO_STA_1         (DEVICE_APC_BASE + 0x404)
#define DEVAPC_D0_VIO_STA_3         (DEVICE_APC_BASE + 0x40C)
#define DEVAPC_VIO_DBG0             (DEVICE_APC_BASE + 0x900)
#define DEVAPC_VIO_DBG1             (DEVICE_APC_BASE + 0x904)
#define DEVAPC_PD_APC_CON           (DEVICE_APC_BASE + 0xF00)


/*
 * Define constants.
 */
#define DEVAPC_DOMAIN_NUMBER    3
#define DEVAPC_DEVICE_NUMBER    64

#define DEVAPC_DOMAIN_AP        0
#define DEVAPC_DOMAIN_MD        1
#define DEVAPC_DOMAIN_CONN      2


/*
 * Define enums.
 */
/* Domain index */
typedef enum
{
    E_DOM_AP=0,
    E_DOM_MD,
    E_DOM_CONN,
    E_MAX_DOM
} DEVAPC_DOM;

/* Access permission attribute */
typedef enum
{
    E_ATTR_L0=0,
    E_ATTR_L1,
    E_ATTR_L2,
    E_ATTR_L3,
    E_MAX_ATTR
} DEVAPC_ATTR;


/*
 * Declare functions.
 */
extern void mt_devapc_set_permission(unsigned int module, DEVAPC_DOM domain_num, DEVAPC_ATTR permission_control);

#endif

