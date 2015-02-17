package com.mediatek.MobileVideoConfig;

//file system
import static org.xmlpull.v1.XmlPullParser.END_DOCUMENT;
import static org.xmlpull.v1.XmlPullParser.END_TAG;
import static org.xmlpull.v1.XmlPullParser.START_TAG;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.android.internal.os.AtomicFile;

import android.content.BroadcastReceiver;
import android.content.IntentFilter;
//import com.android.internal.os;
import java.io.File;
import android.util.Xml;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;
import com.android.internal.util.FastXmlSerializer;


import libcore.io.IoUtils;
import android.os.storage.StorageManager;
import android.os.Environment;
import com.mediatek.storage.StorageManagerEx;

public class MVCfgReceiver extends BroadcastReceiver{
	
	private final static String TAG = "MVCfgReceiver";
	private final static String INTENT_SD_SWAP = "com.mediatek.SD_SWAP";
//	private final static String MV_CFG_FILE_DIR = "/data/data/com.mediatek.MobileVideoConfig/";
	private final static String MV_CFG_FILE_DIR = "/data/misc/mobilevideo";
	private final static String MV_CFG_FILE_NAME = "mobilevideocfg.xml";

	private static final String TAG_ROOT = "root";
	private static final String TAG_ITEM = "item";
	private static final String ATTR_SD_DIR = "sd";
	private static final String ATTR_EXT_SD_DIR = "extsd";
	private static final String ATTR_APN = "apn";

	
	private String mSDCard1;
	private String mSDCard2;
	private String mAPN = "1";//ENetworkMultiAPN_MTK, defined by vendor

	
	private File mFilePath = new File(MV_CFG_FILE_DIR, MV_CFG_FILE_NAME);
	private AtomicFile mConfigFile;
	private static StorageManager mStorageManagerHandle = null;

	private Context mContext;


	@Override
	public void onReceive(Context context,Intent intent){

		mContext = context;

		String action = intent.getAction();
		
		Log.i(TAG, "onReceive receive intent: " + action);
		Log.i(TAG, "mFilePath: " + mFilePath.getPath());

		mConfigFile = new AtomicFile(mFilePath);

		if(action.equals(Intent.ACTION_BOOT_COMPLETED)||
			action.equals(INTENT_SD_SWAP)||
			action.equals(Intent.ACTION_MEDIA_UNMOUNTED)||
			action.equals(Intent.ACTION_MEDIA_MOUNTED)||
			action.equals(Intent.ACTION_MEDIA_EJECT)){

			synchronized(mFilePath)
			{
				mStorageManagerHandle = (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
				
				updateSDCardDir();

				if(action.equals(INTENT_SD_SWAP))
				{
					if(!isConfigFileExist()){
						Log.e(TAG, "config file is not exist but supposed to be");
					}
				}
				if(writeConfigFile()){
					Log.i(TAG, "write to the file success");
				
				}else{
					Log.e(TAG, "write to the file fail");
				
				}
			}
			
		}

	}

	private boolean isConfigFileExist(){

		return mFilePath.exists();	
	}

	private void updateSDCardDir(){

		if(mStorageManagerHandle == null){
			Log.e(TAG, "mStorageManagerHandle is null but not supposed to be");
			return;
		}
	
		mSDCard1 = StorageManagerEx.getInternalStoragePath();
        if (mSDCard1 != null) {
            mSDCard1 = mSDCard1 + "/";
        } else {
    	  mSDCard1 = "";
        }
		mSDCard2 = StorageManagerEx.getExternalStoragePath();
                Log.i(TAG, "mSDCard2 = " + mSDCard2);
		if(mSDCard2!=null){
			//mSDCard2 = mSDCard2 + "/";
			if(!mStorageManagerHandle.getVolumeState(mSDCard2).equals(Environment.MEDIA_MOUNTED)){

				Log.i(TAG, "external SD is not mounted");
				mSDCard2 = "";
			}else {
				mSDCard2 = mSDCard2 + "/";
			}

		} else {
			  mSDCard2 = "";
		}
		
		Log.i(TAG, "dir updated" + "mSDCard1 is" + mSDCard1 + ";" + "mSDCard2 is" + mSDCard2);

	}

	private boolean writeConfigFile(){

		Log.i(TAG, "writeConfigFile");
		
		FileOutputStream outfile = null;
		try {
			outfile = mConfigFile.startWrite();
		
			XmlSerializer out = new FastXmlSerializer();
			out.setOutput(outfile, "utf-8");
		
			out.startDocument(null, true);
			out.startTag(null, TAG_ROOT); {
				out.startTag(null, TAG_ITEM); {
					out.attribute(null, ATTR_SD_DIR, mSDCard1);
					out.attribute(null, ATTR_EXT_SD_DIR, mSDCard2);
					out.attribute(null, ATTR_APN, mAPN);
				}out.endTag(null, TAG_ITEM);
			} out.endTag(null, TAG_ROOT);		
			out.endDocument();
			mConfigFile.finishWrite(outfile);
			Log.e(TAG, "write finish!");
			
		} catch (IOException e) {
		
			if (outfile != null) {
				mConfigFile.failWrite(outfile);
				Log.e(TAG, "write fail!");
				return false;
			}
		}

		if(!mFilePath.setReadable(true, false)){
			Log.e(TAG, "setReadable fail!");
		}

		return true;
    }

}



