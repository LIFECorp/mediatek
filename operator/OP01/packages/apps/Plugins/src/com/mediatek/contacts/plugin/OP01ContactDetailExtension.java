package com.mediatek.contacts.plugin;

import android.app.Activity;
import android.text.Editable;
import android.text.InputType;
import android.text.Selection;
import android.text.method.DialerKeyListener;
import android.util.Log;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.widget.EditText;

import com.mediatek.contacts.ext.ContactDetailExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;

public class OP01ContactDetailExtension extends ContactDetailExtension {
    private static final String TAG = "OP01ContactDetailExtension";
    
    @Override
    public String getCommand() {
        return ContactPluginDefault.COMMD_FOR_OP01;
    }
    
    @Override
    public void setMenu(ContextMenu menu, boolean isNotDirectoryEntry, int simId,
            boolean mOptionsMenuOptions, int delSim, int newSim, Activity activity,
            int removeAssociation, int menuAssociation, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return;
        }
    }
    
    @Override
    public String setSPChar(String string, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return null;
        }
        String result = string;
        Log.i(TAG, "[setSPChar] result : " + result);
        return result;
    }
    
    @Override
    public void setViewKeyListener(EditText fieldView, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return ;
        }
        Log.i(TAG, "[setViewKeyListener] fieldView : " + fieldView);
        if (fieldView != null) {
            fieldView.setKeyListener(SIMKeyListener.getInstance());
        } else {
            Log.e(TAG, "[setViewKeyListener]fieldView is null");
        }
    }

    @Override
    public String TextChanged(int inputType, Editable s, String phoneText, int location, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return null;
        }
        Log.i(TAG, "[TextChanged] inputType : " + inputType + " | s : " + s + " | phoneText : "
                + phoneText + " | location : " + location);
        if (inputType == InputType.TYPE_CLASS_PHONE) {
        	int index = phoneText.indexOf(",");
            if (index > -1) {
            	phoneText = s.replace(index, index + 1, "p").toString();
            } else if ((index = phoneText.indexOf(";")) > -1) {
            	phoneText = s.replace(index, index + 1, "w").toString();
            }
        }
        Log.i(TAG, "[TextChanged] return : " + phoneText);
        return phoneText;
    }
    
    @Override
    public boolean checkMenuItem(boolean mtkGeminiSupport, boolean hasPhoneEntry, boolean b, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return false;
        }
        return false;
    }
    
    @Override
    public void setMenuVisible(MenuItem associationMenuItem, boolean mOptionsMenuOptions,
            boolean isEnabled, String commd) {
    }
    
    @Override
    public String repChar(String phoneNumber, char pause, char p, char wait, char w, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return null;
        }
        phoneNumber = phoneNumber.replace(pause, p).replace(wait, w);
        Log.i(TAG, "phoneNumber : " + phoneNumber + " | PAUSE : " + 
                pause + " | p : " + p + " | WAIT : " + wait + " | w : " + w);
        return phoneNumber;
    }
    
    
    public static class SIMKeyListener extends DialerKeyListener {
        private static SIMKeyListener sKeyListener;
        /**
         * The characters that are used.
         * 
         * @see KeyEvent#getMatch
         * @see #getAcceptedChars
         */
        public static final char[] CHARACTERS = new char[] { '0', '1', '2',
            '3', '4', '5', '6', '7', '8', '9', '+', '*', '#','P','W','p','w',',',';'};

        @Override
        protected char[] getAcceptedChars() {
            return CHARACTERS;
        }

        public static SIMKeyListener getInstance() {
            if (sKeyListener == null) {
                sKeyListener = new SIMKeyListener();
            }
            return sKeyListener;
        }

    }
    
    @Override
    public boolean collapsePhoneEntries(String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return false;
        }
        Log.i(TAG, "[collapsePhoneEntries()]"); 
        return false;
    }
}
