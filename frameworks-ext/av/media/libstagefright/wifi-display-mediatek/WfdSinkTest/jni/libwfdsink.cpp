/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "WFD_SINK_JNI"

#include <jni.h>
#include <cutils/jstring.h>
#include <utils/Log.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "AString.h"
#include "ABase.h"
#include "AHandler.h"
#include "ALooper.h"
#include "AMessage.h"
#include "ANetworkSession.h"

#include "sink/WifiDisplaySink.h"
#include "source/WifiDisplaySource.h"
#include "ParsedMessage.h"
#include <gui/SurfaceComposerClient.h>

#include <gui/ISurfaceComposer.h>
#include <ui/DisplayInfo.h>



extern "C" unsigned char* _jniTest();
extern "C" bool _startWdfSink(jstring srcaddr, jint port);
extern "C" void _stopWdfSink();
extern "C" void _sendGenericTouchEvent(jstring eventDesc);
extern "C" void _sendGenericKeyEvent(jstring eventDesc);
extern "C" void _sendGenericZoomEvent(jstring eventDesc);
extern "C" void _sendGenericScaleEvent(jstring eventDesc);

namespace android {
sp<WifiDisplaySink> sink;
sp<ANetworkSession> session;
int32_t connectToPort;
sp<SurfaceComposerClient> composerClient; 

void* SinkThread(void *arg) {
    ALOGI("startWdfSink [+]");
    AString connectToHost = AString((char*)arg);

    session = new ANetworkSession;
    session->start();

    sp<ALooper> looper = new ALooper;

	composerClient = new SurfaceComposerClient;
    CHECK_EQ(composerClient->initCheck(), (status_t)OK);

    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);

	ssize_t displayWidth = 540;//info.w;
	ssize_t displayHeight = 960;//info.h;

    ALOGV("display is %d x %d\n", displayWidth, displayHeight);

    sp<SurfaceControl> control =
        composerClient->createSurface(
                String8("A Surface"),
                displayWidth,
                displayHeight,
                PIXEL_FORMAT_RGB_565,
                0);


    CHECK(control != NULL);    
	CHECK(control->isValid());

    SurfaceComposerClient::openGlobalTransaction();
    CHECK_EQ(control->setLayer(INT_MAX), (status_t)OK);  
	CHECK_EQ(control->show(), (status_t)OK);
    SurfaceComposerClient::closeGlobalTransaction();

    sp<Surface> surface = control->getSurface();
    CHECK(surface != NULL);



  //  sp<ALooper> looper = new ALooper;

    sink = new WifiDisplaySink(
            0 /* flags */,
            session,
            surface->getIGraphicBufferProducer());

  //  sink = new WifiDisplaySink(session);

    looper->registerHandler(sink);

    if (connectToPort >= 0) {
        ALOGI("sink->start %s, port %d",
              connectToHost.c_str(), connectToPort);
        sink->start(connectToHost.c_str(), connectToPort);
    }

    looper->start(true);
    ALOGI("startWdfSink [-]");
    return NULL;
}

/*****************************************************
*   Wifi Sink JNI interface
*****************************************************/
static jstring jniTest(JNIEnv* env, jobject thiz) {
    return env->NewStringUTF("startWdfSink from JNI !");
}

static jboolean startWfdSink(JNIEnv* env, jobject thiz, jstring srcaddr, jint port) {
    ALOGI("startWdfSink [+]");

    pthread_t tid;
    int i = 0;
    int err;
    const char *c_addr = NULL;

    if (srcaddr == NULL) goto err;

    c_addr = env->GetStringUTFChars(srcaddr, NULL);
    if(c_addr == NULL) goto err;

    connectToPort = port;

    err = pthread_create(&tid, NULL, &SinkThread, (void *)c_addr);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        goto err;
    } else {
        printf("\n Thread created successfully\n");
    }

    ALOGI("startWdfSink [-]");
    return true;
err:
    ALOGI("startWdfSink with error[-]");
    return false;
}

static void stopWfdSink(JNIEnv* env, jobject thiz) {
    ALOGI("stopWdfSink [+]");	
	composerClient->dispose();
    sink->stop();
    session->stop();
    sink = NULL;
    session = NULL;
    ALOGI("stopWdfSink [-]");
}

/*
static void sendGenericTouchEvent(JNIEnv* env, jobject thiz, jstring eventDesc) {
    const char *c_eventDesc = NULL;
    c_eventDesc = env->GetStringUTFChars(eventDesc, NULL);
    ALOGD("sendGenericTouchEvent %s", c_eventDesc);
    sink->sendUIBCGenericTouchEvent(c_eventDesc);
}

static void sendGenericKeyEvent(JNIEnv* env, jobject thiz, jstring eventDesc) {
    const char *c_eventDesc = NULL;
    c_eventDesc = env->GetStringUTFChars(eventDesc, NULL);
    ALOGD("sendGenericKeyEvent %s", c_eventDesc);
    sink->sendGenericKeyEvent(c_eventDesc);
}

static void sendGenericZoomEvent(JNIEnv* env, jobject thiz, jstring eventDesc) {
    const char *c_eventDesc = NULL;
    c_eventDesc = env->GetStringUTFChars(eventDesc, NULL);
    ALOGD("sendGenericZoomEvent %s", c_eventDesc);
    sink->sendUIBCGenericZoomEvent(c_eventDesc);
}

static void sendGenericScaleEvent(JNIEnv* env, jobject thiz, jstring eventDesc) {
    const char *c_eventDesc = NULL;
    c_eventDesc = env->GetStringUTFChars(eventDesc, NULL);
    ALOGD("sendGenericScaleEvent %s", c_eventDesc);
    sink->sendGenericScaleEvent(c_eventDesc);
}

static void sendGenericRotateEvent(JNIEnv* env, jobject thiz, jstring eventDesc) {
    const char *c_eventDesc = NULL;
    c_eventDesc = env->GetStringUTFChars(eventDesc, NULL);
    ALOGD("sendGenericRotateEvent %s", c_eventDesc);
    sink->sendGenericRotateEvent(c_eventDesc);
}

static void sendGenericVendorEvent(JNIEnv* env, jobject thiz, jstring eventDesc) {
    const char *c_eventDesc = NULL;
    c_eventDesc = env->GetStringUTFChars(eventDesc, NULL);
    ALOGD("sendGenericVendorEvent %s", c_eventDesc);
    sink->sendGenericVendorEvent(c_eventDesc);
}
*/
}

/*****************************************************
*   JNI load and register
*****************************************************/

static JNINativeMethod sMethods[] = {
    {"jniTest", "()Ljava/lang/String;", (void *)android::jniTest},
    {"startWfdSink", "(Ljava/lang/String;I)Z", (void *)android::startWfdSink},
    {"stopWfdSink", "()V", (void *)android::stopWfdSink},
/*
    {"sendGenericTouchEvent", "(Ljava/lang/String;)V", (void *)android::sendGenericTouchEvent},
    {"sendGenericKeyEvent", "(Ljava/lang/String;)V", (void *)android::sendGenericKeyEvent},
    {"sendGenericZoomEvent", "(Ljava/lang/String;)V", (void *)android::sendGenericZoomEvent},
    {"sendGenericScaleEvent", "(Ljava/lang/String;)V", (void *)android::sendGenericScaleEvent},
    {"sendGenericRotateEvent", "(Ljava/lang/String;)V", (void *)android::sendGenericRotateEvent},
    {"sendGenericVendorEvent", "(Ljava/lang/String;)V", (void *)android::sendGenericVendorEvent},
*/
};


static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* methods, int numMethods) {
    jclass clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'\n", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'\n", className);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int registerNatives(JNIEnv* env) {
    if (!registerNativeMethods(env, "com/mediatek/wfdsinktest/WfdSinkActivity",
                               sMethods, sizeof(sMethods) / sizeof(sMethods[0]))) {
        ALOGE("registerNativeMethods failed");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


/*
 * When library is loaded by Java by invoking "loadLibrary()".
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    ALOGI("JNI_OnLoad [+]");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (!registerNatives(env)) {
        ALOGE("ERROR: failed to register natives\n");
        goto bail;
    }
    result = JNI_VERSION_1_4;

bail:
    ALOGI("JNI_OnLoad [-]");
    return result;;
}
