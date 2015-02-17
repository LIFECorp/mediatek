
import Tfa9887
from time import sleep


error = Tfa9887.SetSlaveAddress(0x68)
print "SetSlaveAddress ->", error
if not error.isOk():
    raise "Failed to set slave address"

error = Tfa9887.Powerdown(False)
print "Power up ->", error
if not error.isOk():
    raise "Failed to power up the IC"

error = Tfa9887.Configured(True)
print "Configured ->", error
if not error.isOk():
    raise "Failed to Configure the IC"

error = Tfa9887.SelectAmpInput(Tfa9887.InputSel.DSP)
print "SelectAmpInput ->", error
if not error.isOk():
    raise "Failed to set amp input to DSP"

print "Downloading SW to DSP..."
error = Tfa9887.Demo_DownloadCF(r"..\..\Apps\Host\Maximus\App_Demo_Maximus_release.cf6.PMEM",
                                r"..\..\Apps\Host\Maximus\App_Demo_Maximus_release.cf6.XMEM",
                                r"..\..\Apps\Host\Maximus\App_Demo_Maximus_release.cf6.YMEM")
print "DownloadCF ->", error
if not error.isOk():
    raise "Failed to download SW to DSP"

error, preset = Tfa9887.GetPreset()
print "GetPreset ->", error
if not error.isOk():
    raise "Failed to get preset from DSP"
print preset

error = Tfa9887.SetVolume(0)
print "SetVolume ->", error
if not error.isOk():
    raise "Failed to set volume on DSP"


for i in range(100):
    error, stateInfo = Tfa9887.GetStateInfo()
    print "GetStateInfo ->", error
    if not error.isOk():
        raise "Failed to get state info from DSP"
    print stateInfo
    sleep(0.2)


