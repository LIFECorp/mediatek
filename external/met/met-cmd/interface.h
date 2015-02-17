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

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <getopt.h>

typedef struct _met_context_t
{
	int met_exist;
	int met_plf_exist;
	const char *ftrace_events;
} met_context_t;

typedef struct _met_node_t
{
	struct _met_node_t *next;

	int type;
	const char *name;

	struct option *options;
	int option_size;

	int (*init) (int met_exist, int met_plf_exist);
	int (*process_arg) (int option, char *arg);
	int (*check_arg) (met_context_t *ctx);
	void (*prn_helper) (void);
	int (*start) (void);
	void (*stop) (void);
	void (*output_header) (FILE *fp_out);

} met_node_t;

typedef struct _timestamp_t
{
	int sec;
	int usec;
} timestamp_t;

typedef struct _logbuf_info_t
{
	int records;
	int bytes;
	int overrun;
	timestamp_t first_time;
	timestamp_t last_time;

} logbuf_info_t;


#define INTERRUPT	"1"
#define SAMPLING	"2"

extern struct option *pOptions;
extern int has_met;
extern int has_met_plf;

int met_set_options(struct option *new_options, int size);
int met_prn_options(void);
int met_register(met_node_t *node);
int met_deregister(met_node_t *node);
int met_process_arg(int option, char *arg);
int met_check_arg(met_context_t *conext);
void met_prn_helper(void);
int met_start(void);
void met_stop(void);
void met_output_header(FILE *fp_out);

int met_node_exist(char *buf);
char *met_get_platform(void);
int met_probe_devices(void);

#define LOGBUF_INFO_RECORDS		1
#define LOGBUF_INFO_BYTES		2
#define LOGBUF_INFO_OVERRUN		3
#define LOGBUF_INFO_STIME		4
#define LOGBUF_INFO_ETIME		5

int logbuf_info_add(char *p, logbuf_info_t *info, int type);
void logbuf_info_print(logbuf_info_t info[], int max_cpu, FILE *fp_out);
int get_num(const char *str, unsigned int *value, int null_as_zero);
int get_num_backward(char *dc, int *pValue, int len);
int get_timestamp(char *p, long long *pts);

#define NOMSG_IF_NOEXIST	0x1
#define NOMSG_IF_ERROR		0x2
#define NOMSG_OPERATION		0x4

int sys_read(char *path, char *str, int len, int flag);
int sys_write(char *path, char *str, int flag);

#endif //_INTERFACE_H_
