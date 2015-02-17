SCRIBO=../../Scribo/
	
# source files.

SRC =  i2ccalls/i2ccalls.c

OBJ = $(SRC:.c=.o)

OUT = i2ccalls/i2ccalls

# include directories
CPPFLAGS = -I. -I../inc -I../../Scribo/lxScribo

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g

# compiler
CC = gcc

# library paths
LIBS = -L../ -L../../Scribo/lxScribo

# compile flags
LDFLAGS = -g

default: $(OUT) miniInit

$(OUT): $(OBJ)
	$(CC)  $(CFLAGS) $(CPPFLAGS) $<  -o $@   $(LIBS) -l tfa9887_hal -llxScribo
miniInit: init/miniInit.c
	$(CC)  $(CPPFLAGS) -g -O0 $<  -o $@   $(LIBS) -ltfa9887_hal -llxScribo
	
clean:
	rm -f $(OBJ) $(OUT) miniInit
