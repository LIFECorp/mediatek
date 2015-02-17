//#include <stdio.h> 
//#include <stdarg.h>
//#include <string.h>
//#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
//#include "meta_autok_para.h"
//#include <fcntl.h>
//#include <unistd.h>
//#include <sys/ioctl.h>
//#include <sys/stat.h>
//#include <linux/rtc.h>
//#include <sys/mman.h>
//#include <utils/Log.h>
#include "autok.h"
#include <cutils/properties.h>
#include <cstring>
unsigned int *g_autok_vcore;
int VCORE_NO;// = (sizeof(g_autok_vcore)/sizeof(unsigned int));

#if 0
int test_only()
{
    char *p_data;
    int offset;
    int result = 0;
    int i, j;
    unsigned int *test_pattern;
    struct autok_predata test_predata;
    int PARAM_NO = 0;
    PARAM_NO = get_param_count();
    
    test_pattern = (unsigned int*)malloc(sizeof(unsigned int)*PARAM_NO*VCORE_NO);
    for(i=0; i<PARAM_NO*VCORE_NO; i++){
        test_pattern[i] = i%3;    
    }    
    result = pack_param(&test_predata, g_autok_vcore, VCORE_NO, test_pattern, PARAM_NO);
    if(result != 0)
        return -1;

    //offset = serilize_predata(&test_predata, &p_data);
    //printf("PDATA:%s\n", p_data);
    //result = set_node_data(STAGE2_DEVNODE, p_data, offset);
    set_stage2(2, &test_predata);
    set_debug(1);
    printf("debug:%d", get_debug());
    set_debug(0);
    printf("debug:%d", get_debug());
    //get_param(STAGE2_DEVNODE);
    get_stage2(2);
terminate: 
    //free(test_predata);
    free(p_data);
    return result;
    //close(fd_wr);
}
#endif

int main(int argc, char *argv[])
{
    int result = 0;
    int is_ss_corner = 0;
    unsigned int *vol_list;
    int vol_count = 0;
    int i;

    printf("test only\n");
    printf("test only\n");    
    
    //free(temp_str);
    g_autok_vcore = (unsigned int*)malloc(sizeof(unsigned int)*3);
    g_autok_vcore[0] = 1187500;
    g_autok_vcore[1] = 1237500;
    g_autok_vcore[2] = 1281250;
    
    is_ss_corner = get_ss_corner();
    if(is_ss_corner)
        g_autok_vcore[0] = 1150000;
    VCORE_NO = 3;
    // For the special case which provide vol_list from kernel side
    if((vol_count = get_suggest_vols(&vol_list))>0){
      free(g_autok_vcore);
      g_autok_vcore = vol_list;
      VCORE_NO = vol_count;
    }
        
    autok_flow();
    close_nvram();
	return result;
}
