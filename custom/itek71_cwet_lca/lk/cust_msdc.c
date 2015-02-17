#include "cust_msdc.h"
#include "msdc.h"

//For DD2-533
u32 msdc_src_clks[] = {133250000, 160000000, 200000000, 178280000, 0, 0, 26000000, 208000000};
//For DD3-633
//u32 msdc_src_clks[] = {0, 132600000, 165750000, 178280000, 189420000, 0, 26000000, 208000000};


struct msdc_cust msdc_cap = {
    MSDC_CLKSRC_DEFAULT,  /* host clock source        	*/
    MSDC_SMPL_RISING,   /* command latch edge        	*/
    MSDC_SMPL_RISING,   /* data latch edge       		*/
    MSDC0_ODC_3MA,	    /* clock pad driving         	*/
    MSDC0_ODC_3MA,	    /* command pad driving       	*/
    MSDC0_ODC_3MA,	    /* data pad driving      		*/
    MSDC0_ODC_3MA,	    /* clock pad driving for 1.8v   */
    MSDC0_ODC_3MA,	    /* command pad driving   1.8V   */
    MSDC0_ODC_3MA,	    /* data pad driving      1.8V   */
    #if !defined(__FPGA__)
    8,                  /* data pins; for FPGA it may be 4*/
    #else
    4,
    #endif
    0,                  /* data address offset   */

    /* hardware capability flags     */
	MSDC_HIGHSPEED|MSDC_DDR //|MSDC_UHS1//|MSDC_HS200
};

#if defined(MMC_MSDC_DRV_CTP) || defined(MEM_PRESERVED_MODE_ENABLE)
struct msdc_cust msdc_cap_removable = {
    MSDC_CLKSRC_DEFAULT,/* host clock source        */
    MSDC_SMPL_RISING,   /* command latch edge        */
    MSDC_SMPL_RISING,   /* data latch edge       */
    MSDC1_ODC_6MA,    /* clock pad driving         */
    MSDC1_ODC_6MA,    /* command pad driving       */
    MSDC1_ODC_6MA,    /* data pad driving      */
    MSDC0_ODC_6MA,    /* clock pad driving for 1.8v    */ //Note: For 1.8V Use MSDC0_ODC_xMA instead of MSDC1_ODC_xMA
    MSDC0_ODC_6MA,    /* command pad driving   1.8V    */
    MSDC0_ODC_6MA,    /* data pad driving      1.8V    */
    4,          /* data pins             */
    0,          /* data address offset       */

    /* hardware capability flags     */
    MSDC_HIGHSPEED|MSDC_UHS1|MSDC_DDR|MSDC_CD_PIN_EN|MSDC_REMOVABLE
};
#endif

