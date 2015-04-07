DAN64, an AVR based 8-bit microcomputer
=======================================

This is my personal attempt to build a 8-bit microcomputer based on
the popular ATmega328 microcontroller (MCU) as an excuse to learn some
electronics. Check `docs/` directory for further information.

The microcomputer can be assembled and run in a breadboard with an
Arduino Uno board.

General features:

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

Project page: http://www.usebox.net/jjm/dan64/

General directory structure
---------------------------

 - `docs/`: documentation of the project.
 - `schematics/`: KiCad schematics and PCB design.
 - source code:
   - `include/`: general include files.
   - `init/`: main entry point for the firmware, including the implementation
     of the syscalls.
   - `video/`: composite video generation.
   - `input/`: PS/2 keyboard support.
   - `memory/`: memory functions.
   - `vm/`: 6502 virtual machine.
     - `test/`: virtual machine test suite.
   - `storage/`: storage using audio in/out.
     - `tools/`: wav audio file encoder.
   - `dasm/`: DAN64 assembler/disassembler.
     - `tools/`: standalone version.
   - `tools/`: some misc auxiliary tools (eg, font bitmap generation).
   - `cc65/`: CC65 runtime for cross-compiling C programs.
     - `examples/`: several user program examples in ca65 assembler and C.
   - `images/`: logos, pictures, screenshots, etc.


Build instructions
------------------

Install GCC AVR compiler and toolchain, and run `make` in the top level
directory.

If `avrdude` is installed and you want to deploy to an Arduino Uno board
you can try `make upload`, although some tweaking may be requited.

The examples and the CC65 runtime require the
[CC65 compiler](https://github.com/cc65/cc65). If the binaries are in
your path, just use `make`.

The user programs can be encoded into audio using the `encode` tool in the
storage module (requires POSIX `getopt` and `libsndfile`).

Pandoc is required to build the documentation (although it is readable
as it is in markdown format).


Notes
-----

Video:

 - Timing is set for PAL.
 - Output is de-interlaced.
 - It may require set TV's AFC (Automatic Frequency Control) to "mode2"
   (usually for VCRs or Camcorders).

Input:

 - Supported UK PS/2 keyboard layout only, but is easy to adapt to other layouts.

Storage:

 - Audio in requires quite high volume, start with a 90% and adjust.


Author and licensing
--------------------

Juan J. Martinez <jjm@usebox.net>

This software is free software (see LICENSE file for details), unless
explicitly stated otherwise.

