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

#define TIMEOUT	5
#define DO_SOMETHING
#define DO_CNT	1000

#ifdef DO_SOMETHING
#define DO_FUNC(cnt)				\
	do {							\
		int i, j, sum;				\
		for(i=0; i<cnt; i++)		\
			for(j=0; j<10000; j++)	\
				sum += i * j;		\
	} while(0);
#else
#define DO_FUNC(cnt)
#endif


void prn_pid(char *name)
{
	printf("%s -> pid:%d ppid:%d tid:%d\n",
	       name, getpid(), getppid(), (int)syscall(__NR_gettid));
	return;
}


void *threadfuncA(void *arg)
{
	int class_id=100;
	int cnt=0;
	char pname[32];

	strcpy(pname, arg);
	prn_pid(pname);
	class_id = (int)syscall(__NR_gettid);
	//met_tag_start(class_id, pname);
	while ((cnt++)<TIMEOUT) {
		met_tag_oneshot(class_id, pname, cnt);
		DO_FUNC(DO_CNT);
		sleep(1);
	}
	printf("%s Terminate!\n", pname);
	//met_tag_end(class_id, pname);
	return NULL;
}


void *threadfuncB(void *arg)
{
	pthread_t thrdID1, thrdID2;
	int class_id=100;
	int cnt=0;
	char name0[32];
	char name1[32];
	char name2[32];

	prn_pid(arg);
	strcpy(name0, arg);
	class_id = (int)syscall(__NR_gettid);
	//met_tag_start(class_id, name0);
	if (fork() == 0) { //child process
		strcat(name0, "/CP1");
		prn_pid(name0);
		strcpy(name1, name0);
		strcat(name1, "/T1");
		pthread_create(&thrdID1, NULL, threadfuncA, name1);
		strcpy(name2, name0);
		strcat(name2, "/T2");
		pthread_create(&thrdID2, NULL, threadfuncA, name2);
		while ((cnt++)<TIMEOUT) {
			met_tag_oneshot(class_id, name0, cnt);
			DO_FUNC(DO_CNT);
			sleep(1);
		}
		printf("%s Terminate!\n", (char *)name0);
		//met_tag_end(class_id, name0);
	}
	else {
		class_id = (int)syscall(__NR_gettid);
		//met_tag_start(class_id, name0);
		strcpy(name1, name0);
		strcat(name1, "/T1");
		pthread_create(&thrdID1, NULL, threadfuncA, name1);
		strcpy(name2, name0);
		strcat(name2, "/T2");
		pthread_create(&thrdID2, NULL, threadfuncA, name2);
		while ((cnt++)<TIMEOUT) {
			met_tag_oneshot(class_id, name0, cnt);
			DO_FUNC(DO_CNT);
			sleep(1);
		}
		printf("%s Terminate!\n", (char *)name0);
		//met_tag_end(class_id, name0);
	}
	return NULL;
}


int main(int argc, char *argv[])
{
	pthread_t thrdID1, thrdID2;
	int cnt=0;
	int class_id=100;

#ifdef MET_USER_EVENT_SUPPORT
	printf("======= HAS MET_USER_EVENT_SUPPORT ==========\n");
#else
	printf("======= NO MET_USER_EVENT_SUPPORT ==========\n");
#endif

	prn_pid("MP");
	if(met_tag_init()<0) {
		printf("Error:Cannot init mtag libs\n");
		return -1;
	}
	met_record_on();

	class_id = (int)syscall(__NR_gettid);
	//met_tag_start(class_id, "Main Program");

	if (fork() == 0) { //child process
		prn_pid("MP/CP1");
		class_id = (int)syscall(__NR_gettid);
		//met_tag_start(class_id, "MP/CP1");
		pthread_create(&thrdID1, NULL, threadfuncA, "MP/CP1/T1");
		pthread_create(&thrdID2, NULL, threadfuncB, "MP/CP1/T2");
		while ((cnt++)<TIMEOUT) {
			met_tag_oneshot(class_id, "MP/CP1", cnt);
			DO_FUNC(DO_CNT);
			sleep(1);
		}
		//met_tag_end(class_id, "MP/CP1");
		printf("MP/CP1 Terminate!\n");
		sleep(1);
	}
	else {
		pthread_create(&thrdID1, NULL, threadfuncA, "MP/T1");
		pthread_create(&thrdID2, NULL, threadfuncA, "MP/T2");
		while ((cnt++)<TIMEOUT) {
			met_tag_oneshot(class_id, "Main Program", cnt);
			DO_FUNC(DO_CNT);
			sleep(1);
		}
		//printf("MP Terminate!\n");
		printf("MP Terminated!\n");
		//met_tag_end(class_id, "MP");
		sleep(1);
		met_record_off();
		met_tag_uninit();
	}

	return 0;
}
