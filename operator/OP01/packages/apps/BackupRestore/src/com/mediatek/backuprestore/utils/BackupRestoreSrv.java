/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.backuprestore.utils;

import android.net.*;
import java.io.*;

import android.util.Log;

public class BackupRestoreSrv{
	private static final String TAG = "BackupRestoreSrv";

    private BackupRestoreSrvClient mSrvClient;

	public BackupRestoreSrv()
	{
        mSrvClient = BackupRestoreSrvClient.getInstence();;
	}
	
	public int backup(String pathSrc, String pathDest){
        StringBuilder cmd = new StringBuilder("Backup");
        cmd.append(" ");
        cmd.append(pathSrc);
        cmd.append(" ");
        cmd.append(pathDest);
        return mSrvClient.excute(cmd.toString());       
	}

    public int restore(String pathSrc, String pathDest){
        StringBuilder cmd = new StringBuilder("Restore");
        cmd.append(" ");
        cmd.append(pathSrc);
        cmd.append(" ");
        cmd.append(pathDest);
        return mSrvClient.excute(cmd.toString());
    }
    
	static class BackupRestoreSrvClient{
		private static final String SERVER_NAME = "backuprestore";
        private static final int RES_LEN = 2;

        private static BackupRestoreSrvClient mSrvClient = new BackupRestoreSrvClient();
        
		private LocalSocket mClientSocket;
	
		private InputStream mIn;
		private OutputStream mOut;
		private byte res[] = new byte[RES_LEN];
		private int buflen = 0;

        private BackupRestoreSrvClient(){             
        }

        public static BackupRestoreSrvClient getInstence(){
            return mSrvClient;
        }
	
		public boolean connect(){
			if (mClientSocket != null)
				return true;
			Log.i(TAG, "Connecting...");
			try{
				mClientSocket = new LocalSocket();
				LocalSocketAddress address = new LocalSocketAddress(SERVER_NAME,
					LocalSocketAddress.Namespace.RESERVED);
				mClientSocket.connect(address);

				mIn = mClientSocket.getInputStream();
				mOut = mClientSocket.getOutputStream();
			}catch(IOException e){
			    e.printStackTrace();
				disconnect();
				return false;				
			}
            return true;
		}

		public void disconnect() {
	        Log.i(TAG, "Disconnecting...");
	        try {
	            if (mClientSocket != null)
	                mClientSocket.close();
	        } catch (IOException ex) {
	        }
	        try {
	            if (mIn != null)
	                mIn.close();
	        } catch (IOException ex) {
	        }
	        try {
	            if (mOut != null)
	                mOut.close();
	        } catch (IOException ex) {
	        }
	        mClientSocket = null;
	        mIn = null;
	        mOut = null;
    	}

        public int excute(String cmd){
            Log.i(TAG, "excute...");
            if (!connect()) {
                Log.e(TAG, "connection failed");
                return -1;
            }
            
            if(!writeCmd(cmd)){
                Log.e(TAG, "write failed");
                if(!connect() || !writeCmd(cmd)) {
                	Log.e(TAG, "write failed");
                    return -1;
                }
            }
            Log.i(TAG, "send cmd:" + cmd);
            return readReply();
        }

        public boolean writeCmd(String cmd){            
            short len = (short)cmd.getBytes().length;
            byte[] precmd = new byte[2];
            if (len < 1 || len > 2048)
                return false;            
            precmd[0] = (byte)(len & 0xff);
            precmd[1] = (byte)((len >> 8) & 0xff);
            try{
                Log.i(TAG, "before write");
                mOut.write(precmd, 0, 2);
                mOut.write(cmd.getBytes(), 0, len);
                Log.i(TAG, "after write");
            }catch(IOException e){
                Log.e(TAG, "write error");
                disconnect();
                return false;
            }
            return true;
        }

        public int readReply(){
            int off = 0, count = 0;             
            while(off < RES_LEN){
                try{                        
                    count = mIn.read(res, count, RES_LEN - count);
                    if (count <= 0){
                        Log.e(TAG, "readReply failed! count:" + count);
                        break;
                    }
                    off += count;
                }catch(IOException e){
                    Log.e(TAG, "read exception");
                    break;
                }
            }
            if (off == RES_LEN){
                return (res[1] << 8) + res[0];        
            }
            return -1;
        }        
	}
}



