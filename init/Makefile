PRG            = main
OBJ            = main.o syscall.o

MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           = -I../include
LIBS           = -L../lib -lvideo -lkeyboard -lmem -lvm -lstorage -ldasm

# upload with avrdude
#
PORT           = /dev/ttyACM0
PROGRAMMER     = arduino

include ../avr.mk

main.o: ../lib/libvideo.a ../lib/libkeyboard.a ../lib/libmem.a ../lib/libvm.a ../lib/libstorage.a ../lib/libdasm.a strings.h init.h
syscall.o: syscall.c strings.h

