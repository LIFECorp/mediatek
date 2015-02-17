#ifndef MTK_TEST_SURFACE_ACTION_H
#define MTK_TEST_SURFACE_ACTION_H

#include <utils/RefBase.h>

#include <ui/Rect.h>

#include "TestSurface.h"

class Action : public android::RefBase
{
public:
    Action() {}
    virtual ~Action() {}

    virtual bool check(const TestSurface::Config& cfg,
                       const TestSurface::State& state,
                       const android::sp<Frame>& frame) const
    {
        ALOGD("should not use this function");
        return true;
    }

    virtual bool bufferApply(ANativeWindow* const window,
                             TestSurface::State* const state) const
    {
        return true;
    }

    virtual bool surfaceApply(const android::sp<android::SurfaceControl>& control,
                              TestSurface::State* const state) const
    {
        return true;
    }

};

class Scale : public Action
{
public:
    Scale(const int32_t& w, const int32_t& h);
    virtual ~Scale() {}

    bool check(const TestSurface::Config& cfg,
               const TestSurface::State& state,
               const android::sp<Frame>& frame) const;

    bool surfaceApply(const android::sp<android::SurfaceControl>& control,
                      TestSurface::State* const state) const;
private:
    int32_t mW, mH;
};


class Move : public Action
{
public:
    Move(const int32_t& w, const int32_t& h);
    virtual ~Move() {}

    bool check(const TestSurface::Config& cfg,
               const TestSurface::State& state,
               const android::sp<Frame>& frame) const;

    bool surfaceApply(const android::sp<android::SurfaceControl>& control,
                      TestSurface::State* const state) const;
private:
    int32_t mX, mY;
};

class SurfaceCrop : public Action
{
public:
    SurfaceCrop(const android::Rect& crop);
    virtual ~SurfaceCrop() {}

    bool check(const TestSurface::Config& cfg,
               const TestSurface::State& state,
               const android::sp<Frame>& frame) const;

    bool surfaceApply(const android::sp<android::SurfaceControl>& control,
                      TestSurface::State* const state) const;

private:
    android::Rect mCrop;
};

class BufferCrop : public Action
{
public:
    BufferCrop(const android::Rect& crop);
    virtual ~BufferCrop() {}

    bool check(const TestSurface::Config& cfg,
               const TestSurface::State& state,
               const android::sp<Frame>& frame) const;

    bool bufferApply(ANativeWindow* const window,
                     TestSurface::State* const state) const;
private:
    android::Rect mCrop;
};

class Blend : public Action
{
public:
    Blend(const float& alpha);

    bool check(const TestSurface::Config& cfg,
               const TestSurface::State& state,
               const android::sp<Frame>& frame) const;

    bool surfaceApply(const android::sp<android::SurfaceControl>& control,
                      TestSurface::State* const state) const;

    virtual ~Blend() {}

private:
    float mAlpha;
};

class SetZOrder : public Action
{
public:
    SetZOrder(const int32_t& zOrder);
    virtual ~SetZOrder() {}

    bool check(const TestSurface::Config& cfg,
               const TestSurface::State& state,
               const android::sp<Frame>& frame) const;

    bool surfaceApply(const android::sp<android::SurfaceControl>& control,
                      TestSurface::State* const state) const;


private:
    int32_t mZOrder;
};
#endif // MTK_TEST_SURFACE_ACTION_H
