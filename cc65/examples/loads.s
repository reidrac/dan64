; loads.s
; Screen loader
;

.segment "CODE"

.macro		sys				; syscall
.byte		$02
.endmacro

main:
			tsx				; save the stack pointer
			; load into video memory: 0x0200
			lda #00
			pha
			lda #02
			pha
			lda #$01		; load
			sys
			txs				; restore the stack

wait_key:
			lda #$20		; get char
			sys
			ora #$00
			beq wait_key

			tsx				; save the stack pointer
			lda #$20		; space
			pha
			lda #$13		; fill screen
			sys
			txs				; restore the stack

			lda #$00		; terminate program
			pha				; also: success (0)
			sys

			jmp *			; never gets here

