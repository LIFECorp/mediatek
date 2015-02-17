#
# simple python script to remove registers from the generated def file
#
import sys

validreg=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0c,
                        0x48, 0x49, 0x80]

        
def stripHidden(filename):
        ''' echo all lines except the hidden regs '''
        f = open(filename)
        quiet=False
        while 1:
                line = f.readline()
                if not line:
                    return
                if line.__contains__ (' Register ' ):
                        items=line.split()
                        if len(items)<3 :
                            if not quiet:  sys.stdout.write(line)
                            continue
                        regaddr = items[1].split(':')[0] # hex string
                        regaddr =int(regaddr, 16)
                        regname = items[2]
                        if regaddr not in validreg: 
                            quiet=True
                        if not quiet: sys.stderr.write("keep 0x%02x:%s\n"%(regaddr, regname))
                        else:   sys.stderr.write("hide 0x%02x:%s\n"%(regaddr, regname))
                if quiet and line.__contains__('End Register'): 
                                quiet=False
                                continue              
                if not quiet: sys.stdout.write(line)

        
if __name__ == '__main__':    
    if len(sys.argv) ==1:
        deffile = 'tfa9887n1d2.def'        
    else:
        dev = sys.argv[1]
    stripHidden(deffile)
    
