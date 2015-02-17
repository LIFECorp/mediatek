/*
 * This is sample code for LGE WLAN AT command and Hidden Menu
 */

#ifndef __LIBLGERFT_H__
#define __LIBLGERFT_H__

/* basic definitions
 * -------------------------------------------------------------------------- */
#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

/* types
 * -------------------------------------------------------------------------- */
typedef enum {
	LGE_RFT_PREAMBLE_LONG,
	LGE_RFT_PREAMBLE_SHORT,
} PreambleType_t;

/* LGE_RFT_TxDataRate */
#define LGE_RFT_RATE_AUTO		0
#define LGE_RFT_RATE_1MBPS		1
#define LGE_RFT_RATE_2MBPS		2
#define LGE_RFT_RATE_5_5MBPS		3
#define LGE_RFT_RATE_6MBPS		4
#define LGE_RFT_RATE_9MBPS		5
#define LGE_RFT_RATE_11MBPS		6
#define LGE_RFT_RATE_12MBPS		7
#define LGE_RFT_RATE_18MBPS		8
#define LGE_RFT_RATE_24MBPS		9
#define LGE_RFT_RATE_36MBPS		10
#define LGE_RFT_RATE_48MBPS		11
#define LGE_RFT_RATE_54MBPS		12

/* Supported MCS rates */
enum {
	LGE_RFT_MCS_RATE_0 = 0,
	LGE_RFT_MCS_RATE_1 = 1,
	LGE_RFT_MCS_RATE_2 = 2,
	LGE_RFT_MCS_RATE_3 = 3,
	LGE_RFT_MCS_RATE_4 = 4,
	LGE_RFT_MCS_RATE_5 = 5,
	LGE_RFT_MCS_RATE_6 = 6,
	LGE_RFT_MCS_RATE_7 = 7
};

/* Supported STF mode */
enum {
	LGE_RFT_STF_MODE_SISO = 0, 	/* stf mode SISO */
	LGE_RFT_STF_MODE_CDD = 1,  	/* stf mode CDD  */
	LGE_RFT_STF_MODE_STBC = 2,  	/* stf mode STBC */
	LGE_RFT_STF_MODE_SDM  = 3	/* stf mode SDM  */
};

struct pkteng_rx {
    unsigned long pktengrxducast_old;
    unsigned long pktengrxducast_new;
    unsigned long rxbadfcs_old;
    unsigned long rxbadfcs_new;
    unsigned long rxbadplcp_old;
    unsigned long rxbadplcp_new;
};


/* functions
 * -------------------------------------------------------------------------- */
bool LGE_RFT_OpenDUT(void);
bool LGE_RFT_CloseDUT(void);
bool LGE_RFT_TxDataRate(int TxDataRate);
bool LGE_RFT_SetPreamble(PreambleType_t PreambleType);
bool LGE_RFT_Channel(int ChannelNo);
bool LGE_RFT_TxGain(int TxGain);
bool LGE_RFT_TxBurstInterval(int SIFS);
bool LGE_RFT_TxPayloadLength(int TxPayLength);
bool LGE_RFT_TxBurstFrames(int Frames);
bool LGE_RFT_TxDestAddress(unsigned char *addr);
bool LGE_RFT_TxStart(void);
bool LGE_RFT_TxStop(void);
bool LGE_RFT_RxStart(struct pkteng_rx*);
bool LGE_RFT_RxStop(struct pkteng_rx*);
bool LGE_RFT_FRError(int *FError);
bool LGE_RFT_FRGood(int *FRGood);
bool LGE_RFT_RSSI(int *RSSI);
bool LGE_RFT_IsRunning(void);
bool LGE_RFT_IsUp(void);
bool LGE_RFT_TxDataRate11n(int TxDataRate11n, int FrameFormat, int GI);
bool LGE_RFT_FrequencyAccuracy(int band, int ChannelNo);
bool LGE_RFT_FrequencyAccuracy_Stop();
bool LGE_RFT_GetMACAddr(unsigned char *macAddr);
bool LGE_RFT_SetMACAddr(unsigned char *macAddr);
bool LGE_RFT_SetTestMode(bool bEnable);

bool LGE_RFT_SetUnicast(int start);
bool LGE_RFT_TotalPkt(int *FRTotal);
bool LGE_RFT_WriteMac(char *addr);
bool LGE_RFT_ReadMac(char *addr);
bool LGE_RFT_PowerSave(bool enable);
bool LGE_RFT_Roaming(bool enable);
bool LGE_NoMod_TxGain(int rf_gain);
bool LGE_NoMod_TxStart();
bool LGE_NoMod_TxStop();



#endif /* __LIBLGERFT_H__ */
