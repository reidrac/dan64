all: test

CFLAGS = -Wall -Wno-sequence-point -ggdb -I../../include

test: test.c ../vm.c ../../include/vm.h
	gcc $(CFLAGS) test.c ../vm.c -o test
	./test

clean:
	rm -f test

