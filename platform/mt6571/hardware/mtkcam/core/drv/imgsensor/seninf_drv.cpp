/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "SeninfDrvImp"
//
#include <fcntl.h>
#include <sys/mman.h>
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/threads.h>                  // For android::Mutex.
#else
#include <utils/Mutex.h>                    // For android::Mutex.
#endif
#include <cutils/atomic.h>
//
#include "drv_types.h"
#include <mtkcam/drv/isp_drv.h>
#include "camera_isp.h"
#include <mtkcam/drv/isp_reg.h>
#include "seninf_reg.h"
#include "seninf_drv_imp.h"
#include "mtkcam/hal/sensor_hal.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
//

#define GPIO_MODE_00 0
#define GPIO_MODE_01 1
#define GPIO_MODE_02 2
#define GPIO16       16

#define GPIO_CMPCLK        (GPIO16 | 0x80000000)
#define GPIO_CMCSK         (GPIO16 | 0x80000000)
#define GPIO_CMCSK_M_GPIO  GPIO_MODE_00
#define GPIO_CMCSK_M_CLK   GPIO_MODE_01
#define GPIO_CMCSK_M_CMCSK GPIO_MODE_02

/******************************************************************************
*
*******************************************************************************/
#define ISP_DEV_NAME     	"/dev/camera-isp"
#define SENSOR_DRV_NAME     "/dev/kd_camera_hw"

#define SCAM_ENABLE         1     // 1: enable SCAM feature. 0. disable SCAM feature.

#define CAM_MIPI_RX_CONFIG_BASE     0x14013000
#define CAM_MIPI_RX_CONFIG_RANGE    0x1000
#define CAM_MIPI_RX_ANALOG_BASE     0x10011000
#define CAM_MIPI_RX_ANALOG_RANGE    0x1000
#define CAM_IO_CFG_BASE             0x10014000
#define CAM_IO_CFG_RANGE            0x1000

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::createInstance()
{
    return SeninfDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
static SeninfDrvImp singleton;
SeninfDrv*
SeninfDrvImp::
getInstance()
{
    LOG_MSG("[getInstance] \n");
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
SeninfDrvImp::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::SeninfDrvImp() :
    SeninfDrv()
{
    LOG_MSG("[SeninfDrvImp] \n");

    mUsers = 0;
    mfd = 0;
    tg1GrabWidth = 0;
    tg1GrabHeight = 0;	
    m_fdSensor = -1;
    mMipiType = MIPI_OPHY_CSI2;

}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::~SeninfDrvImp()
{
    LOG_MSG("[~SeninfDrvImp] \n");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()
{
    LOG_MSG("[init]: %d \n", mUsers);
    MBOOL result;
    
    Mutex::Autolock lock(mLock);
    //
    if (mUsers > 0) {
        LOG_MSG("  Has inited \n");
        android_atomic_inc(&mUsers);
        return 0;
    }

    // Open isp driver
    mfd = open(ISP_DEV_NAME, O_RDWR);
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }

    //Open sensor driver
     m_fdSensor = open(SENSOR_DRV_NAME, O_RDWR);
    if (m_fdSensor < 0) {
        LOG_ERR("[init]: error opening  %s \n",  strerror(errno));                
        return -13;
    }

    // Access mpIspHwRegAddr
	m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv) {
        LOG_ERR("IspDrvImp::createInstance fail \n");
        return -2;
    }
    //
    result = m_pIspDrv->init();
    if ( MFALSE == result ) {
        LOG_ERR("pIspDrv->init() fail \n");
        return -3;
    }

	//get isp reg for TG module use
    mpIspHwRegAddr = (unsigned long*)m_pIspDrv->getRegAddr();
    if ( NULL == mpIspHwRegAddr ) {
        LOG_ERR("getRegAddr fail \n");
        return -4;
    }

    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned long *) mmap(0, SENINF_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, SENINF_BASE_HW);
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -5;
    }

    mpCAMIODrvRegAddr = (unsigned long *) mmap(0, CAM_IO_CFG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_IO_CFG_BASE);
    if(mpCAMIODrvRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err(2), %d, %s \n", errno, strerror(errno));
        return -6;
    }  

    // mipi rx config address
    mpCSI2RxConfigRegAddr = (unsigned long *) mmap(0, CAM_MIPI_RX_CONFIG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, CAM_MIPI_RX_CONFIG_BASE);
    if (mpCSI2RxConfigRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(4), %d, %s \n", errno, strerror(errno));
        return -8;
    }

    // mipi rx analog address
    mpCSI2RxAnalogRegStartAddr = (unsigned long *) mmap(0, CAM_MIPI_RX_ANALOG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_MIPI_RX_ANALOG_BASE);
    if (mpCSI2RxAnalogRegStartAddr == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s \n", errno, strerror(errno));
        return -9;
    }
    mpCSI2RxAnalogRegAddr = mpCSI2RxAnalogRegStartAddr + (0x800/4);

    {
        unsigned int temp = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        temp |= 0x80000000;
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);
    }

    android_atomic_inc(&mUsers);

    LOG_MSG("[init]: X \n");

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    LOG_MSG("[uninit]: %d \n", mUsers);
    MBOOL result;

    Mutex::Autolock lock(mLock);
    //
    if (mUsers <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);

    
    if (mUsers == 0) {
        // Last user
        setTg1CSI2(0, 0, 0, 0, 0, 0, 0, 0);   // disable CSI2
		setTg1PhaseCounter(0, 0, 0, 0, 0, 0, 0);
  
         //disable MIPI RX analog
        *(mpCSI2RxAnalogRegAddr + (0x24/4)) &= 0xFFFFFFFE;//RG_CSI_BG_CORE_EN
        *(mpCSI2RxAnalogRegAddr + (0x20/4)) &= 0xFFFFFFFE;//RG_CSI0_LDO_CORE_EN        
        *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFFE;//RG_CSI0_LNRC_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFFE;//RG_CSI0_LNRD0_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFFE;//RG_CSI0_LNRD1_LDO_OUT_EN

        // munmap rx analog address
        if ( 0 != mpCSI2RxAnalogRegStartAddr ) {
            munmap(mpCSI2RxAnalogRegStartAddr, CAM_MIPI_RX_ANALOG_RANGE);
            mpCSI2RxAnalogRegStartAddr = NULL;
        }

        // munmap rx config address
        if ( 0 != mpCSI2RxConfigRegAddr ) {
            munmap(mpCSI2RxConfigRegAddr, CAM_MIPI_RX_CONFIG_RANGE);
            mpCSI2RxConfigRegAddr = NULL;
        }

        // munmap IODrv reg 
        if ( 0 != mpCAMIODrvRegAddr ) {
            munmap(mpCAMIODrvRegAddr, CAM_IO_CFG_RANGE);
            mpCAMIODrvRegAddr = NULL;
        }

        // munmap seninf reg       
        if ( 0 != mpSeninfHwRegAddr ) {
            munmap(mpSeninfHwRegAddr, SENINF_BASE_RANGE);
            mpSeninfHwRegAddr = NULL;
        }

        // uninit isp
        mpIspHwRegAddr = NULL;        
        result = m_pIspDrv->uninit();
        if ( MFALSE == result ) {
            LOG_ERR("pIspDrv->uninit() fail \n");
            return -3;
        }
        
        //
        if (mfd > 0) {
            close(mfd);
            mfd = -1;
        }
        if (m_fdSensor > 0) {
            close(m_fdSensor);
            m_fdSensor = -1;
        }           
    }
    else {
        LOG_MSG("  Still users \n");
    }

    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::waitSeninf1Irq(int mode)
{
    int ret = 0;

    LOG_MSG("[waitIrq polling] 0x%x \n", mode);
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int sleepCount = 40;
    int sts;
    ret = -1;
    while (sleepCount-- > 0) {
        sts = SENINF_READ_REG(pSeninf, SENINF1_INTSTA);  // Not sure CTL_INT_STATUS or CTL_INT_EN
        if (sts & mode) {
            LOG_MSG("[waitIrq polling] Done: 0x%x \n", sts);
            ret = 0;
            break;
        }
        LOG_MSG("[waitIrq polling] Sleep... %d, 0x%x \n", sleepCount, sts);
        usleep(100 * 1000);
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1PhaseCounter(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    unsigned int temp = 0;
    bool clkfl_pol = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    
    LOG_MSG("[setTg1PhaseCounter]: pcEn=%d, mclkSel=%d, clkCnt=%d, clkPol=%d, clkFallEdge=%d, clkRiseEdge=%d, padPclkInv=%d\n",
            pcEn, mclkSel, clkCnt, clkPol, clkFallEdge, clkRiseEdge, padPclkInv);

    //MCLK setting
    ISP_PLL_SEL_STRUCT pllCtrl;
    if (mclkSel == CAM_PLL_48_GROUP) 
    {
        pllCtrl.MclkSel = MCLK_USING_UNIV_48M;  // 48MHz
    } 
    else if (mclkSel == CAM_PLL_52_GROUP) 
    {    
        pllCtrl.MclkSel = MCLK_USING_UNIV_208M; // 208MHz
    }
    ioctl(mfd, ISP_IOC_PLL_SEL_IRQ, &pllCtrl);

    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt         

    //Seninf Top pclk clear gating
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN, 1);
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN, 1);    

    temp = ((clkCnt&0x3F)<<16)|((clkRiseEdge&0x3F)<<8)|(clkFallEdge&0x3F);
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_SEN_CK, temp);

    clkfl_pol = (clkCnt & 0x1) ? 0 : 1;
    temp = SENINF_READ_REG(pSeninf, SENINF_TG1_PH_CNT);
    temp &= 0x4FFFFFB8;
    temp |= (((pcEn&0x1)<<31)|(0x20000000)|((clkPol&0x1)<<28)|((padPclkInv&0x1)<<6)|(clkfl_pol<<2)|(0x1));//force PLL due to ISP engine clock dynamic spread
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_PH_CNT, temp);    // mclkSel, 0: 122.88MHz, (others: Camera PLL) 1: 48MHz, 2: 208MHz
    
    temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
    ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x1);

    // Wait 1ms for PLL stable
    usleep(1000);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1GrabRange] \n");
    tg1GrabWidth = pixelEnd - pixelStart;
    tg1GrabHeight = lineEnd - lineStart;

    // TG Grab Win Setting
    ISP_WRITE_REG(pisp, CAM_TG_SEN_GRAB_PXL, ((pixelEnd&0x7FFF)<<16)|(pixelStart&0x7FFF));
    ISP_WRITE_REG(pisp, CAM_TG_SEN_GRAB_LIN, ((lineEnd&0x7FFF)<<16)|(lineStart&0x7FFF));
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1SensorModeCfg(
    unsigned long hsPol, unsigned long vsPol
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1SensorModeCfg] \n");

    // Sensor Mode Config
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, hsPol);
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, vsPol);

    //SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, VS_TYPE, vsPol);//VS_TYPE = 0 for 4T:vsPol(0)
    //if(SEN_CSI2_SETTING)
    //    SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_VSYNC_TYPE, !vsPol);//VSYNC_TYPE = 1 for 4T:vsPol(0)

    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN, 1);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOT_MODE, 1);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1InputCfg(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel,
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb
)
{
    int ret = 0;
    unsigned int temp = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1InputCfg] \n");
    LOG_MSG("inSrcTypeSel = % d \n",inSrcTypeSel);

    temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
    temp &= 0xFFF0FFF;
    if(inSrcTypeSel == MIPI_SENSOR ) {
        if(mMipiType == MIPI_OPHY_NCSI2) {
            LOG_MSG("mMipiType == MIPI_OPHY_NCSI2\n");
            temp |= ((0x80000000)|((padSel&0x7)<<28)|((0x8)<<12));
        }
        else {
            LOG_MSG("mMipiType == MIPI_OPHY_CSI2\n");
            temp |= ((0x80000000)|((padSel&0x7)<<28));    
        }
    }
    else {
        temp |= ((0x80000000)|((padSel&0x7)<<28)|((inSrcTypeSel&0xF)<<12));
    }
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);

    temp = ISP_READ_REG(pisp, CAM_TG_PATH_CFG);
    ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, temp&0xFFFFFFFC);//no matter what kind of format, set 0

    temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_CLR);
    ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_CLR, temp|0x70000);

    temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_SET);
    ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_SET, temp|((inDataType&0x7)<<16));


	if (MIPI_SENSOR == inSrcTypeSel) {
        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp&0xFFFFFCFF);//SOF_SRC = 0

		if (JPEG_FMT == inDataType) {
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            temp &= 0xF000FFFF;
            temp |= ((0x18<<22)|(0x1E<<16));
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);            

            temp = SENINF_READ_REG(pSeninf, SENINF1_SPARE);
            temp &= 0xFFFFC1FF;
            temp |= 0x2C00;//SENINF_FIFO_FULL_SEL=1, SENINF_VCNT_SEL=1, SENINF_CRC_SEL=2;
            SENINF_WRITE_REG(pSeninf, SENINF1_SPARE,temp); 
		}
		else {
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            temp &= 0xF000FFFF;
            temp |= ((0x1B<<22)|(0x1F<<16));
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);            

		}
	}
	else {
        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x100);//SOF_SRC = 0        

        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        temp &= 0xF000FFFF;
        temp |= ((0x1B<<22)|(0x1F<<16));
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);            
        
	}

	//One-pixel mode
	if ( JPEG_FMT != inDataType) {
        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        temp &= 0xFFFFFEFF;
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);         

        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp&0xFFFFFFFD);//DBL_DATA_BUS = 0   

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_CLR);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_CLR, temp|0x1000000);

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_SET);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_SET, temp&0xFEFFFFFF);

        temp = ISP_READ_REG(pisp, CAM_TG_PATH_CFG);
        ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, temp&0xFFFFFFEF);//JPGINF_EN=0


	}
	else {
        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp|0x100);         
        
        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x2);//DBL_DATA_BUS = 1   

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_CLR);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_CLR, temp|0x1000000);

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_SET);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_SET, temp|0x1000000);

		temp = ISP_READ_REG(pisp, CAM_TG_PATH_CFG);
        ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, temp|0x10);//JPGINF_EN=1

	}

    #if 0
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_IRQ_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_SW_RST, 0x0);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_IRQ_SW_RST, 0x0);
    #endif

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1ViewFinderMode(
    unsigned long spMode, unsigned long spDelay
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1ViewFinderMode] \n");
    //
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SPDELAY_MODE, 1);
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SINGLE_MODE, spMode);
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SP_DELAY, spDelay);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::sendCommand(int cmd, int arg1, int arg2, int arg3)
{
    int ret = 0;

    LOG_MSG("[sendCommand] cmd: 0x%x \n", cmd);
    switch (cmd) {
    case CMD_SET_DEVICE:
        mDevice = arg1;
        break;
        
    case CMD_SET_MIPI_TYPE:
#if 1
        mMipiType = MIPI_OPHY_CSI2;
        LOG_MSG("force to set mMipiType = MIPI_OPHY_CSI2\n");
#else
        mMipiType = arg1;
        LOG_MSG("[sendCommand] mMipiType = %d \n", mMipiType);
#endif        
        break;
        
    case CMD_GET_SENINF_ADDR:
        //LOG_MSG("  CMD_GET_ISP_ADDR: 0x%x \n", (int) mpIspHwRegAddr);
        *(int *) arg1 = (int) mpSeninfHwRegAddr;
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned long SeninfDrvImp::readReg(unsigned long addr)
{
    int ret;
    reg_t reg[2];
    int val = 0xFFFFFFFF;

    LOG_MSG("[readReg] addr: 0x%08x \n", (int) addr);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = readRegs(reg, 1);
    if (ret < 0) {
    }
    else {
        val = reg[0].val;
    }

    return val;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::writeReg(unsigned long addr, unsigned long val)
{
    int ret;
    reg_t reg[2];

    LOG_MSG("[writeReg] addr/val: 0x%08x/0x%08x \n", (int) addr, (int) val);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = writeRegs(reg, 1);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::readRegs(reg_t *pregs, int count)
{
    MBOOL result = MTRUE;
    result = m_pIspDrv->readRegs( (ISP_DRV_REG_IO_STRUCT*) pregs, count);
    if ( MFALSE == result ) {
        LOG_ERR("MT_ISP_IOC_G_READ_REG err \n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::writeRegs(reg_t *pregs, int count)
{
    MBOOL result = MTRUE;
    result = m_pIspDrv->writeRegs( (ISP_DRV_REG_IO_STRUCT*) pregs, count);
    if ( MFALSE == result ) {
        LOG_ERR("MT_ISP_IOC_S_WRITE_REG err \n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::holdReg(bool isHold)
{
    int ret;
    int hold = isHold;

    //LOG_MSG("[holdReg]");

    ret = ioctl(mfd, ISP_HOLD_REG, &hold);
    if (ret < 0) {
        LOG_ERR("ISP_HOLD_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::dumpReg()
{
    int ret;

    LOG_MSG("[dumpReg] \n");

    ret = ioctl(mfd, ISP_DUMP_REG, NULL);
    if (ret < 0) {
        LOG_ERR("ISP_DUMP_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::initTg1CSI2(bool csi2_en)
{
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int ret = 0;
    unsigned int temp = 0;

    LOG_MSG("[initCSI2]:enable = %d\n", (int) csi2_en);

	if(csi2_en == 0) 
	{
        //disable mipi BG
        *(mpCSI2RxAnalogRegAddr + (0x24/4)) &= 0xFFFFFFFE;//RG_CSI_BG_CORE_EN

        //switch to CMPLK
    	ISP_GPIO_SEL_STRUCT gpioCtrl;
        gpioCtrl.Pin = GPIO_CMPCLK;	    
        gpioCtrl.Mode = GPIO_CMCSK_M_CLK;
        ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);

        // disable mipi input select
        *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFE7;//main & sub clock lane input select hi-Z
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFE7;//main & sub data lane 0 input select hi-Z
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFE7;//main & sub data lane 1 input select hi-Z
	}
	else 
    {   // enable mipi input select              
        if(mDevice & SENSOR_DEV_MAIN) 
        {
            //enable main camera mipi lane
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) |= 0x00000008;//main clock lane input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) |= 0x00000008;//main data lane 0 input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) |= 0x00000008;//main data lane 1 input select mipi            
            
            //disable sub camera mipi lane
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFEF;//sub clock lane input select mipi disable     
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFEF;//sub data lane 0 input select mipi disable    
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFEF;//sub data lane 1 input select mipi disable                         

            temp = *(mpCSI2RxConfigRegAddr + (0x24/4));//       
            mt65xx_reg_writel(((temp&0xFFFFFF)|0xE4000000), mpCSI2RxConfigRegAddr + (0x24/4));             
        }
        else if(mDevice & SENSOR_DEV_SUB)
        {
            //enable sub camera mipi lane
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) |= 0x00000010;//sub clock lane input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) |= 0x00000010;//sub data lane 0 input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) |= 0x00000010;//sub data lane 1 input select mipi            
            
            //disable main camera mipi lane
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFF7;//main clock lane input select mipi disable     
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFF7;//main data lane 0 input select mipi disable    
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFF7;//main data lane 1 input select mipi disable  

            temp = *(mpCSI2RxConfigRegAddr + (0x24/4));
            mt65xx_reg_writel(((temp&0xFFFFFF)|0x1B000000), mpCSI2RxConfigRegAddr + (0x24/4));
        }

        // enable mipi BG
        *(mpCSI2RxAnalogRegAddr + (0x24/4)) |= 0x00000003;//RG_CSI_BG_CORE_EN
        usleep(30);
        *(mpCSI2RxAnalogRegAddr + (0x20/4)) |= 0x00000001;//RG_CSI0_LDO_CORE_EN
        usleep(1);
        *(mpCSI2RxAnalogRegAddr + (0x00/4)) |= 0x00000001;//RG_CSI0_LNRC_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) |= 0x00000001;//RG_CSI0_LNRD0_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) |= 0x00000001;//RG_CSI0_LNRD1_LDO_OUT_EN

#ifndef FPGA_EARLY_PORTING
        // CSI Offset calibration 
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LNC_HSRXDB_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN0_HSRXDB_EN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN1_HSRXDB_EN, 1);
        
        *(mpCSI2RxConfigRegAddr + (0x24/4)) |= 0x00080000;//MIPI_RX_HW_CAL_START       
        
        *(mpCSI2RxConfigRegAddr + (0x38/4)) |= 0x1; //MIPI_RX_SW_CTRL_MODE 
       
        *(mpCSI2RxConfigRegAddr + (0x3C/4)) = 0x1541; //bit 0,6,8,10,12
        
        *(mpCSI2RxConfigRegAddr + (0x38/4)) |= 0x00000004;//MIPI_RX_HW_CAL_START
        
        LOG_MSG("[initCSI2]:CSI0 calibration start !\n");
        usleep(500); 
        //JH test mark
        if(!((*(mpCSI2RxConfigRegAddr + (0x44/4)) & 0x10101) && (*(mpCSI2RxConfigRegAddr + (0x48/4)) & 0x101))){
            LOG_ERR("[initCSI2]:CSI0 calibration failed!, CSI2Config Reg 0x44=0x%x, 0x48=0x%x\n",*(mpCSI2RxConfigRegAddr + (0x44/4)),*(mpCSI2RxConfigRegAddr + (0x48/4)));      
        }         
        LOG_MSG("[initCSI2]:CSI0 calibration end !\n");
        
        *(mpCSI2RxConfigRegAddr + (0x38/4)) &= 0xFFFFFFFE; //MIPI_RX_SW_CTRL_MODE        
       
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LNC_HSRXDB_EN, 0);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN0_HSRXDB_EN, 0);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN1_HSRXDB_EN, 0);
#endif

        *(mpCSI2RxAnalogRegAddr + (0x20/4)) &= 0xFFFFFFDF;//bit 5:RG_CSI0_4XCLK_INVERT = 0
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFBFFFFF;//bit 22:RG_CSI0_LNRD0_HSRX_BYPASS_SYNC = 0
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFBFFFFF;//bit 22:RG_CSI0_LNRD1_HSRX_BYPASS_SYNC = 0
       
        if(mMipiType != MIPI_OPHY_NCSI2)
        {
			LOG_MSG(" CSI2\n");
            *(mpCSI2RxAnalogRegAddr + (0x20/4)) &= 0xFFFFFFBF;//bit 6:RG_CSI0_4XCLK_DISABLE = 0
            SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_LNMUX,0xE4);         
        }
        else
        {
			LOG_MSG(" nCSI2\n");
            *(mpCSI2RxAnalogRegAddr + (0x20/4)) |= 0x00000040;//bit 6:RG_CSI0_4XCLK_DISABLE = 1
        }       
        *(mpCSI2RxAnalogRegAddr + (0x20/4)) |= 0x00000002;//bit 1:RG_CSI0_LNRD_HSRX_BCLK_INVERT = 1       
	}    

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int SETTLE_TEST = 0x00001A00;
bool HW_MODE = 0;
int SeninfDrvImp::setTg1CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlane_num, unsigned long csi2_en,
    unsigned long dataheaderOrder, unsigned long dataFlow
)
{
    int ret = 0;
    unsigned int temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[configTg1CSI2]:DataTermDelay:%d SettleDelay:%d ClkTermDelay:%d VsyncType:%d dlane_num:%d CSI2 enable:%d HeaderOrder:%d DataFlow:%d\n",
            	(int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlane_num, (int) csi2_en, (int)dataheaderOrder, (int)dataFlow);

    if(mMipiType == MIPI_OPHY_NCSI2) 
    {
        if(csi2_en == 1) 
        {
            // disable NCSI2 first     
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_LANE_EN, 0);              
    
            // add src select earlier, for countinuous MIPI clk issue
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, 0x8);
        
            #ifdef FPGA_EARLY_PORTING
            //temp = (dataSettleDelay&0xFF)<<8;
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);        
            //SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_LNRD_TIMING, SETTLE_PARAMETER, (dataSettleDelay&0xFF));
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, 0x00000300);       
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, ED_SEL, 1);
            #else
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, 0x00003000);//set 0x30 for settle delay               
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_SPARE0, 0x80000000); 
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, ED_SEL, dataheaderOrder); 
            #endif
    		
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, 0); //mipi hsync must be zero.
            //VS_TYPE = 0 for 4T:vsPol(0)
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, VS_TYPE, 0);
    
            //Use sw settle mode
            if(HW_MODE)        
            {
               SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, HSRX_DET_EN, 1);  
            }
            else
            {
               SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, HSRX_DET_EN, 0);  
               SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, SETTLE_TEST);        
            }
            
            // enable NCSI2
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_LANE_EN, csi2_en);   
            switch(dlane_num)
            {
                case 0://SENSOR_MIPI_1_LANE
                    SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE0_EN, 1);               
                    break;
                case 1://SENSOR_MIPI_2_LANE
                    SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE0_EN, 1);               
                    SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE1_EN, 1);               
                    break;
                default:
                    break;                
            }                            
            // turn on all interrupt
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_EN, 0x80007FFF);
        }
        else 
        {
            // disable NCSI2
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_LANE_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE0_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE1_EN, 0);
        }
    }
    else 
    {
        if(csi2_en == 1) 
        {
            // disable CSI2 first     
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, 0);   
    
            // add src select earlier, for countinuous MIPI clk issue
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, 0x0);

            temp = (dataSettleDelay&0xFF)<<16;
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DELAY, temp);//set 0x30 for settle delay 
                        
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_ED_SEL, dataheaderOrder); 

            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, 0); //mipi hsync must be zero.
            //VSYNC_TYPE = 1 for 4T:vsPol(0)
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_VSYNC_TYPE, 1);

            // enable CSI2            
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, csi2_en);   
            switch(dlane_num)
            {                
                case 1://SENSOR_MIPI_2_LANE
                    SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, DLANE1_EN, 1);                   
                    break;
                    case 0://SENSOR_MIPI_1_LANE                           
                default:
                    break;                
            }           
        }
        else
        {
            // disable CSI2        
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, DLANE1_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, DLANE2_EN, 0);
        }
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::initTg1Serial(bool serial_en)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SCAM1_CON, Enable, serial_en);

    return ret;
}
 
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1Serial(
    unsigned long clk_inv, unsigned long width, unsigned long height,
    unsigned long conti_mode, unsigned long csd_num)        
{
    int ret = 0;    
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    
    //switch to CMCSK
    ISP_GPIO_SEL_STRUCT gpioCtrl;
    gpioCtrl.Pin = GPIO_CMCSK;	    
    gpioCtrl.Mode = GPIO_CMCSK_M_CMCSK;
    ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);
        
    SENINF_WRITE_BITS(pSeninf, SCAM1_SIZE, WIDTH, width);
    SENINF_WRITE_BITS(pSeninf, SCAM1_SIZE, HEIGHT, height);    

    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Clock_inverse, clk_inv);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Continuous_mode, conti_mode);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, CSD_NUM, csd_num);  // 0(1-lane);1(2-line);2(4-lane)
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Cycle, 4);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG2, DIS_GATED_CLK, 1);    

    return ret;
}

int SeninfDrvImp::resetSeninf()
{
    int ret = 0;
    unsigned int temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);	
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, (temp|0x3));    
    usleep(10);
    LOG_MSG("[resetSeninf] reset done\n");
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, (temp&0xFFFFFFFC));    
    
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
/*
  0x10014110[5:4]
    00: 2mA
    01: 4mA
    10: 6mA
    11: 8mA
*/
int SeninfDrvImp::setTg1IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    unsigned long* pCAMIODrvCtl; 
    
    if(mpCAMIODrvRegAddr != NULL) 
    {        
        pCAMIODrvCtl = mpCAMIODrvRegAddr + (0x110/4);
        *(pCAMIODrvCtl) &= 0xFFFFFFCF;
        
        switch(ioDrivingCurrent)
        {
            case ISP_DRIVING_2MA:
                *(pCAMIODrvCtl) |= (0x0<<4); 
                break;
            case ISP_DRIVING_4MA:
                *(pCAMIODrvCtl) |= (0x1<<4); 
                break;
            case ISP_DRIVING_6MA:
                *(pCAMIODrvCtl) |= (0x2<<4);                 
                break;
            case ISP_DRIVING_8MA:             
                *(pCAMIODrvCtl) |= (0x3<<4);                 
                break;
            default:   
                *(pCAMIODrvCtl) |= (0x1<<4); 
                break;
        }
    }
    
    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(pCAMIODrvCtl)));

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1MCLKEn(bool isEn)
{
    int ret = 0;

	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN, isEn);

    return ret;
}



int SeninfDrvImp::setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount,
			unsigned long startLine, unsigned long startPixel, unsigned long  flashPol)
{
    int ret = 0;
    unsigned int temp = 0;    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;


    temp = ISP_READ_REG(pisp, CAM_TG_FLASHA_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_CTL, temp&0xFFFFFFFE);//disable flasha_en    

    temp &= 0x2;
    temp |= ((flashPol&0x1)<<12)|((endFrame&0x7)<<8)|((startPoint&0x3)<<4);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_CTL, temp);

    temp = ((lineUnit&0xF)<<20)|(unitCount&0xFFFFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_LINE_CNT, temp);    

    temp = ((startLine&0x1FFF)<<16)|(startPixel&0x7FFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_POS, temp);    

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHB_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp|0x1);//enable flasha_en


	return ret;

}


int SeninfDrvImp::setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel)
{
    int ret = 0;
    unsigned int temp = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHB_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp&0xFFFFFFFE);//disable flashb_en

    temp = ((contiFrm&0x7)<<12)|((startFrame&0xF)<<8);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp);//disable flashb_en    

    temp = ((lineUnit&0xF)<<20)|(unitCount&0xFFFFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_LINE_CNT, temp);

    temp = ((startLine&0x1FFF)<<16)|(startPixel&0x7FFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_POS, temp);

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHB_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp|0x1);//enable flashb_en

	return ret;
}

int SeninfDrvImp::setFlashEn(bool flashEn)
{
	int ret = 0;
    unsigned int temp = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHA_CTL);
    temp &= 0xFFFFFFFD;
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_CTL, temp|(flashEn<<1));

	return ret;

}



int SeninfDrvImp::setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd)
{
    int ret = 0;
    unsigned int temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	if ((hsStart > 4095) || (hsEnd > 4095))
	{
		LOG_ERR("CCIR656 HSTART or HEND value err \n");
		ret = -1;
	}
    temp = SENINF_READ_REG(pSeninf,CCIR656_CTL);
    temp &= 0xFFFFFFF3;
    temp |= (((vsPol&0x1)<<3)|((hsPol&0x1)<<1));
    SENINF_WRITE_REG(pSeninf,CCIR656_CTL, temp);

    temp = ((hsEnd&0xFFF)<<16)|(hsStart&0xFFF);
    SENINF_WRITE_REG(pSeninf, CCIR656_H, temp);

	return ret;
}

int SeninfDrvImp::checkSeninf1Input()
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;          

    temp = SENINF_READ_REG(pSeninf,SENINF1_DEBUG_4);
    LOG_MSG("[checkSeninf1Input]:size = 0x%x",temp);        
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;
    
    if( (tempW >= tg1GrabWidth) && (tempH >= tg1GrabHeight)  ) {
        ret = 0;
    }
    else {           
        ret = 1;
    }

    return ret;

}


int SeninfDrvImp::autoDeskewCalibration()
{
    int ret = 0;
    unsigned int temp = 0;
    MUINT32 lane_num=0,min_lane_code=0;
    MUINT8 lane0_code=0,lane1_code=0,lane2_code=0,lane3_code=0,i=0;
    MUINT8 clk_code=0;
    MUINT32 currPacket = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    
    LOG_MSG("autoDeskewCalibration start \n");

    temp = *(mpCSI2RxAnalogRegAddr + (0x00));//disable clock lane delay
    mt65xx_reg_writel(temp&0xFFEFFFFF, mpCSI2RxAnalogRegAddr + (0x00));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//disable data lane 0 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//disable data lane 2 delay         
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//disable data lane 3 delay       
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));


    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DGB_SEL,0x12);//set debug port to output packet number
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_EN,0x80007FFF);//set interrupt enable //@write clear?

    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration 
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line %d, default input has error, please check it \n",__LINE__);
    }
    LOG_MSG("autoDeskewCalibration start interupt status = 0x%x\n",SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS));
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
    
    //Fix Clock lane
    //set lane 0
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//enable data lane 0 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x04/4));     

    for(i=0; i<=0xF; i++) {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        //@add read interrupt status to clear
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));    
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x04/4));   
        //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//disable data lane 0 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));    
    usleep(5);
    
    //set lane 1
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//enable data lane 1 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x08/4));      

    for(i=0; i<=0xF; i++) {
        lane1_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));    
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x08/4));         
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//disable data lane 1 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));      
    usleep(5);
    
    //set lane 2
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//enable data lane 2 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x0C/4));      
  
    for(i=0; i<=0xF; i++) {
        lane2_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));    
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x0C/4));           
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//disable data lane 2 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));     
    usleep(5);

    //set lane 3    
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//enable data lane 3 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x10/4));      

    for(i=0; i<=0xF; i++) {
        lane3_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));    
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x10/4));                   
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//disable data lane 3 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));     
  
  LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d \n",lane0_code,lane1_code,lane2_code,lane3_code);

    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }
    if(min_lane_code > lane2_code) {
        min_lane_code = lane2_code;
    }
    if(min_lane_code > lane3_code) {
        min_lane_code = lane3_code;
    }
    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d \n",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code);


    //Fix Data lane
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));    
    mt65xx_reg_writel(temp|(((lane0_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x04/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));    
    mt65xx_reg_writel(temp|(((lane1_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x08/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));   
    mt65xx_reg_writel(temp|(((lane2_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x0C/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));   
    mt65xx_reg_writel(temp|(((lane3_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x10/4));      
    temp = *(mpCSI2RxAnalogRegAddr);//enable clock lane delay    
    mt65xx_reg_writel(temp|0x100000, mpCSI2RxAnalogRegAddr );
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//enable data lane 0 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x04/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//enable data lane 1 dela   
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x08/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//enable data lane 2 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x0C/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//enable data lane 3 dela    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x10/4)); 

    LOG_MSG("autoDeskewCalibration start test 5 \n");    

    for(i=0; i<=0xF; i++) {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr);//set to 0 first    
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalogRegAddr );
        temp = *(mpCSI2RxAnalogRegAddr);   
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalogRegAddr );
        while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         //usleep(5);
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);
        lane2_code = lane2_code -((min_lane_code+clk_code)>>1);
        lane3_code = lane3_code -((min_lane_code+clk_code)>>1); 
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));          
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x04/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x08/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x0C/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x10/4));          
        temp = *(mpCSI2RxAnalogRegAddr);//clk code = 0   
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalogRegAddr );
    }
    else {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        temp = *(mpCSI2RxAnalogRegAddr);//set to 0 first   
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalogRegAddr );
        temp = *(mpCSI2RxAnalogRegAddr); 
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalogRegAddr );               
    }
    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d\n",clk_code,min_lane_code);


    
    LOG_MSG("autoDeskewCalibration end \n");
    return ret;
}
