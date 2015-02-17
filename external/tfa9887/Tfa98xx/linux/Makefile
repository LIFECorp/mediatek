
HAL=../../Tfa98xx_hal
	
# source files.
VPATH=../src
LIB_SRC =  ../src/Tfa98xx.c ../src/Tfa98xx_TextSupport.c  ../src/Tfa9887.c ../src/initTfa9887.c 

LIB_OBJ = $(LIB_SRC:.c=.o)

LIB = ../libtfa98xx.a $(HAL)/libtfa98xx_hal.a

# include directories
CPPFLAGS = -I. -I../inc -I$(HAL)/inc

# C++ compiler flags (-g -O2 -Wall)
CFLAGS = -g -O0 -Wall

# compiler
CC = gcc

# library paths
LIBS = -lm

# compile flags
LDFLAGS = -g

default: exTfa9887Stereo exTfa9887 exTfa98xx 
#$(LIB)

$(LIB): $(LIB_OBJ)
	ar rcs $(LIB) $(LIB_OBJ)
	ranlib $(LIB)
		
exTfa9887: ../ex/exTfa9887/src/main.o $(LIB) 
	$(CC) $^ $(LIBS) -o $@

exTfa9887Stereo: ../ex/exTfa9887Stereo/src/main.o $(LIB) 
	$(CC) $^ $(LIBS) -o $@
	
exTfa98xx: ../ex/exTfa98xx/src/main.o $(LIB) 
	$(CC) $^ $(LIBS) -o $@

	
clean:
	rm -f $(LIB_OBJ) $(LIB) Makefile.bak 
