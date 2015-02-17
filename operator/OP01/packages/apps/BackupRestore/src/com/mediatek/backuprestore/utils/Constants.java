package com.mediatek.backuprestore.utils;

public class Constants {
    public static final String BACKUP = "backup";
    public static final String RESTORE = "restore";

    public static final String BACKUP_FILE_EXT = "zip";
    public static final String BACKUP_FOLDER_NAME = ".backup";
    public static final String BACKUP_XML = "backup.xml";
    public static final String SETTINGINFO = "setttings";

    public static final int TIME_SLEEP_WHEN_COMPOSE_ONE = 200;

    public static final String KEY_SAVED_DATA = "data";
    // public static final String SDCARD2 = "%/mnt/sdcard2/%";

    public static final String ANDROID = "Android ";
    public static final String DATE = "date";
    public static final String SIZE = "size";
    public static final String FILE = "file";
    public static final String FILENAME = "filename";
    public static final String ITEM_TEXT = "text";
    public static final String ITEM_NAME = "name";
    public static final String ITEM_RESULT = "result";
    public static final String ITEM_PACKAGENAME = "packageName";
    public static final String RESULT_KEY = "result";
    public static final String INTENT_SD_SWAP = "com.mediatek.SD_SWAP";
    public static final String ACTION_SD_EXIST = "SD_EXIST";
    public static final String SCAN_RESULT_KEY_PERSONAL_DATA = "personalData";
    public static final String SCAN_RESULT_KEY_OLD_DATA = "oldData";
    public static final String SCAN_RESULT_KEY_APP_DATA = "appData";

    public static final String URI_CALENDAR_IMPORTER_EVENTS = "content://com.mediatek.calendarimporter/events";
    public static final String URI_MMS_SMS = "content://mms-sms/conversations/";
    public static final String URI_MMS = "content://mms/";
    public static final String URI_SMS = "content://sms";
    public static final String URI_NOTEBOOK = "content://com.mediatek.notebook.NotePad/notes";

    public static final int NUMBER_IMPORT_CONTACTS_ONE_SHOT = 1500;
    public static final int NUMBER_IMPORT_CONTACTS_EACH = 480;
    public static final int NUMBER_IMPORT_MMS_EACH = 10;
    public static final int NUMBER_IMPORT_SMS_EACH = 40;
    public static final int NUMBER_SEND_BROCAST_MUSIC_EACH = 10;

    public static final String MESSAGE_BOX_TYPE_INBOX = "1";
    public static final String MESSAGE_BOX_TYPE_SENT = "2";
    public static final String MESSAGE_BOX_TYPE_DRAFT = "3";
    public static final String MESSAGE_BOX_TYPE_OUTBOX = "4";

    public static final String PERSON_DATA = "PersonData";
    public static final String APP_DATA = "AppData";

    public static final int RESULT_PERSON_DATA = 100;
    public static final int RESULT_APP_DATA = 200;
    public static final int RESULT_RESTORE_APP = 300;

    public static final int REPLACE_DATA = 1;
    public static final int STARTFORGROUND = 1;
    
    public static final String ONLY_APP = "App";
    public static final String APP_AND_DATA = "App + Data";
    public static final String DATA_TITLE = "title";

    public class ModulePath {
        public static final String FOLDER_APP = "App";
        public static final String FOLDER_DATA = "Data";
        public static final String FOLDER_CALENDAR = "Calendar";
        public static final String FOLDER_TEMP = "temp";
        public static final String FOLDER_CONTACT = "Contact";
        public static final String FOLDER_MMS = "Mms";
        public static final String FOLDER_SMS = "Sms";
        public static final String FOLDER_MUSIC = "Music";
        public static final String FOLDER_PICTURE = "Picture";
        public static final String FOLDER_NOTEBOOK = "Notebook";

        public static final String NAME_CALENDAR = "calendar.vcs";
        public static final String NAME_CONTACT = "contact.vcf";
        public static final String NAME_MMS = "mms";
        public static final String NAME_SMS = "sms";

        public static final String FILE_EXT_APP = ".apk";
        // public static final String FILE_EXT_CALENDAR = ".vcs";
        // public static final String FILE_EXT_CONTACT = ".vcf";
        public static final String FILE_EXT_PDU = ".pdu";

        public static final String ALL_APK_FILES = ".*\\.apk";
        public static final String SCHEMA_ALL_APK = "apps/.*\\.apk";
        public static final String SCHEMA_ALL_CALENDAR = "calendar/calendar[0-9]+\\.vcs";
        public static final String SCHEMA_ALL_CONTACT = "contacts/contact[0-9]+\\.vcf";
        // public static final String SCHEMA_ALL_MMS = "mms/[0-9]+\\.pdu";
        public static final String SCHEMA_ALL_SMS = "sms/sms[0-9]+";
        public static final String SCHEMA_ALL_MUSIC = "music/.*";
        public static final String SCHEMA_ALL_PICTURE = "picture/.*";

        public static final String PICTUREZIP = "picture.zip";
        public static final String MUSICZIP = "music.zip";

        public static final String SMS_VMSG = "sms.vmsg";
        public static final String MMS_XML = "mms_backup.xml";
        public static final String NOTEBOOK_XML = "notebook.xml";

    }

    public class DialogID {
        public static final int DLG_RESTORE_CONFIRM = 2000;
        public static final int DLG_SDCARD_REMOVED = 2001;
        public static final int DLG_SDCARD_FULL = 2002;
        public static final int DLG_RESULT = 2004;
        public static final int DLG_LOADING = 2005;
        public static final int DLG_DELETE_AND_WAIT = 2006;
        public static final int DLG_NO_SDCARD = 2007;
        public static final int DLG_CANCEL_CONFIRM = 2008;
        public static final int DLG_CONTACT_CONFIG = 2009;
        public static final int DLG_EDIT_FOLDER_NAME = 2010;
        public static final int DLG_CREATE_FOLDER_FAILED = 2011;
        public static final int DLG_BACKUP_CONFIRM_OVERWRITE = 2012;
    }

    public class MessageID {
        public static final int PRESS_BACK = 0X501;
        public static final int SCANNER_FINISH = 0X502;
    }

    public class State {
        public static final int INIT = 0X00;
        public static final int RUNNING = 0X01;
        public static final int PAUSE = 0X02;
        public static final int CANCEL_CONFIRM = 0X03;
        public static final int CANCELLING = 0X04;
        public static final int FINISH = 0X05;
        public static final int ERR_HAPPEN = 0X06;
    }

    public class LogTag {
        public static final String LOG_TAG = "BackRestoreLogTag";
        public static final String CONTACT_TAG = "contact";
        public static final String MESSAGE_TAG = "message";
        public static final String MUSIC_TAG = "music";
        public static final String NOTEBOOK_TAG = "notebook";
        public static final String PICTURE_TAG = "picture";
        public static final String SMS_TAG = "sms";
        public static final String MMS_TAG = "mms";
        public static final String BACKUP_ENGINE_TAG = "backupEngine";
    }

    public class ContactType {
        public static final String ALL = "all";
        public static final String PHONE = "phone";
        public static final String SIM1 = "sim1";
        public static final String SIM2 = "sim2";
        public static final int SIMID_PHONE = -1;
        public static final int SIMID_SIM1 = 0;
        public static final int SIMID_SIM2 = 1;
        public static final int SIMID_SIM3 = 2;
        public static final int SIMID_SIM4 = 3;
        public static final int DEFAULT = 100;
    }

}
