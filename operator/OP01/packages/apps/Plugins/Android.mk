# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2012. All rights reserved.
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

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

define all-java-files-under-no-FM
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find -L $(1) -name "*.java" -and -not -name ".*" -and -not -name "ProjectStringExt.java") \
 )
endef

LOCAL_MODULE_TAGS := optional
#LOCAL_SRC_FILES := $(call all-subdir-java-files)

ifeq ($(MTK_FM_SUPPORT), yes)
ifeq ($(MTK_FM_RX_SUPPORT), yes)
LOCAL_SRC_FILES := $(call all-java-files-under, src)
else
LOCAL_SRC_FILES := $(call all-java-files-under-no-FM, src)
endif
else
LOCAL_SRC_FILES := $(call all-java-files-under-no-FM, src)
endif
LOCAL_JAVA_LIBRARIES += mediatek-framework telephony-common
LOCAL_STATIC_JAVA_LIBRARIES := com.android.services.telephony.common
LOCAL_PACKAGE_NAME :=  OP01Plugin
LOCAL_CERTIFICATE := shared
LOCAL_PRIVILEGED_MODULE := true
# link your plug-in interface .jar here 
LOCAL_JAVA_LIBRARIES += com.mediatek.gallery3d.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.camera.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.mms.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.qsb.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.contacts.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.dialer.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.phone.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.systemui.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.keyguard.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.settings.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.browser.plugin
LOCAL_JAVA_LIBRARIES += com.mediatek.downloadmanager.plugin
LOCAL_JAVA_LIBRARIES += mediatek-framework telephony-common
LOCAL_JAVA_LIBRARIES += com.mediatek.music.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.deskclock.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.launcher2.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.launcher3.ext
ifeq ($(MTK_FM_SUPPORT), yes)
ifeq ($(MTK_FM_RX_SUPPORT), yes)
LOCAL_JAVA_LIBRARIES += com.mediatek.FMRadio.ext
endif
endif

LOCAL_JAVA_LIBRARIES += com.mediatek.providers.settings.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.soundrecorder.ext
LOCAL_JAVA_LIBRARIES += com.mediatek.incallui.ext

LOCAL_PROGUARD_FLAG_FILES := proguard.flags
include $(BUILD_PACKAGE)

# This finds and builds the test apk as well, so a single make does both.
include $(call all-makefiles-under,$(LOCAL_PATH))
