#ifndef CLI_INTERNALS_H
#define CLI_INTERNALS_H


#include "Tfa98xx.h"

#define CMDLINE_MAXLEN  256
#define CMDNAME_MAXLEN  32

int ReportIfError(Tfa98xx_Error_t err);
int OpenSession(unsigned char slaveAddr);
void CloseSession(void);
void ExecuteCommand(const char* pCmdStr, int* keepParsing, int* errorInExecution);
int GetCmdIdx(const char* pCmdName);
void PrintCommandList(void);
Tfa98xx_Error_t patch(Tfa98xx_handle_t handle, const char* fileName);


typedef struct
{
  char name[CMDNAME_MAXLEN];
  char argNames[64];
} CommandInfo_t;

extern const CommandInfo_t cmdInfo[];

#endif
