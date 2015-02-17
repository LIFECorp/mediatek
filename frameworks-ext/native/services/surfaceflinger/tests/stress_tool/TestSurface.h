#ifndef MTK_TEST_SURFACE_TEST_SURFACE_H
#define MTK_TEST_SURFACE_TEST_SURFACE_H

#include <utils/RefBase.h>
#include <utils/List.h>

#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <ui/PixelFormat.h>

#include "Dump.h"

class Action;
class Frame;

class TestSurface : public android::RefBase, Dump
{
public:
    struct Config
    {
        uint32_t dispW, dispH;
        uint32_t minSize;
        bool scale;
        bool move;
        bool zOrder;
        bool surfaceCrop;
        bool bufferCrop;
        bool blend;
        Config() : dispW(0), dispH(0), minSize(0),
                   scale(true), move(true), zOrder(true),
                   surfaceCrop(true), bufferCrop(true), blend(true)
        {
        }
    };

    struct State
    {
        int32_t x, y;
        int32_t w, h;
        int32_t zOrder;
        android::Rect bufferCrop;
        android::Rect surfaceCrop;
        float alpha;
        android::PixelFormat format;
        uint32_t flags;
        State() : x(0), y(0), w(0), h(0), zOrder(0), alpha(1.f), format(android::PIXEL_FORMAT_NONE), flags(0)
        {
            bufferCrop.left = bufferCrop.top = bufferCrop.right = bufferCrop.bottom = 0;
            surfaceCrop.left = surfaceCrop.top = surfaceCrop.right = surfaceCrop.bottom = 0;
        }
    };
private:

    const android::List<android::sp<Frame> > mFrames;

    const android::sp<android::SurfaceComposerClient> mComposer;
    android::sp<android::SurfaceControl> mControl;
    android::sp<android::Surface> mSurface;
    ANativeWindow* mWindow;
    ANativeWindowBuffer* mANWBuf;
    android::List<android::sp<Frame> >::const_iterator mShowFrame;
    android::List<android::sp<Action> > mActions;

    const Config mCfg;
    State mState;

    bool apply(const State& state);
public:
    TestSurface(const android::String8& name,
                const android::List<android::sp<Frame> >& frames,
                const Config& cfg,
                const State& state);

    inline const Config& getConfig() const { return mCfg; }
    inline const State& getState() const {return mState; }

    bool accept(const android::sp<Action>& action);

    void prepare();
    void refresh();
    void dump() const;
    inline uint32_t getFrameIdx()
    {
        return mFrames.distance(mFrames.begin(), mShowFrame);
    }
};
#endif // MTK_TEST_SURFACE_TEST_SURFACE_H
