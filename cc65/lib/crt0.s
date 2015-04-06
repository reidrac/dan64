; cc65 startup code for DAN64

.macro		sys				; syscall
.byte		$02
.endmacro

.export __STARTUP__ : absolute = 1
.import initlib, donelib
.export _exit
.import _main

.import __TOPMEM__

.include "zeropage.inc"

.segment  "STARTUP"

_init:
        lda #<__TOPMEM__
        sta sp
        lda #>__TOPMEM__
        sta sp + 1

		jsr initlib

		jsr _main

_exit:
		; return code
		pha

		jsr donelib

		; terminate program
		lda #$00
		sys

		jmp *

