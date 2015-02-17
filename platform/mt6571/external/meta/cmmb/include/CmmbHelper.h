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

#ifndef CMMB_HELPER_H
#define CMMB_HELPER_H

// CmmbHelper.h
//
#include "ftm_cmmb_api.h"
#include <memory.h>
#include <stdio.h>
#include <assert.h>
#include "CmmbParser.h"
#include <pthread.h>
#include "utils/Log.h"
#ifndef CMMB_CHIP_INNO
#include "SmsHostLibTypes.h"
#endif


typedef unsigned char		UINT8;
typedef signed char		INT8;
typedef unsigned short		UINT16;
typedef short				INT16;
typedef unsigned int		UINT32;
typedef int				INT32;
typedef long long			INT64;
typedef unsigned long long	UINT64;
typedef unsigned long		BOOL;
typedef unsigned char 		BYTE;

#define FALSE			0
#define TRUE				1
#define null				0

//cover up for windows specific garbage in the code
#define INFINITE 0
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT  1
#define WAIT_FAILED   2
//! error codes for the os abstraction layer
#define PTHREAD_MUTEX_ERROR 1000
#define PTHREAD_COND_VAR_ERROR 2000
#define PTHREAD_ERROR 3000
#define PTHREAD_ATTR_ERROR 4000


#define		OSAL_OK						0
#define		OSAL_ERROR					       1
#define		OSAL_TIMEOUT					2
#define		OSAL_MAX_WAIT			0xFFFFFFFF


//*****************************************
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CMMB_SP"
#define INNOADR_ENABLE_LOGS

#ifdef INNOADR_ENABLE_LOGS
#define SP_LOGE(fmt, args...) ALOGE( "cmmbSP - %s(): " fmt "\n", __FUNCTION__, ##args)
#define SP_LOGW(fmt, args...) ALOGW( "cmmbSP - %s(): " fmt "\n", __FUNCTION__, ##args)
#define SP_LOGI(fmt, args...) ALOGI( "cmmbSP - %s(): " fmt "\n", __FUNCTION__, ##args)
#define SP_LOGD(fmt, args...) ALOGD( "cmmbSP - %s(): " fmt "\n", __FUNCTION__, ##args)
#else
#define SP_LOGE(fmt, args...) 
#define SP_LOGW(fmt, args...)
#define SP_LOGI(fmt, args...)
#define SP_LOGD(fmt, args...)
#endif
//*****************************************


#ifdef WIN32
// Event type
typedef void*	Event;

#define USER_PRINTF(mask,log_str) \
	printf("%s\n", log_str) 

#else
#ifdef CMMB_CHIP_INNO
//! the linux event representation
typedef struct
{
	int set;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
} Event;
#endif

//*******************************************************************************
// Tasks
typedef unsigned int  (*TaskFunc)(unsigned int );


void * OSAL_TaskCreate (const char*	TaskName,
					                   unsigned int 		TaskPriority,
					                   unsigned int 		TaskStackSize,
					                   TaskFunc		TaskFunction,
					                   void*			TaskFunctionParams);


void  OSAL_TaskCleanup   (void * TaskId);


void  OSAL_TaskSleep  (unsigned int  TaskSleepPeriod);




#define USER_PRINTF(mask,log_str) \
	printf("%s\n", log_str); \
	SP_LOGE("%s\n", log_str)
#endif

/*Create an event.*/	
unsigned int OSAL_EventCreate(Event *pEvent );


/*Delete an event */	
unsigned int OSAL_EventDelete(Event* pEvent);



/*Set an event.*/
unsigned int OSAL_EventSet(Event*	pEvent);


/*Clear the specified event.*/	
unsigned int OSAL_EventClear(Event*pEvent);


/*Wait for a specific event with timeout.*/
unsigned int OSAL_EventWait
	(
	    Event*		       pEvent,
	    unsigned int       timeout
	);



class CEvent
{
public:
	CEvent()
		: event_()
	{
		UINT32 ret;
		ret = OSAL_EventCreate(&event_);
		assert(ret == OSAL_OK);
	}

	~CEvent()
	{
		OSAL_EventDelete(&event_);
	}

	bool Set()
	{
		return (OSAL_EventSet(&event_) == OSAL_OK);
	}

	bool Clear()
	{
		return (OSAL_EventClear(&event_) == OSAL_OK);
	}

	CmmbResult Wait(UINT32 timeout);

private:
	Event	event_;
};
void SetResponseEvent();
class CmmbFileWriter
{
public:
	CmmbFileWriter()
		: file_(null) 
	{
	}

	~CmmbFileWriter()
	{
		close();
	}

	bool isValid()
	{
		return (file_ != null);
	}

	void open(const char* fileName)
	{
		close();

		file_ = fopen("/data/cmmb_meta.mfs", "wb");
		SP_LOGE("CmmbFileWriter, file %s, handle: %d", fileName, (int)file_);
	}

	void write(UINT8* buffer, UINT32 bufSize)
	{
		if (file_ != null)
		{
			SP_LOGE("CmmbFileWriter, write buffer %d bytes", bufSize);
			fwrite(buffer, 1, bufSize, file_);
		}
	}

	void close()
	{
		if (file_ != null)
		{
			fclose(file_);
			file_ = null;
		}
	}

	int fileSize()
	{
		if (file_ != null)
		{
			return ftell(file_);
		}

		return 0;
	}

private:
	FILE*	file_;
};

const int MAX_MFS_FILE_SIZE = 50000000;

extern CmmbFileWriter mfsWriter;

void UserDataCallback(UINT32 serviceHdl, UINT8* buffer, UINT32 bufSize);
CmmbResult ParseTs0();

enum ECurrentRequest
{
	REQ_NONE = 0,
	REQ_UAM_INIT,
	REQ_UAM_SEND,
	REQ_TUNE,
	REQ_START_TS0,
	REQ_STOP_TS0,
	REQ_START_SERVICE,
	REQ_STOP_SERVICE,
	REQ_GET_STAT,
};

extern ECurrentRequest g_curReq;
extern CEvent g_rspEvent;

#define CMMB_CMD_RES_MAX_TIME   10000                         //chang 2000 -- 10000  fix GSM Authenticate timeout
#define SERVICE_HANDLE_TS0		1
#define MAX_TS0_SIZE			(64*1024)


/*************************************************************************
*			 Macros for control table
*************************************************************************/
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define CMMB_MAX_CA_SYSTEMS (20)
#define CMMB_DEMUX_ANY_SUBFRAME_INDEX		0xFF

#define CMMB_MAX_NIT_NEIGH_NETWORKS (20)
#define CMMB_MAX_NIT_OTHER_FREQS (20)
#define CMMB_MAX_NETWORK_NAME_LEN (128)
#define CMMB_MAX_CSCT_SERVICES (80)
#define CMMB_MAX_SUBFRAMES_PER_FRAME (16)

#define CMMB_MAX_VIDEO_STREAMS_IN_SUBFRAME (8)
#define CMMB_MAX_AUDIO_STREAMS_IN_SUBFRAME (8)
#define CMMB_MAX_UNITS_NUM	(100)

enum CMMB_CONTROL_TABLE_ID
{
	CMMB_NOT_SUPPORTED   = 0,
	CMMB_TABLE_SN_NIT    = 1,
	CMMB_TABLE_SN_CMCT   = 2,
	CMMB_TABLE_SN_CSCT   = 3,
	CMMB_TABLE_SN_SMCT   = 4,
	CMMB_TABLE_SN_SSCT   = 5,
	CMMB_TABLE_SN_ESG    = 6,
	CMMB_TABLE_SN_CA     = 7,
	CMMB_TABLE_SN_EMERGENCY  =0x10
};

/*************************************************************************
*			 Structs for Mpx frame 
*************************************************************************/
// Multiplex frame header
typedef struct MTKCmmbMultiplexFrameHeader_T
{
	UINT8	HeaderLengthField;		
	UINT8	ProtocolVersion;		
	UINT8	MinimumProtocolVersion;	
	UINT8	MultiplexFrameID;		
	UINT8	EmergencyBroadcastIndication;	
	UINT8	NextFrameParameterIndication;	
	UINT8	InitialLead;
	UINT8	NIT_SeqNum;		//4
	UINT8	CMCT_SeqNum;	//4
	UINT8	CSCT_SeqNum;	//4
	UINT8	SMCT_SeqNum;	//4
	UINT8	SSCT_SeqNum;	//4
	UINT8	ESG_SeqNum;		//4
	UINT8	CA_SeqNum;		//4 
	UINT8	NumOfSubFrames;	//4
	UINT8	NextFrameParamHeaderLength;		
	UINT8	NextFrameParamMsf1HeaderLength;	
	UINT32	SubFrameLength[16];				
	UINT32	NextFrameParamMsf1Length;		
	UINT32	TotalFrameLength;	
	UINT32	RESERVED1;
	UINT32	RESERVED2;
} MTKCmmbMultiplexFrameHeader;

// Video expansion area in sub frame header
typedef struct MTKCmmbVidExpArea
{
	UINT8	VidType;
	UINT8	VidIndications;

	UINT8	PicDisplayAbscissa;
	UINT8	PicDisplayOrdinate;
	UINT8	PicDisplayPriority;
	UINT8	FrameFreq;

	UINT16	VideoCodeRate;
	UINT16	VidHorizontalResolution;
	UINT16	VidVerticalResolution;
	
	UINT32 RESERVED1;
	UINT32 RESERVED2;
} MTKCmmbVidExpArea;

// Audio expansion area in sub frame header
typedef struct MTKCmmbAudExpArea
{
	UINT8	AudType;		
	UINT8	Indications;
	UINT8	SamplingRate;
	UINT16	CodeRate;
	UINT32	AudioDesc;
	UINT32	RESERVED1;
	UINT32	RESERVED2;
} MTKCmmbAudExpArea;

// Sub frame header
typedef struct MTKCmmbSubFrameHeader
{
	UINT8	SubframeIndex;		
	UINT8	HeaderLengthField;
	UINT8	Indications;
	UINT8	EncryptionMode; 
	UINT8	IsEncapMode1;
	UINT8	TotalVidStreams;
	UINT8	TotalAudStreams;
	UINT32	InitBcastTime;
	UINT32	VidSectLen;
	UINT32	AudSectLen;
	UINT32	DataSectLen;
	UINT32  RESERVED1;
	UINT32  RESERVED2;

	MTKCmmbVidExpArea	VidStreamsDesc[CMMB_MAX_VIDEO_STREAMS_IN_SUBFRAME];
	MTKCmmbAudExpArea	AudStreamsDesc[CMMB_MAX_AUDIO_STREAMS_IN_SUBFRAME];
} MTKCmmbSubFrameHeader;

// Audio/Video/Data unit parameters
// Important note - this struct is held 100 times on stack in an array, its size must not be extended.
// This structure holds the same fields for audio, video and data units, but not all fields are relevant to each type.
// These parameters are parsed from the section header.
typedef struct MTKCmmbUnitParams
{
	UINT16	UnitLengthField;	       // The length of the unit
	UINT16	RelativeBcastTime;	// Relevant only to audio/video units. The relative time for broadcasting this unit. 
	UINT8	StreamNum;			// Relevant only to audio/video units. 
	UINT8	VideoFrameType;	// Relevant only to video units
	UINT8	VideoIndications;	// Relevant only to video units
	UINT8   DataUnitType;		// Relevant only to data units
} MTKCmmbUnitParams;

// Section Header
typedef struct MTKCmmbSectionHeader
{
	UINT32 NumUnits;
	UINT32 HeaderLengthIncludingCrc;
	UINT32 RESERVED1;
	UINT32 RESERVED2;
	MTKCmmbUnitParams  UnitsParamsArr[CMMB_MAX_UNITS_NUM];
} MTKCmmbSectionHeader;

//CMCT--------------------------------
typedef struct MTKCmmbParserMpxFrame
{
	UINT8	Id;
	UINT8	FirstTimeslot;
	UINT8	NumTimeSlots;
	UINT8	NumSubFrames;
	UINT8	RsCr;
	UINT8	InterleavingMode;
	UINT8	LdpcCr;
	UINT8	Constellation;
	UINT8	ScramblingMode;

	UINT8	SubFrameNumsArr[CMMB_MAX_SUBFRAMES_PER_FRAME];
	UINT16	SubFrameServiceIdsArr[CMMB_MAX_SUBFRAMES_PER_FRAME];
} MTKCmmbParserMpxFrame;

typedef struct MTKCmmbParserInfo_CMCT
{
	UINT8	UpdateSeqNum;
	UINT8	FreqPointNum;
	UINT8	NumMpxFrames;
	MTKCmmbParserMpxFrame MultiplexFramesArr[40];
} MTKCmmbParserInfo_CMCT;

//CSCT--------------------------------
typedef struct MTKCmmbParserInfo_CSCT
{
	UINT8	UpdateSeqNum;	//4
	UINT8	SectionNumber;	
	UINT8	NumSections;
	UINT16	SectionLength;	
	UINT16	NumServices;
	UINT16  ServiceIdArr[CMMB_MAX_CSCT_SERVICES];
	UINT8   ServiceFreqPtArr[CMMB_MAX_CSCT_SERVICES];
} MTKCmmbParserInfo_CSCT;

//NIT--------------------------------
typedef struct MTKCmmbFreqPoint
{
	UINT8	Number;		
	UINT8	Bandwidth;		//4
	UINT32	CenterFreq;		
} MTKCmmbFreqPoint;

typedef struct MTKNetworkDesc
{
	UINT16				NetworkId;
	MTKCmmbFreqPoint	Freq;
} MTKNetworkDesc;

typedef struct MTKCmmbParserInfo_NIT
{
	UINT8				UpdateSeqNum;		
	UINT8				NumOfNeighNetworks;
	UINT8				NumOfOtherFreqPoints;
	UINT8				NetworkIdMode;
	UINT32 				SystemTimeHigh;		
	UINT32 				SystemTimeLowByte;	
	UINT32 				CountryCode;			
	UINT16 				NetworkId;			
	MTKCmmbFreqPoint	ThisFreqPoint; 
	UINT8 				NetworkNameLen;		
	UINT8  				NetworkName[CMMB_MAX_NETWORK_NAME_LEN];
	MTKCmmbFreqPoint      OtherFreqPtsArr[CMMB_MAX_NIT_OTHER_FREQS];
	MTKNetworkDesc	       NeigNetworksArr[CMMB_MAX_NIT_NEIGH_NETWORKS];
} MTKCmmbParserInfo_NIT;


typedef struct Demuxer{
  MTKCmmbParserInfo_NIT           m_Nit;    //Network Information
  MTKCmmbParserInfo_CMCT          m_CMct;   //Continual Multiplex Configuration Table
  MTKCmmbParserInfo_CMCT           m_SMct;   // Short time Multiplex Configuration Table
  MTKCmmbParserInfo_CSCT          m_CSct;    // Continual Service Configuration Table
  MTKCmmbParserInfo_CSCT           m_SSct;   // Short time Service Configuration Table
}Demuxer;

Demuxer& CmmbGetDemuxer();
MTKCmmbParserMpxFrame* FindMpxFrameInfo(UINT32 serviceId);
void cmmb_build_crc_table(void);

#endif // CMMB_HELPER_H
