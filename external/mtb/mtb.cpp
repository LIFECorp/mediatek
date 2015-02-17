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



#ifdef MTB_SUPPORT
#define LOG_TAG "MediaTekTraceBridge"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <utils/misc.h>
#include <utils/Timers.h>
#include <cutils/xlog.h>
#include <cutils/trace.h>
#include <cutils/properties.h>
#include "constants.h"
#include "mtb_tracer_factory.h"
#include "mtb.h"


#define MTB_INTERNAL_DEBUG 0
#define THREAD_MAX 20

static uint32_t mtb_trace_type = 0;
static uint64_t mtb_atrace_tags = 0;
static pthread_mutex_t mtb_mutex = PTHREAD_MUTEX_INITIALIZER;

struct event_name_list {
    int32_t index;
    char name[16];
    struct event_name_list *next;
};

struct mtb_event_list {
    int32_t pid;
    struct event_name_list *name_list;
    struct mtb_event_list *next;
};

static struct mtb_event_list* mtb_events = NULL;
static pthread_mutex_t mtb_event_mutex;

static struct mtb_event_list* mtb_event_create(uint32_t number);
static struct event_name_list* mtb_event_search(int32_t pid);
static void mtb_event_add(int32_t pid, char* name);
static void mtb_event_remove(int32_t pid);

struct mtb_event_list* mtb_event_create(uint32_t number) {
    struct mtb_event_list* head = NULL;
    struct mtb_event_list* p = NULL;
    int i;

    for (i=0; i<number; i++) {
        struct mtb_event_list* node = (struct mtb_event_list*)malloc(sizeof(struct mtb_event_list));
        node->pid = -1;
        node->next = NULL;
        node->name_list = NULL;

        if (!head) {
            head = node;
            p = node;
        }
        else {
            p->next = node;
            p = node;
        }
        SXLOGD("event_create, p = %p",p);
    }

    pthread_mutex_init(&mtb_event_mutex, NULL);
    return head;
}

struct event_name_list* mtb_event_search(int32_t pid) {
    struct mtb_event_list* p = NULL;
    struct event_name_list *l = NULL;
    
    if (!mtb_events) return l;
    
    pthread_mutex_lock(&mtb_event_mutex); 
    p = mtb_events;
    while(p != NULL) {
        if (p->pid == pid ) {
            l = p->name_list;
            while (l->next != NULL) {
                l = l->next;
            }
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&mtb_event_mutex);
    return l;
}

void mtb_event_add(int32_t pid, char* name) {
    struct mtb_event_list* p = NULL;

    if (!mtb_events) return;
    
    pthread_mutex_lock(&mtb_event_mutex);
    p = mtb_events;

    struct mtb_event_list *n = NULL;
    while (p != NULL) {
        if (p->pid == pid) {
            n = NULL;
            struct event_name_list *l = p->name_list;
            while (l != NULL) {
                if (l->next == NULL) {
                    struct event_name_list* nl = (struct event_name_list*)malloc(sizeof(struct event_name_list));
                    nl->index = l->index + 1;
                    strcpy(nl->name, name);
                    nl->next = NULL;
                    l->next = nl;
                    break;
                }
                l = l->next;
            }
            break;
        }
        else if (p->pid == -1 && n == NULL) {
            n = p;
        }
        p = p->next;
    }

    if (n != NULL) {
        n->pid = pid;
        n->name_list = (struct event_name_list*)malloc(sizeof(struct event_name_list));
        n->name_list->index = 1;
        strcpy(n->name_list->name, name);
        n->name_list->next = NULL;
    }
    pthread_mutex_unlock(&mtb_event_mutex);
}

void mtb_event_remove(int32_t pid) {
    struct mtb_event_list* p = NULL;

    if (!mtb_events) return;
    
    pthread_mutex_lock(&mtb_event_mutex);
    p = mtb_events;

    while(p != NULL) {
        if (p->pid == pid ) {
            struct event_name_list *n = p->name_list;
            struct event_name_list *pre = n;
            while (n->next) {
                pre = n;
                n = n->next;
            }
            
            if (pre->index == 1 && pre->next == NULL) {
                p->pid = -1;
            }
            else {
                pre->next = NULL;
            }
            free(n);
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&mtb_event_mutex);
}

static inline void mtb_event_name(const char* src, char* name, uint32_t size)
{
    const char *p = src;
    int i = 0;
    while (*p != '\0') {
        if (*p == ',' || i > (size - 2)) 
            break;
        name[i] = *p++;
        i++;
    }
    name[i] = '\0';
}


#if MTB_INTERNAL_DEBUG
inline long long unsigned getNowus() {
    return systemTime(SYSTEM_TIME_MONOTONIC) / 1000ll;
}

class TimeScale{
public:
inline TimeScale(const char* name)
{
    mTime = ::getNowus();
    mName = name;
}

inline ~TimeScale() {
    mTime = ::getNowus() - mTime;
    SXLOGD("Run function %s cost %lld us", mName, mTime);
}

private:
    long long mTime;
    const char* mName;
};

#define TIME_SCALE() TimeScale __scale(__FUNCTION__)
#endif


int mtb_trace_begin(uint64_t tag, const char *name, uint32_t pid, bool direct)
{
#if MTB_INTERNAL_DEBUG
    TIME_SCALE();
#endif
    SXLOGV("Enter function %s, tag = %lld, name = %s", __FUNCTION__, tag, name);
    char event_name[16];
    mtb_event_name(name, event_name, 16);
    if (!direct) {
        mtb_event_add((int32_t)pid, event_name);
    }
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_BEGIN, event_name, pid, NULL);
    return 0;
}

int mtb_trace_end(uint64_t tag, const char* name, uint32_t pid, bool direct)
{ 
#if MTB_INTERNAL_DEBUG
    TIME_SCALE();
#endif
    SXLOGV("Enter function %s, tag = %lld, name = %s", __FUNCTION__, tag, name);
   
    if (direct && name) {
        char event_name[16];
        mtb_event_name(name, event_name, 16);
        tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_END, event_name, pid, NULL);
    }
    else {
        struct event_name_list *p = mtb_event_search((int32_t)pid);
        if (p) {
            tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_END, p->name, pid, NULL);
        }
    }
    mtb_event_remove((int32_t)pid);

    return 0;
}

int mtb_trace_oneshot(uint64_t tag, uint32_t type, const char *name, uint32_t pid)
{
#if MTB_INTERNAL_DEBUG
    TIME_SCALE();
#endif
    SXLOGV("Enter function %s, tag = %lld, type = %d, name = %s", __FUNCTION__, tag, type, name);
    
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_ONESHOT, name, pid, type);
    
    return 0;
}

int mtb_trace_init(uint32_t type, uint64_t tag)
{
    SXLOGD("Enter function %s, type = %d, tag = %lld", __FUNCTION__, type, tag);
    mtb_trace_type = type & MTB_TRACE_TYPE_VALID_MASK;
    mtb_atrace_tags = tag;
    
    tracer_load();
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_INIT, NULL, 0,NULL);
    
    if (mtb_events == NULL) {
        mtb_events = mtb_event_create(THREAD_MAX);
    }

	return 0;
}

/**
 * debug.mtb.tags.tracetype. Can be used as a sysprop change callback.
 */
void mtb_trace_update_types() {
    char mtb_tracetype[PROPERTY_VALUE_MAX];
    char atrace_tags[PROPERTY_VALUE_MAX];
    uint32_t type;
    uint64_t atags;
    
    pthread_mutex_lock(&mtb_mutex);
    property_get("debug.mtb.tracetype", mtb_tracetype, "0");
    property_get("debug.atrace.tags.enableflags", atrace_tags, "0");
    type = (uint32_t)(strtoll(mtb_tracetype, NULL, 0) & MTB_TRACE_TYPE_VALID_MASK);
    atags = strtoll(atrace_tags, NULL, 0);
   
    if ((!type || !atags) || ((mtb_trace_type == type) && (mtb_atrace_tags == atags))) {
        pthread_mutex_unlock(&mtb_mutex);
        return;
    }

    mtb_trace_init(type, atags);
    //atrace_update_tags();
    atrace_enabled_tags = atags;

    pthread_mutex_unlock(&mtb_mutex);
}

uint32_t mtb_trace_get_types() {
    return mtb_trace_type;
}

uint64_t mtb_trace_get_atags() {
    return mtb_atrace_tags;
}

static void mtb_atrace_init() __attribute__((constructor));

static void mtb_atrace_init()
{
    ::android::add_sysprop_change_callback(mtb_trace_update_types, 0);
}

#endif
