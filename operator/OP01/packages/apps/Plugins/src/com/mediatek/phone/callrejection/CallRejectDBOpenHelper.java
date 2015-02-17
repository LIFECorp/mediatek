package com.mediatek.phone.callrejection;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteOpenHelper;
  
public class CallRejectDBOpenHelper extends SQLiteOpenHelper {  
  
    public CallRejectDBOpenHelper(Context context, String name, CursorFactory factory,  
            int version) {  
        super(context, name, factory, version);  
    }    
  
    public CallRejectDBOpenHelper(Context context, String name, int version) {  
        this(context, name, null, version);  
    }  
      
    /** 
     * create db
     */  
    @Override  
    public void onCreate(SQLiteDatabase db) {  
        db.execSQL("create table " + CallRejectContentData.UserTableData.TABLE_NAME  
                + "(" + CallRejectContentData.UserTableData._ID  
                + " INTEGER PRIMARY KEY autoincrement,"  
                + CallRejectContentData.UserTableData.NUMBER + " varchar(20),"  
                + CallRejectContentData.UserTableData.TYPE + " varchar(1),"  
                + CallRejectContentData.UserTableData.NAME + " varchar(20))"  
                + ";");  
    }  
  
    @Override  
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {  
  
    }  
}  

