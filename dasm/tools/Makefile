all: dasm

CFLAGS=-s -O3 -Wall -I../../include -L.

dasm: dasm.c libdasm.a ../../include/dasm.h
	gcc $(CFLAGS) dasm.c -ldasm -o dasm

libdasm.a: ../dasm.c ../../include/dasm.h
	gcc $(CFLAGS) -c ../dasm.c -o dasm.o
	ar rcs libdasm.a dasm.o

clean:
	rm -f dasm *.a *.o

