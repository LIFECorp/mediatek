SCRIBO=../../Scribo/
	
# source files.

SRC =  init/tfainit.c

OBJ = $(SRC:.c=.o)

OUT = tfainit

# include directories
CPPFLAGS = -I. -I../inc 

# C++ compiler flags (-g -O2 -Wall)
CFLAGS = -g -O0 -Wall
#CPPFLAGS+=$(CCFLAGS)

# compiler
CC = gcc

# library paths
LIBS = -L../../Tfa9887_hal -L../ -L$(SCRIBO)/lxScribo

# compile flags
LDFLAGS = -g

default: $(OUT) 

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $<  -o $@   $(LIBS) -ltfa9887  -ltfa9887_hal -lm -llxScribo

	
clean:
	rm -f $(OBJ) $(OUT) 
