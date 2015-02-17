/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "met_tag.h"

#define TIMEOUT	5
#define DO_SOMETHING
#define DO_CNT	1000

#ifdef DO_SOMETHING
#define DO_FUNC(cnt)				\
	do {							\
		int i, j, sum=0;			\
		for(i=0; i<cnt; i++)		\
			for(j=0; j<1000; j++)	\
				sum += i * j;		\
	} while(0);
#else
#define DO_FUNC(cnt)
#endif

int *gdiff1;
int *gdiff2;

int time_diff(struct timeval *start_t, struct timeval *end_t)
{
	int diff;
    diff = (end_t->tv_sec - start_t->tv_sec)*1000000 + (end_t->tv_usec - start_t->tv_usec);
	return diff;
}

void funcA(int cnt1, int cnt2, int flag)
{
	struct timeval st;
	struct timeval ed;

	gettimeofday(&st, NULL);
	if(flag)
		met_tag_start(0, "TAG_TEST");

	DO_FUNC(cnt2);

	if(flag)
		met_tag_end(0, "TAG_TEST");
	gettimeofday(&ed, NULL);
	
	if(flag) {
		gdiff1[cnt1] = time_diff(&st, &ed);
	}
	else {
		gdiff2[cnt1] = time_diff(&st, &ed);
	}
	return;
}

int main(int argc, char *argv[])
{
	int i, cnt1=0, cnt2=0, cnt3;
	int max, min, avg;
#ifdef MET_USER_EVENT_SUPPORT
	printf("======= HAS MET_USER_EVENT_SUPPORT ==========\n");
#else
	printf("======= NO MET_USER_EVENT_SUPPORT ==========\n");
#endif

	if (argc != 3) {
		printf("Usage:%s cnt1 cnt2 flag\n", argv[0]);
		printf("Usage: cnt1: count to execute test function\n");
		printf("Usage: cnt2: count inside test function\n");
		printf("Usage: cnt1 must be 1~1000\n");
		printf("Usage: cnt2 must be 100~10000\n");
		return -1;
	}

	cnt1 = atoi(argv[1]);
	cnt2 = atoi(argv[2]);
	
	if((cnt1<=0)||(cnt1>1000)) {
		printf("Error: counter 0 must be 1 ~ 1000\n");
		return -1;
	}

	if((cnt2<100)||(cnt2>10000)) {
		printf("Error: counter 2 must be 100 ~ 10000\n");
		return -1;
	}

	gdiff1 = (int*)malloc(sizeof(int)*cnt1);
	gdiff2 = (int*)malloc(sizeof(int)*cnt1);
	if((gdiff1==NULL)||(gdiff2==NULL)) {
		printf("Error: Cannot alloc memory\n");
		return -1;
	}
	memset(gdiff1, 0, sizeof(int)*cnt1);
	memset(gdiff2, 0, sizeof(int)*cnt1);

	//============================================================
	for(i=0; i<cnt1; i++) {
		funcA(i, cnt2, 0);
	}
	//============================================================


	//============================================================
	if(met_tag_init()<0) {
		printf("Error:Cannot init mtag libs\n");
	}

	for(i=0; i<cnt1; i++) {
		funcA(i, cnt2, 1);
	}
	//============================================================

	cnt3 = 0;
	for(i=0; i<cnt1; i++) {
		if(gdiff1[i] > gdiff2[i]) {
			gdiff1[i] = gdiff1[i] - gdiff2[i];
		}
		else {
			gdiff1[i] = 0;
			cnt3++;	
		}
	}
	
	max = 0;
	min = 0x7fffffff;
	avg = 0;
	for(i=0; i<cnt1; i++) {
		if(gdiff1[i] > max)	max = gdiff1[i];
		if(gdiff1[i] < min)	min = gdiff1[i];
		avg += gdiff1[i];
	}
	cnt1 -= cnt3;
	avg = avg/cnt1;

	printf("MET Tagging (Cnt=%d): Max=%d Min=%d Avg=%d\n", cnt1<<1, max>>1, min>>1, avg>>1);

	return 0;
}
