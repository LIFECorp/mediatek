package com.mediatek.camera.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.media.CamcorderProfile;

import com.mediatek.op01.plugin.R;

import java.util.ArrayList;

public class Op01CameraExtension extends ContextWrapper implements ICameraExtension {
    private static final String TAG = "Op01CameraExtension";
    private static final boolean LOG = true;
    
    private Context mContext;
    private IFeatureExtension mFeatureExtension;
    private static final int MPEG4_CODEC = 3;
    private static final int H264_CODEC = 2;
    
    public Op01CameraExtension(Context context) {
        super(context);
        mContext = context;
    }
    
    @Override
    public IFeatureExtension getFeatureExtension() {
        if (mFeatureExtension == null) {
            mFeatureExtension = new Op01FeatureSet();
        }
        return mFeatureExtension;
    }

    private class Op01FeatureSet extends FeatureExtension {
        public boolean isDelayRestartPreview() {
            return true;
        }

        public void updateWBStrings(CharSequence[] entries) {
            CharSequence[] newStr = mContext.getResources().getTextArray(
                    R.array.pref_camera_whitebalance_entries_cmcc);
            int size = entries.length;
            for (int i = 0; i < size; i++) {
                entries[i] = newStr[i];
            }
        }

        public void updateSceneStrings(ArrayList<CharSequence> entries,
                ArrayList<CharSequence> entryValues) {
            //op01 will use "normal" mode, so here do nothing. 
        }

        public void checkMMSVideoCodec(int quality, CamcorderProfile profile) {
            if (profile.videoCodec != MPEG4_CODEC && profile.videoCodec != H264_CODEC) {
                profile.videoCodec = MPEG4_CODEC;
            }
        }
        
        public boolean isVideoBitOffSet() {
            return true;
        }
    }
}
