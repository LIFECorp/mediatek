package com.mediatek.basic;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.util.Log;

import com.mediatek.teledongledemo.R;

public class Util {
	
	public final static String LOG_TAG = "3GD-Util";
	private ProgressDialog mProgressDialog;

	public static String quotedString(int resMsg) {

		return "\"" + resMsg + "\"";
	}
	
    public static enum SimState {
        UNKNOWN,
        ABSENT,
        PIN_REQUIRED,
        PUK_REQUIRED,
        NETWORK_LOCKED,
        READY,
        NOT_READY,
        PERM_DISABLED;

        public boolean isPinLocked() {
            return ((this == PIN_REQUIRED) || (this == PUK_REQUIRED));
        }

        public boolean isLocked() {
            return (this.isPinLocked() || (this == NETWORK_LOCKED));
        }


        public boolean iccCardExist() {
            return ((this == PIN_REQUIRED) || (this == PUK_REQUIRED)
                    || (this == NETWORK_LOCKED) || (this == READY)
                    || (this == PERM_DISABLED));
        }
    }
        
    public static class DataState {

        /** Data connection state: Unknown.  Used before we know the state.
         * @hide
         */
        public static final int DATA_UNKNOWN        = -1;
        /** Data connection state: Disconnected. IP traffic not available. */
        public static final int DATA_DISCONNECTED   = 0;
        /** Data connection state: Currently setting up a data connection. */
        public static final int DATA_CONNECTING     = 1;
        /** Data connection state: Connected. IP traffic should be available. */
        public static final int DATA_CONNECTED      = 2;
        /** Data connection state: Suspended. The connection is up, but IP
         * traffic is temporarily unavailable. For example, in a 2G network,
         * data activity may be suspended when a voice call arrives. */
        public static final int DATA_SUSPENDED      = 3;
    }
    
    public static enum ServiceState {}
    
	public static boolean writeFile(String filename, String msg) {

		return writeFile(filename, msg.getBytes());
	}

	public static boolean writeFile(String filename, byte[] buff) {

		OutputStream os = null;
		try {
			os = new FileOutputStream(filename, true);
			os.write(buff);
			os.write(10);
			return true;
		} catch (IOException e) {
		} finally {
			try {
				if (os != null) {
					os.close();
				}
			} catch (IOException e) {
			}
		}
		return false;
	}

	public static void removeFile(String filename) {

		File file = new File(filename);
		if (file.exists()) {
			Log.d(LOG_TAG, "delete = " + file.delete());
		}
	}


	public static String dumpByteArray(byte[] bytes, String seperator) {

		if (bytes == null || bytes.length == 0) {
			return "null";
		}

		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < bytes.length; ++i) {
			sb.append(String.format("%s%02x", seperator, bytes[i]));
		}

		return sb.substring(seperator.length());
	}
	
	public void showProcessDialog(String msg, boolean cancelable,
			DialogInterface.OnCancelListener listener) {
		String title = (cancelable) ? quotedString(R.string.title_dialog_cancel)
				: quotedString(R.string.title_dialog_wait);

		if (mProgressDialog != null) {
			mProgressDialog.setTitle(title);
			mProgressDialog.setMessage(msg);
			mProgressDialog.setCancelable(cancelable);
			mProgressDialog.show();
		} else {
			//mProgressDialog = ProgressDialog.show(getApplicationContect(), title, msg, true,
			//		cancelable, listener);
		}
	}

	public void showProcessDialog(int resMsg,
			DialogInterface.OnCancelListener listener) {
		showProcessDialog(quotedString(resMsg), (listener != null), listener);
	}

	public void showProcessDialog(String msg,
			DialogInterface.OnCancelListener listener) {
		showProcessDialog(msg, (listener != null), listener);
	}

	public void showProcessDialog(int resMsg, boolean cancelable) {
		showProcessDialog(quotedString(resMsg), cancelable, null);
	}

	public void showProcessDialog(String msg, boolean cancelable) {
		showProcessDialog(msg, cancelable, null);
	}

	public void dismissProcessDialog() {
		if (mProgressDialog != null) {
			if (mProgressDialog.isShowing()) {
				mProgressDialog.dismiss();
			}
			mProgressDialog = null;
		}
	}
}
