#define LOG_TAG "MtkCam/PREVIEWFEATUREBuffer"
//
#include <MyUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <stdlib.h> 
#include <linux/cache.h>
//
#include "PreviewFeatureBufMgr.h"
//
#include <cutils/atomic.h>
//
/******************************************************************************
*
*******************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")

/******************************************************************************
*
*******************************************************************************/
void 
PREVIEWFEATUREBuffer::
createBuffer()
{
    FUNCTION_IN;  
    //
    mbufSize = (mbufSize + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
    mpIMemDrv = IImageBufferAllocator::getInstance();
    if ( ! mpIMemDrv ) {
        MY_LOGE("mpIMemDrv->init() error");
        return;
    }
    
    IImageBufferAllocator::ImgParam imgParam(mbufSize, 0);
    PreviewBuffer = mpIMemDrv->alloc("PreviewBuffer", imgParam);
    
    if  ( PreviewBuffer.get() == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return;
    }
    
    if ( !PreviewBuffer->lockBuf( "PanoJpg", (eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK) ) )
    {
        MY_LOGE("lock Buffer failed\n");
        return;
    }
    

    MY_LOGD( "Preview bufAddr(0x%x) size %d!",PreviewBuffer->getBufVA(0), PreviewBuffer->getBufSizeInBytes(0));    
    //
    FUNCTION_OUT;  
}


/******************************************************************************
*
*******************************************************************************/
void
PREVIEWFEATUREBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
    if( !PreviewBuffer->unlockBuf( "PanoBuffer" ) )
    {
        CAM_LOGE("unlock Buffer failed\n");
        return;
    }
	  mpIMemDrv->free(PreviewBuffer.get());   
    //
    FUNCTION_OUT;
}

