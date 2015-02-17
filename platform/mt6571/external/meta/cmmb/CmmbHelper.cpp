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

// CmmbHelper.cpp
//
#include "CmmbHelper.h"
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>


typedef void * (*PthreadTaskFunc)(void*);
/*!  sleep
	\param[in]	TaskSleepPeriod: time to sleep.
*/
void OSAL_TaskSleep ( unsigned int	TaskSleepPeriod )
{
	struct timespec ts, tr;
	
	// calculate wakeup time
	ts.tv_sec = TaskSleepPeriod / 1000;
	ts.tv_nsec = ( TaskSleepPeriod % 1000 ) * 1000000;
	// sleep
	nanosleep ( &ts, &tr );
	
}


//*******************************************************************************
// Create task
//
void* OSAL_TaskCreate ( const char *TaskName,
					   unsigned int TaskPriority,
					   unsigned int  TaskStackSize,
					   TaskFunc TaskFunction,
					   void *TaskFunctionParams )
{
	pthread_t *TaskHandle;
	pthread_attr_t attr;

	// allocate memory for task structure
	TaskHandle = (pthread_t*)malloc(sizeof(pthread_t));

	pthread_attr_init(&attr);
	pthread_attr_setstacksize (&attr, TaskStackSize);
	
	// create linux thread
	pthread_create ( TaskHandle, &attr,
					 (PthreadTaskFunc)TaskFunction,
					 TaskFunctionParams );
	if ( *TaskHandle == 0 )
	{
		free(TaskHandle);
		return NULL;
	}

	return (void*)TaskHandle;
}


//*******************************************************************************
// Destroy task
//
void OSAL_TaskCleanup ( void* pTask )
{
	// free task structure
	free((pthread_t*)pTask);
}


/*! create new event
	\param[in]	event: pointer to the event structure instance
	\return		always 0
*/
unsigned int OSAL_EventCreate ( Event * pEvent)
{
	if (pEvent)
	{
		// init mutex for event
		pthread_mutex_init(&pEvent->mutex, NULL);
		// init condition variable
		pthread_cond_init(&pEvent->cond, NULL);
		pEvent->set = 0;
		return 0;
	}
	
	return 1;
}


/*! destroy the event
	\param[in]	event: pointer to the event structure instance
	\return		always 0
*/
unsigned int OSAL_EventDelete ( Event * pEvent)
{
	if (pEvent)
	{
		pthread_mutex_destroy(&pEvent->mutex);
		pthread_cond_destroy(&pEvent->cond);
		return 0;
	}

	return 1;
}


/*! set the event
	\param[in]	event: pointer to the event structure instance
	\return		error status
*/
unsigned int OSAL_EventSet ( Event * pEvent)
{
	int result = 0;

	result = pthread_mutex_lock ( &pEvent->mutex );
	if ( result != 0 )
	{
		result = pthread_mutex_unlock (  &pEvent->mutex );
		return result;
	}
	// setflag for reolve problem of wait-signal time seqence.
	pEvent->set = 1;
	result = pthread_cond_signal ( &pEvent->cond );
	if ( result != 0 )
	{
		pthread_mutex_unlock(&pEvent->mutex);
		return result;
	}


	result = pthread_mutex_unlock (  &pEvent->mutex );
	if ( result != 0 )
	{
		return result;
	}

	return result;
}


unsigned int OSAL_EventClear ( Event * pEvent)
{
	int result = 0;

	result = pthread_mutex_lock ( &pEvent->mutex );
	if ( result != 0 )
	{
		result = pthread_mutex_unlock (  &pEvent->mutex );
		return result;
	}

	// clear "even set" flag
	pEvent->set = 0;

	result = pthread_mutex_unlock (  &pEvent->mutex );
	if ( result != 0 )
	{
		return result;
	}

	return 0;
}


unsigned int OSAL_EventWait ( Event * pEvent, unsigned int Timeout )
{
	int result = 0;
	unsigned int Rc = 0;

	// critical section
	pthread_mutex_lock ( &pEvent->mutex );

	// check "event set" flag
	if ( pEvent->set )
	{
		// if already set reset and return immidiately
		pEvent->set = 0;
		pthread_mutex_unlock ( &pEvent->mutex );
		return 0;
	}

	if(0xFFFFFFFF == Timeout)
	{
		// wait forever
		pthread_cond_wait ( &pEvent->cond, &pEvent->mutex );
		pEvent->set = 0;
	}
	else
	{
		struct timeval now;
		struct timespec timeout;
		gettimeofday(&now, NULL);

		
		timeout.tv_sec = now.tv_sec + (Timeout/1000);
		timeout.tv_nsec = (now.tv_usec*1000) + ((Timeout % 1000) * 1000000);
		if (timeout.tv_nsec >= 1000000000)
		{
			timeout.tv_nsec -= 1000000000;
			timeout.tv_sec++;			
		}

		result = pthread_cond_timedwait ( &pEvent->cond, &pEvent->mutex, &timeout);
		if(!result)
		{
			pEvent->set = 0;		
		}
		else
		{
			Rc = result;
			if ( result == ETIMEDOUT )
			{
				Rc = OSAL_MAX_WAIT;
			}
		}
	}

	// exit critical section
	pthread_mutex_unlock (  &pEvent->mutex );
	return Rc;
}

//***********************************************************************************************
typedef UINT32 (*cmmb_bits_fill_t)(void* param, UINT8* buf, UINT32 bytes);

typedef struct cmmb_bits_t
{
    UINT8* buf;     /**< buffer pointer */
    UINT32 buf_len; /**< buffer length */

    UINT8* read;    /**< pointer to the next read position */
    UINT8* end;     /**< pointer to the end of buffer */

    cmmb_bits_fill_t fill;
    void* param;        /**< opaque parameter for the callback */

    UINT32 cache;   /**< cached bits */
    UINT32 bits;    /**< number of bits in the cache */

} cmmb_bits_t;

UINT32 cmmb_bits_show(cmmb_bits_t* self, UINT32 n)
{
    assert(self->bits >= n);

    /* cache:
     *
     *  -------- -------- ---- bits
     * |********|********|****####|########|
     *  ---- n
     *      ---- -------- -------- -------- (32 - n)
     */
    return self->cache >> (32 - n);
}

void cmmb_flush_bits(cmmb_bits_t* self, UINT32 n)
{
    assert(self->bits >= n);

    if (n < 32)
        self->cache <<= n;
    else
        self->cache = 0;

    self->bits -= n;
    while (self->bits <= 24)
    {
        if (self->read + 4 <= self->end)
        {
            UINT32 bits = self->bits;

            do
            {
                /* cache:
                 *
                 *  -------- -------- ---- bits
                 * |********|********|****####|########|
                 *                        ---- (24 - bits)
                 */
                self->cache |= *self->read++ << (24 - bits);
                bits += 8;
            }
            while (bits <= 24);

            self->bits = bits;
        }
        else
        {
            do
            {
                /* extra boundary checking here */
                if (self->read >= self->end)
                {
                    UINT32 bytes;

                    if (!self->fill) {
                        return;
                    }

                    bytes = self->fill(self->param, self->buf, self->buf_len);
                    if (bytes == 0) {
                        return;
                    }

                    self->read = self->buf;
                    self->end = self->read + bytes;
                }

                self->cache |= *self->read++ << (24 - self->bits);
                self->bits += 8;
            }
            while (self->bits <= 24);
        }
    }
}

UINT32 cmmb_bits_get(cmmb_bits_t* self, UINT32 n)
{
    UINT32 val;

    val = cmmb_bits_show(self, n);
    cmmb_flush_bits(self, n);
    return val;
}

void cmmb_bits_init(cmmb_bits_t* self, UINT8* buf, UINT32 bytes, cmmb_bits_fill_t fill, void* param)
{
    assert(bytes > 0 && buf != null);

    self->fill = fill;
    self->param = param;
    self->buf = buf;
    self->buf_len = bytes;

    self->read = buf;
    self->end = buf + bytes;
    self->bits = 0;
    self->cache = 0;

    cmmb_flush_bits(self, 0);
}

UINT32 cmmb_bits_get_byte(cmmb_bits_t* self)
{
    assert(self->bits >= 8 && self->bits % 8 == 0);
    return cmmb_bits_get(self, 8);
}

void cmmb_bits_get_block(cmmb_bits_t* self, UINT8* buf, UINT32 bytes)
{
    UINT32 cached_bytes = self->bits / 8;
    UINT32 remaining = self->end - self->read;

	assert(self->bits % 8 == 0);

    if (bytes <= cached_bytes)
    {
        UINT32 i;
        for (i = 0; i < bytes; i++) {
            buf[i] = cmmb_bits_get_byte(self);
        }
        return;
    }
    else
    {
        if (cached_bytes >= 1) {
            buf[0] = (self->cache & 0xFF000000) >> 24;
        }
        if (cached_bytes >= 2) {
            buf[1] = (self->cache & 0x00FF0000) >> 16;
        }
        if (cached_bytes >= 3) {
            buf[2] = (self->cache & 0x0000FF00) >> 8;
        }
        if (cached_bytes >= 4) {
            buf[3] = (self->cache & 0x000000FF);
        }

        buf += cached_bytes;
        bytes -= cached_bytes;

        self->bits = 0;
        self->cache = 0;
    }

    /* move content to the beginnin of buffer */
    if (bytes <= remaining)
    {
        memcpy(buf, self->read, bytes);
        self->read += bytes;
    }
    else if (self->fill)
    {
    	memcpy(buf, self->read, remaining);
    	bytes = self->fill(self->param, buf + remaining, bytes - remaining);
    	self->read = self->end;
    }
    else {
        assert(0);
    }

    cmmb_flush_bits(self, 0);
}

//**************************************************************************************
#define zero_struct(item) memset(&(item), 0, sizeof(item))

#define MAKE_CRC32(b0, b1, b2, b3) \
    (((UINT32)b0 << 24) + ((UINT32)b1 << 16) + ((UINT32)b2 << 8) + (UINT32)b3)

static unsigned long cmmb_crc32_table[256];
const UINT32 CRC32_SEED = 0x04C11DB7;

void cmmb_build_crc_table(void)
{
	UINT32 i, j;	
	for (i = 0; i < 256; ++i)
	{
		UINT32 data = (i << 24);
		UINT32 accum = 0;

		for (j = 0; j < 8; j++ )
		{
			if ((data ^ accum) & 0x80000000)
				accum = (accum << 1) ^ CRC32_SEED;
			else
				accum <<= 1;
			data <<= 1;
		}
		cmmb_crc32_table[i] = accum;
	}
}


UINT32 cmmb_calc_crc32(const UINT8* data, UINT32 len)
{
    UINT32 i;
    UINT32 crc = 0xFFFFFFFF;

    for (i = 0; i < len; ++i) {
        crc = (crc << 8) ^ cmmb_crc32_table[(data[i]) ^ (crc >> 24)];
    }
    return crc;
}

BOOL cmmb_check_crc32(UINT32 crc32, UINT8* data, UINT32 len)
{
    return crc32 == cmmb_calc_crc32(data, len);
}
//****************************************************************************************************

Demuxer demux_tmp_buf;
Demuxer& CmmbGetDemuxer()
{
	return demux_tmp_buf;
}
    
MTKCmmbParserMpxFrame* FindMpxFrameInfo(UINT32 serviceId)
{
	SP_LOGE("serviceId: %d", serviceId);

	Demuxer& demx = CmmbGetDemuxer();
	SP_LOGE("NumMpxFrames: %d", demx.m_CMct.NumMpxFrames);

	for (int i = 0; i < demx.m_CMct.NumMpxFrames; i++)
	{
		SP_LOGE( "SubFrameServiceId: %d", demx.m_CMct.MultiplexFramesArr[i].SubFrameServiceIdsArr[0]);
		if (demx.m_CMct.MultiplexFramesArr[i].SubFrameServiceIdsArr[0] == (UINT16)serviceId)
			return &demx.m_CMct.MultiplexFramesArr[i];
	}

	return null;
}

CmmbFileWriter mfsWriter;

extern const char* ESG_FILE_NAME;

// used when parse TS0
UINT8  g_bufTs0[MAX_TS0_SIZE];
UINT32 g_sizeTs0;

// asynchronous response event
CEvent g_rspEvent;

// current request command
ECurrentRequest g_curReq = REQ_NONE;

void SetResponseEvent()
{
	g_rspEvent.Set();
}

CmmbResult CEvent::Wait(UINT32 timeout)
{
	UINT32 retCode = OSAL_EventWait(&event_, timeout);
	CmmbResult errCode;

	switch (retCode)
	{
	case OSAL_OK:
		errCode = CMMB_S_OK;
		break;

	case OSAL_TIMEOUT:
		errCode = CMMB_E_TIMEOUT;
		break;

	default:
		errCode = CMMB_E_UNKNOWN;
		break;
	}

	return errCode;
}
	
CmmbResult cmmb_mfs_parse_frame( UINT8* pBuf, UINT32 BufSize, UINT32 RequiredSubFrameIndex );
#ifdef CMMB_CHIP_INNO
BYTE GetMpxFrameDemod(MTKCmmbParserMpxFrame& mpxFrm);
void TraceMpxFrameInfo()
{
	Demuxer& demx = CmmbGetDemuxer();
	SP_LOGE("NumMpxFrames: %d", demx.m_CMct.NumMpxFrames);
	MTKCmmbParserMpxFrame* mpxFrm;
	BYTE demod;

	for (int i = 0; i < demx.m_CMct.NumMpxFrames; i++)
	{
		mpxFrm = demx.m_CMct.MultiplexFramesArr+i;
		demod = GetMpxFrameDemod(*mpxFrm);

		SP_LOGE( "ServiceId: %d, firtsTS: %d, ts count: %d, demod: %d",mpxFrm->SubFrameServiceIdsArr[0], mpxFrm->FirstTimeslot, mpxFrm->NumTimeSlots, demod);
	}
}
#else
void TraceMpxFrameInfo()
{
	Demuxer& demx = CmmbGetDemuxer();
	SP_LOGE("NumMpxFrames: %d", demx.m_CMct.NumMpxFrames);
	MTKCmmbParserMpxFrame* mpxFrm;

	for (int i = 0; i < demx.m_CMct.NumMpxFrames; i++)
	{
		mpxFrm = demx.m_CMct.MultiplexFramesArr+i;

		SP_LOGE( "ServiceId: %d, firtsTS: %d, ts count: %d",mpxFrm->SubFrameServiceIdsArr[0], mpxFrm->FirstTimeslot, mpxFrm->NumTimeSlots);
	}
}
#endif //CMMB_CHIP_INNO

CmmbResult ParseTs0()
{
	mfsWriter.write(g_bufTs0, g_sizeTs0);

	// parse TS0 data
	SP_LOGE("parse TS0 begin");

	cmmb_mfs_parse_frame(g_bufTs0,g_sizeTs0,null);

	TraceMpxFrameInfo();

	return CMMB_S_OK;
}

void UserDataCallback(UINT32 serviceHdl, UINT8* buffer, UINT32 bufSize)
{
	// for backward compatibility
	if (bufSize == 0)
		return;

	SP_LOGE( "get a Mpx frame, serviceHdl: %d", serviceHdl);

	if (serviceHdl == SERVICE_HANDLE_TS0)	
	{ 
		// TS0: containing control tables
		if (g_curReq == REQ_START_TS0)
		{
			// save TS0 data
			assert(bufSize <= MAX_TS0_SIZE);
			memcpy(g_bufTs0, buffer, bufSize);
			g_sizeTs0 = bufSize;

			SetResponseEvent();
		}
	}
	else // not TS0: containing service data
	{
		if (mfsWriter.isValid()
			&& mfsWriter.fileSize() < MAX_MFS_FILE_SIZE)
		{
			mfsWriter.write(buffer, bufSize);
		}
	}

	SP_LOGE( "get a Mpx frame end, serviceHdl: %d", serviceHdl);
}


MTKCmmbMultiplexFrameHeader g_MpxFrameHeader;

CmmbResult	cmmb_msf_parse_xMCT( UINT8* pBuf,	UINT32 BufSize, MTKCmmbParserInfo_CMCT* pOutCmct )
{
	UINT32 i;
	CmmbResult RetCode;
	UINT32 Crc;
	
	zero_struct(*pOutCmct);

	Crc = MAKE_CRC32(pBuf[BufSize-4], pBuf[BufSize-3], pBuf[BufSize-2], pBuf[BufSize-1]);	

	if ( !cmmb_check_crc32(Crc, pBuf, BufSize-4))
	{
		SP_LOGE("cmmbparser:cmmb_msf_parse_xMCT (bad CRC)");
		return CMMB_PARSER_E_FAILED_CRC_CHECK;
	}

	cmmb_bits_t bs;
	
	cmmb_bits_init(&bs,pBuf+1, BufSize-1, null, null); // Skip the 8-bit Table ID

	pOutCmct->FreqPointNum = (UINT8)cmmb_bits_get(&bs,8);

	pOutCmct->UpdateSeqNum = (UINT8)cmmb_bits_get(&bs,4);

	cmmb_bits_get(&bs,6);// 6 bits Reserved

	pOutCmct->NumMpxFrames = (UINT8)cmmb_bits_get(&bs,6);

	if (pOutCmct->NumMpxFrames > 40 )
	{
		return CMMB_PARSER_E_ILLEGAL_DATA;
	}

	for ( i=0 ; i < pOutCmct->NumMpxFrames ; i++ )
	{
		MTKCmmbParserMpxFrame* pMpxFrame = &pOutCmct->MultiplexFramesArr[i];
		UINT32 j;

		pMpxFrame->Id = (UINT8)cmmb_bits_get(&bs,6);

		pMpxFrame->RsCr = (UINT8)cmmb_bits_get(&bs,2);

		pMpxFrame->InterleavingMode = (UINT8)cmmb_bits_get(&bs,2);

		pMpxFrame->LdpcCr = (UINT8)cmmb_bits_get(&bs,2);

		pMpxFrame->Constellation = (UINT8)cmmb_bits_get(&bs,2);

		cmmb_bits_get(&bs,1); // 1 bit Reserved

		pMpxFrame->ScramblingMode = (UINT8)cmmb_bits_get(&bs,3);

		pMpxFrame->NumTimeSlots = (UINT8)cmmb_bits_get(&bs,6);

		for ( j = 0 ; j < pMpxFrame->NumTimeSlots ; j++ )
		{
			UINT8 tempnum = (UINT8) cmmb_bits_get(&bs,6);
			if ( j == 0 )
			{
				pMpxFrame->FirstTimeslot = (UINT8)tempnum;
			}
			if ( tempnum != pMpxFrame->FirstTimeslot + j )
			{
				// We don't support non-sequential time slots 
				return CMMB_PARSER_E_NOT_SUPPORTED;
			}

			cmmb_bits_get(&bs,2); // 2 bits Reserved
		}

		cmmb_bits_get(&bs,4); // 4 bits Reserved

		pMpxFrame->NumSubFrames = (UINT8)cmmb_bits_get(&bs,4);

		for ( j = 0 ; j < pMpxFrame->NumSubFrames ; j++ )
		{
			pMpxFrame->SubFrameNumsArr[j] = (UINT8)cmmb_bits_get(&bs,4);

			cmmb_bits_get(&bs,4); // 4 Bits reserved

			pMpxFrame->SubFrameServiceIdsArr[j] = (UINT16)cmmb_bits_get(&bs,16);
		}
	}	//for ( i=0 ; i < pOutCmct->NumMpxFrames ; i++ )

	return CMMB_PARSER_S_OK;
}


 CmmbResult	cmmb_msf_parse_xSCT( UINT8* pBuf,UINT32 BufSize,MTKCmmbParserInfo_CSCT* pOutCsct )
{       
	UINT32 i;
	CmmbResult RetCode;
	UINT32 Crc;

	zero_struct(*pOutCsct);

	Crc = MAKE_CRC32(pBuf[BufSize-4], pBuf[BufSize-3], pBuf[BufSize-2], pBuf[BufSize-1]);	

	if ( !cmmb_check_crc32(Crc, pBuf, BufSize-4))
	{
		SP_LOGE("cmmbparser:cmmb_msf_parse_xSCT (bad CRC)");
		return CMMB_PARSER_E_FAILED_CRC_CHECK;
	}

	cmmb_bits_t bs;	
	cmmb_bits_init(&bs,pBuf+1, BufSize-1, null, null); // Skip the 8-bit Table ID

	pOutCsct->SectionLength = (UINT16)cmmb_bits_get(&bs,16);

	pOutCsct->SectionNumber = (UINT8)cmmb_bits_get(&bs,8);

	pOutCsct->NumSections = (UINT8)cmmb_bits_get(&bs,8);

	pOutCsct->UpdateSeqNum = (UINT8)cmmb_bits_get(&bs,4);

	cmmb_bits_get(&bs,4); 	// 4 bits Reserved

	pOutCsct->NumServices = (UINT16)cmmb_bits_get(&bs,16);
       SP_LOGD("NumServices =%d",pOutCsct->NumServices);
	   
	for ( i=0 ; i < pOutCsct->NumServices ; i++ )
	{
		pOutCsct->ServiceIdArr[i] =	(UINT16)cmmb_bits_get(&bs,16);
		
		pOutCsct->ServiceFreqPtArr[i] =	(UINT8)cmmb_bits_get(&bs,8);
		SP_LOGD(" <%d> Freq=%d,ServiceID=%d",i,pOutCsct->ServiceFreqPtArr[i],pOutCsct->ServiceIdArr[i]);
		
		if ( i >= CMMB_MAX_CSCT_SERVICES - 1 )
		{
			return CMMB_PARSER_E_EXCEEDED_ALLOCATED_MEMORY;
		}
	}

	return CMMB_PARSER_S_OK;
}


 CmmbResult cmmb_msf_parse_NIT(  UINT8* pBuf, UINT32 BufSize, MTKCmmbParserInfo_NIT* pOutNit )
{
	CmmbResult RetCode = CMMB_PARSER_S_OK;
	UINT32 NetworkNameLenToCopy;
	UINT32 i;
	UINT16 NetworkLevel;
	UINT16 NetworkNumber;
	UINT32 Crc;

	zero_struct(*pOutNit);

	Crc = MAKE_CRC32(pBuf[BufSize-4], pBuf[BufSize-3], pBuf[BufSize-2], pBuf[BufSize-1]);	

	if ( !cmmb_check_crc32(Crc, pBuf, BufSize-4))
	{
		SP_LOGE("cmmbparser:cmmb_msf_parse_NIT (bad CRC)");
		return CMMB_PARSER_E_FAILED_CRC_CHECK;
	}
	
	cmmb_bits_t bs;	
	cmmb_bits_init(&bs,pBuf+1, BufSize-1, null, null); // Skip the 8-bit Table ID

	pOutNit->UpdateSeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutNit->NetworkIdMode = (UINT8)cmmb_bits_get(&bs,1);	// 0: 4 bits Level + 12 bits Number, 1: 16 bits ID

	cmmb_bits_get(&bs,3); // 3 bits Reserved 

	pOutNit->SystemTimeHigh = (UINT32)cmmb_bits_get(&bs,32);

	pOutNit->SystemTimeLowByte = (UINT8)cmmb_bits_get(&bs,8);

	pOutNit->CountryCode = (UINT32)cmmb_bits_get(&bs,24);

	NetworkLevel = (UINT8)cmmb_bits_get(&bs,4);

	NetworkNumber = (UINT16)cmmb_bits_get(&bs,12);

	pOutNit->NetworkId = ((UINT16)(NetworkLevel)<<12 | (NetworkNumber));

	pOutNit->NetworkNameLen = (UINT8)cmmb_bits_get(&bs,8);

	NetworkNameLenToCopy = pOutNit->NetworkNameLen;
	if ( NetworkNameLenToCopy > CMMB_MAX_NETWORK_NAME_LEN-1 )
	{
		RetCode = CMMB_PARSER_E_EXCEEDED_ALLOCATED_MEMORY;
		NetworkNameLenToCopy = CMMB_MAX_NETWORK_NAME_LEN-1;
	}

	// Read the network name
	cmmb_bits_get_block(&bs, pOutNit->NetworkName, NetworkNameLenToCopy);
	
	pOutNit->NetworkName[NetworkNameLenToCopy] = '\0';

	pOutNit->ThisFreqPoint.Number = (UINT8)cmmb_bits_get(&bs,8);

	pOutNit->ThisFreqPoint.CenterFreq = (UINT32)cmmb_bits_get(&bs,32);

	pOutNit->ThisFreqPoint.Bandwidth = (UINT8)cmmb_bits_get(&bs,4);

	pOutNit->NumOfOtherFreqPoints =(UINT8)cmmb_bits_get(&bs,4);

	for (i=0; i < pOutNit->NumOfOtherFreqPoints ; i++)
	{
		if ( i < CMMB_MAX_NIT_OTHER_FREQS )
		{
			MTKCmmbFreqPoint* pFrqPt = &pOutNit->OtherFreqPtsArr[i];

			pFrqPt->Number = (UINT8)cmmb_bits_get(&bs,8);

			pFrqPt->CenterFreq = (UINT32)cmmb_bits_get(&bs,32);

			pFrqPt->Bandwidth = (UINT8)cmmb_bits_get(&bs,4);

			cmmb_bits_get(&bs,4); // 4 bits Reserved
		}
		else
		{
			// No more out buffer for freq pts - just keep reading to sync the bit reader
			cmmb_bits_get(&bs,6);
			RetCode = CMMB_PARSER_E_EXCEEDED_ALLOCATED_MEMORY;
		}
	}

	pOutNit->NumOfNeighNetworks = (UINT8)cmmb_bits_get(&bs,4);
	cmmb_bits_get(&bs,4); // 4 bits Reserved

	for (i=0; i < pOutNit->NumOfNeighNetworks ; i++)
	{

		if ( i < CMMB_MAX_NIT_NEIGH_NETWORKS )
		{
			MTKNetworkDesc* pNet = &pOutNit->NeigNetworksArr[i];

			NetworkLevel = (UINT8)cmmb_bits_get(&bs,4);

			NetworkNumber =	(UINT16)cmmb_bits_get(&bs,12);

			pNet->NetworkId = ((UINT16)(NetworkLevel)<<12 | (NetworkNumber));

			pNet->Freq.Number =	(UINT8)cmmb_bits_get(&bs,8);

			pNet->Freq.CenterFreq = (UINT32)cmmb_bits_get(&bs,32);

			pNet->Freq.Bandwidth =	(UINT8)cmmb_bits_get(&bs,4);

			cmmb_bits_get(&bs,4); // 4 bits Reserved
		}
		else
		{
			// No more out buffer for freq pts 
			return CMMB_PARSER_E_EXCEEDED_ALLOCATED_MEMORY;
		}
	}

	return RetCode;
}

CmmbResult cmmb_msf_parse_controltable( 	UINT8* pBuf,
									 			UINT32 BufSize)
{      
      
	unsigned long ret = 0;
	Demuxer *demux = (Demuxer*)&demux_tmp_buf ;
	MTKCmmbMultiplexFrameHeader& MpxFrameHeader = g_MpxFrameHeader;

	switch(*pBuf)
	{
	case CMMB_TABLE_SN_NIT:
		cmmb_msf_parse_NIT(pBuf, BufSize, &(demux->m_Nit));
		break;
	case CMMB_TABLE_SN_CMCT:
		cmmb_msf_parse_xMCT(pBuf, BufSize, &(demux->m_CMct));		
		break;
	case CMMB_TABLE_SN_CSCT:	
		cmmb_msf_parse_xSCT(pBuf, BufSize, &(demux->m_CSct)); 	
		break;
	case CMMB_TABLE_SN_SSCT:
		cmmb_msf_parse_xSCT(pBuf, BufSize, &(demux->m_SSct));				
		break;
	case CMMB_TABLE_SN_SMCT:		
		cmmb_msf_parse_xMCT(pBuf,BufSize, &(demux->m_SMct));	
		break;
	default:
		break;
	}	
	return CMMB_PARSER_S_OK;
}

//*******************************************************************************

CmmbResult	cmmb_mfs_parse_frame_header( UINT8* pBuf,UINT32 BufSize, MTKCmmbMultiplexFrameHeader* pOutFrameHeader )
{
	UINT32 i;
	UINT32 Tmp;
	CmmbResult RetCode;
	UINT32 Crc;
	zero_struct(*pOutFrameHeader);
	
	cmmb_bits_t bs;	
	cmmb_bits_init(&bs,pBuf, BufSize, null, null);
	
	// Read the start code
	if ( cmmb_bits_get(&bs,32) != 0x00000001 )
	{
		return CMMB_PARSER_E_ILLEGAL_DATA;
	}

	pOutFrameHeader->HeaderLengthField = (UINT8)cmmb_bits_get(&bs,8);//header length(do not include CRC)


	Crc = MAKE_CRC32(pBuf[pOutFrameHeader->HeaderLengthField], pBuf[pOutFrameHeader->HeaderLengthField+1], 
	      						pBuf[pOutFrameHeader->HeaderLengthField+2], pBuf[pOutFrameHeader->HeaderLengthField+3]);	
	if ( !cmmb_check_crc32(Crc, pBuf, pOutFrameHeader->HeaderLengthField))
	{	
		SP_LOGE("cmmbparser:bad CRC");   
		return CMMB_PARSER_E_FAILED_CRC_CHECK;
	}

	pOutFrameHeader->TotalFrameLength += pOutFrameHeader->HeaderLengthField + 4; // Add entire header length
	
	pOutFrameHeader->ProtocolVersion = (UINT8)cmmb_bits_get(&bs,5);

	pOutFrameHeader->MinimumProtocolVersion = (UINT8)cmmb_bits_get(&bs,5);

	pOutFrameHeader->MultiplexFrameID = (UINT8)cmmb_bits_get(&bs,6);
	
	pOutFrameHeader->EmergencyBroadcastIndication = (UINT8)cmmb_bits_get(&bs,2);

	pOutFrameHeader->NextFrameParameterIndication = (UINT8)cmmb_bits_get(&bs,1);

	cmmb_bits_get(&bs,3); // 3 bits Reserved

	pOutFrameHeader->InitialLead = (UINT8)cmmb_bits_get(&bs,2);

	pOutFrameHeader->NIT_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->CMCT_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->CSCT_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->SMCT_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->SSCT_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->ESG_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->CA_SeqNum = (UINT8)cmmb_bits_get(&bs,4);

	pOutFrameHeader->NumOfSubFrames = (UINT8)cmmb_bits_get(&bs,4);

	for (i=0 ; i < pOutFrameHeader->NumOfSubFrames ; i++)
	{
		pOutFrameHeader->SubFrameLength[i] = cmmb_bits_get(&bs,24);

		pOutFrameHeader->TotalFrameLength += pOutFrameHeader->SubFrameLength[i];
	}

	// Read each part of next frame parameter separately, to avoid >32 bits reading
	if (pOutFrameHeader->NextFrameParameterIndication)
	{
		pOutFrameHeader->NextFrameParamHeaderLength = (UINT8)cmmb_bits_get(&bs,8);

		pOutFrameHeader->NextFrameParamMsf1Length = (UINT32)cmmb_bits_get(&bs,24);

		pOutFrameHeader->NextFrameParamMsf1HeaderLength = (UINT8)cmmb_bits_get(&bs,8);
	}

	return CMMB_PARSER_S_OK;
}

CmmbResult cmmb_mfs_parse_frame( 	UINT8* pBuf, 
										UINT32 BufSize,  //the whole frame size
										UINT32 RequiredSubFrameIndex )
{
	CmmbResult RetCode = CMMB_PARSER_S_OK; 
	CmmbResult FinalRetCode = CMMB_PARSER_S_OK; 
	MTKCmmbMultiplexFrameHeader MpxFrameHeader;
	UINT32 CurSubFrameI;

	RetCode = cmmb_mfs_parse_frame_header( pBuf, BufSize, &MpxFrameHeader);
	if(RetCode != CMMB_PARSER_S_OK)
	{
		SP_LOGE("Failed to parse MPX frame header. ERROR=0x%x", RetCode);
		return RetCode; 
	}
	
	g_MpxFrameHeader=MpxFrameHeader; 	

	// Skip MPX header + CRC
	pBuf += MpxFrameHeader.HeaderLengthField + 4;
	BufSize -= MpxFrameHeader.HeaderLengthField + 4;

	for (CurSubFrameI = 0 ; CurSubFrameI < MpxFrameHeader.NumOfSubFrames ; CurSubFrameI++ )
	{
		UINT32 SubFrameLength = MpxFrameHeader.SubFrameLength[CurSubFrameI];
		if ( SubFrameLength > BufSize )
		{
			SP_LOGE("Subframe length larger than buffer size - probably some data dropped. Discarding.");
			return CMMB_PARSER_E_DATA_CORRUPTED;
		}

		if ( MpxFrameHeader.MultiplexFrameID == 0 )
		{
			RetCode = cmmb_msf_parse_controltable( pBuf,SubFrameLength);
	
			if(RetCode != CMMB_PARSER_S_OK)
			{					
				SP_LOGE("Error parsing cmmb_msf_parse_controltable  ERROR=0x%x", RetCode ); 
				if (FinalRetCode == CMMB_PARSER_S_OK) FinalRetCode = RetCode;
			}			
		}
		pBuf += SubFrameLength;
		BufSize -= SubFrameLength;
	}

	return FinalRetCode;
}
