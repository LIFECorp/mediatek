
package com.mediatek.contacts.plugin;

import com.mediatek.contacts.ext.ContactAccountExtension;
import com.mediatek.contacts.ext.ContactDetailExtension;
import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.QuickContactExtension;

public class OP01ContactsPlugin extends ContactPluginDefault {

    public ContactAccountExtension createContactAccountExtension() {
        return new OP01ContactAccountExtension(); 
    }

    public ContactDetailExtension createContactDetailExtension() {
        return new OP01ContactDetailExtension();
    }

    public ContactListExtension createContactListExtension() {
        return new OP01ContactListExtension();
    }

    public QuickContactExtension createQuickContactExtension() {
        return new OP01QuickContactExtension();
    }
}
