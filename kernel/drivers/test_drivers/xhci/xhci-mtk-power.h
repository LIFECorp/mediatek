#ifndef _XHCI_MTK_POWER_H
#define _XHCI_MTK_POWER_H

#include <linux/usb.h>
#include "xhci.h"
#include "mtk-test-lib.h"

void enableXhciAllPortPower(struct xhci_hcd *xhci);
void enableAllClockPower();
void disablePortClockPower();
void enablePortClockPower(int port_index, int port_rev);
void disableAllClockPower();

#endif
