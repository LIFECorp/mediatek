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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "interface.h"

struct option *pOptions = NULL;
static int option_size = 0;

#define MAX_MET_DEVICES	20

typedef struct _met_device_t
{
	char type[8];
	char name[16];
	char arg_name[16];
	int mode;
} met_device_t;

met_device_t mdevice[MAX_MET_DEVICES];
int mdevice_num = 0;

int met_set_options(struct option *new_options, int size)
{
	int new_size;
	struct option *ptrNew;

	new_size = option_size + size;
	ptrNew = realloc(pOptions, (new_size + 1)*sizeof(struct option));
	if (ptrNew == NULL) {
		printf("!!! ERROR: memory not enough when realloc \"pOptions\" !!!\n");
		return -1;
	}

	pOptions = ptrNew;
	ptrNew = pOptions + option_size;
	memset(ptrNew, 0, (size+1)*sizeof(struct option));
	memcpy(ptrNew, new_options, size*sizeof(struct option));
	option_size = new_size;
	return 0;
}

int met_prn_options(void)
{
	int i;
	printf("================= option: %d====================\n", option_size);
	for (i=0; i<option_size; i++) {
		printf("name=%s, has_arg=%d, flag=%p, val=%c\n",
		        pOptions[i].name, pOptions[i].has_arg,
		        pOptions[i].flag, pOptions[i].val);
	}
	return 0;
}

met_node_t *met_list = NULL;

int met_register(met_node_t *node)
{
	int ret=0;

	if (node->init)
		ret = node->init(has_met, has_met_plf);

	if (ret<0)
		return -1;

	if (node->options)
		met_set_options(node->options, node->option_size);

	if (met_list==NULL)
		met_list = node;
	else {
		node->next = met_list;
		met_list = node;
	}

	return 0;
}


int met_deregister(met_node_t *node)
{
	met_node_t *tmp_node;

	//The deleted node is the head of the list
	if (met_list == node) {
		met_list = node->next;
		return 0;
	}

	//The list is empty
	if (met_list == NULL) {
		return -1;
	}

	tmp_node = met_list;
	while (tmp_node&&(tmp_node->next != node)) {
		tmp_node = tmp_node->next;
	}

	//No matched node found in the list
	if (tmp_node == NULL) {
		return -1;
	}

	tmp_node->next = node->next;
	return 0;
}

int met_process_arg(int option, char *arg)
{
	int i, ret = 0;
	met_node_t *tmp_node;

	if (option == '?') return 0;

	i = option - 'z' - 1;
	if ((i>=0)&&(i<mdevice_num)) {
		if (arg != NULL) {
			char path[64];
			snprintf(path, sizeof(path),
			     "/sys/class/misc/met/%s/%s/argu",
			     mdevice[i].type, mdevice[i].name);
			ret = sys_write(path, arg, 0);
			if (ret != 0)
				return ret;
			mdevice[i].mode = 2;
		}
		else
			mdevice[i].mode = 1;

		return 1;
	}

	tmp_node = met_list;
	while (tmp_node) {

		if (tmp_node->process_arg) {
			ret = tmp_node->process_arg(option, arg);
		}
		if (ret !=0)
			break;
		tmp_node = tmp_node->next;
	}

	return ret;
}

int met_check_arg(met_context_t *context)
{
	int ret;
	met_node_t *tmp_node;

	tmp_node = met_list;
	while (tmp_node) {
		ret = 0;
		if (tmp_node->check_arg) {
			ret = tmp_node->check_arg(context);
		}
		if (ret<0)	return (-1);
		tmp_node = tmp_node->next;
	}

	return 0;
}

void met_prn_helper(void)
{
	int i;
	met_node_t *tmp_node;
	tmp_node = met_list;
	while (tmp_node) {
		if (tmp_node->prn_helper) {
			tmp_node->prn_helper();
		}
		tmp_node = tmp_node->next;
	}

	for (i=0; i<mdevice_num; i++) {
		FILE *fp;
		char buf[256];
		snprintf(buf, sizeof(buf),
			     "/sys/class/misc/met/%s/%s/help",
			     mdevice[i].type, mdevice[i].name);
		fp = fopen(buf, "r");
		if(fp==NULL) {
			continue;
		}
		while ((fgets(buf, sizeof(buf), fp)) != NULL) {
			printf("%s", buf);
		}
		fclose(fp);

	}

}

int met_start(void)
{
	int i, ret;
	met_node_t *tmp_node;

	tmp_node = met_list;
	while (tmp_node) {
		ret = 0;
		if (tmp_node->start) {
			ret = tmp_node->start();
		}
		if (ret)	return -1;
		tmp_node = tmp_node->next;
	}

	for (i=0; i<mdevice_num; i++) {
		char *buf[2] = { "0", "1" };
		char path[64];

		if ((mdevice[i].mode != 0)&&(mdevice[i].mode != 1))
			continue;

		snprintf(path, sizeof(path),
			     "/sys/class/misc/met/%s/%s/mode",
			     mdevice[i].type, mdevice[i].name);

		sys_write(path, buf[mdevice[i].mode], 0);
	}

	return 0;
}

void met_stop(void)
{
	met_node_t *tmp_node;

	tmp_node = met_list;
	while(tmp_node) {
		if (tmp_node->stop) {
			tmp_node->stop();
		}
		tmp_node = tmp_node->next;
	}
}

void met_output_header(FILE *fp_out)
{
	int i;
	met_node_t *tmp_node;

	tmp_node = met_list;
	while(tmp_node) {
		if (tmp_node->output_header) {
			tmp_node->output_header(fp_out);
		}
		tmp_node = tmp_node->next;
	}

	for (i=0; i<mdevice_num; i++) {
		int len;
		FILE *fp;
		char value[16];
		char buf[2048];

		len = snprintf(buf, sizeof(buf),
			            "/sys/class/misc/met/%s/%s/",
			            mdevice[i].type, mdevice[i].name);

		snprintf(buf + len, sizeof(buf) - len, "mode");
		sys_read(buf, value, 16, 0);

		if (value[0] != '0') {
			snprintf(buf + len, sizeof(buf) - len, "header");
			fp = fopen(buf, "r");
			if(fp==NULL) {
				printf("!!ERROR: can not read header from file:%s !!\n", buf);
				continue;
			}

			while ((fgets(buf, sizeof(buf), fp)) != NULL) {
				fprintf(fp_out, "%s", buf);
			}
			fclose(fp);

		}
	}
}

int met_node_exist(char *buf)
{
	struct stat statbuf;
	char path[512] = "/sys/class/misc/met/";

	strncat(path, buf, strlen(buf));
	if (stat(path, &statbuf) == -1) {
		return 0;
	}

	return 1;
}

char *met_get_platform(void)
{
	static char plf_name[16];
	if(has_met_plf) {
		FILE *fp;
		fp = fopen("/sys/class/misc/met/plf", "r");
		if(fp==NULL) {
			return NULL;
		}
		fgets(plf_name, 16, fp);
		fclose(fp);
	}
	else
		strncpy(plf_name, "none", 16);

	return plf_name;
}

int met_is_registered(char *name)
{
	met_node_t *tmp_node;
	tmp_node = met_list;
	while(tmp_node) {
		if (strcmp(tmp_node->name, name)==0) {
			return 1;
		}
		tmp_node = tmp_node->next;
	}
	return 0;
}

int met_probe_devices(void)
{
	FILE *fp;
	char *ptr;
	char buf[64];

//	if (has_met_plf == 0) {
//		return 0;
//	}

	memset(&mdevice, 0, sizeof(mdevice));
	mdevice_num = 0;

	fp = fopen("/sys/class/misc/met/devices", "r");
	if(fp==NULL) {
		return 0;
	}

	while ((fgets(buf, sizeof(buf), fp)) != NULL) {
		struct option opt;
		memset(&opt, 0, sizeof(opt));

		ptr = strrchr(buf, '\n');
		if (ptr != NULL)	*ptr = '\0';

		ptr = strrchr(buf, ':');
		if (ptr == NULL) continue;
		*ptr++ = '\0';
		opt.has_arg = (*ptr) - '0';
		if (opt.has_arg == 1) {
			opt.has_arg = 2;
		}

		ptr = strrchr(buf, '/');
		if (ptr == NULL) continue;
		ptr++;

		if (met_is_registered(ptr))	continue;

		strncpy(mdevice[mdevice_num].arg_name, ptr, 15);
		strncpy(mdevice[mdevice_num].name, ptr, 15);
		if (strcmp(ptr, "cpu")==0)
			strncpy(mdevice[mdevice_num].arg_name, "pmu-cpu-evt", 15);

		ptr--;
		*ptr = 0;
		strncpy(mdevice[mdevice_num].type, buf, 7);

		opt.name = mdevice[mdevice_num].arg_name;
		opt.val = mdevice_num + 'z' + 1;
		met_set_options(&opt, 1);
		if (mdevice_num >= (MAX_MET_DEVICES - 1)) {
			printf("Warning: MET devices reach MAX!\n");
			break;
		}
		mdevice_num++;
	}

	fclose(fp);

	return 0;
}

int get_num(const char *str, unsigned int *value, int null_as_zero)
{
	unsigned int i;
	unsigned int len;

	if (str == NULL) {
		if (null_as_zero) {
			*value = 0;
			return 0;
		} else {
			return -1;
		}
	}
	len = strlen(str);

	if ((len > 2) &&
		((str[0]=='0') &&
		((str[1]=='x') || (str[1]=='X')))) {
		sscanf(str, "%x", value);
		for (i=2; i<len; i++) {
			if (! (((str[i] >= '0') && (str[i] <= '9'))
			   || ((str[i] >= 'a') && (str[i] <= 'f'))
			   || ((str[i] >= 'A') && (str[i] <= 'F')))) {
				return -1;
			}
		}
	} else {
		sscanf(str, "%d", value);
		for (i=0; i<len; i++) {
			if (! ((str[i] >= '0') && (str[i] <= '9'))) {
				return -1;
			}
		}
	}

	return 0;
}

int get_num_forward(char *dc, int *pValue, int len)
{
	int i=0;
	int digit, value = 0;
	while(((*dc)>='0')&&((*dc)<='9'))
	{
		digit = (*dc) - '0';
		value = value * 10 + digit;
		dc++;
		i++;
		if (i >= len)
			break;
	}

	if(i==0) return 0;

	*pValue = value;
	return i;
}

int get_num_backward(char *dc, int *pValue, int len)
{
	int i=0;
	int w=1;
	int digit, value = 0;
	while (((*dc)>='0')&&((*dc)<='9'))
	{
		digit = (*dc) - '0';
		value += digit * w ;
		dc--;
		w *= 10;
		i++;
		if (i >= len)
			break;
	}

	if (i==0) return 0;

	*pValue = value;
	return i;
}

int get_timestamp(char *p, long long *pts)
{
	int ret, value;

	ret = get_num_backward(p - 1, &value, 8);
	if(ret == 0) return 0;
	*pts = (long long)(value) * 1000000LL;
	ret = get_num_forward(p + 1, &value, 6);
	if(ret == 0) return 0;
	*pts += (long long)value;
	return 1;
}

int logbuf_info_add(char *p, logbuf_info_t *info, int type)
{
	char *tmp;
	int len;

	tmp = strrchr(p, '\n');
	if (tmp != NULL)
		*tmp = '\0';
	else
		return -1;
	tmp--;

	len = tmp - p;

	switch (type) {
		case LOGBUF_INFO_RECORDS:
			get_num_backward(tmp, &(info->records), len);
			break;
		case LOGBUF_INFO_BYTES:
			get_num_backward(tmp, &(info->bytes), len);
			break;
		case LOGBUF_INFO_OVERRUN:
			get_num_backward(tmp, &(info->overrun), len);
			break;
		case LOGBUF_INFO_STIME:
			tmp = strrchr(p, '.');
			get_num_backward(tmp - 1, &(info->first_time.sec), len);
			get_num_forward(tmp + 1, &(info->first_time.usec), len);
			break;
		case LOGBUF_INFO_ETIME:
			tmp = strrchr(p, '.');
			get_num_backward(tmp - 1, &(info->last_time.sec), len);
			get_num_forward(tmp + 1, &(info->last_time.usec), len);
			break;
	}
	return 0;
}

void logbuf_info_print(logbuf_info_t info[], int max_cpu, FILE *fp_out)
{
	int i;

	fprintf(fp_out, "met-info [000] 0.0: buffer_data_kb:");
	fprintf(fp_out, " %d", (info[0].bytes + 1023) >> 10);
	for (i=1; i<=max_cpu; i++) {
		fprintf(fp_out, ", %d", (info[i].bytes + 1023) >> 10);
	}
	fprintf(fp_out, "\n");

	fprintf(fp_out, "met-info [000] 0.0: buffer_overrun:");
	fprintf(fp_out, " %d", info[0].overrun);
	for (i=1; i<=max_cpu; i++) {
		fprintf(fp_out, ", %d", info[i].overrun);
	}
	fprintf(fp_out, "\n");

/*
	fprintf(fp_out, "met-info [000] 0.0: buffer_first_time:");
	fprintf(fp_out, " %d.%d", info[0].first_time.sec, info[0].first_time.usec);
	for (i=1; i<=max_cpu; i++) {
		fprintf(fp_out, ", %d.%d", info[i].first_time.sec, info[i].first_time.usec);
	}
	fprintf(fp_out, "\n");
*/
	fprintf(fp_out, "met-info [000] 0.0: buffer_records:");
	fprintf(fp_out, " %d", info[0].records);
	for (i=1; i<=max_cpu; i++) {
		fprintf(fp_out, ", %d", info[i].records);
	}
	fprintf(fp_out, "\n");

	fprintf(fp_out, "met-info [000] 0.0: buffer_last_time:");
	fprintf(fp_out, " %d.%d", info[0].last_time.sec, info[0].last_time.usec);
	for (i=1; i<=max_cpu; i++) {
		fprintf(fp_out, ", %d.%d", info[i].last_time.sec, info[i].last_time.usec);
	}
	fprintf(fp_out, "\n");
}

int sys_read(char *path, char *str, int len, int flag)
{
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		if ((flag&NOMSG_IF_NOEXIST) == 0)
			printf("!!! ERROR: can not open \"%s\" for read !!!\n", path);
		return -1;
	}
	if (fgets(str, len, fp) == NULL) {
		fclose(fp);
		if ((flag&NOMSG_IF_ERROR) == 0)
			printf("!!! ERROR: can not read from file \"%s\" !!!\n", path);
		return -1;
	}
	fclose(fp);
	if ((flag&NOMSG_OPERATION) == 0)
		printf("R: \"%s\" == %s", path, str);
	return 0;
}

int sys_write(char *path, char *str, int flag)
{
	FILE *fp;

	fp = fopen(path, "w");
	if (fp == NULL) {
		if ((flag&NOMSG_IF_NOEXIST) == 0)
			printf("!!! ERROR: can not open \"%s\" for write !!!\n", path);
		return -1;
	}
	if (fputs(str, fp) == EOF) {
		fclose(fp);
		if ((flag&NOMSG_IF_ERROR) == 0)
			printf("!!! ERROR: can not write \"%s\" to file \"%s\" !!!\n", str, path);
		return -1;
	}
	if (fflush(fp) == EOF) {
		fclose(fp);
		if ((flag&NOMSG_IF_ERROR) == 0)
			printf("!!! ERROR: can not write \"%s\" to file \"%s\" !!!\n", str, path);
		return -1;
	}
	fclose(fp);
	if ((flag&NOMSG_OPERATION) == 0)
		printf("W: \"%s\" = %s\n", path, str);
	return 0;
}



