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
		.byte $B7, $B8, $B9
	.endrepeat
	.word $0000
	.byte $B7, $B8, $B9
	.byte $00
	.repeat 3
		.byte $B7, $B8, $B9
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $C7, $00, $C9
	.endrepeat
	.word $0000
	.byte $C7, $C6, $C9
	.byte $00
		.repeat 3
	.byte $C7, $00, $C9
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $C7, $00, $C9
	.endrepeat
	.word $0000
	.byte $C7, $D6, $C9
	.byte $00
	.repeat 3
		.byte $C7, $00, $C9
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $D7, $D8, $D9
	.endrepeat
	.word $0000
	.byte $D7, $D8, $D9
	.byte $00
	.repeat 3
		.byte $D7, $D8, $D9
	.endrepeat
	.word $0000, $0000
_CardPlaceholderDataSize:
	.byte * - _CardPlaceholderData

	
.segment "FONT"
	.incbin  "font.chr"		; custom font
;	.include "font.inc"		; font that comes with cc65

