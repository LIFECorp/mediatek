/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#if defined(MTK_ION_SUPPORT)
#define LOG_TAG "MtkCam/GrallocImageBufferHeap"
//
#include "MyUtils.h"
#include <mtkcam/utils/GrallocImageBufferHeap.h>
#include <ui/gralloc_extra.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
//
#include <asm/cache.h>
//


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
GrallocImageBufferHeap*
GrallocImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam
)
{
    MUINT const planeCount = Format::queryPlaneCount(rImgParam.imgFormat);
#if 1
    for (MUINT i = 0; i < planeCount; i++)
    {
        CAM_LOGW_IF(
            0!=(rImgParam.bufBoundaryInBytes[i]%L1_CACHE_BYTES), 
            "BoundaryInBytes[%d]=%d is not a multiple of %d", 
            i, rImgParam.bufBoundaryInBytes[i], L1_CACHE_BYTES
        );
    }
#endif
    //
    GrallocImageBufferHeap* pHeap = NULL;
    pHeap = new GrallocImageBufferHeap(szCallerName, rImgParam, rExtraParam);
    if  ( ! pHeap )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat, rImgParam.bufSize) )
    {
        CAM_LOGE("onCreate");
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
GrallocImageBufferHeap::
GrallocImageBufferHeap(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam
)
    : BaseImageBufferHeap(szCallerName)
    //
    , mExtraParam(rExtraParam)
    //
    , mImgFormat(rImgParam.imgFormat)
    , mImgSize(rImgParam.imgSize)
    , mvHeapInfo()
    , mvBufInfo()
    //
{
    MY_LOGD("");
    ::memcpy(mBufStridesInBytesToAlloc, rImgParam.bufStridesInBytes, sizeof(mBufStridesInBytesToAlloc));
    ::memcpy(mBufBoundaryInBytesToAlloc, rImgParam.bufBoundaryInBytes, sizeof(mBufBoundaryInBytesToAlloc));
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    MBOOL ret = MFALSE;
    //
/*    if ( !checkImgFormat() )
    {
        MY_LOGE("unsupport image format(0x%x)", mImgFormat);
        return ret;
    }*/
    //
    //  Allocate memory and setup mBufHeapInfo & rBufHeapInfo.
    //  Allocated memory of each plane is not contiguous.
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    for (MUINT32 i = 0; i < getPlaneCount(); i++)
    {
        if  ( ! helpCheckBufStrides(i, mBufStridesInBytesToAlloc[i]) )
        {
            goto lbExit;
        }
        //
        {
            sp<HeapInfo> pHeapInfo = new HeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            //
            sp<MyBufInfo> pBufInfo = new MyBufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInBytes = mBufStridesInBytesToAlloc[i];
            pBufInfo->iBoundaryInBytesToAlloc = mBufBoundaryInBytesToAlloc[i];
            //
            if  ( ! doAllocGB(*pHeapInfo, *pBufInfo) )
            {
                MY_LOGE("doAllocGB");
            }
            //
            //  setup return buffer information.
            rvBufInfo[i]->stridesInBytes = pBufInfo->stridesInBytes;
            rvBufInfo[i]->sizeInBytes = helpQueryBufSizeInBytes(i, pBufInfo->stridesInBytes);
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        for (MUINT32 i = 0; i < mvBufInfo.size(); i++)
        {
            sp<HeapInfo> pHeapInfo = mvHeapInfo[i];
            sp<MyBufInfo> pBufInfo = mvBufInfo[i];
            //
            doDeallocGB(*pHeapInfo, *pBufInfo);
        }
    }
    MY_LOGD_IF(1, "- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impUninit(BufInfoVect_t const& rvBufInfo)
{
    for (MUINT32 i = 0; i < mvBufInfo.size(); i++)
    {
        sp<HeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<MyBufInfo> pBufInfo = mvBufInfo[i];
        //
        doDeallocGB(*pHeapInfo, *pBufInfo);
    }
    //
    MY_LOGD_IF(1, "-");
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doAllocGB(HeapInfo& rHeapInfo, MyBufInfo& rBufInfo)
{
    //
    status_t err = NO_ERROR;
    MINT32 idx = 0;
    MINT32 num = 0;
    MINT32 width  = rBufInfo.stridesInBytes;
    MINT32 height = mImgSize.h;
    //
    rBufInfo.spGBuffer = new GraphicBuffer(width, height, mImgFormat, mExtraParam.usage);
    //
    MINT32 stride = rBufInfo.spGBuffer->getStride();
    if ( stride != width )
    {
        MY_LOGD("update w(%d)->stride(%d)", rBufInfo.stridesInBytes, stride);
        rBufInfo.stridesInBytes = stride;
    }
    //
    err = ::gralloc_extra_getIonFd(rBufInfo.spGBuffer->handle, &idx, &num);
    if ((NO_ERROR != err) || (num <= 0))
    {
        MY_LOGE("error num(%d)", num);
        goto lbExit;
    }
    else
    {   // current num should be 1
        rHeapInfo.heapID = rBufInfo.spGBuffer->handle->data[idx];
    }
    //
    return  MTRUE;
lbExit:
    return  MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
GrallocImageBufferHeap::
doDeallocGB(HeapInfo& rHeapInfo, MyBufInfo& rBufInfo)
{
    rBufInfo.spGBuffer = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impLockBuf(
    char const* szCallerName, 
    MINT usage, 
    BufInfoVect_t const& rvBufInfo
)
{
    MBOOL ret = MFALSE;
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        char* buf = NULL;
        sp<HeapInfo> pHeapInfo   = mvHeapInfo[i];
        sp<MyBufInfo> pMyBufInfo = mvBufInfo[i];
        sp<BufInfo> pBufInfo     = rvBufInfo[i];
        //
        pMyBufInfo->spGBuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&buf));
        //
        //  SW Access.
        pBufInfo->va = ( 0 != (usage & eBUFFER_USAGE_SW_MASK) ) ? (unsigned int)buf : 0;
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( ! doMapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo) )
            {
                MY_LOGE("%s@ doMapPhyAddr at %d-th plane", szCallerName, i);
                goto lbExit;
            }
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        impUnlockBuf(szCallerName, usage, rvBufInfo);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impUnlockBuf(
    char const* szCallerName, 
    MINT usage, 
    BufInfoVect_t const& rvBufInfo
)
{
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<HeapInfo> pHeapInfo   = mvHeapInfo[i];
        sp<MyBufInfo> pMyBufInfo = mvBufInfo[i];
        sp<BufInfo> pBufInfo     = rvBufInfo[i];
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( 0 != pBufInfo->pa ) {
                doUnmapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo);
                pBufInfo->pa = 0;
            }
            else {
                MY_LOGW("%s@ skip PA=0 at %d-th plane", szCallerName, i);
            }
        }
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            if  ( 0 != pBufInfo->va ) {
                pBufInfo->va = 0;
            }
            else {
                MY_LOGW("%s@ skip VA=0 at %d-th plane", szCallerName, i);
            }
        }
        //
        pMyBufInfo->spGBuffer->unlock();
    }
    //
#if 0
    //  SW Write + Cacheable Memory => Flush Cache.
    if  ( 0!=(usage & eBUFFER_USAGE_SW_WRITE_MASK) && 0==mExtraParam.nocache )
    {
        doFlushCache();
    }
#endif
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doMapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = 0;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpMapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpMapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = param.phyAddr;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doUnmapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = rBufInfo.pa;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpUnmapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpUnmapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = 0;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doFlushCache()
{
    Vector<HelperParamFlushCache> vParam;
    vParam.insertAt(0, mvHeapInfo.size());
    HelperParamFlushCache*const aParam = vParam.editArray();
    for (MUINT i = 0; i < vParam.size(); i++)
    {
        aParam[i].virAddr = mvBufInfo[i]->va;
        aParam[i].ionFd   = mvHeapInfo[i]->heapID;
        aParam[i].size    = mvBufInfo[i]->sizeInBytes;
    }
    if  ( ! helpFlushCache(aParam, vParam.size()) )
    {
        MY_LOGE("helpFlushCache");
        return  MFALSE;
    }
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
/*MBOOL
GrallocImageBufferHeap::
checkImgFormat()
{
    switch (mImgFormat)
    {
        case eImgFmt_YV12:
        case eImgFmt_RGBA8888:
        case eImgFmt_RGBX8888:
        case eImgFmt_RGB888:
        case eImgFmt_RGB565:
        case eImgFmt_BGRA8888:
            return MTRUE;
        break;
        default:
            return MFALSE;
        break;
    }
}*/


/******************************************************************************
 *
 ******************************************************************************/
#endif  //MTK_ION_SUPPORT

