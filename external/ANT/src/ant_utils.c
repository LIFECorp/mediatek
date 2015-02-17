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
*   FILE NAME:      ANT_utils.c
*
*   BRIEF:          
*		This file contains utility functions
*
*
\******************************************************************************/

#include "ant_types.h"
#include "ant_utils.h"


ANT_U16 ANT_UTILS_BEtoHost16(ANT_U8 * num)
{
    return (ANT_U16)(((ANT_U16) *(num) << 8) | ((ANT_U16) *(num+1)));
}
ANT_U16 ANT_UTILS_LEtoHost16(ANT_U8 * num)
{
    return (ANT_U16)(((ANT_U16) *(num+1) << 8) | ((ANT_U16) *(num)));
}
void ANT_UTILS_StoreBE16(ANT_U8 *buff, ANT_U16 be_value) 
{
   buff[0] = (ANT_U8)(be_value>>8);
   buff[1] = (ANT_U8)be_value;
}
void ANT_UTILS_StoreLE16(ANT_U8 *buff, ANT_U16 le_value) 
{
   buff[1] = (ANT_U8)(le_value>>8);
   buff[0] = (ANT_U8)le_value;
}

void ANT_UTILS_StoreBE32(ANT_U8 *buff, ANT_U32 be_value) 
{
   buff[0] = (ANT_U8)(be_value>>24);
   buff[1] = (ANT_U8)(be_value>>16);
   buff[2] = (ANT_U8)(be_value>>8);
   buff[3] = (ANT_U8)be_value;
}