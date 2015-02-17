#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cli_internals.h"
#include "readline.h"


static void InteractiveModeMessage(void)
{
  fprintf(stdout, "Interactive Command Line Interface (CLI) is started\n");
  fprintf(stdout, "'q'=quit, 'h'=help\n");
}


int main(int argc, char* argv[])
{
  int parseCmd, parseScript, abnormalEnd;
  FILE* cmdFile = stdin;

  switch (argc)
  {
  case 2:
    /* interactive cmd console */
    parseCmd = 1;
    parseScript = 0;
    break;
  case 3:
    /* execute cmd script */
    parseScript = 1;
    cmdFile = fopen(argv[2], "rb");
    parseCmd = cmdFile != NULL;
    if (!parseCmd)
    {
      fprintf(stderr, "Unable to open %s for input\n", argv[2]);
    }
    break;
  default:
    fprintf(stderr, "NXP TFA9887 Control Program, build: %s, %s\n", __DATE__, __TIME__);
    fprintf(stderr, "Syntax is\n");
    fprintf(stderr, "(console) %s slaveAddr\n", argv[0]);
    fprintf(stderr, "(script)  %s slaveAddr scriptname\n", argv[0]);
    parseCmd = 0;
    parseScript = 0;
  }

  if (parseCmd)
  {
    int slaveAddr;

    sscanf(argv[1], "%x", &slaveAddr);
    abnormalEnd = OpenSession((unsigned char)slaveAddr);
    if (abnormalEnd)
    {
      fprintf(stderr, "Unable to open session\n");
    }
    else
    {
      char cmdStr[CMDLINE_MAXLEN];
      int nCharsRead, lineNumber;

      lineNumber = 0;
      if (!parseScript) InteractiveModeMessage();
      while (parseCmd)
      {
        /* fetch a command string */
        if (parseScript)
        {
          if (feof(cmdFile))
          {
            fclose(cmdFile);
            cmdFile = stdin;
            parseScript = 0;
            InteractiveModeMessage();
          }
        }
        nCharsRead = readline(cmdFile, cmdStr, CMDLINE_MAXLEN-1, &lineNumber);
        /* execute the command and update 'parseCmd', 'abnormalEnd' if necessary */
        if (nCharsRead>0)
        {
          ExecuteCommand(cmdStr, &parseCmd, &abnormalEnd);
          if (abnormalEnd)
          {
            if (cmdFile!=stdin)
            {
              fprintf(stderr, "%s, line %d\n", argv[2], lineNumber);
              parseCmd = 0; /* when executing script, cmd errors cause program exit */
            }
          }
        }
      }
      if (parseScript) fclose(cmdFile);
      CloseSession();
    }
  }
  else
  {
    abnormalEnd = 1;
  }

	return abnormalEnd;
}
