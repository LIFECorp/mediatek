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
\file		Osw_FileSystem.c

\brief		Linux event implementation

This file contains the application implementation to linux critical section
*/


#include <stdio.h>
#include <unistd.h>
#include "Osw.h"
#include "SmsPlatDefs.h"

#define CWD_MAX_LEN		1024
#define CWD_MAX_FILENAME_LEN	256

char g_cwd[CWD_MAX_LEN];

/*! Init file system - empty function
\param[in]	arg: unused
\return		always 0
*/

UINT32 OSW_FS_Init( UINT32 arg )
{
	memset (g_cwd, 0, CWD_MAX_LEN);
	return 0;
}

/*! set current working directory
\param[in]	cwd: new directory path
\return		error status
*/

UINT32 OSW_FS_SetCwd( const char* cwd )
{
	UINT32	cwd_len;

	cwd_len = strlen(cwd);
	if (cwd_len > CWD_MAX_LEN - 2)
		return 0x80000010;

	strcpy(g_cwd, cwd);
	
	/* force '/' at the end of the current working directory */
	if (g_cwd[cwd_len-1] != '/') {
		g_cwd[cwd_len] = '/';
		g_cwd[cwd_len+1] = 0;
	}
	
	return OSW_OK;	
}

/*! Open file
\param[in]	filename: name of file 
\param[in]	attributes: open attributes 
\return		file handle
*/

OSW_FILEHANDLE OSW_FS_Open(	const char* filename,
					   	const char* attributes )
{
	UINT32	cwd_len;
	char	full_filename[CWD_MAX_LEN+CWD_MAX_FILENAME_LEN];

	cwd_len = strlen(g_cwd);
	
	strcpy(full_filename, g_cwd);

	strcpy(&full_filename[cwd_len], filename);
	
	return fopen(full_filename,attributes);
}

/*! close file
\param[in]	hFile: file handle 
\return		error status
*/

UINT32 OSW_FS_Close(OSW_FILEHANDLE hFile)
{
	return fclose(hFile);
}

/*! write data to file
\param[in]	hFile: file handle
\param[in]	pBuffer: data buffer
\param[in]	buffLen: buffer length

\return		number of bytes written
*/
UINT32 OSW_FS_Write(OSW_FILEHANDLE hFile,
					void* IN   pBuffer,
					UINT32	   buffLen)
{
	return fwrite(pBuffer,1,buffLen,hFile);
}

/*! read data from file
\param[in]	hFile: file handle
\param[in]	pData: data buffer
\param[in]	dataLen: buffer length

\return		number of bytes read
*/
UINT32 OSW_FS_Read(	OSW_FILEHANDLE	hFile,
				   	void* OUT	pData,
				   	UINT32		dataLen)
{
	return fread(pData,1,dataLen,hFile);
}

/*! delete file
\param[in]	filename: name of file 
\return		error status
*/

UINT32 OSW_FS_Delete(const char* filename)
{	
	UINT32	cwd_len;
	char	full_filename[CWD_MAX_LEN+CWD_MAX_FILENAME_LEN];

	cwd_len = strlen(g_cwd);
	
	strcpy(full_filename, g_cwd);

	strcpy(&full_filename[cwd_len], filename);
	
	if (remove(full_filename) == 0)
	{
		return OSW_OK;
	}
	else
	{
		return OSW_ERROR;
	}
}

#ifdef SMS_OSW_FS_EXTENSIONS

UINT32 OSW_FS_Tell(	OSW_FILEHANDLE	hFile)
{
	return ftell(hFile); 
}



UINT32 OSW_FS_Printf(OSW_FILEHANDLE	hFile, const char *fmt, ...)
{

	UINT32 NumWritten; 
	va_list Args;

	va_start(Args, fmt);
	NumWritten = vfprintf(hFile, fmt, Args);
	va_end(Args);

	return NumWritten; 
}



BOOL OSW_FS_Flush(OSW_FILEHANDLE hFile)
{
	return fflush(hFile) == 0; 
}


BOOL OSW_FS_Seek(OSW_FILEHANDLE hFile, UINT32 offset, INT32 whence)
{
	const int WHENCE_VALS[] = {SEEK_SET, SEEK_CUR, SEEK_END};  

	return fseek(hFile, offset, WHENCE_VALS[whence]) == 0; 
}


BOOL OSW_FS_Putc(OSW_FILEHANDLE hFile, INT32 c)
{
	return fputc(c, hFile) != EOF; 
}


OSW_FILEHANDLE OSW_FS_Dopen(INT32 fd, const char *attributes)
{
	return (OSW_FILEHANDLE)fdopen(fd, attributes); 
}



void OSW_FS_ClearErr(OSW_FILEHANDLE hFile)
{
	clearerr(hFile); 
}

INT32 OSW_FS_Error(OSW_FILEHANDLE hFile)
{
	return ferror(hFile);
}

#endif
