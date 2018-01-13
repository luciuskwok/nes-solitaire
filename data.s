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
	.word $0000, $0000
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
	
	.word $0000, $0000
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000
	.byte $D4, $DA, $D6
	.byte $00
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000
	.byte $D4, $EA, $D6
	.byte $00
	.repeat 3
		.byte $D4, $20, $D6
	.endrepeat
	.word $0000, $0000
	
	.word $0000, $0000
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
	.byte $20, $20, $20, $20, $71, $72, $73, $74, $75, $76, $77, $78, $79, $7a, $7b, $73
	.byte $79, $7c, $7d, $7e, $20, $7f, $80, $81, $82, $83, $84, $85, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $86, $87, $88, $89, $8a, $8b, $8c, $8d, $8e, $8f, $90, $91
	.byte $92, $93, $94, $95, $20, $96, $97, $98, $99, $9a, $9b, $9c, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $9d, $9e, $9f, $a0, $a1, $a2, $a3, $a4, $a5, $a6, $a7, $9f
	.byte $a8, $a9, $aa, $ab, $20, $ac, $ad, $ae, $af, $b0, $b1, $9c, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $20, $20, $b2, $20, $20, $b3, $b4, $b5, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $b6, $b7, $b8, $b9, $ba, $bb, $bc, $bd, $20, $82, $be, $bf
	.byte $c0, $c1, $c2, $c3, $82, $c4, $c5, $c6, $c7, $c5, $c8, $c9, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $ca, $cb, $cc, $cd, $ce, $cf, $d0, $d1, $20, $d2, $d3, $d4
	.byte $d1, $20, $d5, $d6, $d2, $d3, $d7, $d8, $d9, $da, $db, $20, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $20, $dc, $dd, $de, $df, $e0, $e1, $d1, $20, $d2, $d3, $d4
	.byte $d1, $e2, $e3, $e4, $d2, $d3, $e5, $e6, $df, $e7, $e8, $20, $20, $20, $20, $20
	.byte $20, $20, $20, $20, $e9, $ea, $eb, $ec, $ed, $ee, $ef, $f0, $c8, $f1, $f2, $f3
	.byte $f4, $f5, $f6, $f7, $f8, $f2, $f9, $fa, $fb, $fc, $c8, $c9
_TitleScreenTileDataSize:
	.word * - _TitleScreenTileData


; FamiTone Music Data
.export _FamiToneMusicData
_FamiToneMusicData:
;	.include "danger_streets.s"
	
; FamiTone Sfx Data
.export _FamiToneSfxData
_FamiToneSfxData:
	.byte $00				; replace with actual data
	
.segment "FONT"
	.incbin  "font.chr"		; custom font
;	.include "font.inc"		; font that comes with cc65

