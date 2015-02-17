/*
 * i2ccall.c
 *
 * simple test for the interface calls
 *
 *  Created on: Mar 23, 2012
 *      Author: nlv02095
 */
#include <stdio.h>
#include "NXP_I2C.h"

int main(int argv, char *argc[])
{
	char buf[]="0123";

	NXP_I2C_EnableLogging(1);
	NXP_I2C_Write(sizeof(buf), buf);
}




