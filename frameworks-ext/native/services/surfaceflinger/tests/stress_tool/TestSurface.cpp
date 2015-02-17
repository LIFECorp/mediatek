#define LOG_TAG "stresstool"

#include "TestSurface.h"

#include <cutils/compiler.h>
#include <utils/String8.h>

#include <log/log.h>

#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferExtra.h>
#include <ui/gralloc_extra.h>

#include "Frame.h"
#include "Action.h"

using namespace android;

TestSurface::TestSurface(const String8& name,
                         const List<sp<Frame> >& frames,
                         const Config& cfg,
                         const State& state)
        : mFrames(frames),
          mComposer(new SurfaceComposerClient()),
          mANWBuf(NULL),
          mCfg(cfg),
          mState(state)
{
    if (CC_UNLIKELY(mComposer == NULL))
    {
        ALOGE("Create SurfaceComposerClient failed");
    }

    if (CC_UNLIKELY(mFrames.size() == 0))
    {
        ALOGE("No frames in the Frames list");
    }

    ALOGI("TestSurface(): s:%d h:%d fmt:%d api:%d", state.w, state.h, state.format, (*frames.begin())->cfg.api );

    mControl = mComposer->createSurface(name,
              state.w,
              state.h,
              state.format);
    if (CC_UNLIKELY(mControl == NULL))
    {
        ALOGE("Create SurfaceControl failed");
    }

    mSurface = mControl->getSurface();
    if (CC_UNLIKELY(mSurface == NULL))
    {
        ALOGE("Create SurfaceControl failed");
    }

    mWindow = mSurface.get();
    if (CC_UNLIKELY(mWindow == NULL))
    {
        ALOGE("Get ANativeWindow failed");
    }

    mShowFrame = mFrames.begin();

    // set api connection type as register
    // TODO: How to get the api type
    if (native_window_api_connect(mWindow, (*frames.begin())->cfg.api) == -EINVAL)
    {
        ALOGE("native_window_api_connect() failed");
    }

    // TODO: Setting buffer count may take as an Action
    // set buffer count
    if (native_window_set_buffer_count(mWindow, frames.size()) == -EINVAL)
    {
        ALOGE("native_window_set_buffer_count() failed");
    }

    accept(new Move(state.x, state.y));
    accept(new Scale(state.w, state.h));
    //accept(new SurfaceCrop(state.surfaceCrop));
    accept(new Blend(1.f));
    //accept(new BufferCrop(state.bufferCrop));
    accept(new SetZOrder(state.zOrder));
}

bool TestSurface::accept(const sp<Action>& action)
{
    if (action->check(mCfg, mState, *mShowFrame))
    {
        mActions.push_back(action);
        return true;
    } else
    {
        ALOGE("Action checking failed");
        return false;
    }
}
/*
bool TestSurface::apply(const State& state) {
    const uint32_t minSize = 10;
    if (state.x + state.w >= minSize && state.w >= minSize && state.x < mCfg.dispW - minSize && state.w < mCfg.dispW * 1.5 &&
            state.y + state.h >= minSize && state.h >= minSize && state.y < mCfg.dispH - minSize && state.w < mCfg.dispH * 1.5 &&
            state.alpha >= 0.f) {
        mState = state;
        return true;
    } else {
        ALOGD("Invalid operation: x:%d y:%d w:%d, h:%d, dispW:%d, dispH:%d minSize:%d", state.x,
                                                                                        state.y,
                                                                                        state.w,
                                                                                        state.h,
                                                                                        mCfg.dispW,
                                                                                        mCfg.dispH,
                                                                                        minSize);
        return false;
    }
}
*/

void TestSurface::prepare()
{
    if (CC_UNLIKELY(mShowFrame == mFrames.end()))
    {
        ALOGE("Invalid mShowFrame");
    }

    ALOGI("prepare(): pitch:%d s:%d h:%d fmt:%d", (*mShowFrame)->cfg.pitch, (*mShowFrame)->cfg.s, (*mShowFrame)->cfg.h, (*mShowFrame)->cfg.format);
    for (List<sp<Action> >::iterator iter = mActions.begin(); iter!= mActions.end(); ++iter)
    {
        (*iter)->bufferApply(mWindow, &mState);
    }
    // set buffer size
    native_window_set_buffers_dimensions(mWindow, (*mShowFrame)->cfg.s, (*mShowFrame)->cfg.h);

    // set format
    native_window_set_buffers_format(mWindow, (*mShowFrame)->cfg.format);

    // set usage software write-able and hardware texture bind-able
    native_window_set_usage(mWindow, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

    // set scaling to match window display size
    native_window_set_scaling_mode(mWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);



    int fenceFd = -1;
    if (mWindow->dequeueBuffer(mWindow, &mANWBuf, &fenceFd))
    {
        ALOGE("dequeueBuffer failed");
        return ;
    }

    sp<Fence> fence = new Fence(fenceFd);
    fence->wait(Fence::TIMEOUT_NEVER);

    sp<GraphicBuffer> gb = new GraphicBuffer(mANWBuf, false);
    if (gb == NULL)
    {
        ALOGE("creating GraphicBuffer failed");
    }
    const Rect rect((*mShowFrame)->cfg.w, (*mShowFrame)->cfg.h);
    void* ptr = NULL;

    GraphicBufferExtra::get().setBufParameter(gb->handle, GRALLOC_EXTRA_MASK_TYPE, (*mShowFrame)->cfg.usageEx);

    if (CC_UNLIKELY(gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr) != NO_ERROR))
    {
        ALOGE("GraphicBuffer lock failed");
    }

    {
        memcpy(ptr, (*mShowFrame)->data, (*mShowFrame)->cfg.pitch * (*mShowFrame)->cfg.h);
    }

    if (CC_UNLIKELY(gb->unlock() != NO_ERROR))
    {
        ALOGE("GraphicBuffer unlock failed");
    }

    ++mShowFrame;
    if (mShowFrame == mFrames.end())
    {
        mShowFrame = mFrames.begin();
    }
    if (CC_UNLIKELY(mWindow->queueBuffer(mWindow, mANWBuf, -1)))
    {
        ALOGE("refresh(): queueBuffer() failed");
    }
}


void TestSurface::refresh()
{
    ALOGI("refresh(): mActions.size():%d x:%d y:%d w:%d h:%d zOrder:%d", mActions.size(), mState.x, mState.y, mState.w, mState.h, mState.zOrder);

    for (List<sp<Action> >::iterator iter = mActions.begin(); iter!= mActions.end(); ++iter)
    {
        (*iter)->surfaceApply(mControl, &mState);
    }

    ALOGD("close transaction");

    mActions.clear();
}

void TestSurface::dump() const
{
}
