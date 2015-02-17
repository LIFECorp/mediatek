
# source files.
VPATH=../src ../src/lxScribo
SRC =  ../src/NXP_I2C_linux.c \
	../src/lxScribo/lxScribo.c \
	../src/lxScribo/lxScriboSerial.c \
	../src/lxScribo/lxScriboSocket.c

OBJ = $(SRC:.c=.o)

OUT = ../libtfa98xx_hal.a

# include directories
CPPFLAGS = -I../src -I../inc -I../src/lxScribo 

# C++ compiler flags (-g -O2 -Wall)
CFLAGS = -g -O2 -Wall

# compiler
CC = gcc

# library paths
LIBS =

# compile flags
LDFLAGS = -g

default: $(OUT)

$(OUT): $(OBJ)
	$(AR) rcs $(OUT) $(OBJ)
	ranlib $(OUT)
	
clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 
