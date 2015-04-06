all: encode

CFLAGS=-s -O3 -Wall -I../../include -L.

encode: encode.c libstorage.a ../../include/storage.h
	gcc $(CFLAGS) encode.c -lsndfile -lstorage -o encode

libstorage.a: ../storage.c ../../include/storage.h
	gcc $(CFLAGS) -c ../storage.c -o storage.o
	ar rcs libstorage.a storage.o

clean:
	rm -f encode *.wav *.raw *.bin *.a *.o

