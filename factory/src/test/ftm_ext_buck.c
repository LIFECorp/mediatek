/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_EXT_BUCK

#define TAG                 "[EXT_BUCK] "

/* IOCTO */
#define CH_EXT_BUCK             _IOW('k', 20, int)

static item_t ext_buck_items[] = {
#if 1
    //auto test
    item(-1, NULL),
#else
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
#endif
};

extern sp_ata_data return_data;

struct ext_buckFTM {    
    char   info[1024];
    bool   exit_thd;    
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;

    text_t title;
    text_t text;
    text_t left_btn;
    text_t center_btn;
    text_t right_btn;
};

#define mod_to_ext_buckFTM(p)     (struct ext_buckFTM*)((char*)(p) + sizeof(struct ftm_module))

int is_hw_ext_buck_exist(void)
{
    int fd = -1;
    int ret = 0;
    int adc_in_data[2] = {1,1}; 
    
    fd = open("/dev/pmic_ftm",O_RDONLY, 0);
    if (fd == -1) {
        LOGD(TAG "Can't open /dev/pmic_ftm\n");
    }
    
    ret = ioctl(fd, CH_EXT_BUCK, adc_in_data);
    close(fd);

    LOGD(TAG "[is_hw_ext_buck_exist] result=%d\n", adc_in_data[0]); 
           
    return adc_in_data[0];
}

static void ext_buck_update_info(struct ext_buckFTM *extbk, char *info)
{
    char *ptr;
    int temp = 0;

    /* preare text view info */
    ptr  = info;
    ptr += sprintf(ptr, "%s : %s \n", uistr_info_title_ext_buck_chip, (is_hw_ext_buck_exist()) ? uistr_info_title_ext_buck_connect : uistr_info_title_ext_buck_no_connect);

    return;
}

static int ext_buck_key_handler(int key, void *priv) 
{
    int handled = 0, exit = 0;
    struct ext_buckFTM *extbk = (struct ext_buckFTM *)priv;
    struct textview *tv = &extbk->tv;
    struct ftm_module *fm = extbk->mod;
    
    switch (key) {
    case UI_KEY_RIGHT:
        exit = 1;
        break;
    case UI_KEY_LEFT:        
        fm->test_result = FTM_TEST_FAIL;
        exit = 1;
        break;
    case UI_KEY_CENTER:
        fm->test_result = FTM_TEST_PASS;
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }
    if (exit) {
        LOGD(TAG "%s: Exit thead\n", __FUNCTION__);
        extbk->exit_thd = true;
        tv->exit(tv);        
    }
    return handled;
}

static void *ext_buck_update_thread(void *priv)
{
    struct ext_buckFTM *extbk = (struct ext_buckFTM *)priv;
    struct textview *tv = &extbk->tv;    
    int count = 1, chkcnt = 10;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(100000);
        chkcnt--;

        if (extbk->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        ext_buck_update_info(extbk, extbk->info);
        tv->redraw(tv);
        chkcnt = 10;
    }
    
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
    return NULL;
}

static void *ext_buck_update_iv_thread(void *priv)
{
    struct ext_buckFTM *extbk = (struct ext_buckFTM *)priv;
    struct itemview *iv = extbk->iv;
    int count = 1, chkcnt = 10;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(100000);
        chkcnt--;

        if (extbk->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        ext_buck_update_info(extbk, extbk->info);
        iv->redraw(iv);
        chkcnt = 10;
    }
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
    return NULL;
}

int ext_buck_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct ext_buckFTM *extbk = (struct ext_buckFTM *)priv;
    struct textview *tv;    
    struct itemview *iv;
    //auto test
    int temp=0;
    int temp_v_bat=0;
    int temp_chr_cuttent=0;
    int temp_v_chr=0;
    int temp_v_bat_temp=0;
    unsigned long i=0;
    unsigned long i_loop_time=100;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&extbk->title, param->name, COLOR_YELLOW);
    init_text(&extbk->text, &extbk->info[0], COLOR_YELLOW);
    init_text(&extbk->left_btn, "Fail", COLOR_YELLOW);
    init_text(&extbk->center_btn, "Pass", COLOR_YELLOW);
    init_text(&extbk->right_btn, "Back", COLOR_YELLOW);

    ext_buck_update_info(extbk, extbk->info);

    /* show text view */
    extbk->exit_thd = false;     

    if (!extbk->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        extbk->iv = iv;
    }
    
    iv = extbk->iv;
    iv->set_title(iv, &extbk->title);
    iv->set_items(iv, ext_buck_items, 0);
    iv->set_text(iv, &extbk->text);
    iv->start_menu(iv,0);
    
    iv->redraw(iv);

    //auto test - check external buck
    if(is_hw_ext_buck_exist())
    {
        LOGD(TAG "[ITEM_EXT_BUCK] is_hw_ext_buck_exist : YES\n");
        extbk->mod->test_result = FTM_TEST_PASS;
        LOGD(TAG "[ITEM_EXT_BUCK] Final : PASS\n");
        return 0;
    }
    else
    {
        LOGD(TAG "[ITEM_EXT_BUCK] is_hw_ext_buck_exist : NO\n");
        extbk->mod->test_result = FTM_TEST_FAIL;
        return 0;
    }

    return 0;
}

int ext_buck_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct ext_buckFTM *extbk;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_EXT_BUCK, sizeof(struct ext_buckFTM));
    extbk = mod_to_ext_buckFTM(mod);

    /* init */
    extbk->mod = mod;    

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, ext_buck_entry, (void*)extbk);

    return ret;
}

#endif

