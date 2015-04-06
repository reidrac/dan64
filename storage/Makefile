MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           =-c -I../include
LIBS           =

libstorage.a: storage.o
	avr-ar rcs libstorage.a storage.o
	cp libstorage.a ../lib

storage.o: storage.c ../include/storage.h ../include/font.h
	$(CC) $(CFLAGS) storage.c -o storage.o

include ../avr.mk

EXTRA_CLEAN_FILES += libstorage.a ../lib/libstorage.a

all: libstorage.a

