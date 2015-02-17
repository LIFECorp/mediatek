/*
 * ANT Stack
 *
 * Copyright 2011 Dynastream Innovations
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
/*******************************************************************************\
*
*   FILE NAME:    ant_driver_defines.h
*
*   BRIEF:
*      This file defines ANT specific HCI values used by the ANT stack for a
*      ST-E CG29XX.
*
*
\*******************************************************************************/

#ifndef __VFS_PRERELEASE_H
#define __VFS_PRERELEASE_H

// ANT HCI Packet Structure
// -----------------------------------------
// |         Header       | Data |  Footer  |
// |----------------------|-----------------|
// |Optional| Data | Opt. | ...  | Optional |
// | Opcode | Size | Sync |      | Checksum |
// Data may include any number of ANT packets, with no sync byte or checksum.
// A read from the driver may return any number of ANT HCI packets.


// ---------------------- REQUIRED

// Which chip is this library being built for:
#define ANT_CHIP_NAME                        "ST-E CG29XX"

// Set the file name the driver creates for the ANT device:
//   If chip uses separate command and data paths:
//#define ANT_COMMANDS_DEVICE_NAME             "/dev/cg2900_antradio_cmd"
//#define ANT_DATA_DEVICE_NAME                 "/dev/cg2900_antradio_data"
#define ANT_DEVICE_NAME                      "/dev/stpant"
// OR
//   If chip uses one path:
// #define ANT_DEVICE_NAME                      "/dev/Z"

// Set to the number of bytes of header is for Opcode:
#define ANT_HCI_OPCODE_SIZE                  0
// Set to the number of bytes of header is for Data Size:
#define ANT_HCI_SIZE_SIZE                    1

// Set to the number of bytes of header is for Sync:
#define ANT_HCI_FOOTER_SIZE                  0
#define ANT_HCI_SYNC_SIZE                    0
// Set to the number of bytes of footer is for Checksum:
#define ANT_HCI_CHECKSUM_SIZE                0


// ---------------------- OPTIONAL

// If hard reset is supported, define ANT_IOCTL_RESET
#define ANT_IOCTL_RESET                      _IOW('U', 210, int)
#define ANT_IOCTL_RESET_PARAMETER            (0)

// If the chip sends flow control messages:
//  Define the Opcode for a Flow Control message:
#define ANT_MESG_FLOW_CONTROL                ((ANT_U8)0xC9)
// AND
//   define the message content:
//     That signals Flow Go:
#define ANT_FLOW_GO                          ((ANT_U8)0x00)
//     That signals Flow Stop:
#define ANT_FLOW_STOP                        ((ANT_U8)0x80)

#endif /* ifndef __VFS_PRERELEASE_H */