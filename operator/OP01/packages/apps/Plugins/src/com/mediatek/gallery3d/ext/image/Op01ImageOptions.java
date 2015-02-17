package com.mediatek.gallery3d.ext.image;

import com.mediatek.gallery3d.ext.IImageOptions;
import com.mediatek.gallery3d.ext.ImageOptions;

public class Op01ImageOptions extends ImageOptions implements IImageOptions {

    @Override
    public boolean shouldUseOriginalSize() {
        return true;
    }

}
