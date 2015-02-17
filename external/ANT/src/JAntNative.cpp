/*
 * ANT Stack
 *
 * Copyright 2009 Dynastream Innovations
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*******************************************************************************\
*
*   FILE NAME:     JAntNative.cpp
*
*   BRIEF:
*      This file provides the implementation of the native interface functions for ANT
*
*
\*******************************************************************************/

#include "android_runtime/AndroidRuntime.h"
#include "jni.h"
#include "nativehelper/JNIHelp.h"

static JNIEnv *g_jEnv = NULL;
static JavaVM *g_jVM = NULL;
static jclass g_sJClazz;
static jmethodID g_sMethodId_nativeCb_AntRxMessage;
static jmethodID g_sMethodId_nativeCb_AntStateChange;

extern "C"
{
   #include "ant_native.h"
   #include "ant_log.h"
   #undef LOG_TAG
   #define LOG_TAG "JAntNative"

   void nativeJAnt_RxCallback(ANT_U8 ucLen, ANT_U8* pucData);
   void nativeJAnt_StateCallback(ANTRadioEnabledStatus uiNewState);
}

static jint nativeJAnt_Create(JNIEnv *env, jobject obj)
{
   ANTStatus antStatus = ANT_STATUS_FAILED;
   (void)env; //unused warning
   (void)obj; //unused warning

   ANT_FUNC_START();

   antStatus = ant_init();
   if (antStatus)
   {
      ANT_DEBUG_D("failed to init ANT stack");
      goto CLEANUP;
   }

   antStatus = set_ant_rx_callback(nativeJAnt_RxCallback);
   if (antStatus)
   {
      ANT_DEBUG_D("failed to set ANT rx callback");
      goto CLEANUP;
   }

   antStatus = set_ant_state_callback(nativeJAnt_StateCallback);
   if (antStatus)
   {
      ANT_DEBUG_D("failed to set ANT state callback");
      goto CLEANUP;
   }

CLEANUP:
   ANT_FUNC_END();
   return antStatus;
}

static jint nativeJAnt_Destroy(JNIEnv *env, jobject obj)
{
   (void)env; //unused warning
   (void)obj; //unused warning
   ANTStatus status;
   ANT_FUNC_START();

   ANT_DEBUG_D("nativeJAnt_Destroy(): calling ant_deinit");
   status = ant_deinit();
   if (status)
   {
      ANT_DEBUG_D("failed to deinit ANT stack returned %d",(int)status);
      return status;
   }
   else
   {
      ANT_DEBUG_D("deinit ANT stack Success");
   }

   ANT_FUNC_END();
   return status;
}

static jint nativeJAnt_Enable(JNIEnv *env, jobject obj)
{
   (void)env; //unused warning
   (void)obj; //unused warning
   ANT_FUNC_START();

   ANTStatus status = ant_enable_radio();

   ANT_FUNC_END();
   return status;
}

static jint nativeJAnt_Disable(JNIEnv *env, jobject obj)
{
   (void)env; //unused warning
   (void)obj; //unused warning
   ANT_FUNC_START();

   ANTStatus status = ant_disable_radio();

   ANT_FUNC_END();
   return status;
}

static jint nativeJAnt_GetRadioEnabledStatus(JNIEnv *env, jobject obj)
{
   (void)env; //unused warning
   (void)obj; //unused warning
   ANT_FUNC_START();

   jint status = ant_radio_enabled_status();

   ANT_FUNC_END();
   return status;
}

static jint nativeJAnt_TxMessage(JNIEnv *env, jobject obj, jbyteArray msg)
{
   (void)obj; //unused warning
   ANT_FUNC_START();

   if (msg == NULL)
   {
      if (jniThrowException(env, "java/lang/NullPointerException", NULL))
      {
         ANT_ERROR("Unable to throw NullPointerException");
      }
      return -1;
   }

   jbyte* msgBytes = env->GetByteArrayElements(msg, NULL);
   jint msgLength = env->GetArrayLength(msg);

   ANTStatus status = ant_tx_message((ANT_U8) msgLength, (ANT_U8 *)msgBytes);
   ANT_DEBUG_D("nativeJAnt_TxMessage: ant_tx_message() returned %d", (int)status);

   env->ReleaseByteArrayElements(msg, msgBytes, JNI_ABORT);

   ANT_FUNC_END();
   return status;
}

static jint nativeJAnt_HardReset(JNIEnv *env, jobject obj)
{
   (void)env; //unused warning
   (void)obj; //unused warning
   ANT_FUNC_START();

   ANTStatus status = ant_radio_hard_reset();

   ANT_FUNC_END();
   return status;
}

extern "C"
{

   /**********************************************************************
    *                              Callback registration
    ***********************************************************************/
   void nativeJAnt_RxCallback(ANT_U8 ucLen, ANT_U8* pucData)
   {
      JNIEnv* env = NULL;
      jbyteArray jAntRxMsg = NULL;
      ANT_FUNC_START();

      ANT_DEBUG_D( "got message %d bytes", ucLen);

      g_jVM->AttachCurrentThread((&env), NULL);

      if (env == NULL)
      {
         ANT_DEBUG_D("nativeJAnt_RxCallback: Entered, env is null");
         return; // log error? cleanup?
      }
      else
      {
         ANT_DEBUG_D("nativeJAnt_RxCallback: jEnv %p", env);
      }

      jAntRxMsg = env->NewByteArray(ucLen);

      if (jAntRxMsg == NULL)
      {
         ANT_ERROR("nativeJAnt_RxCallback: Failed creating java byte[]");
         goto CLEANUP;
      }

      env->SetByteArrayRegion(jAntRxMsg,0,ucLen,(jbyte*)pucData);

      if (env->ExceptionOccurred())
      {
         ANT_ERROR("nativeJAnt_RxCallback: ExceptionOccurred during byte[] copy");
         goto CLEANUP;
      }
      ANT_DEBUG_V("nativeJAnt_RxCallback: Calling java rx callback");
      env->CallStaticVoidMethod(g_sJClazz, g_sMethodId_nativeCb_AntRxMessage, jAntRxMsg);
      ANT_DEBUG_V("nativeJAnt_RxCallback: Called java rx callback");

      if (env->ExceptionOccurred())
      {
         ANT_ERROR("nativeJAnt_RxCallback: Calling Java nativeCb_AntRxMessage failed");
         goto CLEANUP;
      }

      //Delete the local references
      if (jAntRxMsg != NULL)
      {
         env->DeleteLocalRef(jAntRxMsg);
      }

      ANT_DEBUG_D("nativeJAnt_RxCallback: Exiting, Calling DetachCurrentThread at the END");

      g_jVM->DetachCurrentThread();

      ANT_FUNC_END();
      return;

   CLEANUP:
      ANT_ERROR("nativeJAnt_RxCallback: Exiting due to failure");
      if (jAntRxMsg != NULL)
      {
         env->DeleteLocalRef(jAntRxMsg);
      }

      if (env->ExceptionOccurred())
      {
         env->ExceptionDescribe();
         env->ExceptionClear();
      }

      g_jVM->DetachCurrentThread();

      return;
   }

   void nativeJAnt_StateCallback(ANTRadioEnabledStatus uiNewState)
   {
      JNIEnv* env = NULL;
      jint jNewState = uiNewState;
      ANT_BOOL iShouldDetach = ANT_FALSE;
      ANT_FUNC_START();

      g_jVM->GetEnv((void**) &env, JNI_VERSION_1_4);
      if (env == NULL)
      {
         ANT_DEBUG_D("nativeJAnt_StateCallback: called from rx thread, attaching to VM");
         g_jVM->AttachCurrentThread((&env), NULL);
         if (env == NULL)
         {
            ANT_DEBUG_E("nativeJAnt_StateCallback: failed to attach rx thread to VM");
            return;
         }
         iShouldDetach = ANT_TRUE;
      }
      else
      {
         ANT_DEBUG_D("nativeJAnt_StateCallback: called from java enable/disable"
                         ", already attached, don't detach");
      }

      ANT_DEBUG_V("nativeJAnt_StateCallback: Calling java state callback");
      env->CallStaticVoidMethod(g_sJClazz, g_sMethodId_nativeCb_AntStateChange, jNewState);
      ANT_DEBUG_V("nativeJAnt_StateCallback: Called java state callback");

      if (env->ExceptionOccurred())
      {
         ANT_ERROR("nativeJAnt_StateCallback: Calling Java nativeCb_AntStateChange failed");
         env->ExceptionDescribe();
         env->ExceptionClear();
      }

      if (iShouldDetach)
      {
         ANT_DEBUG_D("nativeJAnt_StateCallback: env was attached, detaching");
         g_jVM->DetachCurrentThread();
      }

      ANT_FUNC_END();
      return;
   }
}

static JNINativeMethod g_sMethods[] =
{
   /* name, signature, funcPtr */
   {"nativeJAnt_Create", "()I", (void*)nativeJAnt_Create},
   {"nativeJAnt_Destroy", "()I", (void*)nativeJAnt_Destroy},
   {"nativeJAnt_Enable", "()I", (void*)nativeJAnt_Enable},
   {"nativeJAnt_Disable", "()I", (void*)nativeJAnt_Disable},
   {"nativeJAnt_GetRadioEnabledStatus", "()I", (void*)nativeJAnt_GetRadioEnabledStatus},
   {"nativeJAnt_TxMessage","([B)I", (void*)nativeJAnt_TxMessage},
   {"nativeJAnt_HardReset", "()I", (void *)nativeJAnt_HardReset}
};

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
   ANT_FUNC_START();
   (void)reserved; //unused warning

   g_jVM = vm;
   if (g_jVM->GetEnv((void**) &g_jEnv, JNI_VERSION_1_4) != JNI_OK) {
      ANT_ERROR("GetEnv failed");
      return -1;
   }
   if (NULL == g_jEnv) {
      ANT_ERROR("env is null");
      return -1;
   }

   g_sJClazz = g_jEnv->FindClass("com/dsi/ant/core/JAntJava");
   if (NULL == g_sJClazz) {
      ANT_ERROR("could not find class \"com/dsi/ant/core/JAntJava\"");
      return -1;
   }

   /* Save class information in global reference to prevent class unloading */
   g_sJClazz = (jclass)g_jEnv->NewGlobalRef(g_sJClazz);

   if (g_jEnv->RegisterNatives(g_sJClazz, g_sMethods, NELEM(g_sMethods)) != JNI_OK) {
      ANT_ERROR("failed to register methods");
      return -1;
   }

   g_sMethodId_nativeCb_AntRxMessage = g_jEnv->GetStaticMethodID(g_sJClazz,
                                             "nativeCb_AntRxMessage", "([B)V");
   if (NULL == g_sMethodId_nativeCb_AntRxMessage) {
      ANT_ERROR("VerifyMethodId: Failed getting method id of \"void nativeCb_AntRxMessage(byte[])\"");
      return -1;
   }

   g_sMethodId_nativeCb_AntStateChange = g_jEnv->GetStaticMethodID(g_sJClazz,
                                             "nativeCb_AntStateChange", "(I)V");
   if (NULL == g_sMethodId_nativeCb_AntStateChange) {
      ANT_ERROR("VerifyMethodId: Failed getting method id of \"void nativeCb_AntStateChange(int)\"");
      return -1;
   }

   ANT_FUNC_END();
   return JNI_VERSION_1_4;
}
