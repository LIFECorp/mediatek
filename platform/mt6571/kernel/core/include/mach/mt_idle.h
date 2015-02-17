#ifndef _MT_IDLE_H
#define _MT_IDLE_H

#include <mach/mt_spm_api.h>


extern void enable_soidle_by_bit(int id);
extern void disable_soidle_by_bit(int id);
extern void enable_dpidle_by_bit(int id);
extern void disable_dpidle_by_bit(int id);

extern bool idle_state_get(u8 idx);
extern bool idle_state_en(u8 idx, bool en);

enum {
    IDLE_TYPE_DP = 0,
    IDLE_TYPE_MC = 1,
    IDLE_TYPE_SO = 2,    
    IDLE_TYPE_RG = 3,
    NR_TYPES = 4,
};



#endif
