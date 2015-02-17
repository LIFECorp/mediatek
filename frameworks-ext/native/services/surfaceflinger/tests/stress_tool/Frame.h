#ifndef MTK_TEST_SURFACE_FRAME_H
#define MTK_TEST_SURFACE_FRAME_H

#include <stdio.h>
#include <errno.h>

#include <utils/String8.h>
#include <utils/RefBase.h>
//#include <utils/Log.h>

#include <ui/PixelFormat.h>
struct Frame : public android::RefBase
{
    struct Config
    {
        android::String8 path;
        uint32_t w, s, h;
        uint32_t pitch;
        uint32_t format;
        uint32_t api;
        uint32_t usageEx;
    };


    const Config cfg;
    char* data;

    Frame(const Config& tcfg)
        : cfg(tcfg)
    {
        FILE* fp = fopen(cfg.path.string(), "rb");
        if (fp == NULL) {
            fprintf(stderr, "fopen(%s) failed %s\n", cfg.path.string(), strerror(errno));
            return ;
        }

        data = new char[cfg.pitch * cfg.h];
        if (data == NULL) {
            ALOGE("Frame::Frame()- allocating data failed");
        }
        if (fread(data, 1, cfg.pitch * cfg.h, fp) != cfg.pitch * cfg.h) {
            ALOGE("Frame::Frame()- fread(%s) size:%d failed", cfg.path.string(), cfg.pitch * cfg.h);
            return ;
        }
        fclose(fp);
    }

    virtual ~Frame()
    {
        delete[] data;
    }
};

#endif // MTK_TEST_SURFACE_FRAME_H
