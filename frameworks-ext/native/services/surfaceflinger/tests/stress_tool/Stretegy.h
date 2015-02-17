#ifndef MTK_TEST_SURFACE_STRETEGY_H
#define MTK_TEST_SURFACE_STRETEGY_H
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>

#include "TestSurface.h"

class TestSurface;

class Stretegy : public android::RefBase
{
public:
    Stretegy(const uint64_t& changeFreq)
        : mCount(0), mChangeFreq(changeFreq) {}
    virtual ~Stretegy() {}
    virtual bool apply(const android::Vector<android::sp<TestSurface> >& surfaces) = 0;

protected:
    uint64_t mCount;
    const uint64_t mChangeFreq;
};

class FreeMove : public Stretegy
{
public:
    FreeMove(const uint64_t& changeFreq) : Stretegy(changeFreq) {}
    virtual ~FreeMove() {}
    bool apply(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};

class FreeScale : public Stretegy
{
public:
    FreeScale(const uint64_t& changeFreq) : Stretegy(changeFreq) {}
    virtual ~FreeScale() {}
    bool apply(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};

class FreeBlend : public Stretegy
{
public:
    FreeBlend(const uint64_t& changeFreq) : Stretegy(changeFreq) {}
    virtual ~FreeBlend() {}
    bool apply(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};

class Shuffle : public Stretegy
{
public:
    Shuffle(const uint64_t& changeFreq) : Stretegy(changeFreq) {}
    virtual ~Shuffle() {}
    bool apply(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::Vector<TestSurface::State> mStates;
};

class ClockwiseMove : public Stretegy
{
public:
    ClockwiseMove(const uint64_t& changeFreq) : Stretegy(changeFreq) {}
    virtual ~ClockwiseMove() {}
    bool apply(const android::Vector<android::sp<TestSurface> >& surfaces);
private:
    android::KeyedVector<void*, TestSurface::State> mStates;
};
#endif // MTK_TEST_SURFACE_STRETEGY_H
