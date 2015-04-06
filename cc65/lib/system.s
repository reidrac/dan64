; DAN64 system interface for CC65
;
;

.export		_sys_exit, _sys_load, _sys_save, _putch, _cputs, _gotoxy, _clrscr, _fillscr, _write, _getch, _cgets, _read, _putt, __rand, __srand, _wait_vsync, _sys_ver

.import popa, popax

.segment "CODE"

.macro		sys func
			lda func
.byte		$02 ; SYS
.endmacro

.macro		sys_1 func
			pha
			sys func
			tsx
			inx
			txs
.endmacro

.macro		sys_2 func
			pha
			jsr popa
			pha
			sys func
			tsx
			inx
			inx
			txs
.endmacro

.macro		sys_pt func
			pha
			txa
			pha
			sys func
			tsx
			inx
			inx
			txs
.endmacro

.macro		sys_pt_1 func
			pha
			jsr popax
			pha
			txa
			pha
			sys func
			tsx
			inx
			inx
			inx
			txs
.endmacro

.macro		sys_1_pt_pt func
			pha
			txa
			pha
			jsr popax
			pha
			txa
			pha
			jsr popax
			pha
			txa
			pha
			sys func
			tsx
			inx
			inx
			inx
			inx
			inx
			inx
			txs
.endmacro

; system call definitions

.proc 		_sys_exit: near
			pha
			sys #$00
			jmp *
.endproc

.proc 		_sys_load: near
			sys_pt #$01
			rts
.endproc

.proc 		_sys_save: near
			sys_pt_1 #$02
			rts
.endproc

.proc 		_putch: near
			sys_1 #$10
			rts
.endproc

.proc 		_cputs: near
			sys_pt #$11
			rts
.endproc

.proc 		_gotoxy: near
			sys_2 #$12
			rts
.endproc

.proc 		_clrscr: near
			lda #$20
			jmp _fillscr
.endproc

.proc 		_fillscr: near
			sys_1 #$13
			rts
.endproc

.proc		_write: near
			sys_1_pt_pt #$14
			ldx #$00
			rts
.endproc

.proc 		_getch: near
			sys #$20
			rts
.endproc

.proc 		_cgets: near
			sys_pt_1 #$21
			rts
.endproc

.proc		_read: near
			sys_1_pt_pt #$22
			ldx #$00
			rts
.endproc

.proc 		_putt: near
			sys_pt #$30
			rts
.endproc

.proc 		__rand: near
			sys #$a0
			rts
.endproc

.proc 		_wait_vsync: near
			sys #$a1
			rts
.endproc

.proc 		__srand: near
			sys_pt #$a2
			rts
.endproc

.proc 		_sys_ver: near
			sys #$f0
			rts
.endproc

