/*
 * ANT Stack
 *
 * Copyright 2009 Dynastream Innovations
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************\
*
*   FILE NAME:      ANT_types.h
*
*   BRIEF:
*		This file defines types used in the ANT stack
*
*
\******************************************************************************/

#ifndef __ANT_TYPES_H
#define __ANT_TYPES_H

#include <stdint.h>

/* -------------------------------------------------------------
 *                  8 Bits Types
 */
typedef uint8_t     ANT_U8;
typedef int8_t      ANT_S8;

/* -------------------------------------------------------------
 *                  16 Bits Types
 */
typedef uint16_t    ANT_U16;
typedef int8_t      ANT_S16;

/* -------------------------------------------------------------
 *                  32 Bits Types
 */
typedef uint32_t    ANT_U32;
typedef int32_t     ANT_S32;


/* -------------------------------------------------------------
 *          Native Integer Types (# of bits irrelevant)
 */
typedef int          ANT_INT;
typedef unsigned int ANT_UINT;


/* --------------------------------------------------------------
 *                  Boolean Definitions
 */
typedef ANT_INT     ANT_BOOL;

#define ANT_TRUE    (1 == 1)
#define ANT_FALSE   (1 == 0)

/*
*/
#define ANT_NO_VALUE                                    ((ANT_U32)0xFFFFFFFFUL)


/* -------------------------------------------------------------
 *              ANTRadioEnabledStatus Type
 */
typedef ANT_UINT ANTRadioEnabledStatus;

#define RADIO_STATUS_UNKNOWN                          ((ANTRadioEnabledStatus)0)
#define RADIO_STATUS_ENABLING                         ((ANTRadioEnabledStatus)1)
#define RADIO_STATUS_ENABLED                          ((ANTRadioEnabledStatus)2)
#define RADIO_STATUS_DISABLING                        ((ANTRadioEnabledStatus)3)
#define RADIO_STATUS_DISABLED                         ((ANTRadioEnabledStatus)4)
#define RADIO_STATUS_NOT_SUPPORTED                    ((ANTRadioEnabledStatus)5)
#define RADIO_STATUS_SERVICE_NOT_INSTALLED            ((ANTRadioEnabledStatus)6)
#define RADIO_STATUS_SERVICE_NOT_CONNECTED            ((ANTRadioEnabledStatus)7)
#define RADIO_STATUS_RESETTING                        ((ANTRadioEnabledStatus)8)
#define RADIO_STATUS_RESET                            ((ANTRadioEnabledStatus)9)

/*------------------------------------------------------------------------------
 * ANTStatus Type
 *
 */
typedef ANT_UINT ANTStatus;

#define ANT_STATUS_SUCCESS                                  ((ANTStatus)0)
#define ANT_STATUS_FAILED                                   ((ANTStatus)1)
#define ANT_STATUS_PENDING                                  ((ANTStatus)2)
#define ANT_STATUS_INVALID_PARM                             ((ANTStatus)3)
#define ANT_STATUS_IN_PROGRESS                              ((ANTStatus)4)
#define ANT_STATUS_NOT_APPLICABLE                           ((ANTStatus)5)
#define ANT_STATUS_NOT_SUPPORTED                            ((ANTStatus)6)
#define ANT_STATUS_INTERNAL_ERROR                           ((ANTStatus)7)
#define ANT_STATUS_TRANSPORT_INIT_ERR                       ((ANTStatus)8)
#define ANT_STATUS_HARDWARE_ERR                             ((ANTStatus)9)
#define ANT_STATUS_NO_VALUE_AVAILABLE                       ((ANTStatus)10)
#define ANT_STATUS_CONTEXT_DOESNT_EXIST                     ((ANTStatus)11)
#define ANT_STATUS_CONTEXT_NOT_DESTROYED                    ((ANTStatus)12)
#define ANT_STATUS_CONTEXT_NOT_ENABLED                      ((ANTStatus)13)
#define ANT_STATUS_CONTEXT_NOT_DISABLED                     ((ANTStatus)14)
#define ANT_STATUS_NOT_DE_INITIALIZED                       ((ANTStatus)15)
#define ANT_STATUS_NOT_INITIALIZED                          ((ANTStatus)16)
#define ANT_STATUS_TOO_MANY_PENDING_CMDS                    ((ANTStatus)17)
#define ANT_STATUS_DISABLING_IN_PROGRESS                    ((ANTStatus)18)

#define ANT_STATUS_COMMAND_WRITE_FAILED                     ((ANTStatus)20)
#define ANT_STATUS_SCRIPT_EXEC_FAILED                       ((ANTStatus)21)

#define ANT_STATUS_FAILED_BT_NOT_INITIALIZED                ((ANTStatus)23)
#define ANT_STATUS_AUDIO_OPERATION_UNAVAILIBLE_RESOURCES    ((ANTStatus)24)

#define ANT_STATUS_TRANSPORT_UNSPECIFIED_ERROR              ((ANTStatus)30)

#define ANT_STATUS_NO_VALUE                                 ((ANTStatus)100)

#endif  /* __ANT_TYPES_H */
