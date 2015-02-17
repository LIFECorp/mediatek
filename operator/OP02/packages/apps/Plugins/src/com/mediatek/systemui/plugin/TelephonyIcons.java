package com.mediatek.systemui.plugin;

import com.mediatek.op02.plugin.R;

/**
 * M: This class define the OP02 constants of telephony icons.
 */
final class TelephonyIcons {

    /** Data connection type icons for roaming. @{ */

    static final int[] DATA_1X_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_1x_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_green_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_white_roam
    };
    
    static final int[] DATA_3G_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_3g_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_green_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_white_roam
    };
    
    static final int[] DATA_4G_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_4g_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_green_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_white_roam
    };
    
    static final int[] DATA_E_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_e_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_e_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_e_green_roam,
        R.drawable.stat_sys_gemini_data_connected_e_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_e_white_roam
    };
    
    static final int[] DATA_G_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_g_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_g_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_g_green_roam,
        R.drawable.stat_sys_gemini_data_connected_g_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_g_white_roam
    };
    
    static final int[] DATA_H_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_h_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_h_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_h_green_roam,
        R.drawable.stat_sys_gemini_data_connected_h_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_h_white_roam
    };

    static final int[] DATA_H_PLUS_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_h_plus_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_green_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_white_roam
    };
    
    static final int[][] DATA_ROAM = {
        DATA_1X_ROAM,
        DATA_3G_ROAM,
        DATA_4G_ROAM,
        DATA_E_ROAM,
        DATA_G_ROAM,
        DATA_H_ROAM,
        DATA_H_PLUS_ROAM
    };

    /** Data connection type icons for roaming. @} */

    /** Network type icons. @{ */

    static final int[] NETWORK_TYPE_G = {
        R.drawable.stat_sys_gemini_signal_g_blue,
        R.drawable.stat_sys_gemini_signal_g_orange,
        R.drawable.stat_sys_gemini_signal_g_green,
        R.drawable.stat_sys_gemini_signal_g_purple,
        R.drawable.stat_sys_gemini_signal_g_white
    };
    
    static final int[] NETWORK_TYPE_3G = {
        R.drawable.stat_sys_gemini_signal_3g_blue,
        R.drawable.stat_sys_gemini_signal_3g_orange,
        R.drawable.stat_sys_gemini_signal_3g_green,
        R.drawable.stat_sys_gemini_signal_3g_purple,
        R.drawable.stat_sys_gemini_signal_3g_white
    };

    static final int[][] NETWORK_TYPE = {
        NETWORK_TYPE_G,
        NETWORK_TYPE_3G
    };

    /** Network type icons. @} */

}

