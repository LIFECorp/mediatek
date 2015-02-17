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

	\file		Osw_Mutex.h
	
	\brief		OS Wrapper - Mutex header. 
	
				The following are functions for Mutex mechanism.
																
	\par		Copyright (c) 2005 Siano Mobile Silicon Ltd. All rights reserved	
																	   
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  

    \note 
	    -# Example of Mutex definition: \n
		    <CODE> typedef HANDLE Mutex; </CODE> \n
        Should you need to change the above definition, do as in \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Win/Include/Osw_MutexTarget.h </CODE> \n
        but in your own OS flavor directory (for example \"Linux\") \n
	    <CODE> ~/HostLib/AdaptLayers/OS/Linux/Include/Osw_MutexTarget.h </CODE>
																	 	
*/

#ifndef __OS_MUTEX_H
#define __OS_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Osw.h"

///////////////////////////////////////////////////////////////////////////////
/*! 
	Create a mutual exclusion synchronization object to handle critical
	sections in task context.

	\param[out] pMutex		Where to store the mutex semaphore

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexCreate
	(
	MUTEX*					pMutex
	);


/*!
    Delete a previously created Mutex object [reverse operation to OSW_MutexCreate()]

	\param[in]  pMutex      Pointer to Mutex to delete

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexDelete
	(
	MUTEX*					pMutex
	);


/*!
    Mutex GET operation (at the start of a critical section)

	\param[in]  pMutex      Pointer to Mutex to get

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexGet
	(
	MUTEX*					pMutex
	);


/*!
    Mutex PUT operation (at the end of a critical section)

	\param[in]  pMutex      Pointer to Mutex to put

	\return					OSW_OK if OK - or a different value in case of error

*/
UINT32 OSW_MutexPut
	(
	MUTEX*					pMutex
	);


#ifdef __cplusplus
}
#endif

#endif
