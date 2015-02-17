#define LOG_TAG "stresstool"
#include "Action.h"

using namespace android;

Scale::Scale(const int32_t& w, const int32_t& h) : mW(w), mH(h)
{
}

bool Scale::check(const TestSurface::Config& cfg,
                  const TestSurface::State& state,
                  const android::sp<Frame>& frame) const
{
    if (!cfg.scale)
    {
        ALOGE("Surface do not allow scaling");
        return false;
    }
    ALOGD("Scale::check() mW:%d mH:%d", mW, mH);
    if (!(mW >= 0 && mH >= 0 &&
            state.x + static_cast<uint32_t>(mW) >= cfg.minSize && static_cast<uint32_t>(mW) >= cfg.minSize && static_cast<uint32_t>(mW) < cfg.dispW * 1.5 &&
            state.y + static_cast<uint32_t>(mH) >= cfg.minSize && static_cast<uint32_t>(mH) >= cfg.minSize && static_cast<uint32_t>(mH) < cfg.dispH * 1.5))
    {
        ALOGE("Scaling's checking failed x:%d y:%d w:%d h:%d minSize:%d", state.x, state.y, mW, mH, cfg.minSize);
        return false;
    }
    return true;
}

bool Scale::surfaceApply(const sp<SurfaceControl>& control,
                         TestSurface::State* const state) const
{
    if (CC_UNLIKELY(control->setSize(mW, mH) != NO_ERROR))
    {
        ALOGE("set size(%d, %d) failed", mW, mH);
        return false;
    }
    ALOGD("set size(%d,%d)", mW, mH);
    state->w = mW;
    state->h = mH;
    return true;
}

// =============================================================

Move::Move(const int32_t& x, const int32_t& y) : mX(x), mY(y)
{
}

bool Move::check(const TestSurface::Config& cfg,
                 const TestSurface::State& state,
                 const android::sp<Frame>& frame) const
{
    if (!cfg.move)
    {
        ALOGE("Surface do not allow moving");
        return false;
    }

    if (!(mX + state.w >= static_cast<int32_t>(cfg.minSize) && mX < static_cast<int32_t>(cfg.dispW - cfg.minSize) &&
            mY + state.h >= static_cast<int32_t>(cfg.minSize) && mY < static_cast<int32_t>(cfg.dispH - cfg.minSize)))
    {
        ALOGE("Moving's checking failed x:%d y:%d w:%d h:%d minSize:%d", mX, mY, state.w, state.h, cfg.minSize);
        return false;
    }
    return true;
}

bool Move::surfaceApply(const sp<SurfaceControl>& control,
                        TestSurface::State* const state) const
{
    if (CC_UNLIKELY(control->setPosition(mX, mY) != NO_ERROR))
    {
        ALOGE("set pos(%d, %d) failed", mX, mY);
        return false;
    }
    ALOGD("set pos(%d,%d)", mX, mY);
    state->x = mX;
    state->y = mY;
    return true;
}

// =============================================================

SurfaceCrop::SurfaceCrop(const Rect& crop) : mCrop(crop)
{
}

bool SurfaceCrop::check(const TestSurface::Config& cfg,
                        const TestSurface::State& state,
                        const android::sp<Frame>& frame) const
{
    if (!cfg.surfaceCrop)
    {
        ALOGE("Surface do not allow crop");
        return false;
    }

    if (!(mCrop.left >= 0 && mCrop.top >= 0 &&
            mCrop.right < static_cast<int>(state.w) && mCrop.bottom < static_cast<int>(state.h)))
    {
        ALOGE("SurfaceCrop checking failed- left:%d top:%d right:%d bottom:%d w:%d h:%d", mCrop.left, mCrop.top, mCrop.right, mCrop.bottom, state.w, state.h);
        return false;
    }
    return true;
}

bool SurfaceCrop::surfaceApply(const sp<SurfaceControl>& control,
                               TestSurface::State* const state) const
{
    if (CC_UNLIKELY(control->setCrop(mCrop) != NO_ERROR))
    {
        ALOGE("set surface crop(%d, %d, %d, %d) failed", mCrop.left, mCrop.top, mCrop.right, mCrop.bottom);
        return false;
    }
    ALOGD("set surface crop(%d, %d, %d, %d)", mCrop.left, mCrop.top, mCrop.right, mCrop.bottom);
    state->surfaceCrop.set(mCrop);
    return true;
}

// =============================================================

BufferCrop::BufferCrop(const Rect& crop) : mCrop(crop)
{
}

bool BufferCrop::check(const TestSurface::Config& cfg,
                       const TestSurface::State& state,
                       const android::sp<Frame>& frame) const
{
    if (!cfg.bufferCrop)
    {
        ALOGE("Surface do not allow buffer crop");
        return false;
    }

    if (!(mCrop.left >= 0 && mCrop.top >= 0 &&
            mCrop.right < static_cast<int>(state.w) && mCrop.bottom < static_cast<int>(state.h)))
    {
        ALOGE("BufferCrop checking failed- left:%d top:%d right:%d bottom:%d w:%d h:%d", mCrop.left, mCrop.top, mCrop.right, mCrop.bottom, state.w, state.h);
        return false;
    }
    return true;
}

bool BufferCrop::bufferApply(ANativeWindow* const mWindow,
                             TestSurface::State* const state) const
{
    if (native_window_set_crop(mWindow, ((android_native_rect_t*)(&mCrop))) == -EINVAL)
    {
        ALOGE("set buffer crop(%d, %d, %d, %d) failed", mCrop.left, mCrop.top, mCrop.right, mCrop.bottom);
        return false;
    }
    ALOGD("set buffer crop(%d, %d, %d, %d)", mCrop.left, mCrop.top, mCrop.right, mCrop.bottom);
    state->surfaceCrop.set(mCrop);
    return true;
}

// =============================================================

Blend::Blend(const float& alpha) : mAlpha(alpha)
{
}

bool Blend::check(const TestSurface::Config& cfg,
                  const TestSurface::State& state,
                  const android::sp<Frame>& frame) const
{
    ALOGE("Blending check alpha:%f", mAlpha);
    if (!cfg.blend)
    {
        ALOGE("Surface do not allow blending");
        return false;
    }
    if (mAlpha < 0.f || mAlpha > 1.f)
    {
        ALOGE("Blending checking failed- alpha:%f", mAlpha);
        return false;
    }
    return true;
}

bool Blend::surfaceApply(const sp<SurfaceControl>& control,
                         TestSurface::State* const state) const
{
    if (CC_UNLIKELY(control->setAlpha(mAlpha) != NO_ERROR))
    {
        ALOGE("set alpha(%f) failed", mAlpha);
        return false;
    }
    ALOGD("set alpha(%f)", mAlpha);
    state->alpha = mAlpha;
    return true;
}

// =============================================================

SetZOrder::SetZOrder(const int32_t& zOrder) : mZOrder(zOrder)
{
}

bool SetZOrder::check(const TestSurface::Config& cfg,
                  const TestSurface::State& state,
                  const android::sp<Frame>& frame) const
{
    if (!cfg.zOrder)
    {
        ALOGE("Surface do not allow shuffling");
        return false;
    }
    return true;
}

bool SetZOrder::surfaceApply(const sp<SurfaceControl>& control,
                         TestSurface::State* const state) const
{
    if (CC_UNLIKELY(control->setLayer(mZOrder) != NO_ERROR))
    {
        ALOGE("set ZOrder(%d) failed", mZOrder);
        return false;
    }
    ALOGD("set ZOrder(%d)", mZOrder);
    state->zOrder = mZOrder;
    return true;
}
