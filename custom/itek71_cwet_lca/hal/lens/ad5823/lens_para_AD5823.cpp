#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_lens.h"

const NVRAM_LENS_PARA_STRUCT AD5823_LENS_PARA_DEFAULT_VALUE =
{
    //Version
    NVRAM_CAMERA_LENS_FILE_VERSION,
    // Focus Range NVRAM
    {0, 1023},

    // AF NVRAM
    {
        // -------- AF ------------
        {170, // i4Offset
          7, // i4NormalNum
          7, // i4MacroNum
          0, // i4InfIdxOffset
          0, //i4MacroIdxOffset          
          {
             0,  50,  90, 150, 200, 270, 350,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0    
          },
          15, // i4THRES_MAIN;
          10, // i4THRES_SUB;            
            4,  // i4INIT_WAIT;
          {0, 0, 500, 500, 500}, // i4FRAME_WAIT
          0,  // i4DONE_WAIT;
          0,  // i4FAIL_POS;

          33,  // i4FRAME_TIME                        
          5,  // i4FIRST_FV_WAIT;
            
            40,  // i4FV_CHANGE_THRES;
            100,  // i4FV_CHANGE_OFFSET;        
            10,  // i4FV_CHANGE_CNT;
            20,  // i4GS_CHANGE_THRES;    
            15,  // i4GS_CHANGE_OFFSET;    
            30,  // i4GS_CHANGE_CNT;            
            15,  // i4FV_STABLE_THRES;         // percentage -> 0 more stable  
            5000,  // i4FV_STABLE_OFFSET;        // value -> 0 more stable
            25,  // i4FV_STABLE_NUM;           // max = 50 (more stable), reset = 0
            22,  // i4FV_STABLE_CNT;           // max = 50                                      
            10,  // i4FV_1ST_STABLE_THRES;        
            100,  // i4FV_1ST_STABLE_OFFSET;
            15,  // i4FV_1ST_STABLE_NUM;                        
            12  // i4FV_1ST_STABLE_CNT;      
         },
         
         // -------- ZSD AF ------------
        {170, // i4Offset
          7, // i4NormalNum
          7, // i4MacroNum
          0, // i4InfIdxOffset
          0, //i4MacroIdxOffset          
           {
             0,  50,  90, 150, 200, 270, 350,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0         
           },
           15, // i4THRES_MAIN;
           10, // i4THRES_SUB;            
            4,  // i4INIT_WAIT;
           {500, 500, 500, 500, 500}, // i4FRAME_WAIT
           0,  // i4DONE_WAIT;
           0,  // i4FAIL_POS;
           66,  // i4FRAME_TIME                                  
           5,  // i4FIRST_FV_WAIT;
            40,  // i4FV_CHANGE_THRES;
            100,  // i4FV_CHANGE_OFFSET;        
            10,  // i4FV_CHANGE_CNT;
            20,  // i4GS_CHANGE_THRES;    
            15,  // i4GS_CHANGE_OFFSET;    
            20,  // i4GS_CHANGE_CNT;            
            15,  // i4FV_STABLE_THRES;         // percentage -> 0 more stable  
            5000,  // i4FV_STABLE_OFFSET;        // value -> 0 more stable
            15,   // i4FV_STABLE_NUM;           // max = 50 (more stable), reset = 0
            13,   // i4FV_STABLE_CNT;           // max = 50                                      
            15,  // i4FV_1ST_STABLE_THRES;        
            100,  // i4FV_1ST_STABLE_OFFSET;
            10,  // i4FV_1ST_STABLE_NUM;                        
             8  // i4FV_1ST_STABLE_CNT;       
           }, 
           
           // -------- VAFC ------------
           {170, // i4Offset
            7, // i4NormalNum
            7, // i4MacroNum
            0, // i4InfIdxOffset
            0, //i4MacroIdxOffset           
            {
              0,  50,  90, 150, 200, 270, 350,   0,   0,   0,
              0,   0,    0,     0,   0,   0,    0,     0,   0,   0,
              0,   0,    0,     0,   0,   0,    0,     0,   0,   0         
            },
            10, // i4THRES_MAIN;
            5, // i4THRES_SUB;            
            4,  // i4INIT_WAIT;
           {0, 0, 500, 500, 500}, // i4FRAME_WAIT
           0,  // i4DONE_WAIT;
             
           0,  // i4FAIL_POS;

           33,  // i4FRAME_TIME                          
           5,  // i4FIRST_FV_WAIT;
            20,  // i4FV_CHANGE_THRES;
            100,  // i4FV_CHANGE_OFFSET;        
            5,  // i4FV_CHANGE_CNT;
            20,  // i4GS_CHANGE_THRES;    
            5,  // i4GS_CHANGE_OFFSET;    
            5,  // i4GS_CHANGE_CNT;            
            5,  // i4FV_STABLE_THRES;         // percentage -> 0 more stable  
            100,  // i4FV_STABLE_OFFSET;        // value -> 0 more stable
            4,   // i4FV_STABLE_NUM;           // max = 50 (more stable), reset = 0
            4,   // i4FV_STABLE_CNT;           // max = 50                                      
            20,  // i4FV_1ST_STABLE_THRES;        
            100,  // i4FV_1ST_STABLE_OFFSET;
            5,  // i4FV_1ST_STABLE_NUM;                        
            5  // i4FV_1ST_STABLE_CNT;        
          },

        // --- sAF_TH ---
         {
          8,   // i4ISONum;
          {100,150,200,300,400,600,800,1600},       // i4ISO[ISO_MAX_NUM];
                  
          6,   // i4GMeanNum;
          {20,55,105,150,180,205},        // i4GMean[GMEAN_MAX_NUM];

          { 89,  89,  89,  89,  89,  88,  82,  81,
           127, 127, 127, 127, 126, 126, 122, 121,
           181, 180, 180, 180, 180, 180, 177, 177}, // i4GMR[3][ISO_MAX_NUM];
          
// ------------------------------------------------------------------------                  
            {6000,6000,6000,6000,6000,3000,3000,3000,
             6000,6000,6000,6000,6000,3000,3000,3000,
             6000,6000,6000,6000,6000,3000,3000,3000,
             6000,6000,6000,6000,6000,3000,3000,3000,
             6000,6000,6000,6000,6000,3000,3000,3000,
             6000,6000,6000,6000,6000,3000,3000,3000},         // i4FV_DC[GMEAN_MAX_NUM][ISO_MAX_NUM];
           
            {5000,5000,5000,5000,5000,5000,5000,5000, 
             5000,5000,5000,5000,5000,5000,5000,5000,   
             5000,5000,5000,5000,5000,5000,5000,5000,    
             5000,5000,5000,5000,5000,5000,5000,5000,    
             5000,5000,5000,5000,5000,5000,5000,5000,
             5000,5000,5000,5000,5000,5000,5000,5000},          // i4MIN_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];        

          {3, 3, 3, 3, 5, 5, 5, 8,
           3, 3, 3, 3, 5, 5, 5, 8,
           3, 3, 3, 3, 5, 5, 5, 8,
           3, 3, 3, 3, 5, 5, 5, 8,
           3, 3, 3, 3, 5, 5, 5, 8,
           3, 3, 3, 3, 5, 5, 5, 8}, // i4HW_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];       
// ------------------------------------------------------------------------
          {0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0},        // i4FV_DC2[GMEAN_MAX_NUM][ISO_MAX_NUM];
           
          {0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0},         // i4MIN_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];
          
          {3, 3, 3, 3, 5, 7, 10, 12,
           3, 3, 3, 3, 5, 7, 10, 12,
           3, 3, 3, 3, 5, 7, 10, 12,
           3, 3, 3, 3, 5, 7, 10, 12,
           3, 3, 3, 3, 5, 7, 10, 12,
           3, 3, 3, 3, 5, 7, 10, 12}     // i4HW_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];       
          
         },
// ------------------------------------------------------------------------

         // --- sZSDAF_TH ---
          {
           8,   // i4ISONum;
           {100,150,200,300,400,600,800,1600},       // i4ISO[ISO_MAX_NUM];
                   
           6,   // i4GMeanNum;
           {20,55,105,150,180,205},        // i4GMean[GMEAN_MAX_NUM];

           {88,  88,  87,  87,  86,  86,  85,  82,
           126, 126, 125, 125, 125, 124, 123, 121,
           180, 180, 180, 179, 179, 179, 178, 177},       // i4GMR[3][ISO_MAX_NUM];
           
// ------------------------------------------------------------------------                   
            {3000,3000,3000,3000,3000,1000,1000,1000,
             3000,3000,3000,3000,3000,1000,1000,1000,
             3000,3000,3000,3000,3000,1000,1000,1000,
             3000,3000,3000,3000,3000,1000,1000,1000,
             3000,3000,3000,3000,3000,1000,1000,1000,
             3000,3000,3000,3000,3000,1000,1000,1000,},         // i4FV_DC[GMEAN_MAX_NUM][ISO_MAX_NUM];
            
            {5000,5000,5000,5000,5000,4000,4000,4000, 
             5000,5000,5000,5000,5000,4000,4000,4000, 
             5000,5000,5000,5000,5000,4000,4000,4000, 
             5000,5000,5000,5000,5000,4000,4000,4000,   
             5000,5000,5000,5000,5000,4000,4000,4000, 
             5000,5000,5000,5000,5000,4000,4000,4000, },         // i4MIN_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];        
         
           {6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,},      // i4HW_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];     

// ------------------------------------------------------------------------
           {0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0},        // i4FV_DC2[GMEAN_MAX_NUM][ISO_MAX_NUM];
            
           {0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0},         // i4MIN_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];
           
           
           {6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,
            6, 6, 6, 6, 14, 16, 18, 20,},           // i4HW_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];          
// ------------------------------------------------------------------------           
          },
          1, // i4VAFC_FAIL_CNT;
          0, // i4CHANGE_CNT_DELTA;
          0, // i4LV_THRES;
          18, // i4WIN_PERCENT_W;
          24, // i4WIN_PERCENT_H;                
          120,  // i4InfPos;
          20, //i4AFC_STEP_SIZE;
          {
              {50, 100, 150, 200, 250}, // back to bestpos step
              { 0,   0,   0,   0,   0}  // hysteresis compensate step
          },
        {0, 100, 200, 350, 500}, // back jump
        500,  //i4BackJumpPos

          140, // i4FDWinPercent;
          40, // i4FDSizeDiff;

          3,   //i4StatGain          
        {0,  //i4Coef[0] enable left peak search if i4Coef[0] != 0
         0,  //i4Coef[1] disable left peak search, left step= 3 + i4Coef[1]
         0,  //i4Coef[2] enable 5 point curve fitting if i4Coef[2] != 0
         0,  //i4Coef[3] AF done happen delay count
         0,  //i4Coef[4] enable AF window change with Zoom-in
         1,  //i4Coef[5] AF use sensor lister => 0:disable, 1:enable
         60, //i4Coef[6] post comp max offset => 0:disable, others:enable
         2,  //i4Coef[7] scenechange enhancement level => 0:disable, 1~3:;level stable to sensitive, 9:use coef[8~11] //init will fail when factory mode, modify to 0 for turn off it.
         0,  //i4Coef[8] scenechange params for GS, chgT|chgN|stbT|stbN
         0,  //i4Coef[9] scenechange params for AEBlock, chgT|chgN|stbT|stbN
         0,  //i4Coef[10] scenechange params for GYRO-sensor, chgT|chgN|stbT|stbN
         0,  //i4Coef[11] scenechange params for G-sensor, chgT|chgN|stbT|stbN
         0,  //i4Coef[12] (video mode) scenechange params for GS, chgT|chgN|stbT|stbN
         0,  //i4Coef[13] (video mode) scenechange params for AEBlock, chgT|chgN|stbT|stbN
         0,  //i4Coef[14] (video mode) scenechange params for GYRO-sensor, chgT|chgN|stbT|stbN
         0,  //i4Coef[15] (video mode) scenechange params for G-sensor, chgT|chgN|stbT|stbN
         0,  //i4Coef[16] AF easy tuning paremeters
         0,  //i4Coef[17] AF easy tuning paremeters
         0,  //i4Coef[18] AF easy tuning paremeters
         0   //i4Coef[19] AF easy tuning paremeters
        }// i4Coef[20];
    },

    {0}
};


UINT32 AD5823_getDefaultData(VOID *pDataBuf, UINT32 size)
{
    UINT32 dataSize = sizeof(NVRAM_LENS_PARA_STRUCT);

    if ((pDataBuf == NULL) || (size < dataSize))
    {
        return 1;
    }

    // copy from Buff to global struct
    memcpy(pDataBuf, &AD5823_LENS_PARA_DEFAULT_VALUE, dataSize);

    return 0;
}

PFUNC_GETLENSDEFAULT pAD5823_getDefaultData = AD5823_getDefaultData;


