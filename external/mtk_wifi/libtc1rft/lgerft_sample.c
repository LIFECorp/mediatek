#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lgerft.h"


struct pkteng_rx rx_tmp;

void nomod_tx(int channel, int rf_gain, int duration)
{
	int error = 0;
// 	int channel = 6;
//	int rf_gain = 50;
//	int duration = 3;
	
	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}
		
	if(true != LGE_RFT_Channel(channel)){
		error = 2;
		goto ERR;
	}
// 	if(true != LGE_NoMod_TxGain(rf_gain)){
// 		error = 3;
// 		goto ERR;
//	}
	if(true != LGE_RFT_TxGain(rf_gain)){
		error = 3;
		goto ERR;
	}
	if(true != LGE_NoMod_TxStart()){
		error = 4;
		goto ERR;
	}
	sleep(duration);

	if(true != LGE_NoMod_TxStop()){
		error = 5;
		goto ERR;
	}

	if(true != LGE_RFT_CloseDUT()){
		error = 6;
		goto ERR;
	}

	return;
ERR:
	printf("Tx nomod error is %d\n", error);
	return;
}


void Tx(int datarate, int channel, int rf_gain)
{
	int error = 0;
//	int channel = 11, rf_gain = 50;
//	int datarate = LGE_RFT_RATE_48MBPS;

	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}
		
	if(true != LGE_RFT_TxDataRate(datarate)){
		error = 2;
		goto ERR;
	}

	if(true != LGE_RFT_Channel(channel)){
		error = 3;
		goto ERR;
	}
	
	if(true != LGE_RFT_TxGain(rf_gain)){
		error = 4;
		goto ERR;
	}

	if(true != LGE_RFT_TxBurstInterval(20)){
		error = 5;
		goto ERR;
	}

	if(true != LGE_RFT_TxPayloadLength(1200)){
		error = 6;
		goto ERR;
	}
	
	{
		unsigned char dest[6] = {0x0c, 0x12, 0x34, 0x56, 0x78, 0x9a};
		if(true != LGE_RFT_TxDestAddress(dest)){
			error = 7;
			goto ERR;
		}
	}

// 	if(true != LGE_RFT_TxBurstFrames(10000)){
// 			error = 8;
// 			goto ERR;
// 	}


	if(true != LGE_RFT_TxStart()){
		error = 9;
		goto ERR;
	}

	return;
ERR:
	printf("Tx error is %d\n", error);
	return;
}


void Tx_11n(int mcs, int channel, int rf_gain)
{
	int error = 0;
	//int channel = 11, rf_gain = 50;
	
	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}
	if(true != LGE_RFT_TxDataRate11n(mcs, 2, 2)){
		error = 2;
		goto ERR;
	}
	if(true != LGE_RFT_Channel(channel)){
		error = 3;
		goto ERR;
	}
	if(true != LGE_RFT_TxGain(rf_gain)){
		error = 4;
		goto ERR;
	}

	if(true != LGE_RFT_TxStart()){
		error = 5;
		goto ERR;
	}

	return;
ERR:
	printf("Tx_11n error is %d\n", error);
	return;

}


void TxStop(void)
{
	int error = 0;
	
	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}
		
	if(true != LGE_RFT_TxStop()){
		error = 2;
		goto ERR;
	}

	if(true != LGE_RFT_CloseDUT()){
		error = 3;
		goto ERR;
	}

	return;
ERR:
	printf("TxStop error :  %d", error);
}


void Rx(int channel, int time)
{
	int error = 0;
	//int channel = 1, time = 3;
	int total = 0, good = 0, bad = 0, rssi = 0;	

	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}

	if(true == LGE_RFT_IsRunning())
		printf("DUT is running.\n");
	else
		printf("DUT is stopped.\n");

	if(true == LGE_RFT_IsUp())
		printf("DUT is Up\n");
	else
		printf("DUT is down\n");
	
	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}

	if(true != LGE_RFT_Channel(channel)){
		error = 2;
		goto ERR;
	}

	if(true != LGE_RFT_RxStart(&rx_tmp)){
		error = 3;
		goto ERR;
	}
	sleep(time);

	if(true != LGE_RFT_TotalPkt(&total)){
		error = 4;
		goto ERR;
	}
	if(true != LGE_RFT_FRGood(&good)){
		error = 5;
		goto ERR;
	}

	if(true != LGE_RFT_FRError(&bad)){
		error = 6;
		goto ERR;
	}

	if(true != LGE_RFT_RSSI(&rssi)){
		error = 7;
		goto ERR;
	}

	if(true != LGE_RFT_RxStop(&rx_tmp)){
		error = 8;
		goto ERR;
	}
	
	if(true !=  LGE_RFT_CloseDUT()){
		error = 9;
		goto ERR;
	}
	printf("total =%d, good =%d, bad =%d, rssi =%d\n", total, good, bad, rssi);

	if(true == LGE_RFT_IsRunning())
		printf("DUT is running.\n");
	else
		printf("DUT is stopped.\n");

	if(true == LGE_RFT_IsUp())
		printf("DUT is Up\n");
	else
		printf("DUT is down\n");

	return;

ERR:
	printf("Rx err %d\n", error);
}


void dumpMac(char *mac)
{
	printf("The current mac is 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


void checkMac(void)
{
	char mac[6];
	int error = 0;
	
	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}

	if(true != LGE_RFT_ReadMac(mac)){
		error = 2;
		goto ERR;
	}else 
		dumpMac(mac);

	{
		char tmp;
		tmp = mac[5];
		mac[5] = mac[1];
		mac[1] = tmp;
		
		tmp = mac[4];
		mac[4] = mac[2];
		mac[2] = tmp;
	}
	
	if(true != LGE_RFT_WriteMac(mac)){
		error = 3;
		goto ERR;
	}else 
		dumpMac(mac);

	if(true != LGE_RFT_ReadMac(mac)){
		error = 4;
		goto ERR;
	}else 
		dumpMac(mac);
	
	if(true != LGE_RFT_CloseDUT()){
		error = 6;
		goto ERR;
	}

	return;

ERR:
	printf("checkMac error %d", error);
}

void otaTest(void)
{
	int error = 0;
	
	if(true != LGE_RFT_OpenDUT()){
		error = 1;
		goto ERR;
	}

	if(true != LGE_RFT_PowerSave(0)){
		error = 2;
		goto ERR;
	}
	
	if(true != LGE_RFT_Roaming(0)){
		error = 3;
		goto ERR;
	}
	
	if(true != LGE_RFT_CloseDUT()){
		error = 4;
		goto ERR;
	}

	return;

ERR:
	printf("otaTest error %d\n", error);
}



int main(void) {

#if 0
    int i;
    int rxOk, rxErr, rxRssi;
    char macAddr[6];
    bool retval;
    bool ret[3];

    printf("entering RF testing mode ..\n");



    retval = LGE_RFT_OpenDUT();
    printf("(%d) entered RF testing mode ..\n", retval);

    sleep(1);

    retval = LGE_RFT_GetMACAddr(macAddr);
    printf("(%d) MAC address: [%02X:%02X:%02X:%02X:%02X:%02X]\n", 
            retval,
            macAddr[0],
            macAddr[1],
            macAddr[2],
            macAddr[3],
            macAddr[4],
            macAddr[5]);

    retval = LGE_RFT_Channel(1);
    printf("(%d) changed channel to #1..\n", retval);


    retval = LGE_RFT_RxStart(NULL);
    printf("(%d) RX test started..\n", retval);


    for(i = 0 ; i < 10 ; i++) {
        ret[0] = LGE_RFT_FRGood(&rxOk);
        ret[1] = LGE_RFT_FRError(&rxErr);
        ret[2] = LGE_RFT_RSSI(&rxRssi);


        printf("[%d] (%d)RX OK: %d / (%d)RX ERR: %d / (%d)RSSI: %d\n", i, ret[0], rxOk, ret[1], rxErr, ret[2], rxRssi);
        sleep(1);
    }

    retval = LGE_RFT_CloseDUT();
    printf("(%d) left RF testing mode ..\n", retval);
#else
    	printf("entering RF testing mode ..\n");
		

	while(1)
	{
		int i,datarate,channel,gain,duration,mcs,plcp,gi;
		printf("Please input your selection:\n");
		printf("1:nomod_tx (channel) (gain) (duration)\n");
		printf("2:Tx (datarate) (channel) (gain)\n");
		printf("3:TxStop \n");
		printf("4:Tx_11n (MCS) (channel) (gain) (PLCP) (GI)\n");
		printf("5:Tx11nStop\n");
		printf("6:Rx (datarate) (channel) (duration)\n");
		printf("7:checkMac \n");
		printf("8:otaTest \n");
		printf("datarate 1~12, channel 1~14 gain 2~20, MCS 1~8\n");
		scanf("%d", &i);
		
		switch(i)
		{
			case 1:
				printf("1:nomod_tx (channel) (gain) (duration)\n");
				scanf("%d %d %d",&channel, &gain, &duration);
				nomod_tx(channel,gain,duration);
				//printf("skip nomod tx, which is not ready");
				break;
				
			case 2:
				printf("2:Tx (datarate) (channel) (gain)\n");
				scanf("%d %d %d",&datarate, &channel, &gain);
				Tx(datarate,channel,gain);
				break;

			case 3:
				TxStop();
				break;
			
			case 4:		
				printf("4:Tx_11n (MCS) (channel) (gain) (PLCP) (GI)\n");
				scanf("%d %d %d %d %d",&mcs, &channel, &gain, &plcp, &gi);
				Tx_11n(mcs,channel,gain);
				break;

			case 5:	
				TxStop();
				break;
			
			case 6:
				printf("6:Rx (datarate) (channel) (duration)\n");
				scanf("%d %d %d",&datarate, &channel, &duration);
				Rx(channel, duration);
				break;
			
			case 7:		
				checkMac();
				break;
			case 8:	
				otaTest();
				break;
		}
	}

    	printf("left RF testing mode ..\n");

#endif


    return 0;
}
