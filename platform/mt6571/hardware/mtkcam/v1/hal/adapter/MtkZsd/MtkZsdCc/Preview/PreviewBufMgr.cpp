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
//
#define LOG_TAG "MtkCam/PreviewBufMgr"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
#include <adapter/inc/ImgBufProvidersManager.h>
#include <mtkcam/v1/hwscenario/HwBuffHandler.h>
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
//
#include <mtkcam/v1/IParamsManager.h>
#include <inc/PreviewCmdQueThread.h>
#include <inc/IPreviewBufMgr.h>
using namespace NSMtkZsdCcCamAdapter;

#include <mtkcam/v1/config/PriorityDefs.h>
//
/******************************************************************************
*
*******************************************************************************/
#include "mtkcam/Log.h"
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
 ******************************************************************************/


namespace android {
namespace NSMtkZsdCcCamAdapter {
/******************************************************************************
 *
 ******************************************************************************/

class PreviewBufMgr : public IPreviewBufMgr
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPreviewBufMgr Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool dequeBuffer(int ePort, ImgBufQueNode &node);
    virtual bool enqueBuffer(ImgBufQueNode const& node);
    virtual bool registerBuffer(int ePort, int w, int h, const char* format, int cnt);
    virtual void allocBuffer(int ePort, int cnt); 
    virtual void freeBuffer(int ePort);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
public:
    PreviewBufMgr(sp<ImgBufProvidersManager> &rImgBufProvidersMgr);
    virtual ~PreviewBufMgr();
    virtual void destroyInstance();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++     
 private:
    sp<ImgBufProvidersManager>    mspImgBufProvidersMgr;
    sp<HwBuffProvider>            mspHwBufPvdr;     // imgo
    sp<HwBuffProvider>            mspHwBufDispPvdr; // img2o

    struct memAllocData{
        int port;
        int w;
        int h;
        String8 format;
        int cnt;
    };
    // buffer counts to be allocated
    MUINT32                       muPortAllocated; 
    MUINT32                       muPortAllocating;

    MBOOL                         mbThreadRunning;
    pthread_t                     mAllocMemThreadHandle;
    vector<memAllocData>          mvPortToAlloc;
    list<int>                     mlBufToAlloc;
    Condition                     mCondBufAlloc;
    Condition                     mCondPass1OutReady;
    Condition                     mCondPass1DispOutReady;
    Mutex                         mAllocationLock;

    static void* _allocMemByThread(void* arg);
};

};
};


/*******************************************************************************
*
********************************************************************************/
void*
PreviewBufMgr::
_allocMemByThread(void* arg)
{
    CPTLog(Event_Hal_Adapter_MtkZsdPreview_start_alloc_memBG, CPTFlagStart);
    PreviewBufMgr* mgr = reinterpret_cast<PreviewBufMgr*>(arg);
    
    do{
        int port;
        memAllocData MemData;
        sp<HwBuffProvider> pPvdr;
        Condition *pCond;
        
        mgr->mAllocationLock.lock();
        if( mgr->mlBufToAlloc.size() == 0 )
        {
            mgr->mCondBufAlloc.wait( mgr->mAllocationLock );
        }
        port = mgr->mlBufToAlloc.front();
        mgr->mlBufToAlloc.pop_front();

        vector<memAllocData>::iterator iter;
        for(iter = mgr->mvPortToAlloc.begin() ; 
                iter != mgr->mvPortToAlloc.end(); iter++ )
        {
            if( iter->port == port )
            {
                MemData = *iter;
                mgr->mvPortToAlloc.erase(iter);
                break;
            }
        }
        mgr->mAllocationLock.unlock();

        //allocate memory
        switch (MemData.port)
        {
            case eID_Pass1Out:
                pPvdr = mgr->mspHwBufPvdr;
                pCond = &(mgr->mCondPass1OutReady);
                break;
            case eID_Pass1DispOut:
                pPvdr = mgr->mspHwBufDispPvdr;
                pCond = &(mgr->mCondPass1DispOutReady);
                break;
            default:
                MY_LOGE("not implemented yet!");
                break;
        }

        MY_LOGD("allocate port 0x%x: %dx%d, %s, x%d", 
                MemData.port, MemData.w, MemData.h, MemData.format.string(), MemData.cnt);
        
        sp<HwBuffer> buf = new HwBuffer(MemData.w, MemData.h, MemData.format.string()); 
        CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_alloc_memBG, CPTFlagPulse, "alloc done");
        //
        mgr->mAllocationLock.lock();
        //
        pPvdr->addBuf(buf);
        pCond->signal();
        //
        MemData.cnt = MemData.cnt - 1;
        if( MemData.cnt != 0 )
        {
            mgr->mvPortToAlloc.push_back( MemData );
        }
        else
        {
            MY_LOGD("port 0x%x done", MemData.port );
            mgr->muPortAllocating &= ~MemData.port;
            if( mgr->mvPortToAlloc.size() == 0 )
            {
                //all jobs are done, exit the thread
                mgr->mbThreadRunning = MFALSE;
                mgr->mAllocationLock.unlock();
                break;
            }
        }
        mgr->mAllocationLock.unlock();

    } while(1);

    CPTLog(Event_Hal_Adapter_MtkZsdPreview_start_alloc_memBG, CPTFlagEnd);

    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
bool
PreviewBufMgr::
registerBuffer(int ePort, int w, int h, const char *format, int cnt)
{
    Mutex::Autolock _l(mAllocationLock);

    if( muPortAllocated & ePort )
    {
        MY_LOGD("port(0x%x) is alread allocated(0x%x)", ePort, muPortAllocated);
        return false;
    }

    muPortAllocated  |= ePort;
    muPortAllocating |= ePort;

    memAllocData data;
    data.port = ePort;
    data.w = w;
    data.h = h;
    data.format = String8(format);
    data.cnt = cnt;

    mvPortToAlloc.push_back(data);

    if( !mbThreadRunning )
    {
        pthread_attr_t const attr = {0, NULL, 1024*1024, 4096, SCHED_RR, PRIO_RT_CAMERA_PREVIEW - 1};
        MINT32 err = ::pthread_create(&mAllocMemThreadHandle, NULL, _allocMemByThread, this);
        mbThreadRunning = MTRUE;
    }

    return true;

} 


/*******************************************************************************
*
********************************************************************************/
void 
PreviewBufMgr::
allocBuffer(int ePort, int cnt)
{
    MY_LOGD("to alloc port 0x%x, cnt %d", ePort, cnt);
    mAllocationLock.lock();
    if( !mbThreadRunning )
    {
        MY_LOGE("memory allocating thread is not running: cnt not match");
        mAllocationLock.unlock();
        return;
    }

    for( int i = 0 ; i < cnt ; i++ )
    {
        mlBufToAlloc.push_back(ePort);
    }
    mCondBufAlloc.signal();
    mAllocationLock.unlock();
} 


/*******************************************************************************
*
********************************************************************************/
void 
PreviewBufMgr::
freeBuffer(int ePort)
{
    mAllocationLock.lock();
    if( mbThreadRunning && 0 != mAllocMemThreadHandle)
    {
        mAllocationLock.unlock();

        MY_LOGD("wait for previous allocation done");
        pthread_join(mAllocMemThreadHandle, NULL);

        mAllocationLock.lock();
        mAllocMemThreadHandle = 0;
    }

    if (ePort & eID_Pass1Out )
    {
        mspHwBufPvdr->removeBuf();
        muPortAllocated &= ~eID_Pass1Out;
    }
    
    if (ePort & eID_Pass1DispOut )
    {
        mspHwBufDispPvdr->removeBuf();
        muPortAllocated &= ~eID_Pass1DispOut;
    }
    mAllocationLock.unlock();
}


/*******************************************************************************
*
********************************************************************************/
bool 
PreviewBufMgr::
dequeBuffer(int ePort, ImgBufQueNode &node)
{
    bool ret = false; 
    
    switch (ePort)
    {
        case eID_Pass1Out:
        {
            if ( mspHwBufPvdr != 0 )
            {
                sp<IImgBuf> buf;
                mAllocationLock.lock();
                mspHwBufPvdr->deque(buf);

                if( buf.get() == NULL )
                {
                    //no buf
                    if( muPortAllocating & ePort )
                    {
                        //if buf is allocating, wait
                        MY_LOGD("wait for allocationg");
                        mCondPass1OutReady.wait(mAllocationLock);

                        mspHwBufPvdr->deque(buf);
                    }
                    else
                    {
                        MY_LOGW("should not happens, no buf and not allocating");
                    }
                }
                mAllocationLock.unlock();

                node = ImgBufQueNode(buf, ImgBufQueNode::eSTATUS_TODO);
                node.setCookieDE(eBuf_Pass1);
                ret = true;
            }
        }
        break;
        //
        case eID_Pass1RawOut:
        {
            //for Pass1Out with timestamp: get non-empty buffer
            if ( mspHwBufPvdr != 0 )
            {
                sp<IImgBuf> buf;
                mAllocationLock.lock();
                if( muPortAllocating & eID_Pass1Out )
                {
                    MY_LOGW("buffers are allocating");
                    node = ImgBufQueNode(buf, ImgBufQueNode::eSTATUS_TODO);
                    node.setCookieDE(eBuf_Pass1);
                    ret = true;
                    break;
                }
                mAllocationLock.unlock();

                mspHwBufPvdr->deque(buf);

                if( buf->getTimestamp() == 0 )
                {
                    MY_LOGD("get empty pass1 buf");
                    //deque all buffer and find the one with timestamp
                    vector< sp<IImgBuf> > queue;

                    queue.push_back( buf );
                    buf = NULL;

                    do{
                        sp<IImgBuf> buffer;
                        mspHwBufPvdr->deque(buffer);

                        if( buffer.get() )
                        {
                            queue.push_back( buffer );
                        } else {
                            break;
                        }
                    }while(1);

                    MY_LOGD("deque all buf %i", queue.size());

                    vector< sp<IImgBuf> >::iterator iter;
                    for(iter = queue.begin() ; iter != queue.end(); iter++ )
                    {
                        if( (*iter)->getTimestamp() != 0 && buf.get() == NULL )
                        {
                            MY_LOGD("find timestamp(%lld)", (*iter)->getTimestamp() );
                            buf = *iter;
                        }
                        else
                        {
                            mspHwBufPvdr->enque( *iter );
                        }
                    }
                    queue.clear();

                    if( buf == NULL )
                    {
                        MY_LOGW("all %i bufers are empty, get empty one", queue.size());
                        mspHwBufPvdr->deque(buf);
                    }
                }

                node = ImgBufQueNode(buf, ImgBufQueNode::eSTATUS_TODO);
                node.setCookieDE(eBuf_Pass1);
                ret = true;
            }
        }
        break;
        //
        case eID_Pass1DispOut:
        {
            if ( mspHwBufDispPvdr != 0 )
            {
                sp<IImgBuf> buf;
                mAllocationLock.lock();
                mspHwBufDispPvdr->deque(buf);

                if( buf.get() == NULL )
                {
                    //no buf
                    if( muPortAllocating & ePort )
                    {
                        //if buf is allocating, wait
                        MY_LOGD("wait for allocationg");
                        mCondPass1DispOutReady.wait(mAllocationLock);

                        mspHwBufDispPvdr->deque(buf);
                    }
                    else
                    {
                        MY_LOGW("should not happens, no buf and not allocating");
                    }
                }
                mAllocationLock.unlock();

                node = ImgBufQueNode(buf, ImgBufQueNode::eSTATUS_TODO);
                node.setCookieDE(eBuf_Pass1Disp);
                ret = true;
            }
        }
        break;
        //
        case eID_Pass2DISPO:
        {
            sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getDisplayPvdr();
            if (bufProvider != 0 && bufProvider->dequeProvider(node))
            {
                node.setCookieDE(eBuf_Disp);
                ret = true;
            }
        }
        break;
        //      
        case eID_Pass2VIDO:
        {
            {
                sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getPrvCBPvdr();
                if (bufProvider != 0 && bufProvider->dequeProvider(node))
                {
                    node.setCookieDE(eBuf_AP);
                    ret = true;
                    break;
                }
            }
            {
                sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getFDBufPvdr();
                if (bufProvider != 0 && bufProvider->dequeProvider(node))
                {
                    node.setCookieDE(eBuf_FD);
                    ret = true;
                    break;
                }
            }
            {
                sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getGenericBufPvdr();
                if (bufProvider != 0 && bufProvider->dequeProvider(node))
                {
                    node.setCookieDE(eBuf_Generic);
                    ret = true;
                    break;
                }                
            }
            
        }
        break;
        //
        default:
            MY_LOGE("unknown port!!");
        break;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
bool 
PreviewBufMgr::
enqueBuffer(ImgBufQueNode const& node)
{
    // (1) set DONE tag into package
    const_cast<ImgBufQueNode*>(&node)->setStatus(ImgBufQueNode::eSTATUS_DONE);

    // (2) choose the correct "client"
    switch (node.getCookieDE())
    {
        case eBuf_Pass1:
        {
            if (mspHwBufPvdr != 0)
            {
                mspHwBufPvdr->enque(node.getImgBuf());
            }
        }
        break;
        //
        case eBuf_Pass1Disp:
        {
            if (mspHwBufDispPvdr != 0)
            {
                mspHwBufDispPvdr->enque(node.getImgBuf());
            }
        }
        break;
        //
        case eBuf_Disp:
        {
            sp<IImgBufProvider> bufProvider = mspImgBufProvidersMgr->getDisplayPvdr();
            if (bufProvider != 0)
            {
                bufProvider->enqueProvider(node);
            }
        }
        break;
        //
        case eBuf_AP:
        {
            sp<IImgBufProvider> bufProvider;
            {
                bufProvider = mspImgBufProvidersMgr->getPrvCBPvdr();
                if ( bufProvider != 0 )
                {
                    const_cast<ImgBufQueNode*>(&node)->setCookieDE(0); // 0 for preview
                    bufProvider->enqueProvider(node);
                }

                // If fd exists, copy to it
                bufProvider = mspImgBufProvidersMgr->getFDBufPvdr();
                ImgBufQueNode FDnode; 
                if (bufProvider != 0 && bufProvider->dequeProvider(FDnode))
                {
                    if ( FDnode.getImgBuf()->getBufSize() >= node.getImgBuf()->getBufSize())
                    {
                        memcpy(FDnode.getImgBuf()->getVirAddr(), 
                           node.getImgBuf()->getVirAddr(), 
                           node.getImgBuf()->getBufSize());
                    }
                    else 
                    {
                        MY_LOGE("fd buffer size < ap buffer size");
                        const_cast<ImgBufQueNode*>(&FDnode)->setStatus(ImgBufQueNode::eSTATUS_CANCEL);
                    }
                    //
                    bufProvider->enqueProvider(FDnode);                    
                }
            }
        }
        break;        
        //
        case eBuf_FD:
        {
            sp<IImgBufProvider> bufProvider = mspImgBufProvidersMgr->getFDBufPvdr();
            if (bufProvider != 0)
            {
                bufProvider->enqueProvider(node);
            }
        }
		break;
        //
        case eBuf_Generic:
        {
            sp<IImgBufProvider> bufProvider = mspImgBufProvidersMgr->getGenericBufPvdr();
            if (bufProvider != 0)
            {
                bufProvider->enqueProvider(node);
            }            
        }
        break;
        //
        default:
            MY_LOGE("unknown port(%d)!!", node.getCookieDE());
        break;
    }
    
    return true;
}


/*******************************************************************************
*
********************************************************************************/
PreviewBufMgr::
PreviewBufMgr(sp<ImgBufProvidersManager> &rImgBufProvidersMgr)
    : mspImgBufProvidersMgr(rImgBufProvidersMgr)
    , mspHwBufPvdr(new HwBuffProvider())
    , mspHwBufDispPvdr(new HwBuffProvider())
    , muPortAllocated(0x0)
    , muPortAllocating(0x0)
    , mbThreadRunning(0)
    , mAllocMemThreadHandle(0)
{
}


/*******************************************************************************
*
********************************************************************************/
PreviewBufMgr::~PreviewBufMgr()
{
    MY_LOGD("this=%p, ImgBufMgr=%p, HwBufPvdr=%p, HwBuf2Pvdr=%p, sizeof:%d", 
            this, &mspImgBufProvidersMgr, &mspHwBufPvdr, &mspHwBufDispPvdr,
            sizeof(PreviewBufMgr));
}


/*******************************************************************************
*
********************************************************************************/
void 
PreviewBufMgr::
destroyInstance()
{
    // let mspHwBufPvdr de-allocate by itself
    // let mspHwBufDispPvdr de-allocate by itself
}


/*******************************************************************************
*
********************************************************************************/
IPreviewBufMgr*
IPreviewBufMgr::
createInstance(sp<ImgBufProvidersManager> &rImgBufProvidersMgr)
{
    return new PreviewBufMgr(rImgBufProvidersMgr);
}
