package com.mediatek.systemui.plugin;

import com.mediatek.op01.plugin.R;

/**
 * M: This class define the OP01 constants of telephony icons.
 */
final class TelephonyIcons {

    /** Signal level icons for normal. @{ */

    static final int[][] TELEPHONY_SIGNAL_STRENGTH = {
        { R.drawable.stat_sys_signal_0,
          R.drawable.stat_sys_signal_1,
          R.drawable.stat_sys_signal_2,
          R.drawable.stat_sys_signal_3,
          R.drawable.stat_sys_signal_4,
          R.drawable.stat_sys_signal_5},
        { R.drawable.stat_sys_signal_0_fully,
          R.drawable.stat_sys_signal_1_fully,
          R.drawable.stat_sys_signal_2_fully,
          R.drawable.stat_sys_signal_3_fully,
          R.drawable.stat_sys_signal_4_fully,
          R.drawable.stat_sys_signal_5_fully}
    };

    static final int[][] DATA_SIGNAL_STRENGTH = TELEPHONY_SIGNAL_STRENGTH;

    /** Signal level icons for normal. @} */

    /** Data connection type icons. @{ */

    static final int[] DATA_T = {
        R.drawable.stat_sys_gemini_data_connected_t_blue,
        R.drawable.stat_sys_gemini_data_connected_t_orange,
        R.drawable.stat_sys_gemini_data_connected_t_green,
        R.drawable.stat_sys_gemini_data_connected_t_purple
    };
    static final int[] DATA_T_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_t_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_t_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_t_green_roam,
        R.drawable.stat_sys_gemini_data_connected_t_purple_roam
    };

    /** Data connection type icons. @} */

    /** Data activity type icons. @{ */

    static final int[] DATA_ACTIVITY = {
        R.drawable.stat_sys_signal_not_inout,
        R.drawable.stat_sys_signal_in,
        R.drawable.stat_sys_signal_out,
        R.drawable.stat_sys_signal_inout
    };

    static final int[][] DATA_ACTIVITY_S = {
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_blue,
          R.drawable.stat_sys_signal_out_blue,
          R.drawable.stat_sys_signal_inout_blue},
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_orange,
          R.drawable.stat_sys_signal_out_orange,
          R.drawable.stat_sys_signal_inout_orange},
	    { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_green,
          R.drawable.stat_sys_signal_out_green,
          R.drawable.stat_sys_signal_inout_green},
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_purple,
          R.drawable.stat_sys_signal_out_purple,
          R.drawable.stat_sys_signal_inout_purple}
    }; 
    /** Data activity type icons. @} */

}

