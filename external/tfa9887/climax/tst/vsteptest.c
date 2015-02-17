#include <stdio.h>
#include <sys/types.h>
#include "tfa98xx.h"

int main(int argc, char *argv[]) {
	FILE *f;
	nxpTfa98xxVolumeStepFile_t file;
	char ID[7];
	int step, steps;
	unsigned char *ptr;

	if (argc > 1)
		f = fopen(argv[1], "rb");
	else {
		printf("no arg");
		return;
	}

	fread((void*) &file, sizeof(nxpTfa98xxVolumeStepFile_t), 1, f);
	ID[7] = 0;
	strncpy(ID, file.id, 6);
	printf("nxpTfa98xxVolumeStep_t:%d\n",sizeof(nxpTfa98xxVolumeStep_t));
	printf("id=%s\n", ID);
	printf("size=%d, crc=0x%04x\n", file.size, file.crc);
	steps = file.size / sizeof(nxpTfa98xxVolumeStep_t);
	for (step = 0; step < 16; step++) {
		printf("att[%d] %f ", step, file.vstep[step].attenuation);
		ptr = &file.vstep[step].attenuation;
		printf("%02x %02x %02x %02x\n", ptr[3], ptr[2], ptr[1], ptr[0]);

	}

}
