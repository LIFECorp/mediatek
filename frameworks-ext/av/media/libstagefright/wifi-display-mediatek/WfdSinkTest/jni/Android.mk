# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libstagefright_wfd              \
        libutils                        \
		liblog							\
		libcutils						\

LOCAL_SRC_FILES := libwfdsink.cpp

LOCAL_C_INCLUDES:= \
        $(JNI_H_INCLUDE) \
        $(TOP)/frameworks/av/media/libstagefright \
        $(TOP)/frameworks/av/include/media/stagefright/foundation \
        $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/media/libstagefright/wifi-display-mediatek \
        $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/media/libstagefright/wifi-display-mediatek/uibc
        
LOCAL_MODULE_TAGS := optional

LOCAL_CERTIFICATE := platform

LOCAL_MODULE := libwfdsink

LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)