MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           =-c -I../include 
LIBS           =

libkeyboard.a: keyboard.o
	avr-ar rcs libkeyboard.a keyboard.o
	cp libkeyboard.a ../lib

keyboard.o: keyboard.c ../include/keyboard.h ../include/keyb_map.h
	$(CC) $(CFLAGS) keyboard.c -o keyboard.o

include ../avr.mk

EXTRA_CLEAN_FILES += libkeyboard.a ../lib/libkeyboard.a 

all: libkeyboard.a

