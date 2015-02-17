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
*   FILE NAME:      ANT_utils.h
*
*   BRIEF:          
*		This file defines utility functions
*
*
\******************************************************************************/
#ifndef __ANT_UTILS_H
#define __ANT_UTILS_H

#include "ant_types.h"

/****************************************************************************
 *
 * Type Definitions
 *
 ****************************************************************************/



/*------------------------------------------------------------------------------
 * ANT_xxx()
 *
 * Brief:  
 *      xxx
 *
 * Description:
 *      xxx
 *
 * Type:
 *      Syxxx/As
 *
 * Parameters:
 *      xxx [in]    - 
 *
 * Returns:
 *      xxx
 */
ANT_U16 ANT_UTILS_BEtoHost16(ANT_U8 * num);
/*------------------------------------------------------------------------------
 * ANT_xxx()
 *
 * Brief:  
 *      xxx
 *
 * Description:
 *      xxx
 *
 * Type:
 *      Syxxx/As
 *
 * Parameters:
 *      xxx [in]    - 
 *
 * Returns:
 *      xxx
 */
ANT_U16 ANT_UTILS_LEtoHost16(ANT_U8 * num);
/*------------------------------------------------------------------------------
 * ANT_xxx()
 *
 * Brief:  
 *      xxx
 *
 * Description:
 *      xxx
 *
 * Type:
 *      Syxxx/As
 *
 * Parameters:
 *      xxx [in]    - 
 *
 * Returns:
 *      xxx
 */
void ANT_UTILS_StoreBE16(ANT_U8 *buff, ANT_U16 be_value) ;
/*------------------------------------------------------------------------------
 * ANT_xxx()
 *
 * Brief:  
 *      xxx
 *
 * Description:
 *      xxx
 *
 * Type:
 *      Syxxx/As
 *
 * Parameters:
 *      xxx [in]    - 
 *
 * Returns:
 *      xxx
 */
void ANT_UTILS_StoreLE16(ANT_U8 *buff, ANT_U16 le_value) ;
/*------------------------------------------------------------------------------
 * ANT_xxx()
 *
 * Brief:  
 *      xxx
 *
 * Description:
 *      xxx
 *
 * Type:
 *      Syxxx/As
 *
 * Parameters:
 *      xxx [in]    - 
 *
 * Returns:
 *      xxx
 */
void ANT_UTILS_StoreBE32(ANT_U8 *buff, ANT_U32 be_value) ;



#endif  /* __ANT_UTILS_H */