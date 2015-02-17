package com.mediatek.backuprestore.utils;

import android.content.Context;

import com.mediatek.backuprestore.R;
import com.mediatek.backuprestore.modules.CalendarRestoreComposer;
import com.mediatek.backuprestore.modules.Composer;
import com.mediatek.backuprestore.modules.ContactRestoreComposer;
import com.mediatek.backuprestore.modules.MessageRestoreComposer;
import com.mediatek.backuprestore.modules.MusicRestoreComposer;
import com.mediatek.backuprestore.modules.NoteBookRestoreComposer;
import com.mediatek.backuprestore.modules.PictureRestoreComposer;

public class ModuleType {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/ModuleType";
    public static final int TYPE_INVALID = 0x0;
    public static final int TYPE_CONTACT = 0x1;
    public static final int TYPE_SMS = 0x2;
    public static final int TYPE_MMS = 0x4;
    public static final int TYPE_CALENDAR = 0x8;
    public static final int TYPE_APP = 0x10;
    public static final int TYPE_PICTURE = 0x20;
    public static final int TYPE_MESSAGE = 0x40;
    public static final int TYPE_MUSIC = 0x80;
    public static final int TYPE_NOTEBOOK = 0x100;

    public static String getModuleStringFromType(Context context, int type) {
        int resId = 0;
        switch (type) {
        case ModuleType.TYPE_CONTACT:
            resId = R.string.contact_module;
            break;

        case ModuleType.TYPE_MESSAGE:
            resId = R.string.message_module;
            break;

        case ModuleType.TYPE_CALENDAR:
            resId = R.string.calendar_module;
            break;

        case ModuleType.TYPE_PICTURE:
            resId = R.string.picture_module;
            break;

        case ModuleType.TYPE_APP:
            resId = R.string.app_module;
            break;

        case ModuleType.TYPE_MUSIC:
            resId = R.string.music_module;
            break;

        case ModuleType.TYPE_NOTEBOOK:
            resId = R.string.notebook_module;
            break;
        default:
            break;
        }
        MyLogger.logD(CLASS_TAG, "getModuleStringFromType: resId = " + resId);
        return context.getResources().getString(resId);
    }

    /*
     * private ModuleType() { }
     */

    public static Composer getModuleRestoreComposerFromType(Context context, int type) {
        Composer composer = null;
        switch (type) {
        case ModuleType.TYPE_CONTACT:
            composer = new ContactRestoreComposer(context);
            break;

        case ModuleType.TYPE_CALENDAR:
            composer = new CalendarRestoreComposer(context);
            break;

        case ModuleType.TYPE_MESSAGE:
            composer = new MessageRestoreComposer(context);
            break;

        case ModuleType.TYPE_MUSIC:
            composer = new MusicRestoreComposer(context);
            break;

        case ModuleType.TYPE_NOTEBOOK:
            composer = new NoteBookRestoreComposer(context);
            break;

        case ModuleType.TYPE_PICTURE:
            composer = new PictureRestoreComposer(context);
            break;

        default:
            break;
        }
        MyLogger.logD(CLASS_TAG, "getModuleRestoreComposerFromType: type is " + type + ", composer is " + composer);
        return composer;
    }

    // To compatible with old backup data.

    /*
     * public static Composer getOldModuleRestoreComposerFromType(Context
     * context, int type) { Composer composer = null; switch (type) { case
     * ModuleType.TYPE_CONTACT: composer = new
     * OldContactRestoreComposer(context); break;
     * 
     * case ModuleType.TYPE_CALENDAR: composer = new
     * OldCalendarRestoreComposer(context); break;
     * 
     * case ModuleType.TYPE_MESSAGE: String temp = "Mtk"; composer = new
     * MessageRestoreComposer(context, temp); break;
     * 
     * case ModuleType.TYPE_MUSIC: composer = new
     * OldMusicRestoreComposer(context); break;
     * 
     * case ModuleType.TYPE_NOTEBOOK: composer = new
     * NoteBookRestoreComposer(context); break;
     * 
     * case ModuleType.TYPE_PICTURE: composer = new
     * OldPictureRestoreComposer(context); break; case ModuleType.TYPE_APP:
     * composer = new OldAppRestoreComposer(context);
     * 
     * default: break; } MyLogger.logD(CLASS_TAG,
     * "getModuleRestoreComposerFromType: type is " + type + ", composer is " +
     * composer); return composer; }
     */
}
