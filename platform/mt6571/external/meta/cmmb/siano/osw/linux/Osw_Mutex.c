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
	\file		Osw_Mutex.c

	\brief		linux mutex implementation
*/

#include "Osw.h"
#include "SmsPlatDefs.h"


/*!  create a mutex
	\param[in] 	mutex: pointer to a mutex structure
	\return		status of the operation.
*/
UINT32 OSW_MutexCreate (MUTEX * mutex )
{
	pthread_mutexattr_t attr;
	SMS_ASSERT ( mutex );

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype((pthread_mutexattr_t *)&attr, (int)PTHREAD_MUTEX_RECURSIVE_NP);

	// init mutex
	int rc = pthread_mutex_init ( mutex, &attr );
	if ( rc != 0 )
	{
		return PTHREAD_MUTEX_ERROR | rc;
	}

	return rc;
}

/*!  release a mutex
	\param[in] 	mutex: pointer to a mutex
	\return		status of the operation.
*/
UINT32 OSW_MutexDelete ( MUTEX * mutex )
{

	SMS_ASSERT ( mutex );
	// destroy mutex
	int rc = pthread_mutex_destroy ( mutex );
	if ( rc != 0 )
	{
		return PTHREAD_MUTEX_ERROR | rc;
	}

	return rc;
}

/*!  unlock a mutex
	\param[in] 	mutex: pointer to a mutex
	\return		status of the operation.
*/
UINT32 OSW_MutexPut ( MUTEX * mutex )
{
	SMS_ASSERT ( mutex );

	// unlock mutex
	int rc = pthread_mutex_unlock ( mutex );
	if ( rc != 0 )
	{
		return PTHREAD_MUTEX_ERROR | rc;
	}
	return rc;
}

/*!  lock a mutex
	\param[in] 	mutex: pointer to a mutex
	\return		status of the operation.
*/
UINT32 OSW_MutexGet ( MUTEX * mutex )
{
	SMS_ASSERT ( mutex );

	// get mutex
	int rc = pthread_mutex_lock ( mutex );
	if ( rc != 0 )
	{
		return PTHREAD_MUTEX_ERROR | rc;
	}
	return rc;
}
