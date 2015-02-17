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

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <linux/limits.h>
#include <stdlib.h>

#include "version.h"
#include "interface.h"

#define MAX_CPU_COUNT 32

enum ARM_TYPE {
	CORTEX_A7 = 0xC07,
	CORTEX_A9 = 0xC09,
	CORTEX_A15 = 0xC0F,
	CHIP_UNKNOWN = 0xFFF
};

enum { NONE, START, STOP, EXTRACT };


int has_met = 0;
int has_met_plf = 0;
int need_met_run = 0;
int pause_flag = 0;

static int stop(void)
{
	char buf[16];
	if (met_node_exist("dvfs")) {
		sys_read("/sys/class/misc/met/dvfs", buf, 16, NOMSG_OPERATION);
		if (buf[0] == '1')
			sys_write("/sys/class/misc/met/dvfs", "1", 0);
	}

	if (sys_write("/sys/kernel/debug/tracing/tracing_on", "0", 0) < 0) { // buf
		return -1;
	}

	if (has_met) {
		sys_write("/sys/class/misc/met/run", "0", 0);
	}
//	if (sys_write("/sys/kernel/debug/tracing/set_event", "", 0) < 0) {
//		return -1;
//	}
	return 0;
}


static int start(char *trace_events)
{
	stop();

	if (sys_write("/sys/kernel/debug/tracing/current_tracer", "nop", 0) < 0) {
		return -1;
	}

	if (has_met) {
		if (met_start() < 0) {
			return -1;
		}
	}

	if (sys_write("/sys/kernel/debug/tracing/set_event", trace_events, 0) < 0) {
		return -1;
	}

	if (pause_flag == 0) {
		if (sys_write("/sys/kernel/debug/tracing/tracing_on", "1", 0) < 0) { // buf
			return -1;
		}
	}

	if (has_met) {
		char buf[16];
		if (need_met_run) {
			sys_read("/sys/class/misc/met/sample_rate", buf, 16, 0);
			if (buf[0] == '0') {
				if (sys_write("/sys/class/misc/met/sample_rate", "1000", 0) < 0) {
					return -1;
				}
			}
		}
		else {
			if (sys_write("/sys/class/misc/met/sample_rate", "0", 0) < 0) {
				return -1;
			}
		}

		if (strstr(trace_events, "sched_switch") != NULL)
			sys_write("/sys/class/misc/met/ctrl", "1", NOMSG_IF_NOEXIST);
		else
			sys_write("/sys/class/misc/met/ctrl", "0", NOMSG_IF_NOEXIST);

		if (sys_write("/sys/class/misc/met/run", "1", 0) < 0) { // pmu
			return -1;
		}
	}

	if (sys_write("/sys/kernel/debug/tracing/trace", "", 0) < 0) {
		return -1;
	}

	if (met_node_exist("dvfs")) {
		if (strstr(trace_events, "cpu_frequency") != NULL) {
			if (sys_write("/sys/class/misc/met/dvfs", "1", 0) < 0) {
				return -1;
			}
		}
		else {
			if (sys_write("/sys/class/misc/met/dvfs", "0", 0) < 0) {
				return -1;
			}
		}
	}

	return 0;
}

int get_hardware_info(FILE *fp_out)
{
	FILE *fp;
	char *p, *tmp;
	char buf[4096];

	fp = fopen("/proc/cpuinfo", "r");
	if (fp == NULL) {
		printf("!!! ERROR: can not open \"/proc/cpuinfo\" for read !!!\n");
		return 0;
	}

	while ((p=fgets(buf, 4096, fp)) != NULL) {
		if (strncasecmp(p, "Hardware", 8) == 0) {
			strtok(p, ": \t\n");
			tmp = strtok(NULL, ": \t\n");
			fprintf(fp_out, "met-info [000] 0.0: platform: %s\n", tmp);
		}
	}
	fclose(fp);

	return 1;
}

int output_version(FILE *fp_out)
{
	FILE *fp;
	char *p, *tmp;
	char buf[4096];

	fp = fopen("/proc/version", "r");
	if (fp == NULL) {
		printf("!!! ERROR: can not open \"/proc/version\" for read !!!\n");
		return 0;
	}

	if ((p=fgets(buf, 4096, fp)) != NULL) {
		tmp = strstr(p, "Linux version ");
		if (tmp != 0) {
			p = tmp + strlen("Linux version ");
			tmp = strchr(p, ' ');
			if(tmp!=NULL) {
				*tmp = 0;
				fprintf(fp_out, "met-info [000] 0.0: linux_ver: %s\n", p);
			}
		}
	}
	fclose(fp);

	return 1;
}

static int mask_parsing(char *buf, int *out, int len, int clean_it)
{
	int i, j, max;
	int v1, v2;
	int loop;
	int begin;

	i = 0;
	max = -1;
	loop = 1;
	if (clean_it != 0) {
		memset(out, 0, sizeof(int) * len);
	}

	while (loop != 0) {
		begin = i;
		while ( (buf[i] >= '0') && (buf[i] <= '9') ) {
			i++;
		}

		switch(buf[i]) {
		case '\n':
		case '\0':
			loop = 0;
		case ',':
			sscanf(&(buf[begin]), "%d", &v1);
			if (v1 > len) {
				return -2;
			}
			out[v1] = 1;
			if (v1 > max) {
				max = v1;
			}
			break;
		case '-':
			buf[i] = '_';
			i++;
			while ( (buf[i] >= '0') && (buf[i] <= '9') ) {
				i++;
			}
			sscanf(&(buf[begin]), "%d_%d", &v1, &v2);
			if (v2 > len) {
				return -3;
			}
			for (j=v1; j<=v2; j++) {
				out[j] = 1;
			}
			if (v2 > max) {
				max = v2;
			}
			break;
		default:
			return -4;
		}
		i++;
	}

	return max;
}

static int output_cluster(FILE *fp_out)
{
	int cpu_present[MAX_CPU_COUNT];
	int as_core0[MAX_CPU_COUNT];
	char buf[64];
	int i, max;
	int cpu0_type, not_cpu0_type;
	int a, b, first;

	if (sys_read("/sys/class/misc/met/core_topology", buf, 64, NOMSG_IF_NOEXIST) == 0) {
		fprintf(fp_out, "met-info [000] 0.0: cpu_cluster: %s", buf);
		return 1;
	}

	/*
	 * cpu0_id
	 */
	if (sys_read("/sys/class/misc/met/cpu0_id", buf, 64, NOMSG_IF_NOEXIST) != 0) {
		return 0;
	}
	sscanf(buf, "%x", &cpu0_type);

	if ( (cpu0_type == CORTEX_A7) || (cpu0_type == CORTEX_A15) ) {
		not_cpu0_type = CORTEX_A7 + CORTEX_A15 - cpu0_type;
	} else {
		return 0;
	}

	/*
	 * present
	 */
	if (sys_read("/sys/devices/system/cpu/present", buf, 64, NOMSG_IF_NOEXIST) != 0) {
		return 0;
	}
	max = mask_parsing(buf, cpu_present, MAX_CPU_COUNT, 1);

	/*
	 * as core0 ?
	 */
	if (sys_read("/sys/devices/system/cpu/cpu0/topology/core_siblings_list", buf, 64, NOMSG_IF_NOEXIST) != 0) {
		return 0;
	}
	mask_parsing(buf, as_core0, MAX_CPU_COUNT, 1);

	/*
	 * have both big and LITTLE ?
	 */
	a = 0;
	b = 0;
	for (i=0; i<=max; i++) {
		if (cpu_present[i] != 0) {
			if (as_core0[i] != 0) {
				cpu_present[i] = cpu0_type;
				a = 1;
			} else {
				cpu_present[i] = not_cpu0_type;
				b = 1;
			}
		}
	}

	/*
	 * print header
	 */
	if ( (a != 0) && (b != 0) ) {
		// met-info [000] 0.0: cpu_cluster: big:0,1|LITTLE:2,3
		fprintf(fp_out, "met-info [000] 0.0: cpu_cluster: big:");
		first = 1;
		for (i=0; i<=max; i++) {
			if (cpu_present[i] == CORTEX_A15) {
				if (first == 1) {
					first = 0;
					fprintf(fp_out, "%d", i);
				} else {
					fprintf(fp_out, ",%d", i);
				}
			}
		}
		fprintf(fp_out, "|LITTLE:");
		first = 1;
		for (i=0; i<=max; i++) {
			if (cpu_present[i] == CORTEX_A7) {
				if (first == 1) {
					first = 0;
					fprintf(fp_out, "%d", i);
				} else {
					fprintf(fp_out, ",%d", i);
				}
			}
		}
		fprintf(fp_out, "\n");
	}

	return 1;
}

int output_ftrace_event(FILE *fp_out)
{
	int i, max_cpu, ret;
	FILE *fp;
	char *p, *tmp;
	char path[64];
	char buf[64];
	logbuf_info_t lbf_info[8];

	//Find present CPUs
	sys_read("/sys/devices/system/cpu/present", buf, 16, 0);
	tmp = strrchr(buf, '\n');
	if (tmp != NULL)	*tmp = '\0';
	fprintf(fp_out, "met-info [000] 0.0: present_cpu: %s\n", buf);

	//Get Max CPU
	get_num_backward(buf + strlen(buf) - 1, &max_cpu, 5);

	if (max_cpu >= 8) {
		printf("!!! ERROR: max_cpu=%d is unreasonable!!\n", max_cpu);
		return -1;
	}

	//Find log buffer size
	sys_read("/sys/kernel/debug/tracing/buffer_size_kb", buf, 16, 0);
	tmp = strrchr(buf, '\n');
	if (tmp != NULL)	*tmp = '\0';
	fprintf(fp_out, "met-info [000] 0.0: buffer_size_kb: %s\n", buf);

	//Get and print log buffer information
	ret = 0;
	memset(lbf_info, 0, sizeof(lbf_info));
	for (i=0; i<=max_cpu; i++) {

		snprintf(path, sizeof(path),
		         "/sys/kernel/debug/tracing/per_cpu/cpu%d/stats", i);
		if ((fp = fopen(path, "r"))==NULL)
			continue;

		while ((p=fgets(buf, sizeof(buf), fp)) != NULL) {
			if (strncmp(p, "entries:", 8) == 0) {
				p += 8;
				ret = logbuf_info_add(p, &lbf_info[i], LOGBUF_INFO_RECORDS);
			}
			else if (strncmp(p, "overrun:", 8) == 0) {
				p += 8;
				ret = logbuf_info_add(p, &lbf_info[i], LOGBUF_INFO_OVERRUN);
			}
			else if (strncmp(p, "bytes:", 6) == 0) {
				p += 6;
				ret = logbuf_info_add(p, &lbf_info[i], LOGBUF_INFO_BYTES);
			}
			else if (strncmp(p, "oldest event ts:", 16) == 0) {
				p += 16;
				ret = logbuf_info_add(p, &lbf_info[i], LOGBUF_INFO_STIME);
			}
			else if (strncmp(p, "now ts:", 7) == 0) {
				p += 7;
				ret = logbuf_info_add(p, &lbf_info[i], LOGBUF_INFO_ETIME);
			}
			if (ret < 0) {
				printf("!!! ERROR: unknown buffer information!!!\n");
				fclose(fp);
				return -1;
			}
		}
		fclose(fp);
	}
	logbuf_info_print(lbf_info, max_cpu, fp_out);


	//Find current ftrace events
	fp = fopen("/sys/kernel/debug/tracing/set_event", "r");
	if (fp == NULL) {
		printf("!!! ERROR: can not open \"tracing/set_event\" for read !!!\n");
		return -1;
	}

	if ((p=fgets(buf, sizeof(buf), fp)) != NULL) {
		tmp = strrchr(p, '\n');
		if (tmp != NULL)	*tmp = '\0';
		fprintf(fp_out, "met-info [000] 0.0: ftrace_event: %s", p);
		while ((p=fgets(buf, sizeof(buf), fp)) != NULL) {
			tmp = strrchr(p, '\n');
			if (tmp != NULL)	*tmp = '\0';
			fprintf(fp_out, " %s", p);
		}
		fprintf(fp_out, "\n");
	}
	fclose(fp);

	if (sys_write("/sys/kernel/debug/tracing/set_event", "", 0) < 0) {
		return -1;
	}

	return 1;
}

int output_met_info(FILE *fp_out)
{
	char str[16];

	if (has_met) {
		sys_read("/sys/class/misc/met/ver", str, 16, 0);
		fprintf(fp_out, "met-info [000] 0.0: met_ver: %s", str);
		if(has_met_plf) {
			sys_read("/sys/class/misc/met/plf", str, 16, 0);
			fprintf(fp_out, "met-info [000] 0.0: met_platform: %s", str);
		}
		sys_read("/sys/class/misc/met/sample_rate", str, 16, 0);
		fprintf(fp_out, "met-info [000] 0.0: met_sample_rate_hz: %s", str);
		met_output_header(fp_out);
	}

	return 1;
}

char idlename[] = {"<idle>-0"};

long long start_time;

int add_log_tag(FILE *fp_out, char *log, int len, int flag)
{
	char buf[256];
	char *ptrA = log;
	char *ptrB;
	int nstart, i=0;
	long long ts;

	while ((ptrA != (log + len - 1))&&(*ptrA != '[')) {
		ptrA++;
		buf[i++] = ' ';
	}
	if (ptrA == (log + len - 1))
		return -1;

	ptrB = ptrA;
	while ((ptrB != log)&&(*ptrB != '-'))
        ptrB--;
	if (ptrB == log)
		return -1;
	nstart = (ptrB - log) - 6;

	while ((ptrA != (log + len - 1))&&(*ptrA != ']'))
		buf[i++] = *ptrA++;
	if (ptrA == (log + len - 1))
		return -1;

	while ((ptrA != (log + len - 1))&&(*ptrA != ':'))
		buf[i++] = *ptrA++;
	if (ptrA == (log + len - 1))
		return -1;

	get_timestamp(ptrA - 7, &ts);

	buf[i++] = *ptrA++;
	buf[i++] = *ptrA++;
	buf[i] = 0;

	if (flag==1) {
		strcat(buf, "tracing_mark_write: B|0|_MET_Global_\n");
		start_time = ts;
	}
	else if (flag==2) {
		strcat(buf, "tracing_mark_write: E|_MET_Global_\n");
	}

	for (i=0; i<(int)strlen(idlename); i++) {
		buf[i+nstart] = idlename[i];
	}

	fputs(buf, fp_out);
	if (flag==2) {
		ts -= start_time;
		fprintf(fp_out, "met-info [000] 0.0: met_total_time: %d.%06d\n",
		        (int)(ts/1000000), (int)(ts%1000000));
	}
	return 0;
}

static char * op_get_line(FILE * fp)
{
	char * buf;
	char * new;
	char * cp;
	int c;
	size_t max = 512;

	buf = malloc(max);
	if (buf == NULL)
		return NULL;
	cp = buf;

	while (1) {
		switch (c = getc(fp)) {
			case EOF:
				free(buf);
				return NULL;
				break;

			case '\n':
			case '\0':
				*cp = '\0';
				return buf;
				break;

			default:
				*cp = (char)c;
				cp++;
				if (((size_t)(cp - buf)) == max) {
					new = realloc(buf, max + 128);
					if (new == NULL) {
						free(buf);
						return NULL;
					} else {
						buf = new;
					}
					cp = buf + max;
					max += 128;
				}
				break;
		}
	}
}

static int extract(char *out_file)
{
	int err = 0;
	FILE *fp_trace = NULL;
	FILE *fp_out = NULL;
	char buf[4096];
	unsigned int header;
	char *p, *tmp;
	int i;
	char *line;
	int length;
	int ret;

	fp_trace = fopen("/sys/kernel/debug/tracing/trace", "r");
	if (fp_trace == NULL) {
		printf("!!! ERROR: can not open \"/sys/kernel/debug/tracing/trace\" for read !!!\n");
		err++;
		goto out;
	}

	fp_out = fopen(out_file, "w");
	if (fp_out == NULL) {
		printf("!!! ERROR: can not open \"%s\" for write !!!\n", out_file);
		err++;
		goto out;
	}

	header = 1;
	while ((p=fgets(buf, 4096, fp_trace)) != NULL) {
		if ( (header==1) && (buf[0] != '#') ) {
			header = 0;
			output_version(fp_out);
			get_hardware_info(fp_out);
			output_cluster(fp_out);
			output_ftrace_event(fp_out);
			output_met_info(fp_out);

			add_log_tag(fp_out, buf, strlen(buf), 1);

		}
		fputs(buf, fp_out);
	}

	add_log_tag(fp_out, buf, strlen(buf), 2);

out:

	if (fp_trace != NULL) {
		fclose(fp_trace);
	}
	if (fp_out != NULL) {
		fclose(fp_out);
	}

	if (has_met) {
		sys_write("/sys/class/misc/met/run", "-1", 0); // release dcookie
	}

	return err;
}

static int talk_to_driver(int mode, char *info)
{
	char value[4];
	switch (mode) {
		case START:
			sys_read("/sys/kernel/debug/tracing/tracing_enabled",
			         value, 4, NOMSG_IF_NOEXIST);
			if(value[0]=='0') {
				sys_write("/sys/kernel/debug/tracing/tracing_enabled",
				          "1", NOMSG_OPERATION);
			}
			if (start(info) < 0) {
				return -1;
			}
			break;
		case STOP:
			if (stop() < 0) {
				return -1;
			}
			break;
		case EXTRACT:
			if (extract(info) < 0) {
				return -1;
			}
			break;
	}

	return 0;
}

static int met_module_exist(void)
{
	return met_node_exist("");
}

static int met_plf_exist(void)
{
	return met_node_exist("plf");
}

extern met_node_t met_pmuL2;

struct option long_options[] = {
			{"start", 0, 0, 's'},
			{"stop", 0, 0, 'p'},
			{"pause", 0, 0, 'u'},
			{"extract", 0, 0, 'x'},
			{"output", 1, 0, 'o'},
			{"event-tracer", 1, 0, 'e'},
			{"help", 0, 0, 'h'},
};

int main(int argc, char *argv[])
{
	int err = 0;
	int start = 0;
	int stop = 0;
	int extract = 0;
	char *out_file = "trace.log";
	int has_out = 0;
	char *trace_events = NULL;

	trace_events = malloc(sizeof(char));
	if(trace_events == NULL) {
		printf("!!! ERROR: line must >= 1 !!!\n");
		err++;
		goto out;
	}
	trace_events[0] = '\0';

	has_met = met_module_exist();
	has_met_plf = met_plf_exist();

	met_set_options(long_options, sizeof(long_options)/sizeof(struct option));
	met_register(&met_pmuL2);
	met_probe_devices();
	//met_prn_options();

	/*
	 * argument parsing
	 */
	while (1) {
		int ret;
		int c, option_index = 0;

		c = getopt_long(argc, argv, "o:e:h", pOptions, &option_index);
		if (c == -1) {
			break;
		}

		ret = met_process_arg(c, optarg);
		if (ret<0) {
			err++;
			goto out;
		}
		else if (ret>0) {
			need_met_run++;
			continue;
		}


		switch (c) {
		case 's':
			start = 1;
			break;
		case 'u':
			start = 1;
			pause_flag = 1;
			break;
		case 'p':
			stop = 1;
			break;
		case 'x':
			extract = 1;
			break;
		case 'o':
			out_file = optarg;
			has_out = 1;
			break;
		case 'e': // event-tracer
			/* slow, but we are not that care */
			trace_events = realloc(trace_events, strlen(trace_events) + strlen(optarg) + 2);
			if (trace_events == NULL) {
				printf("!!! ERROR: memory not enough when realloc \"trace_events\" !!!\n");
				err++;
				goto out;
			}
			if (trace_events[0] != '\0') {
				strncat(trace_events, " ", 1);
			}
			strncat(trace_events, optarg, strlen(optarg));
			break;
		case 'h':
			printf("\nMET Backend met-cmd (version: %s)\n", MET_BACKEND_VERSION);
			printf("\nUsage: %s [--start|--pause|--stop|--extract] [OPTION] \n", argv[0]);
			printf("\n");
			printf("Current Platform: %s\n", met_get_platform());
			printf("Functions:\n");
			printf("  --start                               start tracing\n");
			printf("  --pause                               start tracing but pause recording\n");
			printf("  --stop                                stop tracing\n");
			printf("  --extract                             extract tracing data\n");
			printf("\n");
			printf("Options for --start:\n");
			printf("  -e, --event-tracer=EVENT_TRACER	enable ftrace tracers, for example: -e irq -e sched:sched_switch\n");
			met_prn_helper();
			printf("\n");
			printf("Options for --stop: (NONE)\n");
			printf("\n");
			printf("Options for --extract:\n");
			printf("  -o, --output=LOG_FILE_NAME            specify log file name (default to \"trace.log\")\n");
			printf("\n");
			printf("Others options:\n");
			printf("  -h, --help                            display this help\n");
			printf("\n");
			err++;
			goto out;
		case '?':
			printf("!!! ERROR: Unknown argument: %s\n", optarg);
			err++;
			goto out;
		}
	}


	if (start + stop + extract != 1) {
		printf("!!! ERROR: please choose only one mode from [--start | --stop | --extract] !!!\n");
		err++;
		goto out;
	}

	if ((start && has_out) || (stop && has_out)) {
		printf("!!! ERROR: \"-o\" option can not cowork with \"[--start | --stop]\" !!!\n");
		err++;
		goto out;
	}

	if (start == 1) {
		met_context_t met_ctx;

		memset(&met_ctx, 0, sizeof(met_ctx));
		met_ctx.met_exist = has_met;
		met_ctx.met_plf_exist = has_met_plf;
		met_ctx.ftrace_events = trace_events;

		if(met_check_arg(&met_ctx)<0) {
			err++;
			goto out;
		}

		if (talk_to_driver(START, trace_events) < 0) {
			err++;
			goto out;
		}
	} else if (stop == 1) {
		if (trace_events[0] != '\0') {
			printf("!!! ERROR: \"-e\" option can not cowork with \"--stop\" !!!\n");
			err++;
			goto out;
		}
		if (talk_to_driver(STOP, NULL) < 0) {
			err++;
			goto out;
		}
	} else if (extract == 1) {
		if (trace_events[0] != '\0') {
			printf("!!! ERROR: \"-e\" option can not cowork with \"--extract\" !!!\n");
			err++;
			goto out;
		}
		if (talk_to_driver(EXTRACT, out_file) < 0) {
			err++;
			goto out;
		}
	}

out:

	if (trace_events != NULL) {
		free(trace_events);
	}

	return err;
}

