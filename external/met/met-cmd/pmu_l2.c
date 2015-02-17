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
#include "interface.h"
#include "pmu_name.h"

static struct option pmuL2_options[] = {
	{"pmu-l2-evt", 1, 0, '2'},
	{"pmu-l2-toggle", 0, 0, 't'},
};

struct pmu_struct {
	unsigned int event;
	unsigned int freq;
};

#define MAX_L2_PMU 2
static int toggle_mode = 0;
static unsigned int l2_idx = 0;
static struct pmu_struct l2_pmu[MAX_L2_PMU];

static int assign_counter(unsigned int event, unsigned int freq)
{
	unsigned int i;
	struct pmu_desc *p;

	if (l2_idx == MAX_L2_PMU) {
		printf("!!! ERROR: hardware L2_PMU not enough (up to %d) !!!\n", MAX_L2_PMU);
		return -1;
	}

	p = pl310_pmu_desc;
	for (i=0; i<PL310_PMU_DESC_COUNT; i++) {
		if (p->event == event) {
			break;
		}
		p++;
	}
	if (i == PL310_PMU_DESC_COUNT) {
		printf("!!! ERROR: 0x%x is not a valid L2_PMU event !!!\n", event);
		return -1;
	}

	for (i=0; i<l2_idx; i++) {
		if (l2_pmu[i].event == event) {
			printf("!!! ERROR: duplicated L2_PMU event: 0x%x !!!\n", event);
			return -1;
		}
	}
	l2_pmu[i].event = event;
	l2_pmu[i].freq = freq;
	l2_idx++;

	return 0;
}

static int compare(const void *s1, const void *s2)
{
	struct pmu_struct *p1 = (struct pmu_struct *) s1;
	struct pmu_struct *p2 = (struct pmu_struct *) s2;
	return p1->event - p2->event;
}

static int pmuL2_init(int met_exist, int met_plf_exist)
{
	if (! met_node_exist("pmu/l2")) {
		return -1;
	}

	memset(l2_pmu, 0, sizeof(l2_pmu));
	return 0;
}

static int pmuL2_process_arg(int option, char *arg)
{
	unsigned int event, freq;
	int err1, err2;
	/*
	 * argument parsing
	 */
	switch (option) {
	case '2': // pmu-l2-evt
		// getopt_long guarants optarg is not NULL
		err1 = get_num(strtok(arg, ":"), &event, 0);
		err2 = get_num(strtok(NULL, ":"), &freq, 1);
		if (err1 || err2) {
			printf("!!! ERROR: invalid L2_PMU argument: \"%s\" !!!\n", arg);
			return -1;
		}
		if(assign_counter(event, freq)<0) {
			return -1;
		}
		return 1;
	case 't': // pmu-l2-toggle
		toggle_mode = 1;
		return 1;
	}

	return 0;
}

static int pmuL2_check_arg(met_context_t *ctx)
{
	if ((toggle_mode==1) && (l2_idx!=0)) {
		printf("!!! ERROR: \"pmu-l2-toggle\" can not work with \"pmu-l2-evt\" options !!!\n");
		return -1;
	}

	qsort(l2_pmu, l2_idx, sizeof(struct pmu_struct), compare);

	if ((toggle_mode==1) || (l2_idx!=0)){
		sys_write("/sys/class/misc/met/pmu/l2/mode", "1", 0);
	} else {
		sys_write("/sys/class/misc/met/pmu/l2/mode", "0", 0);
	}

	return 0;
}

static void pmuL2_prn_helper(void)
{
	printf("  --pmu-l2-evt=EVENT                    select L2-PMU events. you can enable at most 2 L2 events\n");
	printf("  --pmu-l2-toggle                       use toggling mode (half sampling mode) to monitor 4 L2 events,\n");
	printf("                                        which are cache read/write, hit/request\n");
	printf("                                        note: \"--pmu-l2-evt\" option can not be used in toggling mode\n");
}

static int pmuL2_start(void)
{
	unsigned int i;
	char path[256], buf[256];

	for (i=0; i<l2_idx; i++) {
		snprintf(path, sizeof(path), "/sys/class/misc/met/pmu/l2/%d/mode", i);
		sys_write(path, SAMPLING, 0);

		snprintf(path, sizeof(path), "/sys/class/misc/met/pmu/l2/%d/event", i);
		snprintf(buf, sizeof(buf), "0x%x", l2_pmu[i].event);
		sys_write(path, buf, 0);
	}

	if (toggle_mode == 0) {
		sys_write("/sys/class/misc/met/pmu/l2/toggle_mode", "0", 0);
	} else {
		sys_write("/sys/class/misc/met/pmu/l2/toggle_mode", "1", 0);
	}
	return 0;
}

static void pmuL2_stop(void)
{
	return;
}

static void pmuL2_output_header(FILE *fp_out)
{
	unsigned int i;
	char path[4096], value[16];
	unsigned int tmp, first;

	sys_read("/sys/class/misc/met/pmu/l2/toggle_mode", value, 16, 0);
	if (value[0] == '1') {
		fputs("# mp_2pr: timestamp, l2_pmu_value1, ...\n", fp_out);
		fputs("met-info [000] 0.0: met_2pr_header: 0x2, 0x3\n", fp_out);
		fputs("# mp_2pw: timestamp, l2_pmu_value1, ...\n", fp_out);
		fputs("met-info [000] 0.0: met_2pw_header: 0x4, 0x5\n", fp_out);
	} else {
		first = 1;
		for (i=0; i<MAX_L2_PMU; i++) {
			snprintf(path, sizeof(path), "/sys/class/misc/met/pmu/l2/%d/mode", i);
			sys_read(path, value, 16, 0);
			switch (value[0]) {
			case '2': // polling
				snprintf(path, sizeof(path), "/sys/class/misc/met/pmu/l2/%d/event", i);
				sys_read(path, value, 16, 0);
				sscanf(value, "0x%x", &tmp);
				if (first != 0) {
					fputs("# mp_2p: timestamp, l2_pmu_value1, ...\n", fp_out);
					fputs("met-info [000] 0.0: met_2p_header: ", fp_out);
					fprintf(fp_out, "0x%x", tmp);
					first = 0;
				} else {
					fprintf(fp_out, ",0x%x", tmp);
				}
				break;
			}
		}
		if (first == 0) {
			fprintf(fp_out, "\n");
		}
	}

	return;
}


met_node_t met_pmuL2 = {
	.name = "l2",
	.options = pmuL2_options,
	.option_size = (sizeof(pmuL2_options)/sizeof(struct option)),
	.init = pmuL2_init,
	.process_arg = pmuL2_process_arg,
	.check_arg = pmuL2_check_arg,
	.prn_helper = pmuL2_prn_helper,
	.start = pmuL2_start,
	.stop = pmuL2_stop,
	.output_header = pmuL2_output_header
};
