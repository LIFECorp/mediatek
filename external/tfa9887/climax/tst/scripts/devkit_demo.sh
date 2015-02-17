#!/bin/bash
#
# init script env
./climax -s devkit.parms	# create emtpy params file
alias climax='./climax -d/dev/ttyUSB0 -l devkit.parms -s devkit.parms'	# climax with intermediate saved params 

 # 1.	Present all loadable parameters by calling the Store() functions:
 #	a.	DSP patch, setup & config
climax --params settings/Patch_N1D2_1_5_7_9_11_12_13_Delay.patch 
climax --params settings/Setup.config 
climax --params settings/KS_13X18_DUMBO.speaker
 #	b.	A volumesteps preset file for each profile
climax --profile 0 --params settings/JP_Volume_2_KS_13X18_DUMBO.preset
climax --profile 1 --params settings/Music_KS_13X18_DUMBO.preset
climax --profile 2 --params settings/HQ_KS_13X18_DUMBO.preset
 # 2.	 Notify that the clock has been applied via ClockEnable()
#sleep 2
climax --clock 1 ;sleep 2
 #	a.	The lower level will detect the cold poweron state and loads all parameters and profiles
 #	b.	DSP is started
 # 3.	While audio is active the SetVolume() can be called to set a profile volume step or mute.
climax --profile=1 --volume=7  --verbose;sleep 2
climax --profile=1 --volume=0  --verbose;sleep 2
#ure
climax --profile=1 --volume=16  --verbose;sleep 2
climax --profile=1 --volume=7  --verbose;sleep 2

 # 4.	If audio becomes inactive and the hardware will go down notify by ClockDisable()
climax --clock 0 ;sleep 2
 #	a.	DSP + AMP are shutdown gracefully (muted)
 # 5.	Audio is requested again, notify via ClockEnable()
climax --clock 1
 #	a.	If the TFA9887 was already loaded then the system will only be unmuted


