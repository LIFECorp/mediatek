#!/usr/bin/python
'''
Created on Apr 28, 2012

@author: Wim
'''

from scanf import sscanf,IncompleteCaptureError

# idx=bit15
bitlines=[
  '+------------------', #bit=15
  '|+-----------------',
  '||+----------------',
  '|||+---------------',
  '|||| +-------------',
  '|||| |+------------',
  '|||| ||+-----------',
  '|||| |||+----------',
  '|||| |||| +--------',
  '|||| |||| |+-------',
  '|||| |||| ||+------',
  '|||| |||| |||+-----',
  '|||| |||| |||| +---',
  '|||| |||| |||| |+--',
  '|||| |||| |||| ||+-',
  '|||| |||| |||| |||+', #bit=0
  '|||| |||| |||| ||||'] #start =[16]

def print16bits(val):
    ''' print as 4 bit nibbles '''
    bitstring=''
    nibble=[]
    for i in range(4):
        nibble.append((val>>i*4) & 0x0f)
    for i in range(3,-1,-1):
        for b in range(3,-1,-1):
            bit=(nibble[i]>>b) & 1
            bitstring += ("%d"%(bit)) # sys.stdout.write("%d"%(bit))
        bitstring += (" ") #sys.stdout.write(" ")
    return bitstring

def skipUntil(file, stopstring):
    ''' read the line until the stopstring is read '''
    while line in file:
        if line._contains__(stopstring): break
        
class bitgroup():
    def __init__(self,start,end, name,string=''):
        self.start=start
        self.end=end
        self.name=name
        self.statename={}
        msk= (1<<(end-start+1))-1
        self.mask= msk<<start
    def setName(self, state, name):
        self.statename.update({state:name})
    
        
class regbit():
    def __init__(self,pos,name,string=''):
        self.pos=pos
        self.name=name
        self.string=string
    
class register:
    def __init__(self, addr, name,string=''):
        self.addr=addr
        self.name=name
        self.string=string
        self.bits={}
        self.bitgroups={}

class device:
    '''
    classdocs
    '''

    def __init__(self, device,slave=0x34,string=''):
        '''
        Constructor
        '''
        self.device=device
        self.slave=slave
        self.registers={}
    def registerName(self, address, name):
        r=register(address,name)
        self.registers.update({address:r})
        
    def registerBits(self, regaddress, f):
        ''' get the bits from the opened file until End Bits'''
        while 1:
         try:
            line=f.next()
            items = line.split()
            if items[0] == 'End': break #any end will stop
            # 0 hid_code_0 "...string with spaces..." 
            (bits, string, x)= line.split('"')
            pos=int(bits.split()[0],16)
            name=bits.split()[1]
            b=regbit(pos,name,string)
            self.registers[regaddress].bits.update({pos:b})
         except StopIteration:
            pass
            
    def registerBitGroups(self, regaddress, f):
        ''' get the bitgroups from the opened file until End Bitgroups'''
        while 1:
         try:
            line=f.next()
            items = line.split()
            if items[0] == 'End': 
                if items[1] == 'Bitgroups' : break #End Bitgroups will stop
            #
            #sel_i2so_l 0:2 "Output selection dataout left channel" 
            #  0 "CurrentSense signal"
            #  ....
            #End 
            (bits, string, x)= line.split('"')
            field=bits.split()[1]
            (start,end)=field.split(':')
            name=bits.split()[0]
            grp=bitgroup(int(start),int(end),name,string)
            # the state values
            while 1:
                try : 
                    line=f.next()
                    if line.__contains__('End'): break
                    (state,name,x)=line.split('"')
                    grp.setName( int(state), name)
                except StopIteration:
                    pass
            self.registers[regaddress].bitgroups.update({int(start):grp})
         except StopIteration:
            print 'reg done'
    def FullRegPrint(self, address, value):
        ''' print a full register layout '''
        # only for 16 bitters
        if len( self.registers[address].bitgroups) == 16:
            # first print all bitlines
            for b in range(15,-1,-1): 
                state=(dev.registers[reg].bitgroups[b].mask & data)>>dev.registers[reg].bitgroups[b].start
                bitname = b,dev.registers[reg].bitgroups[b].statename[state]
                print bitlines[15-b],"%2d"%b,state,bitname[1]
            print bitlines[16]
        #    
        print print16bits(value),"0x%02x=0x%04x %s"%(address, value, self.registers[address].name)  
        
    def parseRegs(self, filename):
        ''' find registers in file and add to device'''
        f = open(filename)
        while 1:
         try:
            items = f.next().split()
            if items[0] == 'Register':
                #Register 0x0A:2 I2S_sel_reg " "
                regaddr = items[1].split(':')[0] # hex string
                regaddr =int(regaddr, 16)
                regname = items[2]
                #if regname.upper().__contains__('HIDDEN'): 
                #    skipUntil(f, 'End Register')
                self.registerName(address=regaddr, name=regname)
            if items[0] == 'Bits':
                self.registerBits(regaddr, f)
            if items[0] == 'Bitgroups':
                self.registerBitGroups(regaddr, f)
            if items[0] == 'End':
                    if items[1] == 'Registers': break
         except StopIteration:
            pass
        f.close()

def getDevice(filename):
        f = open(filename)
        dev=None
        try:
           while 1:   
            items = f.next().split()
            if items[0] == 'Device':
                #Device 0x34 TFA9887 "TFA9887 N1D"
                dev = device(device=items[2], slave=items[1], string=items[3])
                while items[0] + items[1] != 'EndDevice':
                    items = f.next().split()
                    if items[0] == 'Revision': 
                        dev.rev = items[1]
        except StopIteration:
            pass
        f.close()
        return dev
        
def t():
    dev = getDevice("tfa.def")
    dev.parseRegs("tfa.def")
    return dev

def incfile(dev):
    for r in dev.registers.keys():
        if  not dev.registers[r].name.upper().__contains__('PROTECTED') and not dev.registers[r].name.upper().__contains__('HIDDEN'):
            print "#define TFA9887_%s (0x%02x)"%(dev.registers[r].name.upper(),r)
    
def parseReg(inwords):
    ''' see if this is something like 0x12 : 0x234 esle return none'''
    try:
        (reg,data)=sscanf(inwords,"%x:%x")
    except IncompleteCaptureError:
        reg=data=None
    return (reg,data)
    
if __name__ == '__main__':
    import sys
    loop =True
    if len(sys.argv) > 1:
        dev = getDevice(sys.argv[1])
        dev.parseRegs(sys.argv[1])
    if len(sys.argv) > 2:
        if sys.argv[2]=='-': 
            fin=sys.stdin
        elif sys.argv[2].__contains__(':'):
            loop = False
            (reg, data) = parseReg(sys.argv[2])
            dev.FullRegPrint(reg, data) 
        else: fin=open(sys.argv[2])
    #incfile(dev)

    while loop:
        l=fin.readline().replace(' ','') # no spaces
        if not l: break
#        (reg,data)=l.split(':')
        (reg, data) = parseReg(l)    
        if reg != None:
            dev.FullRegPrint(reg, data)    
       # reg=int(reg)
       # data=int(data,16)
#        print "0x%02x=%s"%(reg, dev.registers[reg].name )
#        grps=dev.registers[reg].bitgroups.keys()
##        for b in grps: 
#        for b in range(15,-1,-1): 
#        #print b, dev.registers[reg].bitgroups[b].name,hex(dev.registers[reg].bitgroups[b].mask)
#            state=(dev.registers[reg].bitgroups[b].mask & data)>>dev.registers[reg].bitgroups[b].start
#            print  b,dev.registers[reg].bitgroups[b].statename[state]
#    for r in dev.registers.keys():
#        print r,dev.registers[r].name
#        print "bits"
#        for b in dev.registers[r].bits.keys():
#            print b, dev.registers[r].bits[b].name
#        print "groups"
#        for b in dev.registers[r].bitgroups.keys():
#            print b, dev.registers[r].bitgroups[b].name
            
      
    
    
