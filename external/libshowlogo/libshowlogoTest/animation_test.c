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



#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>


#include <utils/Log.h>

#include <charging_animation.h>


#define  LOG_ANIM(x...)     dprintf(0, x)


void test_kernel_logo()
{
 
     // set parameter before init
    set_draw_mode(DRAW_ANIM_MODE_FB);    
    anim_init();
    show_kernel_logo();
    anim_deinit();
    
}

void test_boot_logo()
{
 
     // set parameter before init
    set_draw_mode(DRAW_ANIM_MODE_FB);    
    anim_init();
    show_boot_logo();
    anim_deinit();
    
}

void test_charging_animation()
{
    int i = 0;
    for (; i<6; i++ )
    {
        show_battery_capacity(20*i);
        sleep(2);
    }

}

int main(int argc, char *argv[])
{
// DRAW_ANIM_MODE_SURFACE 0
// DRAW_ANIM_MODE_FB  1
    
    printf("[logo_test %s %d]libshowlogo Test ...\n",__FUNCTION__,__LINE__);

    printf("***************     libshowlogo Test       ********************\n");
    printf("*******     Testlibshowlogo  introduce...               *******\n"); 
    printf("It can test boot logo, kernel logo and charging animation using \n framebuffer or surface flinger with different parameters\n"); 
    printf("***************************************************************\n");
    printf(" ---> para 1:boot kernel charging ut\n"); 
    printf("    boot: boot logo\n"); 
    printf("    kernel: kernel logo \n"); 
    printf("    charging: charging animation\n"); 
    printf("    ut: test all\n"); 
        
    printf(" ---> para 2: fb sf \n");
    printf("    fb: framebuffer\n"); 
    printf("    sf: surface flinger\n"); 

    printf("\n ---> Example (Default):libshowlogoTest boot  sf\n");     
    printf("**************************************************************\n");    
    
    
    int draw_mode = DRAW_ANIM_MODE_SURFACE;
    int test_item = 0;

	if(argc > 1){
	    printf("[logo_test %s %d] argv[1]=%s\n",__FUNCTION__,__LINE__,argv[1]); 
        if(!strcmp(argv[1],"boot")){
            test_item = 0;
        }else if(!strcmp(argv[1],"kernel")){
            test_item = 1;
        }else if(!strcmp(argv[1],"charging")){
            test_item = 2;
        }else if(!strcmp(argv[1],"ut")){
            test_item = 3;
        }
        
	}
	    
	if(argc > 2){
	   printf("[logo_test %s %d] argv[2]=%s\n",__FUNCTION__,__LINE__,argv[2]); 
       if(!strcmp(argv[2],"sf"))
	   	{
	   	    draw_mode = DRAW_ANIM_MODE_SURFACE;
	   	}else if(!strcmp(argv[2],"fb")){
	   	    draw_mode = DRAW_ANIM_MODE_FB;
	   	}
	}
	

	
	printf("[logo_test %s %d] argc =%d, draw_mode=%d,test_item=%d\n",__FUNCTION__,__LINE__,argc, draw_mode,test_item); 

     // set parameter before init
    set_draw_mode(draw_mode);    
    anim_init();
    
    printf("[logo_test %s %d]libshowlogo Test start...\n",__FUNCTION__,__LINE__);

    switch(test_item){
        case 0:
            show_boot_logo();
            break;
        case 1:
            show_kernel_logo();
            break;            
        case 2:
            test_charging_animation();
            break;            
        case 3:
            show_boot_logo();
            sleep(2);
            show_kernel_logo();
            sleep(2);
            test_charging_animation();
            break;            
        default:
            show_boot_logo();  
     }       

    
    sleep(5);
    anim_deinit();
    
    return 0;
}


