#include <string.h>
#include "cli_internals.h"
#include <stdio.h>

const CommandInfo_t cmdInfo[]=
{
/*  "1234567890123456789012345678901",  "" */
  { ";",                                "" },
  { "h",                                "" },
  { "q",                                "" },
  { "Powerdown",                        "powerdown(d)" },
  { "WriteRegister16",                  "subaddress(x8) value(x16)" },
  { "ReadRegister16",                   "subaddress(x8)" },
  { "DspPatch",                         "filename(s)" },
  { "SetConfigured",                    "" },
  { "SetVolume",                        "voldB(f)" }
};

int GetCmdIdx(const char* pCmdName)
{
  int i;

  /* retrieve LUT index */
  for (i=0; i<(sizeof(cmdInfo)/sizeof(CommandInfo_t)); i++)
  {
    if (0==strcmp(pCmdName, cmdInfo[i].name))
    {
      break;
    }
  }
  return i;
}

void PrintCommandList(void)
{
  int i;

  fprintf(stdout, "\nHelp : available commands\n\n");
  fprintf(stdout, "  listed as e.g. commandName parameter1(format1) parameter2(format2) ...\n");
  fprintf(stdout, "  to be typed as commandName value1 value2 ...\n\n");
  for (i=3; i<(sizeof(cmdInfo)/sizeof(CommandInfo_t)); i++)
  {
    fprintf(stdout, "%32s %s\n", cmdInfo[i].name, cmdInfo[i].argNames);
  }
  fprintf(stdout, "\n");
}


#if 0 /* Command Line Interface functionlist */
 Powerdown(Tfa98xx_handle_t handle, int powerdown);
 SetConfigured(Tfa98xx_handle_t handle);
SelectAmplifierInput(Tfa98xx_handle_t handle, Tfa98xx_AmpInputSel_t input_sel);
SelectI2SOutputLeft(Tfa98xx_handle_t handle, Tfa98xx_OutputSel_t output_sel);
SelectI2SOutputRight(Tfa98xx_handle_t handle, Tfa98xx_OutputSel_t output_sel);
 SetVolume(Tfa98xx_handle_t handle, float voldB);
DspSetVolumeParameters(Tfa98xx_handle_t handle, int volume_index);
GetVolume(Tfa98xx_handle_t handle, float* pVoldB);
SetSampleRate(Tfa98xx_handle_t handle, int rate);
GetSampleRate(Tfa98xx_handle_t handle, int* rate);
SelectChannel(Tfa98xx_handle_t handle, Tfa98xx_Channel_t channel);
SetMute(Tfa98xx_handle_t handle, Tfa98xx_Mute_t mute);
GetMute(Tfa98xx_handle_t handle, Tfa98xx_Mute_t* pMute);

 DspPatch(Tfa98xx_handle_t handle, int patchLength, const unsigned char* patchBytes);
DspConfig(Tfa98xx_handle_t handle, const Tfa98xx_Config_t pParameters);

DspSelectSpeaker(Tfa98xx_handle_t handle, Tfa98xx_SpeakerType_e speaker);

DspWriteSpeakerParameters(Tfa98xx_handle_t handle, const Tfa98xx_SpeakerParameters_t pParameters);
DspReadSpeakerParameters(Tfa98xx_handle_t handle, Tfa98xx_SpeakerParameters_t pParameters);
DspWritePreset(Tfa98xx_handle_t handle, int presetLength, const char* presetBytes);

DspBiquad_SetCoeff(Tfa98xx_handle_t handle, int biquad_index, float b0, float b1, float b2, float a1, float a2);
DspBiquad_SetEQ(Tfa98xx_handle_t handle, int biquad_index, float b0, float b1, float b2, float a1, float a2);
DspBiquad_Disable(Tfa98xx_handle_t handle, int biquad_index);

 ReadRegister16(Tfa98xx_handle_t handle, unsigned char subaddress, unsigned short *value);
 WriteRegister16(Tfa98xx_handle_t handle, unsigned char subaddress, unsigned short value);
WriteData(Tfa98xx_handle_t handle, unsigned char subaddress, int num_bytes, const unsigned char data[]);
ReadData(Tfa98xx_handle_t handle, unsigned char subaddress, int num_bytes, unsigned char data[]);
DspSetParam(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, const unsigned char data[]);
DspGetParam(Tfa98xx_handle_t handle, unsigned char module_id, unsigned char param_id, int num_bytes, unsigned char data[]);
#endif

