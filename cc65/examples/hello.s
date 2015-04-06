; hello.s
; Hello World example using syscalls
;

.segment "CODE"

.macro		sys				; syscall
.byte		$02
.endmacro

main:
			tsx				; save the stack pointer
			lda #<message
			pha
			lda #>message
			pha
			lda #$11		; put string
			sys
			txs				; restore the stack

			lda #$00		; terminate program
			pha				; also: success (0)
			sys

			jmp *			; never gets here

message:
			.byte "Hello world from 6502!", $0a, 0

