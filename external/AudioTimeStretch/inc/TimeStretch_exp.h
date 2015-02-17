/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 * BesTS_exp.h
 *
 * Project:
 * --------
 * BesSound
 *
 * Description:
 * ------------
 * BesTS (Time stretch) interface
 *
 * Author:
 * -------
 * Guyger Fan
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 * 10 07 2010 richie.hsieh
 * [WCPSP00000522] [BesSound SWIP] Assertion removal
 * .
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __BESTS_EXP_H__
#define __BESTS_EXP_H__

typedef struct {
   unsigned int SampleRate;
   // Sampling rate of audio samples that are going to be processed.
   int StereoFlag;
   // 0==> Mono
   // 1==> Stereo  
   int TD_FD;
   // 0 ==> FD
   // 1 ==> TD
}BTS_InitParam;

typedef struct {
   int TS_Ratio;    // 55  ~ 100 ~ 199
                    //0.55    1    1.99
   // < 100 ==> time compression
   // > 100 ==> time extension
}BTS_RuntimeParam;

typedef struct {
    int Version;
    // The number of samples that applications have to feed to 
    // engines for flushing out the audio samples buffered in engines,
    // or to flush out the tail.
    // L/R pair counts as 2 samples.
    int FlushOutSampleCount;
} BTS_EngineInfo;

int BTS_GetBufferSize(unsigned int *uiInternalBufferSize, unsigned int *uiTempBufferSize);
int BTS_Open(void **pp_handle,
              char *pInternalBuffer, 
              const int cBytes,
              const void *pInitParam,
              int bEnableOLA);
int BTS_SetParameters(void *pHandle,
                              const int cBytes,
                              const void *pRuntimeParam);
int BTS_Process(void *handle,
                        char *pTempBuffer,
                        const short *pInputBuffer,
                        int   *InputSampleCount,
                        short *pOutputBuffer,
                        int   *OutputSampleCount);
int BTS_Close(void *pHandle, int bEnableOLA);

#endif /* __SURROUND_H__ */
