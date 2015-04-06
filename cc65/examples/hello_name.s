; hello_name.s
; Hello <name> example using syscalls
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

			tsx				; save the stack pointer
			lda #9			; characters limit, inc ending zero
			pha
			lda #<name
			pha
			lda #>name
			pha
			lda #$21		; get input
			sys
			txs				; restore the stack

			tax
			lda #$21
			sta name,x
			inx
			lda #$0a
			sta name,x
			inx
			lda #$0
			sta name,x		; finish the message

			tsx				; save the stack pointer
			lda #<response
			pha
			lda #>response
			pha
			lda #$11		; put string
			sys
			txs				; restore the stack

			tsx				; save the stack pointer
			lda #<name
			pha
			lda #>name
			pha
			lda #$11		; put string
			sys
			txs				; restore the stack

			lda #$00		; terminate program
			pha				; also: success (0)
			sys

			jmp *			; never gets here

message:
			.asciiz "What's your name? "

response:
			.asciiz "Hello "
name:		.res 11, $0

