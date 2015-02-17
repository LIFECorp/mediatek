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

/****************************************************************

Siano Mobile Silicon, Inc. 
MDTV receiver kernel modules. 
Copyright (C) 2006-2008, Uri Shkolnik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************/
/*!                                                                  
	\file		Osw_Task.c
                                                                       
	\brief		linux thread implementation
*/

#include <time.h>
#include <stdlib.h>
#include "Osw.h"
#include "SmsPlatDefs.h"

typedef void *(*PthreadTaskFunc)(void*);

/*!  create a task
	\param[in]	TaskName: name of the Task
	\param[in]	TaskPriority: priority of the task
	\param[in]	TaskStackSize: stack size of the Task
	\param[in]	TaskFunction: the function that this task will run
	\param[in]	TaskFunctionParams: parameters to be passed to the task function
	\return		handle to the task.
*/
OSW_TaskId OSW_TaskCreate ( const char *TaskName,
					   UINT32 TaskPriority,
					   UINT32 TaskStackSize,
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
		// if failed - free task structure
		free(TaskHandle);
		return NULL;
	}

	return (void*)TaskHandle;
}

/*!  cancel a task
	\param[in]	pTask: handle to the task.
*/
void OSW_TaskCleanup ( OSW_TaskId pTask )
{
	// android don't support pthread_cancel
	//// cancel thread
	//pthread_cancel ( *(pthread_t*)pTask );

	// free task structure
	free((pthread_t*)pTask);
}

/*!  sleep
	\param[in]	TaskSleepPeriod: time to sleep.
*/
void OSW_TaskSleep ( UINT32 TaskSleepPeriod )
{
	struct timespec ts, tr;
	
	// calculate wakeup time
	ts.tv_sec = TaskSleepPeriod / 1000;
	ts.tv_nsec = ( TaskSleepPeriod % 1000 ) * 1000000;
	// sleep
	nanosleep ( &ts, &tr );
}

/*!  unimplemented */
OSW_TaskId OSW_TaskGetCurrent ( void )
{
	return NULL;
}

