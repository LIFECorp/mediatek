/*
 * ANT Stack testing appication
 *
 * Copyright 2009 Dynastream Innovations
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <signal.h>

#include "ant_native.h"
#include "ant_types.h"
#include "ant_log.h"
#undef LOG_TAG
#define LOG_TAG "antradio_app"



/* transient stage */

typedef ANTStatus ant_status;

void app_ANT_rx_callback(ANT_U8 ucLen, ANT_U8* pucData);
void app_ANT_state_callback(ANTRadioEnabledStatus uiNewState);

static int Ant_Create(void)
{
    ANTStatus antStatus;

    //register_antsig_handlers();

   antStatus = ant_init();
   if (antStatus)
   {
      printf("failed to init ANT rx stacki\n");
      goto CLEANUP;
   }

   antStatus = set_ant_rx_callback(app_ANT_rx_callback);
   if (antStatus)
   {
      printf("failed to set ANT rx callback");
      goto CLEANUP;
   }

   antStatus = set_ant_state_callback(app_ANT_state_callback);
   if (antStatus)
   {
      printf("failed to set ANT rx callback");
      goto CLEANUP;
   }
   return antStatus;

CLEANUP:
   return ANT_STATUS_FAILED;
}

void ProcessCommand(char cCmd)
{
   ANT_U8 TxMessage[256];
   switch (cCmd)
   {
      case 'V':
         TxMessage[0] = 0x02;   //Size
         TxMessage[1] = 0x4D;   //MESG_REQUEST_ID
         TxMessage[2] = 0x00;   //Ch0
         TxMessage[3] = 0x3E;
         ant_tx_message(4,TxMessage);
         break;
      case 'R':
         TxMessage[0] = 0x01;   //Size
         TxMessage[1] = 0x4A;   //MESG_RESET_ID
         TxMessage[2] = 0x00;   //Ch0
         ant_tx_message(3,TxMessage);
         break;
      case 'K':
      //   printf("Hard Reset returned: %d\n", ant_radio_hard_reset());
         break;

      case 'H':
         //Normally we will not blindly send commands like this and actually check for responses before sending the next command, but this is just for a quick test.
         ProcessCommand('R');   //Reset chip
         ProcessCommand('A');   //Assign channel
         ProcessCommand('F');   //Set RF Freq
         ProcessCommand('I');   //Set Channel ID
         ProcessCommand('P');   //Set Channel Period
         ProcessCommand('O');   //Open Channel
         break;

      case 'A':
         TxMessage[0] = 0x03;   //Size
         TxMessage[1] = 0x42;   //MESG_ASSIGN_ID
         TxMessage[2] = 0x00;   //Ch0
         TxMessage[3] = 0x00;   //Assignment Params (Rx channel)
         TxMessage[4] = 0x01;   //Network 1 (ANT+)
         ant_tx_message(5,TxMessage);
         break;

      case 'F':
         TxMessage[0] = 0x02;   //Size
         TxMessage[1] = 0x45;   //MESG_CHANNEL_RADIO_FREQ_ID
         TxMessage[2] = 0x00;   //Ch0
         TxMessage[3] = 57;   //2.457GHz
         ant_tx_message(4,TxMessage);
         break;

      case 'I':
         TxMessage[0] = 0x05;   //Size
         TxMessage[1] = 0x51;   //MESG_CHANNEL_ID_ID
         TxMessage[2] = 0x00;   //Ch0
         TxMessage[3] = 0x00;   //Wildcard Device Number
         TxMessage[4] = 0x00;   //Wildcard Device Number
         TxMessage[5] = 0x78;   //Set HRM Device Type
         TxMessage[6] = 0x00;   //Wildcard Transmission Type
        ant_tx_message(7,TxMessage);
         break;

      case 'P':
         TxMessage[0] = 0x03;   //Size
         TxMessage[1] = 0x43;   //MESG_CHANNEL_MESG_PERIOD_ID
         TxMessage[2] = 0x00;   //Ch0
         TxMessage[3] = 0x86;   //
         TxMessage[4] = 0x1F;   // HRM MESG Peroid 0x1F86 (8070)
         ant_tx_message(5,TxMessage);
         break;

      case 'O':
         TxMessage[0] = 0x01;   //Size
         TxMessage[1] = 0x4B;   //MESG_OPEN_CHANNEL__ID
         TxMessage[2] = 0x00;   //Ch0
         ant_tx_message(3,TxMessage);
         break;
      case 'E':
          printf("Enable returned: %d\n", ant_enable_radio());
         break;
      case 'D':
          printf("Disable returned: %d\n", ant_disable_radio());
         break;
      case 'S':
          printf("State is: %d\n", ant_radio_enabled_status());
         break;
      case '1':
         TxMessage[0] = 0x0A;   //Size
         TxMessage[1] = 0x01;   //Enable
         TxMessage[2] = 0x00;   //Code upload
         TxMessage[3] = 0x00;
         TxMessage[4] = 0x00;
         TxMessage[5] = 0x00;
         TxMessage[6] = 0x00;
         TxMessage[7] = 0x00;
         TxMessage[8] = 0x00;
         TxMessage[9] = 0x00;
         TxMessage[10] = 0x00;
         //ANT_CORE_Send_VS_Command(0xFDD0, 11, TxMessage);
         printf("Not Implemented\n");
         break;

      case '2':
         TxMessage[0] = 0x00;   //Size
         //ANT_CORE_Send_VS_Command(0xFF22, 1, TxMessage);
         printf("Not Implemented\n");
         break;


      case 'X':
         printf("Exiting\n");
         break;

      default:
         printf("Invalid command: %#02x\n", cCmd);
         break;
   }
}


void app_ANT_rx_callback(ANT_U8 ucLen, ANT_U8* pucData)
{
   ANT_U8 i;

   for(i=0; i <ucLen; i++)
      printf("[%02X]",pucData[i]);
   switch (pucData[1])
   {
      case 0x3E:
         printf(" ANT FW Version:%s\n", &(pucData[2]));
         break;
      case 0x6F:
         printf(" Chip Reset\n");
         break;
      case 0x40:
         if (pucData[3] != 0x01)
         {
            printf(" Response (Ch:%d Mesg:%02X) ", pucData[2], pucData[3]);
            if (pucData[4] == 0)
               printf("Success\n");
            else
               printf("Error - %02X\n", pucData[4]);
         }
         else
         {
            printf(" Event (Ch:%d) %02X\n", pucData[2], pucData[4]);
         }
         break;
      case 0x4E:
         if (pucData[2] == 0)  //we are using channel 0 for HRM
         {
            // We are just assuming this is a HRM message and pulling the BPM out, refer to the ANT+ HRM profile documentation for proper/complete decoding instructions.
            printf(" BPM: %u\n", pucData[10]);
         }
         break;

      default:

         break;
   }
   return;
}

void app_ANT_state_callback(ANTRadioEnabledStatus uiNewState)
{
   const char *pcState;
   switch (uiNewState) {
   case RADIO_STATUS_UNKNOWN:
      pcState = "UNKNOWN";
      break;
   case RADIO_STATUS_ENABLING:
      pcState = "ENABLING";
      break;
   case RADIO_STATUS_ENABLED:
      pcState = "ENABLED";
      break;
   case RADIO_STATUS_DISABLING:
      pcState = "DISABLING";
      break;
   case RADIO_STATUS_DISABLED:
      pcState = "DISABLED";
      break;
   case RADIO_STATUS_NOT_SUPPORTED:
      pcState = "NOT SUPPORTED";
      break;
   case RADIO_STATUS_SERVICE_NOT_INSTALLED:
      pcState = "SERVICE NOT INSTALLED";
      break;
   case RADIO_STATUS_SERVICE_NOT_CONNECTED:
      pcState = "SERVICE NOT CONNECTED";
      break;
   case RADIO_STATUS_RESETTING:
      pcState = "RESETTING";
      break;
   case RADIO_STATUS_RESET:
      pcState = "RESET";
      break;
   default:
      printf("State change to: %d is an undefined state\n", uiNewState);
      return;
   }
   printf("State change to: %s\n", pcState);
}


int main(void)
{
   char buffer[1024];
   int ret = 0;

   
   if (Ant_Create())
   {
      printf("failed to init ANT\n");
      goto out;
   }

   printf("===ANT Test===\n");
   printf("Using libantradio version:\n");
   printf("%s\n", ant_get_lib_version());
   printf("\n");
   printf("Press V to get Version\n");
   printf("Press R to Reset\n");
   printf("Press K to hard reset\n");
   printf("Press H to setup an ANT+ HRM rx channel\n");
   printf("\n");
   printf("Press A to Assign channel\n");
   printf("Press F to set radio Frequency\n");
   printf("Press I to set channel Id\n");
   printf("Press P to set channel Peroid\n");
   printf("Press O to Open channel\n");
   printf("\n");
   printf("Press E to Enable ANT\n");
   printf("Press D to Disable ANT\n");
   printf("Press S to get State\n");
   printf("\n");
   printf("Press X to eXit\n");

   while (1)
   {
      memset(&buffer,0,sizeof(buffer));
      fgets(buffer, sizeof(buffer), stdin);
      ProcessCommand(buffer[0]);
      if (buffer[0] == 'X')
         goto done;
   }

done:
   ProcessCommand('R');
   sleep(1);
   ProcessCommand('D');
   ant_deinit();


out:

   return ret;
}
