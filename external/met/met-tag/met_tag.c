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



#ifdef MET_USER_EVENT_SUPPORT
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "met_tag.h"

#define DEBUGFS		"/sys/kernel/debug/tracing/"

static int mtag_fd = -1;

int met_tag_start(unsigned int class_id, const char *name)
{
	mtag_cmd_t cmd;

	if(mtag_fd < 0) return 0;

	cmd.class_id = class_id;
	cmd.slen = strlen(name) + 1;
	memcpy(cmd.tname, name, cmd.slen);
	return ioctl(mtag_fd, MTAG_CMD_START, &cmd);
}

int met_tag_end(unsigned int class_id, const char *name)
{
	mtag_cmd_t cmd;

	if(mtag_fd < 0) return 0;

	cmd.class_id = class_id;
	cmd.slen = strlen(name) + 1;
	memcpy(cmd.tname, name, cmd.slen);
	return ioctl(mtag_fd, MTAG_CMD_END, &cmd);
}

int met_tag_oneshot(unsigned int class_id, const char *name, unsigned int value)
{
	mtag_cmd_t cmd;

	if(mtag_fd < 0) return 0;

	cmd.class_id = class_id;
	cmd.value = value;
	cmd.slen = strlen(name) + 1;
	memcpy(cmd.tname, name, cmd.slen);
	return ioctl(mtag_fd, MTAG_CMD_ONESHOT, &cmd);
}

int met_tag_dump(unsigned int class_id, const char *name, void *data, unsigned int length)
{
	mtag_cmd_t cmd;

	if(mtag_fd < 0) return 0;

	cmd.class_id = class_id;
	cmd.slen = strlen(name) + 1;
	memcpy(cmd.tname, name, cmd.slen);
	cmd.data = data;
	cmd.size = length;
	return ioctl(mtag_fd, MTAG_CMD_DUMP, &cmd);
}

int met_tag_disable(unsigned int class_id)
{
	if(mtag_fd < 0) return 0;

	return ioctl(mtag_fd, MTAG_CMD_DISABLE, class_id);
}

int met_tag_enable(unsigned int class_id)
{
	if(mtag_fd < 0) return 0;

	return ioctl(mtag_fd, MTAG_CMD_ENABLE, class_id);
}

int met_tag_init(void)
{
	//Check if the mtag
	if(mtag_fd >= 0) return 0;

	mtag_fd = open("/dev/met", O_RDWR);
	if(mtag_fd<0) {
		printf("Error: cannot open mtag device, err=%d(%s)\n",
		        errno, strerror(errno));
		return -1;
	}
	
	met_tag_oneshot(0, "_MET_TAG_INIT_", 0);
	return 0;
}

int met_tag_uninit(void)
{
	//Changed by bktseng: 
	//file closed by clean up when program was terminated
	//int ret;
	//if(mtag_fd < 0) return 0;
	//ret = close(mtag_fd);
	//mtag_fd = -1;
	//return ret;
	return 0;
}

int met_record_on(void)
{
	if(mtag_fd < 0) return 0;

	return ioctl(mtag_fd, MTAG_CMD_REC_SET, 1);
}

int met_record_off(void)
{
	if(mtag_fd < 0) return 0;

	return ioctl(mtag_fd, MTAG_CMD_REC_SET, 0);
}

int met_set_dump_buffer(int size)
{
	if(mtag_fd < 0) return 0;

	return ioctl(mtag_fd, MTAG_CMD_DUMP_SIZE, size);
}

int met_save_dump_buffer(const char *pathname)
{
	mtag_cmd_t cmd;

	if(mtag_fd < 0) return 0;

	cmd.slen = strlen(pathname) + 1;
	memcpy(cmd.tname, pathname, cmd.slen);
	return ioctl(mtag_fd, MTAG_CMD_DUMP_SAVE, &cmd);
}

#endif
