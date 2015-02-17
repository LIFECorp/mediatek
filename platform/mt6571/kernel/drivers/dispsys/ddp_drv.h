#ifndef __DDP_DRV_H__
#define __DDP_DRV_H__
#include <linux/ioctl.h>

#include "ddp_hal.h"
#include "ddp_aal.h"

typedef enum DISP_MODULE_ENUM_
{
    DISP_MODULE_CONFIG,
    DISP_MODULE_MUTEX,
    DISP_MODULE_OVL,
    DISP_MODULE_WDMA0,
    DISP_MODULE_RDMA0,
    DISP_MODULE_PQ,
    DISP_MODULE_BLS,
    DISP_MODULE_DPI0,
    DISP_MODULE_DBI,
    DISP_MODULE_DSI_CMD,
    DISP_MODULE_DSI_VDO,
    DISP_MODULE_MAX
} DISP_MODULE_ENUM;


typedef struct
{
    unsigned int reg;
    unsigned int val;
    unsigned int mask;
} DISP_WRITE_REG;

typedef struct
{
    unsigned int reg;
    unsigned int *val;
    unsigned int mask;
} DISP_READ_REG;

typedef enum
{
    DDP_YUYV ,
    DDP_UYVY ,
    DDP_YVYU ,
    DDP_VYUY ,

    DDP_YUV444   ,

    DDP_RGB565    ,  // 14
    DDP_RGB888    ,  // 15
    DDP_ARGB8888  ,  // 16
    DDP_ABGR8888  ,  // 17

    DDP_RGBA8888,
    DDP_BGRA8888,

    DDP_PARGB8888 ,
    DDP_XARGB8888 ,
    DDP_NONE_FMT
} DDP_OVL_FORMAT;

typedef struct DISP_EXEC_COMMAND
{
    int taskID;
    uint32_t scenario;
    uint32_t priority;
    uint32_t engineFlag;
    uint32_t *pFrameBaseSW;
    uint32_t *pTileBaseSW;
    uint32_t blockSize;
} DISP_EXEC_COMMAND;

typedef struct
{
    int layer;

    unsigned int addr;
    DDP_OVL_FORMAT fmt;

    int x;
    int y;
    int w;
    int h;                  // clip region
    int pitch;
} DISP_OVL_INFO;



//PQ
#define PQ_TUNING_INDEX     (10 + 3)
#define SHP_TUNING_INDEX    12
#define SHP_PARAM_MAX       11


#define PQ_HUE_ADJ_PHASE_CNT 4
#define PQ_SAT_ADJ_PHASE_CNT 4

#define SHARPNESS_PARAM_SIZE 7
#define CONTRAST_PARAM_SIZE 8
#define SATURATION_PARAM_SIZE 8

typedef struct {
    unsigned long u4SharpnessAdj;// 0 : min , 9 : max.
    unsigned long u4ContrastAdj;// 0 : min , 9 : max.
    unsigned long u4SaturationAdj;// 0 : min , 9 : max.
} DISP_PQ_PARAM;

typedef struct{
//    unsigned char sharpness[PQ_TUNING_INDEX][SHARPNESS_PARAM_SIZE];
    unsigned char contrast[PQ_TUNING_INDEX][CONTRAST_PARAM_SIZE];
    unsigned short saturation[PQ_TUNING_INDEX][SATURATION_PARAM_SIZE];
} DISPLAY_PQ_T;

#define GAMMA_LUT_ENTRY     512
typedef struct{
    unsigned long entry[3][GAMMA_LUT_ENTRY];
} DISPLAY_GAMMA_T;


typedef struct{

    unsigned int entry[SHP_TUNING_INDEX][SHP_PARAM_MAX];

} DISPLAY_SHP_T;


#define PWM_LUT_ENTRY       33
#define PWM_LUT_ENTRY_BIT   12
typedef struct{
    unsigned long entry[PWM_LUT_ENTRY];
} DISPLAY_PWM_T;


typedef enum
{
    DISP_INTERLACE_FORMAT_NONE,
    DISP_INTERLACE_FORMAT_TOP_FIELD,
    DISP_INTERLACE_FORMAT_BOTTOM_FIELD
}DISP_INTERLACE_FORMAT;

typedef enum
{
    DISP_COLOR_FORMAT_YUV_420_3P      , // 0
    DISP_COLOR_FORMAT_YUV_420_2P_YUYV ,
    DISP_COLOR_FORMAT_YUV_420_2P_UYVY ,
    DISP_COLOR_FORMAT_YUV_420_2P_YVYU ,
    DISP_COLOR_FORMAT_YUV_420_2P_VYUY ,
    DISP_COLOR_FORMAT_YUV_420_2P_ISP_BLK  , // 5
    DISP_COLOR_FORMAT_YUV_420_2P_VDO_BLK  ,
    DISP_COLOR_FORMAT_YUV_422_3P    ,
    DISP_COLOR_FORMAT_YUV_422_2P    ,
    DISP_COLOR_FORMAT_YUV_422_I     ,
    DISP_COLOR_FORMAT_YUV_422_I_BLK , // 10
    DISP_COLOR_FORMAT_YUV_444_3P    ,
    DISP_COLOR_FORMAT_YUV_444_2P    ,
    DISP_COLOR_FORMAT_YUV_444_1P    ,

    DISP_COLOR_FORMAT_RGB565    ,  // 14
    DISP_COLOR_FORMAT_RGB888    ,  // 15
    DISP_COLOR_FORMAT_ARGB8888  ,  // 16
    DISP_COLOR_FORMAT_ABGR8888  ,  // 17

    DISP_COLOR_FORMAT_RGBA8888,
    DISP_COLOR_FORMAT_BGRA8888,

    DISP_COLOR_FORMAT_PARGB8888 ,
    DISP_COLOR_FORMAT_XARGB8888 ,

    DISP_COLOR_FORMAT_MTKYUV,
    DISP_COLOR_FORMAT_YUV_420_3P_YVU
}DISP_COLOR_FORMAT;


#define DISP_IOCTL_MAGIC        'x'

#define DISP_IOCTL_WRITE_REG       _IOW     (DISP_IOCTL_MAGIC, 1, DISP_WRITE_REG)
#define DISP_IOCTL_READ_REG        _IOWR    (DISP_IOCTL_MAGIC, 2, DISP_READ_REG)
#define DISP_IOCTL_WAIT_IRQ        _IOR     (DISP_IOCTL_MAGIC, 3, disp_wait_irq_struct)
#define DISP_IOCTL_DUMP_REG        _IOR     (DISP_IOCTL_MAGIC, 4, int)
#define DISP_IOCTL_LOCK_THREAD     _IOR     (DISP_IOCTL_MAGIC, 5, int)
#define DISP_IOCTL_UNLOCK_THREAD   _IOR     (DISP_IOCTL_MAGIC, 6, int)
#define DISP_IOCTL_MARK_CMQ        _IOR     (DISP_IOCTL_MAGIC, 7, int)
#define DISP_IOCTL_WAIT_CMQ        _IOR     (DISP_IOCTL_MAGIC, 8, int)
#define DISP_IOCTL_SYNC_REG        _IOR     (DISP_IOCTL_MAGIC, 9, int)

#define DISP_IOCTL_LOCK_MUTEX      _IOW     (DISP_IOCTL_MAGIC, 20, int)
#define DISP_IOCTL_UNLOCK_MUTEX    _IOR     (DISP_IOCTL_MAGIC, 21, int)

#define DISP_IOCTL_LOCK_RESOURCE   _IOW     (DISP_IOCTL_MAGIC, 25, int)
#define DISP_IOCTL_UNLOCK_RESOURCE _IOR     (DISP_IOCTL_MAGIC, 26, int)

#define DISP_IOCTL_SET_INTR        _IOR     (DISP_IOCTL_MAGIC, 10, int)
#define DISP_IOCTL_TEST_PATH       _IOR     (DISP_IOCTL_MAGIC, 11, int)

#define DISP_IOCTL_CLOCK_ON        _IOR     (DISP_IOCTL_MAGIC, 12, int)
#define DISP_IOCTL_CLOCK_OFF       _IOR     (DISP_IOCTL_MAGIC, 13, int)

#define DISP_IOCTL_RUN_DPF         _IOW     (DISP_IOCTL_MAGIC, 30, int)
#define DISP_IOCTL_CHECK_OVL       _IOR     (DISP_IOCTL_MAGIC, 31, int)
#define DISP_IOCTL_GET_OVL         _IOWR    (DISP_IOCTL_MAGIC, 32, DISP_OVL_INFO)

#define DISP_IOCTL_EXEC_COMMAND    _IOW     (DISP_IOCTL_MAGIC, 33, DISP_EXEC_COMMAND)
#define DISP_IOCTL_RESOURCE_REQUIRE   _IOR     (DISP_IOCTL_MAGIC, 34, int)

//Add for AAL control - S
//0 : disable AAL event, 1 : enable AAL event
#define DISP_IOCTL_AAL_EVENTCTL    _IOW    (DISP_IOCTL_MAGIC, 15 , int)
//Get AAL statistics data.
#define DISP_IOCTL_GET_AALSTATISTICS  _IOR  (DISP_IOCTL_MAGIC, 16 , DISP_AAL_STATISTICS)
//Update AAL setting
#define DISP_IOCTL_SET_AALPARAM    _IOW    (DISP_IOCTL_MAGIC, 17 , DISP_AAL_PARAM)
//Update PQ setting
#define DISP_IOCTL_SET_PQPARAM     _IOW    (DISP_IOCTL_MAGIC, 60 , DISP_PQ_PARAM)
#define DISP_IOCTL_SET_PQINDEX     _IOW    (DISP_IOCTL_MAGIC, 19 , DISPLAY_PQ_T)
#define DISP_IOCTL_SET_GAMMALUT    _IOW    (DISP_IOCTL_MAGIC, 20 , DISPLAY_GAMMA_T)
//Update BLS setting
#define DISP_IOCTL_SET_PWMLUT      _IOW    (DISP_IOCTL_MAGIC, 22 , DISPLAY_PWM_T)

//Add for AAL control - E
/*-----------------------------------------------------------------------------
    DDP Kernel Mode API  (for Kernel Trap)
  -----------------------------------------------------------------------------*/
//DDPK Bitblit
//#define DISP_IOCTL_G_WAIT_REQUEST  _IOR     (DISP_IOCTL_MAGIC , 40 , DDPIOCTL_DdpkBitbltConfig)
//#define DISP_IOCTL_T_INFORM_DONE   _IOW     (DISP_IOCTL_MAGIC , 41 , DDPIOCTL_DdpkBitbltInformDone)

#define DISP_IOCTL_SET_CLKON _IOW (DISP_IOCTL_MAGIC, 50 , DISP_MODULE_ENUM)
#define DISP_IOCTL_SET_CLKOFF _IOW (DISP_IOCTL_MAGIC, 51 , DISP_MODULE_ENUM)

//Get PQ setting
#define DISP_IOCTL_GET_PQPARAM          _IOR    (DISP_IOCTL_MAGIC, 52 , DISP_PQ_PARAM)
#define DISP_IOCTL_SET_SHPINDEX         _IOW    (DISP_IOCTL_MAGIC, 53 , DISPLAY_SHP_T)
#define DISP_IOCTL_GET_SHPINDEX         _IOR    (DISP_IOCTL_MAGIC, 54 , DISPLAY_SHP_T)
#define DISP_IOCTL_MUTEX_CONTROL        _IOW    (DISP_IOCTL_MAGIC, 55 , int)
#define DISP_IOCTL_GET_LCMINDEX         _IOR    (DISP_IOCTL_MAGIC, 56 , int)
#define DISP_IOCTL_SET_PQ_CAM_PARAM     _IOW    (DISP_IOCTL_MAGIC, 57 , DISP_PQ_PARAM)
#define DISP_IOCTL_GET_PQ_CAM_PARAM     _IOR    (DISP_IOCTL_MAGIC, 68 , DISP_PQ_PARAM)
#define DISP_IOCTL_SET_PQ_GAL_PARAM     _IOW    (DISP_IOCTL_MAGIC, 59 , DISP_PQ_PARAM)
#define DISP_IOCTL_GET_PQ_GAL_PARAM     _IOR    (DISP_IOCTL_MAGIC, 70 , DISP_PQ_PARAM)
typedef struct
{
    DISP_MODULE_ENUM module;
    unsigned int timeout_ms;  //timeout, unit is ms
} disp_wait_irq_struct;

#ifdef __KERNEL__
typedef void (*DDP_IRQ_CALLBACK)(unsigned int param);

//-------------------------------------------------------
// functions
//-------------------------------------------------------
int disp_wait_intr(DISP_MODULE_ENUM module, unsigned int timeout_ms);
int disp_dump_reg(DISP_MODULE_ENUM module);

int disp_set_overlay_roi(int layer, int x, int y, int w, int h, int pitch);
int disp_set_overlay_addr(int layer, unsigned int addr, DDP_OVL_FORMAT fmt);
int disp_set_overlay(int layer, int enable);
int disp_is_dp_framework_run(void);

int disp_set_mutex_status(int enable);
int disp_get_mutex_status(void);
int disp_register_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb);
int disp_unregister_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb);
int disp_set_needupdate(DISP_MODULE_ENUM eModule , unsigned long u4En);
void disp_power_off(DISP_MODULE_ENUM eModule , unsigned int * pu4Record);
void disp_power_on(DISP_MODULE_ENUM eModule , unsigned int * pu4Record);
void cmdq_ion_flush(void);

void disp_aal_lock(void);
void disp_aal_unlock(void);

void disp_m4u_dump_reg(void);
#endif

#endif
