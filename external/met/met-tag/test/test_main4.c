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


#include <sys/types.h>
#include <sys/syscall.h>

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "met_tag.h"

#define THRD_NUM	3

char *tname[THRD_NUM] = { "AA", "BB", "CC" };
int func_time[THRD_NUM] = { 200000, 400000, 600000};
pthread_t thrdID[THRD_NUM];
volatile int bEnd=0;

void *threadfunc(void *arg)
{
	int i=0;
	char pname[32];

	i = (int)arg;
	usleep(i*300000);
	while (bEnd == 0) {
		met_tag_start(i+1, tname[i]);
		sleep(1);
//		usleep(func_time[i]);
		met_tag_end(i+1, tname[i]);
//		usleep(300000);
//		met_tag_oneshot(i+1, tname[i], 3);
		sleep(1);
	}
	return NULL;
}



int main(int argc, char *argv[])
{
	int timeout=3;
	int i, cnt=0;
	int rate=100;

#ifdef MET_USER_EVENT_SUPPORT
	printf("======= HAS MET_USER_EVENT_SUPPORT ==========\n");
#else
	printf("======= NO MET_USER_EVENT_SUPPORT ===========\n");
#endif
#if 0
	if(argc >= 2)
		timeout = atoi(argv[1]);

	if(argc >= 3) {
		rate = atoi(argv[2]);
		for(i=0; i<THRD_NUM; i++) {
			func_time[i] = (func_time[i]*rate)/100;
		}
	}
#endif

	if(met_tag_init()<0) {
		printf("Error:Cannot init mtag libs\n");
		return -1;
	}
//	met_record_on();

	for(i=0; i<THRD_NUM; i++) {
		pthread_create(&thrdID[i], NULL, threadfunc, (void*)i);
	}

	while ((cnt++)<timeout) {
		sleep(1);
	}

	bEnd = 1;
	sleep(1);

//	met_record_off();
	met_tag_uninit();
	printf("======= TEST END ==========\n");
	return 0;
}
