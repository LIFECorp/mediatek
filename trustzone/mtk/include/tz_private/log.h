
#ifndef __MTEE_LOG_H__
#define __MTEE_LOG_H__

#include <printf.h>

/* should match constant in ta_logctrl.h */
typedef enum {
    MTEE_LOG_LVL_INFO    = 0x00000000,
    MTEE_LOG_LVL_DEBUG   = 0x00000001,
    MTEE_LOG_LVL_PRINTF  = 0x00000002,
    MTEE_LOG_LVL_WARN    = 0x00000003,
    MTEE_LOG_LVL_BUG     = 0x00000004,
    MTEE_LOG_LVL_ASSERT  = 0x00000005,
    MTEE_LOG_LVL_DISABLE = 0x0000000f,
} MTEE_LOG_LVL;

/* mark this configure to disable all log in build time */
#define MTEE_ENABLE_LOG_IN_BUILD_TIME

/*
    MTEE_LOG_BUILD_LEVEL is the build time log level
    note:
        USER_BUILD is defined in alps\mediatek\trustzone\mtk\Setting.mk
*/
#ifdef USER_BUILD
/* user build */
#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_PRINTF
#else
/* eng debug build */
#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_INFO
#endif

int _MTEE_LOG(int level, const char *fmt, ...) __PRINTFLIKE(2, 3);

#define MTEE_LOG(lvl, args...) do { if ((lvl) >= MTEE_LOG_BUILD_LEVEL) { _MTEE_LOG(lvl, args); } } while (0)

extern void MTEE_EnableLogREE(void);

/*
    change run-time log output level 
    this is the run-time log level
*/
extern void MTEE_SetLogLvl(MTEE_LOG_LVL lvl);

#endif /* __MTEE_LOG_H__ */

