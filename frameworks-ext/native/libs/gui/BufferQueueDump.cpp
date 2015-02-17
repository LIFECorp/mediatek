#define LOG_TAG "BufferQueue"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <gui/BufferQueue.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>

#include <ui/GraphicBufferExtra.h>

#include <gui/mediatek/BufferQueueDump.h>

// ----------------------------------------------------------------------------
#define PROP_DUMP_NAME      "debug.bq.dump"
#define PROP_DUMP_BUFCNT    "debug.bq.bufscnt"
#define DEFAULT_DUMP_NAME   "GOD'S IN HIS HEAVEN, ALL'S RIGHT WITH THE WORLD."
#define DEFAULT_BUFCNT      "0"
#define STR_DUMPALL         "dump_all"
#define PREFIX_NODUMP       "[:::nodump:::]"
#define DUMP_FILE_PATH      "/data/SF_dump/"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define BQD_LOGV(x, ...) XLOGV("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGD(x, ...) XLOGD("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGI(x, ...) XLOGI("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGW(x, ...) XLOGW("[%s] "x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGE(x, ...) XLOGE("[%s] "x, mName.string(), ##__VA_ARGS__)


namespace android {

// ----------------------------------------------------------------------------
void BufferQueueDump::setName(const String8& name) {
    mName = name;

    // update dumper's name
    if (mBackupBufDumper != NULL) {
        mBackupBufDumper->setName(name);
    }

    // check and reset current dump setting
    checkBackupCount();
}

void BufferQueueDump::dumpBuffer() {
    String8 name;
    String8 prefix;

    char value[PROPERTY_VALUE_MAX];
    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    // check if prefix for no dump
    if (strstr(value, PREFIX_NODUMP) == value) {
        goto check;
    }

    // check if not dump for me
    if (!((!strcmp(value, STR_DUMPALL)) || (-1 != mName.find(value)))) {
        goto check;
    }

    // at first, dump backup buffer
    if (mBackupBuf.getSize() > 0) {
        mBackupBuf.dump();
    }

    getDumpFileName(name, mName);

    // dump acquired buffers
    if (0 == mAcquiredBufs.size()) {
        // if no acquired buf, try to dump the last one kept
        if (mLastBuf != NULL) {
            prefix = String8::format("[%s](LAST_ts%lld)",
                                     name.string(), ns2ms(mLastBuf->mTimeStamp));
            mLastBuf->dump(prefix);

            BQD_LOGD("[dump] LAYER, handle(%p)", mLastBuf->mGraphicBuffer->handle);
        }
    } else {
        // dump acquired buf old to new
        for (uint32_t i = 0; i < mAcquiredBufs.size(); i++) {
            const sp<AcquiredBuffer>& buffer = mAcquiredBufs[i];
            if (buffer->mGraphicBuffer != NULL) {
                prefix = String8::format("[%s](ACQ%02u_ts%lld)",
                                         name.string(), i, ns2ms(buffer->mTimeStamp));
                buffer->dump(prefix);

                BQD_LOGD("[dump] ACQ:%02u, handle(%p)", i, buffer->mGraphicBuffer->handle);
            }
        }
    }

check:
    checkBackupCount();
}

void BufferQueueDump::getDumpFileName(String8& fileName, const String8& name) {
    fileName = name;

    // check file name, filter out invalid chars
    const char invalidChar[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};
    size_t size = fileName.size();
    char *buf = fileName.lockBuffer(size);
    for (unsigned int i = 0; i < ARRAY_SIZE(invalidChar); i++) {
        for (size_t c = 0; c < size; c++) {
            if (buf[c] == invalidChar[i]) {
                // find invalid char, replace it with '_'
                buf[c] = '_';
            }
        }
    }
    fileName.unlockBuffer(size);
}

int BufferQueueDump::checkBackupCount() {
    char value[PROPERTY_VALUE_MAX];
    char *name = value;
    int count;

    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    if (strstr(value, PREFIX_NODUMP) == value) {
        // find prefix for no dump, skip it
        name = &value[strlen(PREFIX_NODUMP)];
    }

    if ((!strcmp(name, STR_DUMPALL)) || (-1 != mName.find(name))) {
        property_get(PROP_DUMP_BUFCNT, value, DEFAULT_BUFCNT);
        count = atoi(value);
    } else {
        count = 0;
    }

    if (count > 0) {
        // create backup buffer if needed
        if (!mIsBackupBufInited) {
            mBackupBufPusher = new BackupBufPusher(mBackupBuf);
            mBackupBufDumper = new BackupBufDumper(mBackupBuf);
            if ((mBackupBufPusher != NULL) && (mBackupBufDumper != NULL)) {
                mBackupBufDumper->setName(mName);
                sp< RingBuffer< sp<BackupBuffer> >::Pusher > proxyPusher = mBackupBufPusher;
                sp< RingBuffer< sp<BackupBuffer> >::Dumper > proxyDumper = mBackupBufDumper;
                mBackupBuf.setPusher(proxyPusher);
                mBackupBuf.setDumper(proxyDumper);
                mIsBackupBufInited = true;
            } else {
                mBackupBufPusher.clear();
                mBackupBufDumper.clear();
                count = 0;
                BQD_LOGE("[%s] create Backup pusher or dumper failed", __func__);
            }
        }

        // resize backup buffer
        mBackupBuf.resize(count);
    } else {
        mBackupBuf.resize(0);
    }

    return count;
}

void BufferQueueDump::onAcquireBuffer(int slot, const sp<GraphicBuffer>& buffer, const sp<Fence>& fence, int64_t timestamp) {
    if (buffer == NULL) {
        return;
    }

    sp<AcquiredBuffer> v = mAcquiredBufs.valueFor(slot);
    if (v == NULL) {
        sp<AcquiredBuffer> b = new AcquiredBuffer(buffer, fence, timestamp);
        mAcquiredBufs.add(slot, b);
    } else {
        BQD_LOGW("[%s] slot(%d) acquired, seems to be abnormal, just update ...", __func__, slot);
        v->mGraphicBuffer = buffer;
        v->mFence = fence;
        v->mTimeStamp = timestamp;
    }
}

void BufferQueueDump::updateBuffer(int slot) {
    if (mBackupBuf.getSize() > 0) {
        const sp<AcquiredBuffer>& v = mAcquiredBufs.valueFor(slot);
        if (v != NULL) {
            // push GraphicBuffer into backup buffer if buffer ever acquired
            sp<BackupBuffer> buffer = new BackupBuffer(v->mGraphicBuffer, v->mTimeStamp);
            mBackupBuf.push(buffer);
        }
    }

    // keep for the last one before removed
    if (1 == mAcquiredBufs.size()) {
        mLastBuf = mAcquiredBufs[0];
    }
    mAcquiredBufs.removeItem(slot);
}

void BufferQueueDump::onReleaseBuffer(int slot) {
    updateBuffer(slot);
}

void BufferQueueDump::onFreeBuffer(int slot) {
    updateBuffer(slot);
}

// ----------------------------------------------------------------------------
bool BackupBufPusher::push(const sp<BackupBuffer>& in) {
    if ((in == NULL) || (in->mGraphicBuffer == NULL)) {
        return false;
    }

    sp<BackupBuffer>& buffer = editHead();

    // check property of GraphicBuffer, realloc if needed
    bool needCreate = false;
    if ((buffer == NULL) || (buffer->mGraphicBuffer == NULL)) {
        needCreate = true;
    } else {
        if ((buffer->mGraphicBuffer->width != in->mGraphicBuffer->width) ||
            (buffer->mGraphicBuffer->height != in->mGraphicBuffer->height) ||
            (buffer->mGraphicBuffer->format != in->mGraphicBuffer->format)) {
            needCreate = true;
            XLOGD("[%s] geometry changed, backup=(%d, %d, %d) => active=(%d, %d, %d)",
                __func__, buffer->mGraphicBuffer->width, buffer->mGraphicBuffer->height,
                buffer->mGraphicBuffer->format, in->mGraphicBuffer->width,
                in->mGraphicBuffer->height, in->mGraphicBuffer->format);
        }
    }

    if (needCreate) {
        sp<GraphicBuffer> newGraphicBuffer = new GraphicBuffer(
                                             in->mGraphicBuffer->width, in->mGraphicBuffer->height,
                                             in->mGraphicBuffer->format, in->mGraphicBuffer->usage);
        if (newGraphicBuffer == NULL) {
            XLOGE("[%s] alloc GraphicBuffer failed", __func__);
            return false;
        }

        if (buffer == NULL) {
            buffer = new BackupBuffer();
            if (buffer == NULL) {
                XLOGE("[%s] alloc BackupBuffer failed", __func__);
                return false;
            }
        }

        buffer->mGraphicBuffer = newGraphicBuffer;
    }

    int width = in->mGraphicBuffer->width;
    int height = in->mGraphicBuffer->height;
    int format = in->mGraphicBuffer->format;
    int usage = in->mGraphicBuffer->usage;
    int stride = in->mGraphicBuffer->stride;

    float bpp = GraphicBufferExtra::getBpp(format);
    status_t err;

    // backup
    void *src;
    void *dst;
    err = in->mGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &src);
    if (err != NO_ERROR) {
        XLOGE("[%s] lock GraphicBuffer failed", __func__);
        return false;
    }

    err = buffer->mGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dst);
    if (err != NO_ERROR) {
        in->mGraphicBuffer->unlock();
        XLOGE("[%s] lock backup buffer failed", __func__);
        return false;
    }

    memcpy(dst, src, stride * height * bpp);

    buffer->mGraphicBuffer->unlock();
    in->mGraphicBuffer->unlock();

    // update timestamp
    buffer->mTimeStamp = in->mTimeStamp;
    buffer->mSourceHandle = in->mGraphicBuffer->handle;

    return true;
}

// ----------------------------------------------------------------------------
void BackupBufDumper::dump() {
    String8 name;
    String8 prefix;

    BufferQueueDump::getDumpFileName(name, mName);

    for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
        const sp<BackupBuffer>& buffer = getItem(i);
        prefix = String8::format("[%s](REL%02u_H%p_ts%lld)",
                                 name.string(), i, buffer->mSourceHandle, ns2ms(buffer->mTimeStamp));
        GraphicBufferExtra::dump(buffer->mGraphicBuffer, prefix.string(), DUMP_FILE_PATH);

        BQD_LOGI("[dump] REL:%02u, handle(source=%p, backup=%p)",
            i, buffer->mSourceHandle, buffer->mGraphicBuffer->handle);
    }
}

// ----------------------------------------------------------------------------
void AcquiredBuffer::dump(const String8& prefix) {
    if (mFence != NULL) {
        mFence->waitForever(__func__);
    }
    GraphicBufferExtra::dump(mGraphicBuffer, prefix.string(), DUMP_FILE_PATH);
}

}
