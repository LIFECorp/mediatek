#ifndef __MET_TAG_EX_H__
#define __MET_TAG_EX_H__

#ifdef BUILD_WITH_MET
void force_sample(void *unused);
#else
#include <linux/string.h>
#endif


extern struct met_api_tbl met_ext_api;

#endif	/* __MET_TAG_EX_H__ */

