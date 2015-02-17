#define LOG_TAG "stresstool"
#include "Stretegy.h"

#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>

#include "Action.h"
#include "TestSurface.h"

using namespace android;

bool FreeMove::apply(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i)
    {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0)
            {
                ALOGE("Allocation state failed when FreeMove::Apply()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);

        if (mCount % mChangeFreq == 0)
        {
            state.x = rand() % 3 - 1;
            state.y = rand() % 3 - 1;
        }
        ALOGD("App::run() idx:%d(%d) - dice_x:%d dice_x:%d", i, surfaces.size(), state.x, state.y);
        if (!surfaces[i]->accept(new Move(surfaceState.x + state.x, surfaceState.y + state.y)))
        {
            state.x = rand() % 3 - 1;
            state.y = rand() % 3 - 1;
        }
    }
    ++mCount;
    return true;
}

// =======================================================================================

bool FreeScale::apply(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i)
    {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0)
            {
                ALOGE("Allocation state failed when FreeMove::Apply()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);
        if (mCount % mChangeFreq == 0)
        {
            state.w = rand() % 3 - 1;
            state.h = rand() % 3 - 1;
        }
        ALOGD("App::run() - dice_w:%d dice_w:%d", state.w, state.h);
        if (!surfaces[i]->accept(new Scale(surfaceState.w + state.w, surfaceState.h + state.h)))
        {
            state.w = rand() % 3 - 1;
            state.h = rand() % 3 - 1;
        }
    }
    ++mCount;
    return true;
}

// =======================================================================================

bool FreeBlend::apply(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i)
    {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0)
            {
                ALOGE("Allocation state failed when FreeMove::Apply()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);
        if (mCount % mChangeFreq == 0)
        {
            state.alpha = (rand() % 3 - 1) / 10.f;
        }
        ALOGD("App::run() - alpha:%f", state.alpha);
        if (!surfaces[i]->accept(new Blend(surfaceState.alpha + state.alpha)))
        {
            state.alpha = (rand() % 3 - 1) / 10.f;
        }
    }
    ++mCount;
    return true;
}

// =======================================================================================

bool Shuffle::apply(const Vector<sp<TestSurface> >& surfaces) {
    if (mStates.size() != surfaces.size()) {
        if (mStates.resize(surfaces.size()) < 0) {
            ALOGE("Resizing mState failed when FreeShuffle::apply()");
            return false;
        }
    }

    for (uint32_t i = 0; i < surfaces.size(); ++i)
    {
        uint32_t chosen = 0;
        if (mCount % mChangeFreq == 0 && mStates[i].zOrder == false)
        {
            chosen = rand() % mStates.size();
            if (mStates[chosen].zOrder == false)
            {
                int32_t aZOrder = surfaces[chosen]->getState().zOrder;
                int32_t bZOrder = surfaces[i]->getState().zOrder;
                ALOGD("App::run() - i:%d chosen:%d  chosenZ:%d z:%d", i, chosen, bZOrder, aZOrder);
                surfaces[chosen]->accept(new SetZOrder(bZOrder));
                surfaces[i]->accept(new SetZOrder(aZOrder));

                mStates.editItemAt(chosen).zOrder = true;
                mStates.editItemAt(i).zOrder = true;
            }
        }
    }

    for (uint32_t i = 0; i< mStates.size(); ++i)
        mStates.editItemAt(i).zOrder = false;

    ++mCount;
    return true;
}

// =======================================================================================

bool ClockwiseMove::apply(const Vector<sp<TestSurface> >& surfaces) {
    for (uint32_t i = 0; i < surfaces.size(); ++i)
    {
        const TestSurface::State& surfaceState = surfaces[i]->getState();
        ssize_t statesIdx = mStates.indexOfKey(surfaces[i].get());

        if (NAME_NOT_FOUND == statesIdx) {
            statesIdx = mStates.add(surfaces[i].get(), TestSurface::State());
            if (statesIdx < 0)
            {
                ALOGE("Allocation state failed when FreeMove::Apply()");
                return false;
            }
        }

        TestSurface::State& state = mStates.editValueAt(statesIdx);

        if (mCount % mChangeFreq == 0)
        {
            if (state.x == 0 && state.y ==0)
                state.x = 1;
        }
        ALOGD("App::run() - dice_x:%d dice_x:%d", state.x, state.y);
        if (!surfaces[i]->accept(new Move(surfaceState.x + state.x, surfaceState.y + state.y)))
        {
            if (state.x == 1)
            {
                state.x = 0;
                state.y = 1;
            } else if (state.y == 1)
            {
                state.y = 0;
                state.x = -1;
            } else if (state.x == -1)
            {
                state.x = 0;
                state.y = -1;
            } else
            {
                state.y = 0;
                state.x = 1;
            }
        }
    }
    ++mCount;
    return true;
}
