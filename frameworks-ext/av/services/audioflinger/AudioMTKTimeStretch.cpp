/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2013. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioMTKTimeStretch"

extern "C" {
#include "AudioMTKTimeStretch.h"
}
#include <cutils/xlog.h>
#ifdef MTK_AUDIO
#include "AudioUtilmtk.h"
#endif

#ifdef MTK_AUDIO
#ifdef DEBUG_AUDIO_PCM
//    static   const char * gaf_timestretch_in_pcm = "/sdcard/mtklog/audio_dump/af_timestretch_in_pcm.pcm";
    //static   const char * gaf_timestretch_out_pcm = "/sdcard/mtklog/audio_dump/af_timestretch_out_pcm.pcm";

//    static   const char * gaf_timestretch_in_propty = "af.timestretch.in.pcm";
    //static   const char * gaf_timestretch_out_propty = "af.timestretch.out.pcm";
#endif
#endif
namespace android {

AudioMTKTimeStretch::AudioMTKTimeStretch(int framecount)
{
	mpHandle = NULL;
	mInternalBufferSize = 0;
	mTempBufferSize = 0;
	mInBuf_smpl_cnt = 0;
	mInputBufferSize = framecount;
	mInputBuffer = new short[framecount*2];//  *2 for stereo
	mInBuf_smpl_cnt = 0;
}
AudioMTKTimeStretch::AudioMTKTimeStretch(int ratio, int inChannelCount, int sampleRate,int framecount)
{
	mpHandle = NULL;
	mInternalBufferSize = 0;
	mTempBufferSize = 0;
	mInBuf_smpl_cnt = 0;
	mInitParam.SampleRate = sampleRate;
	mInitParam.StereoFlag = inChannelCount-1; 
	mInitParam.TD_FD = 1;
	mBTS_RTParam.TS_Ratio= ratio; // initially no change. 
}
AudioMTKTimeStretch::~AudioMTKTimeStretch()
{
	
	BTS_Close(mpHandle, 1);

	if(NULL != mWorkingBuffer) {
		delete[] mWorkingBuffer;
		mWorkingBuffer = NULL;
	}
	if(NULL != mTempBuffer) {
		delete[] mTempBuffer;
		mTempBuffer = NULL;
	}
	if(NULL != mInputBuffer) {
		delete[] mInputBuffer;
		mInputBuffer = NULL;
	}
	 mpHandle = NULL;
	 	
}

void AudioMTKTimeStretch::reset()
{
	SXLOGD("reset");	
	//mBTS_RTParam.TS_Ratio = 100;
	// reset buffer
	if(mWorkingBuffer != NULL);
	{memset(mWorkingBuffer, 0 ,mInternalBufferSize);
		}
	if(mTempBufferSize != NULL)
		memset((void*)mTempBuffer, 0 ,mTempBufferSize);
	// reset internal input buffer count.
	mInBuf_smpl_cnt = 0;
	// keep the same init parameter
	BTS_Open(&mpHandle,
						   mWorkingBuffer, 
						   sizeof(BTS_InitParam),
						   (void*)&mInitParam,
						   0);
	 
	 BTS_SetParameters(mpHandle, sizeof(BTS_RuntimeParam),(void*)&mBTS_RTParam);
}

int AudioMTKTimeStretch::init(int SampleRate, int inChannelCount, int ratio)
{
  
	int ret;
	mInitParam.SampleRate = SampleRate;
	mInitParam.StereoFlag = inChannelCount-1; 
	mInitParam.TD_FD = 1;
	/*if (ratio >200)
		ratio = 200;
	else if (ratio <50)
		ratio = 50;*/
	mBTS_RTParam.TS_Ratio= ratio; // initially no change. 
	BTS_GetBufferSize(&mInternalBufferSize, &mTempBufferSize);
	SXLOGD("InternalBufferSize %d, TempBufferSize %d",mInternalBufferSize, mTempBufferSize);

	//mWorkingBuffer = (char*)((int *)malloc(InternalBufferSize));
	//mTempBuffer = (int *)malloc(TempBufferSize);
	mWorkingBuffer = (char*) new int[mInternalBufferSize];
	mTempBuffer = new int[mTempBufferSize];
	SXLOGD("&mpHandle 0x%x", &mpHandle);
	SXLOGD("SampleRate %d", SampleRate);
	SXLOGD("inChannelCount %d", inChannelCount);
	SXLOGD("ratio %d",ratio);
	ret= BTS_Open(&mpHandle,
						mWorkingBuffer, 
						sizeof(BTS_InitParam),
						(void*)&mInitParam,
						0);
	if (ret!=0){
		SXLOGD("AudioMTKTimeStretch init, OPEN Fail");
		return -1;}
	BTS_SetParameters(mpHandle, sizeof(BTS_RuntimeParam),(void*)&mBTS_RTParam);
	return 0;
}
/*void AudioMTKTimeStretch::Close()
{
	
	BTS_Close(mpHandle, 1);

	if(NULL != mWorkingBuffer) {
		delete[] mWorkingBuffer;
		mWorkingBuffer = NULL;
	}
	if(NULL != mTempBuffer) {
		delete[] mTempBuffer;
		mTempBuffer = NULL;
	}
	
	 *mpHandle = NULL;
	 

}*/
/*
inputByte: I: input sample, O: remained sample


*/
int AudioMTKTimeStretch::process(short* InputBuffer, short* OutputBuffer ,int* inputByte, int* outputByte )
{
	int process_smple_cnt,in_smpl_cnt,OutputCount ;
	short*  InputBufferPtr = InputBuffer;		
	short*  OutputBufferPtr = OutputBuffer;	
	int OutSampleCount = *outputByte >> 1;
	int OutTotalSampleCount = 0;
	int OutBuf_smpl = OutSampleCount; // total buffer size
	process_smple_cnt = *inputByte >> 1;//(1 + mInitParam.StereoFlag);
	in_smpl_cnt = process_smple_cnt;
	
	int X_WinSize = 256;
	if(mInitParam.SampleRate<=24000)
		X_WinSize >>= 1;
	if(mInitParam.SampleRate <=12000)
		X_WinSize >>= 1;
	SXLOGD("* input Buffer 0x%x, *OutputBuffer 0x%x , * inputByte %d, outputByte %d",InputBuffer,OutputBuffer,* inputByte ,*outputByte);
	//if(in_smpl_cnt < 512 || mInBuf_smpl_cnt !=0)
	{
		short* inptr = mInputBuffer + mInBuf_smpl_cnt;
		memcpy(inptr,InputBuffer,  *inputByte );
		mInBuf_smpl_cnt += in_smpl_cnt;
		ALOGD("add input sample %d, mInBuf_smpl_cnt %d", in_smpl_cnt, mInBuf_smpl_cnt);
		in_smpl_cnt = 0;
		process_smple_cnt = 0;
	}
	if(mInBuf_smpl_cnt >= X_WinSize)
	{
		ALOGD("use internal buffer sample count %d", mInBuf_smpl_cnt);
		in_smpl_cnt = mInBuf_smpl_cnt;
		InputBufferPtr = mInputBuffer;
		process_smple_cnt = in_smpl_cnt;
	}
	while(in_smpl_cnt >= X_WinSize && OutSampleCount >0)
	{
	SXLOGD("in_smpl_cnt %d", in_smpl_cnt);
	SXLOGD("OutSampleCount %d", OutSampleCount);
		  /* ========================================================= */
		  /* Process one block of data								   */
		  /* ========================================================= */
		  //OutputLength =0x240; //0x1c8;
		  
		  {
			 BTS_Process(mpHandle,
									 (char*)mTempBuffer,
									 (const short*)InputBufferPtr,
									 &process_smple_cnt,
									 OutputBufferPtr,
									 &OutSampleCount);
		  }
		  /*
		  else
		  {
			 memcpy(OutputBuffer, InputBuffer, process_smple_cnt*sizeof(short));
		  }
		  */
		  

			  SXLOGD("process_smple_cnt %d, byte %d\n",process_smple_cnt,(process_smple_cnt<< 1));

		  InputBufferPtr = InputBufferPtr + (process_smple_cnt); //advance input buffer
		  in_smpl_cnt -= process_smple_cnt; // process_smple_cnt is consummed sample count.
		  process_smple_cnt = in_smpl_cnt; // Next Round inbuffer size.
		  OutputBufferPtr += OutSampleCount; // advance out buffer
		  OutTotalSampleCount += OutSampleCount; // accumulate out sample count
		  OutSampleCount = OutBuf_smpl - OutTotalSampleCount; // next round out buffer size
		  SXLOGD("%10d sample generated!\n",OutTotalSampleCount);
	   }
	
#ifdef MTK_AUDIO
#if 0//  def DEBUG_AUDIO_PCM
	const int SIZE = 256;
	char fileName[SIZE];
	sprintf(fileName,"%s_%p.pcm",gaf_timestretch_in_pcm,this);
	if(OutTotalSampleCount !=0)
	{
	short* dump_ptr;
	int dump_sz;
	dump_ptr = mInputBuffer;
	dump_sz = mInBuf_smpl_cnt - in_smpl_cnt;
	AudioDump::dump(fileName,dump_ptr,(dump_sz << 1),gaf_timestretch_in_propty);
	ALOGD(" process dump addr %x, size %d", InputBuffer,(*inputByte) -(in_smpl_cnt<< 1 ) );
	//sprintf(fileName,"%s_%p.pcm",gaf_timestretch_out_pcm,this);
	//AudioDump::dump(fileName,OutputBuffer,OutTotalSampleCount<<1 ,gaf_timestretch_out_propty);
	}
#endif
#endif
	//if(mInBuf_smpl_cnt > 512)
	{
		if(in_smpl_cnt != 0)

		{
			short* inptr = mInputBuffer + (mInBuf_smpl_cnt - in_smpl_cnt);
			memcpy(mInputBuffer, inptr,  (in_smpl_cnt<<1) );
		}
		mInBuf_smpl_cnt = in_smpl_cnt;
		ALOGD("update internal buffer sample count %d", mInBuf_smpl_cnt );
	}
	// return unused input sample count.						
	*inputByte = 0;// in_smpl_cnt<< 1;
	*outputByte = OutTotalSampleCount <<1;
	
	return 0;

}
int AudioMTKTimeStretch::setParameters(
							   int *ratio)
{
	SXLOGD("bf setParameters %d", *ratio);
	if(mpHandle ==NULL)
		return -1;
	if(mBTS_RTParam.TS_Ratio == *ratio)
	{
		SXLOGD("af setParameters %d", *ratio);	
		return 0;}
	mBTS_RTParam.TS_Ratio = *ratio;
	BTS_SetParameters(mpHandle,(const int) sizeof(BTS_RuntimeParam),(const void*)&mBTS_RTParam);
	mBTS_RTParam.TS_Ratio = mBTS_RTParam.TS_Ratio;
	
	SXLOGD("af setParameters %d", *ratio);	
	return 0;
}


}

