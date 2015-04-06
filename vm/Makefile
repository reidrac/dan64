MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           =-c -I../include -Wno-sequence-point 
LIBS           =

libvm.a: vm.o
	avr-ar rcs libvm.a vm.o
	cp libvm.a ../lib

vm.o: vm.c ../include/vm.h
	$(CC) $(CFLAGS) vm.c -o vm.o

include ../avr.mk

EXTRA_CLEAN_FILES += libvm.a ../lib/libvm.a

all: libvm.a

