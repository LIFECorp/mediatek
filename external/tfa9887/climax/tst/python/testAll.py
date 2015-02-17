#!/usr/bin/python
#
import commands,sys, os,  shlex
from time import sleep

saveout = sys.stdout
cmdlogfile=open('cmds.sh', 'w')
logfile=open('cmds_output_log.txt', 'w')

TGT="dummy"
CLIMAX="./climax -d"+TGT+" "
MAXTEST=12
PARMS="tester.parms"

CLIMAXP=CLIMAX+"-l %s -s %s "%(PARMS,PARMS) # climax with parms save/load

def setTarget(target="dummy",extra=""):  # TODO Make a class
    global TGT
    global CLIMAX
    global CLIMAXP
    TGT=target   
    CLIMAX="./climax -d"+TGT+extra
    CLIMAXP=CLIMAX+" -l %s -s %s "%(PARMS,PARMS) # climax with parms save/load
     
###############################################################################
#
def testMakeParms():
    """create a binary params file """
    
    cmd("rm -rf %s"%(PARMS)) # create empty
    cmd("touch %s"%(PARMS)) # create empty
    cmd(CLIMAXP+"-p settings/Patch_N1D2_1_5_7_15_16_18_21_22_23_Delay2.patch")  # exits on failure
    cmd(CLIMAXP+"-p settings/Setup87.config")
    cmd(CLIMAXP+"-p settings/KS_13X18_DUMBO_tCoef.speaker") 
    # 3 profiles
    cmd(CLIMAXP+"-p settings/HQ_KS_13X18_DUMBO.preset  --profile=0")
    cmd(CLIMAXP+"-p settings/JP_Volume_2_KS_13X18_DUMBO.preset  --profile=1")
    cmd(CLIMAXP+"-p settings/Music_KS_13X18_DUMBO.preset  --profile=2")
       
    return True

###############################################################################
#
def parseCalibrationResponse(resp):
    """ input looks like:
         0 calibrate always
        1 written patch data to device
        2 written patch data to device
        3 tCoef = 0.02625
        4 written patch data to device
        5 written config data to device
        6 written speaker data to device
        7 written preset data to device
        8 Calibration value is 7.76 ohm
"""
    lines = resp.split('\n')
#    for l in lines:
#        print l
    if len(lines)!=9:
        return ("wrong input length %d"%(len(lines)),0,0,0) # error
    type = lines[0].split()[1] #

    for n in (1,2,4):
        if lines[n] != "written patch data to device":
            return ("wrong patch line[%d]:%s "%(n,lines[n]),0,0,0) # error
        
    try: tCoefA = float(lines[3].split()[2])
    except  ValueError or IndexError:
        return ("wrong input1",0,0,0) # error
    # except    
    try :R = float(lines[8].split()[3])
    except  ValueError or IndexError:
        return ("wrong input3",0,0,0) # error
   
    return ( type,0,tCoefA, R )

def parseSpecialCalibrationResponse(resp):
    """ input looks like:
         0     calibrate always
         1   written patch data to device
         2   written patch data to device
         3   tCoef = 0.00340
         4   still tCoef in speaker file, special calibration needed
         5   re25 = 0.000000
         6   written patch data to device
         7   written config data to device
         8   written speaker data to device
         9   written preset data to device
         10  tCoef = 0.003400
         11   Calibration value is 7.82 ohm @ 25 degress
         12  Final tCoefA 0.02657
         13  written patch data to device
         14  written patch data to device
         15   written patch data to device
         16   written config data to device
         17   written speaker data to device
         18   written preset data to device
         19   Calibration value is 7.80 ohm

    """
    lines = resp.split('\n')
#    for l in lines:
#        print l
    if len(lines)!=20:
        return ("wrong input length %d"%(len(lines)),0,0,0) # error
    type = lines[0].split()[1] #

    for n in (1,2,6,13,14,15):
        if lines[n] != "written patch data to device":
            return ("wrong patch line[%d]:%s "%(n,lines[n]),0,0,0) # error
        
    try: tCoef = float(lines[10].split()[2])
    except  ValueError or IndexError:
        return ("wrong input1",0,0,0) # error
    try: tCoefA = float(lines[12].split()[2])
    except  ValueError or IndexError:
        return ("wrong input2",0,0,0) # error
    # except    
    try :R = float(lines[19].split()[3])
    except  ValueError or IndexError:
        return ("wrong input3",0,0,0) # error
   
    return ( type, tCoef,tCoefA, R )

def testCalibrateAlwaystCoef():
    """testing calibrate always with tCoef"""
    
    return testCalibrate(tcoef='',target="/dev/ttyUSB0")
        

def testCalibrateAlwaystCoefA():
    """testing calibrate always with tCoefA in speakerfile"""

    return testCalibrate(tcoef='A', type='always', target="/dev/ttyUSB0") 

def testCalibrateOncetCoef():
    """testing calibrate once with tCoef"""
    
    return testCalibrate(type='once', tcoef='',target="/dev/ttyUSB0")
        

def testCalibrateOncetCoefA():
    """testing calibrate once with tCoefA in speakerfile"""

    return testCalibrate(tcoef='A', type='once', target="/dev/ttyUSB0") 

def checkCalibrate(resp, tcoef='A', type='always'):
    """check the calibration response output"""
    
    #check
    if  tcoef=='A':
        (respType, resptCoef,  resptCoefA, respR ) = parseCalibrationResponse(resp)
        if type != respType:
            print "testCalibrate %s failed, type=%s"%(type, respType)
            return False
        if resptCoefA < 0.01 or resptCoefA> 0.08:
            print "testCalibrate failed, tCoefA=%f"%(resptCoefA)
            return False
        if respR < 6 or respR > 10:
            print "testCalibrate failed, R=%f"%(respR)
            return False
    else:
        (respType, resptCoef,  resptCoefA, respR )= parseSpecialCalibrationResponse(resp)
        #check
        if type != respType:
            print "testCalibrate %s failed, type=%s"%(type, respType)
            return False
        if resptCoef < 0.0005 or resptCoef> 0.01:
            print "testCalibrate failed, tCoef=%f"%(resptCoef)
            return False
        if resptCoefA < 0.005 or resptCoefA > 0.1:
            print "testCalibrate failed, tCoefA=%f"%(resptCoefA)
            return False
        if respR < 6 or respR > 10:
            print "testCalibrate failed, R=%f"%(respR)
            return False       
   
    return True

def testCalibrate(tcoef='A', type='always',target="dummy"):
    """generic calibration test"""
    
    setTarget(target)  #

    if  tcoef=='A':
        cmd(CLIMAXP+"--tcoef=0.0272") # set to tCoefA
    else:
        cmd(CLIMAXP+"--tcoef=0.0034") # set to tCoef
        
    if type=='always':
        resp = cmd(CLIMAXP+"--calibrate=always")
    else:
        resp = cmd(CLIMAXP+"--calibrate=once")
            
    return checkCalibrate(resp, tcoef, type)  
        
        
###############################################################################
#
def testReset():
    """reset the hardware to coldstart"""
    cmd(CLIMAX+" -R")  # coldboot
    cmd(CLIMAX+" -r9 -w2")  # reset i2c regs
    
    return True

###############################################################################
#
def testDiag():
    """run all diagnostic test=0"""
    cmd(CLIMAX+"--diag")  # exits on failure
    
    return True
###############################################################################
#
def testDiags():
    """reset and run all diagnostic tests independently (pass)"""
    cmd(CLIMAX+"-r9 -w2") #reset i2c regs
    for testnr in range(1,MAXTEST+1):
        cmdline="%s --diag=%d"%(CLIMAX,testnr)
        cmd(cmdline)  # exits on failure
    
    return True
###############################################################################
#
def testDiagsFail():
    """reset and run all diagnostic tests independently (fail)"""
    if TGT!="dummy":
        print  "only works for dummy target"
        return False
    cmd(CLIMAX+"-r9 -w2") #reset i2c regs
    for testnr in range(1,MAXTEST+1):
        cmdline="%s --diag=%d %d"%(CLIMAX,testnr,testnr)
        cmd(cmdline, fail=256, withstat=False)  # exits on failure
    
    return True

###############################################################################
#
def testX():
    """ """
    cmd("") # exits on failure
    
    return True


##################################################
#
#  common tests
#

##################################################
#
def check_framework():
    """Check if this test has executed"""

    return True

##################################################
#
def check_env():
    """Check env parameters"""
    try: 
        if os.environ['TOP']: 
            return True
    except KeyError:
        print "\n>>>> Please point TOP to the workspace topdir <<<<"
    return False
    

##################################################
#
def check_serialDevice():
    """Check and select the serial device (/dev/ttyUSB0)"""  
    
    cmd('ls /dev/ttyUSB0') # exits if fail
    setTarget("/dev/ttyUSB0")
    return True

##################################################
#
def testBuild():
    """clean rebuild and check executable"""
   # cmd("scons -c")
    cmd("scons")
    cmd("ls climax")
    
    return True

##################################################
# the test framework from here
##################################################
#
def main():
    global saveout, logfile
    saveout = sys.stdout  # save stdout, in case we redirect
    #logfile=open('cmds.log', 'w')
    logfile.write("Starting %s %s\n"%(os.path.basename(sys.argv[0]), commands.getoutput('date')) )
    # define the following list to run tests in that order
    sys_tests= (check_framework,
                   testBuild,
                   testDiag,   # with dummy
                   testDiags,
                   testDiagsFail,
                   check_serialDevice,
                   testMakeParms,
                   testCalibrateOncetCoef,
                   testCalibrateOncetCoefA,
                   testCalibrateAlwaystCoef,
                   testCalibrateAlwaystCoefA,
                   testReset,
                   testDiag,  
                   testDiags
            
                )
    
    tests_to_run=sys_tests #+builds
    
    if len(sys.argv)>1:
        show(tests_to_run)
            
    print "------------- Running regression tests (%s) --------------"%os.path.basename(sys.argv[0])

    for test in tests_to_run:
        do_test(test)
    
    sys.exit("\n %s Passed...\n"%os.path.basename(sys.argv[0]))

    return


##################################################
#
def do_test(function):
    sys.stdout.write(" -%s (%s): "%( function.func_doc, function.func_name))
    sys.stdout.flush()
    if function(): 
        print 'Ok'
    else:
        sys.exit("\n %s: %s() FAILED!!!!"%(os.path.basename(sys.argv[0]),function.func_name))

##################################################
#
def cmd(cmdline,  verbose=False,  dryrun=False,  withstat=False, fail=0):
    """Run commandline, stop on failure else return response"""
    global logfile
    global cmdlogfile
    cmdlogfile.write("%s\n"%cmdline)
    logfile.write("cmd: %s\n"%cmdline)
    if verbose: print cmdline
    if dryrun==True: 
        print cmdline
        return True
    (stat,resp)=commands.getstatusoutput(cmdline) #execute cmd
    logfile.write("%s\n"%resp)
    if stat != fail: 
       sys.stderr.write( '\n\ncommandline FAILED: '+cmdline+'\nResponse: '+resp+'\n') 
       sys.exit("\n %s FAILED!!!"%os.path.basename(sys.argv[0]))
    if withstat:
        return stat
    return resp
##################################################
#


def show(tests_to_run):
         #just show which tests will be excuted
        print "The following tests will be executed by %s:"%os.path.basename(sys.argv[0])
        i=1
        for test in tests_to_run:
            print "%d=%s \t\t: %s "%(i,  test.func_name,  test.func_doc)
            i=i+1
        sys.exit()   

##################################################
##################################################

if __name__ == '__main__':
  main()

