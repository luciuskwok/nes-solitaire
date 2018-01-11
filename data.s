; data.s

.rodata

.export _PaletteData, _PaletteDataSize
_PaletteData:
	.byte	$0a, $3d, $0f, $30 
	.byte	$0a, $26, $16, $30
	.byte	$0a, $3a, $2a, $30 
	.byte	$0a, $31, $01, $30
	.byte	$0a, $3d, $0f, $30
	.byte	$00, $00, $00, $01
	.byte	$00, $00, $00, $00
	.byte	$00, $00, $00, $00
_PaletteDataSize: 
	.byte * - _PaletteData


.export _AttributeData, _AttributeDataSize
_AttributeData:
	.byte	$44, $BB, $44, $BB
_AttributeDataSize:
	.byte * - _AttributeData


.export _CardPlaceholderData, _CardPlaceholderDataSize
_CardPlaceholderData:
	.word $0000, $0000
	.repeat 3
		.byte $17, $18, $19
	.endrepeat
	.word $0000
	.byte $17, $18, $19
	.byte $00
	.repeat 3
		.byte $17, $18, $19
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $27, $00, $29
	.endrepeat
	.word $0000
	.byte $27, $26, $29
	.byte $00
		.repeat 3
	.byte $27, $00, $29
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $27, $00, $29
	.endrepeat
	.word $0000
	.byte $27, $36, $29
	.byte $00
	.repeat 3
		.byte $27, $00, $29
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $37, $38, $39
	.endrepeat
	.word $0000
	.byte $37, $38, $39
	.byte $00
	.repeat 3
		.byte $37, $38, $39
	.endrepeat
	.word $0000, $0000
_CardPlaceholderDataSize:
	.byte * - _CardPlaceholderData

	
.segment "FONT"
;	.incbin  "font.chr"
	.include "font.inc"

