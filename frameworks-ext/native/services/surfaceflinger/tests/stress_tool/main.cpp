#define LOG_TAG "stresstool"

#include <stdio.h>

#include <log/log.h>

#include <utils/List.h>
#include <utils/Vector.h>

#include <ui/DisplayInfo.h>
#include <ui/gralloc_extra.h>

#include "Stretegy.h"
#include "Frame.h"
#include "TestSurface.h"
#include "Action.h"
#include "App.h"
//#include "Monkey.h"

using namespace android;

inline int getBitsOfFormat(int format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        //case SGX_COLOR_FORMAT_BGRX_8888:
            return 32;

        case HAL_PIXEL_FORMAT_RGB_565:
            return 16;

        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YV12:
            return 12;
    }
    return 0;
}

/*
Frame::Config framesCfg[] =
{
    //{String8("/data/00_1088_960.RGBA"),                  1088,  1088,  960, 0, HAL_PIXEL_FORMAT_RGBA_8888,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_CPU},
    {String8("/data/01_544_960.RGBA"), 544, 544,  960, 0, HAL_PIXEL_FORMAT_RGBA_8888,       NATIVE_WINDOW_API_CPU,   GRALLOC_EXTRA_BIT_TYPE_CPU},
    {String8("/data/02_544_960.RGBA"),     544,  544,  960, 0, HAL_PIXEL_FORMAT_RGBA_8888,   NATIVE_WINDOW_API_CPU,   GRALLOC_EXTRA_BIT_TYPE_CPU},
};
*/

Frame::Config framesCfg[] =
{
    {String8("/data/LGE.yv12"),                  400,  416,  240, 0, HAL_PIXEL_FORMAT_YV12,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/BigShips_1280x720_1.i420"), 1280, 1280,  720, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/football_qvga_1.i420"),      320,  320,  240, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/indoor_slow_1.i420"),        848,  848,  480, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/Kimono1_1920x1088_1.i420"), 1920, 1920, 1088, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/mobile_qcif_1.i420"),        176,  176,  144, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/newyork_640x368_1.i420"),    640,  640,  368, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/out_176_144.i420"),          176,  176,  144, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/school_640x480_1.i420"),     640,  640,  480, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/prv03.i420"),                640,  640,  480, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_CAMERA,  GRALLOC_EXTRA_BIT_TYPE_CAMERA},
    // because the MTKYUB raw data does not contain padding bits,
    // keep the stride be the same value as width.
    {String8("/data/ibmbw_720x480_mtk.yuv"),     720,  720,  480, 0, HAL_PIXEL_FORMAT_NV12_BLK,   NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
};

const static unsigned int NUM_FRAMES = sizeof(framesCfg) / sizeof(Frame::Config);

int main()
{
    uint32_t surfaceNum = 2;
    sp<App> app = new App();


    ALOGI("Load frames");
    Vector<sp<Frame> > frames;
    for (uint32_t i = 0; i < NUM_FRAMES; ++i)
    {
        uint32_t bpp = getBitsOfFormat(framesCfg[i].format);
        if (bpp == 0)
        {
            //ALOGE("Unknown format");
        }
        framesCfg[i].pitch = framesCfg[i].s * bpp / 8;
        sp<Frame> frame = new Frame(framesCfg[i]);
        if (frame == NULL)
        {
            ALOGE("create Frame failed");
        } else
        {
            frames.push_back(frame);
        }
    }

    Vector<List<sp<Frame> > > showFramesSet;
    for (uint32_t i = 0; i < surfaceNum; ++i)
    {
        List<sp<Frame> > showFrames;
        //showFrames.push_back(frames[i]);
        for (uint32_t i = 0; i < NUM_FRAMES; ++i) {
            showFrames.push_back(*(frames.begin() + i));
        }
        showFramesSet.push_back(showFrames);
    }

    DisplayInfo dinfo;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    SurfaceComposerClient::getDisplayInfo(display, &dinfo);

    ALOGI("Create Surface");
    Vector<sp<TestSurface> > surfaces;
    for (uint32_t i = 0; i < surfaceNum; ++i)
    {
        TestSurface::Config cfg;
        {
            cfg.dispW = dinfo.w;
            cfg.dispH = dinfo.h;
            cfg.minSize = 65;
            cfg.scale = true;
            cfg.move = true;
            cfg.zOrder = true;
            cfg.surfaceCrop = true;
            cfg.bufferCrop = true;
            cfg.blend = true;
        }
        TestSurface::State state;
        {
            state.x = 0 + i * 100;
            state.y = 0 + i * 100;
            state.w = 400;
            state.h = 400;
            //state.w = dinfo.w;
            //state.h = dinfo.h;
            state.zOrder = 1000000 + i;
            //state.crop.left = 0;
            //state.crop.top = 0;
            //state.crop.right = 100;
            //state.crop.bottom = 100;
            state.bufferCrop.left = 0;
            state.bufferCrop.left = 0;
            state.bufferCrop.top = (*showFramesSet[i].begin())->cfg.w - 1;
            state.bufferCrop.right = (*showFramesSet[i].begin())->cfg.h - 1;
            state.surfaceCrop.left = 0;
            state.surfaceCrop.left = 0;
            state.surfaceCrop.top = 0;
            state.surfaceCrop.right = state.w - 1;
            state.surfaceCrop.bottom = state.h - 1;
            state.alpha = 1.f;
            state.format = HAL_PIXEL_FORMAT_RGB_565;
            //state.flags = 0;
        }
        sp<TestSurface> surface = new TestSurface(String8("test"),
                                           showFramesSet[i],
                                           cfg,
                                           state);
        if (surface == NULL)
        {
            ALOGE("create surface failed");
        } else
        {
            app->addSurface(surface);
        }
    }

    ALOGD("Run app");
    //app->setActionFlag(App::ACTION_SCALE | App::ACTION_MOVE | App::ACTION_SHUFFLE | App::ACTION_BLEND);
    //app->setActionFlag(App::ACTION_CLOCKWISEMOVE);
    //app->setActionFlag(App::ACTION_JUMP);
    //app->addStretegy(new FreeMove(60));
    //app->addStretegy(new FreeScale(60));
    //app->addStretegy(new Shuffle(60));
    //app->addStretegy(new FreeBlend(60));
    //app->addStretegy(new ClockwiseMove(60));
    app->run();
    app->join();
}

