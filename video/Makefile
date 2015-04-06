MCU_TARGET     = atmega328p
OPTIMIZE       = -O2

DEFS           =-c -I../include
LIBS           =

libvideo.a: video.o
	avr-ar rcs libvideo.a video.o
	cp libvideo.a ../lib

video.o: video.c ../include/video.h ../include/font.h
	$(CC) $(CFLAGS) video.c -o video.o

include ../avr.mk

EXTRA_CLEAN_FILES += libvideo.a ../lib/libvideo.a

all: libvideo.a

