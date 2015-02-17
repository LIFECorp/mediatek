#include <platform/mt_reg_base.h>
#include <platform/mt_clkmgr.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>
#include <platform/sec_devinfo.h>

#include "disp_drv_log.h"


#define DISP_INT_MUTEX_BIT_MASK 0x00000002

#define DDP_MMSYS_CG0       ( DISP_PQ_SW_CG_BIT \
                            | DISP_BLS_SW_CG_BIT \
                            | DISP_WDMA_SW_CG_BIT \
                            | DISP_RDMA_SW_CG_BIT \
                            | DISP_OVL_SW_CG_BIT \
                            | DISP_PWM_SW_CG_BIT \
                            | DISP_PWM_26M_SW_CG_BIT \
                            | MUTEX_SLOW_CLOCK_SW_CG_BIT )

#define SMI_REG_SMI_LARB_OSTD_CTRL_EN             (SMI_LARB0_BASE + 0x064)

unsigned int gMutexID = 0;

int disp_path_get_mutex(void)
{
    int cnt = 0;
   
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);        
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 1);

    while ((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID)) & DISP_INT_MUTEX_BIT_MASK) != DISP_INT_MUTEX_BIT_MASK)
    {
        if (cnt > 2000)
        {
            printf("[DDP] error! disp_path_get_mutex(), get mutex timeout!\n");
            disp_dump_reg(DISP_MODULE_MUTEX);
            disp_dump_reg(DISP_MODULE_CONFIG);
            return - 1;
        }
        mdelay(1);
        cnt ++;
    }

    return 0;
}

int disp_path_release_mutex(void)
{
    unsigned int reg = 0;
    unsigned int cnt = 0;

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 0);

    while ((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID)) & DISP_INT_MUTEX_BIT_MASK) != 0)
    {
        if (cnt > 2000)
        {
            break;
        }
        mdelay(1);
        cnt ++;
    }

    if (cnt > 2000)
    {
        if ((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(gMutexID+8))) == (unsigned int)(1<<(gMutexID+8)))
        {
            printf("[DDP] error! disp_path_release_mutex(), release mutex timeout! \n");
            disp_dump_reg(DISP_MODULE_CONFIG);
            //print error engine
            reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
            if (reg != 0)
            {
                if (reg & DDP_MOD_DISP_OVL) { printf(" OVL update reg timeout! \n"); disp_dump_reg(DISP_MODULE_OVL); }
                if (reg & DDP_MOD_DISP_WDMA) { printf(" WDMA update reg timeout! \n"); disp_dump_reg(DISP_MODULE_WDMA0); }
                if (reg & DDP_MOD_DISP_PQ) { printf(" PQ update reg timeout! \n"); disp_dump_reg(DISP_MODULE_PQ); }
                if (reg & DDP_MOD_DISP_BLS) { printf(" BLS update reg timeout! \n"); disp_dump_reg(DISP_MODULE_BLS); }
                if (reg & DDP_MOD_DISP_RDMA) { printf(" RDMA update reg timeout! \n"); disp_dump_reg(DISP_MODULE_RDMA0); }
            }

            disp_dump_reg(DISP_MODULE_MUTEX);
            disp_dump_reg(DISP_MODULE_CONFIG);
            return - 1;
        }
    }

    return 0;
}



int disp_path_config_layer(OVL_CONFIG_STRUCT* pOvlConfig)
{
   //    unsigned int reg_addr;

   DISP_LOG("[DDP]disp_path_config_layer(), layer=%d, source=%d, fmt=%d, addr=0x%x, x=%d, y=%d \n\
             w=%d, h=%d, pitch=%d, keyEn=%d, key=%d, aen=%d, alpha=%d \n ", 
             pOvlConfig->layer,   // layer
             pOvlConfig->source,   // data source (0=memory)
             pOvlConfig->fmt, 
             pOvlConfig->addr, // addr 
             pOvlConfig->x,  // x
             pOvlConfig->y,  // y
             pOvlConfig->w, // width
             pOvlConfig->h, // height
             pOvlConfig->pitch, //pitch, pixel number
             pOvlConfig->keyEn,  //color key
             pOvlConfig->key,  //color key
             pOvlConfig->aen, // alpha enable
             pOvlConfig->alpha);	

   // config overlay
   OVLLayerSwitch(pOvlConfig->layer, pOvlConfig->layer_en);

   if(pOvlConfig->layer_en!=0)
   {
      OVLLayerConfig(pOvlConfig->layer,   // layer
                                 pOvlConfig->source,   // data source (0=memory)
                                 pOvlConfig->fmt, 
                                 pOvlConfig->addr, // addr 
                                 pOvlConfig->x,  // x
                                 pOvlConfig->y,  // y
                                 pOvlConfig->w, // width
                                 pOvlConfig->h, // height
                                 pOvlConfig->pitch, //pitch, pixel number
                                 pOvlConfig->keyEn,  //color key
                                 pOvlConfig->key,  //color key
                                 pOvlConfig->aen, // alpha enable
                                 pOvlConfig->alpha); // alpha
   }    

   //    printf("[DDP]disp_path_config_layer() done, addr=0x%x \n", pOvlConfig->addr);
   
   return 0;
}

int disp_path_config_layer_addr(unsigned int layer, unsigned int addr)
{
   unsigned int reg_addr = 0;
   

   DISP_LOG("[DDP]disp_path_config_layer_addr(), layer=%d, addr=0x%x\n ", layer, addr);
   
   switch(layer)
   {
      case 0:
         DISP_REG_SET(DISP_REG_OVL_L0_ADDR, addr);
         reg_addr = DISP_REG_OVL_L0_ADDR;
         break;
      case 1:
         DISP_REG_SET(DISP_REG_OVL_L1_ADDR, addr);
         reg_addr = DISP_REG_OVL_L1_ADDR;
         break;
      case 2:
         DISP_REG_SET(DISP_REG_OVL_L2_ADDR, addr);
         reg_addr = DISP_REG_OVL_L2_ADDR;
         break;
      case 3:
         DISP_REG_SET(DISP_REG_OVL_L3_ADDR, addr);
         reg_addr = DISP_REG_OVL_L3_ADDR;
         break;
      default:
         printf("[DDP] error! error: unknow layer=%d \n", layer);
         ASSERT(0);
   }
   DISP_LOG("[DDP]disp_path_config_layer_addr() done, addr=0x%x \n", DISP_REG_GET(reg_addr));
   
   return 0;
}

int disp_path_ddp_clock_on()
{
    DISP_REG_SET(CLR_CLK_GATING_CTRL0, PWM_MM_SW_CG_BIT);
    DISP_REG_SET(DISP_REG_CONFIG_MMSYS_CG_CLR0, DDP_MMSYS_CG0);
    DISP_REG_SET(SMI_REG_SMI_LARB_OSTD_CTRL_EN, 0xFFFFF);

    DISP_LOG("[DISP] - disp_path_ddp_clock_on 0. 0x%8x\n", INREG32(DISP_REG_CONFIG_MMSYS_CG_CON0));
    return 0;	
}

int disp_path_ddp_clock_off()
{
    DISP_REG_SET(DISP_REG_CONFIG_MMSYS_CG_SET0, DDP_MMSYS_CG0);

    DISP_LOG("[DISP] - disp_path_ddp_clock_off 0. 0x%8x\n", INREG32(DISP_REG_CONFIG_MMSYS_CG_CON0));
    return 0;	
}


int disp_path_config(struct disp_path_config_struct* pConfig)
{
   ///> get mutex and set mout/sel
   //        unsigned int gMutexID = 0;
   unsigned int mutex_mode  = 0;
   

   DISP_LOG("[DDP]disp_path_config(), srcModule=%d, addr=0x%x, inFormat=%d, \n\
             pitch=%d, bgROI(%d,%d,%d,%d), bgColor=%d, outFormat=%d, dstModule=%d, dstAddr=0x%x,  \n",
             pConfig->srcModule,            
             pConfig->addr,  
             pConfig->inFormat,  
             pConfig->pitch, 
             pConfig->bgROI.x, 
             pConfig->bgROI.y, 
             pConfig->bgROI.width, 
             pConfig->bgROI.height, 
             pConfig->bgColor, 
             pConfig->outFormat,  
             pConfig->dstModule, 
             pConfig->dstAddr);

   if (pConfig->srcModule==DISP_MODULE_RDMA0 && pConfig->dstModule==DISP_MODULE_WDMA0)
   {
      printf("[DDP] error! rdma0 wdma1 can not enable together! \n");
      return -1;
   }

   
   switch(pConfig->dstModule)
   {
      case DISP_MODULE_DSI_VDO:
         mutex_mode = 1;
         break;
      
      case DISP_MODULE_DPI0:
         mutex_mode = 2;
         break;
      
      case DISP_MODULE_DBI:
      case DISP_MODULE_DSI_CMD:
      case DISP_MODULE_WDMA0:
         mutex_mode = 0;
         break;
      
      default:
         printf("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
         ASSERT(0);
   }
   
   
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 1);
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 0);

   if (pConfig->srcModule == DISP_MODULE_RDMA0)
   {
      DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_RDMA | DDP_MOD_DISP_PQ | DDP_MOD_DISP_BLS | DDP_MOD_DISP_PWM);
   }
   else
   {
      if (pConfig->dstModule == DISP_MODULE_WDMA0)
      {

         DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_WDMA);
      }
      else
      {
         DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_RDMA | DDP_MOD_DISP_PQ | DDP_MOD_DISP_BLS | DDP_MOD_DISP_PWM);
      }
   }		
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), mutex_mode);
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, (1 << gMutexID));
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN, (1 << gMutexID));        
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);        
   
   ///> config config reg
   switch(pConfig->dstModule)
   {
      case DISP_MODULE_DSI_VDO:
      case DISP_MODULE_DSI_CMD:
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
         DISP_REG_SET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL, 0x0);  // display output, 0: DSI, 1: DPI, 2: DBI
         DISP_REG_SET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL, 0x0);  // RDMA output, 0: DISP_PQ, 1: DSI, 2: DPI
         DISP_REG_SET(DISP_REG_CONFIG_DISP_PQ_SEL, 0x0);  // PQ input, 0: DISP_RDMA, 1: DISP_OVL
         DISP_REG_SET(DISP_REG_CONFIG_DISP_DSI_SEL, 0x0);  // DSI input, 0: DISP_BLS, 1: DISP_RDMA
         break;
      
      case DISP_MODULE_DPI0:
         DISP_LOG("DISI_MODULE_DPI0\n");
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
         DISP_REG_SET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL, 0x1);  // display output, 0: DSI, 1: DPI, 2: DBI
         DISP_REG_SET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL, 0x0);  // RDMA output, 0: DISP_PQ, 1: DSI, 2: DPI
         DISP_REG_SET(DISP_REG_CONFIG_DISP_DPI_SEL, 0);  // DPI input, 0: DISP_BLS, 1: DISP_RDMA
         DISP_REG_SET(DISP_REG_CONFIG_DISP_PQ_SEL, 0x0);  // PQ input, 0: DISP_RDMA, 1: DISP_OVL
         break;
      
      case DISP_MODULE_DBI:
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
         DISP_REG_SET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL, 0x2);  // display output, 0: DSI, 1: DPI, 2: DBI
         DISP_REG_SET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL, 0x0);  // RDMA output, 0: DISP_PQ, 1: DSI, 2: DPI
         DISP_REG_SET(DISP_REG_CONFIG_DISP_PQ_SEL, 0x0);  // PQ input, 0: DISP_RDMA, 1: DISP_OVL
         break;

      case DISP_MODULE_WDMA0:
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, 0: DISP_RDMA, 1: DISP_WDMA, 2: DISP_PQ
         break;
      
      default:
         printf("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
   }    
   
   ///> config engines
   if(pConfig->srcModule!=DISP_MODULE_RDMA0)
   {            // config OVL
      OVLStop();
      OVLROI(pConfig->bgROI.width, // width
                   pConfig->bgROI.height, // height
                   pConfig->bgColor);// background B
      
      OVLLayerSwitch(pConfig->ovl_config.layer, pConfig->ovl_config.layer_en);
      if(pConfig->ovl_config.layer_en!=0)
      {
         OVLLayerConfig(pConfig->ovl_config.layer,   // layer
                                    pConfig->ovl_config.source,   // data source (0=memory)
                                    pConfig->ovl_config.fmt, 
                                    pConfig->ovl_config.addr, // addr 
                                    pConfig->ovl_config.x,  // x
                                    pConfig->ovl_config.y,  // y
                                    pConfig->ovl_config.w, // width
                                    pConfig->ovl_config.h, // height
                                    pConfig->ovl_config.pitch,
                                    pConfig->ovl_config.keyEn,  //color key
                                    pConfig->ovl_config.key,  //color key
                                    pConfig->ovl_config.aen, // alpha enable
                                    pConfig->ovl_config.alpha); // alpha
      }
      
      OVLStart();

      if (pConfig->dstModule==DISP_MODULE_WDMA0)
      {
         WDMAReset(1);
         WDMAConfig(1, 
                              WDMA_INPUT_FORMAT_ARGB, 
                              pConfig->srcROI.width, 
                              pConfig->srcROI.height, 
                              0, 
                              0, 
                              pConfig->srcROI.width, 
                              pConfig->srcROI.height, 
                              pConfig->outFormat, 
                              pConfig->dstAddr, 
                              pConfig->srcROI.width, 
                              1, 
                              0);      
         WDMAStart(1);
      }
      else    //2. ovl->bls->rdma0->lcd
      {
                disp_bls_init(pConfig->srcROI.width, pConfig->srcROI.height);
         
         ///config RDMA
         RDMAStop(0);
         RDMAReset(0);
         RDMAConfig(0, 
                             RDMA_MODE_DIRECT_LINK,       ///direct link mode
                             RDMA_INPUT_FORMAT_RGB888,    // inputFormat
                             (unsigned int)NULL,                        // address
                             pConfig->outFormat,          // output format
                             pConfig->pitch,              // pitch
                             pConfig->srcROI.width,       // width
                             pConfig->srcROI.height,      // height
                             0,                           //byte swap
                             0);                          // is RGB swap        
         
         RDMAStart(0);
      }
   }
   else  //3. mem->rdma->lcd
   {
      ///config RDMA
      RDMAStop(0);
      RDMAReset(0);
      RDMAConfig(0, 
                          RDMA_MODE_MEMORY,       ///direct link mode
                          pConfig->inFormat,      // inputFormat
                          pConfig->addr,          // address
                          pConfig->outFormat,     // output format
                          pConfig->pitch,          //                                         
                          pConfig->srcROI.width,
                          pConfig->srcROI.height,
                          0,                       //byte swap    
                          0);                      // is RGB swap          
      RDMAStart(0);
   }

   printf("DDP Config = 0x%08X\n", get_devinfo_with_index(16));

   return 0;
}

unsigned int dbg_log = 1;
unsigned int irq_log = 0;
#define DISP_WRN(string, args...) if(dbg_log) printf("[DSS]"string,##args)
#define DISP_MSG(string, args...) printf("[DSS]"string,##args)
#define DISP_ERR(string, args...) printf("[DSS]error:"string,##args)
#define DISP_IRQ(string, args...) if(irq_log) printf("[DSS]"string,##args)

int disp_dump_reg(DISP_MODULE_ENUM module)
{
    switch (module)
    {
        case DISP_MODULE_CONFIG :
            DISP_MSG("===== DISP CFG Reg Dump: ============\n");
            DISP_MSG("(0x01C)DISP_REG_CONFIG_CAM_MDP_MOUT_EN          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_CAM_MDP_MOUT_EN));
            DISP_MSG("(0x020)DISP_REG_CONFIG_MDP_RDMA_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RDMA_MOUT_EN));
            DISP_MSG("(0x024)DISP_REG_CONFIG_MDP_RSZ0_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ0_MOUT_EN));
            DISP_MSG("(0x028)DISP_REG_CONFIG_MDP_RSZ1_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ1_MOUT_EN));
            DISP_MSG("(0x02C)DISP_REG_CONFIG_MDP_SHP_MOUT_EN          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_SHP_MOUT_EN));
            DISP_MSG("(0x030)DISP_REG_CONFIG_DISP_OVL_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN));
            DISP_MSG("(0x034)DISP_REG_CONFIG_MMSYS_MOUT_RST           = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MOUT_RST));
            DISP_MSG("(0x038)DISP_REG_CONFIG_MDP_RSZ0_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ0_SEL));
            DISP_MSG("(0x03C)DISP_REG_CONFIG_MDP_RSZ1_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ1_SEL));
            DISP_MSG("(0x040)DISP_REG_CONFIG_MDP_SHP_SEL              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_SHP_SEL));
            DISP_MSG("(0x044)DISP_REG_CONFIG_MDP_WROT_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_SEL));
            DISP_MSG("(0x048)DISP_REG_CONFIG_MDP_WDMA_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WDMA_SEL));
            DISP_MSG("(0x04C)DISP_REG_CONFIG_DISP_BLS_SOUT_SEL        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL));
            DISP_MSG("(0x050)DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL       = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL));
            DISP_MSG("(0x054)DISP_REG_CONFIG_DISP_PQ_SEL              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_PQ_SEL));
            DISP_MSG("(0x058)DISP_REG_CONFIG_DISP_DSI_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_DSI_SEL));
            DISP_MSG("(0x05C)DISP_REG_CONFIG_DISP_DPI_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_DPI_SEL));
            DISP_MSG("(0x060)DISP_REG_CONFIG_DBI_PAD_DELAY_SEL        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DBI_PAD_DELAY_SEL));
            DISP_MSG("(0x064)DISP_REG_CONFIG_DBI_DPI_IO_SEL           = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DBI_DPI_IO_SEL));
            DISP_MSG("(0x070)DISP_REG_CONFIG_LCD_RESET_CON            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_LCD_RESET_CON));
            DISP_MSG("(0x100)DISP_REG_CONFIG_MMSYS_CG_CON0            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
            DISP_MSG("(0x104)DISP_REG_CONFIG_MMSYS_CG_SET0            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET0));
            DISP_MSG("(0x108)DISP_REG_CONFIG_MMSYS_CG_CLR0            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR0));
            DISP_MSG("(0x110)DISP_REG_CONFIG_MMSYS_CG_CON1            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1));
            DISP_MSG("(0x114)DISP_REG_CONFIG_MMSYS_CG_SET1            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET1));
            DISP_MSG("(0x118)DISP_REG_CONFIG_MMSYS_CG_CLR1            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR1));
            DISP_MSG("(0x120)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0));
            DISP_MSG("(0x124)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0));
            DISP_MSG("(0x128)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0));
            DISP_MSG("(0x12C)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1));
            DISP_MSG("(0x130)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1));
            DISP_MSG("(0x134)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1));
            DISP_MSG("(0x138)DISP_REG_CONFIG_MMSYS_SW_RST_B           = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_SW_RST_B));
            DISP_MSG("(0x200)DISP_REG_CONFIG_DISP_FAKE_ENG_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_EN));
            DISP_MSG("(0x204)DISP_REG_CONFIG_DISP_FAKE_ENG_RST        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_RST));
            DISP_MSG("(0x208)DISP_REG_CONFIG_DISP_FAKE_ENG_CON0       = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_CON0));
            DISP_MSG("(0x20C)DISP_REG_CONFIG_DISP_FAKE_ENG_CON1       = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_CON1));
            DISP_MSG("(0x210)DISP_REG_CONFIG_DISP_FAKE_ENG_RD_ADDR    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_RD_ADDR));
            DISP_MSG("(0x214)DISP_REG_CONFIG_DISP_FAKE_ENG_WR_ADDR    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_WR_ADDR));
            DISP_MSG("(0x218)DISP_REG_CONFIG_DISP_FAKE_ENG_STATE      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_STATE));
            DISP_MSG("(0x800)DISP_REG_CONFIG_MMSYS_MBIST_MODE         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_MODE));
            DISP_MSG("(0x804)DISP_REG_CONFIG_MMSYS_MBIST_HOLDB        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_HOLDB));
            DISP_MSG("(0x808)DISP_REG_CONFIG_MMSYS_MBIST_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_CON));
            DISP_MSG("(0x80C)DISP_REG_CONFIG_MMSYS_MBIST_DONE         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_DONE));
            DISP_MSG("(0x810)DISP_REG_CONFIG_MMSYS_MBIST_FAIL0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL0));
            DISP_MSG("(0x814)DISP_REG_CONFIG_MMSYS_MBIST_FAIL1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL1));
            DISP_MSG("(0x818)DISP_REG_CONFIG_MMSYS_MBIST_FAIL2        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL2));
            DISP_MSG("(0x820)DISP_REG_CONFIG_MMSYS_MBIST_BSEL0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL0));
            DISP_MSG("(0x824)DISP_REG_CONFIG_MMSYS_MBIST_BSEL1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL1));
            DISP_MSG("(0x828)DISP_REG_CONFIG_MMSYS_MBIST_BSEL2        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL2));
            DISP_MSG("(0x830)DISP_REG_CONFIG_MMSYS_MEM_DELSEL0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL0));
            DISP_MSG("(0x834)DISP_REG_CONFIG_MMSYS_MEM_DELSEL1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL1));
            DISP_MSG("(0x838)DISP_REG_CONFIG_MMSYS_MEM_DELSEL2        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL2));
            DISP_MSG("(0x83C)DISP_REG_CONFIG_MMSYS_MEM_DELSEL3        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL3));
            DISP_MSG("(0x840)DISP_REG_CONFIG_MMSYS_MEM_DELSEL4        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL4));
            DISP_MSG("(0x844)DISP_REG_CONFIG_MMSYS_MEM_DELSEL5        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL5));
            DISP_MSG("(0x848)DISP_REG_CONFIG_MMSYS_MEM_DELSEL6        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL6));
            DISP_MSG("(0x84C)DISP_REG_CONFIG_MMSYS_MEM_DELSEL7        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL7));
            DISP_MSG("(0x850)DISP_REG_CONFIG_MDP_WROT_MBISR_RESET     = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_MBISR_RESET));
            DISP_MSG("(0x854)DISP_REG_CONFIG_MDP_WROT_MBISR_FAIL      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_MBISR_FAIL));
            DISP_MSG("(0x858)DISP_REG_CONFIG_MDP_WROT_MBISR_OK        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_MBISR_OK));
            DISP_MSG("(0x860)DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL));
            DISP_MSG("(0x864)DISP_REG_CONFIG_MMSYS_DUMMY              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DUMMY));
            DISP_MSG("(0x870)DISP_REG_CONFIG_MMSYS_DL_VALID_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_VALID_0));
            DISP_MSG("(0x874)DISP_REG_CONFIG_MMSYS_DL_READY_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_READY_0));
            DISP_MSG("(0x880)DISP_REG_CONFIG_MMSYS_DL_VALID_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_VALID_1));
            DISP_MSG("(0x884)DISP_REG_CONFIG_MMSYS_DL_READY_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_READY_1));
            break;

        case DISP_MODULE_OVL :
            DISP_MSG("===== DISP OVL Reg Dump: ============\n");
            DISP_MSG("(0x000)DISP_REG_OVL_STA                         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_STA));
            DISP_MSG("(0x004)DISP_REG_OVL_INTEN                       = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_INTEN));
            DISP_MSG("(0x008)DISP_REG_OVL_INTSTA                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_INTSTA));
            DISP_MSG("(0x00C)DISP_REG_OVL_EN                          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_EN));
            DISP_MSG("(0x010)DISP_REG_OVL_TRIG                        = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_TRIG));
            DISP_MSG("(0x014)DISP_REG_OVL_RST                         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RST));
            DISP_MSG("(0x020)DISP_REG_OVL_ROI_SIZE                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_ROI_SIZE));
            DISP_MSG("(0x024)DISP_REG_OVL_DATAPATH_CON                = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_DATAPATH_CON));
            DISP_MSG("(0x028)DISP_REG_OVL_ROI_BGCLR                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_ROI_BGCLR));
            DISP_MSG("(0x02C)DISP_REG_OVL_SRC_CON                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_SRC_CON));
            DISP_MSG("(0x030)DISP_REG_OVL_L0_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_CON));
            DISP_MSG("(0x034)DISP_REG_OVL_L0_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_SRCKEY));
            DISP_MSG("(0x038)DISP_REG_OVL_L0_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE));
            DISP_MSG("(0x03C)DISP_REG_OVL_L0_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_OFFSET));
            DISP_MSG("(0x040)DISP_REG_OVL_L0_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_ADDR));
            DISP_MSG("(0x044)DISP_REG_OVL_L0_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_PITCH));
            DISP_MSG("(0x048)DISP_REG_OVL_L0_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_TILE));
            DISP_MSG("(0x050)DISP_REG_OVL_L1_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_CON));
            DISP_MSG("(0x054)DISP_REG_OVL_L1_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_SRCKEY));
            DISP_MSG("(0x058)DISP_REG_OVL_L1_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE));
            DISP_MSG("(0x05C)DISP_REG_OVL_L1_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_OFFSET));
            DISP_MSG("(0x060)DISP_REG_OVL_L1_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_ADDR));
            DISP_MSG("(0x064)DISP_REG_OVL_L1_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_PITCH));
            DISP_MSG("(0x068)DISP_REG_OVL_L1_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_TILE));
            DISP_MSG("(0x070)DISP_REG_OVL_L2_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_CON));
            DISP_MSG("(0x074)DISP_REG_OVL_L2_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_SRCKEY));
            DISP_MSG("(0x078)DISP_REG_OVL_L2_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE));
            DISP_MSG("(0x07C)DISP_REG_OVL_L2_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_OFFSET));
            DISP_MSG("(0x080)DISP_REG_OVL_L2_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_ADDR));
            DISP_MSG("(0x084)DISP_REG_OVL_L2_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_PITCH));
            DISP_MSG("(0x088)DISP_REG_OVL_L2_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_TILE));
            DISP_MSG("(0x090)DISP_REG_OVL_L3_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_CON));
            DISP_MSG("(0x094)DISP_REG_OVL_L3_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_SRCKEY));
            DISP_MSG("(0x098)DISP_REG_OVL_L3_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE));
            DISP_MSG("(0x09C)DISP_REG_OVL_L3_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_OFFSET));
            DISP_MSG("(0x0A0)DISP_REG_OVL_L3_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_ADDR));
            DISP_MSG("(0x0A4)DISP_REG_OVL_L3_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_PITCH));
            DISP_MSG("(0x0A8)DISP_REG_OVL_L3_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_TILE));
            DISP_MSG("(0x0C0)DISP_REG_OVL_RDMA0_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_CTRL));
            DISP_MSG("(0x0C8)DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1));
            DISP_MSG("(0x0CC)DISP_REG_OVL_RDMA0_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_SLOW_CON));
            DISP_MSG("(0x0D0)DISP_REG_OVL_RDMA0_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_FIFO_CTRL));
            DISP_MSG("(0x0E0)DISP_REG_OVL_RDMA1_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_CTRL));
            DISP_MSG("(0x0E8)DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1));
            DISP_MSG("(0x0EC)DISP_REG_OVL_RDMA1_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_SLOW_CON));
            DISP_MSG("(0x0F0)DISP_REG_OVL_RDMA1_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_FIFO_CTRL));
            DISP_MSG("(0x100)DISP_REG_OVL_RDMA2_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_CTRL));
            DISP_MSG("(0x108)DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1));
            DISP_MSG("(0x10C)DISP_REG_OVL_RDMA2_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_SLOW_CON));
            DISP_MSG("(0x110)DISP_REG_OVL_RDMA2_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_FIFO_CTRL));
            DISP_MSG("(0x120)DISP_REG_OVL_RDMA3_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_CTRL));
            DISP_MSG("(0x128)DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1));
            DISP_MSG("(0x12C)DISP_REG_OVL_RDMA3_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_SLOW_CON));
            DISP_MSG("(0x130)DISP_REG_OVL_RDMA3_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_FIFO_CTRL));
            DISP_MSG("(0x134)DISP_REG_OVL_L0_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_R0));
            DISP_MSG("(0x138)DISP_REG_OVL_L0_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_R1));
            DISP_MSG("(0x13C)DISP_REG_OVL_L0_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_G0));
            DISP_MSG("(0x140)DISP_REG_OVL_L0_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_G1));
            DISP_MSG("(0x144)DISP_REG_OVL_L0_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_B0));
            DISP_MSG("(0x148)DISP_REG_OVL_L0_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_B1));
            DISP_MSG("(0x14C)DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0));
            DISP_MSG("(0x150)DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1));
            DISP_MSG("(0x154)DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0));
            DISP_MSG("(0x158)DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1));
            DISP_MSG("(0x15C)DISP_REG_OVL_L1_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_R0));
            DISP_MSG("(0x160)DISP_REG_OVL_L1_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_R1));
            DISP_MSG("(0x164)DISP_REG_OVL_L1_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_G0));
            DISP_MSG("(0x168)DISP_REG_OVL_L1_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_G1));
            DISP_MSG("(0x16C)DISP_REG_OVL_L1_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_B0));
            DISP_MSG("(0x170)DISP_REG_OVL_L1_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_B1));
            DISP_MSG("(0x174)DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0));
            DISP_MSG("(0x178)DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1));
            DISP_MSG("(0x17C)DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0));
            DISP_MSG("(0x180)DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1));
            DISP_MSG("(0x184)DISP_REG_OVL_L2_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_R0));
            DISP_MSG("(0x188)DISP_REG_OVL_L2_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_R1));
            DISP_MSG("(0x18C)DISP_REG_OVL_L2_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_G0));
            DISP_MSG("(0x190)DISP_REG_OVL_L2_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_G1));
            DISP_MSG("(0x194)DISP_REG_OVL_L2_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_B0));
            DISP_MSG("(0x198)DISP_REG_OVL_L2_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_B1));
            DISP_MSG("(0x19C)DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0));
            DISP_MSG("(0x1A0)DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1));
            DISP_MSG("(0x1A4)DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0));
            DISP_MSG("(0x1A8)DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1));
            DISP_MSG("(0x1AC)DISP_REG_OVL_L3_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_R0));
            DISP_MSG("(0x1B0)DISP_REG_OVL_L3_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_R1));
            DISP_MSG("(0x1B4)DISP_REG_OVL_L3_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_G0));
            DISP_MSG("(0x1B8)DISP_REG_OVL_L3_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_G1));
            DISP_MSG("(0x1BC)DISP_REG_OVL_L3_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_B0));
            DISP_MSG("(0x1C0)DISP_REG_OVL_L3_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_B1));
            DISP_MSG("(0x1C4)DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0));
            DISP_MSG("(0x1C8)DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1));
            DISP_MSG("(0x1CC)DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0));
            DISP_MSG("(0x1D0)DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1));
            DISP_MSG("(0x1D4)DISP_REG_OVL_DEBUG_MON_SEL               = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_DEBUG_MON_SEL));
            DISP_MSG("(0x1E0)DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2));
            DISP_MSG("(0x1E4)DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2));
            DISP_MSG("(0x1E8)DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2));
            DISP_MSG("(0x1EC)DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2));
            DISP_MSG("(0x200)DISP_REG_OVL_DUMMY_REG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_DUMMY_REG));
            DISP_MSG("(0x240)DISP_REG_OVL_FLOW_CTRL_DBG               = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG));
            DISP_MSG("(0x244)DISP_REG_OVL_ADDCON_DBG                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG));
            DISP_MSG("(0x24C)DISP_REG_OVL_RDMA0_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_DBG));
            DISP_MSG("(0x250)DISP_REG_OVL_RDMA1_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_DBG));
            DISP_MSG("(0x254)DISP_REG_OVL_RDMA2_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_DBG));
            DISP_MSG("(0x258)DISP_REG_OVL_RDMA3_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_DBG));
            break;

        case DISP_MODULE_PQ :
            DISP_MSG("===== DISP PQ Reg Dump: ============\n");
            DISP_MSG("(0x000)DISP_REG_PQ_CTRL                         = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CTRL));
            DISP_MSG("(0x004)DISP_REG_PQ_INTEN                        = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INTEN));
            DISP_MSG("(0x008)DISP_REG_PQ_INTSTA                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INTSTA));
            DISP_MSG("(0x00C)DISP_REG_PQ_STATUS                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_STATUS));
            DISP_MSG("(0x010)DISP_REG_PQ_CFG                          = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CFG));
            DISP_MSG("(0x014)DISP_REG_PQ_INPUT_COUNT                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INPUT_COUNT));
            DISP_MSG("(0x018)DISP_REG_PQ_CHKSUM                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CHKSUM));
            DISP_MSG("(0x01C)DISP_REG_PQ_OUTPUT_COUNT                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_OUTPUT_COUNT));
            DISP_MSG("(0x020)DISP_REG_PQ_INPUT_SIZE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INPUT_SIZE));
            DISP_MSG("(0x024)DISP_REG_PQ_OUTPUT_OFFSET                = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_OUTPUT_OFFSET));
            DISP_MSG("(0x028)DISP_REG_PQ_OUTPUT_SIZE                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_OUTPUT_SIZE));
            DISP_MSG("(0x02C)DISP_REG_PQ_HSYNC                        = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_HSYNC));
            DISP_MSG("(0x030)DISP_REG_PQ_DEMO_HMASK                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_DEMO_HMASK));
            DISP_MSG("(0x034)DISP_REG_PQ_DEMO_VMASK                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_DEMO_VMASK));
            DISP_MSG("(0x040)DISP_REG_PQ_SHP_CON_00                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SHP_CON_00));
            DISP_MSG("(0x044)DISP_REG_PQ_SHP_CON_01                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SHP_CON_01));
            DISP_MSG("(0x050)DISP_REG_PQ_CONT_CP                      = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CONT_CP));
            DISP_MSG("(0x054)DISP_REG_PQ_CONT_SLOPE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CONT_SLOPE));
            DISP_MSG("(0x058)DISP_REG_PQ_CONT_OFFSET                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CONT_OFFSET));
            DISP_MSG("(0x060)DISP_REG_PQ_SAT_CON_00                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_CON_00));
            DISP_MSG("(0x064)DISP_REG_PQ_SAT_CON_01                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_CON_01));
            DISP_MSG("(0x068)DISP_REG_PQ_SAT_GAIN                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_GAIN));
            DISP_MSG("(0x06C)DISP_REG_PQ_SAT_SLOPE                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_SLOPE));
            DISP_MSG("(0x070)DISP_REG_PQ_HIST_X_CFG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_HIST_X_CFG));
            DISP_MSG("(0x074)DISP_REG_PQ_HIST_Y_CFG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_HIST_Y_CFG));
            DISP_MSG("(0x078)DISP_REG_PQ_LUMA_HIST_00                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_00));
            DISP_MSG("(0x07C)DISP_REG_PQ_LUMA_HIST_01                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_01));
            DISP_MSG("(0x080)DISP_REG_PQ_LUMA_HIST_02                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_02));
            DISP_MSG("(0x084)DISP_REG_PQ_LUMA_HIST_03                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_03));
            DISP_MSG("(0x088)DISP_REG_PQ_LUMA_HIST_04                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_04));
            DISP_MSG("(0x08C)DISP_REG_PQ_LUMA_HIST_05                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_05));
            DISP_MSG("(0x090)DISP_REG_PQ_LUMA_HIST_06                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_06));
            DISP_MSG("(0x094)DISP_REG_PQ_LUMA_HIST_07                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_07));
            DISP_MSG("(0x098)DISP_REG_PQ_LUMA_HIST_08                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_08));
            DISP_MSG("(0x09C)DISP_REG_PQ_LUMA_HIST_09                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_09));
            DISP_MSG("(0x0A0)DISP_REG_PQ_LUMA_HIST_10                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_10));
            DISP_MSG("(0x0A4)DISP_REG_PQ_LUMA_HIST_11                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_11));
            DISP_MSG("(0x0A8)DISP_REG_PQ_LUMA_HIST_12                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_12));
            DISP_MSG("(0x0AC)DISP_REG_PQ_LUMA_HIST_13                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_13));
            DISP_MSG("(0x0B0)DISP_REG_PQ_LUMA_HIST_14                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_14));
            DISP_MSG("(0x0B4)DISP_REG_PQ_LUMA_HIST_15                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_15));
            DISP_MSG("(0x0B8)DISP_REG_PQ_LUMA_HIST_16                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_16));
            DISP_MSG("(0x0C0)DISP_REG_PQ_LUMA_SUM                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_SUM));
            DISP_MSG("(0x0C4)DISP_REG_PQ_LUMA_MIN_MAX                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_MIN_MAX));
            DISP_MSG("(0x0D0)DISP_REG_PQ_Y_FTN_1_0                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_1_0));
            DISP_MSG("(0x0D4)DISP_REG_PQ_Y_FTN_3_2                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_3_2));
            DISP_MSG("(0x0D8)DISP_REG_PQ_Y_FTN_5_4                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_5_4));
            DISP_MSG("(0x0DC)DISP_REG_PQ_Y_FTN_7_6                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_7_6));
            DISP_MSG("(0x0E0)DISP_REG_PQ_Y_FTN_9_8                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_9_8));
            DISP_MSG("(0x0E4)DISP_REG_PQ_Y_FTN_11_10                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_11_10));
            DISP_MSG("(0x0E8)DISP_REG_PQ_Y_FTN_13_12                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_13_12));
            DISP_MSG("(0x0EC)DISP_REG_PQ_Y_FTN_15_14                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_15_14));
            DISP_MSG("(0x0F0)DISP_REG_PQ_Y_FTN_16                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_16));
            DISP_MSG("(0x100)DISP_REG_PQ_C_BOOST_CON                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_BOOST_CON));
            DISP_MSG("(0x110)DISP_REG_PQ_C_HIST_X_CFG                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_X_CFG));
            DISP_MSG("(0x114)DISP_REG_PQ_C_HIST_Y_CFG                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_Y_CFG));
            DISP_MSG("(0x118)DISP_REG_PQ_C_HIST_CON                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_CON));
            DISP_MSG("(0x11C)DISP_REG_PQ_C_HIST_BIN                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_BIN));
            DISP_MSG("(0x200)DISP_REG_PQ_DEMO_MAIN                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_DEMO_MAIN));
            DISP_MSG("(0x208)DISP_REG_PQ_INK_LUMA                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INK_LUMA));
            DISP_MSG("(0x20C)DISP_REG_PQ_INK_CHROMA                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INK_CHROMA));
            DISP_MSG("(0x210)DISP_REG_PQ_CAP_POS                      = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_POS));
            DISP_MSG("(0x218)DISP_REG_PQ_CAP_IN_Y                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_IN_Y));
            DISP_MSG("(0x21C)DISP_REG_PQ_CAP_IN_C                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_IN_C));
            DISP_MSG("(0x220)DISP_REG_PQ_CAP_OUT_Y                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_OUT_Y));
            DISP_MSG("(0x224)DISP_REG_PQ_CAP_OUT_C                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_OUT_C));
            break;

        case DISP_MODULE_BLS :
            DISP_MSG("===== DISP PWM Reg Dump: ============\n");
            DISP_MSG("(0x000)DISP_REG_PWM_EN                          = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_EN));
            DISP_MSG("(0x004)DISP_REG_PWM_RST                         = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_RST));
            DISP_MSG("(0x010)DISP_REG_PWM_CON_0                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_CON_0));
            DISP_MSG("(0x014)DISP_REG_PWM_CON_1                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_CON_1));
            DISP_MSG("(0x018)DISP_REG_PWM_GRADUAL                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_GRADUAL));
            DISP_MSG("(0x01C)DISP_REG_PWM_GRADUAL_RO                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_GRADUAL_RO));
            DISP_MSG("(0x020)DISP_REG_PWM_DEBUG                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_DEBUG));
            DISP_MSG("(0x030)DISP_REG_PWM_DUMMY                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_DUMMY));

            DISP_MSG("===== DISP BLS Reg Dump: ============\n");
            DISP_MSG("(0x000)DISP_REG_BLS_EN                          = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_EN));
            DISP_MSG("(0x004)DISP_REG_BLS_RST                         = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_RST));
            DISP_MSG("(0x008)DISP_REG_BLS_INTEN                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_INTEN));
            DISP_MSG("(0x00C)DISP_REG_BLS_INTSTA                      = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_INTSTA));
            DISP_MSG("(0x010)DISP_REG_BLS_BLS_SETTING                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_BLS_SETTING));
            DISP_MSG("(0x014)DISP_REG_BLS_FANA_SETTING                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FANA_SETTING));
            DISP_MSG("(0x018)DISP_REG_BLS_SRC_SIZE                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SRC_SIZE));
            DISP_MSG("(0x020)DISP_REG_BLS_GAIN_SETTING                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAIN_SETTING));
            DISP_MSG("(0x024)DISP_REG_BLS_MANUAL_GAIN                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MANUAL_GAIN));
            DISP_MSG("(0x028)DISP_REG_BLS_MANUAL_MAXCLR               = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MANUAL_MAXCLR));
            DISP_MSG("(0x030)DISP_REG_BLS_GAMMA_SETTING               = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAMMA_SETTING));
            DISP_MSG("(0x038)DISP_REG_BLS_LUT_UPDATE                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_LUT_UPDATE));
            DISP_MSG("(0x060)DISP_REG_BLS_MAXCLR_THD                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_THD));
            DISP_MSG("(0x064)DISP_REG_BLS_DISTPT_THD                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_THD));
            DISP_MSG("(0x068)DISP_REG_BLS_MAXCLR_LIMIT                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_LIMIT));
            DISP_MSG("(0x06C)DISP_REG_BLS_DISTPT_LIMIT                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_LIMIT));
            DISP_MSG("(0x070)DISP_REG_BLS_AVE_SETTING                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_AVE_SETTING));
            DISP_MSG("(0x074)DISP_REG_BLS_AVE_LIMIT                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_AVE_LIMIT));
            DISP_MSG("(0x078)DISP_REG_BLS_DISTPT_SETTING              = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_SETTING));
            DISP_MSG("(0x07C)DISP_REG_BLS_HIS_CLEAR                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_HIS_CLEAR));
            DISP_MSG("(0x080)DISP_REG_BLS_SC_DIFF_THD                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SC_DIFF_THD));
            DISP_MSG("(0x084)DISP_REG_BLS_SC_BIN_THD                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SC_BIN_THD));
            DISP_MSG("(0x088)DISP_REG_BLS_MAXCLR_GRADUAL              = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_GRADUAL));
            DISP_MSG("(0x08C)DISP_REG_BLS_DISTPT_GRADUAL              = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_GRADUAL));
            DISP_MSG("(0x090)DISP_REG_BLS_FAST_IIR_XCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_XCOEFF));
            DISP_MSG("(0x094)DISP_REG_BLS_FAST_IIR_YCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_YCOEFF));
            DISP_MSG("(0x098)DISP_REG_BLS_SLOW_IIR_XCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_XCOEFF));
            DISP_MSG("(0x09C)DISP_REG_BLS_SLOW_IIR_YCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_YCOEFF));
            DISP_MSG("(0x0A0)DISP_REG_BLS_PWM_DUTY                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
            DISP_MSG("(0x0B0)DISP_REG_BLS_DEBUG                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DEBUG));
            DISP_MSG("(0x0B4)DISP_REG_BLS_PATTERN                     = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_PATTERN));
            DISP_MSG("(0x0B8)DISP_REG_BLS_CHKSUM                      = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_CHKSUM));
            DISP_MSG("(0x0C0)DISP_REG_BLS_SAFE_MEASURE                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SAFE_MEASURE));
            DISP_MSG("(0x100)DISP_REG_BLS_HIS_BIN                     = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_HIS_BIN_));
            DISP_MSG("(0x200)DISP_REG_BLS_PWM_DUTY_RD                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD));
            DISP_MSG("(0x204)DISP_REG_BLS_FRAME_AVE_RD                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FRAME_AVE_RD));
            DISP_MSG("(0x208)DISP_REG_BLS_MAXCLR_RD                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_RD));
            DISP_MSG("(0x20C)DISP_REG_BLS_DISTPT_RD                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_RD));
            DISP_MSG("(0x210)DISP_REG_BLS_GAIN_RD                     = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAIN_RD));
            DISP_MSG("(0x214)DISP_REG_BLS_SC_RD                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SC_RD));
            DISP_MSG("(0x300)DISP_REG_BLS_LUMINANCE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE_));
            DISP_MSG("(0x384)DISP_REG_BLS_LUMINANCE_255               = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE_255));
            DISP_MSG("(0x400)DISP_REG_BLS_GAMMA_LUT                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT_));
            DISP_MSG("(0xE00)DISP_REG_BLS_DITHER_0                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_0));
            DISP_MSG("(0xE14)DISP_REG_BLS_DITHER_5                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_5));
            DISP_MSG("(0xE18)DISP_REG_BLS_DITHER_6                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_6));
            DISP_MSG("(0xE1C)DISP_REG_BLS_DITHER_7                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_7));
            DISP_MSG("(0xE20)DISP_REG_BLS_DITHER_8                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_8));
            DISP_MSG("(0xE24)DISP_REG_BLS_DITHER_9                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_9));
            DISP_MSG("(0xE28)DISP_REG_BLS_DITHER_10                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_10));
            DISP_MSG("(0xE2C)DISP_REG_BLS_DITHER_11                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_11));
            DISP_MSG("(0xE30)DISP_REG_BLS_DITHER_12                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_12));
            DISP_MSG("(0xE34)DISP_REG_BLS_DITHER_13                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_13));
            DISP_MSG("(0xE38)DISP_REG_BLS_DITHER_14                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_14));
            DISP_MSG("(0xE3C)DISP_REG_BLS_DITHER_15                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_15));
            DISP_MSG("(0xE40)DISP_REG_BLS_DITHER_16                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_16));
            DISP_MSG("(0xE44)DISP_REG_BLS_DITHER_17                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_17));
            DISP_MSG("(0xF00)DISP_REG_BLS_DUMMY                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DUMMY));
            break;

        case DISP_MODULE_WDMA0 :
            DISP_MSG("===== DISP WDMA Reg Dump: ============\n");
            DISP_MSG("(0x000)DISP_REG_WDMA_INTEN                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_INTEN));
            DISP_MSG("(0x004)DISP_REG_WDMA_INTSTA                     = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_INTSTA));
            DISP_MSG("(0x008)DISP_REG_WDMA_EN                         = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_EN));
            DISP_MSG("(0x00C)DISP_REG_WDMA_RST                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_RST));
            DISP_MSG("(0x010)DISP_REG_WDMA_SMI_CON                    = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_SMI_CON));
            DISP_MSG("(0x014)DISP_REG_WDMA_CFG                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CFG));
            DISP_MSG("(0x018)DISP_REG_WDMA_SRC_SIZE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE));
            DISP_MSG("(0x01C)DISP_REG_WDMA_CLIP_SIZE                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE));
            DISP_MSG("(0x020)DISP_REG_WDMA_CLIP_COORD                 = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD));
            DISP_MSG("(0x024)DISP_REG_WDMA_DST_ADDR0                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR0));
            DISP_MSG("(0x028)DISP_REG_WDMA_DST_W_IN_BYTE              = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE));
            DISP_MSG("(0x02C)DISP_REG_WDMA_ALPHA                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_ALPHA));
            DISP_MSG("(0x038)DISP_REG_WDMA_BUF_CON1                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON1));
            DISP_MSG("(0x03C)DISP_REG_WDMA_BUF_CON2                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON2));
            DISP_MSG("(0x040)DISP_REG_WDMA_C00                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C00));
            DISP_MSG("(0x044)DISP_REG_WDMA_C02                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C02));
            DISP_MSG("(0x048)DISP_REG_WDMA_C10                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C10));
            DISP_MSG("(0x04C)DISP_REG_WDMA_C12                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C12));
            DISP_MSG("(0x050)DISP_REG_WDMA_C20                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C20));
            DISP_MSG("(0x054)DISP_REG_WDMA_C22                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C22));
            DISP_MSG("(0x058)DISP_REG_WDMA_PRE_ADD0                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD0));
            DISP_MSG("(0x05C)DISP_REG_WDMA_PRE_ADD2                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD2));
            DISP_MSG("(0x060)DISP_REG_WDMA_POST_ADD0                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD0));
            DISP_MSG("(0x064)DISP_REG_WDMA_POST_ADD2                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD2));
            DISP_MSG("(0x070)DISP_REG_WDMA_DST_ADDR1                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR1));
            DISP_MSG("(0x074)DISP_REG_WDMA_DST_ADDR2                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR2));
            DISP_MSG("(0x078)DISP_REG_WDMA_DST_UV_PITCH               = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH));
            DISP_MSG("(0x080)DISP_REG_WDMA_DST_ADDR_OFFSET0           = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR_OFFSET0));
            DISP_MSG("(0x084)DISP_REG_WDMA_DST_ADDR_OFFSET1           = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR_OFFSET1));
            DISP_MSG("(0x088)DISP_REG_WDMA_DST_ADDR_OFFSET2           = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR_OFFSET2));
            DISP_MSG("(0x0A0)DISP_REG_WDMA_FLOW_CTRL_DBG              = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG));
            DISP_MSG("(0x0A4)DISP_REG_WDMA_EXEC_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG));
            DISP_MSG("(0x0A8)DISP_REG_WDMA_CT_DBG                     = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CT_DBG));
            DISP_MSG("(0x0AC)DISP_REG_WDMA_DEBUG                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DEBUG));
            DISP_MSG("(0x100)DISP_REG_WDMA_DUMMY                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DUMMY));
            DISP_MSG("(0xE00)DISP_REG_WDMA_DITHER_0                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_0));
            DISP_MSG("(0xE14)DISP_REG_WDMA_DITHER_5                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_5));
            DISP_MSG("(0xE18)DISP_REG_WDMA_DITHER_6                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_6));
            DISP_MSG("(0xE1C)DISP_REG_WDMA_DITHER_7                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_7));
            DISP_MSG("(0xE20)DISP_REG_WDMA_DITHER_8                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_8));
            DISP_MSG("(0xE24)DISP_REG_WDMA_DITHER_9                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_9));
            DISP_MSG("(0xE28)DISP_REG_WDMA_DITHER_10                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_10));
            DISP_MSG("(0xE2C)DISP_REG_WDMA_DITHER_11                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_11));
            DISP_MSG("(0xE30)DISP_REG_WDMA_DITHER_12                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_12));
            DISP_MSG("(0xE34)DISP_REG_WDMA_DITHER_13                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_13));
            DISP_MSG("(0xE38)DISP_REG_WDMA_DITHER_14                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_14));
            DISP_MSG("(0xE3C)DISP_REG_WDMA_DITHER_15                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_15));
            DISP_MSG("(0xE40)DISP_REG_WDMA_DITHER_16                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_16));
            DISP_MSG("(0xE44)DISP_REG_WDMA_DITHER_17                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_17));
            break;

        case DISP_MODULE_RDMA0 :
            DISP_MSG("===== DISP RDMA Reg Dump: ======== \n");
            DISP_MSG("(0x000)DISP_REG_RDMA_INT_ENABLE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE));
            DISP_MSG("(0x004)DISP_REG_RDMA_INT_STATUS                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_INT_STATUS));
            DISP_MSG("(0x010)DISP_REG_RDMA_GLOBAL_CON                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON));
            DISP_MSG("(0x014)DISP_REG_RDMA_SIZE_CON_0                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0));
            DISP_MSG("(0x018)DISP_REG_RDMA_SIZE_CON_1                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1));
            DISP_MSG("(0x01C)DISP_REG_RDMA_TARGET_LINE                = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_TARGET_LINE));
            DISP_MSG("(0x024)DISP_REG_RDMA_MEM_CON                    = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_CON));
            DISP_MSG("(0x028)DISP_REG_RDMA_MEM_START_ADDR             = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR));
            DISP_MSG("(0x02C)DISP_REG_RDMA_MEM_SRC_PITCH              = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH));
            DISP_MSG("(0x030)DISP_REG_RDMA_MEM_GMC_SETTING_0          = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0));
            DISP_MSG("(0x034)DISP_REG_RDMA_MEM_SLOW_CON               = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON));
            DISP_MSG("(0x038)DISP_REG_RDMA_MEM_GMC_SETTING_1          = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1));
            DISP_MSG("(0x040)DISP_REG_RDMA_FIFO_CON                   = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_FIFO_CON));
            DISP_MSG("(0x044)DISP_REG_RDMA_FIFO_LOG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG));
            DISP_MSG("(0x054)DISP_REG_RDMA_C00                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C00));
            DISP_MSG("(0x058)DISP_REG_RDMA_C01                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C01));
            DISP_MSG("(0x05C)DISP_REG_RDMA_C02                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C02));
            DISP_MSG("(0x060)DISP_REG_RDMA_C10                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C10));
            DISP_MSG("(0x064)DISP_REG_RDMA_C11                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C11));
            DISP_MSG("(0x068)DISP_REG_RDMA_C12                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C12));
            DISP_MSG("(0x06C)DISP_REG_RDMA_C20                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C20));
            DISP_MSG("(0x070)DISP_REG_RDMA_C21                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C21));
            DISP_MSG("(0x074)DISP_REG_RDMA_C22                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C22));
            DISP_MSG("(0x078)DISP_REG_RDMA_PRE_ADD_0                  = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_0));
            DISP_MSG("(0x07C)DISP_REG_RDMA_PRE_ADD_1                  = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_1));
            DISP_MSG("(0x080)DISP_REG_RDMA_PRE_ADD_2                  = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_2));
            DISP_MSG("(0x084)DISP_REG_RDMA_POST_ADD_0                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_POST_ADD_0));
            DISP_MSG("(0x088)DISP_REG_RDMA_POST_ADD_1                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_POST_ADD_1));
            DISP_MSG("(0x08C)DISP_REG_RDMA_POST_ADD_2                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_POST_ADD_2));
            DISP_MSG("(0x090)DISP_REG_RDMA_DUMMY                      = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_DUMMY));
            DISP_MSG("(0x094)DISP_REG_RDMA_DEBUG_OUT_SEL              = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL));
            break;

        case DISP_MODULE_MUTEX :
            DISP_MSG("===== DISP DISP_REG_MUTEX_CONFIG Reg Dump: ============\n");
            DISP_MSG("(0x000)DISP_REG_CONFIG_MUTEX_INTEN              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN));
            DISP_MSG("(0x004)DISP_REG_CONFIG_MUTEX_INTSTA             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA));
            DISP_MSG("(0x008)DISP_REG_CONFIG_REG_UPD_TIMEOUT          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_REG_UPD_TIMEOUT));
            DISP_MSG("(0x00C)DISP_REG_CONFIG_REG_COMMIT               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT));
            DISP_MSG("(0x020)DISP_REG_CONFIG_MUTEX0_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_EN));
            DISP_MSG("(0x024)DISP_REG_CONFIG_MUTEX0                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0));
            DISP_MSG("(0x028)DISP_REG_CONFIG_MUTEX0_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_RST));
            DISP_MSG("(0x02C)DISP_REG_CONFIG_MUTEX0_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_MOD));
            DISP_MSG("(0x030)DISP_REG_CONFIG_MUTEX0_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_SOF));
            DISP_MSG("(0x040)DISP_REG_CONFIG_MUTEX1_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_EN));
            DISP_MSG("(0x044)DISP_REG_CONFIG_MUTEX1                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1));
            DISP_MSG("(0x048)DISP_REG_CONFIG_MUTEX1_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_RST));
            DISP_MSG("(0x04C)DISP_REG_CONFIG_MUTEX1_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_MOD));
            DISP_MSG("(0x050)DISP_REG_CONFIG_MUTEX1_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_SOF));
            DISP_MSG("(0x060)DISP_REG_CONFIG_MUTEX2_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_EN));
            DISP_MSG("(0x064)DISP_REG_CONFIG_MUTEX2                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2));
            DISP_MSG("(0x068)DISP_REG_CONFIG_MUTEX2_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_RST));
            DISP_MSG("(0x06C)DISP_REG_CONFIG_MUTEX2_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_MOD));
            DISP_MSG("(0x070)DISP_REG_CONFIG_MUTEX2_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_SOF));
            DISP_MSG("(0x080)DISP_REG_CONFIG_MUTEX3_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_EN));
            DISP_MSG("(0x084)DISP_REG_CONFIG_MUTEX3                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3));
            DISP_MSG("(0x088)DISP_REG_CONFIG_MUTEX3_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_RST));
            DISP_MSG("(0x08C)DISP_REG_CONFIG_MUTEX3_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_MOD));
            DISP_MSG("(0x090)DISP_REG_CONFIG_MUTEX3_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_SOF));
            DISP_MSG("(0x0A0)DISP_REG_CONFIG_MUTEX4_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_EN));
            DISP_MSG("(0x0A4)DISP_REG_CONFIG_MUTEX4                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4));
            DISP_MSG("(0x0A8)DISP_REG_CONFIG_MUTEX4_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_RST));
            DISP_MSG("(0x0AC)DISP_REG_CONFIG_MUTEX4_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_MOD));
            DISP_MSG("(0x0B0)DISP_REG_CONFIG_MUTEX4_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_SOF));
            DISP_MSG("(0x0C0)DISP_REG_CONFIG_MUTEX5_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_EN));
            DISP_MSG("(0x0C4)DISP_REG_CONFIG_MUTEX5                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5));
            DISP_MSG("(0x0C8)DISP_REG_CONFIG_MUTEX5_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_RST));
            DISP_MSG("(0x0CC)DISP_REG_CONFIG_MUTEX5_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_MOD));
            DISP_MSG("(0x0D0)DISP_REG_CONFIG_MUTEX5_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_SOF));
            DISP_MSG("(0x0E0)DISP_REG_CONFIG_MUTEX6_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_EN));
            DISP_MSG("(0x0E4)DISP_REG_CONFIG_MUTEX6                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6));
            DISP_MSG("(0x0E8)DISP_REG_CONFIG_MUTEX6_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_RST));
            DISP_MSG("(0x0EC)DISP_REG_CONFIG_MUTEX6_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_MOD));
            DISP_MSG("(0x0F0)DISP_REG_CONFIG_MUTEX6_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_SOF));
            DISP_MSG("(0x100)DISP_REG_CONFIG_MUTEX7_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_EN));
            DISP_MSG("(0x104)DISP_REG_CONFIG_MUTEX7                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7));
            DISP_MSG("(0x108)DISP_REG_CONFIG_MUTEX7_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_RST));
            DISP_MSG("(0x10C)DISP_REG_CONFIG_MUTEX7_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_MOD));
            DISP_MSG("(0x110)DISP_REG_CONFIG_MUTEX7_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_SOF));
            DISP_MSG("(0x200)DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL));
            break;

        default :
            DISP_MSG("disp_dump_reg() invalid module id=%d \n", module);
    }

    return 0;
}

