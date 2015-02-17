# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


PRODUCT_PACKAGES += \
    CMCCClient \
    MobilePortal \
    MyFavorite \
    OP01Plugin \
    Stopwatch \
    TaskManager \
    ConfigureCheck \
    ConfigureCheck2 \
    BackupAndRestore \
    PhoneVTDialer \
    DataDialog \
    TXTViewer \
    MobileVideoConfig \
    

#    SogouInput \

ifneq ($(OP01_CTS_COMPATIBLE),yes)

ifneq ($(MTK_CHIPTEST_INT),yes)
PRODUCT_PACKAGES += \
    CMCCMobileMusic \
    CMCCNavigation \
    Fetion \
    MobileMarket \
    MMPlugCartoon \
    MMPlugComic \
    MMPlugMusic \
    MMPlugVideo \
    MMPlugRead \
    MMPlugKvSafeCenter \
    MobileReader \
    MobileVideo \
    WLANClient \
    SinaWeibo \
    Lingxi \
    Xunfei \
    CMCCQQBrowser \
    Taobao \
    SohuNews \

ifneq ($(strip $(MTK_LCA_ROM_OPTIMIZE)), yes)    
PRODUCT_PACKAGES += \
    GameHall \
    PIMClient \
    UCBrowser \
    Amazon \
    OperaBrowser \
    PandaReader \
    Tmall \
    QQMusic \
    QQLive \
    QQBuy \
    BaiduSearch \

endif 
endif
endif

# Add resource files and configuration files
PRODUCT_PACKAGES += \
    bootanimation.zip \
    shutanimation.zip \
    bootaudio.mp3 \
    shutaudio.mp3 \
    VersionControl.xml \
    reminder.xml \
    tree.xml \
    DmApnInfo.xml \
    config.xml \
    MTK_MDM_APP_CONF_TEST_REMINDER \
    MTK_MDM_APP_CONF_TEST_TREE \
    MTK_MDM_APP_CONF_TEST_DMAPNINFO \
    MTK_MDM_APP_CONF_TEST_CONFIG \
    MTK_MDM_APP_CONF_PRODUCTIVE_REMINDER \
    MTK_MDM_APP_CONF_PRODUCTIVE_TREE \
    MTK_MDM_APP_CONF_PRODUCTIVE_DMAPNINFO \
    MTK_MDM_APP_CONF_PRODUCTIVE_CONFIG \
    MTK_SMSREG_APP_CONF_TEST \
    MTK_SMSREG_APP_CONF_PRODUCTIVE \
