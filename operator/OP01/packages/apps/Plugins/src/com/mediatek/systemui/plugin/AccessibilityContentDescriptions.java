package com.mediatek.systemui.plugin;

import com.mediatek.op01.plugin.R;

/**
 * M: This class define the OP01 constants of signal strength description.
 */
final class AccessibilityContentDescriptions {

    private AccessibilityContentDescriptions() {}

    static final int[] PHONE_SIGNAL_STRENGTH = {
        R.string.accessibility_no_phone,
        R.string.accessibility_phone_one_bar,
        R.string.accessibility_phone_two_bars,
        R.string.accessibility_phone_three_bars,
        R.string.accessibility_phone_four_bars,
        R.string.accessibility_phone_signal_full
    };

}
