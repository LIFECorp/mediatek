package com.mediatek.dialer.plugin;

import com.mediatek.dialer.ext.CallDetailExtension;
import com.mediatek.dialer.ext.DialerPluginDefault;
import com.mediatek.dialer.ext.DialPadExtension;
import com.mediatek.dialer.ext.DialtactsExtension;
import com.mediatek.dialer.ext.SpeedDialExtension;

public class OP01DialerPlugin extends DialerPluginDefault {

    public CallDetailExtension createCallDetailExtension() {
        return new OP01CallDetailExtension();
    }
	
    public DialPadExtension createDialPadExtension() {
        return new OP01DialPadExtension();
    }

    public DialtactsExtension createDialtactsExtension() {
        return new OP01DialtactsExtension();
    }

    public SpeedDialExtension createSpeedDialExtension() {
        return new OP01SpeedDialExtension();
    }
}
