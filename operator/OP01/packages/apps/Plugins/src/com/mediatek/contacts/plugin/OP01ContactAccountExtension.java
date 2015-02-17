package com.mediatek.contacts.plugin;

import com.mediatek.contacts.ext.ContactAccountExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;

public class OP01ContactAccountExtension extends ContactAccountExtension {
    
    public boolean needNewDataKind(String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        return true;
    }

}
