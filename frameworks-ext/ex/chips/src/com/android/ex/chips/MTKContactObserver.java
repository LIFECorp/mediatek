package com.android.ex.chips;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;
import android.util.Log;

@SuppressLint("NewApi")
public class MTKContactObserver {
	static final private String TAG = "MTKRecipContactObserver";
	private Context mContext = null;
	static private final int DEINIT_DELAY = 3000; 
	static private int _IDIndex = 0;
	static private int CONTACT_IDIndex = 0;
	static private int DIRTYIndex = 0;
	static private int DELETEDIndex = 0;
	static private int VERSIONIndex = 0;
	static final private String sProjection[] = {
			RawContacts._ID,
			RawContacts.CONTACT_ID,
			RawContacts.DIRTY,
			RawContacts.DELETED,
			RawContacts.VERSION
			};
	static final private String sSelection = 
			RawContacts.DIRTY + "=? or " + 
			RawContacts.DELETED + "=?"
			;
	static final private String sSelectionArgs[] = {
			String.valueOf(1),String.valueOf(1)
			};
	private Cursor mCursor;
	private boolean mStateReady = false;
	private Set mPreDirtyContactSet = new HashSet();
	private Set mDirtyContactSet = new HashSet();
	private static HashMap<Long,DirtyContactEvent> sPreDirtyContactMap = new HashMap<Long,DirtyContactEvent>();
	private HandlerThread mQueryThread = null;
	private Handler mQueryHandler = null;
	private ContentObserver mObserver = null;
	private ContentResolver mResolver = null;
	private ArrayList<ContactListener> mListeners = new ArrayList<ContactListener>();
	private Runnable mInitRunnable = new Runnable() {
		@Override
		public void run() {
			// TODO Auto-generated method stub
			initState();
		}
	};
	private Runnable mDeInitRunnable = new Runnable() {
		@Override
		public void run() {
			// TODO Auto-generated method stub
			deInitState();
		}
	};
	public static interface ContactListener{
		void onContactChange(Set s);
	}
	public void addContactListener(ContactListener l) {
		Log.d(TAG,"addContactListener = " + l);
		mListeners.add(l);
		if (mListeners.size() == 1) {
			synchronized (mListeners) {
				if (mQueryHandler == null) {
					mQueryThread = new HandlerThread(TAG);
					mQueryThread.start();
					mQueryHandler = new Handler(mQueryThread.getLooper());
				}
				mQueryHandler.removeCallbacks(mDeInitRunnable);
				mQueryHandler.post(mInitRunnable);
			}
		}	
	}
	public void removeContactListener(ContactListener l) {
		Log.d(TAG,"removeContactListener = " + l);
		mListeners.remove(l);
		if (mListeners.size() == 0) {
			synchronized (mListeners) {
				mQueryHandler.postDelayed(mDeInitRunnable, DEINIT_DELAY);
			}
		}
	}
	
	final public static class DirtyContactEvent {
	static final public int DELETE = 0;
	static final public int UPDATE = 1;
	static final public int ADD = 2;
	static final String action[] ={"delete", "update", "add"};
	int eventType;
	long _ID;
	long CID;
	int dirty;
	int delete;
	int version;
	public DirtyContactEvent(long _id, long cid, int dt, int dl, int v) {
		_ID = _id;
		CID = cid;
		dirty = dt;
		delete = dl;
		version = v;
		if (dl == 1) {
			eventType = DELETE;
			//recover CID
			if (sPreDirtyContactMap.containsKey(_ID)) {
				DirtyContactEvent e = sPreDirtyContactMap.get(_ID);
				CID = e.CID;
			}
		} else {
			eventType = UPDATE;
			if(!sPreDirtyContactMap.containsKey(_ID)) {
				eventType = ADD;
			}
		}
		
	}
	public void update(int et ,int dt, int dl, int v) {
		eventType = et;
		dirty = dt;
		delete = dl;
		version = v;
	}
	@Override
	public boolean equals(Object o) {
		// TODO Auto-generated method stub
		return hashCode()==o.hashCode();
	}
	@Override
	public int hashCode() {
		// TODO Auto-generated method stub
		int r = ((int)_ID)<<12|version|dirty<<11|delete<<10;
		return r;
	}
	@Override
	public String toString() {
		// TODO Auto-generated method stub
		String item = action[eventType]+"_ID"+_ID+"CID"+CID+"dt"+dirty+"dl"+delete+"v"+version;
		return item;
	}	
}
	void deInitState() {
		Log.d(TAG,"deInitState");
		mStateReady = false;
		mResolver.unregisterContentObserver(mObserver);
		mObserver = null;	
		mResolver = null;
		mCursor = null;
    	mPreDirtyContactSet.clear();
    	sPreDirtyContactMap.clear();
    	mDirtyContactSet.clear();
    	synchronized (mListeners) {
			mQueryThread.getLooper().quit();
	    	mQueryThread = null;
	    	mQueryHandler = null;
		}
    	
	}
	void initState() {
		if (mObserver == null){
			synchronized (mListeners) {
				mObserver = new ContentObserver(mQueryHandler) {
					@Override
					public void onChange(boolean selfChange, Uri uri) {
						// TODO Auto-generated method stub
						super.onChange(selfChange, uri);
						contactChange(selfChange, uri);
					}
				};
			}
			mResolver = mContext.getContentResolver();
			mResolver.registerContentObserver(
	        		RawContacts.CONTENT_URI,
	        		true,
	        		mObserver);
		}
		mCursor = queryDirtyRawContact(null, null);
		initCursorIndex();
		try{
			while (mCursor.moveToNext()) {
				DirtyContactEvent e = getDirtyContactItem(mCursor);
	        	mPreDirtyContactSet.add(e);
	        	sPreDirtyContactMap.put(e._ID, e);
			}
		} finally {
			 mCursor.close();
		}
		mStateReady = true;
		Log.d(TAG,"initState");
	}
	void initCursorIndex() {
		_IDIndex = mCursor.getColumnIndex(RawContacts._ID);
        CONTACT_IDIndex = mCursor.getColumnIndex(RawContacts.CONTACT_ID);
        DELETEDIndex = mCursor.getColumnIndex(RawContacts.DELETED);
        DIRTYIndex = mCursor.getColumnIndex(RawContacts.DIRTY);
        VERSIONIndex = mCursor.getColumnIndex(RawContacts.VERSION);
	}
	Cursor queryDirtyRawContact(String selection, String[] selectionArgs) {
		Cursor cursor = mResolver.query(
				 		RawContacts.CONTENT_URI, 
				 		sProjection, 
				 		selection, 
				 		selectionArgs, 
				        null);
		Log.d(TAG,"queryDirtyRawContact cursor count = " + cursor.getCount());
		return cursor;
	}
	private void contactChange(boolean selfChange, Uri uri) {
		if (!mStateReady) return;	
		Log.d(TAG,"selfChange = "+ selfChange +", uri = " +uri);
		loadCurrentDirtyContact();
	}
    private void loadCurrentDirtyContact() {
    	mCursor = queryDirtyRawContact(null, null);
    	try {
			if(mCursor.getCount() < sPreDirtyContactMap.size()){
				Log.d(TAG,"loadCurrentDirtyContact1+++");
				HashSet<DirtyContactEvent> tmpDirtyContactSet = new HashSet<DirtyContactEvent>();
				mDirtyContactSet.clear();
				mPreDirtyContactSet.clear();
				while (mCursor.moveToNext()) {
					DirtyContactEvent e = getDirtyContactItem(mCursor);
					tmpDirtyContactSet.add(e);
					mPreDirtyContactSet.add(e);
				}
				Collection<DirtyContactEvent> oldSet = sPreDirtyContactMap.values();
				Iterator it = oldSet.iterator();
				while (it.hasNext()) {
					DirtyContactEvent e = (DirtyContactEvent)it.next();
					if(!tmpDirtyContactSet.contains(e)) {
						e.update(DirtyContactEvent.DELETE, 1, 1, e.version);
						mDirtyContactSet.add(e);
						Log.d(TAG,e.toString());
					}
				}
				Log.d(TAG,"loadCurrentDirtyContact1---");
				for (ContactListener l : mListeners) {
				 l.onContactChange(mDirtyContactSet);
			 	}
				sPreDirtyContactMap.clear();
				it = tmpDirtyContactSet.iterator();
				while (it.hasNext()) {
					DirtyContactEvent e = (DirtyContactEvent)it.next();
					sPreDirtyContactMap.put(e._ID, e);
				}
				return;
			}
			mDirtyContactSet.clear();
			Log.d(TAG,"loadCurrentDirtyContact2+++");
			while (mCursor.moveToNext()) {
				 DirtyContactEvent e = getDirtyContactItem(mCursor);
			     if (!mPreDirtyContactSet.contains(e)) {
			    	 mPreDirtyContactSet.add(e); 
			    	 mDirtyContactSet.add(e);
			    	 sPreDirtyContactMap.put(e._ID, e);
			    	 Log.d(TAG,e.toString());
			     }
			 }
			 Log.d(TAG,"loadCurrentDirtyContact2---");
			 for (ContactListener l : mListeners){
				 l.onContactChange(mDirtyContactSet);
			 }
		 } finally {
			 Log.d(TAG, "cursor position = " + mCursor.getPosition());
			 mCursor.close();
		 }
    }
	public MTKContactObserver(Context context) {
		mContext = context;
	}
	DirtyContactEvent getDirtyContactItem(Cursor c) {
		long _ID = c.getLong(_IDIndex);
        long contactID = c.getLong(CONTACT_IDIndex);
        int  delete = c.getInt(DELETEDIndex);
        int  dirty = c.getInt(DIRTYIndex);
        int  version = c.getInt(VERSIONIndex);
        DirtyContactEvent e = new DirtyContactEvent(_ID, contactID ,dirty ,delete ,version);
        return e;
	}

}

