#ifndef MTK_GRALLOC_EXTRA_GRAPHIC_BUFFER_EXTRA_H
#define MTK_GRALLOC_EXTRA_GRAPHIC_BUFFER_EXTRA_H

#include <stdint.h>
#include <sys/types.h>

#include <system/window.h>

#include <utils/Singleton.h>
#include <utils/RefBase.h>

struct extra_device_t;
struct gralloc_buffer_info_t;


namespace android {
// ---------------------------------------------------------------------------
class GraphicBuffer;

class GraphicBufferExtra : public Singleton<GraphicBufferExtra>
{
public:
    static inline GraphicBufferExtra& get() { return getInstance(); }

    int getIonFd(buffer_handle_t handle, int *idx, int *num);

    int getBufInfo(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo);

    int getSecureBuffer(buffer_handle_t handle, int *type, int *hBuffer);

    int setBufParameter(buffer_handle_t handle, int mask, int value);

    int getMVA(buffer_handle_t handle, void ** mvaddr);

    int setBufInfo(buffer_handle_t handle, const char * str);

    /** @brief dump the GraphicBuffer
     *
     * @param prefix the prefix of file name
     * @param dir the stored directory, default is "/data/"
     */
    static void dump(const sp<GraphicBuffer> &gb, const char *prefix = "", const char *dir = "/data/");

    static float getBpp(int format);

	/** @return the expected buffer size by the given parameters.
	 * 
	 * @param width, height, format
	 * @return size in byte
	 */
	/** Currently the formulas of IMG and MALI are different.
	 *
	 * IMG:
	 * stride = ALIGN(w, 32);
	 * size = stride * h * bpp;
	 *
	 * MALI:
	 * tmp = ALIGN(w * bpp, 64);
	 * size = tmp * h;
	 * stride = tmp / bpp;
	 */
	static int computeSize(int width, int height, int format);

	/** so we need _stride for now...
	 * FIXME, please fix the gralloc module to align the function above.
	 */
	static int computeSize_stride(int width, int height, int _stride, int format);

private:
    friend class Singleton<GraphicBufferExtra>;

    GraphicBufferExtra();

    ~GraphicBufferExtra();

    extra_device_t *mExtraDev;

};

// ---------------------------------------------------------------------------
}; // namespace android

#endif // MTK_GRALLOC_EXTRA_GRAPHIC_BUFFER_EXTRA_H
