; Character specification table for code page 437
;
.include "ctype.inc"

.rodata

__ctype:
.res	32, CT_CTRL
.byte	CT_SPACE
.res	15, CT_NONE
.res	10, CT_DIGIT | CT_XDIGIT
.res	7, CT_NONE
.res	6, CT_UPPER | CT_XDIGIT
.res    20, CT_UPPER
.res	6, CT_NONE
.res	6, CT_LOWER | CT_XDIGIT
.res	20, CT_LOWER
.res    132, CT_NONE

