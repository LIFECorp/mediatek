#ifndef __DDP_PQ_H__
#define __DDP_PQ_H__

#include "ddp_drv.h"

void disp_pq_config(unsigned int srcWidth,unsigned int srcHeight);
void disp_pq_init(void);
void disp_pq_reset(void);


//IOCTL , for AAL service to wait vsync and get latest histogram
void disp_set_hist_readlock(unsigned long bLock);

DISP_AAL_STATISTICS * disp_get_hist_ptr(void);


int disp_get_hist(unsigned int * pHist);

//Called by interrupt to refresh histogram
void disp_update_hist(void);

DISP_PQ_PARAM * get_PQ_config(void);
DISP_PQ_PARAM * get_PQ_Cam_config(void);
DISP_PQ_PARAM * get_PQ_Gal_config(void);
DISPLAY_PQ_T * get_PQ_index(void);

//Called by tasklet to config registers
void disp_onConfig_luma(unsigned long *luma);

void disp_pq_set_window(unsigned int sat_upper, unsigned int sat_lower, 
			   unsigned int hue_upper, unsigned int hue_lower);

#endif

