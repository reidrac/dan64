# DAN64: an AVR based 8-bit Microcomputer

![DAN64](images/dan64-logo.png "DAN64 Logo")\

This is my first _serious_ attempt to learn electronics. DAN64 is my first
project and it has been a discovery process during 3 months of my free time. I
had to learn a lot of things I didn't know much about, from basic electronics
to the details of the AVRs -and specifically the ATmega328-, and a whole world of
things in between such as signalling, protocols, interfaces, modulation and
demodulation, SDKs, EDA software, prototyping, PCB fabrication, etc.

I'm certain that in this project I'm doing many stupid things and I'm sure my
approach to solving some of the problems is not the best, but in my discharge I can
only say: _it works!_ (to some extent at least).

I got lots of _gotcha!_ moments, ups and downs where I though I couldn't finish
the project because perhaps what I was trying to achieve was just impossible.

So this is not about perfection but about good enough _for me_ and about the
learning process and having fun.

## Objectives

The term "8-bit microcomputer" might not be clear enough. What I meant with it
is that I wanted to build a computer like the ones I grew up in the 80s, the
[ZX Spectrum](https://en.wikipedia.org/wiki/ZX_Spectrum), the [Commodore
64](https://en.wikipedia.org/wiki/Commodore_64) or the [Amstrad
CPC](https://en.wikipedia.org/wiki/Amstrad_CPC).

It will be a single board computer capable of input using a keyboard, output to
a screen, and able to load external programs and run them.  It should be
possible to use the computer to write new programs that could be saved to be
loaded and run at a later time.

So basically: an 8-bit microcomputer.

## Target platform

I've used the [Arduino Uno R3](http://arduino.cc/en/main/arduinoBoardUno) as
development board so the target platform for the microcomputer is the
[ATmega328](http://www.atmel.com/devices/atmega328.aspx), a popular 8-bit
microcontroller in the AVR family manufactured by Atmel.

The AVR family implements a modified [Harvard
architecture](https://en.wikipedia.org/wiki/Harvard_architecture). The main
characteristic is that code and data are accessed using different buses. The
microcontroller (MCU from now on) includes different types of purpose specific
memory:

 - Flash: contains the executable code and the MCU can't execute code that is
   not in this type of non-volatile memory. AVRs use a "modified" Harvard
   architecture and that means that the code can be read as it was data (with
   some restrictions).
 - SRAM: volatile static memory.
 - EEPROM: read only memory that can be overwritten under some special conditions.

So my requirement of "load and run programs" is not trivial to achieve in a
Harvard architecture because the flash memory is not easy to rewrite, so I
explored two options based on the idea that I could load the programs as data
and make the native code in the flash "interpret" that data:

 - Program interpreter: Basic or Forth are interpreters that were implemented in
   the microcomputers of the 80s.
 - Virtual machine: I looked at "simple" RISC designs with a cross compiler that
   I could use to compile programs in a high level language. I read the specs of
   [MIPS-I](https://en.wikipedia.org/wiki/R2000_\(microprocessor\)),
   different [AVR](https://en.wikipedia.org/wiki/Atmel_AVR_ATtiny_comparison_chart),
   [Zilog Z80](https://en.wikipedia.org/wiki/Zilog_Z80) and
   [MOS 6502](https://en.wikipedia.org/wiki/MOS_Technology_6502).

A microcomputer without software is not very useful but I didn't feel like writing
much software myself using my own dialect of Basic or Forth so I decided to
implement a virtual machine, specially because there was the option to use a
cross compiler.

I finally went with the MOS 6502, for several reasons. Partly because it seemed
to me simpler than the other contenders (as in _simpler to implement_), it is an
8-bit CPU (the AVR is 8-bit too), and there are a lot of resources and documentation
really useful to build simulators.

## Incremental design and modularity

I split the project in several modules and started working on them one by one,
incrementally, so I had a working prototype with both hardware and software almost
all the time.

The parts are:

 - Video output: composite video generation (PAL, I'm in Europe).
 - Memory: because there's only one external SRAM unit, the memory can only be read
   or written when is not in use by the video generation.
 - Virtual machine: the 6502 VM, including some services via an unused instruction of
   the CPU.
 - Keyboard input: the PS2 keyboard driver (UK layout).
 - Storage: load and save programs from external storage.
 - Init: this is the module that contains the program entry point and basically puts
   together all the other modules. Whilst the other modules are more or less reusable,
   this one includes the specifics of DAN64.

Early on in the process I decided that I needed a way to document the hardware design
so I did some research and experimented with different EDA software for Linux,
deciding finally that [KiCad](http://www.kicad-pcb.org/) was the best package for my needs.

It was hard to learn KiCad on the go, but at the end it was very useful when I had
to try and test new ideas without losing a working design.

## Features

When I started the project I didn't know what was possible and what was not so I
kept my spec simple and as open as I could. In some cases I managed to do more than
I was hoping and in others my expectations were too high, so let's take a look
at the features I built into the microcomputer.

Current version of the project has the following features:

 - Composite video black and white output, 256 x 192 resolution, 32 x 24
   characters (8 x 8 pixels font, code page 437 character set).
 - PS/2 keyboard support.
 - 6502 virtual machine with system call interface to native code
   services.
 - Linear 64KB memory access from the virtual machine (256 bytes page zero,
   256 bytes hardware stack, 6144 bytes of video RAM and 58880 bytes for
   user programs).
 - External storage support via audio in/out.
 - Integrated 6502 assembler and disassembler.
 - Basic shell supporting peek, poke, load, run, etc.

### Rendering video from external SRAM

The core of the design is the video generation, as it often was in 80s microcomputers'
designs. The video is generated by bit-banging, so first of all speed is important.

I did some experiments using the SPI interface, nothing new and probably one of
the simplest ways of getting the required speed. Once I was happy with the quality of
the output, the next problem to solve was the memory. In the final microcomputer
6144 bytes of video RAM are used (this is 256 x 192 pixels, 1 bit per pixel).
That's definitely too much for the ATmega328p and its 2048 bytes of internal
SRAM.

If only text modes were supported, the memory usage could be reduced to 768 bytes
(32 x 24 characters using a 8 x 8 pixels font), but I wanted to support graphics and
user defined characters (tiles).

Then I found the [Microchip
23LCV512](http://www.microchip.com/wwwproducts/Devices.aspx?product=23LCV512),
a 64KB SPI SRAM. In my original idea I was planning to use a 32KB memory IC to
store programs and somehow generate the video, but the 23LCV512 has the
interesting property of being able to operate at 5 volts, so that's the main
reason the microcomputer is called *DAN64*.

The MCU writes into the external SRAM normally using SPI, but when the data is
read back from the SRAM, the IC bit-bangs the data into the video output. Then two
transistors are used to isolate the video output when the MCU needs to actually read
data from the SRAM.

Because the external SRAM is used for both video memory and to store programs, this
is the main bottleneck of the design as the memory can be only used by the 6502
virtual machine when is not actively used by the video output.

Most of the success and the flaws of the microcomputer are directly derived from
this design.

The memory layout is as follows:

	0x0000 - 0x00ff : zero page
	0x0100 - 0x01ff : stack
	0x0200 - 0x1aff : video memory
	0x1a00 - 0xffff : user program

The terms *zero page* and *stack* are related to the 6502 virtual machine.

### The 6502 virtual machine

The virtual machine is pretty straightforward. I used the wonderful [6502 functional
tests](https://github.com/Klaus2m5/6502_65C02_functional_tests) by Klaus
Dormann to validate the virtual machine and the CC65 compiler suite (assembler
and C compiler) to write some programs.

The two first pages of memory are implemented with internal SRAM of the MCU to speed
up the VM execution. The code the C compiler generates is still slow though, specially
using the CC65's standard C library.

The virtual machine provides a special instruction SYS (opcode 0x02) to expose some
services. The service is specified in the accumulator (that will hold the result
of the call), and the 6502 stack is used to pass parameters.

C programs can use the VM services (see `cc65/include/d64.h`) and the generated
binaries are almost as fast as manually written assembler code.

Current supported services are:

 * 0x00: Terminate program
 * 0x01: Load data
 * 0x02: Save data
 * 0x10: Put character
 * 0x11: Put string
 * 0x12: Set cursor position
 * 0x13: Fill screen
 * 0x14: Write (used by CC65 C compiler, supports stdout and stderr only)
 * 0x20: Get character
 * 0x21: Get input
 * 0x22: Read (used by CC65 C compiler, supports stdin only)
 * 0x30: Put tile
 * 0xa0: Get random
 * 0xa1: Wait for vsync
 * 0xa2: Set random seed
 * 0xf0: Get version

Refer to the manual in the downloads section for further details about the different
services.

### Storage

Because I wanted to build a 8-bit microcomputer in 80s style, I thought loading and
storing programs encoded as audio would be quite nice.

DAN64 uses a variant of PSK to encode the programs. The data block starts with a sync
pulse followed by a header that includes a magic number, the block length and a
parity byte as rudimentary checksum. Then the actual data follows, ending in
another parity byte.

I took inspiration from the Commodore 64 and the ZX Spectrum encodings, and the
resulting scheme is not too bad (although I didn't spend to much time searching for
limits; so there's a chance it could be faster). For example 58880 bytes
(biggest program to load into RAM), using random data, results in a 4 minutes 32
seconds audio file.

I didn't use any amplifier so the volume when loading programs must be quite high. I've
written a encoder tool (see `storage/tools/`) that can generate a WAV file from a 6502
binary.

The loading process is quite reliable and I've loaded quite large programs, including
screenshots loaded directly into video memory.

Because a pin change interrupt is used to load programs and it affects the video
generation, the video output is disabled during program loading.

Programs can be saved as audio files and loaded back, although volume may need to be adjusted
before that and it is not as reliable as loading data generated with the encoder (the
encoder uses 16-bit audio at 44100Hz, DAN64 outputs 8-bit audio at 15625Hz).

Save timing is important and it affects video generation so the video output is again
disabled when programs are being saved.

## Media

You can watch several short videos of the development process in [this YouTube
playlist](https://www.youtube.com/playlist?list=PLvI1iQmfH6UMJUAfsyax0VGHfUk-wekLz).

Click on the images for a full size version.

[![Breadboard prototype](./images/breadboard-small.jpg "Breadboard prototype")](./images/breadboard.jpg)
[![TV Out](./images/tv-out-small.jpg "TV output")](./images/tv-out.jpg)
[![PCB V1.R0](./images/pcb-small.jpg "PCB V1.R0")](./images/pcb.jpg)
[![Welcome](./images/welcome-small.png "Welcome screen")](./images/welcome.png)
[![Mandelbrot set](./images/mandelbrot-small.png "Mandelbrot output")](./images/mandelbrot.png)
[![YUM](./images/yum-small.png "Game of YUM")](./images/yum.png)

The DAN64 screenshots were taken using VLC and a composite video capture card.

## Downloads

Unless stated otherwise all the software is MIT licensed (see [LICENSE](LICENSE) file for further
details).

 * [DAN64 v1 binary](dan64_v1-bin.zip) (ZIP; includes the latest firmware for Arduino Uno: 2015-04-06)
 * [DAN64 hello world binary](hello.wav) (WAV; be careful, noisy!)
 * [6502 bin to wav encoder](encode-windows-bin.zip) (ZIP; windows console app)

The schematics and the build instructions are distributed under CERN OHL 1.2
terms (see [cern\_ohl\_v\_1\_2.txt](cern_ohl_v_1_2.txt) for further details).

 * [DAN64 rev3 schematics](dan64_v1-schematics.pdf) (PDF)
 * [DAN64 V1.R3 manual](dan64_v1-manual.pdf) (PDF; includes VM API docs, schematics, component list, etc)

The microcomputer can be assembled on a breadboard with a regular Arduino Uno.

## Development

The hardware is considered "final" and I don't plan more revisions.

Bug fixes and pull requests to improve the software are welcome. The project is
currently [hosted at GitHub](https://github.com/reidrac/dan64).

