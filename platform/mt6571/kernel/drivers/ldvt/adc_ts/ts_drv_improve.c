/*****************************************************************************
 *
 * Filename:
 * ---------
 *    ts_drv_improve.c.c
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This file is defined for touch screen driver -- dual touch
 *
 * Author:
 * -------
 *  Toy Chen
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:   1.0  $
 * $Modtime:   14 May 2012 13:00:40  $
 * $Log:   //mtkvs01/vmdata/Maui_sw/archives/mcu/drv/drv_tp/ts_drv_improve.c-arc  $
 *
 * 06 25 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 06 18 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 06 18 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 06 18 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 06 18 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 06 12 2012 toy.chen
 * [MAUI_03197117] [RTP_Dual_Touch] Put fingers on the boundary of screen, id shakes
 * <saved by Perforce>
 *
 * 06 01 2012 toy.chen
 * [MAUI_03193655] [Feature Switch Verification Fail] PROXIMITY_SENSOR from NONE to CM3623
 * <saved by Perforce>
 *
 * 06 01 2012 toy.chen
 * [MAUI_03193655] [Feature Switch Verification Fail] PROXIMITY_SENSOR from NONE to CM3623
 * <saved by Perforce>
 *
 * 06 01 2012 toy.chen
 * [MAUI_03192877] [RTP_Dual Touch_General_006]just put fingers on the screen and do not move, but the image will shake
 * <saved by Perforce>
 *
 * 05 25 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 05 22 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 05 21 2012 toy.chen
 * [MAUI_03184904] [TouchPanel] Touch panel multiple touch feature.
 * <saved by Perforce>
 *
 * 05 18 2012 toy.chen
 * NULL
 * <saved by Perforce>
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/kernel.h>
#include <mach/mt_typedefs.h>
#include "ts_hw.h"
#include "touch_panel.h"
#include <asm/io.h>

#ifdef CONFIG_MTK_LDVT_ADC_TS


//extern kal_uint16  ts_read_adc(kal_uint16 pos, kal_bool IsPressure);

extern unsigned int ts_udvt_read(int);


#if 0
#if defined(DRV_MISC_CACHE_REGION_SUPPORT)
  // If cache is supported, put RW, ZI into cahched region
  #pragma arm section rwdata = "CACHEDRW" , zidata = "CACHEDZI"
#endif  // #if defined(DRV_MISC_CACHE_REGION_SUPPORT)
#endif

kal_int32 TP_SCREEN_X_END = 480;
kal_int32 TP_SCREEN_Y_END = 800;
kal_int32 TP_X_PLATE = 267;
kal_int32 TP_Y_PLATE = 766;
kal_int32 TP_AVERAGE_NUMBER;
kal_int32 TP_RANGE;
kal_int32 TP_STEP;
kal_int32 TP_MIN_DELTA;
kal_int16 tp_multiple_touch_calibration_value[TP_CALI_FINISH] = {100, -1481, -92, 1545, 104 , 0, 0, 0, 0, 0, 0, 0, 0};
//extern Touch_Panel_Calibration_Mode_enum tp_cali_mode;
static kal_int16  gXp, gXn, gYp, gYn, gZ1, gZ2;
static kal_int16  gDualTouchA, gDualTouchB, gDualTouchC, gDualTouchD, gDualTouchE, gDualTouchF;
//extern Touch_Panel_Dual_Calibration_Mode_enum tp_dual_cali_mode;
kal_bool tp_dual_init_cali[TP_CALI_FINISH];
static kal_int16 tp_multiple_calibration_step_max[TP_CALI_FINISH]={0xFFFF};
static kal_int16 tp_multiple_calibration_step_min[TP_CALI_FINISH]={0xFFFF};



#define TS_CMD_ADDR_DUAL      0x0080 //hidden register bit.

#define TP_SMOOTH_ARRAY_SIZE 20
kal_uint8 tp_log_count_base = 30;
kal_uint8 tp_log_count=0;
kal_int32 tp_yp_yn_previous[TP_SMOOTH_ARRAY_SIZE], tp_rTouch_previous[TP_SMOOTH_ARRAY_SIZE];
kal_int32 tp_smooth_w, tp_smooth_r; //read write pointer of the array.
kal_uint16 tp_previous_x0, tp_previous_y0; //read write pointer of the array.
kal_uint16 tp_previous_x1, tp_previous_y1; //read write pointer of the array.

kal_uint16 tp_Xser_noTouch = 0; //Mode F
kal_uint16 tp_Yser_noTouch = 0; //Mode B
kal_uint16 tp_Xser_full_apart = 0; //Mode F
kal_uint16 tp_Yser_full_apart = 0; //Mode B
kal_uint16 tp_Yser = 0; //Mode B //gDualTouchB
kal_uint16 tp_Xser = 0; //Mode F //gDualTouchF
kal_uint16 tp_delta_Xer;// = tp_Xser_noTouch - tp_Xser;
kal_uint16 tp_delta_Yer;// = tp_Yser_noTouch - tp_Yser;
kal_uint16 tp_delta_full_apart_Xer;// = tp_Xser_noTouch - tp_Xser_full_apart;
kal_uint16 tp_delta_full_apart_Yer;// = tp_Yser_noTouch - tp_Yser_full_apart;

#if defined(DRV_MISC_CACHE_REGION_SUPPORT)
  // If cache is supported, put RW, ZI into cahched region
  #pragma arm section rwdata, zidata
#endif  // #if defined(DRV_MISC_CACHE_REGION_SUPPORT)

#if 0
//tp_multiple_get_NVRAM: store calibration value to nvram
void tp_multiple_set_NVRAM(kal_int16 *tp_multiple_touch_calibration_value)
{
    nvram_external_write_data(NVRAM_EF_TOUCHPANEL_PARA_LID,    /* LID */
                              1,    /* Record ID */
                              (kal_uint8*)tp_multiple_touch_calibration_value,  /* Source buffer */
                              NVRAM_EF_TOUCHPANEL_PARA_SIZE);  /* Buffer size of source */
}

//tp_multiple_get_NVRAM: get calibration value from nvram
void tp_multiple_get_NVRAM(kal_int16 *tp_multiple_touch_calibration_value)
{
    nvram_external_read_data(NVRAM_EF_TOUCHPANEL_PARA_LID,    /* LID */
                             1,    /* Record ID */
                             (kal_uint8*)tp_multiple_touch_calibration_value,  /* Dest buffer */
                             NVRAM_EF_TOUCHPANEL_PARA_SIZE);  /* Buffer size of dest */
}
#endif

//tp_multiple_init: initial the calibration value and customize value
void tp_multiple_init(TouchPanel_custom_data_struct *tp_data)
{
   TP_X_PLATE = tp_data->x_plate; // x axis resistance
   TP_Y_PLATE = tp_data->y_plate; // y axis resistance

   tp_multiple_touch_calibration_value[TP_CALI_POINT_LOW_BOUND] = tp_data->tp_dual_cali.point_low_bound;

   tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND] = tp_data->tp_dual_cali.negative_low_bound;
   tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND]  = tp_data->tp_dual_cali.negative_up_bound;
   tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND]  = tp_data->tp_dual_cali.positive_up_bound;
   tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND] = tp_data->tp_dual_cali.positive_low_bound;

   tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_LOW_BOUND] = tp_data->tp_dual_cali.min_y_low_bound;
   tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_UP_BOUND]  = tp_data->tp_dual_cali.min_y_up_bound; 
   tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_LOW_BOUND] = tp_data->tp_dual_cali.max_y_low_bound;
   tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_UP_BOUND]  = tp_data->tp_dual_cali.max_y_up_bound; 
   tp_multiple_touch_calibration_value[TP_CALI_MIN_X_LOW_BOUND] = tp_data->tp_dual_cali.min_x_low_bound;  
   tp_multiple_touch_calibration_value[TP_CALI_MIN_X_UP_BOUND]  = tp_data->tp_dual_cali.min_x_up_bound;   
   tp_multiple_touch_calibration_value[TP_CALI_MAX_X_LOW_BOUND] = tp_data->tp_dual_cali.max_x_low_bound;  
   tp_multiple_touch_calibration_value[TP_CALI_MAX_X_UP_BOUND]  = tp_data->tp_dual_cali.max_x_up_bound;   

   TP_AVERAGE_NUMBER = tp_data->average_number;
   TP_RANGE = tp_data->range;
   TP_STEP = tp_data->step;
   TP_MIN_DELTA = tp_data->min_delta;
}

/*
Calculate the distance ratio from center to the dual point location
return the interpolate offset
*/
double tp_multiple_touch_offset(kal_int32 p_n_diff)
{
	if(p_n_diff > tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND])
	{
		#if 0
		return (double)1000*(p_n_diff - tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND])/(double)(tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND] - tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND]);
		#else
		double tmp;		
		tmp = (double)1000*(p_n_diff - tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND]);
		do_div(tmp, (double)(tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND] - tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND]));
		return tmp;
		#endif
	}
	else if(p_n_diff < tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND])
	{
		#if 0
		return (double)1000*(tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND] - p_n_diff)/(double)(tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND] - tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND]);
		#else
		double tmp;		
		tmp = (double)1000*(tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND] - p_n_diff);

		do_div(tmp, (double)(tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND] - tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND]));
		return tmp;
		#endif
		
	}
	else
	{
		return 0;
	}
}

//Calculate the interpolate offset with X resolution
kal_int32 tp_multiple_touch_offset_X(kal_int32 p_n_diff)
{
#if 0
	return (kal_int32)((double)tp_multiple_touch_offset(p_n_diff)*(TP_SCREEN_X_END/2)/1000);
#else
	double tmp;
	tmp = (double)tp_multiple_touch_offset(p_n_diff)*(TP_SCREEN_X_END);
	do_div(tmp, 2*2000);
	return tmp;
#endif
}

//Calculate the interpolate offset with Y resolution
kal_int32 tp_multiple_touch_offset_Y(kal_int32 p_n_diff)
{
#if 0
	return (kal_int32)((double)tp_multiple_touch_offset(p_n_diff)*(TP_SCREEN_Y_END/2)/1000);
#else
	double tmp;
	tmp = (double)tp_multiple_touch_offset(p_n_diff)*(TP_SCREEN_Y_END);
	do_div(tmp, 2*2000);
	return tmp;	
#endif
}

kal_int32 tp_multiple_touch_horizontal_offset(kal_int32 rTouch, TouchPanelMultipleEventStruct *event)
{
	kal_int32 delta=0;

	if(tp_Xser_noTouch - tp_Xser) //MT6260
	{
		delta = (((tp_Xser_noTouch-tp_Xser) * TP_SCREEN_X_END / (tp_Xser_noTouch - tp_Xser_full_apart) )>>1);
		event->points[0].x = event->points[0].x + delta;
		event->points[1].x = event->points[1].x - delta;
		//drv_trace4(TRACE_GROUP_2, DUAL_TP_HORIZONTAL_DELTA, tp_Xser, TP_SCREEN_X_END, tp_Xser_full_apart, delta);
		return delta;
	}
	
	//if(event->points[0].y < distance) //calibration use the location 5% distance away from boundary
	//{
	//	r_up = tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_UP_BOUND];
	//	r_low= tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_LOW_BOUND];
	//}
	//else if(event->points[0].y > TP_SCREEN_Y_END-distance)
	//{
	//	r_up = tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_UP_BOUND];
	//	r_low= tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_LOW_BOUND];
	//}
	//else
	//{
	//	r_up = tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_UP_BOUND]+(tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_UP_BOUND]-tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_UP_BOUND])*(event->points[0].y-distance)/(TP_SCREEN_Y_END-2*distance);
	//	r_low= tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_LOW_BOUND]+(tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_LOW_BOUND]-tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_LOW_BOUND])*(event->points[0].y-distance)/(TP_SCREEN_Y_END-2*distance);
	//}
	//
	//if(r_up == r_low)
	//	return 0;
	//if(rTouch > r_up)
	//	rTouch = r_up;
	//else if(rTouch < r_low)
	//	rTouch = r_low;
	////rTouch decrease when X dispart
  //
	//delta = (TP_SCREEN_X_END/2)*(r_up-rTouch)/(r_up - r_low); 
  //
	//if( (event->points[0].x<=(TP_SCREEN_X_END>>2)) || (event->points[0].x>=((TP_SCREEN_X_END*3)>>2))) // 0~X/4 or X*3/4 ~ X
	//	delta = 0;
	//else if(event->points[0].x<=(TP_SCREEN_X_END>>1)) //X/4 ~ X/2, delta from 0%~100%
	//{
	//	delta = delta*(event->points[0].x-(TP_SCREEN_X_END>>2))/(TP_SCREEN_X_END>>2);
	//}
	//else // X/2 ~ X*3/4, delta from 100% ~ 0%
	//{
	//	delta = delta*(((TP_SCREEN_X_END*3)>>2)-event->points[0].x)/(TP_SCREEN_X_END>>2);
	//}
	//
	//if((tp_log_count)%tp_log_count_base == 0) //print per second
	//	//drv_trace1(TRACE_GROUP_2, DUAL_TP_HORIZONTAL_DELTA, delta);
	//if( ((delta != 0)&&(event->model == 2)) || (delta>TP_MIN_DELTA) )
	//{
	//	event->model = 2;
	//	if(event->points[0].x > delta)
	//		event->points[0].x -= delta;
	//	else
	//		event->points[0].x = 0;
	//	event->points[1].x += delta;
	//}
	return delta;
}

kal_int32 tp_multiple_touch_vertical_offset(kal_int32 rTouch, TouchPanelMultipleEventStruct *event)
{
	kal_int32 r_up, r_low, delta=0;
	kal_int32 distance = TP_SCREEN_X_END/20;
	

	if(tp_Yser_noTouch - tp_Yser)
	{
		delta = (((tp_Yser_noTouch-tp_Yser) * TP_SCREEN_Y_END / (tp_Yser_noTouch - tp_Yser_full_apart) )>>1);
		event->points[0].y = event->points[0].y + delta;
		event->points[1].y = event->points[1].y - delta;
		//drv_trace4(TRACE_GROUP_2, DUAL_TP_VERTICAL_DELTA, tp_Yser, TP_SCREEN_Y_END, tp_Yser_full_apart, delta);
		return delta;
	}

	//if(event->points[0].x < distance) //calibration use the location 5% distance away from boundary
	//{
	//	r_up = tp_multiple_touch_calibration_value[TP_CALI_MIN_X_UP_BOUND];
	//	r_low= tp_multiple_touch_calibration_value[TP_CALI_MIN_X_LOW_BOUND];
	//}
	//else if(event->points[0].x > TP_SCREEN_X_END-distance)
	//{
	//	r_up = tp_multiple_touch_calibration_value[TP_CALI_MAX_X_UP_BOUND];
	//	r_low= tp_multiple_touch_calibration_value[TP_CALI_MAX_X_LOW_BOUND];
	//}
	//else
	//{
	//	r_up = tp_multiple_touch_calibration_value[TP_CALI_MIN_X_UP_BOUND]+(tp_multiple_touch_calibration_value[TP_CALI_MAX_X_UP_BOUND]-tp_multiple_touch_calibration_value[TP_CALI_MIN_X_UP_BOUND])*(event->points[0].x-distance)/(TP_SCREEN_X_END-2*distance);
	//	r_low= tp_multiple_touch_calibration_value[TP_CALI_MIN_X_LOW_BOUND]+(tp_multiple_touch_calibration_value[TP_CALI_MAX_X_LOW_BOUND]-tp_multiple_touch_calibration_value[TP_CALI_MIN_X_LOW_BOUND])*(event->points[0].x-distance)/(TP_SCREEN_X_END-2*distance);
	//}	
	//
	//if(r_up <= r_low)
	//	return 0;
	//if(rTouch > r_up)
	//	rTouch = r_up;
	//else if(rTouch < r_low)
	//	rTouch = r_low;
	////rTouch increase when Y dispart
	//
	//delta = (TP_SCREEN_Y_END/2)*(rTouch-r_low)/(r_up - r_low); 
  //
	//if( (event->points[0].y<=(TP_SCREEN_Y_END>>2)) || (event->points[0].y>=((TP_SCREEN_Y_END*3)>>2))) // 0~Y/4 or Y*3/4 ~ Y
	//	delta = 0;
	//else if(event->points[0].y<=(TP_SCREEN_Y_END>>1)) //Y/4 ~ Y/2, delta from 0%~100%
	//{
	//	delta = delta*(event->points[0].y-(TP_SCREEN_Y_END>>2))/(TP_SCREEN_Y_END>>2);
	//}
	//else // Y/2 ~ Y*3/4, delta from 100% ~ 0%
	//{
	//	delta = delta*(((TP_SCREEN_Y_END*3)>>2)-event->points[0].y)/(TP_SCREEN_Y_END>>2);
	//}
  //
	//if((tp_log_count)%tp_log_count_base == 0) //print per second
	//	//drv_trace4(TRACE_GROUP_2, DUAL_TP_VERTICAL_DELTA, r_up, r_low, rTouch, delta);
	//if( ((delta != 0)&&(event->model == 2)) || (delta>TP_MIN_DELTA) )
	//{
	//	event->model = 2;
	//	if(event->points[0].y > delta)
	//		event->points[0].y -= delta;
	//	else
	//		event->points[0].y = 0;
	//	event->points[1].y += delta;
	//}
	return delta;
}

#if 0
/* 
From different panel rotation, we can not know each calibration step we should get max or min value.
First we got a range of max and min value in each step.
In final step compare the values with the pair step(XXX_MAX with XXX_MIN).
*/
kal_int16 tp_multiple_min(DCL_TP_CALIBRATION_MODE_Enum index1, DCL_TP_CALIBRATION_MODE_Enum index2)
{
	if(tp_multiple_calibration_step_min[index1] < tp_multiple_calibration_step_min[index2])
	{
		printk("dual step %d&%d min is %d value:%d", index1, index2, index1, tp_multiple_calibration_step_min[index1]);
		return tp_multiple_calibration_step_min[index1];
	}
	else
	{
		printk("dual step %d&%d min is %d: value:%d", index1, index2, index2, tp_multiple_calibration_step_min[index2]);
		return tp_multiple_calibration_step_min[index2];
	}
}

//tp_multiple_max: compare the value in step_max array.
kal_int16 tp_multiple_max(DCL_TP_CALIBRATION_MODE_Enum index1, DCL_TP_CALIBRATION_MODE_Enum index2)
{
	if(tp_multiple_calibration_step_max[index1] > tp_multiple_calibration_step_max[index2])
	{
		printk("dual step %d&%d max is %d value:%d", index1, index2, index1, tp_multiple_calibration_step_max[index1]);
		return tp_multiple_calibration_step_max[index1];
	}
	else
	{
		printk("dual step %d&%d max is %d value:%d", index1, index2, index2, tp_multiple_calibration_step_max[index2]);
		return tp_multiple_calibration_step_max[index2];
	}
}

void tp_multiple_touch_save_calibration()
{
	tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND] = tp_multiple_calibration_step_min[DCL_TP_CALI_LEFT_TOP_RIGHT_DOWN_MAX];//the MAX separate must be the min value
	tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND]  = tp_multiple_calibration_step_max[DCL_TP_CALI_LEFT_TOP_RIGHT_DOWN_MIN];//the MIN separate must be the max value
	tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND]  = tp_multiple_calibration_step_max[DCL_TP_CALI_RIGHT_TOP_LEFT_DOWN_MAX];//the MAX separate must be the max value
	tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND] = tp_multiple_calibration_step_min[DCL_TP_CALI_RIGHT_TOP_LEFT_DOWN_MIN];//the MIN separate must be the min value

	tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_LOW_BOUND] = tp_multiple_min(DCL_TP_CALI_TOP_RIGHT_LEFT_MAX, DCL_TP_CALI_TOP_RIGHT_LEFT_MIN);  
	tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_UP_BOUND]  = tp_multiple_max(DCL_TP_CALI_TOP_RIGHT_LEFT_MAX, DCL_TP_CALI_TOP_RIGHT_LEFT_MIN);  
	tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_LOW_BOUND] = tp_multiple_min(DCL_TP_CALI_DOWN_RIGHT_LEFT_MAX, DCL_TP_CALI_DOWN_RIGHT_LEFT_MIN);
	tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_UP_BOUND]  = tp_multiple_max(DCL_TP_CALI_DOWN_RIGHT_LEFT_MAX, DCL_TP_CALI_DOWN_RIGHT_LEFT_MIN);

	tp_multiple_touch_calibration_value[TP_CALI_MIN_X_LOW_BOUND] = tp_multiple_min(DCL_TP_CALI_LEFT_TOP_DOWN_MAX, DCL_TP_CALI_LEFT_TOP_DOWN_MIN);  
	tp_multiple_touch_calibration_value[TP_CALI_MIN_X_UP_BOUND]  = tp_multiple_max(DCL_TP_CALI_LEFT_TOP_DOWN_MAX, DCL_TP_CALI_LEFT_TOP_DOWN_MIN);  
	tp_multiple_touch_calibration_value[TP_CALI_MAX_X_LOW_BOUND] = tp_multiple_min(DCL_TP_CALI_RIGHT_TOP_DOWN_MAX, DCL_TP_CALI_RIGHT_TOP_DOWN_MIN);
	tp_multiple_touch_calibration_value[TP_CALI_MAX_X_UP_BOUND]  = tp_multiple_max(DCL_TP_CALI_RIGHT_TOP_DOWN_MAX, DCL_TP_CALI_RIGHT_TOP_DOWN_MIN);

	tp_multiple_set_NVRAM(tp_multiple_touch_calibration_value);

	printk("NEGATIVE_LOW_BOUND:%d",     tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND]);
	printk("NEGATIVE_UP_BOUND:%d",      tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND]);
	printk("POSITIVE_UP_BOUND:%d",      tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND]);
	printk("POSITIVE_LOW_BOUND:%d",     tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND]);
	printk("MIN_Y_LOW_BOUND:%d",        tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_LOW_BOUND]);
	printk("MIN_Y_UP_BOUND:%d",         tp_multiple_touch_calibration_value[TP_CALI_MIN_Y_UP_BOUND]);
	printk("MAX_Y_LOW_BOUND:%d",        tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_LOW_BOUND]);
	printk("MAX_Y_UP_BOUND:%d",         tp_multiple_touch_calibration_value[TP_CALI_MAX_Y_UP_BOUND]);
	printk("MIN_X_LOW_BOUND:%d",        tp_multiple_touch_calibration_value[TP_CALI_MIN_X_LOW_BOUND]);
	printk("MIN_X_UP_BOUND:%d",         tp_multiple_touch_calibration_value[TP_CALI_MIN_X_UP_BOUND]);
	printk("MAX_X_LOW_BOUND:%d",        tp_multiple_touch_calibration_value[TP_CALI_MAX_X_LOW_BOUND]);
	printk("MAX_X_UP_BOUND:%d",         tp_multiple_touch_calibration_value[TP_CALI_MAX_X_UP_BOUND]);
	printk("SINGLE_POINT_LOW_BOUND:%d", tp_multiple_touch_calibration_value[TP_CALI_POINT_LOW_BOUND]);
}

void tp_multiple_start_dual_cali_(void)
{
#if defined(__TOUCH_PANEL_MULTITOUCH__)
	tp_cali_mode = TP_DUAL_CALIBRATION_MODE;
	tp_dual_cali_mode = TP_CALI_POINT_LOW_BOUND;
	//memset(tp_multiple_touch_calibration_value, 0, sizeof(kal_int16)*TP_CALI_FINISH);
	memset(tp_dual_init_cali, 0, sizeof(kal_bool)*TP_CALI_FINISH);
#endif //#if defined(__TOUCH_PANEL_MULTITOUCH__)
}
void tp_multiple_stop_dual_cali_(void)
{
#if defined(__TOUCH_PANEL_MULTITOUCH__)
	tp_cali_mode = TP_FINISH_CALIBRATION_MODE;
	tp_dual_cali_mode = TP_CALI_FINISH;
	memset(tp_dual_init_cali, 0, sizeof(kal_bool)*TP_CALI_FINISH);
#endif //#if defined(__TOUCH_PANEL_MULTITOUCH__)
}

kal_bool tp_multiple_dual_cali_pass(Touch_Panel_Dual_Calibration_Mode_enum mode)
{
	return tp_dual_init_cali[mode];
}

//tp_multiple_get_dual_cali: factory mode to save the calibration value.
void tp_multiple_get_dual_cali(kal_int32 yp_yn, kal_int32 rTouch, kal_int32 rTouch1)
{
	// xxx_min: means the min value
	// XXX_MIN: means the step of closer 2 point
	if((tp_cali_mode==TP_DUAL_CALIBRATION_MODE) && (tp_dual_cali_mode<TP_CALI_FINISH))
		printk("tp_dual_cali_mode=%d", tp_dual_cali_mode );
	else
		return;

	if( (tp_dual_cali_mode!=DCL_TP_CALI_SINGLE_POINT) && (rTouch1 > tp_multiple_touch_calibration_value[TP_CALI_POINT_LOW_BOUND]) )//it should be only one touch
		return;

	if(tp_dual_init_cali[tp_dual_cali_mode]==KAL_FALSE)//not initial the value yet
	{
		tp_dual_init_cali[tp_dual_cali_mode] = KAL_TRUE;
		if( (tp_dual_cali_mode >= DCL_TP_CALI_LEFT_TOP_RIGHT_DOWN_MAX) && (tp_dual_cali_mode <= DCL_TP_CALI_RIGHT_TOP_LEFT_DOWN_MIN))
		{
			tp_multiple_calibration_step_max[tp_dual_cali_mode]=yp_yn;
			tp_multiple_calibration_step_min[tp_dual_cali_mode]=yp_yn;
		}
		else if( (tp_dual_cali_mode >= DCL_TP_CALI_LEFT_TOP_DOWN_MAX) && (tp_dual_cali_mode <= DCL_TP_CALI_DOWN_RIGHT_LEFT_MIN))
		{
			tp_multiple_calibration_step_max[tp_dual_cali_mode]=rTouch;
			tp_multiple_calibration_step_min[tp_dual_cali_mode]=rTouch;
		}
		else if(tp_dual_cali_mode==DCL_TP_CALI_SINGLE_POINT)
		{
			tp_multiple_touch_calibration_value[tp_dual_cali_mode] = rTouch1;
		}
	}
	else //if(tp_dual_init_cali==KAL_TRUE)//initial the value
	{
			if( (tp_dual_cali_mode >= DCL_TP_CALI_LEFT_TOP_RIGHT_DOWN_MAX) && (tp_dual_cali_mode <= DCL_TP_CALI_RIGHT_TOP_LEFT_DOWN_MIN))
		{
			if(tp_multiple_calibration_step_max[tp_dual_cali_mode]<yp_yn)
				tp_multiple_calibration_step_max[tp_dual_cali_mode]=yp_yn;
			if(tp_multiple_calibration_step_min[tp_dual_cali_mode]>yp_yn)
				tp_multiple_calibration_step_min[tp_dual_cali_mode]=yp_yn;
		}
			else if( (tp_dual_cali_mode >= DCL_TP_CALI_LEFT_TOP_DOWN_MAX) && (tp_dual_cali_mode <= DCL_TP_CALI_DOWN_RIGHT_LEFT_MIN))
		{
			if(tp_multiple_calibration_step_max[tp_dual_cali_mode]<rTouch)
				tp_multiple_calibration_step_max[tp_dual_cali_mode]=rTouch;
			if(tp_multiple_calibration_step_min[tp_dual_cali_mode]>rTouch)
				tp_multiple_calibration_step_min[tp_dual_cali_mode]=rTouch;
		}
		else if(tp_dual_cali_mode == DCL_TP_CALI_SINGLE_POINT)
		{
			if(tp_multiple_touch_calibration_value[tp_dual_cali_mode]>rTouch1)
				tp_multiple_touch_calibration_value[tp_dual_cali_mode]=rTouch1;
		}
	}//else //if(tp_dual_init_cali==KAL_TRUE)//initial the value
}

//kal_int32 yp_yn_previous[10], rTouch_previous[10];
//kal_int32 yp_yn_w, yp_yn_r, rTouch_w, rTouch_r; //read write pointer of the array.
//clear smooth read write pointer when pen up or down
#endif

void tp_multiple_stop_dual_smooth()
{
	tp_smooth_w = tp_smooth_r = 0;
}

//tp_multiple_get_medium: get stable value, average the middle of previous data.
//Smooth_r: read pointer of previous buff.
//Count: valid value count
//Previous: value buff
//Return average value of middle 60%
kal_int32 tp_multiple_get_medium(kal_int32 smooth_r, kal_int32 count, kal_int32 *previous)
{
	kal_int32 sorting[TP_SMOOTH_ARRAY_SIZE];
	kal_int32 i,j, header, tail, total=0;
	kal_int32 value;
	
	if((tp_log_count)%tp_log_count_base == 0) //print per second
	{
		//drv_trace8(TRACE_GROUP_2, DUAL_TP_PREVIOUS_1, previous[0], previous[1], previous[2], previous[3], previous[4],0,0,0);
		//drv_trace8(TRACE_GROUP_2, DUAL_TP_PREVIOUS_2, previous[5], previous[6], previous[7], previous[8], previous[9],0,0,0);
	}

	for(i=0; i<count; i++) //get from read pointer to write pointer
	{
		value = previous[(smooth_r+i)%TP_SMOOTH_ARRAY_SIZE];
		
		if(i==0) 
		{
			sorting[0] = value;
		}
		else
		{
			for(j=i;j>0;j--) //insertion sorting
			{
				if(value < sorting[j-1])
				{
					sorting[j] = sorting[j-1];
					sorting[j-1] = value;
				}
				else
				{
					sorting[j] = value;
				}
			}
		}//else
	}
	//trimmed mean, Wilcox 2003, delete 20%
	if(count<5)
	{
		header = 0;
		tail = count-1;
	}
	else
	{
		header = count/5;
		tail = count*4/5-1;
	}
	for(i=header; i<=tail; i++)
		total += sorting[i];
	if((tp_log_count)%tp_log_count_base == 0) //print per second
	{
		//drv_trace8(TRACE_GROUP_2, DUAL_TP_SORTING_1, sorting[0], sorting[1], sorting[2], sorting[3], sorting[4],0,0,0 );
		//drv_trace8(TRACE_GROUP_2, DUAL_TP_SORTING_2, sorting[5], sorting[6], sorting[7], sorting[8], sorting[9],0,0,0 );
		//drv_trace4(TRACE_GROUP_2, DUAL_TP_SORTING_RESULT, header, tail, total, 0 );
	}
	return total/(tail-header+1);
}

//tp_multiple_touch_smooth: get yp-yn and rTouch buffer stable(smooth) value, 
//update the read/write pointer and call get_medium API.
void tp_multiple_touch_smooth(kal_int32* yp_yn, kal_int32* rTouch)
{
	kal_int32 count=0;
	
	if((tp_log_count)%tp_log_count_base == 0) //print per second
	{
		////drv_trace4(TRACE_GROUP_2, DUAL_TP_SMOOTH_START, tp_smooth_w, tp_smooth_r, tp_rTouch_previous[tp_smooth_w], *rTouch );
	}
	//update tp_smooth_w
	tp_smooth_w = (tp_smooth_w+1)%TP_SMOOTH_ARRAY_SIZE;
	tp_yp_yn_previous[tp_smooth_w]   = *yp_yn;
	tp_rTouch_previous[tp_smooth_w] = *rTouch;

	//update tp_smooth_r, when buffer is full.
	if(tp_smooth_r < tp_smooth_w) //not wrap around yet
	{
		if(tp_smooth_r == (tp_smooth_w-TP_AVERAGE_NUMBER))
			tp_smooth_r++;
	}
	else //wrap around
	{
		if(tp_smooth_r == (tp_smooth_w+TP_SMOOTH_ARRAY_SIZE-TP_AVERAGE_NUMBER))
		tp_smooth_r = (tp_smooth_r+1)%TP_SMOOTH_ARRAY_SIZE;
	}
	
	if(tp_smooth_w >= tp_smooth_r)
		count = tp_smooth_w - tp_smooth_r + 1;
	else
		count = TP_SMOOTH_ARRAY_SIZE + tp_smooth_w - tp_smooth_r + 1;
	*yp_yn  = tp_multiple_get_medium(tp_smooth_r, count, tp_yp_yn_previous);
	*rTouch = tp_multiple_get_medium(tp_smooth_r, count, tp_rTouch_previous);
	
	if((tp_log_count)%tp_log_count_base == 0) //print per second
	{
		//drv_trace4(TRACE_GROUP_2, DUAL_TP_SMOOTH_END, tp_smooth_r, count, *yp_yn, *rTouch);
	}
}

void tp_multiple_touch_limit_range(kal_uint16 *point, kal_int32 lowBound, kal_int32 upBound)
{
	if(*point<lowBound)
		*point=lowBound;
	else if(*point>upBound)
		*point=upBound;
}

//tp_multiple_stable_boundary: if the point locate at boundary, 0~TP_RANGE, iff different distance > X_resolution/TP_STEP then update the move point.
void tp_multiple_stable_boundary(kal_uint16 *pre, kal_uint16 *current, kal_uint16 coord)
{
	kal_uint16 diff;
	
	TP_RANGE = TP_RANGE>=2?TP_RANGE:2;
	if( (*current<(coord/TP_RANGE)) || (*current>((coord*(TP_RANGE-1))/TP_RANGE))) //x<(X/6) || x>(5X/6)
	{
		if(*pre!=0xFFFF)
		{
			if(*pre>*current)
				diff = *pre-*current;
			else
				diff = *current-*pre;
			if(diff>(coord/TP_STEP)) // x_diff > X/32
				*pre = *current;
			else
				*current = *pre;
		}
		else //new down
		{
			*pre = *current;
		}
	}
}

//tp_multiple_touch_get_2nd_point: calicate yp-yn and rTouch1(pressure) rTouch2, 
//smooth the yp-yn and rTouch
//add 1-3, 2-4 offset and vertical or horiztal offset
//also check if locate at boundary.
kal_bool tp_multiple_touch_get_2nd_point(TouchPanelMultipleEventStruct *dual_touch_event)
{
	kal_int32 yp_yn_adc, delta;
	double rTouch1, rTouch2;
	kal_int32 rTouch, TP_rTouch1,TP_rTouch2;
	
	unsigned int a, b, c;
	
	// handle dual down event.
	// because we need to use rTouch for add X, Y offset and don't know whether it already change in yp-yn process, save them first.
	dual_touch_event->model = 1;
	dual_touch_event->points[1].x = dual_touch_event->points[0].x = gYp;
	dual_touch_event->points[1].y = dual_touch_event->points[0].y = gXp;
	dual_touch_event->points[1].z = dual_touch_event->points[0].z = gZ1;
	
	if(gZ1 == 0) //already up, avoid to divid 0.
	{
		return KAL_FALSE;
	}
		
	tp_log_count++;
	
	yp_yn_adc = gYp-gYn;

	#if 0
	rTouch1 = (double)TP_X_PLATE*(double)(gXp+gXn)/(double)8192*(double)(gZ2-gZ1)/(double)gZ1; // TP_X_PLATE*gXp*((double)(gZ2/gZ1)-1);
	#else
	{ //do_div
		rTouch1 = (double)TP_X_PLATE*(double)(gXp+gXn);
		do_div(rTouch1, (double)8192*(double)(gZ2-gZ1));
		do_div(rTouch1, (double)gZ1);
	}
	#endif
	TP_rTouch1 = (kal_int32)rTouch1;
	//rTouch2 = (double)TP_X_PLATE*(double)((double)(gXp+gXn)/(double)8192)*(double)((double)(4096-gZ1)/(double)gZ1) + (double)TP_Y_PLATE*(double)(((double)(gYp+gYn)/(double)8192)-(double)1); //((1/gZ1)-1) - TP_Y_PLATE*(1-gYp);
	#if 0
	rTouch2 = ((double)TP_X_PLATE*(double)(gXp+gXn)*(double)(4096-gZ1)/(double)gZ1/(double)8192) + \
			  ((double)TP_Y_PLATE*(double)((double)(gYp+gYn)*1000/(double)8192-1000)/1000); //((1/gZ1)-1) - TP_Y_PLATE*(1-gYp);
	#else
	{ //do_div
		double rTouch2_X, rTouch2_Y;
		
		rTouch2_X = (double)TP_X_PLATE*(double)(gXp+gXn)*(double)(4096-gZ1);
		do_div(rTouch2_X, (double)gZ1);
		do_div(rTouch2_X, (double)8192);

		rTouch2_Y = (double)(gYp+gYn)*1000;
		do_div(rTouch2_Y, (double)8192-1000);
		do_div(rTouch2_Y, (double)1000);
		rTouch2_Y = (double)TP_Y_PLATE*(double)rTouch2_Y;

		rTouch2 = rTouch2_X + rTouch2_Y;
	}
	#endif
	TP_rTouch2 = (kal_int32)rTouch2;
	rTouch = TP_rTouch1-TP_rTouch2;
#if 1

	printk("[%s] TP_rTouch1=%d, TP_rTouch2=%d, yp_yn_adc=%d\n", __FUNCTION__, TP_rTouch1, TP_rTouch2, yp_yn_adc);


	if((tp_log_count)%tp_log_count_base == 0) //print per second
	{
		//drv_trace8(TRACE_GROUP_2, DUAL_TP_DATA_XPXN_YPYN, dual_touch_event->points[0].x, (gXp+gXn)/2, dual_touch_event->points[0].y, (gYp+gYn)/2, gXp-gXn, gYp-gYn,0,0);
		//drv_trace8(TRACE_GROUP_2, DUAL_TP_DATA_Z, gZ1, gZ2, TP_rTouch1, TP_rTouch2, TP_rTouch1-TP_rTouch2,0,0,0);
	}

	//if(tp_cali_mode!=TP_DUAL_CALIBRATION_MODE) //always get min/max yp_yn_adc from positive up/negative low bound
	{
		if(yp_yn_adc < tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND])
			yp_yn_adc = tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND];
		else if(yp_yn_adc > tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND])
			yp_yn_adc = tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND];
	}

	//tp_multiple_touch_smooth(&yp_yn_adc, &rTouch); //average the yp-yn and rTouch
	//tp_multiple_get_dual_cali(yp_yn_adc, rTouch, TP_rTouch1); //save the calibration values

	if(0)//(TP_rTouch1>tp_multiple_touch_calibration_value[TP_CALI_POINT_LOW_BOUND])//it should be only one touch
	{
		printk("#### One Touch\n");
		dual_touch_event->model = 1;		
	}
	else //dual touch
	{
		printk("#### Dual Touch\n");

		if(yp_yn_adc < tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND]) //2-4 quadrant
		{
			dual_touch_event->model = 2;
			delta = tp_multiple_touch_offset_X(yp_yn_adc);
			printk("[DUAL][2,4] deltaX=%d\n", delta);
			dual_touch_event->points[1].x += delta;     //4 quadrant
			if(dual_touch_event->points[0].x > delta)
				dual_touch_event->points[0].x -= delta; //2 quadrant
			else
				dual_touch_event->points[0].x = 0;
			
			delta = tp_multiple_touch_offset_Y(yp_yn_adc);
			printk("[DUAL][2,4] deltaY=%d\n", delta);
			dual_touch_event->points[1].y += delta;
			if(dual_touch_event->points[0].y > delta)
				dual_touch_event->points[0].y -= delta; //2 quadrant
			else
				dual_touch_event->points[0].y = 0;
			
			if(yp_yn_adc < (tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_LOW_BOUND]-tp_multiple_touch_calibration_value[TP_CALI_NEGATIVE_UP_BOUND])/2) //yp-yn already small enough means not in the same vertical or horizontal
				return KAL_TRUE;
		}
		else if(yp_yn_adc > tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND]) //1-3 quadrant
		{
			dual_touch_event->model = 2;
			delta = tp_multiple_touch_offset_X(yp_yn_adc);
			printk("[DUAL][1,3] deltaX=%d\n", delta);
			if(dual_touch_event->points[1].x > delta)
				dual_touch_event->points[1].x -= delta; //3 quadrant
			else
				dual_touch_event->points[1].x = 0;
			dual_touch_event->points[0].x += delta;     //1 quadrant
			
			delta = tp_multiple_touch_offset_Y(yp_yn_adc);
			printk("[DUAL][1,3] deltaY=%d\n", delta);
			dual_touch_event->points[1].y += delta;
			if(dual_touch_event->points[0].y > delta)
				dual_touch_event->points[0].y -= delta; //2 quadrant
			else
				dual_touch_event->points[0].y = 0;
			if(yp_yn_adc > (tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_UP_BOUND]-tp_multiple_touch_calibration_value[TP_CALI_POSITIVE_LOW_BOUND])/2) //yp-yn already large enough means not in the same vertical or horizontal
				return KAL_TRUE;
		}
		else
		{
			delta = tp_multiple_touch_vertical_offset(rTouch, dual_touch_event);
			printk("[DUAL][VT] delta=%d\n", delta);
			delta = tp_multiple_touch_horizontal_offset(rTouch, dual_touch_event);
			printk("[DUAL][HZ] delta=%d\n", delta);
		}
	}

	//Limit the coordinate within range
	tp_multiple_touch_limit_range(&dual_touch_event->points[0].x, 0, TP_SCREEN_X_END);
	tp_multiple_touch_limit_range(&dual_touch_event->points[1].x, 0, TP_SCREEN_X_END);
	tp_multiple_touch_limit_range(&dual_touch_event->points[0].y, 0, TP_SCREEN_Y_END);
	tp_multiple_touch_limit_range(&dual_touch_event->points[1].y, 0, TP_SCREEN_Y_END);

//kal_uint16 x0_diff, x1_diff, y0_diff, y1_diff;	
	//tp_multiple_stable_boundary(&tp_previous_x0, &(dual_touch_event->points[0].x), TP_SCREEN_X_END);
	//tp_multiple_stable_boundary(&tp_previous_x1, &(dual_touch_event->points[1].x), TP_SCREEN_X_END);
	//tp_multiple_stable_boundary(&tp_previous_y0, &(dual_touch_event->points[0].y), TP_SCREEN_Y_END);
	//tp_multiple_stable_boundary(&tp_previous_y1, &(dual_touch_event->points[1].y), TP_SCREEN_Y_END);
#endif

	if(dual_touch_event->model == 1)
		return KAL_FALSE;  //signal touch
	else return KAL_TRUE;   //touch touch
}

#include <linux/delay.h>

//get xp, xn, yp, yn, z1, z2
//get mode A~F (MT6260)
void tp_multiple_touch_get_data()
{
	//Differential mode 
	gXp = ts_udvt_read(AUXADC_TS_POSITION_XP) * AUXADC_CALI_XP;
	gXn = ts_udvt_read(AUXADC_TS_POSITION_XN) * AUXADC_CALI_XN;
	gYp = ts_udvt_read(AUXADC_TS_POSITION_YP) * AUXADC_CALI_YP;
	gYn = ts_udvt_read(AUXADC_TS_POSITION_YN) * AUXADC_CALI_YN;
	gZ1 = ts_udvt_read(AUXADC_TS_POSITION_Z1);
	gZ2 = ts_udvt_read(AUXADC_TS_POSITION_Z2);

	// single end mode,  dual-touch bit enabled, Ron
	__raw_writew(__raw_readw(AUXADC_TS_CMD) | 0x1<<2, AUXADC_TS_CMD); 
	*(volatile u32*)AUXADC_CON2 |= 0x1 << 10; 
	*(volatile u32*)0xF020540C = (0x1 << 4) | (0x0 << 0); 
	mdelay(5);

	tp_Yser = gDualTouchB = ts_udvt_read(AUXADC_TS_POSITION_B); /*TS_CMD_ADDR_YN|TS_CMD_ADDR_DUAL*/
	tp_Xser = gDualTouchF = ts_udvt_read(AUXADC_TS_POSITION_F); /*TS_CMD_ADDR_XN|TS_CMD_ADDR_DUAL*/

	*(volatile u32*)0xF020540C = 0x88;
	*(volatile u32*)AUXADC_CON2 &= ~(0x1 << 10);
	__raw_writew(__raw_readw(AUXADC_TS_CMD) & ~(0x1<<2), AUXADC_TS_CMD);

	mdelay(5);	
	printk("### gXp=%d, gXn=%d, gYp=%d, gYn=%d, gZ1=%d, gZ2=%d, gDualTouchB=%d, gDualTouchF=%d\n", gXp, gXn, gYp, gYn, gZ1, gZ2, gDualTouchB, gDualTouchF);
}

//tp_multiple_touch_cb_get_2nd_point: get the adc value then predict 2 points' location
kal_bool tp_multiple_touch_cb_get_2nd_point(TouchPanelMultipleEventStruct *dual_touch_event)
{
	tp_multiple_touch_get_data();
	//return TRUE;
	return tp_multiple_touch_get_2nd_point(dual_touch_event);
}
//tp_multiple_touch_down_get_2nd_point:
//clear smooth buffer(yp-yn and rTouch buffer) and value buffer(xp, xn, yp, yn) read/write pointer then get 2 points' location
kal_bool tp_multiple_touch_down_get_2nd_point(TouchPanelMultipleEventStruct *dual_touch_event)
{
	tp_multiple_stop_dual_smooth();
	
	tp_previous_x0 = tp_previous_y0 = 0xFFFF; //read write pointer of the array.
	tp_previous_x1 = tp_previous_y1 = 0xFFFF; //read write pointer of the array.
	//alread got gXp ~ gZ2 in HISR
	return tp_multiple_touch_get_2nd_point(dual_touch_event);
}

#endif //CONFIG_MTK_LDVT_ADC_TS

