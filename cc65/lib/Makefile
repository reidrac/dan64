all: d64.lib

CC65PATH=../cc65

OBJS=crt0.o system.o ctype.o oserror.o

rt:
	rm -f rt.lib
	./build-rt.sh $(CC65PATH)

d64.lib: $(OBJS)
	cp rt.lib d64.lib
	ar65 a d64.lib $(OBJS)

%.o: %.s
	ca65 $< -o $@

clean:
	rm -f *.o *.c.s d64.lib

