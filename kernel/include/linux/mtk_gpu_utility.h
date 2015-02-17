#ifndef __MTK_GPU_UTILITY_H__
#define __MTK_GPU_UTILITY_H__

#ifdef __cplusplus
extern "C"
{
#endif

// returning false indicated no implement 

// unit: x bytes
bool mtk_get_gpu_memory_usage(unsigned int* pMemUsage);
bool mtk_get_gpu_page_cache(unsigned int* pPageCache);

// unit: 0~100 %
bool mtk_get_gpu_loading(unsigned int* pLoading);
bool mtk_get_gpu_GP_loading(unsigned int* pLoading);
bool mtk_get_gpu_PP_loading(unsigned int* pLoading);
bool mtk_get_gpu_power_loading(unsigned int* pLoading);

#ifdef __cplusplus
}
#endif

#endif
