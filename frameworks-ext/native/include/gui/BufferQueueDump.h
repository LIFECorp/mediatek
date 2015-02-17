/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_GUI_BUFFERQUEUEDUMP_H
#define ANDROID_GUI_BUFFERQUEUEDUMP_H

#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <utils/KeyedVector.h>

#include <gui/mediatek/RingBuffer.h>

namespace android {
// ----------------------------------------------------------------------------

#ifdef USE_DP

class BufferQueue;
class AcquiredBuffer;
class BackupBuffer;
class BackupBufPusher;
class BackupBufDumper;


// class for BufferQueue backup and dump utils impl
class BufferQueueDump : public LightRefBase<BufferQueueDump> {
private:
    String8 mName;

    // RingBuffer utils for buffer backup storage
    RingBuffer< sp<BackupBuffer> > mBackupBuf;
    sp<BackupBufPusher> mBackupBufPusher;
    sp<BackupBufDumper> mBackupBufDumper;
    bool mIsBackupBufInited;

    // keep reference for 
    DefaultKeyedVector< uint32_t, sp<AcquiredBuffer> > mAcquiredBufs;
    sp<AcquiredBuffer> mLastBuf;

    // update buffer into back up
    void updateBuffer(int slot);

    // check backup depth setting, and reset length if changed
    int checkBackupCount();

public:
    BufferQueueDump()
        : mName("unnamed BufferQueueDump")
        , mIsBackupBufInited(false)
        , mAcquiredBufs(NULL) {}

    // name for this dump
    void setName(const String8& name);

    // trigger the dump process
    void dumpBuffer();

    // related functions into original BufferQueue APIs
    void onAcquireBuffer(int slot, const sp<GraphicBuffer>& buffer, const sp<Fence>& fence, int64_t timestamp);
    void onReleaseBuffer(int slot);
    void onFreeBuffer(int slot);
    
    // generate path for file dump
    static void getDumpFileName(String8& fileName, const String8& name);
};


// implement of buffer push
class BackupBufPusher : public RingBuffer< sp<BackupBuffer> >::Pusher {
public:
    BackupBufPusher(RingBuffer< sp<BackupBuffer> >& rb) :
        RingBuffer< sp<BackupBuffer> >::Pusher(rb) {}

    // the main API to implement
    virtual bool push(const sp<BackupBuffer>& in);
};


// implement of buffer dump
class BackupBufDumper : public RingBuffer< sp<BackupBuffer> >::Dumper {
private:
    String8 mName;

public:
    BackupBufDumper(RingBuffer< sp<BackupBuffer> >& rb)
        : RingBuffer< sp<BackupBuffer> >::Dumper(rb)
        , mName("unnamed BackupBufDumper") {}

    void setName(const String8& name) { mName = name; }

    // the main API to implement
    virtual void dump();
};


// struct of record for acquired buffer
class AcquiredBuffer : public LightRefBase<AcquiredBuffer> {
public:
    AcquiredBuffer(const sp<GraphicBuffer> buffer = NULL,
                   const sp<Fence>& fence = Fence::NO_FENCE,
                   int64_t timestamp = 0)
        : mGraphicBuffer(buffer)
        , mFence(fence)
        , mTimeStamp(timestamp) {}

    sp<GraphicBuffer> mGraphicBuffer;
    sp<Fence> mFence;
    int64_t mTimeStamp;

    void dump(const String8& prefix);
};


// struct of record for backup buffer
class BackupBuffer : public LightRefBase<BackupBuffer> {
public:
    BackupBuffer(const sp<GraphicBuffer> buffer = NULL,
                 nsecs_t timestamp = 0)
        : mGraphicBuffer(buffer)
        , mTimeStamp(timestamp)
        , mSourceHandle(NULL) {}

    sp<GraphicBuffer> mGraphicBuffer;
    nsecs_t mTimeStamp;
    const void* mSourceHandle;
};

#else // nodef USE_DP

// dummy implement for no dpframework
// the public member must be consistent with original implement
class BufferQueueDump : public LightRefBase<BufferQueueDump> {
public:
    BufferQueueDump() {};
    void setName(const String8& name) {};
    void dumpBuffer() {};
    void onAcquireBuffer(int slot, const sp<GraphicBuffer>& buffer, const sp<Fence>& fence, int64_t timestamp) {};
    void onReleaseBuffer(int slot) {};
    void onFreeBuffer(int slot) {};
};

#endif // USE_DP

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_BUFFERQUEUEDUMP_H
