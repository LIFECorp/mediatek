import sys

def wreg(l):
    reg=l.split()[0][:-1]
    data=l.split()[1]
    cmd="-r%s -w%s "%(reg,data)
    return cmd

f=open(sys.argv[1])

for line in f.readlines():
    sys.stdout.write (wreg(line))
    
sys.stdout.write("\n")
 
