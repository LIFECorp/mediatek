/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef ANDROID_AUDIO_MTK_TIMESTRETCH_H
#define ANDROID_AUDIO_MTK_TIMESTRETCH_H

#include "TimeStretch_exp.h"

namespace android {
// ----------------------------------------------------------------------------
class AudioMTKTimeStretch  {

public:
	
//============================================================//
// ratio = 100 : original speed.
// ratio = 200: stretch length x2
// ratio = 400 stretch length x4
// inChannelCount: Mono : 1, Stereo : 2
// sampleRate 	
// input framecount. for allocate internal buffer.	
//============================================================//
    AudioMTKTimeStretch(int ratio, int inChannelCount, int sampleRate, int framecount);
	 
    // input framecount. for allocate internal buffer.	
    AudioMTKTimeStretch(int framecount);

    ~AudioMTKTimeStretch();
	
//============================================================//
//setParameters
 // *pRuntimeParameter: use to set time stretch ratio.
 // *pRuntimeParam = 100 : original speed.
 // *pRuntimeParam = 200: stretch length x2
 // *pRuntimeParam = 400 stretch length x4 
//============================================================//
    int setParameters(
								   int *pRuntimeParam);
	 
//============================================================//
// Process data from input buffer to output buffer, output can only be Stereo Channel.
// * pInputBuffer: input buffer pointer.
//* pOutputBuffer: output buffer pointer.
// InputSampleCount:	input,
// [I/O]				the number of intput bytes.
//					the number of intput bytes that is NOT consumed by timestretch.
// OutputSampleCount:	
//[I] 					output, the number of valid bytes in output buffer
//[O]				the number of	generated output bytes.
//============================================================//
    int process(							 short* pInputBuffer,
							short* pOutputBuffer,
							int * InputSampleCount,
							int* OutputSampleCount);	
//============================================================//
//  reset(void)
// Reset Internal buffer. and re-Init.
//============================================================//
    void reset(void);

//============================================================//
//	init: Initialize
// Must be executed before processing.
//============================================================//
     	
    int init(int SampleRate, int inChannelCount, int ratio);
	BTS_RuntimeParam mBTS_RTParam;

private:
	void *mpHandle;
	BTS_InitParam mInitParam;
	char * mWorkingBuffer;
	int* mTempBuffer;
	short* mInputBuffer; // for accumulate input sample.
	int mInputBufferSize; // mInputBuffer.
	int mInBuf_smpl_cnt;
	unsigned int mInternalBufferSize;
	unsigned int mTempBufferSize;

	};

}
#endif
