MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           =-c -I../include
LIBS           =

libdasm.a: dasm.o
	avr-ar rcs libdasm.a dasm.o
	cp libdasm.a ../lib

dasm.o: dasm.c ../include/dasm.h ../include/font.h
	$(CC) $(CFLAGS) dasm.c -o dasm.o

include ../avr.mk

EXTRA_CLEAN_FILES += libdasm.a ../lib/libdasm.a

all: libdasm.a

