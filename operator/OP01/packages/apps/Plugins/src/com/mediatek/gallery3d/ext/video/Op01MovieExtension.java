package com.mediatek.gallery3d.ext.video;

import com.mediatek.gallery3d.ext.IMovieExtension;
import com.mediatek.gallery3d.ext.IMovieItem;
import com.mediatek.gallery3d.ext.IMovieStrategy;
import com.mediatek.gallery3d.ext.MovieExtension;
import com.mediatek.gallery3d.ext.MovieUtils;

import java.util.ArrayList;
import java.util.List;

public class Op01MovieExtension extends MovieExtension implements IMovieStrategy {
    private static final String TAG = "Op01MovieExtension";
    private static final boolean LOG = true;
    
    @Override
    public List<Integer> getFeatureList() {
        ArrayList<Integer> list = new ArrayList<Integer>();
        list.add(IMovieExtension.FEATURE_ENABLE_VIDEO_LIST);
        list.add(IMovieExtension.FEATURE_ENABLE_BOOKMARK);
        list.add(IMovieExtension.FEATURE_ENABLE_NOTIFICATION_PLUS);
        list.add(IMovieExtension.FEATURE_ENABLE_STEREO_AUDIO);
        list.add(IMovieExtension.FEATURE_ENABLE_STREAMING);
        return list;
    }
    
    @Override
    public boolean shouldEnableNMP(IMovieItem item) {
        boolean enable = false;
        if (!MovieUtils.isLocalFile(item.getUri(), item.getMimeType())) {
            enable = true;
        }
        return enable;
    }

    @Override
    public boolean shouldEnableCheckLongSleep() {
        return false;
    }

    @Override
    public boolean shouldEnableServerTimeout() {
        return true;
    }

    @Override
    public boolean shouldEnableRewindAndForward() {
        return false;
    }

    @Override
    public IMovieStrategy getMovieStrategy() {
        return this;
    }
}
