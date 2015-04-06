; os specific error code mapping for DAN 64
;

.include "errno.inc"
.export __osmaperrno

.proc 	__osmaperrno

		lda #<EUNKNOWN
		ldx #>EUNKNOWN
		rts
.endproc


