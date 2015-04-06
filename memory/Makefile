MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           =-c -I../include
LIBS           =

libmem.a: mem.o
	avr-ar rcs libmem.a mem.o
	cp libmem.a ../lib

mem.o: mem.c ../include/memory.h
	$(CC) $(CFLAGS) mem.c -o mem.o

include ../avr.mk

EXTRA_CLEAN_FILES += libmem.a ../lib/libmem.a

all: libmem.a

