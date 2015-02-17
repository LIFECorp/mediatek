#define LOG_TAG "stresstool"
#include "TestSurface.h"
#include "Stretegy.h"
#include "App.h"
#include "Action.h"

using namespace android;

App::App() : mDelay(1E6), mActionFlag(0), mCount(0), mChangeFreq(60)
{
    srand(time(NULL));
}

App::~App()
{
}

void App::addSurface(sp<TestSurface> surface)
{
    mSurfaces.push_back(surface);
    mDiceStates.push_back(TestSurface::State());
    mDiceStates.editItemAt(mDiceStates.size() - 1).zOrder = false;
}

void App::addStretegy(sp<Stretegy> stretegy)
{
    mStretegys.push_back(stretegy);
}

void App::setActionFlag(const uint32_t& actionFlag)
{
    mMutexActionFlag.lock();
    {
        mActionFlag = actionFlag;
    }
    mMutexActionFlag.unlock();
}

uint32_t App::getActionFlag()
{
    uint32_t retValue;
    mMutexActionFlag.lock();
    {
        retValue = mActionFlag;
    }
    mMutexActionFlag.unlock();
    return retValue;
}

bool App::threadLoop()
{
    for (uint32_t i = 0; i < mStretegys.size(); ++i)
    {
        mStretegys[i]->apply(mSurfaces);
    }

    for (uint32_t i = 0; i < mSurfaces.size(); ++i)
    {
        const sp<TestSurface>& surface = mSurfaces[i];
        const TestSurface::Config& cfg = surface->getConfig();
        volatile const TestSurface::State& state = surface->getState();
        TestSurface::State& diceState = mDiceStates.editItemAt(i);

        /*
        uint32_t actionFlag = 0;
        mMutexActionFlag.lock();
        {
            actionFlag = mActionFlag;
        }
        mMutexActionFlag.unlock();
        ALOGD("App::run() - actionFlag:%d", actionFlag);

        if (mActionFlag & ACTION_JUMP)
        {
            if (mCount % mChangeFreq == 0)
            {
                if (diceState.x == 0 && diceState.y ==0)
                {
                    diceState.x = 1;
                }
            }
            ALOGD("App::run() - dice_x:%d dice_x:%d", diceState.x, diceState.y);
            if (!surface->accept(new Move(state.x + diceState.x, state.y + diceState.y)))
            {
                if (diceState.x == 1)
                {
                    diceState.x = 0;
                    diceState.y = 1;
                } else if (diceState.y == 1)
                {
                    diceState.x = 0;
                    diceState.y = -1;
                }
                else
                {
                    diceState.x = 0;
                    diceState.y = 1;
                }
            }
        }
           if (mActionFlag & ACTION_BUFFERCROP) {
           if (mCount % mChangeFreq == 0) {
           dice_x = rand() % 3 - 1;
           dice_y = rand() % 3 - 1;
           }
           ALOGD("App::run() - dice_buffer_crop_x:%d dice_x:%d", dice_x, dice_y);
           surface->accept(new Move(state.x + dice_x, state.y + dice_y));
           }
           */

        ALOGI("Play(%d) before prepare()", i);
        surface->prepare();
        ALOGI("Play(%d) after prepare()", i);
    }
    SurfaceComposerClient::openGlobalTransaction();
    for (uint32_t i = 0; i < mSurfaces.size(); ++i)
    {
        const sp<TestSurface>& surface = mSurfaces[i];
        ALOGI("Play(%d) before refresh()", i);
        surface->refresh();
        ALOGI("Play(%d) after refresh()", i);
    }
    SurfaceComposerClient::closeGlobalTransaction();

    for (uint32_t i = 0; i < mDiceStates.size(); ++i)
    {
        mDiceStates.editItemAt(i).zOrder = 0;
    }
    ++mCount;
    usleep(mDelay);

    return true;
}

void App::dump() const
{
}
