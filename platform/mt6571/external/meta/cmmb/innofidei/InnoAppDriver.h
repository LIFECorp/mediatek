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

#ifndef INNOAPPDRIVER_H
#define INNOAPPDRIVER_H

// InnoAppDriver.h - Innofidei CMMB API functions
//
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char BYTE;

/*
 * CMD_GET_SYS_STATE
 */
#define STATTYPE_SYNC_STATE			0x00
#define STATTYPE_SIGNAL_STRENGTH	0x01
#define STATTYPE_LDPC_TOTAL_COUNT	0x02
#define STATTYPE_LDPC_ERR_COUNT		0x03
#define STATTYPE_RS_TOTAL_COUNT		0x04
#define STATTYPE_RS_ERR_COUNT		0x05
#define STATTYPE_SIGNAL_QUALITY		0x06                                     //xingyu add
#define FW_ERR_STATUS			              0x07                                    //xingyu add
#define STATTYPE_BER_COUNT                        0x08                                    //xingyu add
#define STATTYPE_SNR_COUNT                        0x09                                   //xingyu add

// INNO_GET_SYS_STATE command parameter
struct inno_sys_state{
	BYTE	stattype;
	union{
		BYTE sync_state;
		int	signal_strength;
		int	ldpc_total_count;
		int   ldpc_err_count;
		int	rs_total_count;
		int	rs_err_count;
		int	signal_quality;
		int    fw_err_status;                  //xingyu 0317 uam debug
		int    BER;
		int    SNR;
	} statdata;
};

typedef enum{
	INNO_NO_ERROR = 0,
	INNO_GENERAL_ERROR = 1,
	INNO_TIMEOUT_ERROR = 2,
	INNO_FW_OPERATION_ERROR = 3,
	INNO_FW_DOWNLOAD_ERROR = 4,
	INNO_PARAMETER_ERROR = 5,
} INNO_RET;

typedef void (*DemuxFrameCallBack)(BYTE *buffer, int buf_size, BYTE channel_id);

/*
 * Init interface, such as SPI, I2C or SDIO, and test communication
 *
 * Parameter:
 *		data_callback <in>	: TS data callback function
 *		
 */
INNO_RET InnoAppDriverInit(DemuxFrameCallBack data_callback);


/*
 * Deinit SPI, I2C, SDIO
 *
 * Parameter:
 *		void
 *
 */
INNO_RET InnoAppDriverDeinit();

/*
 * Debug memset mfs buffer,in order to check mfs data error(shift 4)
 * due to the spi issue
 *
 * Parameter:
 *	flag <in>	                                       : memset yes(1) or no(0)
 *
 */
INNO_RET InnoMfsMemset(int flag);

/*
 * Set Tuner Frequency 
 *
 * Parameter:
 *		freq_dot <in>					: the Tuner freq dot.
 *
 *
 * Example:
 *		InnoSetTunerFrequency(20);		// Set to 20 frequency dot. meaning 530000KHZ 	
 *		InnoSetTunerFreqency(43);		// Set to 43 frequency dot. meaning 754000KHZ 	
 *
 */
INNO_RET InnoSetTunerFrequency(BYTE freq_dot);


/*
 * Set Channel Configuration 
 *
 * Parameter:
 *		ts_start <in>:		logic channel start ts number
 *		ts_count <in>:		logic channel ts count
 *		demod   <in>:		logic channel demod config 
 *		channel_id <in>:	logic channel ID, TS0 and ESG - 1, program - 2
 *      
 * Example:
 *		InnoSetChannelConfig(1,7,4,0x54);
 *		Set logic channel 1 to receive timeslot 7,8,9,10, demod config: QPSK, RS(240,224),Mode1,1/2LDPC
 *
 *
 * NOTE: 
 *	demod config = 
 *		|	Bit7	Bit6	|	Bit5	Bit4	|	Bit3	Bit2	|	Bit1	Bit0
 *		|	Modulate Type	|	RS Mode			|	Outer Mode		|	LDPC Mode
 *	00	|	BPSK			|	RS(240,240)		|	Reserved		|	1/2 LDPC
 *	01	|	QPSK			|	RS(240,224)		|	Mode 1			|	3/4 LDPC
 *	10	|	16QAM			|	RS(240,192)		|	Mode 2			|	Reserved
 *	11	|	Reserved		|	RS(240,176)		|	Mode 3			|	Reserved
 *
 */
INNO_RET InnoSetChannelConfig(BYTE channel_id, BYTE ts_start, BYTE ts_count, BYTE demod,BYTE subframe_ID);


/*
 * Get Channel Configuration
 *
 * Parameter:
 *		channel_id <in>:		logic channel ID
 *		*ts_start<out>:		channel start ts number
 *		*ts_count <out>:		channel  ts count
 *		*demod<out>:			channel demod config
 *
 */
INNO_RET InnoGetChannelConfig(BYTE channel_id, BYTE *ts_start, BYTE *ts_count, BYTE *demod,BYTE* subframe_ID);


/*
 * Get System Status
 *
 * Parameter:
 *		sys_status <out>
 *			sys_status.sync						//sync status
 *			sys_status.current_frequency		//current frequency dot
 *			sys_status.signal_strength			//signal strength. unit:-dBm
 *			sys_status.ldpc_err_percent		//LDPC error. unit:%
 *			sys_status.rs_err_percent		//rs error. unit:%
 *
*/
INNO_RET InnoGetSysStatus(struct inno_sys_state* sys_state);


/*
 * Close all Channel
 *
 * Parameter:
 *		channel_id <in>:		logic channel ID
 *
 */
INNO_RET InnoCloseChannel(BYTE channel_id);


/*
 * Init UAM: include reset uam and pps exchange
 *	
 * Parameter:
 *		void
 *
*/
INNO_RET InnoUamInit();


/*
 * reset uam
 *	
 * Parameter:
 *		pATRValue<out>: response value after reset uam
 *		pATRLen<in/out>:  response buffer size/response value length after reset uam
*/
INNO_RET InnoUamReset(BYTE *pATRValue, unsigned int *pATRLen);


/*
 * UAM APDU Transfer
 *	
 * Parameter:
 *		pBufIn<in>: APDU cmd
 *		bufInLen<in>: APDU cmd length
 *		pBufOut<out>: response value after sending APDU cmd
 *		pBufOutLen<out>: response value len after sending APDU cmd
 *		sw<out>: response status words
*/
INNO_RET InnoUamTransfer(BYTE *pBufIn, unsigned int bufInLen, BYTE *pBufOut, unsigned short*pBufOutLen, unsigned short *sw);

typedef struct _CMBBMS_SK
{
	unsigned char ISMACrypSalt[18];
	unsigned char SKLength;
}CMBBMS_SK;

typedef struct _CMBBMS_ISMA
{
	unsigned char MBBMS_ECMDataType;
	CMBBMS_SK ISMACrypAVSK[2];

}CMBBMS_ISMA;

#define CMD_SET_CARD_ENV			0x07
#define CMD_SET_MBBMS_ISMA 			0xc8
#define CMD_SET_UAM_OVER 			0xED
#define CMD_SET_UAM_AID_3G			0xc9

typedef enum{
	CAS_OK = 0x00,
	NO_MATCHING_CAS = 0x15,
	CARD_OP_ERROR = 0x17,
	MAC_ERR = 0x80,
	GSM_ERR = 0x81,
	KEY_ERR	= 0x82,
	KS_NOT_FIND	= 0x83,
	KEY_NOT_FIND	= 0x84,
	CMD_ERR	= 0x85,
}ERR_STATUS;
/*
 * Identify that UAM APDU is Over
 *
 * Parameter:
 *		void
 *
 * Note:
 *	Call it when a group of UAM APDU is over
 *
 */
INNO_RET Inno_SetUAMOver(void);
/*
 * Set SIM Card Environment
 *
 * Parameter:
 *		airnetwork <in> : 2g or 3g
 *
 * Example:
 *		Inno_SetCardEnv(2); 	//sim card, 2g environment; 	
 *		Inno_SetCardEnv(3); 	//usim card, 3g environment;	
 */
INNO_RET Inno_SetCardEnv(unsigned char airnetwork);

/*
 * Set MBBMS ISMA 
 *
 * Parameter:
 *		isBase64 <in>	: is base64
 *		mbbms_isma<in> : mbbms isma
 *
 */
INNO_RET Inno_Set_MBBMS_ISMA(unsigned char isBase64, CMBBMS_ISMA mbbms_isma);
/*
 * Set UAM 3G AID
 *
 * Parameter:
 *		uam_aid <in>	: UAM AID Value
 *		aid_len<in> : UAM AID Value Length
 *
 */
INNO_RET Inno_Set_UAM_AID_3G(unsigned char *uam_aid, unsigned char aid_len);

/*
 * Read err status that firmware get cw
 *
 * Parameter:
 *		errstatus <out>	: err status
 *
 */
void Inno_ReadErrStatus(unsigned long *errstatus);
#ifdef __cplusplus
}
#endif

#endif // INNOAPPDRIVER_H
