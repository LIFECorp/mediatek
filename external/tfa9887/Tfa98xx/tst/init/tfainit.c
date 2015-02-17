/*
 * tfainit.c
 *
 * 	small test to bringup the tfa9887
 *
 *  Created on: Mar 23, 2012
 *      Author: nlv02095
 */
#include <stdio.h>
#include "Tfa9887.h"

static Tfa9887_handle_t handle_L;

#define assert(ERR)

static void InitSettings(Tfa9887_handle_t handle, Tfa9887_Channel_t channel)
{
	Tfa9887_Error_t err;

	err = Tfa9887_Init(handle);
	assert(err == Tfa9887_Error_Ok);

	err = Tfa9887_SetSampleRate(handle, 44100);
	assert(err == Tfa9887_Error_Ok);
	err = Tfa9887_SelectChannel(handle, channel);
	assert(err == Tfa9887_Error_Ok);
	err = Tfa9887_SelectAmplifierInput(handle, Tfa9887_AmpInputSel_DSP);
	assert(err == Tfa9887_Error_Ok);

	/* start at 0 dB */
	err = Tfa9887_SetVolume(handle, 0.0);
	assert(err == Tfa9887_Error_Ok);

	err = Tfa9887_Powerdown(handle, 0);
	assert(err == Tfa9887_Error_Ok);

	err=err; //unused
}
int main(int argv, char *argc[])
{
	Tfa9887_Error_t err;
	handle_L = -1;

	err = Tfa9887_Open(0x6C, &handle_L);
	Tfa9887_Powerdown(handle_L, 1);
	InitSettings(handle_L, Tfa9887_Channel_L);

	return err;
}



