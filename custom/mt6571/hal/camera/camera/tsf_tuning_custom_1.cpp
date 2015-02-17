/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "camera_custom_types.h"
#include "tsf_tuning_custom.h"

MBOOL
isTSFVdoStop(MINT32 const i4SensorDev)
{
    if (i4SensorDev == 1)
    {
    #ifdef MTK_SLOW_MOTION_VIDEO_SUPPORT
        return MTRUE;
    #else
        return MFALSE;
    #endif
    }
    else
    {
        return MFALSE;
    }
}

