#ifndef _VFB_CONFIG_H
#define _VFB_CONFIG_H

typedef struct
{
    int temporal_smooth_level;
    int sort_face_weight;

}VFB_Customize_PARA_STRUCT;


void get_VFB_CustomizeData(VFB_Customize_PARA_STRUCT *a_pDataOut);
	
#endif /* _VFB_CONFIG_H */

