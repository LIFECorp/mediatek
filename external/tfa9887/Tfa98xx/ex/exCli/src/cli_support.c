#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cli_internals.h"

static Tfa98xx_handle_t h;

int ReportIfError(Tfa98xx_Error_t err)
{
  int thereIsAnError = err != Tfa98xx_Error_Ok;

  if (thereIsAnError)
  {
    fprintf(stderr, "Tfa98xx_Error = %s\n", Tfa98xx_GetErrorString(err));
  }
  return thereIsAnError;
}

int OpenSession(unsigned char slaveAddr)
{
  Tfa98xx_Error_t err;

	err = Tfa98xx_Open(slaveAddr, &h);
	if (!ReportIfError(err))
	{
		err = Tfa98xx_Init(h);
	}
	return ReportIfError(err);
}

void CloseSession(void)
{
  Tfa98xx_Error_t err;

	err = Tfa98xx_Close(h);
  ReportIfError(err);
}

void ExecuteCommand(const char* pCmdStr, int* keepParsing, int* errorInExecution)
{
  int cmdIdx, i1, paramsOK;
  unsigned u1, u2;
  float f1;
  unsigned short w16;
  char cmdName[CMDNAME_MAXLEN];
  Tfa98xx_Error_t err;
  const char* pParamStr;

  *keepParsing = 1;
  sscanf(pCmdStr, "%s", cmdName);
  cmdIdx = GetCmdIdx(cmdName);
  *errorInExecution = 0;
  pParamStr = pCmdStr+strlen(cmdName);

  switch (cmdIdx)
  {
  case 0:
    /* Comment line : ignore it */
    paramsOK = 1;
    err = Tfa98xx_Error_Ok;
    break;
  case 1:
    PrintCommandList();
    paramsOK = 1;
    err = Tfa98xx_Error_Ok;
    break;
  case 2:
    /* fprintf(stdout, "Quitting\n"); */
    *keepParsing = 0;
    paramsOK = 1;
    err = Tfa98xx_Error_Ok;
    break;
  case 3:
    paramsOK = 1 == sscanf(pParamStr, "%d", &i1);
    if (paramsOK)
    {
      err = Tfa98xx_Powerdown(h, i1);
    }
    break;
  case 4:
    paramsOK = 2 == sscanf(pParamStr, "%x %x",  &u1, &u2);
    if (paramsOK)
    {
      err = Tfa98xx_WriteRegister16(h, (unsigned char)u1, (unsigned short)u2);
    }
    break;
  case 5:
    paramsOK = 1 == sscanf(pParamStr, "%x",  &u1);
    if (paramsOK)
    {
      err = Tfa98xx_ReadRegister16(h, (unsigned char)u1, &w16);
      if (Tfa98xx_Error_Ok==err) fprintf(stdout, "%04x\n", (unsigned)w16);
    }
    break;
  case 6:
    paramsOK = strlen(pParamStr) > 1;
    if (paramsOK)
    {
      err = patch(h, pParamStr+1);
    }
    break;
  case 7:
    paramsOK = strlen(pParamStr) == 0;
    if (paramsOK)
    {
      err = Tfa98xx_SetConfigured(h);
    }
    break;
  case 8:
    paramsOK = 1 == sscanf(pParamStr, "%f",  &f1);
    if (paramsOK)
    {
      err = Tfa98xx_SetVolume(h, f1);
    }
    break;
  default:
    fprintf(stderr, "Command name not recognised\n");
    paramsOK = 0;
    err = Tfa98xx_Error_Ok; /* this shouldn't matter */
    *errorInExecution = 1;
  }

  if (paramsOK)
  {
    /* the API call was executed, so check 'err' */
    *errorInExecution = ReportIfError(err);
  }
  else
  {
    if (0 == *errorInExecution)
    {
      fprintf(stderr, "Wrong number of parameters, or unexpected format\n");
    }
  }
}
