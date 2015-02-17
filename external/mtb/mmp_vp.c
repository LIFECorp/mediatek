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



#ifdef MTK_MMPROFILE_SUPPORT
#define LOG_TAG "MediaTekTraceBridge"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <cutils/xlog.h>
#include <linux/mmprofile.h>
#include "constants.h"
#include "mmp_vp.h"
#include "mmp_hash.h"

MMP_Event MMP_VP_ROOT;
MMP_Event MMP_VP_CONTROL;
MMP_Event MMP_VP_TIMEDEVENT;
MMP_Event MMP_VP_MESSAGE;
MMP_Event MMP_VP_ADATA;
MMP_Event MMP_VP_VDATA;
MMP_Event MMP_VP_SPECIAL;
MMP_Event MMP_VP_AUDIOTRACK;

static struct mmp_event_info mmp_vp_hashTable[MMP_EVENT_HASHTABLE_LENGTH];


int32_t  mmp_vp_init(tracer_handle_t self)
{
    SXLOGD("Enter function %s", __FUNCTION__);
    MMP_VP_ROOT = MMProfileRegisterEvent(MMP_RootEvent, "playback");
    MMProfileEnableEvent(MMP_VP_ROOT, 1); 
    MMP_VP_CONTROL = MMProfileRegisterEvent(MMP_VP_ROOT, "control");
    MMProfileEnableEvent(MMP_VP_CONTROL, 1);

    MMP_VP_TIMEDEVENT = MMProfileRegisterEvent(MMP_VP_ROOT, "timedevent");
    MMProfileEnableEvent(MMP_VP_TIMEDEVENT, 1); 

    MMP_VP_MESSAGE = MMProfileRegisterEvent(MMP_VP_ROOT, "message");
    MMProfileEnableEvent(MMP_VP_MESSAGE,1); 

    MMP_VP_ADATA = MMProfileRegisterEvent(MMP_VP_ROOT, "audiodata");
    MMProfileEnableEvent(MMP_VP_ADATA, 1); 
    
    MMP_VP_VDATA = MMProfileRegisterEvent(MMP_VP_ROOT, "videodata");
    MMProfileEnableEvent(MMP_VP_VDATA, 1); 
    
    MMP_VP_SPECIAL = MMProfileRegisterEvent(MMP_VP_ROOT, "special");
    MMProfileEnableEvent(MMP_VP_SPECIAL, 1);
    MMP_VP_AUDIOTRACK = MMProfileRegisterEvent(MMP_VP_ADATA, "audiotrack");
    MMProfileEnableEvent(MMP_VP_AUDIOTRACK, 0);

    return 0;

}

int32_t mmp_vp_begin(tracer_handle_t self, const char *name, uint32_t pid) {
    MMP_Event event;
    struct mmp_event_info *info;
    
    if ((info = mmp_hash_search(name, mmp_vp_hashTable)) == NULL) {
        event = MMProfileRegisterEvent(MMP_VP_CONTROL, name);
        MMProfileEnableEvent(event, 1); 
        mmp_hash_insert(name, event, mmp_vp_hashTable);
    }
    else {
        event = info->id;
    }
    MMProfileLogEx(event, MMProfileFlagStart, pid, 0);
    return 0;
}

int32_t mmp_vp_end(tracer_handle_t self, const char*name, uint32_t pid) {
    MMP_Event event;
    struct mmp_event_info *info;
    
    if ((info = mmp_hash_search(name, mmp_vp_hashTable)) != NULL) {
        MMProfileLogEx(info->id, MMProfileFlagEnd, pid, 0);
    }
    return 0;
}

int32_t mmp_vp_oneshot(tracer_handle_t self, uint32_t type, const char *name, uint32_t pid) {

    MMP_Event event;
    SXLOGV("Enter function %s, type = %d, name = %s", __FUNCTION__, type, name);
    
    switch(type) {
        case ATRACE_ONESHOT_EVENT:
        {    
            struct mmp_event_info *info;
            if ((info = mmp_hash_search(name, mmp_vp_hashTable)) == NULL) {
                event = MMProfileRegisterEvent(MMP_VP_TIMEDEVENT, name);
                MMProfileEnableEvent(event, 1); 
                mmp_hash_insert(name, event,mmp_vp_hashTable);
            }
            else {
                event = info->id;
            }
            break;
        }
        case ATRACE_ONESHOT_MESSAGE:
            event = MMP_VP_MESSAGE;
            break;
        case ATRACE_ONESHOT_ADATA:
            event = MMP_VP_ADATA;
            break;
        case ATRACE_ONESHOT_VDATA:
            event = MMP_VP_VDATA;
            break;
        case ATRACE_ONESHOT_SPECIAL:
        {
            const char *p = name;
            char event_name[32];
            int i = 0;
            while (*p != '\0') {
                if (*p == ',' || i > 14) 
                    break;
                event_name[i] = *p++;
                i++;
            }
            event_name[i] = '\0';
            
            struct mmp_event_info *info;
            if ((info = mmp_hash_search(event_name, mmp_vp_hashTable)) == NULL) {
                event = MMProfileRegisterEvent(MMP_VP_SPECIAL, event_name);
                MMProfileEnableEvent(event, 1); 
                mmp_hash_insert(event_name, event,mmp_vp_hashTable);
            }
            else {
                event = info->id;
            }
            break;
        }
		case ATRACE_ONESHOT_AUDIOTRACK:
		{
            const char *p = name;
            char event_name[32];
            int i = 0;
			char disp[32];
            while (*p != '\0') {
                if (*p == ',' || i > 14) 
                    break;
                event_name[i] = *p++;
                i++;
            }
            event_name[i] = '\0';
            
            struct mmp_event_info *info;
            if ((info = mmp_hash_search(event_name, mmp_vp_hashTable)) == NULL) {
                event = MMProfileRegisterEvent(MMP_VP_AUDIOTRACK, event_name);
                MMProfileEnableEvent(event, 1); 
                mmp_hash_insert(event_name, event,mmp_vp_hashTable);
            }
            else {
                event = info->id;
            }

//			MMProfileLogEx(event, MMProfileFlagPulse, DispStru.data[0], DispStru.data[1]);
			if(strlen(name) - strlen(event_name) > 0 && strlen(name) - strlen(event_name) < 32)
			{
				memcpy(disp, name+strlen(event_name)+1, strlen(name) - strlen(event_name) - 1);
				disp[strlen(name) - strlen(event_name) - 1] = '\0';
			}
			else if(strlen(name) - strlen(event_name) >= 32)
			{
				memcpy(disp, name+strlen(event_name)+1, 31);
				disp[31] = '\0';
			}
			else
				disp[0] = '\0';
			MMProfileLogMetaString(event, MMProfileFlagPulse, disp); 
			return 0;
		}
        default:
            event = MMP_VP_CONTROL;
            break;
    }

    MMProfileLogEx(event, MMProfileFlagPulse, (unsigned int)(*self)->tag, 0);
    return 0;
}
#endif











