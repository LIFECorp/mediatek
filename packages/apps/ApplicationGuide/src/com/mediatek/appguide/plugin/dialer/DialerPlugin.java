package com.mediatek.appguide.plugin.dialer;

import com.mediatek.dialer.ext.ContactAccountExtension;
import com.mediatek.dialer.ext.DialerPluginDefault;

public class DialerPlugin extends DialerPluginDefault {

    public ContactAccountExtension createContactAccountExtension() {
        return new SwitchSimContactsExt(); 
    }

}
