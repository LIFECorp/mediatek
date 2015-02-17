package com.mediatek.deskclock.plugin;

import android.content.Context;
import android.content.ContextWrapper;

import com.mediatek.deskclock.ext.ICMCCSpecialSpecExtension;

/**
 * M: Default implementation of Plug-in definition of Desk Clock.
 */
public class Op01CMCCSpecialSpecExtension extends ContextWrapper implements ICMCCSpecialSpecExtension {

    public Op01CMCCSpecialSpecExtension(Context ctx) {
        super(ctx);
    }

    /**
     * @return Return if CMCC spec
     */
    public boolean isCMCCSpecialSpec() {
        return true;
    }
}
