#include "xhci-mtk-power.h"
#include "xhci.h"
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>
#include <linux/delay.h>


/* set 1 to PORT_POWER of PORT_STATUS register of each port */
void enableXhciAllPortPower(struct xhci_hcd *xhci){
	int i;
	u32 port_id, temp;
	u32 __iomem *addr;
    u32	num_u3_port;
	u32 num_u2_port;

	num_u3_port = SSUSB_U3_PORT_NUM(readl(SSUSB_IP_CAP));
	num_u2_port = SSUSB_U2_PORT_NUM(readl(SSUSB_IP_CAP));

	for(i=1; i<=num_u3_port; i++){
		port_id=i;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(port_id-1 & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		temp |= PORT_POWER;
		xhci_writel(xhci, temp, addr);
	}

	for(i=1; i<=num_u2_port; i++){
		port_id = i + num_u3_port;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(port_id-1 & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		temp |= PORT_POWER;
		xhci_writel(xhci, temp, addr);
	}
}

void enableAllClockPower(){

	int i;
	u32 temp;
	u32 num_u3_port;
	u32 num_u2_port;

	num_u3_port = SSUSB_U3_PORT_NUM(readl(SSUSB_IP_CAP));
	num_u2_port = SSUSB_U2_PORT_NUM(readl(SSUSB_IP_CAP));

	//2.	Enable xHC
#if TEST_OTG
	writel(readl(SSUSB_IP_PW_CTRL) & (~SSUSB_IP_SW_RST), SSUSB_IP_PW_CTRL);
	writel(readl(SSUSB_IP_PW_CTRL_1) & (~SSUSB_IP_PDN), SSUSB_IP_PW_CTRL_1);
#else
	writel(readl(SSUSB_IP_PW_CTRL) | (SSUSB_IP_SW_RST), SSUSB_IP_PW_CTRL);
	writel(readl(SSUSB_IP_PW_CTRL) & (~SSUSB_IP_SW_RST), SSUSB_IP_PW_CTRL);
	writel(readl(SSUSB_IP_PW_CTRL_1) & (~SSUSB_IP_PDN), SSUSB_IP_PW_CTRL_1);
#endif

	//1.	Enable target ports
	for(i=0; i<num_u3_port; i++){
		temp = readl(SSUSB_U3_CTRL(i));
		temp = temp & (~SSUSB_U3_PORT_PDN) & (~SSUSB_U3_PORT_DIS) | SSUSB_U3_PORT_HOST_SEL;
		writel(temp, SSUSB_U3_CTRL(i));
	}

    /*
     * FIXME: clock is the correct 30MHz, if the U3 device is enabled
     */
    #if 0
    temp = readl(SSUSB_U3_CTRL(0));
    temp = temp & (~SSUSB_U3_PORT_PDN) & (~SSUSB_U3_PORT_DIS) & (~SSUSB_U2_PORT_HOST_SEL);
    writel(temp, SSUSB_U3_CTRL(0));
    #endif

	for(i=0; i<num_u2_port; i++){
		temp = readl(SSUSB_U2_CTRL(i));
#if TEST_OTG
		temp = temp & (~SSUSB_U2_PORT_PDN) & (~SSUSB_U2_PORT_DIS) & (~SSUSB_U2_PORT_HOST_SEL);
		temp = temp | SSUSB_U2_PORT_OTG_SEL | SSUSB_U2_PORT_OTG_MAC_AUTO_SEL;
#else
		temp = temp & (~SSUSB_U2_PORT_PDN) & (~SSUSB_U2_PORT_DIS) | SSUSB_U2_PORT_HOST_SEL;
#endif
		writel(temp, SSUSB_U2_CTRL(i));
	}

	msleep(100);
}

//called after HC initiated
void disableAllClockPower(){
	int i;
	u32 temp;
	int num_u3_port;
	int num_u2_port;

#if TEST_OTG
		return;
#endif

	num_u3_port = SSUSB_U3_PORT_NUM(readl(SSUSB_IP_CAP));
	num_u2_port = SSUSB_U2_PORT_NUM(readl(SSUSB_IP_CAP));

	//disable target ports
	for(i=0; i<num_u3_port; i++){
		temp = readl(SSUSB_U3_CTRL(i));
		temp = temp | SSUSB_U3_PORT_PDN;
		writel(temp, SSUSB_U3_CTRL(i));
	}
	for(i=0; i<num_u2_port; i++){
		temp = readl(SSUSB_U2_CTRL(i));
		temp = temp | SSUSB_U2_PORT_PDN;
		writel(temp, SSUSB_U2_CTRL(i));
	}
	msleep(100);
}

//(X)disable clock/power of a port
//(X)if all ports are disabled, disable IP ctrl power
//disable all ports and IP clock/power, this is just mention HW that the power/clock of port
//and IP could be disable if suspended.
//If doesn't not disable all ports at first, the IP clock/power will never be disabled
//(some U2 and U3 ports are binded to the same connection, that is, they will never enter suspend at the same time
//port_index: port number
//port_rev: 0x2 - USB2.0, 0x3 - USB3.0 (SuperSpeed)
void disablePortClockPower(int port_index, int port_rev){
	int i;
	u32 temp;
	int real_index;

#if TEST_OTG
	return;
#endif

	real_index = port_index;

	if(port_rev == 0x3){
		temp = readl(SSUSB_U3_CTRL(real_index));
		temp = temp | (SSUSB_U3_PORT_PDN);
		writel(temp, SSUSB_U3_CTRL(real_index));
	}
	else if(port_rev == 0x2){
		temp = readl(SSUSB_U2_CTRL(real_index));
		temp = temp | (SSUSB_U2_PORT_PDN);
		writel(temp, SSUSB_U2_CTRL(real_index));
	}
	writel(readl(SSUSB_IP_PW_CTRL_1) | (SSUSB_IP_PDN), SSUSB_IP_PW_CTRL_1);
}

//if IP ctrl power is disabled, enable it
//enable clock/power of a port
//port_index: port number
//port_rev: 0x2 - USB2.0, 0x3 - USB3.0 (SuperSpeed)
void enablePortClockPower(int port_index, int port_rev){
	int i;
	u32 temp;
	int real_index;

#if TEST_OTG
	return;
#endif

	real_index = port_index;
	writel(readl(SSUSB_IP_PW_CTRL_1) & (~SSUSB_IP_PDN), SSUSB_IP_PW_CTRL_1);

	if(port_rev == 0x3){
		temp = readl(SSUSB_U3_CTRL(real_index));
		temp = temp & (~SSUSB_U3_PORT_PDN);
		writel(temp, SSUSB_U3_CTRL(real_index));
	}
	else if(port_rev == 0x2){
		temp = readl(SSUSB_U2_CTRL(real_index));
		temp = temp & (~SSUSB_U2_PORT_PDN);
		writel(temp, SSUSB_U2_CTRL(real_index));
	}
}
