; data.s

.rodata

.export _PaletteData, _PaletteDataSize
_PaletteData:
	.byte	$0a, $3d, $0f, $30 		; Background
	.byte	$0a, $26, $16, $30
	.byte	$0a, $2a, $1a, $30 
	.byte	$0a, $31, $01, $30
	.byte	$0a, $3d, $0f, $30		; Sprite
	.byte	$0a, $00, $00, $31
	.byte	$00, $00, $00, $00
	.byte	$00, $00, $00, $00
_PaletteDataSize: 
	.byte * - _PaletteData


.export _PlaceholderTileData, _PlaceholderTileDataSize
_PlaceholderTileData:
	.byte $B7, $B8, $B9
	.byte $C7, $20, $C9
	.byte $C7, $20, $C9
	.byte $D7, $D8, $D9
_PlaceholderTileDataSize:
	.byte * - _PlaceholderTileData

.export _PlaceholderRowData, _PlaceholderRowDataSize
_PlaceholderRowData:
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
		.byte $C7, $20, $C9
	.endrepeat
	.word $0000
	.byte $C7, $C6, $C9
	.byte $00
		.repeat 3
	.byte $C7, $20, $C9
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $C7, $20, $C9
	.endrepeat
	.word $0000
	.byte $C7, $D6, $C9
	.byte $00
	.repeat 3
		.byte $C7, $20, $C9
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
_PlaceholderRowDataSize:
	.byte * - _PlaceholderRowData

	
.segment "FONT"
	.incbin  "font.chr"		; custom font
;	.include "font.inc"		; font that comes with cc65

