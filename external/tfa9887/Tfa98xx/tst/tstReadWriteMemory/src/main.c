#include <Tfa98xx.h>
#include <Tfa98xx_Registers.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <NXP_I2C.h>
#ifndef WIN32
// need PIN access
#include <inttypes.h>
#include <lxScribo.h>
#endif

#ifndef WIN32

#define Sleep(ms) usleep((ms)*1000)
#define _fileno fileno
#define _GNU_SOURCE   /* to avoid link issues with sscanf on NxDI? */
#endif

#define SAMPLE_RATE 44100

#ifdef WIN32
// cwd = dir where vcxproj is
#define LOCATION_FILES ".\\"
#else
// cwd = linux dir
#define LOCATION_FILES "../win/exTfa98xx/"
#endif

/* ROM patch */
#define PATCH_FILENAME "..\\..\\..\\..\\Apps\\Maximus\\App_Demo_Maximus\\coolflux\\FullCode87.patch"

/* load a DSP ROM code patch from file */
static void dspPatch(Tfa98xx_handle_t handle, const char* fileName)
{
	int ret;
	int fileSize;
	unsigned char* buffer;
	FILE* f;
	struct stat st;
	Tfa98xx_Error_t err;

	printf("Using patch %s\n", fileName);

	f=fopen(fileName, "rb");
	assert(f!=NULL);
	ret = fstat(_fileno(f), &st);
	assert(ret == 0);
	fileSize = st.st_size;
	buffer = malloc(fileSize);
	assert(buffer != NULL);
	ret = fread(buffer, 1, fileSize, f);
	assert(ret == fileSize);
	err = Tfa98xx_DspPatch(handle, fileSize, buffer);
	assert(err == Tfa98xx_Error_Ok);
	fclose(f);
	free(buffer);
}
static void coldStartup(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	unsigned short status;

	/* load the optimal TFA9887 in HW settings */
	err = Tfa98xx_Init(handle);
	assert(err == Tfa98xx_Error_Ok);

	err = Tfa98xx_SetSampleRate(handle, SAMPLE_RATE);
	assert(err == Tfa98xx_Error_Ok);

	err = Tfa98xx_Powerdown(handle, 0);
	assert(err == Tfa98xx_Error_Ok);

	printf("Waiting for IC to start up\n");
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
	assert(err == Tfa98xx_Error_Ok);
	while ( (status & TFA98XX_STATUS_PLLS) == 0)
	{
		/* not ok yet */
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
		assert(err == Tfa98xx_Error_Ok);
	}

	dspPatch(handle, LOCATION_FILES "coldboot.patch");

	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUS, &status);
	assert(err == Tfa98xx_Error_Ok);

	assert(status & TFA98XX_STATUS_ACS);  /* ensure cold booted */

	/* cold boot, need to load all parameters and patches */
	/* patch the ROM code */
	dspPatch(handle, LOCATION_FILES PATCH_FILENAME);
	
}

static void check_ROM_contents(Tfa98xx_handle_t handle, Tfa98xx_DMEM_e which_mem, char* filename)
{
	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
	FILE* memFile;
	char* scan_format = "@%06x %06x\n";

	/* check that XROM contents is correct */
	memFile = fopen(filename, "r");
	assert(memFile != NULL);

	while (!feof(memFile))
	{
		int ret, offset, data, output;

		ret = fscanf(memFile, scan_format, &offset, &data);
		if (data >= 8388608)
		{
			data -= 2*8388608; /* signed extend */
		}
		if (ret == 2)
		{
			err = Tfa98xx_DspReadMemory(handle, which_mem, (unsigned short)offset, 1 /* number words to read */, &output);
			assert(err == Tfa98xx_Error_Ok);
			assert(output == data);
		}
	}
	fclose(memFile);
}

static void testXmem(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	int buffer[10];
	int check_buffer[10];
	int i;

	/* write and read back, see memory map for safe range of memory to be written */
	for (i=0; i<10; ++i)
	{
		buffer[i] = rand();
	}
	memset(check_buffer, 0, 10*sizeof(int));
	err = Tfa98xx_DspWriteMemory(handle, Tfa98xx_DMEM_XMEM, 3526 /* between top most var and stack */, 10, buffer);
	assert(err == Tfa98xx_Error_Ok);

	err = Tfa98xx_DspReadMemory(handle, Tfa98xx_DMEM_XMEM, 3526 /* offset */, 10 /* number words to read */, check_buffer);
    assert(err == Tfa98xx_Error_Ok);
	assert(0 == memcmp(buffer, check_buffer, 10*sizeof(int)));

}

static void checkIOi2c(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
	int output;
	unsigned short reg;

	/* use registers which are fully readable (no bits masked) */
	err = Tfa98xx_ReadRegister16(handle, 0x3, &reg);
	assert(err == Tfa98xx_Error_Ok);
	err = Tfa98xx_DspReadMemory(handle, Tfa98xx_DMEM_IOMEM, (unsigned short)0x8003 /* REV register */, 1 /* number words to read */, &output);
	assert(err == Tfa98xx_Error_Ok);
	assert(output == reg);

	err = Tfa98xx_ReadRegister16(handle, 0x5, &reg);
	assert(err == Tfa98xx_Error_Ok);
	err = Tfa98xx_DspReadMemory(handle, Tfa98xx_DMEM_IOMEM, (unsigned short)0x8005 /* DCDC register */, 1 /* number words to read */, &output);
	assert(err == Tfa98xx_Error_Ok);
	assert(output == reg);
}

static void testIOmem(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err;
	int buffer[3];
	int check_buffer[3];
	int i;

	/* write and read back, see memory map for safe range of memory to be written */
	for (i=0; i<3; ++i)
	{
		buffer[i] = rand();
	}
	memset(check_buffer, 0, 3*sizeof(int));
	err = Tfa98xx_DspWriteMemory(handle, Tfa98xx_DMEM_IOMEM, 0x301 /* T&P register */, 3, buffer);
	assert(err == Tfa98xx_Error_Ok);
	err = Tfa98xx_DspReadMemory(handle, Tfa98xx_DMEM_IOMEM, 0x301 /* offset */, 3 /* number words to read */, check_buffer);
    assert(err == Tfa98xx_Error_Ok);
	assert(0 == memcmp(buffer, check_buffer, 3*sizeof(int)));

	memset(check_buffer, 0, 3*sizeof(int));
	err = Tfa98xx_DspWriteMemory(handle, Tfa98xx_DMEM_IOMEM, 0x8064 /* MTP_INP register */, 1, buffer);
	assert(err == Tfa98xx_Error_Ok);
	err = Tfa98xx_DspReadMemory(handle, Tfa98xx_DMEM_IOMEM, 0x8064 /* offset */, 1 /* number words to read */, check_buffer);
    assert(err == Tfa98xx_Error_Ok);
	assert(0 == memcmp(buffer, check_buffer, 1*sizeof(int)));
}

static void testYmem(Tfa98xx_handle_t handle)
{
	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
	int buffer[20];
	int check_buffer[20];
	int i;

	/* write and read back, see memory map for safe range of memory to be written */
	for (i=0; i<20; ++i)
	{
		buffer[i] = rand();
	}
	memset(check_buffer, 0, 10*sizeof(int));
	err = Tfa98xx_DspWriteMemory(handle, Tfa98xx_DMEM_YMEM, 421 , 20, buffer);
	assert(err == Tfa98xx_Error_Ok);

	err = Tfa98xx_DspReadMemory(handle, Tfa98xx_DMEM_YMEM, 421 /* offset */, 20 /* number words to read */, check_buffer);
    assert(err == Tfa98xx_Error_Ok);
	assert(0 == memcmp(buffer, check_buffer, 20*sizeof(int)));

}

int main(/* int argc, char* argv[] */)
{
	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
	Tfa98xx_handle_t handle;
    unsigned char slave_address;

	/* create handle */
    /* try all possible addresses, stop at the first found */
    for (slave_address = 0x68; slave_address <= 0x6E ; slave_address+=2)
    {
        printf("Trying slave address 0x%x\n", slave_address);
   	    err = Tfa98xx_Open(slave_address, &handle);
	    if (err == Tfa98xx_Error_Ok)
        {
            break;
        }
    }
    /* should have found a device */
    assert(err == Tfa98xx_Error_Ok);


	coldStartup(handle);
	Sleep(1000);

	/* testing while the DSP is waiting for configured bit */
	/* this should fail, for testing the mechanism */
	//check_ROM_contents(handle, Tfa98xx_DMEM_XMEM, "..\\..\\..\\..\\Apps\\Maximus\\App_Demo_Maximus\\coolflux\\Release87DRC\\App_Demo_Maximus_release.cf6.XMEM");

	/* all the following tests should succeed */
	//check_ROM_contents(handle, Tfa98xx_DMEM_XMEM, "..\\..\\..\\..\\Apps\\Maximus\\App_Demo_Maximus\\coolflux\\Release87\\App_Demo_Maximus_release.cf6.XMEM");
	//check_ROM_contents(handle, Tfa98xx_DMEM_YMEM, "..\\..\\..\\..\\Apps\\Maximus\\App_Demo_Maximus\\coolflux\\Release87\\App_Demo_Maximus_release.cf6.YMEM");
	testXmem(handle);
	testYmem(handle);
	testIOmem(handle);
	checkIOi2c(handle);

	err = Tfa98xx_SetConfigured(handle);
    assert(err == Tfa98xx_Error_Ok);

	//check_ROM_contents(handle, Tfa98xx_DMEM_XMEM, "..\\..\\..\\..\\Apps\\Maximus\\App_Demo_Maximus\\coolflux\\Release87\\App_Demo_Maximus_release.cf6.XMEM");
	//check_ROM_contents(handle, Tfa98xx_DMEM_YMEM, "..\\..\\..\\..\\Apps\\Maximus\\App_Demo_Maximus\\coolflux\\Release87\\App_Demo_Maximus_release.cf6.YMEM");
	testXmem(handle);
	testYmem(handle);
	testIOmem(handle);
	checkIOi2c(handle);
	
	return 0;
}
