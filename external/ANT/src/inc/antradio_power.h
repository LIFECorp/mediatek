/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ANTRADIOPM_H
#define __ANTRADIOPM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Enable the ANT radio interface.
 *
 * Responsible for power on, and bringing up HCI interface.
 * Will block until the HCI interface is ready to use.
 *
 * Returns 0 on success, -ve on error */
int ant_enable();

/* Disable the ANT radio interface.
 *
 * Responsbile for pulling down the HCI interface, and powering down the chip.
 * Will block until power down is complete, and it is safe to immediately call 
 * enable().
 *
 * Returns 0 on success, -ve on error */
int ant_disable();

/* Returns 1 if enabled, 0 if disabled, and -ve on error */
int ant_is_enabled();

#ifdef __cplusplus
}
#endif

#endif