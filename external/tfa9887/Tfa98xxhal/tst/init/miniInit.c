/*
 * miniInit.c
 *
 * quicksetup of the TFA9887.
 * assume a 2.2uH coil
 *
 *  Created on: Mar 26, 2012
 *      Author: nlv02095
 */

#include <stdio.h>
#include "NXP_I2C.h"

unsigned char bus=0x36*2;
#define REG(sub) (sub<<8|bus)

void writereg(unsigned char reg, unsigned short data)
{
	unsigned char wdata[4];

	wdata[0] = bus;
	wdata[1] = reg;
	wdata[2] = data >> 8;
	wdata[3] = data & 0xff;

	NXP_I2C_Write (sizeof(wdata), wdata);

}

int main(int argc, char *argv[])
{
	unsigned short version;
	unsigned char reg, rdata[3], wdata[2];

	reg=3;
	wdata[0]=bus;
	wdata[1]=reg;
	rdata[0]=bus|1;
	/* slave address and subaddress ; slave address and 2 bytes with data from the register */
	NXP_I2C_WriteRead(2, wdata, 2, rdata);
	version = rdata[1]<<8|rdata[2];
	printf("version=0x%x\n", version);

	// The following I2C code needs to be send for a quick check if audio is coming out, these settings should not be used for audio quality measurements since more registers need to be written for this:

	//Write
	writereg( 4,0x880B ); //48 kHz I2S with coolflux in Bypass
//	writereg( 9,0x0619 ); //2.2 uF coil and other system settings
	writereg( 9,0x0219 ); //1.0 uF coil and other system settings
	writereg( 9,0x0618  ); //power on

	bus=0x37*2;
	NXP_I2C_WriteRead(2, wdata, 2, rdata);
	version = rdata[1]<<8|rdata[2];
	printf("version=0x%x\n", version);

	writereg( 4,0x880B ); //48 kHz I2S with coolflux in Bypass
	writereg( 9,0x0219 ); //1.0 uF coil and other system settings
	writereg( 9,0x0618  ); //power on

	return 1;
}
