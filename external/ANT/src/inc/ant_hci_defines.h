/*
 * ANT Stack
 *
 * Copyright 2013 Dynastream Innovations
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
*   FILE NAME:    ant_hci_defines.h
*
*   BRIEF:
*      This file defines ANT specific HCI values used by the ANT chip that are
*      not specific to the underlying chip. These should not need to be modified,
*      but should be verified they are correct.
*
\*******************************************************************************/

#ifndef __VFS_INDEPENDENT_H
#define __VFS_INDEPENDENT_H

// ANT HCI Packet Structure
// -----------------------------------------
// |         Header       | Data |  Footer  |
// |----------------------|-----------------|
// |Optional| Data | Opt. | ...  | Optional |
// | Opcode | Size | Sync |      | Checksum |
// Data may include any number of ANT packets, with no sync byte or checksum.
// A read from the driver may return any number of ANT HCI packets.

#include "ant_driver_defines.h"

#define ANT_HCI_HEADER_SIZE                  ((ANT_HCI_OPCODE_SIZE) + (ANT_HCI_SIZE_SIZE) + (ANT_HCI_SYNC_SIZE))
#define ANT_HCI_FOOTER_SIZE                  (ANT_HCI_CHECKSUM_SIZE)

#define ANT_HCI_OPCODE_OFFSET                0
#define ANT_HCI_SIZE_OFFSET                  ((ANT_HCI_OPCODE_OFFSET) + (ANT_HCI_OPCODE_SIZE))
#define ANT_HCI_SYNC_OFFSET                  ((ANT_HCI_SIZE_OFFSET) + (ANT_HCI_SIZE_SIZE))
#define ANT_HCI_DATA_OFFSET                  (ANT_HCI_HEADER_SIZE)

#define ANT_FLOW_GO_WAIT_TIMEOUT_SEC         10

#endif /* ifndef __VFS_INDEPENDENT_H */
