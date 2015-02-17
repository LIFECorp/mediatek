#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "cli_internals.h"

Tfa98xx_Error_t patch(Tfa98xx_handle_t handle, const char* fileName)
{
	FILE* f;
	Tfa98xx_Error_t err;

	f = fopen(fileName, "rb");
  if (f==NULL)
  {
    fprintf(stderr, "Unable to open %s for input\n", fileName);
    err = Tfa98xx_Error_Bad_Parameter;
  }
  else
  {
	  int fileSize, ret;
	  unsigned char* buffer;

    ret = fseek(f, 0, SEEK_END);
	  assert(ret == 0);
    fileSize = ftell(f);
	  assert(fileSize>=0);
    ret = fseek(f, 0, SEEK_SET);
	  assert(ret == 0);
    buffer = (unsigned char*)malloc(fileSize);
	  assert(buffer != NULL);
    ret = fread(buffer, 1, fileSize, f);
	  assert(ret == fileSize);
	  fclose(f);
	  err = Tfa98xx_DspPatch(handle, fileSize, buffer);
	  free(buffer);
  }
  return err;
}
