BINS=hello.bin hello_name.bin loads.bin \
	 hello.c.bin map.c.bin ball.c.bin vidmem.c.bin \
	 hello_printf.c.bin \
	 adventure.c.bin yum.c.bin \
	 mandelbrot.c.bin

all: $(BINS)

wav: $(BINS:.bin=.wav)

%.wav: %.bin ../../storage/tools/encode
	../../storage/tools/encode $< -o $@

../storage/tools/encode:
	make -C ../../storage/tools

%.c.s: %.c
	cc65 -t none -Or -O2 -I ../include $< -o $@

%.o: %.s
	ca65 $< -o $@

%.bin: %.o
	ld65 -C ../../d64.cfg -L ../lib $< -o $@ --lib d64.lib

clean:
	rm -f *.o *.bin *.c.s *.wav

