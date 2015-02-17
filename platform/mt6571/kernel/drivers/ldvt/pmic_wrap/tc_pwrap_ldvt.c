#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <generated/autoconf.h>
#include <linux/platform_device.h>

#include <linux/types.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/smp.h>

#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/random.h>
#include <asm/system.h>

//extern int rand(void);
#define RAND_MAX 20000000

//#include <processor.h>
//#include <BusMonitor.h> //for high pri test
#include <reg_pmic_wrap.h>
#include <mach/mt_pmic_wrap.h>

#include "tc_pwrap_ldvt.h"
#include "register_read_write_test.h"


S32 tc_wrap_init_test(void);
S32 tc_wrap_access_test(void);
S32 tc_status_update_test(void);
S32 tc_dual_io_test(void);
S32 tc_reg_rw_test(void);
S32 tc_mux_switch_test(void);
S32 tc_man_test(void);
S32 tc_reset_pattern_test(void);
S32 tc_soft_reset_test(void);
S32 tc_high_pri_test(void);
S32 tc_spi_encryption_test(void);
S32 tc_wdt_test(void);
S32 tc_int_test(void);
S32 tc_concurrence_test(void);

static void _concurrence_eint_test_callback1(int eint_idx);

void pwrap_interrupt_on_ldvt(void);
void pwrap_lisr_normal_test(void);
void pwrap_lisr_for_wdt_test(void);
void pwrap_lisr_for_int_test(void);
DECLARE_COMPLETION(pwrap_done);


#define WRAP_ACCESS_TEST_REG DEW_WRITE_TEST
#define ldvt_follow_up
#define DEBUG_LDVT

#if 0 // don't exist in 6323
U32 WACS0_TEST_REG = DEW_CIPHER_IV4;
U32 WACS1_TEST_REG = DEW_CIPHER_IV0;
U32 WACS2_TEST_REG = DEW_CIPHER_IV1;
U32 WACS3_TEST_REG = DEW_CIPHER_IV2;
U32 WACS4_TEST_REG = DEW_CIPHER_IV3;
#endif
U32 WACS0_TEST_REG = EFUSE_VAL_0_15;
U32 WACS1_TEST_REG = EFUSE_VAL_16_31;
U32 WACS2_TEST_REG = EFUSE_VAL_32_47;

U32 eint_in_cpu0 = 0;
U32 eint_in_cpu1 = 2;
static void pwrap_delay_us(U32 us)
{
	volatile U32 delay = 100 * 1000;
	while(delay--);
}

static inline void pwrap_complete(void *arg)
{
	complete(arg);
}

//--------------wrap test API---------------------------------------------------

/* only reset wrapper and spislv, don't open dio and cipher */
static S32 _pwrap_init_partial(void)
{
	S32 sub_return=0;
	U32 rdata=0x0;
	U32 clk_sel = 0;
	U32 cg_mask = 0;
	U32 backup = 0;

	//U32 timeout=0;
	PWRAPFUC();
	//###############################
	//toggle PMIC_WRAP and pwrap_spictl reset
	//###############################
	// Turn off module clock
	cg_mask = ((1 << 20) | (1 << 27) | (1 << 28) | (1 << 29));
	backup = (~WRAP_RD32(CLK_SWCG_1)) & cg_mask; // backup for later turn on after reset
	WRAP_WR32(CLK_SETCG_1, cg_mask);
	// dummy read to add latency (to wait clock turning off)
	rdata = WRAP_RD32(PMIC_WRAP_SWRST);

	// Toggle module reset
	WRAP_WR32(PMIC_WRAP_SWRST, 1);
	rdata = WRAP_RD32(WDT_FAST_SWSYSRST);
	WRAP_WR32(WDT_FAST_SWSYSRST, (rdata | (0x1 << 11)) | (0x88 << 24));
	WRAP_WR32(WDT_FAST_SWSYSRST, (rdata & (~(0x1 << 11))) | (0x88 << 24));
	WRAP_WR32(PMIC_WRAP_SWRST, 0);

	// Turn on module clock
	WRAP_WR32(CLK_CLRCG_1, backup | (1 << 20)); // ensure cg for AP is off;

	// Turn on module clock dcm (in global_con)
	WRAP_WR32(CLK_SETCG_3, (1 << 2) | (1 << 1));

	//###############################
	// Set SPI_CK freq = 26MHz for both 6320/6323
	//###############################
	clk_sel = WRAP_RD32(CLK_SEL_0);
	WRAP_WR32(CLK_SEL_0, clk_sel | (0x3 << 24));

	//###############################
	//toggle PERI_PWRAP_BRIDGE reset
	//###############################
	//WRAP_SET_BIT(0x04,PERI_GLOBALCON_RST1);
	//WRAP_CLR_BIT(0x04,PERI_GLOBALCON_RST1);

	//###############################
	//Enable DCM
	//###############################
	WRAP_WR32(PMIC_WRAP_DCM_EN , 1);
	WRAP_WR32(PMIC_WRAP_DCM_DBC_PRD ,0);

	//###############################
	//Enable 6320 option
	//###############################
#ifdef SLV_6320
	WRAP_WR32(PMIC_WRAP_OP_TYPE , 1);
	WRAP_WR32(PMIC_WRAP_MSB_FIRST , 0);
#endif

	//###############################
	//Reset SPISLV
	//###############################
	sub_return=_pwrap_reset_spislv();
	if( sub_return != 0 )
	{
		 PWRAPERR("error,_pwrap_reset_spislv fail,sub_return=%x\n",sub_return);
		 return E_PWR_INIT_RESET_SPI;
	}
	//###############################
	// Enable WACS2
	//###############################
	WRAP_WR32(PMIC_WRAP_WRAP_EN,1);//enable wrap
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,8); //Only WACS2
	WRAP_WR32(PMIC_WRAP_WACS2_EN,1);

	//###############################
	// Set Dummy cycle to make it the same at both AP side and PMIC side
	//###############################
#ifdef SLV_6320
	// default value of 6320 dummy cycle is already 0x8
#else
	WRAP_WR32(PMIC_WRAP_RDDMY, 0xF);
#endif

	//###############################
	// Input data calibration flow
	//###############################
	sub_return = _pwrap_init_sistrobe();
	if( sub_return != 0 )
	{
		 PWRAPERR("error,DrvPWRAP_InitSiStrobe fail,sub_return=%x\n",sub_return);
		 return E_PWR_INIT_SIDLY;
	}
#if 0
	//###############################
	// SPI Waveform Configuration
	//###############################
	/* 0:safe mode, 1:6MHz, 2:12MHz
	 * no support 6MHz since the clock is too slow to transmit data
 	 * (due to RDDMY's limit -> only 4'hf)
 	 */
	sub_return = _pwrap_init_reg_clock(2);
	if( sub_return != 0)  {
		 PWRAPERR("error,_pwrap_init_reg_clock fail,sub_return=%x\n",sub_return);
		 return E_PWR_INIT_REG_CLOCK;
	}
#endif
	return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_status_update_test()
// Description : 7. Status_Update
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 _pwrap_status_update_test(void)
{
	U32 rdata;
	PWRAPFUC();

	//disable signature interrupt
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x0);
	pwrap_write(DEW_WRITE_TEST, WRITE_TEST_VALUE);
	WRAP_WR32(PMIC_WRAP_SIG_ADR, DEW_WRITE_TEST);
	WRAP_WR32(PMIC_WRAP_SIG_VALUE, 0xAA55);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x1);

	pwrap_delay_us(5000);//delay 5 seconds
	rdata = WRAP_RD32(PMIC_WRAP_SIG_ERRVAL) & 0xFFFF;
	if (rdata != WRITE_TEST_VALUE) {
		PWRAPERR("_pwrap_status_update_test error, rdata=%x\n", rdata);
		return -1;
	}

	WRAP_WR32(PMIC_WRAP_SIG_VALUE,WRITE_TEST_VALUE); //the same as write test
	//clear sig_error interrupt flag bit
	WRAP_WR32(PMIC_WRAP_INT_CLR, 1 << 1);

	//enable signature interrupt
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x7fffffff);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x0);
	WRAP_WR32(PMIC_WRAP_SIG_ADR ,DEW_CRC_VAL);

  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_wrap_access_test()
// Description : 3. SPI_Access_1  6. WACS_1
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 _pwrap_wrap_access_test(void)
{
	U32 rdata = 0;
	U32 res = 0;
	U32 reg_value_backup = 0;
	U32 return_value = 0;
	PWRAPFUC();

	reg_value_backup = WRAP_RD32(PMIC_WRAP_INT_EN);
	WRAP_CLR_BIT(1 << 1, PMIC_WRAP_INT_EN); //clear sig_error interrupt test

	//###############################
	// Read/Write test using WACS0
	//###############################
	PWRAPLOG("start test WACS0\n");
	return_value = pwrap_wacs0(0, DEW_READ_TEST, 0, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("read test error(using WACS0),return_value=%x, rdata=%x\n",
			return_value, rdata);
		res += 1;
	}

	rdata = 0;
	pwrap_wacs0(1, WRAP_ACCESS_TEST_REG, 0x1234, &rdata);
	return_value = pwrap_wacs0(0, WRAP_ACCESS_TEST_REG, 0, &rdata);
	if (rdata != 0x1234) {
		PWRAPERR("write test error(using WACS0),return_value=%x, rdata=%x\n", 
			return_value, rdata);
		res += 1;
	}

	//###############################
	// Read/Write test using WACS1
	//###############################
	PWRAPLOG("start test WACS1\n");
	return_value = pwrap_wacs1(0, DEW_READ_TEST, 0, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("read test error(using WACS1),return_value=%x, rdata=%x\n", 
			return_value, rdata);
		res += 1;
	}
	rdata = 0;
	pwrap_wacs1(1, WRAP_ACCESS_TEST_REG, 0x5678, &rdata);
	return_value = pwrap_wacs1(0, WRAP_ACCESS_TEST_REG, 0, &rdata);
	if (rdata != 0x5678) {
		PWRAPERR("write test error(using WACS1),return_value=%x, rdata=%x\n", 
			return_value, rdata);
		res += 1;
	}
	rdata = 0;

	//###############################
	// Read/Write test using WACS2
	//###############################
	PWRAPLOG("start test WACS2\n");
	return_value = pwrap_read(DEW_READ_TEST,&rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("read test error(using WACS2),return_value=%x, rdata=%x\n", 
			return_value, rdata);
		res += 1;
	}

	rdata = 0;
	pwrap_write(WRAP_ACCESS_TEST_REG, 0xABCD);
	return_value = pwrap_wacs2(0, WRAP_ACCESS_TEST_REG, 0, &rdata);
	if (rdata != 0xABCD) {
		PWRAPERR("write test error(using WACS2),return_value=%x, rdata=%x\n", 
			return_value, rdata);
		res += 1;
	}

	WRAP_WR32(PMIC_WRAP_INT_EN, reg_value_backup);
	return res;
}

//--------------------------------------------------------
//    Function : _pwrap_man_access_test()
// Description : 9. MAN
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 _pwrap_man_access_test(void)
{
/*  Must be in Dual IO mode, or the test will fail */
	U32 rdata = 0;
	U32 res = 0;
	U32 return_value = 0;
	U32 reg_value_backup;

	PWRAPFUC();
	//###############################
	// Read/Write test using manual mode
	//###############################
	reg_value_backup = WRAP_RD32(PMIC_WRAP_STAUPD_GRPEN);
	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN, reg_value_backup & (~(0x1)));
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x1); // 20us

	return_value = _pwrap_manual_modeAccess(0, DEW_READ_TEST, 0, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		/* TERR="Error: [ReadTest] fail, rdata=%x, exp=0x5aa5", rdata */
		PWRAPERR("read test error(using manual mode),return_value=%x, rdata=%x", 
			return_value, rdata);
		res += 1;
	}

	rdata = 0;
	_pwrap_manual_modeAccess(1, WRAP_ACCESS_TEST_REG, 0x1234, &rdata);
	return_value = _pwrap_manual_modeAccess(0, WRAP_ACCESS_TEST_REG, 0, &rdata);

	if(rdata != 0x1234) {
		/* TERR="Error: [WriteTest] fail, rdata=%x, exp=0x1234", rdata*/
		PWRAPERR("write test error(using manual mode),return_value=%x, rdata=%x",
			return_value, rdata);
		res += 1;
	}
	_pwrap_switch_mux(0);// switch to wrap mode

	//MAN
	_pwrap_AlignCRC();

	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN,reg_value_backup);

	return res;
}

S32 tc_wrap_init_test(void)
{
	U32 ret = 0;
	U32 res = 0;
	U32 regValue = 0;
	struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
	pwrap_obj->complete = pwrap_complete;
	pwrap_obj->context = &pwrap_done;
	//CTP_GetRandom(&u4Random, 200);

	ret = pwrap_init();
	if (ret == 0)
		PWRAPLOG("wrap_init test pass.\n");
	else {
		PWRAPLOG("error:wrap_init test fail.return_value=%d.\n",ret);
		res += 1;
	}

#ifdef DEBUG_LDVT
	regValue = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG =%x\n",regValue);
	//regValue=WRAP_RD32(PERI_PWRAP_BRIDGE_INT_FLG);
	// PWRAPLOG("PERI_PWRAP_BRIDGE_INT_FLG =%x\n",regValue);
	regValue = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG =%x\n",regValue);
	//regValue=WRAP_RD32(PERI_PWRAP_BRIDGE_WDT_FLG);
	//PWRAPLOG("PERI_PWRAP_BRIDGE_WDT_FLG =%x\n",regValue);
#endif

	ret = _pwrap_status_update_test();
	if (ret == 0)
		PWRAPLOG("_pwrap_status_update_test pass.\n");
	else {
		PWRAPLOG("error:_pwrap_status_update_test fail.\n");
		res += 1;
	}

#ifdef DEBUG_CTP
	regValue = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG =%x\n",regValue);
	//regValue=WRAP_RD32(PERI_PWRAP_BRIDGE_INT_FLG);
	//  PWRAPLOG("PERI_PWRAP_BRIDGE_INT_FLG =%x\n",regValue);
	regValue = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG =%x\n",regValue);
	//regValue=WRAP_RD32(PERI_PWRAP_BRIDGE_WDT_FLG);
	//  PWRAPLOG("PERI_PWRAP_BRIDGE_WDT_FLG =%x\n",regValue);
#endif

	//pwrap_interrupt_on_ldvt();
	if (res != 0)
		PWRAPLOG("error:tc_wrap_init_test fail.\n");
	else
		PWRAPLOG("tc_wrap_init_test pass.\n");

	return res;
}

/* 3. SPI_Access_1
 * 6. WACS_1
 */
S32 tc_wrap_access_test(void)
{
	U32 res = 0;

	res = _pwrap_wrap_access_test();
	if (res == 0)
		PWRAPLOG("WRAP_UVVF_WACS_TEST pass.\n");
	else
		PWRAPLOG("WRAP_UVVF_WACS_TEST fail.res=%d\n",res);

	return res;
}

/* 7. Status_Update */
S32 tc_status_update_test(void)
{
	U32 res = 0;

	res = _pwrap_status_update_test();
	if(res == 0)
		PWRAPLOG("_pwrap_status_update_test pass.\n");
	else
		PWRAPLOG("_pwrap_status_update_test fail.res=%d\n",res);

	return res;
}

/* 5. SPI_Access_2 */
S32 tc_dual_io_test(void)
{
	U32 res = 0;

	//enable dual io mode
	PWRAPLOG("enable dual io mode.\n");
	// XXX: Don't we disable WRAP_EN first?
	_pwrap_switch_dio(1);
	res = _pwrap_wrap_access_test();

	if (res == 0)
		PWRAPLOG("_pwrap_wrap_access_test pass.\n");
	else
		PWRAPLOG("_pwrap_wrap_access_test fail.res=%d\n", res);

	//disable dual io mode
	PWRAPLOG("disable dual io mode.\n");
	_pwrap_switch_dio(0);
	res = _pwrap_wrap_access_test();

	if (res == 0)
		PWRAPLOG("_pwrap_wrap_access_test pass.\n");
	else
		PWRAPLOG("_pwrap_wrap_access_test fail.res=%d\n", res);

	if (res == 0)
		PWRAPLOG("tc_dual_io_test pass.\n");
	else
		PWRAPLOG("tc_dual_io_test fail.res=%d\n", res);

	return res;
}

U32 RegWriteValue[] = {0, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA};

/* 1. APB_RW */
/* XXX: Just use the codeviser to check and read/write registers */
S32 tc_reg_rw_test(void)
{
	U32 i, j;
	U32 pmic_wrap_reg_size = 0;
	U32 DEW_reg_size = 0;
	U32 reg_write_size = 0;
	U32 test_result = 0;
	U32 ret = 0;
	U32 reg_data = 0;

	PWRAPFUC();

	pmic_wrap_reg_size = sizeof(PMIC_WRAP_reg_tbl_6572) / 
			     sizeof(PMIC_WRAP_reg_tbl_6572[0]);

	DEW_reg_size = sizeof(DEW_reg_tbl_6323) / sizeof(DEW_reg_tbl_6323[0]);
	reg_write_size = sizeof(RegWriteValue) / sizeof(RegWriteValue[0]);

	PWRAPLOG("pmic_wrap_reg_size=%d\n", pmic_wrap_reg_size);
	PWRAPLOG("DEW_reg_size=%d\n", DEW_reg_size);

	PWRAPLOG("start test PMIC_WRAP_reg_tbl_6572:default value test\n");
	// toggle SWRST
	WRAP_WR32(PMIC_WRAP_SWRST, 1);
	WRAP_WR32(PMIC_WRAP_SWRST, 0);

	for (i = 0; i < pmic_wrap_reg_size; i++) {
		//Only R/W or RO should do default value test
		if (REG_TYP_WO != PMIC_WRAP_reg_tbl_6572[i][3]) {
#if 0
			PWRAPLOG("Register %x Default Value test DEF %x,i=%d\n", 
				PMIC_WRAP_reg_tbl_6572[i][0],
				PMIC_WRAP_reg_tbl_6572[i][1], i);
#endif

			if ((WRAP_RD32(PMIC_WRAP_BASE + PMIC_WRAP_reg_tbl_6572[i][0])
				!= PMIC_WRAP_reg_tbl_6572[i][1])) {
				PWRAPLOG("Register %x Default Value test fail. DEF %x, read %x \r\n",
					PMIC_WRAP_reg_tbl_6572[i][0] + PMIC_WRAP_BASE, 
					PMIC_WRAP_reg_tbl_6572[i][1], 
					WRAP_RD32(PMIC_WRAP_BASE + PMIC_WRAP_reg_tbl_6572[i][0]));
				test_result++;
			}
		}
	}

	PWRAPLOG("start test PMIC_WRAP_reg_tbl_6572:R/W test\n");
	for (i = 0; i < pmic_wrap_reg_size; i++) {
		if(REG_TYP_RW == PMIC_WRAP_reg_tbl_6572[i][3]) {
			for (j = 0; j < reg_write_size; j++) {
#if 0
				PWRAPLOG("Register %x  value %x\n", 
					PMIC_WRAP_reg_tbl_6572[i][0] + PMIC_WRAP_BASE, 
					RegWriteValue[j]);
#endif
				WRAP_WR32((PMIC_WRAP_reg_tbl_6572[i][0] + PMIC_WRAP_BASE), 
					RegWriteValue[j] & PMIC_WRAP_reg_tbl_6572[i][2]);
				if((WRAP_RD32(PMIC_WRAP_reg_tbl_6572[i][0] + PMIC_WRAP_BASE))
					!= (RegWriteValue[j] & PMIC_WRAP_reg_tbl_6572[i][2])) {
					PWRAPLOG("Register %x R/W test fail. write %x, read %x \r\n", 
					PMIC_WRAP_reg_tbl_6572[i][0] + PMIC_WRAP_BASE,
					RegWriteValue[j] & PMIC_WRAP_reg_tbl_6572[i][2],
					WRAP_RD32(PMIC_WRAP_reg_tbl_6572[i][0] + PMIC_WRAP_BASE));
					test_result++;
				}
			}
		}
	}

	PWRAPLOG("start test DEW_reg_tbl_6323:default value test\n");
	/* reset spislv register */
	ret = _pwrap_init_partial();

	for (i = 0; i < DEW_reg_size; i++)
	{
		//Only R/W or RO should do default value test
		if (REG_TYP_WO != DEW_reg_tbl_6323[i][3])
		{
			_pwrap_wacs2_nochk(0, DEW_reg_tbl_6323[i][0], 0, &reg_data);

			if ((reg_data != DEW_reg_tbl_6323[i][1]))
			{
				PWRAPLOG("Register %x Default Value test fail. DEF %x, read %x \r\n",
					DEW_reg_tbl_6323[i][0],  DEW_reg_tbl_6323[i][1], reg_data);
				test_result++;
			}
		}
	}

	PWRAPLOG("start test DEW_reg_tbl_6323:R/W test\n");
	for (i = 0; i < DEW_reg_size; i++)
	{
		/* ignore registers which change R/W behavior */
		if ((DEW_reg_tbl_6323[i][0] == DEW_DIO_EN) ||
			(DEW_reg_tbl_6323[i][0] == DEW_CIPHER_MODE) ||
			(DEW_reg_tbl_6323[i][0] == DEW_RDDMY_NO))
			continue;

		if (REG_TYP_RW == DEW_reg_tbl_6323[i][3])
		{
			for (j = 0; j < reg_write_size; j++)
			{
				reg_data = 0;

				_pwrap_wacs2_nochk(1, DEW_reg_tbl_6323[i][0],
					 RegWriteValue[j] & DEW_reg_tbl_6323[i][2], 0);

				_pwrap_wacs2_nochk(0, DEW_reg_tbl_6323[i][0], 0, &reg_data);

				if (reg_data != (RegWriteValue[j] & DEW_reg_tbl_6323[i][2]))
				{
					PWRAPLOG("Register %x R/W test fail. write %x, read %x \r\n",
						DEW_reg_tbl_6323[i][0],
						(RegWriteValue[j] & DEW_reg_tbl_6323[i][2]), 
						reg_data);
					test_result++;
				}
			}
		}
	}

	if(test_result == 0)
		PWRAPLOG("tc_reg_rw_test pass.\n");
	else
		PWRAPLOG("tc_reg_rw_test fail.res=%d\n", test_result);

	return test_result;
}

/* 8. Mux_Switching */
/* 9. Man */
S32 tc_mux_switch_test(void)
{
	U32 res = 0;

	res = _pwrap_man_access_test();

	if(res == 0)
		PWRAPLOG("tc_mux_switch_test pass.\n");
	else
		PWRAPLOG("tc_mux_switch_test fail.res=%d\n", res);

	return res;
}

/* 4. SPI_Reset */
S32 tc_reset_pattern_test(void)
{
	U32 res = 0;
	U32 rdata = 0;


	res = pwrap_init();
	res = _pwrap_wrap_access_test();

	pwrap_write(DEW_WRITE_TEST, 0x1234);

	/* reset spislv register */
	res = _pwrap_init_partial();

	rdata = _pwrap_wacs2_nochk(0, DEW_WRITE_TEST, 0, &rdata);
	if (rdata != 0)
		res = -1;

	if(res == 0)
		PWRAPLOG("tc_reset_pattern_test pass.\n");
	else
		PWRAPLOG("tc_reset_pattern_test fail.res=%d\n",res);

	return res;
}

/* 2. Module_Soft_Reset */
S32 tc_soft_reset_test(void)
{
	U32 res = 0;
	U32 rdata = 0;

	//---do wrap init and wrap access test-----------------------------------
	res = pwrap_init();
	res = _pwrap_wrap_access_test();
	//---reset wrap-------------------------------------------------------------
	rdata = WRAP_RD32(PMIC_WRAP_DIO_EN);
	PWRAPLOG("DEW_DIO_EN before reset: 0x%x\n", rdata);

	PWRAPLOG("start reset wrapper\n");

	//toggle SWRST bit
	WRAP_WR32(PMIC_WRAP_SWRST, 1);
	WRAP_WR32(PMIC_WRAP_SWRST, 0);

	rdata = WRAP_RD32(PMIC_WRAP_DIO_EN);
	PWRAPLOG("DEW_DIO_EN after reset: 0x%x\n", rdata);

	PWRAPLOG("the wrap access test should be fail after reset,before init\n");
	res = _pwrap_wrap_access_test();
	if (res == 0)
		PWRAPLOG("_pwrap_wrap_access_test pass.\n");
	else
		PWRAPLOG("_pwrap_wrap_access_test fail.res=%d\n",res);

	PWRAPLOG("the wrap access test should be pass after reset and wrap init\n");

	//---do wrap init and wrap access test-----------------------------------
	res = pwrap_init();
	res = _pwrap_wrap_access_test();

	if (res == 0)
		PWRAPLOG("_pwrap_wrap_access_test pass.\n");
	else
		PWRAPLOG("_pwrap_wrap_access_test fail.res=%d\n",res);
	if (res == 0)
		PWRAPLOG("tc_soft_reset_test pass.\n");
	else
		PWRAPLOG("tc_soft_reset_test fail.res=%d\n",res);

	return res;
}

S32 tc_high_pri_test(void)
{
	U32 res = 0;
	U32 rdata = 0;
	U64 pre_time = 0;
	U64 post_timer = 0;
	U64 enable_staupd_time = 0;
	U64 disable_staupd_time = 0;
	U64 GPT2_COUNT_value = 0;

// TODO: figure out what the following code is doing
	//----enable status updata and do wacs0-------------------------------
	PWRAPLOG("enable status updata and do wacs0,record the cycle\n");
	//0x1:20us,for concurrence test,MP:0x5;  //100us
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x1);
	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN,0xff);

	//###############################
	// Read/Write test using WACS0
	//###############################
	//perfmon_start();//record time start,ldvt_follow_up
	GPT2_COUNT_value = WRAP_RD32(APMCU_GPTIMER_BASE + 0x0028);
	pre_time = sched_clock();
	PWRAPLOG("GPT2_COUNT_value=%lld pre_time=%lld\n", GPT2_COUNT_value,pre_time);
	pwrap_wacs0(0, DEW_READ_TEST, 0, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("read test error(using WACS0),error code=%x, rdata=%x\n", 1, rdata);
		res += 1;
	}

	//perfmon_end();
	post_timer = sched_clock();
	enable_staupd_time = post_timer - pre_time;
	PWRAPLOG("pre_time=%lld post_timer=%lld\n", pre_time,post_timer);
	PWRAPLOG("pwrap_wacs0 enable_staupd_time=%lld\n", enable_staupd_time);

	//----disable status updata and do wacs0-------------------------------------
	PWRAPLOG("disable status updata and do wacs0,record the cycle\n");
	//0x1:20us,for concurrence test,MP:0x5;  //100us
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0xF);
	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN,0x00);

	//###############################
	// Read/Write test using WACS0
	//###############################
	//perfmon_start();
	pre_time = sched_clock();
	pwrap_wacs0(0, DEW_READ_TEST, 0, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("read test error(using WACS0),error code=%x, rdata=%x\n", 1, rdata);
		res += 1;
	}

	//perfmon_end();
	post_timer=sched_clock();
	disable_staupd_time = post_timer - pre_time;
	PWRAPLOG("pre_time=%lld post_timer=%lld\n", pre_time,post_timer);
	PWRAPLOG("pwrap_wacs0 disable_staupd_time=%lld\n", disable_staupd_time);
	if (disable_staupd_time <= enable_staupd_time)
		PWRAPLOG("tc_high_pri_test pass.\n");
	else
		PWRAPLOG("tc_high_pri_test fail.res=%d\n",res);

	return res;
}

/* 10. SPI_Encryption */
S32 tc_spi_encryption_test(void)
{
	U32 res = 0;

	// mask WDT interrupt
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x7ffffffe);

	// disable status update,to check the wave on oscilloscope
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x0);  //0x0:disable
	//disable dio mode,single io wave
	// XXX: Don't we disable WRAP_EN first?
	_pwrap_switch_dio(0);

	//###############################
	// disable Encryption
	//###############################
	res = _pwrap_disable_cipher();//set breakpoint here
	if (res != 0) {
		PWRAPERR("disable Encryption error,error code=%x, rdata=%x", 0x21, res);
		return -EINVAL;
	}
	res = _pwrap_wrap_access_test();

	//###############################
	// enable Encryption
	//###############################
	res = _pwrap_enable_cipher();
	if (res != 0) {
		PWRAPERR("Enable Encryption error,error code=%x, res=%x", 0x21, res);
		return -EINVAL;
	}
	res = _pwrap_wrap_access_test();

	if (res == 0)
		PWRAPLOG("tc_spi_encryption_test pass.\n");
	else
		PWRAPLOG("tc_spi_encryption_test fail.res=%d\n",res);

	return res;
}

//-------------------irq init start-------------------------------------
/*
 * choose_lisr=0: normal test
 * choose_lisr=1: watch dog test
 * choose_lisr=2: interrupt test
 */
//#define CHOOSE_LISR     2
#define NORMAL_TEST     0
#define WDT_TEST        1
#define INT_TEST        2

static U32 choose_lisr = NORMAL_TEST;

static U32 wrapper_lisr_count_cpu0=0;
static U32 wrapper_lisr_count_cpu1=0;

// global value for int test
static U32 int_test_bit = 0;
static U32 wait_int_flag = 0;
static U32 int_test_fail_count = 0;

//global value for wdt test
static U32 wdt_test_bit = 0;
static U32 wait_for_wdt_flag = 0;
static U32 wdt_test_fail_count = 0;

void pwrap_interrupt_on_ldvt(void)
{
	struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
	//PWRAPFUC();

	switch(choose_lisr)
	{
	case NORMAL_TEST:
		pwrap_lisr_normal_test();
		break;
	case WDT_TEST:
		pwrap_lisr_for_wdt_test();
		break;
	case INT_TEST:
		pwrap_lisr_for_int_test();
		break;
	}

	//PWRAPLOG("before complete\n");
	if (pwrap_obj->complete)
		pwrap_obj->complete(pwrap_obj->context);
	//PWRAPLOG("after complete\n");
}

void pwrap_lisr_normal_test(void)
{
	U32 reg_int_flg = 0;
	U32 reg_wdt_flg = 0;
	PWRAPFUC();

//#ifndef ldvt_follow_up
	if (raw_smp_processor_id() == 0)
		wrapper_lisr_count_cpu0++;
	else if (raw_smp_processor_id() == 1)
		wrapper_lisr_count_cpu1++;
//#endif

	reg_int_flg = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG=0x%x.\n", reg_int_flg);
	reg_wdt_flg = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG=0x%x.\n", reg_wdt_flg);
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);//clear watch dog
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0xffffffff);
	WRAP_WR32(PMIC_WRAP_INT_CLR, reg_int_flg);
}

void pwrap_lisr_for_wdt_test(void)
{
	U32 reg_int_flg = 0;
	U32 reg_wdt_flg = 0;
	PWRAPFUC();

	reg_int_flg = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG=0x%x.\n", reg_int_flg);
	reg_wdt_flg = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG=0x%x.\n", reg_wdt_flg);

	PWRAPLOG("wdt_test_bit=%d\n", wdt_test_bit);
	if ((reg_int_flg & 0x1) != 0) {
		//dispatch_WDT();
		if((reg_wdt_flg & (1 << wdt_test_bit)) != 0) {
			PWRAPLOG("watch dog test:recieve the right wdt.\n");
			wait_for_wdt_flag = 1;
			//clear watch dog and interrupt
			WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
			//WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,0xffffff);
		} else {
			PWRAPLOG("watch dog test fail:recieve the wrong wdt.\n");
			wdt_test_fail_count++;
			//clear the unexpected watch dog and interrupt
			WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
			WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 1 << wdt_test_bit);
		}
	}

	WRAP_WR32(PMIC_WRAP_INT_CLR, reg_int_flg);
	reg_int_flg = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG=0x%x.\n", reg_int_flg);
	reg_wdt_flg=WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG=0x%x.\n", reg_wdt_flg);
}

void pwrap_lisr_for_int_test(void)
{
	U32 reg_int_flg = 0;
	U32 reg_wdt_flg = 0;
	PWRAPFUC();

	reg_int_flg = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG=0x%x.\n", reg_int_flg);
	reg_wdt_flg = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG=0x%x.\n", reg_wdt_flg);

	//-------------------------interrupt test---------------
	PWRAPLOG("int_test_bit=%d\n", int_test_bit);
	if ((reg_int_flg & (1 << int_test_bit)) != 0) {
		PWRAPLOG(" int test:recieve the right pwrap interrupt.\n");
		wait_int_flag = 1;
	} else {
		PWRAPLOG(" int test fail:recieve the wrong pwrap interrupt.\n");
		int_test_fail_count++;
	}

	WRAP_WR32(PMIC_WRAP_INT_CLR, reg_int_flg);
	reg_int_flg = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("PMIC_WRAP_INT_FLG=0x%x.\n", reg_int_flg);
	reg_wdt_flg = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG=0x%x.\n", reg_wdt_flg);

	//for int test bit[1]
	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN, 0);
}

//-------------------irq init end-------------------------------------------

//-------------------watch dog test start-------------------------------------
// U32 watch_dog_test_reg=DEW_CIPHER_IV4;
U32 watch_dog_test_reg = DEW_WRITE_TEST;

//static U32 wrap_WDT_flg=0;

S32 _wdt_test_disable_other_int(void)
{
	//disable other interrupt bit
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x1);

	return 0;
}

//[1]: HARB_WACS0_ALE: HARB to WACS0 ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,and send a WACS0 write command
S32 _wdt_test_bit1(void)
{
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 1;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

	//disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 0x1ff);
	WRAP_CLR_BIT(1 << wdt_test_bit, PMIC_WRAP_HIPRIO_ARB_EN);

	pwrap_wacs0(1, watch_dog_test_reg, 0x1234, 0);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[2]: HARB_WACS1_ALE: HARB to WACS1 ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,and send a WACS1 write command
S32 _wdt_test_bit2(void)
{
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 2;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

	//disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 0x1ff);
	WRAP_CLR_BIT(1 << wdt_test_bit, PMIC_WRAP_HIPRIO_ARB_EN);

	pwrap_wacs1(1, watch_dog_test_reg, 0x1234, 0);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[3]: HARB_WACS2_ALE: HARB to WACS2 ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,and send a WACS2 write command
S32 _wdt_test_bit3(void)
{
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 3;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

	//disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 0x1ff);
	WRAP_CLR_BIT(1 << wdt_test_bit, PMIC_WRAP_HIPRIO_ARB_EN);

	pwrap_write(watch_dog_test_reg, 0x1234);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}
#if 0
//[5]: HARB_ERC_ALE: HARB to ERC ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,do event test
S32 _wdt_test_bit5( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=6;
  wait_for_wdt_flag=0;
  //disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0x1ff);
  WRAP_CLR_BIT(1<<wdt_test_bit,PMIC_WRAP_HIPRIO_ARB_EN);
  //similar to event  test case
  //WRAP_WR32(PMIC_WRAP_EVENT_STACLR , 0xffff);
#if 0
  res=pwrap_wacs0(1, DEW_EVENT_TEST, 0x1, &rdata);
#endif
  wait_for_completion(&pwrap_done);
  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  return 0;
}
#endif
//[6]: HARB_STAUPD_ALE: HARB to STAUPD ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,and do status update test
S32 _wdt_test_bit6(void)
{
	//U32 rdata = 0;
	U32 res = 0;
	U32 reg_rdata = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 6;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

	//disable other wdt bit
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 1 << wdt_test_bit);
	reg_rdata = WRAP_RD32(PMIC_WRAP_WDT_SRC_EN);
	PWRAPLOG("PMIC_WRAP_WDT_SRC_EN=0x%x\n", reg_rdata);
	
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x0); // disable auto status update

	//disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 0x1ff);
	WRAP_CLR_BIT(1 << 5, PMIC_WRAP_HIPRIO_ARB_EN);

	//similar to status updata test case
	//pwrap_wacs0(1, DEW_WRITE_TEST, 0x55AA, &rdata);
	//WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
	//WRAP_WR32(PMIC_WRAP_SIG_VALUE,0xAA55);

	// manually trigger status update.
	WRAP_WR32(PMIC_WRAP_STAUPD_MAN_TRIG, 0x1);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	//WRAP_WR32(PMIC_WRAP_SIG_VALUE, 0x55AA);//the same as write test
	return res;
}
#if 0
//[7]: PWRAP_PERI_ALE: HARB to PWRAP_PERI_BRIDGE ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,and send a WACS3 write command
S32 _wdt_test_bit7( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=7;
  wait_for_wdt_flag=0;
  //disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0x1ff);
  WRAP_CLR_BIT(1<<wdt_test_bit,PMIC_WRAP_HIPRIO_ARB_EN);
  //do wacs3
  //pwrap_wacs3(1, watch_dog_test_reg, 0x55AA, &rdata);
  wait_for_completion(&pwrap_done);

  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  return 0;
}

//[8]: HARB_EINTBUF_ALE: HARB to EINTBUF ALE timeout monitor
//disable the corresponding bit in HIPRIO_ARB_EN,and send a eint interrupt
S32 _wdt_test_bit8( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=8;
  wait_for_wdt_flag=0;
  #ifdef ENABLE_KEYPAD_ON_LDVT
    //kepadcommand
    *((volatile kal_uint16 *)(KP_BASE + 0x1c)) = 0x1;
    kpd_init();
    initKpdTest();
  #endif
  //disable corresponding bit in PMIC_WRAP_HIPRIO_ARB_EN
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0x1ff);
  WRAP_CLR_BIT(1<<wdt_test_bit,PMIC_WRAP_HIPRIO_ARB_EN);
  wait_for_completion(&pwrap_done);
  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  return 0;
}

//[9]: WRAP_HARB_ALE: WRAP to HARB ALE timeout monitor
//disable RRARB_EN[0],and do eint test
S32 _wdt_test_bit9( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0,i=0;
  PWRAPFUC();
  wdt_test_bit=9;
  wait_for_wdt_flag=0;



#ifdef ENABLE_EINT_ON_LDVT
  //eint_init();
  //_concurrence_eint_test_code(eint_in_cpu0);
  //eint_unmask(eint_in_cpu0);
  //Delay(500);
#endif
  //disable wrap_en
  //WRITE_REG(0xFFFFFFFF, EINT_MASK_CLR);



  //Delay(1000);
   for (i=0;i<(300*20);i++);
   wait_for_completion(&pwrap_done);
  while(wait_for_wdt_flag!=1);
  //WRAP_WR32(PMIC_WRAP_WRAP_EN ,1);//recover
  PWRAPLOG("%s pass\n", __FUNCTION__);
  return 0;
}

//[10]: PWRAP_AG_ALE#1: PWRAP to AG#1 ALE timeout monitor
//disable RRARB_EN[1],and do keypad test
S32 _wdt_test_bit10( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=10;
  wait_for_wdt_flag=0;
#ifdef ENABLE_KEYPAD_ON_LDVT
  //kepad command
  *((volatile kal_uint16 *)(KP_BASE + 0x1c)) = 0x1;
  kpd_init();
  initKpdTest();
#endif
  //disable wrap_en
  //WRAP_CLR_BIT(1<<1 ,PMIC_WRAP_RRARB_EN);
  wait_for_completion(&pwrap_done);
  //push keypad key
  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  return 0;
}

//[11]: PWRAP_AG_ALE#2: PWRAP to AG#2 ALE timeout monitor
//disable RRARB_EN[0],and do eint test
S32 _wdt_test_bit11( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=11;
  wait_for_wdt_flag=0;
  //kepadcommand
#ifdef ENABLE_EINT_ON_LDVT
  //eint_init();
  //_concurrence_eint_test_code(eint_in_cpu0);
  //eint_unmask(eint_in_cpu0);
#endif

  //disable wrap_en
  //WRAP_CLR_BIT(1<<1 ,PMIC_WRAP_RRARB_EN);
  wait_for_completion(&pwrap_done);
  //push keypad key
  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  return 0;
}
#endif
//[12]: WRAP_HARB_ALE: WRAP to HARB ALE timeout monitor
//  Disable WRAP_EN and set a WACS0 read command
S32 _wdt_test_bit12(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 12;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	//_pwrap_switch_mux(1);//manual mode
	//WRAP_WR32(PMIC_WRAP_MUX_SEL , 0);
	WRAP_WR32(PMIC_WRAP_WRAP_EN, 0);//disble WRAP_EN
	//WRAP_WR32(PMIC_WRAP_MAN_EN , 1);//enable manual

	pwrap_wacs0(0, watch_dog_test_reg, 0, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	//_pwrap_switch_mux(0);// recover
	return res;
}

//[13]: MUX_WRAP_ALE: MUX to WRAP ALE timeout monitor
// set MUX to manual mode ,enable WRAP_EN and set a WACS0 write command
S32 _wdt_test_bit13(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 13;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	_pwrap_switch_mux(1);//manual mode
	//WRAP_WR32(PMIC_WRAP_MUX_SEL , 0);
	WRAP_WR32(PMIC_WRAP_WRAP_EN, 1);//enable wrap
	//WRAP_WR32(PMIC_WRAP_MAN_EN , 1);//enable manual

	pwrap_wacs0(0, watch_dog_test_reg, 0, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	_pwrap_switch_mux(0);// recover
	return res;
}

//[14]: MUX_MAN_ALE: MUX to MAN ALE timeout monitor
//MUX to MAN ALE:set MUX to wrap mode and send manual command
S32 _wdt_test_bit14(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 14;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	_pwrap_switch_mux(0);//wrap mode
	//WRAP_WR32(PMIC_WRAP_WRAP_EN , 0);//enable wrap
	WRAP_WR32(PMIC_WRAP_MAN_EN, 1);//enable manual

	_pwrap_manual_mode(0,  OP_IND, 0, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	_pwrap_switch_mux(1);//
	return res;
}

//[16]: HARB_WACS0_DLE: HARB to WACS0 DLE timeout monitor
//Disable MUX, and send a read command with WACS0
S32 _wdt_test_bit16(void)
{
	U32 rdata = 0;
	U32 reg_rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 16;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	//disable other wdt bit
	//WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,0);
	//WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,1<<wdt_test_bit);
	reg_rdata = WRAP_RD32(PMIC_WRAP_WDT_SRC_EN);
	PWRAPLOG("PMIC_WRAP_WDT_SRC_EN=%x.\n", reg_rdata);

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 1 << 1);//enable wrap
	reg_rdata = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
	PWRAPLOG("PMIC_WRAP_WDT_SRC_EN=%x.\n", reg_rdata);
	//set status update period to the max value,or disable status update
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0xF);

	_pwrap_switch_mux(1);//manual mode
	WRAP_WR32(PMIC_WRAP_WRAP_EN , 1);//enable wrap

	//read command
	pwrap_wacs0(0, watch_dog_test_reg, 0, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	_pwrap_switch_mux(0);//recover
	return res;
}

//[17]: HARB_WACS1_DLE: HARB to WACS1 DLE timeout monitor
//Disable MUX,and send a read command with WACS1
S32 _wdt_test_bit17(void)
{
	U32 rdata = 0;
	U32 reg_rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 17;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 1 << 2);//enable wrap
	reg_rdata = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
	PWRAPLOG("PMIC_WRAP_HIPRIO_ARB_EN=%x.\n", reg_rdata);
	//set status update period to the max value,or disable status update
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x0);

	_pwrap_switch_mux(1);//manual mode
	WRAP_WR32(PMIC_WRAP_WRAP_EN, 1);//enable wrap

	//read command
	pwrap_wacs1(0, watch_dog_test_reg, 0, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	_pwrap_switch_mux(0);//recover
	return res;
}

//[18]: HARB_WACS2_DLE: HARB to WACS2 DLE timeout monitor
//Disable MUX,and send a read command with WACS2
S32 _wdt_test_bit18(void)
{
	U32 rdata = 0;
	U32 reg_rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 18;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	//disable other wdt bit
	//WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
	//WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 1 << wdt_test_bit);

	reg_rdata = WRAP_RD32(PMIC_WRAP_WDT_SRC_EN);
	PWRAPLOG("PMIC_WRAP_HIPRIO_ARB_EN=%x.\n", reg_rdata);

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 1 << 3);
	reg_rdata = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
	PWRAPLOG("PMIC_WRAP_HIPRIO_ARB_EN=%x.\n", reg_rdata);
	//set status update period to the max value,or disable status update
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0xF);

	reg_rdata = WRAP_RD32(PMIC_WRAP_STAUPD_PRD);
	PWRAPLOG("PMIC_WRAP_STAUPD_PRD=%x.\n", reg_rdata);

	_pwrap_switch_mux(1);//manual mode
	WRAP_WR32(PMIC_WRAP_WRAP_EN, 1);//enable wrap
	//clear INT
	WRAP_WR32(PMIC_WRAP_INT_CLR, 0xFFFFFFFF);

	//read command
	pwrap_read(watch_dog_test_reg, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	_pwrap_switch_mux(0);//recover
	return res;
}
#if 0
//[19]: HARB_ERC_DLE: HARB to ERC DLE timeout monitor
//HARB to staupda DLE:disable event,write de_wrap event test,then swith mux to manual mode ,enable wrap_en enable event
//similar to bit5
S32 _wdt_test_bit19( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=19;
  wait_for_wdt_flag=0;
  //disable event
  //WRAP_WR32(PMIC_WRAP_EVENT_IN_EN , 0);

  //do event test
  //WRAP_WR32(PMIC_WRAP_EVENT_STACLR , 0xffff);
#if 0
  res=pwrap_wacs0(1, DEW_EVENT_TEST, 0x1, &rdata);
#endif
  //disable mux
  _pwrap_switch_mux(1);//manual mode
  WRAP_WR32(PMIC_WRAP_WRAP_EN , 1);//enable wrap
  //enable event
  //WRAP_WR32(PMIC_WRAP_EVENT_IN_EN , 1);
  wait_for_completion(&pwrap_done);

  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  _pwrap_switch_mux(0);//recover
  return 0;
}

//[20]: HARB_STAUPD_DLE: HARB to STAUPD DLE timeout monitor
//  HARB to staupda DLE:disable MUX,then send a read commnad ,and do status update test
//similar to bit6
S32 _wdt_test_bit20( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=20;
  wait_for_wdt_flag=0;
  _pwrap_switch_mux(1);//manual mode
  WRAP_WR32(PMIC_WRAP_WRAP_EN , 1);//enable wrap
  //similar to status updata test case
  pwrap_wacs0(1, DEW_WRITE_TEST, 0x55AA, &rdata);
  WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
  WRAP_WR32(PMIC_WRAP_SIG_VALUE,0xAA55);
  wait_for_completion(&pwrap_done);

  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  _pwrap_switch_mux(0);//recover
  WRAP_WR32(PMIC_WRAP_SIG_VALUE,0x55AA);//tha same as write test

  return 0;
}
//[21]: HARB_RRARB_DLE: HARB to RRARB DLE timeout monitor HARB to RRARB DLE
//:disable MUX,do keypad test
S32 _wdt_test_bit21( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  U32 reg_backup;
  PWRAPFUC();
  wdt_test_bit=21;
  wait_for_wdt_flag=0;
#ifdef ENABLE_KEYPAD_ON_LDVT
  //kepad command
  *((volatile kal_uint16 *)(KP_BASE + 0x1c)) = 0x1;
  kpd_init();
  initKpdTest();
#endif
  //WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,0 );
  //WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,1<<wdt_test_bit);
  reg_backup=WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x80);//only enable keypad
  _pwrap_switch_mux(1);//manual mode
  WRAP_WR32(PMIC_WRAP_WRAP_EN , 1);//enable wrap
  WRAP_WR32(0x10016020 , 0);//write keypad register,to send a keypad read request
  wait_for_completion(&pwrap_done);

  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , reg_backup);

  _pwrap_switch_mux(0);//recover
  return 0;
}


//[22]: MUX_WRAP_DLE: MUX to WRAP DLE timeout monitor
//MUX to WRAP DLE:disable MUX,then send a read commnad ,and do WACS0
S32 _wdt_test_bit22( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=22;
  wait_for_wdt_flag=0;
  //WRAP_WR32(PMIC_WRAP_WRAP_EN , 1);//enable wrap
  pwrap_wacs1(0, watch_dog_test_reg, 0, &rdata);
  _pwrap_switch_mux(1);//manual mode
  WRAP_WR32(PMIC_WRAP_WRAP_EN , 1);//enable wrap
  wait_for_completion(&pwrap_done);

  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  _pwrap_switch_mux(0);//recover
  return 0;
}
#endif
//[23]: MUX_MAN_DLE: MUX to MAN DLE timeout monitor
//Disable MUX,then send a read command in manual mode
S32 _wdt_test_bit23(void)
{
	U32 rdata = 0;
	U32 reg_rdata = 0;
	U32 res = 0;
	U32 return_value = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 23;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

// TODO: figure out...

	//disable other wdt bit
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 1 << wdt_test_bit);
	reg_rdata = WRAP_RD32(PMIC_WRAP_WDT_SRC_EN);
	PWRAPLOG("PMIC_WRAP_WDT_SRC_EN=%x.\n", reg_rdata);

	return_value = _pwrap_switch_mux(1);//manual mode
	PWRAPLOG("_pwrap_switch_mux return_value=%x.\n", return_value);

	WRAP_WR32(PMIC_WRAP_SI_CK_CON, 0x6);
	reg_rdata = WRAP_RD32(PMIC_WRAP_SI_CK_CON);
	PWRAPLOG("PMIC_WRAP_SI_CK_CON=%x.\n", reg_rdata);

	return_value = _pwrap_manual_mode(0,  OP_IND, 0, &rdata);
	PWRAPLOG("_pwrap_manual_mode return_value=%x.\n", return_value);

	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	_pwrap_switch_mux(0);//recover
	return res;
}
#if 0
//[24]: MSTCTL_SYNC_DLE: MSTCTL to SYNC DLE timeout monitor
//MSTCTL to SYNC  DLE:disable sync,then send a read commnad with wacs0
S32 _wdt_test_bit24( )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 res=0;
  PWRAPFUC();
  wdt_test_bit=24;
  wait_for_wdt_flag=0;
  _pwrap_switch_mux(1);//manual mode
  wait_for_completion(&pwrap_done);
  while(wait_for_wdt_flag!=1);
  PWRAPLOG("%s pass\n", __FUNCTION__);
  _pwrap_switch_mux(0);//recover
  return 0;
}
#endif
//[25]: STAUPD_TRIG: STAUPD trigger signal timeout monitor
//After init, set period = 0
S32 _wdt_test_bit25(void)
{
	U32 res = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 25;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

	//0x1:20us,for concurrence test,MP:0x5;  //100us
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x0);
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[26]: PREADY: APB PREADY timeout monitor
//disable WRAP_EN and write wacs0 6 times
S32 _wdt_test_bit26(void)
{
	//U32 rdata = 0;
	U32 wdata = 0;
	U32 res = 0;
	U32 i = 0;
	PWRAPFUC();
	wait_for_wdt_flag = 0;
	wdt_test_bit = 26;

	res = pwrap_init();
	if (res != 0) {
		wdt_test_fail_count++;
		return wdt_test_bit;
	}

	_wdt_test_disable_other_int();
	init_completion(&pwrap_done);

	//disable other wdt bit
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 1 << wdt_test_bit);

	WRAP_WR32(PMIC_WRAP_WRAP_EN, 0);//disable wrap
	for (i = 0; i < 10; i++) {
		wdata += 0x20;
		//pwrap_wacs0(1, watch_dog_test_reg, wdata, &rdata);
		WRAP_WR32(PMIC_WRAP_WACS0_CMD, 0x10000000);
		PWRAPLOG("send %d command .\n",i);
	}
	wait_for_completion(&pwrap_done);

	if (wait_for_wdt_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = wdt_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

S32 tc_wdt_test(void)
{
	U32 ret = 0;
	U32 return_value = 0;
	U32 reg_data = 0;

	struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
	pwrap_obj->complete = pwrap_complete;
	pwrap_obj->context = &pwrap_done;

	wdt_test_fail_count = 0;
	choose_lisr = WDT_TEST; // switch IRQ handler

	//enable watch dog
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0xffffff);

	ret = _wdt_test_bit1();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _wdt_test_bit2();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _wdt_test_bit3();
	if (ret != 0)
		return_value |= (1 << ret);
#if 0
	ret=_wdt_test_bit5();
#endif

#if 1 // use codevisor to toggle
	ret = _wdt_test_bit6();//fail
	if (ret != 0)
		return_value |= (1 << ret);
#endif
#if 0
	//ret=_wdt_test_bit7();

	ret=_wdt_test_bit8();

	ret=_wdt_test_bit9();//eint

	ret=_wdt_test_bit10();//eint

	//ret=_wdt_test_bit11();//eint

	reg_data=WRAP_RD32(PMIC_WRAP_WDT_SRC_EN);
	PWRAPLOG("PMIC_WRAP_WDT_SRC_EN=%x.\n",reg_data);
#endif

#if 1
	ret = _wdt_test_bit12(); //need to add timeout
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _wdt_test_bit13();
	if (ret != 0)
		return_value |= (1 << ret);

	reg_data = WRAP_RD32(PMIC_WRAP_INT_FLG);
	PWRAPLOG("wrap_int_flg=%x.\n", reg_data);
	reg_data = WRAP_RD32(PMIC_WRAP_WDT_FLG);
	PWRAPLOG("PMIC_WRAP_WDT_FLG=%x.\n", reg_data);

	ret = _wdt_test_bit14();
	if (ret != 0)
		return_value |= (1 << ret);

	//ret=_wdt_test_bit15();
#endif

	ret = _wdt_test_bit16();//pass
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _wdt_test_bit17();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _wdt_test_bit18();
	if (ret != 0)
		return_value |= (1 << ret);
#if 0
	ret=_wdt_test_bit19();

	ret=_wdt_test_bit20();

	ret=_wdt_test_bit21();
#endif
	ret = _wdt_test_bit23(); //pass
	if (ret != 0)
		return_value |= (1 << ret);

	//ret=_wdt_test_bit24();

	ret = _wdt_test_bit25();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _wdt_test_bit26();//cann't test
	if (ret != 0)
		return_value |= (1 << ret);

	choose_lisr = NORMAL_TEST; // switch back IRQ handler

//	PWRAPLOG("wdt_test_fail_count=%d.\n", wdt_test_fail_count);
	if (wdt_test_fail_count == 0) {
		PWRAPLOG("%s pass. ret=0x%x\n", __FUNCTION__, return_value);
		return 0;
	}
	else {
		PWRAPLOG("%s fail. fail_count=%d ret=0x%x\n",
			__FUNCTION__, wdt_test_fail_count, return_value);
		return return_value;
	}
}
//-------------------watch dog test end-------------------------------------

//start----------------interrupt test ------------------------------------
U32 interrupt_test_reg = DEW_WRITE_TEST;

S32 _int_test_disable_watch_dog(void)
{
	//disable watch dog
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);

	return 0;
}

//[1]:  SIG_ERR: Signature Checking failed.
S32 _int_test_bit1(void)
{
	//U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 1;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

#if 1 //CRC mode
	WRAP_WR32(PMIC_WRAP_ADC_RDY_ADDR, 0x1);
#endif

#if 0 //sig_value mode
	pwrap_write(DEW_WRITE_TEST, 0x55AA);
	WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
	WRAP_WR32(PMIC_WRAP_SIG_VALUE,0xAA55);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x1);

	pwrap_delay_us(5000);//delay 5 seconds
	rdata=WRAP_RD32(PMIC_WRAP_SIG_ERRVAL);
	if( rdata != 0x55AA )
	{
	  PWRAPERR("_pwrap_status_update_test error,error code=%x, rdata=%x", 1, rdata);
	  //return 1;
	}
	//WRAP_WR32(PMIC_WRAP_SIG_VALUE,0x55AA);//tha same as write test
	//clear sig_error interrupt flag bit
	//WRAP_WR32(PMIC_WRAP_INT_CLR,1<<1);
#endif
	wait_for_completion(&pwrap_done);
	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[5]:  MAN_CMD_MISS: A MAN CMD is written while MAN is disabled.
//Disable MAN, send a manual command
S32 _int_test_bit5(void)
{
	U32 rdata = 0;
	U32 res = 0;
	U32 return_value = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 5;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	WRAP_WR32(PMIC_WRAP_MAN_EN, 0);// disable MAN

	return_value = _pwrap_manual_mode(OP_WR, OP_CSH, 0, &rdata);
	PWRAPLOG("return_value of _pwrap_manual_mode=%x.\n", return_value);
	wait_for_completion(&pwrap_done);

	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[14]: WACS0_CMD_MISS: A WACS0 CMD is written while WACS0 is disabled.
//Disable WACS0_EN, send a wacs0 command
S32 _int_test_bit14(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 14;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	WRAP_WR32(PMIC_WRAP_WACS0_EN, 0);// disable MAN

	pwrap_wacs0(1, WRAP_ACCESS_TEST_REG, 0x55AA, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[17]: WACS1_CMD_MISS: A WACS1 CMD is written while WACS1 is disabled.
//Disable WACS1_EN, send a wacs1 command
S32 _int_test_bit17(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 17;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	WRAP_WR32(PMIC_WRAP_WACS1_EN, 0);// disable MAN

	pwrap_wacs1(1, WRAP_ACCESS_TEST_REG, 0x55AA, &rdata);
	wait_for_completion(&pwrap_done);

	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[20]: WACS2_CMD_MISS: A WACS2 CMD is written while WACS2 is disabled.
//Disable WACS2_EN, send a wacs2 command
S32 _int_test_bit20(void)
{
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 20;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	WRAP_WR32(PMIC_WRAP_WACS2_EN, 0);// disable MAN

	pwrap_write(WRAP_ACCESS_TEST_REG, 0x55AA);
	wait_for_completion(&pwrap_done);

	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[3]:  MAN_UNEXP_VLDCLR: MAN unexpected VLDCLR
//Send a manual write command,and clear valid big
S32 _int_test_bit3(void)
{
	U32 rdata = 0;
	U32 res = 0;
	U32 return_value = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 3;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	_pwrap_switch_mux(1);
	return_value = _pwrap_manual_mode(OP_WR, OP_CSH, 0, &rdata);
	PWRAPLOG("return_value of _pwrap_manual_mode=%x.\n", return_value);
	WRAP_WR32(PMIC_WRAP_MAN_VLDCLR, 1);
	wait_for_completion(&pwrap_done);

	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[12]: WACS0_UNEXP_VLDCLR: WACS0 unexpected VLDCLR
//Send a wacs0 write command,and clear valid big
S32 _int_test_bit12(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 12;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	pwrap_wacs0(1, WRAP_ACCESS_TEST_REG, 0x55AA, &rdata);
	WRAP_WR32(PMIC_WRAP_WACS0_VLDCLR, 1);
	wait_for_completion(&pwrap_done);

	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[15]: WACS1_UNEXP_VLDCLR: WACS1 unexpected VLDCLR
//Send a wacs1 write command,and clear valid big
S32 _int_test_bit15(void)
{
	U32 rdata = 0;
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 15;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	pwrap_wacs1(1, WRAP_ACCESS_TEST_REG, 0x55AA, &rdata);
	WRAP_WR32(PMIC_WRAP_WACS1_VLDCLR, 1);

	wait_for_completion(&pwrap_done);
	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}

//[18]: WACS2_UNEXP_VLDCLR: WACS2 unexpected VLDCLR
//Send a wacs2 write command,and clear valid big
S32 _int_test_bit18(void)
{
	U32 res = 0;
	PWRAPFUC();
	wait_int_flag = 0;
	int_test_bit = 18;

	res = pwrap_init();
	if (res != 0) {
		int_test_fail_count++;
		return int_test_bit;
	}

	_int_test_disable_watch_dog();
	init_completion(&pwrap_done);

	pwrap_write(WRAP_ACCESS_TEST_REG, 0x55AA);
	WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR, 1);

	wait_for_completion(&pwrap_done);
	if (wait_int_flag == 1)
		PWRAPLOG("%s pass\n", __FUNCTION__);
	else {
		res = int_test_bit;
		PWRAPLOG("%s fail\n", __FUNCTION__);
	}

	return res;
}
#if 0
//[21]: PERI_WRAP_INT: PERI_PWRAP_BRIDGE interrupt is asserted.
//    send a wacs3 write command,and clear valid big
S32 _int_test_bit21( )
{
  //U32 rdata=0;
  //U32 res=0;
  PWRAPFUC();
  int_test_bit=21;

  wait_int_flag=0;
  //pwrap_wacs3(1, WRAP_ACCESS_TEST_REG, 0x55AA, &rdata);
  //WRAP_WR32(PERI_PWRAP_BRIDGE_WACS3_VLDCLR , 1);

  wait_for_completion(&pwrap_done);
  if(wait_int_flag==1)
    PWRAPLOG("%s pass\n", __FUNCTION__);
  else
    PWRAPLOG("%s fail\n", __FUNCTION__);
  return 0;
}
#endif
S32 tc_int_test(void)
{
	U32 ret = 0;
	U32 return_value = 0;
	struct pmic_wrap_obj *pwrap_obj = g_pmic_wrap_obj;
	pwrap_obj->complete = pwrap_complete;
	pwrap_obj->context = &pwrap_done;

	int_test_fail_count = 0;
	choose_lisr = INT_TEST; // switch IRQ handler

	ret = _int_test_bit1();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit5();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit14();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit17();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit20();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit3();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit12();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit15();
	if (ret != 0)
		return_value |= (1 << ret);

	ret = _int_test_bit18();
	if (ret != 0)
		return_value |= (1 << ret);

	//ret = _int_test_bit21();
	//ret = pwrap_init();

	choose_lisr = NORMAL_TEST; // switch back IRQ handler

	if (int_test_fail_count == 0) {
		PWRAPLOG("%s pass. ret=0x%x\n", __FUNCTION__, return_value);
		return 0;
	}
	else {
		PWRAPLOG("%s fail. fail_count=%d ret=0x%x\n",
			__FUNCTION__, int_test_fail_count, return_value);
		return return_value;
	}
}

S32 tc_clock_gating_test(void)
{
	U32 return_value = 0;
	PWRAPFUC();

#if 0 // resolve linking error
	pwrap_power_off();
#endif
	return_value = _pwrap_wrap_access_test();
	if (return_value == 0)
		PWRAPLOG("tc_clock_gating_test pass.\n");
	else
		PWRAPLOG("tc_clock_gating_test fail.res=%d\n",return_value);

	return return_value;
}

volatile U32 index_wacs0 = 0;
volatile U32 index_wacs1 = 0;
volatile U32 index_wacs2 = 0;
U64 start_time_wacs0 = 0;
U64 start_time_wacs1 = 0;
U64 start_time_wacs2 = 0;
U64 end_time_wacs0 = 0;
U64 end_time_wacs1 = 0;
U64 end_time_wacs2 = 0;
void _throughput_wacs0_test(void)
{
	U32 rdata = 0;
	PWRAPFUC();

	start_time_wacs0 = sched_clock();
	for (index_wacs0 = 0; index_wacs0 < 10000; index_wacs0++)
		pwrap_wacs0(0, WACS0_TEST_REG, 0, &rdata);

	end_time_wacs0 = sched_clock();
	PWRAPLOG("_throughput_wacs0_test send 10000 read command:average time(ns)=%lld.\n",(end_time_wacs0-start_time_wacs0));
	PWRAPLOG("index_wacs0=%d index_wacs1=%d index_wacs2=%d\n",index_wacs0,index_wacs1,index_wacs2);
	PWRAPLOG("start_time_wacs0=%lld start_time_wacs1=%lld start_time_wacs2=%lld\n",start_time_wacs0,start_time_wacs1,start_time_wacs2);
	PWRAPLOG("end_time_wacs0=%lld end_time_wacs1=%lld end_time_wacs2=%lld\n",end_time_wacs0,end_time_wacs1,end_time_wacs2);
}

void _throughput_wacs1_test( void )
{
	U32 rdata = 0;
	PWRAPFUC();

	start_time_wacs1 = sched_clock();
	for (index_wacs1 = 0; index_wacs1 < 10000; index_wacs1++)
	    pwrap_wacs1(0, WACS1_TEST_REG, 0, &rdata);

	end_time_wacs1 = sched_clock();
	PWRAPLOG("_throughput_wacs1_test send 10000 read command:average time(ns)=%lld.\n",(end_time_wacs1-start_time_wacs1));
	PWRAPLOG("index_wacs0=%d index_wacs1=%d index_wacs2=%d\n",index_wacs0,index_wacs1,index_wacs2);
	PWRAPLOG("start_time_wacs0=%lld start_time_wacs1=%lld start_time_wacs2=%lld\n",start_time_wacs0,start_time_wacs1,start_time_wacs2);
	PWRAPLOG("end_time_wacs0=%lld end_time_wacs1=%lld end_time_wacs2=%lld\n",end_time_wacs0,end_time_wacs1,end_time_wacs2);
}

void _throughput_wacs2_test( void )
{
	//U32 i=0;
	U32 rdata=0;
	U32 return_value=0;
	PWRAPFUC();
	start_time_wacs2=sched_clock();
	for (index_wacs2 = 0; index_wacs2 < 10000; index_wacs2++) {
	    return_value = pwrap_wacs2(0, WACS2_TEST_REG, 0, &rdata);
//      if(return_value!=0)
//        PWRAPLOG("return_value=%d.index_wacs2=%d\n",return_value,index_wacs2);
	}
	end_time_wacs2 = sched_clock();
	PWRAPLOG("_throughput_wacs2_test send 10000 read command:average time(ns)=%lld.\n",(end_time_wacs2-start_time_wacs2));
	PWRAPLOG("index_wacs0=%d index_wacs1=%d index_wacs2=%d\n",index_wacs0,index_wacs1,index_wacs2);
	PWRAPLOG("start_time_wacs0=%lld start_time_wacs1=%lld start_time_wacs2=%lld\n",start_time_wacs0,start_time_wacs1,start_time_wacs2);
	PWRAPLOG("end_time_wacs0=%lld end_time_wacs1=%lld end_time_wacs2=%lld\n",end_time_wacs0,end_time_wacs1,end_time_wacs2);
}

S32 tc_throughput_test(void)
{
	U32 return_value = 0;
	//U32 i = 0;
	U64 start_time = 0;
	U64 end_time = 0;

	struct task_struct *wacs0_throughput_task = 0;
	struct task_struct *wacs1_throughput_task = 0;
	struct task_struct *wacs2_throughput_task = 0;

	U32 wacs0_throughput_cpu_id = 0;
	U32 wacs1_throughput_cpu_id = 1;
	U32 wacs2_throughput_cpu_id = 2;

	PWRAPFUC();

	//disable INT
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0);
	//except for [31] debug_int, [1]: SIGERR, [0]: WDT
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x7ffffffc);
#if 0
	//---------------------------------------------------------------------
	PWRAPLOG("write throughput,start.\n");
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,8); //Only WACS2
	start_time = sched_clock();
	for (i = 0; i < 10000; i++)
		pwrap_write(WACS2_TEST_REG, 0x30);

	end_time = sched_clock();
	PWRAPLOG("send 100 write command:average time(ns)=%llx.\n",(end_time-start_time));//100000=100*1000
	PWRAPLOG("write throughput,end.\n");
	//---------------------------------------------------------------------
#endif
#if 0
	dsb();
	PWRAPLOG("1-core read throughput,start.\n");
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 1 << 1); //Only WACS0
	wacs0_throughput_task = kthread_create(_throughput_wacs0_test,0,"wacs0_concurrence");
	kthread_bind(wacs0_throughput_task, wacs0_throughput_cpu_id);
	wake_up_process(wacs0_throughput_task);
	pwrap_delay_us(5000);
	//kthread_stop(wacs0_throughput_task);
	PWRAPLOG("stop wacs0_throughput_task.\n");
	PWRAPLOG("1-core read throughput,end.\n");
	//-----------------------------------------------------------------------------------
#endif
#if 1
	dsb();
	PWRAPLOG("2-core read throughput,start.\n");
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 6); //Only WACS0 and WACS1
	wacs0_throughput_task = kthread_create(_throughput_wacs0_test, 0, 
						"wacs0_concurrence");
	if (IS_ERR(wacs0_throughput_task))
    		PWRAPERR("Unable to start kernelthread \n");
	kthread_bind(wacs0_throughput_task, wacs0_throughput_cpu_id);
	wake_up_process(wacs0_throughput_task);

	wacs1_throughput_task = kthread_create(_throughput_wacs1_test, 0, 
						"wacs1_concurrence");
	if (IS_ERR(wacs1_throughput_task))
    		PWRAPERR("Unable to start kernelthread \n");
	kthread_bind(wacs1_throughput_task, wacs1_throughput_cpu_id);
	wake_up_process(wacs1_throughput_task);

	pwrap_delay_us(50000);
	//kthread_stop(wacs0_throughput_task);
	//kthread_stop(wacs1_throughput_task);
	PWRAPLOG("stop wacs0_throughput_task and wacs1_throughput_task.\n");
	PWRAPLOG("2-core read throughput,end.\n");
	//---------------------------------------------------------------------
#endif
#if 0
	dsb();
	PWRAPLOG("3-core read throughput,start.\n");
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0xE); //Only WACS0 and WACS1
	wacs0_throughput_task = kthread_create(_throughput_wacs0_test,0,"wacs0_concurrence");
	kthread_bind(wacs0_throughput_task, wacs0_throughput_cpu_id);
	wake_up_process(wacs0_throughput_task);

	wacs1_throughput_task = kthread_create(_throughput_wacs1_test,0,"wacs1_concurrence");
	kthread_bind(wacs1_throughput_task, wacs1_throughput_cpu_id);
	wake_up_process(wacs1_throughput_task);

	wacs2_throughput_task = kthread_create(_throughput_wacs2_test,0,"wacs2_concurrence");
	kthread_bind(wacs2_throughput_task, wacs2_throughput_cpu_id);
	wake_up_process(wacs2_throughput_task);
	pwrap_delay_us(50000);
	//kthread_stop(wacs0_throughput_task);
	//kthread_stop(wacs1_throughput_task);
	//kthread_stop(wacs2_throughput_task);
	//PWRAPLOG("stop wacs0_throughput_task /wacs1_throughput_task/wacs2_throughput_task.\n");
	PWRAPLOG("3-core read throughput,end.\n");
#endif
	if(return_value == 0)
		PWRAPLOG("tc_throughput_test pass.\n");
	else
		PWRAPLOG("tc_throughput_test fail.res=%d\n",return_value);

	return return_value;
}

//#ifdef PWRAP_CONCURRENCE_TEST

//###############################concurrence_test start#########################
//---define wacs direction flag:  read:WACS0_READ_WRITE_FLAG=0;write:WACS0_READ_WRITE_FLAG=0;
//#define RANDOM_TEST
//#define NORMAL_TEST
//#define stress_test_on_concurrence

static U8 wacs0_send_write_cmd_done=0;
static U8 wacs0_send_read_cmd_done=0;
static U8 wacs0_read_write_flag=0;


static U8 wacs1_send_write_cmd_done=0;
static U8 wacs1_send_read_cmd_done=0;
static U8 wacs1_read_write_flag=0;

static U8 wacs2_send_write_cmd_done=0;
static U8 wacs2_send_read_cmd_done=0;
static U8 wacs2_read_write_flag=0;


static U16 wacs0_test_value=0x10;
static U16 wacs1_test_value=0x20;
static U16 wacs2_test_value=0x30;


U32 wacs_read_cmd_done=0;
//U32 test_count0=100000000;
//U32 test_count1=100000000;
U32 test_count0=0;
U32 test_count1=0;




static U16 concurrence_fail_count_cpu0=0;
static U16 concurrence_fail_count_cpu1=0;
static U16 concurrence_pass_count_cpu0=0;
static U16 concurrence_pass_count_cpu1=0;


U32 g_spm_pass_count0=0;
U32 g_spm_fail_count0=0;
U32 g_spm_pass_count1=0;
U32 g_spm_fail_count1=0;

U32 g_pwm_pass_count0=0;
U32 g_pwm_fail_count0=0;
U32 g_pwm_pass_count1=0;
U32 g_pwm_fail_count1=0;

U32 g_wacs0_pass_count0=0;
U32 g_wacs0_fail_count0=0;
U32 g_wacs0_pass_count1=0;
U32 g_wacs0_fail_count1=0;

U32 g_wacs1_pass_count0=0;
U32 g_wacs1_fail_count0=0;
U32 g_wacs1_pass_count1=0;
U32 g_wacs1_fail_count1=0;

U32 g_wacs2_pass_count0=0;
U32 g_wacs2_fail_count0=0;
U32 g_wacs2_pass_count1=0;
U32 g_wacs2_fail_count1=0;


U32 g_stress0_cpu0_count=0;
U32 g_stress1_cpu0_count=0;
U32 g_stress2_cpu0_count=0;
U32 g_stress3_cpu0_count=0;
U32 g_stress4_cpu0_count=0;
//U32 g_stress5_cpu0_count=0;
U32 g_stress0_cpu1_count=0;
U32 g_stress1_cpu1_count=0;
U32 g_stress2_cpu1_count=0;
U32 g_stress3_cpu1_count=0;
U32 g_stress4_cpu1_count=0;
U32 g_stress5_cpu1_count=0;

U32 g_stress0_cpu0_count0=0;
U32 g_stress1_cpu0_count0=0;
U32 g_stress0_cpu1_count0=0;

U32 g_stress0_cpu0_count1=0;
U32 g_stress1_cpu0_count1=0;
U32 g_stress0_cpu1_count1=0;

U32 g_stress2_cpu0_count1=0;
U32 g_stress3_cpu0_count1=0;

U32 g_random_count0=0;
U32 g_random_count1=0;
U32 g_wacs0_pass_cpu0=0;
U32 g_wacs0_pass_cpu1=0;
U32 g_wacs0_pass_cpu2=0;
U32 g_wacs0_pass_cpu3=0;

U32 g_wacs0_fail_cpu0=0;
U32 g_wacs0_fail_cpu1=0;
U32 g_wacs0_fail_cpu2=0;
U32 g_wacs0_fail_cpu3=0;

U32 g_wacs1_pass_cpu0=0;
U32 g_wacs1_pass_cpu1=0;
U32 g_wacs1_pass_cpu2=0;
U32 g_wacs1_pass_cpu3=0;

U32 g_wacs1_fail_cpu0=0;
U32 g_wacs1_fail_cpu1=0;
U32 g_wacs1_fail_cpu2=0;
U32 g_wacs1_fail_cpu3=0;

U32 g_wacs2_pass_cpu0=0;
U32 g_wacs2_pass_cpu1=0;
U32 g_wacs2_pass_cpu2=0;
U32 g_wacs2_pass_cpu3=0;

U32 g_wacs2_fail_cpu0=0;
U32 g_wacs2_fail_cpu1=0;
U32 g_wacs2_fail_cpu2=0;
U32 g_wacs2_fail_cpu3=0;




//--------------------------------------------------------
//    Function : pwrap_wacs0()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 _concurrence_pwrap_wacs0( U32 write, U32 adr, U32 wdata, U32 *rdata,U32 read_cmd_done )
{
  U32 reg_rdata=0;
  U32 wacs_write=0;
  U32 wacs_adr=0;
  U32 wacs_cmd=0;
  //PWRAPFUC();
  if(read_cmd_done==0)
  {
    reg_rdata = WRAP_RD32(PMIC_WRAP_WACS0_RDATA);
    if( GET_INIT_DONE0( reg_rdata ) != 1)
    {
      PWRAPERR("initialization isn't finished when write data\n");
      return 1;
    }
    if( GET_WACS0_FSM( reg_rdata ) != WACS_FSM_IDLE) //IDLE State
    {
      PWRAPERR("WACS0 is not in IDLE state\n");
      return 2;
    }
    // check argument validation
    if( (write & ~(0x1))    != 0)  return 3;
    if( (adr   & ~(0xffff)) != 0)  return 4;
    if( (wdata & ~(0xffff)) != 0)  return 5;

    wacs_write  = write << 31;
    wacs_adr    = (adr >> 1) << 16;
    wacs_cmd= wacs_write | wacs_adr | wdata;
    WRAP_WR32(PMIC_WRAP_WACS0_CMD,wacs_cmd);
  }
  else
  {
    if( write == 0 )
    {
      do
      {
        reg_rdata = WRAP_RD32(PMIC_WRAP_WACS0_RDATA);
        if( GET_INIT_DONE0( reg_rdata ) != 1)
        {
          //wrapper may be reset when error happen,so need to check if init is done
          PWRAPERR("initialization isn't finished when read data\n");
          return 6;
        }
      } while( GET_WACS0_FSM( reg_rdata ) != WACS_FSM_WFVLDCLR ); //WFVLDCLR

      *rdata = GET_WACS0_RDATA( reg_rdata );
      WRAP_WR32(PMIC_WRAP_WACS0_VLDCLR , 1);
    }
  }
  return 0;
}
//--------------------------------------------------------
//    Function : pwrap_wacs1()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 _concurrence_pwrap_wacs1( U32  write, U32  adr, U32  wdata, U32 *rdata ,U32 read_cmd_done)
{
  U32 reg_rdata=0;
  U32 wacs_write=0;
  U32 wacs_adr=0;
  U32 wacs_cmd=0;
  if(read_cmd_done==0)
  {
    //PWRAPFUC();
    reg_rdata = WRAP_RD32(PMIC_WRAP_WACS1_RDATA);
    if( GET_INIT_DONE0( reg_rdata ) != 1)
    {
      PWRAPERR("initialization isn't finished when write data\n");
      return 1;
    }
    if( GET_WACS0_FSM( reg_rdata ) != WACS_FSM_IDLE) //IDLE State
    {
      PWRAPERR("WACS1 is not in IDLE state\n");
      return 2;
    }
    // check argument validation
    if( (write & ~(0x1))    != 0)  return 3;
    if( (adr   & ~(0xffff)) != 0)  return 4;
    if( (wdata & ~(0xffff)) != 0)  return 5;

    wacs_write  = write << 31;
    wacs_adr    = (adr >> 1) << 16;
    wacs_cmd= wacs_write | wacs_adr | wdata;

    WRAP_WR32(PMIC_WRAP_WACS1_CMD,wacs_cmd);
  }
  else
  {
    if( write == 0 )
    {
      do
      {
        reg_rdata = WRAP_RD32(PMIC_WRAP_WACS1_RDATA);
        if( GET_INIT_DONE0( reg_rdata ) != 1)
        {
          //wrapper may be reset when error happen,so need to check if init is done
          PWRAPERR("initialization isn't finished when read data\n");
          return 6;
        }
      } while( GET_WACS0_FSM( reg_rdata ) != WACS_FSM_WFVLDCLR ); //WFVLDCLR State

      *rdata = GET_WACS0_RDATA( reg_rdata );
      WRAP_WR32(PMIC_WRAP_WACS1_VLDCLR , 1);
    }
  }
  return 0;
}
//----wacs API implement for concurrence test-----------------------
S32 _concurrence_pwrap_wacs2( U32  write, U32  adr, U32  wdata, U32 *rdata, U32 read_cmd_done )
{
  U32 reg_rdata=0;
  U32 wacs_write=0;
  U32 wacs_adr=0;
  U32 wacs_cmd=0;
  if(read_cmd_done==0)
  {
    //PWRAPFUC();
    reg_rdata = WRAP_RD32(PMIC_WRAP_WACS2_RDATA);
    if( GET_INIT_DONE0( reg_rdata ) != 1)
      return 1;
    if( GET_WACS0_FSM( reg_rdata ) != WACS_FSM_IDLE) //IDLE State
    {
      PWRAPERR("WACS2 is not in IDLE state\n");
      return 2;
    }

    // check argument validation
    if( (write & ~(0x1))    != 0)  return 3;
    if( (adr   & ~(0xffff)) != 0)  return 4;
    if( (wdata & ~(0xffff)) != 0)  return 5;

    wacs_write  = write << 31;
    wacs_adr    = (adr >> 1) << 16;
    wacs_cmd= wacs_write | wacs_adr | wdata;

    WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);
  }
  else
  {
    if( write == 0 )
    {
      do
      {
        reg_rdata = WRAP_RD32(PMIC_WRAP_WACS2_RDATA);
        //if( GET_INIT_DONE0( reg_rdata ) != 1)
        //  return 3;
      } while( GET_WACS0_FSM( reg_rdata ) != WACS_FSM_WFVLDCLR ); //WFVLDCLR

      *rdata = GET_WACS0_RDATA( reg_rdata );
      WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
    }
  }
  return 0;
}

void _concurrence_wacs0_test( void )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 rand_number=0;
  PWRAPFUC();
  while(1)
  {
  #ifdef RANDOM_TEST
    rand_number=(U32)random32();
    if((rand_number%2)==1)
      msleep(10);
    else
  #endif
	{
      pwrap_wacs0(1, WACS0_TEST_REG, wacs0_test_value, &rdata);
      //printk("write (using WACS0),value=%x\n", wacs0_test_value);
		pwrap_wacs0(0, WACS0_TEST_REG, wacs0_test_value, &rdata);
      //printk("read (using WACS0),rdata=%x\n", rdata);

		if( rdata != wacs0_test_value )
		{
			g_wacs0_fail_count0++;
			PWRAPERR("read test error(using WACS0),wacs0_test_value=%x, rdata=%x\n", wacs0_test_value, rdata);
			switch ( raw_smp_processor_id())
			{
			  case 0:
				g_wacs0_fail_cpu0++;
				break;
			  case 1:
				g_wacs0_fail_cpu1++;
				break;
			  case 2:
				g_wacs0_fail_cpu2++;
				break;
			  case 3:
				g_wacs0_fail_cpu3++;
				break;
			  default:
				break;
			}
			  //PWRAPERR("concurrence_fail_count_cpu2=%d", ++concurrence_fail_count_cpu0);
		}
		else
		{
			g_wacs0_pass_count0++;
		  //PWRAPLOG("WACS0 concurrence_test pass,rdata=%x.\n",rdata);
		  //PWRAPLOG("WACS0 concurrence_test pass,concurrence_pass_count_cpu0=%d\n",++concurrence_pass_count_cpu0);

			switch ( raw_smp_processor_id())
			{
			  case 0:
				g_wacs0_pass_cpu0++;
				break;
			  case 1:
				g_wacs0_pass_cpu1++;
				break;
			  case 2:
				g_wacs0_pass_cpu2++;
				break;
			  case 3:
				g_wacs0_pass_cpu3++;
				break;
			  default:
				break;
			}
		}
		  wacs0_test_value+=0x1;

    }
  }//end of while(1)
}
void _concurrence_wacs1_test( void )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 rand_number=0;
  PWRAPFUC();
  while(1)
  {
  #ifdef RANDOM_TEST
    rand_number=(U32)random32();
    if((rand_number%2)==1)
      msleep(10);
    else
  #endif
	{
      pwrap_wacs1(1, WACS1_TEST_REG, wacs1_test_value, &rdata);
      //printk("write (using WACS1),value=%x\n", wacs1_test_value);
      pwrap_wacs1(0, WACS1_TEST_REG, wacs1_test_value, &rdata);
      //printk("read  (using WACS1),rdata=%x\n", rdata);
      if( rdata != wacs1_test_value )
      {
		g_wacs1_fail_count0++;
        PWRAPERR("read test error(using WACS1),wacs1_test_value=%x, rdata=%x\n", wacs1_test_value, rdata);
        switch ( raw_smp_processor_id())
        {
          case 0:
            g_wacs1_fail_cpu0++;
            break;
          case 1:
            g_wacs1_fail_cpu1++;
            break;
          case 2:
            g_wacs1_fail_cpu2++;
            break;
          case 3:
            g_wacs1_fail_cpu3++;
            break;
          default:
            break;
        }
            // PWRAPERR("concurrence_fail_count_cpu1=%d", ++concurrence_fail_count_cpu1);
	  }
	  else
	  {
		g_wacs1_pass_count0++;
        switch ( raw_smp_processor_id())
        {
          case 0:
            g_wacs1_pass_cpu0++;
            break;
          case 1:
            g_wacs1_pass_cpu1++;
            break;
          case 2:
            g_wacs1_pass_cpu2++;
            break;
          case 3:
            g_wacs1_pass_cpu3++;
            break;
          default:
            break;
        }
      }
    wacs1_test_value+=0x3;
  }
  }//end of while(1)
}
void _concurrence_wacs2_test( void )
{
  U32 rdata=0;
  U32 reg_rdata=0;
  U32 rand_number=0;
  PWRAPFUC();
  while(1)
  {
  #ifdef RANDOM_TEST
    rand_number=(U32)random32();
    if((rand_number%2)==1)
      msleep(10);
    else
  #endif
    {

		pwrap_write(WACS2_TEST_REG, wacs2_test_value);
      //printk("write (using WACS2),value=%x\n", wacs2_test_value);
      pwrap_read(WACS2_TEST_REG,  &rdata);
      if( rdata != wacs2_test_value )
      {
		g_wacs2_fail_count0++;
        switch ( raw_smp_processor_id())
        {
          case 0:
            g_wacs2_fail_cpu0++;
            break;
          case 1:
            g_wacs2_fail_cpu1++;
            break;
          case 2:
            g_wacs2_fail_cpu2++;
            break;
          case 3:
            g_wacs2_fail_cpu3++;
            break;
          default:
            break;
        }
        PWRAPERR("read test error(using WACS2),wacs2_test_value=%x, rdata=%x\n", wacs2_test_value, rdata);
      }
      else
      {
		g_wacs2_pass_count0++;
        switch ( raw_smp_processor_id())
        {
          case 0:
            g_wacs2_pass_cpu0++;
            break;
          case 1:
            g_wacs2_pass_cpu1++;
            break;
          case 2:
            g_wacs2_pass_cpu2++;
            break;
          case 3:
            g_wacs2_pass_cpu3++;
            break;
          default:
            break;
        }
      }////end of if( rdata != wacs2_test_value )
      wacs2_test_value+=0x2;
	}
  }//end of while(1)
}


struct task_struct *spm_task;
U32 spm_cpu_id=1;

struct task_struct *wacs0_task;
struct task_struct *wacs1_task;
struct task_struct *wacs2_task;
U32 wacs0_cpu_id=1;
U32 wacs1_cpu_id=1;
U32 wacs2_cpu_id=1;


struct task_struct *log0_task;
U32 log0_cpu_id=0;

U32 log1_task=0;
U32 log1_cpu_id=1;

struct task_struct *kthread_stress0_cpu0;
U32 stress0_cpu_id=0;

struct task_struct *kthread_stress1_cpu0;
U32 stress1_cpu_id=0;

struct task_struct *kthread_stress2_cpu0;
U32 stress2_cpu_id=0;

struct task_struct *kthread_stress3_cpu0;
U32 stress3_cpu_id=0;

struct task_struct *kthread_stress4_cpu0;
U32 stress4_cpu_id=0;

struct task_struct *kthread_stress0_cpu1;
U32 stress01_cpu_id=0;

struct task_struct *kthread_stress1_cpu1;
struct task_struct *kthread_stress2_cpu1;
struct task_struct *kthread_stress3_cpu1;
struct task_struct *kthread_stress4_cpu1;
struct task_struct *kthread_stress5_cpu1;

void _concurrence_spm_test_code(unsigned int spm)
{
  PWRAPFUC();
#ifdef ENABLE_SPM_ON_LDVT
  U32 i=0;
  //while(i<20)
  while(1)
  {
    //mtk_pmic_dvfs_wrapper_test(10);
    //i--;
  }
#endif
}

void _concurrence_log0(unsigned int spm)
{
  PWRAPFUC();
  U32 i=1;
  U32 index=0;
  U32 cpu_id=0;
  U32 rand_number=0;
  U32 reg_value=0;
  while(1)
  {
    if((i%1000000)==0)
    {
     // PWRAPLOG("spm,pass count=%d,fail count=%d\n",g_spm_pass_count0,g_spm_fail_count0);

      PWRAPLOG("wacs0,cup0,pass count=%d,fail count=%d\n",g_wacs0_pass_cpu0,g_wacs0_fail_cpu0);
      PWRAPLOG("wacs1,cup0,pass count=%d,fail count=%d\n",g_wacs1_pass_cpu1,g_wacs1_fail_cpu1);
      PWRAPLOG("wacs2,cup0,pass count=%d,fail count=%d\n",g_wacs2_pass_cpu2,g_wacs2_fail_cpu2);
      PWRAPLOG("\n");
      //PWRAPLOG("wacs4,pass count=%d,fail count=%d\n",g_wacs4_pass_count0,g_wacs4_fail_count0);
#if 0
      PWRAPLOG("g_stress0_cpu0_count0=%d\n",g_stress0_cpu0_count0);
      PWRAPLOG("g_stress1_cpu0_count0=%d\n",g_stress1_cpu0_count0);
      PWRAPLOG("g_stress0_cpu1_count0=%d\n",g_stress0_cpu1_count0);
      PWRAPLOG("g_random_count0=%d\n",g_random_count0);
      PWRAPLOG("g_random_count1=%d\n",g_random_count1);
#endif
	  reg_value = WRAP_RD32(PMIC_WRAP_HARB_STA1);
      PWRAPLOG("PMIC_WRAP_HARB_STA1=%d\n",reg_value);
	  //reg_value = WRAP_RD32(PMIC_WRAP_RRARB_STA1);
      //PWRAPLOG("PMIC_WRAP_RRARB_STA1=%d\n",reg_value);
	  //reg_value = WRAP_RD32(PERI_PWRAP_BRIDGE_IARB_STA1);
      //PWRAPLOG("PERI_PWRAP_BRIDGE_IARB_STA1=%d\n",reg_value);

    //}
    //if((i%1000000)==0)
    //if(0)
    //{
//      rand_number=(U32)random32();
//      if((rand_number%2)==1)
//      {
//        cpu_id=((spm_cpu_id++)%2);
//        if (wait_task_inactive(spm_task, TASK_UNINTERRUPTIBLE))
//        {
//          PWRAPLOG("spm_cpu_id=%d\n",cpu_id);
//          kthread_bind(spm_task, cpu_id);
//        }
//        else
//         spm_cpu_id--;
//      }


      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(wacs0_cpu_id++)%2;
        if (wait_task_inactive(wacs0_task, TASK_UNINTERRUPTIBLE))
        {
          PWRAPLOG("wacs0_cpu_id=%d\n",cpu_id);
          kthread_bind(wacs0_task, cpu_id);
        }
        else
         wacs0_cpu_id--;
      }

      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(wacs1_cpu_id++)%2;
        if (wait_task_inactive(wacs1_task, TASK_UNINTERRUPTIBLE))
        {
          PWRAPLOG("wacs1_cpu_id=%d\n",cpu_id);
          kthread_bind(wacs1_task, cpu_id);
        }
        else
         wacs1_cpu_id--;
      }

      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(wacs2_cpu_id++)%2;
        if (wait_task_inactive(wacs2_task, TASK_UNINTERRUPTIBLE))
        {
          PWRAPLOG("wacs2_cpu_id=%d\n",cpu_id);
          kthread_bind(wacs2_task, cpu_id);
        }
        else
         wacs2_cpu_id--;
      }

#if 0
      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(stress0_cpu_id++)%2;
        //kthread_bind(kthread_stress0_cpu0, cpu_id);
      }

      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(stress1_cpu_id++)%2;
        //kthread_bind(kthread_stress1_cpu0, cpu_id);
      }
      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(stress2_cpu_id++)%2;
        //kthread_bind(kthread_stress2_cpu0, cpu_id);
      }

      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(stress3_cpu_id++)%2;
        //kthread_bind(kthread_stress3_cpu0, cpu_id);
      }

      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(stress4_cpu_id++)%2;
        //kthread_bind(kthread_stress4_cpu0, cpu_id);
      }

      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        cpu_id=(stress01_cpu_id++)%2;
        //kthread_bind(kthread_stress0_cpu1, cpu_id);
      }
#endif
    }
    i++;
   }

}

void _concurrence_log1(unsigned int spm)
{
  PWRAPFUC();
  U32 i=0;
  //while(i<20)
  while(1)
  {
    //log---------------------------------------------------------------
    //if((test_count0%10000)==0)
    if((i%100)==0)
    {
      //PWRAPLOG("spm,pass count=%d,fail count=%d\n", g_spm_pass_count0, g_spm_fail_count0);
      PWRAPLOG("wacs0,pass count=%d,fail count=%d\n",g_wacs0_pass_count0,g_wacs0_fail_count0);
      PWRAPLOG("wacs1,pass count=%d,fail count=%d\n",g_wacs1_pass_count0,g_wacs1_fail_count0);
      PWRAPLOG("wacs2,pass count=%d,fail count=%d\n",g_wacs2_pass_count0,g_wacs2_fail_count0);
      PWRAPLOG("\n");
      //PWRAPLOG("g_stress0_cpu1_count=%d\n",g_stress0_cpu1_count);
#if 0
      PWRAPLOG("g_stress0_cpu0_count1=%d\n",g_stress0_cpu0_count1);
      PWRAPLOG("g_stress1_cpu0_count1=%d\n",g_stress1_cpu0_count1);
      PWRAPLOG("g_stress0_cpu1_count1=%d\n",g_stress0_cpu1_count1);
#endif

    }
    i++;
   }

}

void _concurrence_stress0_cpu0(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;

  while(1)
  {
      g_random_count0++;
      rand_number=(U32)random32();
      if((rand_number%2)==1)
      {
        g_random_count1++;
        for(i=0;i<100000;i++)
        {
          //g_stress0_cpu0_count++;
          if (raw_smp_processor_id() == 0)
          {
            g_stress0_cpu0_count0++;
          } else if (raw_smp_processor_id() == 1)
          {
            g_stress0_cpu0_count1++;
          }
        }
      }
  }
}
void _concurrence_stress1_cpu0(unsigned int stress)
{
  PWRAPFUC();
  U32 rand_number=0;
  U32 i=0;
  //while(i<20)
  for(;;)
  {
    for(i=0;i<100000;i++)
    {
      //g_stress1_cpu0_count++;
      if (raw_smp_processor_id() == 0)
      {
        g_stress1_cpu0_count0++;
      } else if (raw_smp_processor_id() == 1)
      {
        g_stress1_cpu0_count1++;
      }
    }
  }
}

void _concurrence_stress2_cpu0(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
        g_stress1_cpu0_count++;
    }
  }
}

void _concurrence_stress3_cpu0(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
        g_stress3_cpu0_count++;
    }
  }
}


void _concurrence_stress4_cpu0(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
        g_stress4_cpu0_count++;
    }
  }
}

void _concurrence_stress0_cpu1(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
      {
        if (raw_smp_processor_id() == 0)
        {
          g_stress0_cpu1_count0++;
        } else if (raw_smp_processor_id() == 1)
        {
          g_stress0_cpu1_count1++;
        }
      }
    }
  }
}

void _concurrence_stress1_cpu1(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
        g_stress1_cpu1_count++;
    }
  }
}

void _concurrence_stress2_cpu1(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
    //while(i<20)
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
     for(i=0;i<100000;i++)
        g_stress2_cpu0_count++;
    }
  }
}

void _concurrence_stress3_cpu1(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;
    //while(i<20)
  for(;;)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
        g_stress3_cpu0_count++;
    }
  }
}

void _concurrence_stress4_cpu1(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;

  while(1)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
      g_stress4_cpu1_count++;
    }
  }
}

void _concurrence_stress5_cpu1(unsigned int stress)
{
  PWRAPFUC();
  U32 i=0;
  U32 rand_number=0;

  while(1)
  {
    rand_number=(U32)random32();
    if((rand_number%2)==1)
    {
      for(i=0;i<100000;i++)
        g_stress5_cpu1_count++;
    }
  }
}

//----wacs concurrence test start ------------------------------------------

S32 tc_concurrence_test(void)
{
  UINT32 res=0;
  U32 rdata=0;
  U32 i=0;
  res=0;
  struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };

  PWRAPFUC();
//  spm_task = kthread_create(_concurrence_spm_test_code,0,"spm_concurrence");
//  if(IS_ERR(spm_task)){
//    PWRAPERR("Unable to start kernelthread \n");
//    res = -5;
//  }
//  //kthread_bind(spm_task, spm_cpu_id);
//  wake_up_process(spm_task);

  wacs0_task = kthread_create(_concurrence_wacs0_test,0,"wacs0_concurrence");
  if(IS_ERR(wacs0_task)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(wacs0_task, wacs0_cpu_id);
  wake_up_process(wacs0_task);

  wacs1_task = kthread_create(_concurrence_wacs1_test,0,"wacs1_concurrence");
  if(IS_ERR(wacs1_task)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(wacs1_task, wacs1_cpu_id);
  wake_up_process(wacs1_task);

  wacs2_task = kthread_create(_concurrence_wacs2_test,0,"wacs2_concurrence");
  if(IS_ERR(wacs2_task)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(wacs2_task, wacs2_cpu_id);
  wake_up_process(wacs2_task);


  log0_task = kthread_create(_concurrence_log0,0,"log0_concurrence");
  if(IS_ERR(log0_task)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //sched_setscheduler(log0_task, SCHED_FIFO, &param);
  kthread_bind(log0_task, log0_cpu_id);
  wake_up_process(log0_task);

  log1_task = kthread_create(_concurrence_log1,0,"log1_concurrence");
  if(IS_ERR(log1_task)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //sched_setscheduler(log1_task, SCHED_FIFO, &param);
  kthread_bind(log1_task, log1_cpu_id);
  wake_up_process(log1_task);
#ifdef stress_test_on_concurrence
  //increase cpu load
  kthread_stress0_cpu0 = kthread_create(_concurrence_stress0_cpu0,0,"stress0_cpu0_concurrence");
  if(IS_ERR(kthread_stress0_cpu0)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  kthread_bind(kthread_stress0_cpu0, 0);
  wake_up_process(kthread_stress0_cpu0);

  kthread_stress1_cpu0 = kthread_create(_concurrence_stress1_cpu0,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress1_cpu0)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  kthread_bind(kthread_stress1_cpu0, 0);
  wake_up_process(kthread_stress1_cpu0);

  kthread_stress2_cpu0 = kthread_create(_concurrence_stress2_cpu0,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress2_cpu0)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress2_cpu0, 0);
  wake_up_process(kthread_stress2_cpu0);

  kthread_stress3_cpu0 = kthread_create(_concurrence_stress3_cpu0,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress3_cpu0)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress3_cpu0, 0);
  wake_up_process(kthread_stress3_cpu0);

  //kthread_stress4_cpu0 = kthread_create(_concurrence_stress4_cpu0,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress4_cpu0)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress4_cpu0, 1);
  //wake_up_process(kthread_stress4_cpu0);

  kthread_stress0_cpu1 = kthread_create(_concurrence_stress0_cpu1,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress0_cpu1)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  kthread_bind(kthread_stress0_cpu1, 1);
  wake_up_process(kthread_stress0_cpu1);

  kthread_stress1_cpu1 = kthread_create(_concurrence_stress1_cpu1,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress1_cpu1)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress1_cpu1, 1);
  wake_up_process(kthread_stress1_cpu1);

  kthread_stress2_cpu1 = kthread_create(_concurrence_stress2_cpu1,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress2_cpu1)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress2_cpu1, 0);
  wake_up_process(kthread_stress2_cpu1);

  kthread_stress3_cpu1 = kthread_create(_concurrence_stress3_cpu1,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress3_cpu1)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress3_cpu1, 1);
  wake_up_process(kthread_stress3_cpu1);

  kthread_stress4_cpu1 = kthread_create(_concurrence_stress4_cpu1,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress3_cpu1)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress4_cpu1, 1);
  wake_up_process(kthread_stress4_cpu1);

  kthread_stress5_cpu1 = kthread_create(_concurrence_stress5_cpu1,0,"stress0_cpu1_concurrence");
  if(IS_ERR(kthread_stress3_cpu1)){
    PWRAPERR("Unable to start kernelthread \n");
    res = -5;
  }
  //kthread_bind(kthread_stress5_cpu1, 1);
  wake_up_process(kthread_stress5_cpu1);

#endif //stress test
	if(res == 0)
	{
		//delay 1 hour
		U32 i,j;
		for(i=0;i<1;i++)
			for(j=0;j<60;j++)
				msleep(60000);
		PWRAPLOG("stop concurrent thread \n");

		//kthread_stop(spm_task);
		kthread_stop(wacs0_task);
		kthread_stop(wacs1_task);
		kthread_stop(wacs2_task);
		kthread_stop(log0_task);
		kthread_stop(log1_task);
	}

	if(res==0)
	{
		U32 count = g_wacs0_fail_count0 + g_wacs0_fail_count0 + g_wacs0_fail_count0;
		if(count = 0){
			PWRAPLOG("tc_concurrence_test pass.\n");
		}else{
			PWRAPLOG("tc_concurrence_test failed %d.\n",count);
		}
	} 
	else
	{
		PWRAPLOG("tc_concurrence_test build environment fail.res=%d\n",res);
	}
	return res;
}
//-------------------concurrence_test end-------------------------------------
