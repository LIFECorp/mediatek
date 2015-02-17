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

/*!  

	\file		Osw_Task.h
	
	\brief		OS Wrapper - Tasks header

				The following are functions for task mechanism.

	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  

    \note 
	    -# Example of Task structure definition can be seen for Win32 OS at: \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Win/Include/Osw_TaskTarget.h </CODE> \n
        Should you need to change the above definition, please use your own OS directory: \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Linux/Include/Osw_TaskTarget.h </CODE> \n
		The TaskPriority is exactly the selected OS priority number. 
		It is warmly recommended to add a generic priority \n 
		definition envelope to make the OS wrapper an OS independent at all. 
		In the following example one should use its OS appropriate priority numbers. \n
        <CODE> typedef enum TaskPriority_E   </CODE> \n
        <CODE> {                             </CODE> \n 
		<CODE>     TASK_PRIORITY_HIGHEST	= 0, </CODE> \n
		<CODE>     TASK_PRIORITY_2 			= 1, </CODE> \n
		<CODE>     TASK_PRIORITY_3 			= 2, </CODE> \n
		<CODE>     TASK_PRIORITY_4 			= 3, </CODE> \n
		<CODE>     TASK_PRIORITY_63 		= 62,</CODE> \n
		<CODE>     TASK_PRIORITY_LOWEST 	= 63,</CODE> \n
        <CODE> } TaskPriority_ET;            </CODE> 
						 	
*/

#ifndef __OSW_TASK_H
#define __OSW_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Osw.h"

typedef UINT32 (*TaskFunc)(UINT32);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*! 
	Creates a task and makes it ready to run, or suspended.

    \param[in]   TaskName            A pointer to a unique task name string.
    \param[in]   TaskPriority        The task priority.
    \param[in]   TaskStackSize       The task stack size in bytes.
    \param[in]   TaskFunction        TaskFunc type task function to run.
    \param[in]   TaskFunctionParams  A pointer to an optional parameter to be passed to the task when it is activated.

	\return		                     Task ID on success, or NULL if operation failes.

*/
OSW_TaskId OSW_TaskCreate (const char*	TaskName,
					  UINT32		TaskPriority,
					  UINT32		TaskStackSize,
					  TaskFunc		TaskFunction,
					  void*			TaskFunctionParams);


/*! 
	Cleanup after task termination - if applicable (e.g. closing task handlers)

    \param[in]   TaskId            Task ID.

*/
void  OSW_TaskCleanup   (OSW_TaskId TaskId);

/*! 

	suspend the active task for the number of milliseconds
    
    \param[in]   TaskSleepPeriod     The maximum number of milliseconds to suspend the task.

*/
void  OSW_TaskSleep  (UINT32 TaskSleepPeriod);

/*! 
	Returns the current running task ID.
    
	\return							Current running task ID.

*/
OSW_TaskId OSW_TaskGetCurrent(void);


#ifdef __cplusplus
}
#endif

#endif
