package com.mediatek.backuprestore;

import com.mediatek.backuprestore.utils.ModuleType;

public class PersonalItemData {

    private int mType;
    private boolean mIsEnable;
    private int mDataCount;

    public PersonalItemData(int type, boolean isEnable) {
        mType = type;
        mIsEnable = isEnable;
    }

    public PersonalItemData(int type, boolean isEnable, int count) {
        mType = type;
        mIsEnable = isEnable;
        mDataCount = count;
    }

    public int getType() {
        return mType;
    }

    public int getDataCount() {
        return mDataCount;
    }

    public int getIconId() {
        int ret = ModuleType.TYPE_INVALID;
        switch (mType) {
        case ModuleType.TYPE_CONTACT:
            ret = R.drawable.ic_contact;
            break;

        case ModuleType.TYPE_MESSAGE:
            ret = R.drawable.ic_message;
            break;

        case ModuleType.TYPE_PICTURE:
            ret = R.drawable.ic_picture;
            break;
        case ModuleType.TYPE_CALENDAR:
            ret = R.drawable.ic_canlendar;
            break;

        case ModuleType.TYPE_MUSIC:
            ret = R.drawable.ic_music;
            break;

        case ModuleType.TYPE_NOTEBOOK:
            ret = R.drawable.ic_notebook;
            break;
        case ModuleType.TYPE_APP:
            ret = R.drawable.ic_application;
            break;

        default:
            break;
        }
        return ret;
    }

    public int getTextId() {
        int ret = ModuleType.TYPE_INVALID;
        switch (mType) {
        case ModuleType.TYPE_CONTACT:
            ret = R.string.contact_module;
            break;

        case ModuleType.TYPE_MESSAGE:
            ret = R.string.message_module;
            break;

        case ModuleType.TYPE_PICTURE:
            ret = R.string.picture_module;
            break;
        case ModuleType.TYPE_CALENDAR:
            ret = R.string.calendar_module;
            break;

        case ModuleType.TYPE_MUSIC:
            ret = R.string.music_module;
            break;

        case ModuleType.TYPE_NOTEBOOK:
            ret = R.string.notebook_module;
            break;

        case ModuleType.TYPE_APP:
            ret = R.string.app_module;
            break;

        default:
            break;
        }
        return ret;
    }

    public boolean isEnable() {
        return mIsEnable;
    }

    public void setEnable(boolean enable) {
        mIsEnable = enable;
    }
}
