; data.s

.rodata

.export _MainPaletteData, _MainPaletteDataSize
_MainPaletteData:
	.byte	$0a, $3d, $0f, $30 		; Background
	.byte	$0a, $26, $16, $30
	.byte	$0a, $2a, $1a, $30 
	.byte	$0a, $31, $01, $30
	.byte	$0a, $3d, $0f, $30		; Sprite
	.byte	$0a, $00, $00, $31
	.byte	$00, $00, $00, $00
	.byte	$00, $00, $00, $00
_MainPaletteDataSize: 
	.byte * - _MainPaletteData


.export _FaceDownCardTileData
_FaceDownCardTileData:
	.byte $C7, $C8, $C9
	.byte $D7, $D8, $D9
	.byte $D7, $D8, $D9
	.byte $E7, $E8, $E9

.export _PlaceholderTileData
_PlaceholderTileData:
	.byte $C4, $C5, $C6
	.byte $D4, $20, $D6
	.byte $D4, $20, $D6
	.byte $E4, $E5, $E6

.export _PlaceholderRowData, _PlaceholderRowDataSize
_PlaceholderRowData:
	.word $0000, $0000				; row 0 
	.repeat 3
		.byte $C4, $C5, $C6
	.endrepeat
	.word $0000
	.byte $C4, $C5, $C6
	.byte $00
	.repeat 3
		.byte $C4, $C5, $C6
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000				; row 1
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000
	.byte $D4, $8E, $D6
	.byte $00
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000				; row 2
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000
	.byte $D4, $9E, $D6
	.byte $00
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000				; row 3
	.repeat 3
		.byte $E4, $E5, $E6
	.endrepeat
	.word $0000
	.byte $E4, $E5, $E6
	.byte $00
	.repeat 3
		.byte $E4, $E5, $E6
	.endrepeat
	.word $0000, $0000
_PlaceholderRowDataSize:
	.byte * - _PlaceholderRowData


; Title Screen
.export _TitleScreenTileData, _TitleScreenTileDataSize
_TitleScreenTileData:
	.incbin "title_screen.nam"
_TitleScreenTileDataSize:
	.word * - _TitleScreenTileData


; Title Screen Color Palette
.export _TitleScreenPaletteData, _TitleScreenPaletteDataSize
_TitleScreenPaletteData:
	.byte	$0f, $26, $27, $36 		; Background
	.byte	$0f, $16, $06, $24
	.byte	$0f, $11, $37, $30 
	.byte	$0f, $26, $16, $24
_TitleScreenPaletteDataSize: 
	.byte * - _TitleScreenPaletteData


.segment "FONT"
	.incbin  "font.chr"		; custom font
;	.include "font.inc"		; font that comes with cc65

