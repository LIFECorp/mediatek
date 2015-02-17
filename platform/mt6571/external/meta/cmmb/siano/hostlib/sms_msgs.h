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

#ifndef SMS_FIRMWARE_API_H_
#define SMS_FIRMWARE_API_H_

#define SMS_TYPES_H		// For firmware compilation, to avoid duplication with sms_types_rom.h


#define SMS_MIN(a,b)    (((a)<(b))?(a):(b))

#define SMS_MAX(a,b)    (((a)>(b))?(a):(b))

enum Bool_E
{
	SMS_FALSE = 0,
	SMS_TRUE  = 1
};
typedef enum Bool_E Bool_ET;



typedef struct SmsMsgHdr_S
{
	UINT16 	msgType;
	UINT8	msgSrcId;
	UINT8	msgDstId;
	UINT16	msgLength;	// Length is of the entire message, including header
	UINT16	msgFlags;
} SmsMsgHdr_ST;

typedef struct SmsMsgData_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			msgData[1];
} SmsMsgData_ST;

typedef struct SmsMsgData2Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			msgData[2];
} SmsMsgData2Args_ST;


typedef struct SmsMsgData3Args_S
{
	SmsMsgHdr_ST	xMsgHeader;
	UINT32			msgData[3];
} SmsMsgData3Args_ST;

//******************************
// Log messages definitions 


typedef struct SMSHOSTLIB_LOG_ITEM_EX_S
{
	UINT32 NumLogs;
	SMSHOSTLIB_LOG_ITEM_ST LogItems[1]; //Variable size for printf logs.
} SMSHOSTLIB_LOG_ITEM_EX_ST;

// Definitions for the flags field in the message header
// bits 13,14,15 of msgFlags allocated to SmsCommIntf
// The definition is on a per-bit basis
#define MSG_HDR_DEFAULT_DYNAMIC_MSG	0x0000	// Message is dynamic 
#define MSG_HDR_FLAG_STATIC_MSG		0x0001	// Message is dynamic when this bit is '0'
#define MSG_HDR_FLAG_RX_DIR			0x0002	// Direction is RX when this bit is '1'
#define MSG_HDR_FLAG_SPLIT_MSG_HDR  0x0004  // Message format is SmsMessage_ST, with pMsg pointing to remainder of message
#define MSG_HDR_FLAG_TS_PAYLOAD		0x0008	// Message payload is in the form of TS packets
#define MSG_HDR_FLAG_ALIGN_MASK     0x0300	// Two bits denoting number of padding bytes inserted at the 
// start of the data in split messages. Used for alignment
#define MSG_HDR_FLAG_EXT_LEN_HDR	0xF000  // Extended msg len (MS nibble).

/*------------------------------**
** Base Error and success codes **
**------------------------------*/

// The standard return value that each function must return
// See sm_results.h for standard return values
typedef UINT32 SmsResult;


/* general success */
#define SMS_S_OK 	((SmsResult)0x00000000)

/* general error */
#define SMS_E_FAIL	((SmsResult)0x80000000)

/*-------------------------------------------------**
** General Macros to test for fail/success result. **
**-------------------------------------------------*/
#define SMS_FAILED(x) 	    ((x) & SMS_E_FAIL)
#define SMS_SUCCEEDED(x) 	(!((x) & SMS_E_FAIL))

#define MALLOC_ERR				(SMS_E_FAIL | 0x80000)
#define FREE_ERR				(SMS_E_FAIL | 0x90000)


/*---------------------**
** general Success codes **
**---------------------*/

#define SMS_S_PENDING              (SMS_S_OK | 1) /* successful but uncompleted asynchronous call.*/
#define SMS_S_EMPTY                (SMS_S_OK | 2) 

/*---------------------**
** general Error codes **
**---------------------*/

#define SMS_E_FATAL              (SMS_E_FAIL | 1)
#define SMS_E_OUT_OF_MEM         (SMS_E_FAIL | 2)
#define SMS_E_BAD_PARAMS         (SMS_E_FAIL | 3)
#define SMS_E_NOT_FOUND          (SMS_E_FAIL | 4)
#define SMS_E_MAX_EXCEEDED       (SMS_E_FAIL | 5)
#define SMS_E_OUT_OF_RANGE       (SMS_E_FAIL | 6)
#define SMS_E_NOT_INIT           (SMS_E_FAIL | 7)
#define SMS_E_FULL               (SMS_E_FAIL | 8)
#define SMS_E_EMPTY              (SMS_E_FAIL | 9)
#define SMS_E_INVALID_SIZE       (SMS_E_FAIL | 10)
#define SMS_E_INVALID_OPERATION  (SMS_E_FAIL | 11)
#define SMS_E_TIMEOUT            (SMS_E_FAIL | 12)
#define SMS_E_NULL_PTR           (SMS_E_FAIL | 13)
#define SMS_E_BUSY               (SMS_E_FAIL | 14)
#define SMS_E_ALREADY_INIT       (SMS_E_FAIL | 15)
#define SMS_E_NACK			     (SMS_E_FAIL | 16)
#define SMS_E_ALREADY_EXISTING   (SMS_E_FAIL | 17)

#define SMS_MAX_SERVICE_HANDLE	(16)

/********************************************************
SMS Host Library IDs
*********************************************************/
#define SMS_HOST_ID_BASE	100
#define SMS_HOST_LIB				(SMS_HOST_ID_BASE + 50)
#define SMS_HOST_LIB_INTERNAL		(SMS_HOST_ID_BASE + 51)
#define SMS_HOST_LIB_INTERNAL2		(SMS_HOST_ID_BASE + 52)

#define HIF_TASK				11		// Firmware messages processor task IS
#define HIF_TASK_SLAVE			22
#define HIF_TASK_SLAVE2			33


typedef enum
{
	FW_LOG_SEVERITY_NONE,
	FW_LOG_SEVERITY_ERROR,
	FW_LOG_SEVERITY_INFO,
	FW_LOG_SEVERITY_DEBUG,
} LogSeverity_ET;

typedef enum LogMode_E
{
	LOG_TYPE_DISABLED,
	LOG_TYPE_ENABLED, //old log mode
	LOG_TYPE_EXTENDED,  //new log mode (many logs in a single message)

}LogMode_ET;

enum CmmbEventTypes_E
{
	CMMB_EVENT_TEST_EVENT = 1,
	CMMB_EVENT_SET_RAW_MULTIPLEX_REC = 3,
	CMMB_EVENT_SET_IGNORE_CRC = 4,
	CMMB_EVENT_SET_OFF_CLKS = 5,
	CMMB_EVENT_ADD_OFF_CLK = 6,
	CMMB_EVENT_REM_OFF_CLK = 7,
	CMMB_EVENT_NAGRA_EADT_MOCKUP = 8,
};

// Definitions of the message types.
// For each type, the format used (excluding the header) is specified
// The message direction is also specified
typedef enum MsgTypes_E
{
	MSG_TYPE_BASE_VAL = 500,

	//MSG_SMS_RESERVED1 = 501,				//		
	//MSG_SMS_RESERVED1 = 502,				//

	MSG_SMS_GET_VERSION_REQ = 503,			// Get version
											// Format: None
											// Direction: Host->SMS

	MSG_SMS_GET_VERSION_RES = 504,			// The response to MSG_SMS_GET_VERSION_REQ
											// Format:	8-bit - Version string
											// Direction: SMS->Host

	//MSG_SMS_RESERVED1 = 505,				//
	//MSG_SMS_RESERVED1 = 506,				//
	//MSG_SMS_RESERVED1 = 507,				//
	//MSG_SMS_RESERVED1 = 508,				//

	MSG_SMS_GPIO_SET_LEVEL_REQ = 509,		// Set GPIO level high / low
											// Format: Data[0] = UINT32 PinNum
											//		   Data[1] = UINT32 NewLevel
											// Direction: Host-->FW

	MSG_SMS_GPIO_SET_LEVEL_RES = 510,		// The response to MSG_SMS_GPIO_SET_LEVEL_REQ
											// Direction: FW-->Host

	MSG_SMS_GPIO_GET_LEVEL_REQ = 511,		// Get GPIO level high / low
											// Format: Data[0] = UINT32 PinNum
											//		   Data[1] = 0
											// Direction: Host-->FW
											  
	MSG_SMS_GPIO_GET_LEVEL_RES = 512,		// The response to MSG_SMS_GPIO_GET_LEVEL_REQ
											// Direction: FW-->Host

	//MSG_SMS_RESERVED1 = 513,				//

	MSG_SMS_LOG_ENABLE_CHANGE_REQ = 514,	// Change the state of (enable/disable) log messages flow from SMS to Host (MSG_SMS_LOG_ITEM)
											// Format: 32-bit address value for g_log_enable
											// Direction: Host->SMS

	MSG_SMS_LOG_ENABLE_CHANGE_RES = 515,	// A reply to MSG_SMS_LOG_ENABLE_CHANGE_REQ
											// Format: 32-bit address value for g_log_enable
											// Direction: SMS->Host

	MSG_SMS_SET_MAX_TX_MSG_LEN_REQ = 516,	// Set the maximum length of a receiver message
											// Format: 32-bit value of length in bytes, must be modulo of 4
	MSG_SMS_SET_MAX_TX_MSG_LEN_RES = 517,	// ACK/ERR for MSG_SMS_SET_MAX_TX_MSG_LEN_REQ

	MSG_SMS_SPI_HALFDUPLEX_TOKEN_HOST_TO_DEVICE	= 518,	// SPI Half-Duplex protocol
	MSG_SMS_SPI_HALFDUPLEX_TOKEN_DEVICE_TO_HOST	= 519,  //

	// DVB-T MRC background scan messages
	MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_REQ	= 520, 
	MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_RES = 521, 
	MSG_SMS_BACKGROUND_SCAN_SIGNAL_DETECTED_IND		= 522,  
	MSG_SMS_BACKGROUND_SCAN_NO_SIGNAL_IND			= 523,  

	MSG_SMS_CONFIGURE_RF_SWITCH_REQ		= 524,
	MSG_SMS_CONFIGURE_RF_SWITCH_RES		= 525,

	MSG_SMS_MRC_PATH_DISCONNECT_REQ		= 526,
	MSG_SMS_MRC_PATH_DISCONNECT_RES		= 527,

	//MSG_SMS_RESERVED1 = 528,			//
	//MSG_SMS_RESERVED1 = 529,			// 
	//MSG_SMS_RESERVED1 = 530,			//
	//MSG_SMS_RESERVED1 = 531,			// 
	//MSG_SMS_RESERVED1 = 532,			// 

	MSG_WR_REG_RFT_REQ   =533,			// Write value to a given RFT register
										// Format: 32-bit address of register, following header
										//		   32-bit of value, following address
										// Direction: Host->SMS

	MSG_WR_REG_RFT_RES   =534,			// Response to MSG_WR_REG_RFT_REQ message
										// Format: Status of write operation, following header
										// Direction: SMS->Host

	MSG_RD_REG_RFT_REQ   =535,			// Read the value of a given RFT register
										// Format: 32-bit address of the register, following header
										// Direction: Host->SMS

	MSG_RD_REG_RFT_RES   =536,			// Response to MSG_RD_REG_RFT_RES message
										// Format: 32-bit value of register, following header
										// Direction: SMS->Host

	MSG_RD_REG_ALL_RFT_REQ=537,			// Read all 16 RFT registers
										// Format: N/A (nothing after the header)
										// Direction: Host->SMS

	MSG_RD_REG_ALL_RFT_RES=538,			// Response to MSG_RD_REG_ALL_RFT_REQ message
										// Format: For each register, 32-bit address followed by 32-bit value (following header)
										// Direction: SMS->Host

	MSG_HELP_INT          =539,			// Internal (SmsMonitor) message
										// Format: N/A (nothing after header)
										// Direction: Host->Host

	MSG_RUN_SCRIPT_INT    =540,			// Internal (SmsMonitor) message
										// Format: Name of script(file) to run, immediately following header
										// direction: N/A

	//MSG_SMS_RESERVED1 = 541,			//
	//MSG_SMS_RESERVED1 = 542,			// 
	//MSG_SMS_RESERVED1 = 543,			//
	//MSG_SMS_RESERVED1 = 544,			// 
	//MSG_SMS_RESERVED1 = 545,			// 
	//MSG_SMS_RESERVED1 = 546,			//  
	//MSG_SMS_RESERVED1 = 547,			// 
	//MSG_SMS_RESERVED1 = 548,			// 							
	//MSG_SMS_RESERVED1 = 549,			// 										
	//MSG_SMS_RESERVED1 = 550,			// 															
	//MSG_SMS_RESERVED1 = 551,			// 

	MSG_SMS_RD_MEM_REQ    =552,			// A request to read address in memory
										// Format: 32-bit of address, followed by 32-bit of range (following header)
										// Direction: Host->SMS

	MSG_SMS_RD_MEM_RES    =553,			// The response to MSG_SMS_RD_MEM_REQ
										// Format: 32-bit of data X range, following header
										// Direction: SMS->Host

	MSG_SMS_WR_MEM_REQ    =554,			// A request to write data to memory
										// Format:	32-bit of address
										//			32-bit of range (in bytes)
										//			32-bit of value
										// Direction: Host->SMS

	MSG_SMS_WR_MEM_RES    =555,			// Response to MSG_SMS_WR_MEM_REQ
										// Format: 32-bit of result
										// Direction: SMS->Host

	//MSG_SMS_RESERVED1 = 556,			//
	//MSG_SMS_RESERVED1 = 557,			//
	//MSG_SMS_RESERVED1 = 558,			//
	//MSG_SMS_RESERVED1 = 559,			//
	//MSG_SMS_RESERVED1 = 560,			//

	MSG_SMS_RF_TUNE_REQ=561,			// Application: CMMB, DVBT/H 
										// A request to tune to a new frequency
										// Format:	32-bit - Frequency in Hz
										//			32-bit - Bandwidth (in CMMB always use BW_8_MHZ)
										//			32-bit - Crystal (Use 0 for default, always 0 in CMMB)
										// Direction: Host->SMS

	MSG_SMS_RF_TUNE_RES=562,			// Application: CMMB, DVBT/H 
										// A response to MSG_SMS_RF_TUNE_REQ
										// In DVBT/H this only indicates that the tune request
										// was received.
										// In CMMB, the response returns after the demod has determined
										// if there is a valid CMMB transmission on the frequency
										//
										// Format:
										//	DVBT/H:
										//		32-bit Return status. Should be SMSHOSTLIB_ERR_OK.
										//	CMMB:
										//		32-bit CMMB signal status - SMSHOSTLIB_ERR_OK means that the 
										//					frequency has a valid CMMB signal
										// 
										// Direction: SMS->Host

	//MSG_SMS_RESERVED1 = 563,			//
	//MSG_SMS_RESERVED1 = 564,			//
	//MSG_SMS_RESERVED1 = 565,			//
	//MSG_SMS_RESERVED1 = 566,			//
	//MSG_SMS_RESERVED1 = 567,			//
	//MSG_SMS_RESERVED1 = 568,			//
	//MSG_SMS_RESERVED1 = 569,			// 
	//MSG_SMS_RESERVED1 = 570,			// 

	MSG_SMS_EEPROM_WRITE_REQ=571,		// A request to program the EEPROM
										// Format:	32-bit - Section status indication (0-first,running index,0xFFFFFFFF -last)
										//			32-bit - (optional) Image CRC or checksum
										//			32-bit - Total image length, in bytes, immediately following this DWORD
										//			32-bit - Actual section length, in bytes, immediately following this DWORD
										// Direction: Host->SMS

	MSG_SMS_EEPROM_WRITE_RES=572,		// The status response to MSG_SMS_EEPROM_WRITE_REQ
										// Format:	32-bit of the response
										// Direction: SMS->Host

	//MSG_SMS_RESERVED1 =573, 			// 
	//MSG_SMS_RESERVED1 =574,			// 
	//MSG_SMS_RESERVED1 =575,			// 
	//MSG_SMS_RESERVED1 =576,			// 
	//MSG_SMS_RESERVED1 =577,			//

	MSG_SMS_INIT_DEVICE_REQ=578,		// A request to init device
										// Format: 32-bit - device mode (DVBT,DVBH,TDMB,DAB)
										//		   32-bit - Crystal
										//		   32-bit - Clk Division
										//		   32-bit - Ref Division
										// Direction: Host->SMS

	MSG_SMS_INIT_DEVICE_RES=579,		// The response to MSG_SMS_INIT_DEVICE_REQ
										// Format:	32-bit - status
										// Direction: SMS->Host

	//MSG_SMS_RESERVED1 =580, 			//
	//MSG_SMS_RESERVED1 =581,			//
	//MSG_SMS_RESERVED1 =582,			//
	//MSG_SMS_RESERVED1 =583,			//
	//MSG_SMS_RESERVED1 =584,			// 
	//MSG_SMS_RESERVED1 =585,			// 
	//MSG_SMS_RESERVED1 =586,			//
	//MSG_SMS_RESERVED1 =587,			//
	//MSG_SMS_RESERVED1 =588,			//

	MSG_SMS_SUB_CHANNEL_START_REQ =589,	// DAB
	MSG_SMS_SUB_CHANNEL_START_RES =590,	// DAB

	MSG_SMS_SUB_CHANNEL_STOP_REQ =591,	// DAB
	MSG_SMS_SUB_CHANNEL_STOP_RES =592,	// DAB

	//MSG_SMS_RESERVED1 =593,			// 
	//MSG_SMS_RESERVED1 =594,			// 
	//MSG_SMS_RESERVED1 =595,			// 
	//MSG_SMS_RESERVED1 =596,			// 
	//MSG_SMS_RESERVED1 =597,			// 
	//MSG_SMS_RESERVED1 =598,			// 

	MSG_SMS_WAIT_CMD =599,				// Internal (SmsMonitor) message
										// Format: Name of script(file) to run, immediately following header
										// direction: N/A
	//MSG_SMS_RESERVED1 = 600,			// 

	MSG_SMS_ADD_PID_FILTER_REQ=601,		// Application: DVB-T/DVB-H
										// Add PID to filter list
										// Format: 32-bit PID
										// Direction: Host->SMS

	MSG_SMS_ADD_PID_FILTER_RES=602,		// Application: DVB-T/DVB-H
										// The response to MSG_SMS_ADD_PID_FILTER_REQ
										// Format:	32-bit - Status
										// Direction: SMS->Host

	MSG_SMS_REMOVE_PID_FILTER_REQ=603,	// Application: DVB-T/DVB-H
										// Remove PID from filter list
										// Format: 32-bit PID
										// Direction: Host->SMS

	MSG_SMS_REMOVE_PID_FILTER_RES=604,	// Application: DVB-T/DVB-H
										// The response to MSG_SMS_REMOVE_PID_FILTER_REQ
										// Format:	32-bit - Status
										// Direction: SMS->Host

	MSG_SMS_FAST_INFORMATION_CHANNEL_REQ=605,// Application: DAB
										     // A request for a of a Fast Information Channel (FIC)
											 // Direction: Host->SMS

	MSG_SMS_FAST_INFORMATION_CHANNEL_RES=606,// Application: DAB
										     // Forwarding of a Fast Information Channel (FIC)
											 // Format:	Sequence counter and FIC bytes with Fast Information Blocks { FIBs  as described in "ETSI EN 300 401 V1.3.3 (2001-05)":5.2.1 Fast Information Block (FIB))
											 // Direction: SMS->Host

	MSG_SMS_DAB_CHANNEL=607,			// Application: All
										// Forwarding of a played channel
										// Format:	H.264
										// Direction: SMS->Host

	MSG_SMS_GET_PID_FILTER_LIST_REQ=608,// Application: DVB-T
										// Request to get current PID filter list
										// Format: None
										// Direction: Host->SMS

	MSG_SMS_GET_PID_FILTER_LIST_RES=609,// Application: DVB-T
										// The response to MSG_SMS_GET_PID_FILTER_LIST_REQ
										// Format:	array of 32-bit of PIDs
										// Direction: SMS->Host
	//MSG_SMS_RESERVED1 = 610,			// 
	//MSG_SMS_RESERVED1 = 611,    		//
	//MSG_SMS_RESERVED1 = 612, 			//
	//MSG_SMS_RESERVED1 = 613,			//
	//MSG_SMS_RESERVED1 = 614,			//

	MSG_SMS_GET_STATISTICS_REQ=615,		// Application: DVB-T / DAB
										// Request statistics information 
										// In DVB-T uses only at the driver level (BDA)
										// Direction: Host->FW

	MSG_SMS_GET_STATISTICS_RES=616,		// Application: DVB-T / DAB
										// The response to MSG_SMS_GET_STATISTICS_REQ
										// Format:	SmsMsgStatisticsInfo_ST
										// Direction: SMS->Host

	MSG_SMS_SEND_DUMP=617,				// uses for - Dump msgs
										// Direction: SMS->Host

	MSG_SMS_SCAN_START_REQ=618,			// Application: CMMB
										// Start Scan
										// Format:
										//			32-bit - Bandwidth
										//			32-bit - Scan Flags
										//			32-bit - Param Type
										// In CMMB Param type must be 0 - because of CMRI spec, 
										//	and only range is supported.
										//
										// In other standards:
										// If Param Type is SCAN_PARAM_TABLE:
										//			32-bit - Number of frequencies N
										//			N*32-bits - List of frequencies
										// If Param Type is SCAN_PARAM_RANGE:
										//			32-bit - Start Frequency
										//			32-bit - Gap between frequencies
										//			32-bit - End Frequency
										// Direction: Host->SMS

	MSG_SMS_SCAN_START_RES=619,			// Application: CMMB
										// Scan Start Reply
										// Format:	32-bit - ACK/NACK
										// Direction: SMS->Host

	MSG_SMS_SCAN_STOP_REQ=620,			// Application: CMMB
										// Stop Scan
										// Direction: Host->SMS

	MSG_SMS_SCAN_STOP_RES=621,			// Application: CMMB
										// Scan Stop Reply
										// Format:	32-bit - ACK/NACK
										// Direction: SMS->Host

	MSG_SMS_SCAN_PROGRESS_IND=622,		// Application: CMMB
										// Scan progress indications
										// Format:
										//		32-bit RetCode: SMSHOSTLIB_ERR_OK means that the frequency is Locked
										//		32-bit Current frequency 
										//		32-bit Number of frequencies remaining for scan
										//		32-bit NetworkID of the current frequency - if locked. If not locked - 0.
										
	MSG_SMS_SCAN_COMPLETE_IND=623,		// Application: CMMB
										// Scan completed
										// Format: Same as SCAN_PROGRESS_IND

	MSG_SMS_LOG_ITEM = 624,             // Application: All
										// Format:	SMSHOSTLIB_LOG_ITEM_ST.
										// Actual size depend on the number of parameters
										// Direction: Host->SMS

	//MSG_SMS_RESERVED1 = 625,			//
	//MSG_SMS_RESERVED1 = 626,			//
	//MSG_SMS_RESERVED1 = 627,			//  

	MSG_SMS_DAB_SUBCHANNEL_RECONFIG_REQ = 628,	// Application: DAB
	MSG_SMS_DAB_SUBCHANNEL_RECONFIG_RES = 629,	// Application: DAB

	// Handover - start (630)
	MSG_SMS_HO_PER_SLICES_IND		= 630,		// Application: DVB-H 
												// Direction: FW-->Host

	MSG_SMS_HO_INBAND_POWER_IND		= 631,		// Application: DVB-H 
												// Direction: FW-->Host

	MSG_SMS_MANUAL_DEMOD_REQ		= 632,		// Application: DVB-H
												// Debug msg 
												// Direction: Host-->FW

	//MSG_SMS_HO_RESERVED1_RES		= 633,		// Application: DVB-H  
	//MSG_SMS_HO_RESERVED2_RES		= 634,		// Application: DVB-H  
	//MSG_SMS_HO_RESERVED3_RES		= 635,		// Application: DVB-H 

	MSG_SMS_HO_TUNE_ON_REQ			= 636,		// Application: DVB-H  
	MSG_SMS_HO_TUNE_ON_RES			= 637,		// Application: DVB-H  
	MSG_SMS_HO_TUNE_OFF_REQ			= 638,		// Application: DVB-H 	
	MSG_SMS_HO_TUNE_OFF_RES			= 639,		// Application: DVB-H  
	MSG_SMS_HO_PEEK_FREQ_REQ		= 640,		// Application: DVB-H 
	MSG_SMS_HO_PEEK_FREQ_RES		= 641,		// Application: DVB-H  
	MSG_SMS_HO_PEEK_FREQ_IND		= 642,		// Application: DVB-H 
	// Handover - end (642)

	//MSG_SMS_RESERVED1				= 643,		//
	//MSG_SMS_RESERVED1				= 644,		//
	//MSG_SMS_RESERVED1				= 645,		//
	//MSG_SMS_RESERVED1				= 646,		//						
	//MSG_SMS_RESERVED1				= 647,		//	
	//MSG_SMS_RESERVED1				= 648,		//

	MSG_SMS_ENABLE_STAT_IN_I2C_REQ = 649,		// Application: DVB-T (backdoor)
												// Enable async statistics in I2C polling 
												// Direction: Host->FW

	MSG_SMS_ENABLE_STAT_IN_I2C_RES = 650,		// Application: DVB-T
												// Response to MSG_SMS_ENABLE_STAT_IN_I2C_REQ
												// Format: N/A
												// Direction: FW->Host

	MSG_SMS_GET_STATISTICS_EX_REQ   = 653,		// Application: ISDBT / FM
												// Request for statistics 
												// Direction: Host-->FW

	MSG_SMS_GET_STATISTICS_EX_RES   = 654,		// Application: ISDBT / FM
												// Format:
												// 32 bit ErrCode
												// The rest: A mode-specific statistics struct starting
												// with a 32 bits type field.
												// Direction: FW-->Host

	MSG_SMS_SLEEP_RESUME_COMP_IND	= 655,		// Application: All
												// Indicates that a resume from sleep has been completed
												// Uses for Debug only
												// Direction: FW-->Host

	//MSG_SMS_RESERVED1				= 656,		// 
	//MSG_SMS_RESERVED1				= 657,		// 

	MSG_SMS_DATA_DOWNLOAD_REQ		= 660,		// Application: All
												// Direction: Host-->FW

	MSG_SMS_DATA_DOWNLOAD_RES		= 661,		// Application: All
												// Direction: FW-->Host

	MSG_SMS_DATA_VALIDITY_REQ		= 662,		// Application: All
												// Direction: Host-->FW
												
	MSG_SMS_DATA_VALIDITY_RES		= 663,		// Application: All
												// Direction: FW-->Host
												
	MSG_SMS_SWDOWNLOAD_TRIGGER_REQ	= 664,		// Application: All
												// Direction: Host-->FW
												
	MSG_SMS_SWDOWNLOAD_TRIGGER_RES	= 665,		// Application: All
												// Direction: FW-->Host

	MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ	= 666,		// Application: All
												// Direction: Host-->FW
	
	MSG_SMS_SWDOWNLOAD_BACKDOOR_RES	= 667,		// Application: All
												// Direction: FW-->Host

	MSG_SMS_GET_VERSION_EX_REQ		= 668,		// Application: All Except CMMB
												// Direction: Host-->FW

	MSG_SMS_GET_VERSION_EX_RES		= 669,		// Application: All Except CMMB
												// Direction: FW-->Host

	MSG_SMS_CLOCK_OUTPUT_CONFIG_REQ = 670,		// Application: All 
												// Request to clock signal output from SMS
												// Format: 32-bit - Enable/Disable clock signal
												//         32-bit - Requested clock frequency
												// Direction: Host-->FW

	MSG_SMS_CLOCK_OUTPUT_CONFIG_RES = 671,		// Application: All
												// Response to clock signal output config request
												// Format: 32-bit - Status
												// Direction: FW-->Host

	MSG_SMS_I2C_SET_FREQ_REQ		= 685,		// Application: All 
												// Request to start I2C configure with new clock Frequency
												// Format: 32-bit - Requested clock frequency
												// Direction: Host-->FW

	MSG_SMS_I2C_SET_FREQ_RES		= 686,		// Application: All 
												// Response to MSG_SMS_I2C_SET_FREQ_REQ
												// Format: 32-bit - Status
												// Direction: FW-->Host

	MSG_SMS_GENERIC_I2C_REQ			= 687,		// Application: All 
												// Request to write buffer through I2C
												// Format: 32-bit - device address
												//		   32-bit - write size
												//		   32-bit - requested read size
												//		   n * 8-bit - write buffer

	MSG_SMS_GENERIC_I2C_RES			= 688,		// Application: All 
												// Response to MSG_SMS_GENERIC_I2C_REQ
												// Format: 32-bit - Status
												//		   32-bit - read size
												//         n * 8-bit - read data

	//MSG_SMS_RESERVED1				= 689,		// 
	//MSG_SMS_RESERVED1				= 690,		// 
	//MSG_SMS_RESERVED1				= 691,		// 
	//MSG_SMS_RESERVED1				= 692,		// 

	MSG_SMS_DVBT_BDA_DATA			= 693,		// Application: All (BDA)
												// Direction: FW-->Host
												
	//MSG_SMS_RESERVED1				= 694,		//
	//MSG_SMS_RESERVED1				= 695,		//
	//MSG_SMS_RESERVED1				= 696,		//
	//MSG_SMS_RESERVED1				= 697,		// 
	//MSG_SMS_RESERVED1				= 698,		//

	MSG_SMS_DATA_MSG				= 699,		// Application: All
												// Direction: FW-->Host

	///  NOTE: Opcodes targeted for Stellar cannot exceed 700
	MSG_TABLE_UPLOAD_REQ			= 700,		// Request for PSI/SI tables in DVB-H
												// Format: 
												// Direction Host->SMS

	MSG_TABLE_UPLOAD_RES			= 701,		// Reply to MSG_TABLE_UPLOAD_REQ
												// Format: 
												// Direction SMS->Host

	// reload without reseting the interface
	MSG_SW_RELOAD_START_REQ			= 702,		// Request to prepare to reload 
	MSG_SW_RELOAD_START_RES			= 703,		// Response to 
	MSG_SW_RELOAD_EXEC_REQ			= 704,		// Request to start reload
	MSG_SW_RELOAD_EXEC_RES			= 705,		// Response to MSG_SW_RELOAD_EXEC_REQ

	//MSG_SMS_RESERVED1				= 706,		//
	//MSG_SMS_RESERVED1				= 707,		//
	//MSG_SMS_RESERVED1				= 708,		//
	//MSG_SMS_RESERVED1				= 709,		//

	MSG_SMS_SPI_INT_LINE_SET_REQ	= 710,		//
	MSG_SMS_SPI_INT_LINE_SET_RES	= 711,		//

	MSG_SMS_GPIO_CONFIG_EX_REQ		= 712,		//
	MSG_SMS_GPIO_CONFIG_EX_RES		= 713,		//

	//MSG_SMS_RESERVED1				= 714,		//
	//MSG_SMS_RESERVED1  			= 715,		//

	MSG_SMS_WATCHDOG_ACT_REQ		= 716,		//
	MSG_SMS_WATCHDOG_ACT_RES		= 717,		//

	MSG_SMS_LOOPBACK_REQ			= 718,		//
	MSG_SMS_LOOPBACK_RES			= 719,		//  

	MSG_SMS_RAW_CAPTURE_START_REQ	= 720,  	//
	MSG_SMS_RAW_CAPTURE_START_RES	= 721,  	//

	MSG_SMS_RAW_CAPTURE_ABORT_REQ	= 722,  	//
	MSG_SMS_RAW_CAPTURE_ABORT_RES	= 723,  	//

	//MSG_SMS_RESERVED1				=  724,  	//
	//MSG_SMS_RESERVED1				=  725,  	//
	//MSG_SMS_RESERVED1				=  726,  	// 
	//MSG_SMS_RESERVED1				=  727,  	//

	MSG_SMS_RAW_CAPTURE_COMPLETE_IND = 728, 	//

	MSG_SMS_DATA_PUMP_IND			= 729,  	// USB debug - _TEST_DATA_PUMP 
	MSG_SMS_DATA_PUMP_REQ			= 730,  	// USB debug - _TEST_DATA_PUMP 
	MSG_SMS_DATA_PUMP_RES			= 731,  	// USB debug - _TEST_DATA_PUMP 

	MSG_SMS_FLASH_DL_REQ			= 732,		// A request to program the FLASH
												// Format:	32-bit - Section status indication (0-first,running index,0xFFFFFFFF -last)
												//			32-bit - (optional) Image CRC or checksum
												//			32-bit - Total image length, in bytes, immediately following this DWORD
												//			32-bit - Actual section length, in bytes, immediately following this DWORD
												// Direction: Host->SMS

	MSG_SMS_FLASH_DL_RES			= 733,		// The status response to MSG_SMS_FLASH_DL_REQ
												// Format:	32-bit of the response
												// Direction: SMS->Host

	MSG_SMS_EXEC_TEST_1_REQ			= 734,		// USB debug - _TEST_DATA_PUMP 
	MSG_SMS_EXEC_TEST_1_RES			= 735,  	// USB debug - _TEST_DATA_PUMP 

	MSG_SMS_ENBALE_TS_INTERFACE_REQ	= 736,		// A request set TS interface as the DATA(!) output interface
												// Format:	32-bit - Requested Clock speed in Hz(0-disable)
												//			32-bit - transmission mode (Serial or Parallel)
												// Direction: Host->SMS

	MSG_SMS_ENBALE_TS_INTERFACE_RES	= 737,  	//

	MSG_SMS_SPI_SET_BUS_WIDTH_REQ	= 738,  	//
	MSG_SMS_SPI_SET_BUS_WIDTH_RES	= 739,  	//

	//MSG_SMS_RESERVED1				= 740,  	//  
	//MSG_SMS_RESERVED1				= 741,  	// 

	MSG_SMS_DISABLE_TS_INTERFACE_REQ = 742, 	//
	MSG_SMS_DISABLE_TS_INTERFACE_RES = 743, 	//

	//MSG_SMS_RESERVED1				= 744,    	//
	//MSG_SMS_RESERVED1				= 745,    	//

	MSG_SMS_EXT_ANTENNA_REQ			= 746,  	//Activate external antenna search algorithm 
	MSG_SMS_EXT_ANTENNA_RES			= 747,  	//confirmation 

	MSG_SMS_CMMB_GET_NET_OF_FREQ_REQ_OBSOLETE= 748,		// Obsolete
	MSG_SMS_CMMB_GET_NET_OF_FREQ_RES_OBSOLETE= 749,	    // Obsolete

	//MSG_SMS_RESERVED1				= 750,		//
	//MSG_SMS_RESERVED1				= 751,		//

	MSG_SMS_CMMB_INJECT_TABLE_REQ_OBSOLETE	= 752,		// Obsolete
	MSG_SMS_CMMB_INJECT_TABLE_RES_OBSOLETE	= 753,		// Obsolete
	
	MSG_SMS_FM_RADIO_BLOCK_IND		= 754,		// Application: FM_RADIO
												// Description: RDS blocks
												// Format: Data[0] = 	
												// Direction: FW-->Host

	MSG_SMS_HOST_NOTIFICATION_IND 	= 755,		// Application: CMMB
												// Description: F/W notification to host
												// Data[0]:	SMSHOSTLIB_CMMB_HOST_NOTIFICATION_TYPE_ET
												// Direction: FW-->Host

	MSG_SMS_CMMB_GET_CONTROL_TABLE_REQ_OBSOLETE	= 756,	// Obsolete
	MSG_SMS_CMMB_GET_CONTROL_TABLE_RES_OBSOLETE = 757,	// Obsolete


	//MSG_SMS_RESERVED1				= 758,	// 
	//MSG_SMS_RESERVED1				= 759,	// 

	MSG_SMS_CMMB_GET_NETWORKS_REQ	= 760,	// Data[0]: Reserved - has to be 0
	MSG_SMS_CMMB_GET_NETWORKS_RES	= 761,	// Data[0]: RetCode
											// Data[1]: Number of networks (N)
											// Followed by N * SmsCmmbNetworkInfo_ST

	MSG_SMS_CMMB_START_SERVICE_REQ	= 762,	// Data[0]: UINT32 Reserved 0xFFFFFFFF (was NetworkLevel)
											// Data[1]: UINT32 Reserved 0xFFFFFFFF (was NetworkNumber)
											// Data[2]: UINT32 ServiceId

	MSG_SMS_CMMB_START_SERVICE_RES	= 763,	// Data[0]: UINT32 RetCode
											// Data[1]: UINT32 ServiceHandle
											// Data[2]: UINT32 Service sub frame index
											//		The index of the sub frame that contains the service
											//      inside the multiplex frame. Usually 0.
											// Data[1]: UINT32 Service ID
											//		The started service ID 

	MSG_SMS_CMMB_STOP_SERVICE_REQ	= 764,	// Data[0]: UINT32 ServiceHandle
	MSG_SMS_CMMB_STOP_SERVICE_RES	= 765,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_ADD_CHANNEL_FILTER_REQ		= 768,	// Data[0]: UINT32 Channel ID
	MSG_SMS_CMMB_ADD_CHANNEL_FILTER_RES		= 769,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_REQ	= 770,	// Data[0]: UINT32 Channel ID
	MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_RES	= 771,	// Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_START_CONTROL_INFO_REQ		= 772,	// Format:	
													// Data[0]: UINT32 Reserved 0xFFFFFFFF (was NetworkLevel)
													// Data[1]: UINT32 Reserved 0xFFFFFFFF (was NetworkNumber)

	MSG_SMS_CMMB_START_CONTROL_INFO_RES		= 773,	// Format:	Data[0]: UINT32 RetCode

	MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ		= 774,	// Format: No Payload
	MSG_SMS_CMMB_STOP_CONTROL_INFO_RES		= 775,	// Format: Data[0]: UINT32 RetCode

	MSG_SMS_ISDBT_TUNE_REQ			= 776,	// Application Type: ISDB-T
											// Description: A request to tune to a new frequency
											// Format:	Data[0]:	UINT32 Frequency
											//			Data[1]:	UINT32 Bandwidth
											//			Data[2]:	UINT32 Crystal
											//			Data[3]:	UINT32 Segment number
											// Direction: Host->SMS

	MSG_SMS_ISDBT_TUNE_RES			= 777,	// Application Type: ISDB-T
											// Data[0]:	UINT32 RetCode
											// Direction: SMS->Host

	//MSG_SMS_RESERVED1		        = 778,	// 
	//MSG_SMS_RESERVED1             = 779,	// 
	//MSG_SMS_RESERVED1				= 780,	// 
	//MSG_SMS_RESERVED1         	= 781,	// 

	MSG_SMS_TRANSMISSION_IND		= 782,  // Application Type: DVB-T/DVB-H 
											// Description: Send statistics info using the following structure:
											// TRANSMISSION_STATISTICS_ST
											//	 Data[0] = UINT32 Frequency																
											//   Data[1] = UINT32 Bandwidth				
											//   Data[2] = UINT32 TransmissionMode		
											//   Data[3] = UINT32 GuardInterval			
											//   Data[4] = UINT32 CodeRate				
											//   Data[5] = UINT32 LPCodeRate				
											//   Data[6] = UINT32 Hierarchy				
											//   Data[7] = UINT32 Constellation			
											//   Data[8] = UINT32 CellId					
											//   Data[9] = UINT32 DvbhSrvIndHP			
											//   Data[10]= UINT32 DvbhSrvIndLP			
											//   Data[11]= UINT32 IsDemodLocked			
											// Direction: FW-->Host
												
	MSG_SMS_PID_STATISTICS_IND		= 783,	// Application Type: DVB-H 
											// Description: Send PID statistics info using the following structure:
											// PID_DATA_ST
											//	 Data[0] = UINT32 pid
											//   Data[1] = UINT32 num rows 
											//   Data[2] = UINT32 size  
											//   Data[3] = UINT32 padding_cols
											//   Data[4] = UINT32 punct_cols
											//   Data[5] = UINT32 duration
											//   Data[6] = UINT32 cycle
											//   Data[7] = UINT32 calc_cycle
											//   Data[8] = UINT32 tot_tbl_cnt 
											//   Data[9] = UINT32 invalid_tbl_cnt 
											//   Data[10]= UINT32 tot_cor_tbl
											// Direction: FW-->Host

	MSG_SMS_POWER_DOWN_IND			= 784,	// Application Type: DVB-H 
											// Description: Indicates start of the power down to sleep mode procedure
											//  data[0] - requestId, 
											//  data[1] - message quarantine time
											// Direction: FW-->Host

	MSG_SMS_POWER_DOWN_CONF			= 785,	// Application Type: DVB-H 
											// Description: confirms the power down procedure, 
											// data[0] - requestId, 
											// data[1] - quarantine time
											// Direction: Host-->FW 

	MSG_SMS_POWER_UP_IND			= 786,	// Application Type: DVB-H 
											// Description: Indicates end of sleep mode,       
											// data[0] - requestId
											// Direction: FW-->Host

	MSG_SMS_POWER_UP_CONF			= 787,	// Application Type: DVB-H 
											// Description: confirms the end of sleep mode,    
											// data[0] - requestId
											// Direction: Host-->FW 

	//MSG_SMS_RESERVED1             = 788,	//
	//MSG_SMS_RESERVED1				= 789,	//

	MSG_SMS_POWER_MODE_SET_REQ		= 790,	// Application: DVB-H 
											// Description: set the inter slice power down (sleep) mode (Enable/Disable)
											// Format: Data[0] = UINT32 sleep mode
											// Direction: Host-->FW 

	MSG_SMS_POWER_MODE_SET_RES		= 791,	// Application: DVB-H
											// Description: response to the previous request
											// Direction: FW-->Host

	MSG_SMS_DEBUG_HOST_EVENT_REQ	= 792,	// Application: CMMB (Internal) 
											// Description: An opaque event host-> FW for debugging internal purposes (CMMB)
											// Format:	data[0] = Event type (enum)
											//			data[1] = Param

	MSG_SMS_DEBUG_HOST_EVENT_RES	= 793,	// Application: CMMB (Internal)
											// Description: Response. 
											// Format:  data[0] = RetCode, 
											//			data[1] = RetParam


	MSG_SMS_NEW_CRYSTAL_REQ			= 794,	// Application: All 
											// report crystal input to FW
											// Format:  data[0] = UINT32 crystal 
											// Direction: Host-->FW 

	MSG_SMS_NEW_CRYSTAL_RES			= 795,  // Application Type: All 
											// Response to MSG_SMS_NEW_CRYSTAL_REQ
											// Direction: FW-->Host

	MSG_SMS_CONFIG_SPI_REQ			= 796,	// Application: All 
											// Configure SPI interface (also activates I2C slave interface)
											// Format:	data[0] = SPI Controller (UINT32)
											//			data[1] = SPI Mode - Master/Slave (UINT32)
											//			data[2] = SPI Type - Mot/TI (UINT32)
											//			data[3] = SPI Width - 8bit/32bit (UINT32)
											//			data[4] = SPI Clock - in Hz (UINT32)
											// Direction: Host-->FW

	MSG_SMS_CONFIG_SPI_RES			= 797,	// Application: All 
											// Response to MSG_SMS_CONFIG_SPI_RES
											// Direction: FW-->Host

	MSG_SMS_I2C_SHORT_STAT_IND		= 798,	// Application Type: DVB-T/ISDB-T 
											// Format: ShortStatMsg_ST
											//		Data[0] = UINT16 msgType
											//		Data[1] = UINT8	msgSrcId
											//		Data[2] = UINT8	msgDstId
											//		Data[3] = UINT16	msgLength	
											//		Data[4] = UINT16	msgFlags
											//  The following parameters relevant in DVB-T only - in isdb-t should be Zero
											//		Data[5] = UINT32 IsDemodLocked;
											//		Data[6] = UINT32 InBandPwr;
											//		Data[7] = UINT32 BER;
											//		Data[8] = UINT32 SNR;
											//		Data[9] = UINT32 TotalTsPackets;
											//		Data[10]= UINT32 ErrorTSPackets;
											// Direction: FW-->Host

	MSG_SMS_START_IR_REQ			= 800,  // Application: All
											// Description: request to start sampling IR controller
											// Format: Data[0] = irController;
											//		   Data[1] = irTimeout;
											// Direction: Host-->FW

	MSG_SMS_START_IR_RES			= 801,  // Application: All
											// Response to MSG_SMS_START_IR_REQ
											// Direction: FW-->Host

	MSG_SMS_IR_SAMPLES_IND			= 802,  // Application: All
											// Send IR samples to Host
											// Format: Data[] = 128 * UINT32 
											// Direction: FW-->Host
	
	MSG_SMS_CMMB_CA_SERVICE_IND		= 803,	// Format:	UINT32 data[0] UINT32 Indication type, according to
											//					SmsCaServiceIndicationTypes_EN enum
											//			UINT32 data[1] UINT32 Service ID

	MSG_SMS_SLAVE_DEVICE_DETECTED	= 804,  // Application: DVB-T MRC
											// Description: FW indicate that Slave exist in MRC - DVB-T application
											// Direction: FW->Host

	MSG_SMS_INTERFACE_LOCK_IND		= 805,	// Application: All
											// Description: firmware requests that the host does not transmit anything on the interface
											// Direction: FW->Host

	MSG_SMS_INTERFACE_UNLOCK_IND	= 806,	// Application: All
											// Description: firmware signals that the host may resume transmission
											// Direction: FW->Host

	//MSG_SMS_RESERVED1				= 807,	// 
	//MSG_SMS_RESERVED1				= 808,	// 
	//MSG_SMS_RESERVED1				= 809,	//

	MSG_SMS_SEND_ROSUM_BUFF_REQ		= 810,  // Application: Rosum
											// Description: Host send buffer to Rosum internal module in FW 
											// Format: msg structure is proprietary to rosum, size can be up to 240
											// Direction: Host-->FW

	MSG_SMS_SEND_ROSUM_BUFF_RES		= 811,  // Application: Rosum
											// Response to MSG_SMS_SEND_ROSUM_BUFF_RES
											// Direction: FW->Host

	MSG_SMS_ROSUM_BUFF				= 812,  // Application: Rosum
											// Description: Rosum internal module in FW  send buffer to Host
											// Format: msg structure is proprietary to rosum, size can be up to 240
											// Direction: FW->Host

	//MSG_SMS_RESERVED1				= 813,	// 
	//MSG_SMS_RESERVED1				= 814,	// 

	MSG_SMS_SET_AES128_KEY_REQ		= 815,  // Application: ISDB-T
											// Description: Host send key for AES128
											// Format: String
											// Direction: Host-->FW

	MSG_SMS_SET_AES128_KEY_RES		= 816,  // Application: ISDB-T
											// Description: response to MSG_SMS_SET_AES128_KEY_REQ
											// Direction: FW-->Host

	MSG_SMS_MBBMS_WRITE_REQ			= 817,	// MBBMS-FW communication message - downstream
	MSG_SMS_MBBMS_WRITE_RES			= 818,	// MBBMS-FW communication message - downstream response
	MSG_SMS_MBBMS_READ_IND			= 819,	// MBBMS-FW communication message - upstream

	MSG_SMS_IQ_STREAM_START_REQ		= 820,  // Application: Streamer
	MSG_SMS_IQ_STREAM_START_RES		= 821,  // Application: Streamer
	MSG_SMS_IQ_STREAM_STOP_REQ		= 822,  // Application: Streamer
	MSG_SMS_IQ_STREAM_STOP_RES		= 823,  // Application: Streamer
	MSG_SMS_IQ_STREAM_DATA_BLOCK	= 824,  // Application: Streamer

	MSG_SMS_GET_EEPROM_VERSION_REQ  = 825,	// Request to get EEPROM version string

	MSG_SMS_GET_EEPROM_VERSION_RES  = 826,	// Response to get EEPROM version string request
											// Format: 32-bit - Status
											//         32-bit - Length of string
											//         N*bytes - EEPROM version string

	MSG_SMS_SIGNAL_DETECTED_IND		= 827,  // Application: DVB-T/ISDB-T/TDMB
											// Description: Indication on good signal - after Tune 
											// Direction: FW-->Host

	MSG_SMS_NO_SIGNAL_IND			= 828,  // Application: DVB-T/ISDB-T/TDMB
											// Description: Indication on bad signal - after Tune 
											// Direction: FW-->Host

	//MSG_SMS_RESERVED1				= 829,	//	

	MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ	= 830,	// Application: DVB-T MRC
											// Description: Power down MRC slave to save power
											// Direction: Host-->FW

	MSG_SMS_MRC_SHUTDOWN_SLAVE_RES	= 831,	// Application: DVB-T MRC
											// Description: response to MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ 
											// Direction: FW-->Host

	MSG_SMS_MRC_BRINGUP_SLAVE_REQ	= 832,	// Application: DVB-T MRC
											// Description: Return back the MRC slave to operation
											// Direction: Host-->FW

	MSG_SMS_MRC_BRINGUP_SLAVE_RES	= 833,  // Application: DVB-T MRC
											// Description: response to MSG_SMS_MRC_BRINGUP_SLAVE_REQ 
											// Direction: FW-->Host

	MSG_SMS_EXTERNAL_LNA_CTRL_REQ   = 834,  // APPLICATION: DVB-T 
											// Description: request from driver to control external LNA
											// Direction: Host-->FW

	MSG_SMS_EXTERNAL_LNA_CTRL_RES   = 835,  // APPLICATION: DVB-T 
											// Description: response to MSG_SMS_EXTERNAL_LNA_CTRL_REQ
											// Direction: FW-->Host

	MSG_SMS_SET_PERIODIC_STATISTICS_REQ		= 836,	// Application: CMMB
													// Description: Enable/Disable periodic statistics.
													// Format:	32 bit enable flag. 0 - Disable, 1- Enable 
													// Direction: Host-->FW

	MSG_SMS_SET_PERIODIC_STATISTICS_RES		= 837,  // Application: CMMB
													// Description: response to MSG_SMS_SET_PERIODIC_STATISTICS_REQ 
													// Direction: FW-->Host

	MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ	= 838,	// Application: CMMB
													// Description: Enable/Disable auto output of TS0
													// Format: 32 bit enable flag. 0 - Disable, 1- Enable 
													// Direction: Host-->FW

	MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_RES	= 839,  // Application: CMMB
													// Description: response to MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ 
													// Direction: FW-->Host

	LOCAL_TUNE						= 850,	// Application: DVB-T (Internal)
											// Description: Internal message sent by the demod after tune/resync
											// Direction: FW-->FW	

	LOCAL_IFFT_H_ICI				= 851,  // Application: DVB-T (Internal)
											// Direction: FW-->FW

	MSG_RESYNC_REQ					= 852,	// Application: DVB-T (Internal)
											// Description: Internal resync request used by the MRC master
											// Direction: FW-->FW

	MSG_SMS_CMMB_GET_MRC_STATISTICS_REQ		= 853,	// Application: CMMB (Internal)
													// Description: MRC statistics request (internal debug, not exposed to users)
													// Direction: Host-->FW
	MSG_SMS_CMMB_GET_MRC_STATISTICS_RES		= 854,	// Description: MRC statistics response (internal debug, not exposed to users)
													// Direction: FW-->Host

	MSG_SMS_LOG_EX_ITEM				= 855,  // Application: All
											// Format:	32-bit - number of log messages
											//			followed by N  SMSHOSTLIB_LOG_ITEM_ST  
											// Direction: FW-->Host

	MSG_SMS_DEVICE_DATA_LOSS_IND	= 856,  // Application: LBS
											// Description: Indication on data loss on the device level
											// Direction: FW-->Host

	//MSG_SMS_RESERVED1				= 857,  // 

	MSG_SMS_USER_MSG_REQ			= 858,  // Application: All
											// Description: Data message for Data Cards internal 
											// Direction: Host-->Data card 

	MSG_SMS_USER_MSG_RES			= 859,  // Application: All 
											// Data message response from Data card to host.
											// Direction: Data card-->Host

	MSG_SMS_SMART_CARD_INIT_REQ		= 860, 	// ISO-7816 SmartCard access routines
	MSG_SMS_SMART_CARD_INIT_RES		= 861,  //
	MSG_SMS_SMART_CARD_WRITE_REQ	= 862,  //
	MSG_SMS_SMART_CARD_WRITE_RES	= 863,  //
	MSG_SMS_SMART_CARD_READ_IND		= 864,  //

	MSG_SMS_TSE_ENABLE_REQ			= 866,	// Application: DVB-T/ISDB-T 
											// Description: Send this command in case the Host wants to handle TS with Error Bit enable
											// Direction: Host-->FW

	MSG_SMS_TSE_ENABLE_RES			= 867,	// Application: DVB-T/ISDB-T 
											// Description: Response to MSG_SMS_TSE_ENABLE_REQ 
											// Direction: FW-->Host

	MSG_SMS_CMMB_GET_SHORT_STATISTICS_REQ	= 868,  // Application: CMMB
													// Description: Short statistics for CMRI standard.
													// Direction: Host-->FW
													// supported only in Venice

	MSG_SMS_CMMB_GET_SHORT_STATISTICS_RES	= 869,  // Description: Short statistics response
													// Format: SMSHOSTLIB_CMMB_SHORT_STATISTICS_ST
													// (No return code).

	MSG_SMS_LED_CONFIG_REQ			= 870,	// Application: DVB-T/ISDB-T
											// Description: uses for LED reception indication
											// Format: Data[0] = UINT32 GPIO number
											// Direction: Host-->FW

	MSG_SMS_LED_CONFIG_RES			= 871,	// Application: DVB-T/ISDB-T
											// Description: Response to MSG_SMS_LED_CONFIG_REQ
											// Direction: FW-->Host

	// Chen Temp for PCTV PWM FOR ANTENNA
	MSG_PWM_ANTENNA_REQ				= 872,  // antenna array reception request
	MSG_PWM_ANTENNA_RES				= 873,  // antenna array reception response

	//MSG_SMS_RESERVED1				= 872,  // 

	
	MSG_SMS_CMMB_SMD_SN_REQ			= 874,  // Application: CMMB
											// Description: Get SMD serial number
											// Direction: Host-->FW
											// supported only by SMD firmware 

								
	MSG_SMS_CMMB_SMD_SN_RES			= 875,  // Application: CMMB
											// Description: Get SMD serial number response
											// Format: 
											// UINT32 RetCode
											// UINT8 SmdSerialNumber[SMS_CMMB_SMD_SN_LEN==8]

	MSG_SMS_CMMB_SET_CA_CW_REQ		= 876,  // Application: CMMB
											// Description: Set current and next CA control words 
											//	for firmware descrambler
											// Format: SMSHOSTLIB_CA_CW_PAIR_ST

	MSG_SMS_CMMB_SET_CA_CW_RES		= 877,  // Application: CMMB
											// Description: Set control words response
											// Format: UINT32 RetCode

	MSG_SMS_CMMB_SET_CA_SALT_REQ	= 878,  // Application: CMMB
											// Description: Set Set CA salt key for 
											// firmware descrambler
											// Format: SMSHOSTLIB_CA_SALT_ST
	MSG_SMS_CMMB_SET_CA_SALT_RES	= 879,	// Application: CMMB
											// Description: Set salt keys response
											// Format: UINT32 RetCode
	
	MSG_SMS_NSCD_INIT_REQ			= 880, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_INIT_RES			= 881, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_PROCESS_SECTION_REQ= 882, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_PROCESS_SECTION_RES= 883, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CREATE_OBJECT_REQ	= 884, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CREATE_OBJECT_RES	= 885, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CONFIGURE_REQ		= 886, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_CONFIGURE_RES		= 887, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_SET_KEYS_REQ		= 888, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_SET_KEYS_RES		= 889, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_HEADER_REQ	= 890, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_HEADER_RES	= 891, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_DATA_REQ	= 892, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_DATA_RES	= 893, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_GET_DATA_REQ= 894, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_DBD_PROCESS_GET_DATA_RES= 895, //NSCD injector(Internal debug fw versions only) 


	MSG_SMS_NSCD_OPEN_SESSION_REQ	= 896, //NSCD injector(Internal debug fw versions only)
	MSG_SMS_NSCD_OPEN_SESSION_RES	= 897, //NSCD injector(Internal debug fw versions only)

    MSG_SMS_SEND_HOST_DATA_TO_DEMUX_REQ		= 898, // CMMB Data to Demux injector (Internal debug fw versions only)
    MSG_SMS_SEND_HOST_DATA_TO_DEMUX_RES		= 899, // CMMB Data to Demux injector (Internal debug fw versions only)

	MSG_LAST_MSG_TYPE				= 900  // Note: Stellar ROM limits this number to 700, other chip sets to 900


}MsgTypes_ET;


/************************************************************************/
/* Definitions for useful macros                                        */
/************************************************************************/

#define STELLAR_CHIP_MODEL		(0x1002)
#define NOVA_CHIP_MODEL			(0x1102)

#define SMS_MAX_SERVICE_HANDLE	(16)
#define	MAX_PROBE_CHANNELS		24		// maximum nr of channels to probe

typedef struct SmsMsgRFTRdAllRes_S
{
	UINT32 regData[14];		// Registers 0x0-0xB, 0xD, 0xE
	UINT32 regDataC[4];		// Register 0xC, Test RAM Data
	UINT32 regDataF[2];		// Register 0xF, Debug-2
} SmsMsgRFTRdAllRes_ST;

typedef struct SmsMsgOrionRdAllRes_S
{
	UINT32 regData[14];		// Registers 0x0-0xD				- 8 to 32 bits
	UINT32 regDataE[2];		// Register 0xE - "testmode reg2"	- 64 bits
	UINT32 regDataF[2];		// Register 0xF - "Ram test Data"	- 40 bits
} SmsMsgOrionRdAllRes_ST;


///////////////////////////////////////////////////////////////////////////////
//		Source / Target IP
///////////////////////////////////////////////////////////////////////////////


/// Statistics information returned by MSG_SMS_GET_STATISTICS_RES from sms1000 to smsCntrlLib
// and
// Statistics information returned by SMSHOSTLIB_MSG_GET_STATISTICS_RES from smsCntrlLib to caller

typedef struct
{
	UINT32 RequestResult;

	SMSHOSTLIB_STATISTICS_ST Stat;

	// Split the calc of the SNR in DAB
	UINT32 Signal;				//!< dB
	UINT32 Noise;				//!< dB

} SmsMsgStatisticsInfo_ST;
typedef struct
{
	SmsMsgHdr_ST xMsgHeader;
	//Statistics parameters
	UINT32 IsDemodLocked;
	UINT32 InBandPwr;
	UINT32 BER;
	UINT32 SNR;
	UINT32 TotalTsPackets;
	UINT32 ErrorTSPackets;
} ShortStatMsg_ST;

typedef struct
{
	SmsMsgHdr_ST						xMsgHeader;
	SMSHOSTLIB_CMMB_SHORT_STATISTICS_ST stats;
} CMMBShortStatMsg_ST;

typedef struct SmsVersionRes_S
{
	SmsMsgHdr_ST					xMsgHeader;
	SMSHOSTLIB_VERSION_ST			xVersion;
} SmsVersionRes_ST;

typedef struct SMSHOSTLIB_SET_CA_S
{
	UINT32 SvcHdl;
	UINT32 SfIdx; 
	SMSHOSTLIB_CA_CW_PAIR_ST CwPair; 
}SMSHOSTLIB_SET_CA_CW_ST;


typedef struct SMSHOSTLIB_CA_SET_SALT_S
{
	UINT32 SvcHdl;
	UINT32 SfIdx; 
	UINT8 pVidSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE]; 
	UINT8 pAudSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE]; 
	UINT8 pDataSalt[SMSHOSTLIB_CMMB_CA_SALT_SIZE]; 
}SMSHOSTLIB_CA_SET_SALT_ST;

#endif





