
# TODO: a better way to do this ?
# I looked at SWIG, SIP and Boost.Python, but none seem to handle all automatically the conversion from .h -> python
# (especially for the more complex functions with takes structs etc as parameters)
# They generate complex wrapper code that is hard to debug
# so do manually now with ctypes and deliver a nice python-like API 

import os
from ctypes import CDLL, windll, util, byref, create_string_buffer, c_float, c_int, c_uint8, Structure
from numpy import fft, multiply, add
from math import pi
from cmath import sqrt # complex number version

# add the location of the dlls to the system path
os.environ['PATH'] += r";..\..\..\Apps\Host\Maximus"

# need windll for this dll, which uses stdcall iso cdecl calling convention
dll = windll.LoadLibrary("crdi2c32")
dll.i2cDebug(1)

# other DLL uses cdecl, so can just use CDLL
#dll = CDLL("Tfa9887_hal")
dll = CDLL("Tfa9887")

class Error:
    Ok = 0
    DSP_not_running = 1
    Bad_Parameter = 2
    Not_Implemented = 3
    Not_Supported = 4
    I2C = 5
    Other = 1000
    
    def __init__(self, value):
        self.value = value
        
    def isOk(self):
        return self.value == self.Ok
    
    def __repr__(self):
        if self.value==self.Ok: return "ok"
        if self.value==self.DSP_not_running: return "DSP not running"
        if self.value==self.Bad_Parameter: return "Bad Parameter"
        if self.value==self.Not_Implemented: return "Not Implemented"
        if self.value==self.Not_Supported: return "Not Supported"
        if self.value==self.I2C: return "I2C error"
        raise "unknown error %d" % (self.value)
    
    
class InputSel:
    I2SLeft = 0
    I2SRight = 1
    DSP = 2

    def __init__(self, value):
        self.value = value
        
        
def dump_struct(struct, name):
    bFirst = True
    result = "%s<" %(name)
    for f in struct._fields_:
        if not bFirst: result += ","
        bFirst = False
        result += f[0] + ":"
        if f[1] == c_int: 
            result += "%d" % (struct.__getattribute__(f[0]))
        elif f[1] == c_float: 
            result += "%2.1f" % (struct.__getattribute__(f[0]))
        else:
            raise "unknown type"
    result += ">"
    return result

    
class Preset(Structure):
    _fields_ = [("sbOn", c_int),
                ("fLowCutOrder", c_int),
                ("fLowCutSbOff", c_float),
                ("fLowCutSbOn", c_float),
                ("BWExtOn", c_int),
                ("fBWExt", c_float),
                ("agcDownhalfTime", c_float),
                ("agcUpDoubleTime", c_float)
                ]
     
    def __repr__(self):
        #return "Preset<sbOn:%d, fLowCutOrder:%d, fLowCutSbOff:%3.1f, fLowCutSbOn:%3.1f, BWExtOn:%d, fBWExt:%3.1f, agcDownhalfTime:%3.1f, agcUpDoubleTime:%3.1f>" % (self.sbOn, self.fLowCutOrder,self.fLowCutSbOff,self.fLowCutSbOff,self.BWExtOn,self.fBWExt,self.agcDownhalfTime,self.agcUpDoubleTime)
        return dump_struct(self, "Preset")

class StateInfo(Structure):
    _fields_ = [("agcGain", c_float), # Current AGC Gain value
                ("limGain", c_float), # Current Limiter Gain value
                ("sMax", c_float),    # Current Clip/Lim threshold
                ("T", c_int),         # Current Temperature value
                ("acxtmFlag", c_int), # masked bit word containing: A(bit0)=Activity, C(bit1)=Clip Exceeds, X(bit2)=Excursion exceeds(X), T(bit3)=Temperature exceed, M(bit4)=Toggle flag to indicate new model estimate
                ("X1", c_float), # Current Excursion value  caused by Speakerboost gain control
                ("X2", c_float)  # Current Excursion value caused by manual gain setting
                ]
     
    def __repr__(self):
        return dump_struct(self, "StateInfo")

class LSModelRaw(Structure):
    _fields_ = [("parameters", c_uint8*450)]

    def __repr__(self):
        result = "LSModelRaw<"
        for i in range(449):
            result+= "%d," % (self.parameters[i])
        result+= "%d>" % (self.model[449])
        return result

class LSModel(Structure):
    _fields_ = [("model", c_float*128)]

    def __repr__(self):
        result = "LSModel<"
        for i in range(127):
            result+= "%2.1f," % (self.model[i])
        result+= "%2.1f>" % (self.model[127])
        return result

def SetSlaveAddress(slave_address):
    _Tfa9887_SetSlaveAddress = dll.Tfa9887_SetSlaveAddress
    _Tfa9887_SetSlaveAddress.restype = Error
    _Tfa9887_SetSlaveAddress.argtypes = [c_uint8]
    error = _Tfa9887_SetSlaveAddress(slave_address)
    return error

def SelectAmpInput(input_sel):
    _Tfa9887_SelectAmpInput = dll.Tfa9887_SelectAmpInput
    _Tfa9887_SelectAmpInput.restype = Error
    error = _Tfa9887_SelectAmpInput(input_sel)
    return error

def Powerdown(bPowerdown):
    _Tfa9887_Powerdown = dll.Tfa9887_Powerdown
    _Tfa9887_Powerdown.restype = Error
    error = _Tfa9887_Powerdown(bPowerdown)
    return error

def Configured(bConfigured):
    _Tfa9887_Configured = dll.Tfa9887_Configured
    _Tfa9887_Configured.restype = Error
    error = _Tfa9887_Configured(bConfigured)
    return error


def isDspRunning():
    # TODO: ask the tag ?
    pass
    
def Demo_EnableDsp(bEnable):
    # TODO: enbl_CoolFlux bit
    pass
    
def Demo_DownloadCF(pmem_file, xmem_file, ymem_file):
    _Tfa9887_Demo_DownloadCF = dll.Tfa9887_Demo_DownloadCF
    _Tfa9887_Demo_DownloadCF.restype = Error
    error = _Tfa9887_Demo_DownloadCF(pmem_file, xmem_file, ymem_file)
    return error

def GetPreset():
    _Tfa9887_GetPreset = dll.Tfa9887_GetPreset
    _Tfa9887_GetPreset.restype = Error
    preset = Preset()
    error = _Tfa9887_GetPreset(byref(preset))
    return [error, preset]

# set the volume in dB, only negative volumes allowed
def SetVolume(volume):
    _Tfa9887_SetVolume = dll.Tfa9887_SetVolume
    _Tfa9887_SetVolume.restype = Error
    _Tfa9887_SetVolume.argtypes = [c_float]
    error = _Tfa9887_SetVolume(volume)
    return error

def GetStateInfo():
    _Tfa9887_GetStateInfo = dll.Tfa9887_Demo_GetStateInfo
    _Tfa9887_GetStateInfo.restype = Error
    info = StateInfo()
    error = _Tfa9887_GetStateInfo(byref(info))
    return [error, info]

def ReadSpeakerParameters():
    _Tfa9887_ReadSpeakerParameters = dll.Tfa9887_ReadSpeakerParameters
    _Tfa9887_ReadSpeakerParameters.restype = Error
    modelInfoRaw = LSModelRaw()
    modelInfo = LSModel()
    error = _Tfa9887_ReadSpeakerParameters(byref(modelInfoRaw))
    
    Bl = 0.5740
    Re0 = 8
    
    # convert from bytes/fixed point -> floating point
    for i in range(128):
        value = (modelInfoRaw.parameters[3*i]<<16) + (modelInfoRaw.parameters[3*i+1]<<8) + (modelInfoRaw.parameters[3*i+2])
        if (modelInfoRaw.parameters[3*i] & 0x80): # sign bit was set
            value = - ((1<<24)-value); # 2's complement
        modelInfo.model[i] = value*1.0/(1<<23)
        #print value, modelInfo.model[i]
    wX = modelInfo.model
    hX = fft.fft(wX);
    nFft = 128; 
    nPoints = nFft / 2 + 1;
    fs = 12000;   # LS model is always at 12kHz
    multiplier = pi/(nPoints-1)
    wVec = [n*multiplier for n in range(nPoints)]
    fVec = multiply(wVec, fs / 2 / pi);
    wVecRadS = multiply(wVec, fs);
    sVec = multiply(sqrt( -1 ), wVecRadS);
    hX = hX[0:nPoints];
    BlMin = Bl / 0.82;
    hZ = multiply(hX, sVec)
    hZ = multiply(hZ, ( Re0 * BlMin ) / 1000);
    hZ = add(hZ, Re0)
    print hX
    print hZ
    
    return [error, fVec, hX, hZ] #modelInfo]
    

