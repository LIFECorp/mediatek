package com.orangelabs.rcs.wizard;

import java.util.List;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningManager;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.client.ImsApiIntents;
import com.orangelabs.rcs.utils.logger.Logger;

public class RcsWizardManagerService extends Service {

	 private boolean isUserConsentRequiredForRegistration = false;
	 private boolean isWizardActive = false;
	 private Handler mWizardUIHandle = null;
	 private boolean mSimExist = false;
	 private NetworkInfo mCurrentNetworkInfo = null;
	 private Context mContext = null;
	 public final static boolean isDebug = true;
	 
	 public static final int MSG_SHOW_PROCESSING = 0;
	 public static final int MSG_SHOW_JOYN_INTRO = 1;
	 public static final int MSG_SHOW_REGISTRATION_ASK_DIALOG = 2;
	 public static final int MSG_SHOW_REGISTRATION_SUCCESS = 3;
	 public static final int MSG_SHOW_REGISTRATION_FAILURE = 4;
	 public static final int MSG_SHOW_PROVISION_SUCCESS = 5;
	 public static final int MSG_SHOW_PROVISION_FAILURE = 6;
	 public static final int MSG_RCS_OOBE_FINISH = 7;
	 public static final int MSG_REGISTER_REGISTRATION_RECEIVER = 8;
	 public static final int MSG_INITIALIZE_JOYN = 9;
	 public static final int MSG_STOP_SERVICE = 10;
	 public static final int MSG_PROVISON_FAIURE = 11;
	 public static final int MSG_PROVISON_SUCCESS = 12;
	 public static final int MSG_CLOSE_TIMER = 13;
	 

	  private static boolean isRunning = false;
	 final Messenger mMessenger = new Messenger(new IncomingHandler());
	 Messenger mOutgoingMessenger;
	 
	 
	 
	 private static BroadcastReceiver rcsRegistrationListner = null;
	 private static BroadcastReceiver rcsProvisioningListner = null;  
	   
	  
	 
	 /**
	     * The logger
	     */
	    private Logger logger = Logger.getLogger(this.getClass().getName());
	    
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return mMessenger.getBinder();
	}
	
	
	 @Override
	    public void onCreate() {
	        super.onCreate();
	        isRunning = true;
	    }
	 
	 @Override
	    public void onDestroy() {
	        super.onDestroy();
	        isRunning = false;
	        finishRcsOOBE();
	    }
	 
	class IncomingHandler extends Handler { // Handler of incoming messages from clients.
        @Override
        public void handleMessage(Message msg) {
        	
        	 if (logger.isActivated()) {
                 logger.debug("RcsWizardManagerService : IncomingHandler message :"+ msg);
             }
        	 
            switch (msg.what) {
            case MSG_INITIALIZE_JOYN:
            	
            	
            	if (logger.isActivated()) {
                    logger.debug(" message :MSG_INITIALIZE_JOYN");
                }
            	//set the OUtgoing messneger for the client
            	mOutgoingMessenger = msg.replyTo;
            	
            	registerProvisioningListner();
            	registerRegistrationListner();
            	
            	
            	startJoynService(getApplicationContext());
            	
                break;
            case MSG_STOP_SERVICE:
            	if (logger.isActivated()) {
                    logger.debug(" message :MSG_STOP_SERVICE");
                }
            	
            	mOutgoingMessenger = null;
            	finishRcsOOBE();
            	break;
            	
            case MSG_CLOSE_TIMER :
            	if (logger.isActivated()) {
                    logger.debug(" message :MSG_CLOSE_TIMER");
                }
            	
            	  sendMessageToUI(MSG_CLOSE_TIMER);
            	break;
            default:
                super.handleMessage(msg);
            }
        }
    }
	
	 @Override
	    public int onStartCommand(Intent intent, int flags, int startId) {
	        return START_NOT_STICKY; // run until explicitly stopped.
	    }
	 
	   public static boolean isRunning()
	    {
	        return isRunning;
	    }

	   private void sendMessageToUI(int intvaluetosend) {
	            try {  	
	                mOutgoingMessenger.send(Message.obtain(null, intvaluetosend, 0, 0));

	            } catch (RemoteException e) {
	                // The client is dead. Remove it from the list; we are going through the list from back to front so this is safe to do inside the loop.
	                mOutgoingMessenger = null;
	            }
	        }
	
	   
	   
	   public void setWizardState(boolean State){
			 isWizardActive = State;
		 }
		  

	  
	   /**
	    * show processing screen
	    * */
    private  void showProcessingScreen(){
      sendMessageToUI(MSG_SHOW_PROCESSING);
    }
    
    
    
    private void registerProvisioningListner(){
	   	// Register network state listener
    	   if (logger.isActivated()) {
               logger.debug("RcsWizardManagerService : registerProvisioningListner");
           }
	   	 
	   	 IntentFilter intentFilter = new IntentFilter();
	   		intentFilter.addAction(HttpsProvisioningManager.PROVISIONING_INTENT);	
	      
	        if (null == rcsProvisioningListner) {
	        	rcsProvisioningListner = new BroadcastReceiver() {
	                    @Override
	                    public void onReceive(Context context, final Intent intent) {
	                   
	                   	 boolean status = false;
	                   	 status = intent.getBooleanExtra("status", false);
	                   	 if (logger.isActivated()) {
                             logger.debug("RcsWizardManagerService : registerProvisioningListner : status :"+status);
                         }
	                   	 
	                   	 if(!status){
	                   		getApplicationContext().unregisterReceiver(rcsRegistrationListner);
		                   	rcsRegistrationListner = null;
	                   		 
		                   	//notify UI about provision failure
		                   	notifyProvisionFailure();
	                   	 }else{
	                   		notifyProvisionSuccess();
	                   	 }
	                   	getApplicationContext().unregisterReceiver(rcsProvisioningListner);
	                   	 rcsProvisioningListner = null;
	                   	
	                   	 
	                    }
	                };
	                AndroidFactory.getApplicationContext().registerReceiver(rcsProvisioningListner,
	                        intentFilter);
	        }
	    }
    

    
    private void registerRegistrationListner(){
	   	// Register network state listener
	   	 
	   	 
	   	 IntentFilter intentFilter = new IntentFilter();
	   		intentFilter.addAction(ImsApiIntents.IMS_STATUS);	
	      
	        if (null == rcsRegistrationListner) {
	       	 rcsRegistrationListner = new BroadcastReceiver() {
	                    @Override
	                    public void onReceive(Context context, final Intent intent) {
	                   	 
	                   	 boolean status = true;
	                   	 status = intent.getBooleanExtra("status", false);
	                   	 
	                   	 if (logger.isActivated()) {
                             logger.debug("RcsWizardManagerService : registerRegistrationListner : status :"+status);
                         }
	                   	 
	                   	getApplicationContext().unregisterReceiver(rcsRegistrationListner);
	                   	rcsRegistrationListner = null;
	                   	
	                   	 //registartion is successful
	                   	 updateRegistrationStatus(status);
	                    }
	                };
	                AndroidFactory.getApplicationContext().registerReceiver(rcsRegistrationListner,
	                        intentFilter);
	        }
	    }
    
    
   public void updateRegistrationStatus(boolean status){
	   if (logger.isActivated()) {
           logger.debug("RcsWizardManagerService : updateRegistrationStatus");
       }
       sendMessageToUI(MSG_SHOW_JOYN_INTRO);
      
    }

   private void notifyProvisionFailure(){
	   
	   if (logger.isActivated()) {
           logger.debug("RcsWizardManagerService : notifyProvisionFailure");
       }
	   sendMessageToUI(MSG_PROVISON_FAIURE);
	   
   }
   
   
   private void notifyProvisionSuccess(){
	   if (logger.isActivated()) {
           logger.debug("RcsWizardManagerService : notifyProvisionSuccess");
       }
	   sendMessageToUI(MSG_PROVISON_SUCCESS);
	   
   }
   
   
   
   private  void  finishRcsOOBE(){
    	
    	//set the wizard state to false
		 setWizardState(false);
		 
		 if(rcsProvisioningListner!= null){
		 	getApplicationContext().unregisterReceiver(rcsProvisioningListner);
		 	rcsProvisioningListner = null;
		 }
		 	
		 
		 if(rcsRegistrationListner!= null){
			 	getApplicationContext().unregisterReceiver(rcsRegistrationListner);
			 	rcsRegistrationListner = null;
		  }
		 
		 //send message to UI
		 //sendMessageToUI(MSG_RCS_OOBE_FINISH);
    	 
		 //this.stopSelf();
    }

   //start detecting the Joyn Service
   private void startJoynService(Context context){
	   
	   if (logger.isActivated()) {
           logger.debug("RcsWizardManagerService : startJoynService");
       }
	   
	     // Instantiate the settings manager
          RcsSettings.createInstance(context);
      	  RcsSettings.getInstance().setServiceActivationState(true);
      	  
		  LauncherUtils.launchRcsService(context, true, false);
	    	 
	  }
}
