/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <cutils/xlog.h>
#include <cutils/properties.h>

#include <gui/ISurfaceComposer.h>
#include <gui/BufferQueue.h>

#include "SurfaceFlinger.h"
#include "Layer.h"

#include "RenderEngine/RenderEngine.h"

#ifndef MTK_EMULATOR_SUPPORT
#include "ui/gralloc_extra.h"
#include "ui/GraphicBufferExtra.h"
#endif

#define ALIGN_CEIL(x,a) (((x) + (a) - 1L) & ~((a) - 1L))

namespace android {

void Layer::setBufferCount() {
    int num = mFlinger->getHwComposer().queryExtraBuffer();
    char value[PROPERTY_VALUE_MAX];
    property_get("ro.sf.triplebuf.disable", value, "0");
    if (atoi(value)) {
        ALOGW("triple buffer is disabled...");
        num += 2;
    } else {
        num += 3;
    }
    mSurfaceFlingerConsumer->setDefaultMaxBufferCount(num);
}

void Layer::drawProtectedImage(
    const sp<const DisplayDevice>& hw, const Region& clip) const
{
    const State& s(getDrawingState());
    const Transform tr(hw->getTransform() * s.transform);
    const uint32_t hw_h = hw->getHeight();
    Rect win(s.active.w, s.active.h);
    if (!s.active.crop.isEmpty()) {
        win.intersect(s.active.crop, &win);
    }

    int w = win.getWidth();
    int h = win.getHeight();
    if (w > h) {
        win.left += ((w - h) / 2);
        win.right = win.left + h;
    } else {
        win.top += ((h - w) / 2);
        win.bottom = win.top + w;
    }

    Mesh::VertexArray<vec2> position(mMesh.getPositionArray<vec2>());
    position[0] = tr.transform(win.left,  win.top);
    position[1] = tr.transform(win.left,  win.bottom);
    position[2] = tr.transform(win.right, win.bottom);
    position[3] = tr.transform(win.right, win.top);
    for (size_t i=0 ; i<4 ; i++) {
        position[i].y = hw_h - position[i].y;
    }

    Mesh::VertexArray<vec2> texCoords(mMesh.getTexCoordArray<vec2>());
    texCoords[0] = vec2(0, 0);
    texCoords[1] = vec2(0, 1);
    texCoords[2] = vec2(1, 1);
    texCoords[3] = vec2(1, 0);

    RenderEngine& engine(mFlinger->getRenderEngine());
    engine.setupLayerProtectedImage();
    engine.drawMesh(mMesh);
    engine.disableBlending();
}

// dump current using buffer in Layer
// not support EMU case now for using GrallocExtra directly
void Layer::dumpActiveBuffer() const {
#ifndef MTK_EMULATOR_SUPPORT
    char value[PROPERTY_VALUE_MAX];
    uint32_t layerdump;

    property_get("debug.sf.layerdump", value, "0");
    layerdump = strtoul(value, NULL, 16);

    // check which layer to dump
    // pointer match, or -1 to dump all
    if ((~0U != layerdump) && (uintptr_t(this) != layerdump))
        return;

    if (mActiveBuffer == NULL)
        return;

    XLOGV("[dumpActiveBuffer] + id=%p", this);

    snprintf(value, 10, "%p", this);
    GraphicBufferExtra::dump(mActiveBuffer, value, "/data/SF_dump");

    XLOGV("[dumpActiveBuffer] - id=%p", this);
#endif // MTK_EMULATOR_SUPPORT
}

};
