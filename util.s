; util.s
;
; C function prototypes:
; unsigned char __fastcall__ readJoypad(void);
; unsigned char __fastcall__ pseudorandom(void);
; void __fastcall__ seedrandom(unsigned int);
; void __fastcall__ updateVramFast(void);

FAMITONE_ENABLE = 0 	; set to 1 to use FamiTone

; Zero-page globals
.zeropage
rngseed: .res 2

.code
.export _readJoypad
.export _pseudorandom
.export _seedrandom
.export _updateVramFast

.import _vramUpdates
.import _vramUpdateIndex
.import _get_tv

; PPU registers
PPUADDR = $2006
PPUDATA = $2007


; ==== readJoypad() ====
.proc _readJoypad
					; At the same time that we strobe bit 0, we initialize the ring counter
					; so we're hitting two birds with one stone here
	JOYPAD0 = $4016
	lda #$01		; While the strobe bit is set, buttons will be continuously reloaded.  
	sta JOYPAD0		; This means that reading from JOYPAD0 will only return the state of the first button: button A.
	tax				; store the result in X register
	lsr a			; now A register is 0
	sta JOYPAD0		; By storing 0 into JOYPAD0, the strobe bit is cleared and the reloading stops. This allows all 8 buttons (newly reloaded) to be read from JOYPAD0.
loop:	
	lda JOYPAD0
	lsr a			; bit0 -> Carry
	txa				; transfer result from X to A register
	rol a			; Carry -> bit0; bit 7 -> Carry
	tax				; transfer result back to X register
	bcc loop
	txa				; return the button state in A register
	ldx #0			; zero out the X register
	rts
.endproc


; ==== pseudorandom() ====
.proc _pseudorandom
	ldx #8     ; iteration count (generates 8 bits)
	lda rngseed+0
:
	asl        ; shift the register
	rol rngseed+1
	bcc :+
	eor #$2D   ; apply XOR feedback whenever a 1 bit is shifted out
:
	dex
	bne :--
	sta rngseed+0
	cmp #0     ; reload flags
	rts
.endproc

; ==== seedrandom() ====
.proc _seedrandom
	sta rngseed
	stx rngseed + 1
	rts
.endproc


; ==== updateVramFast() ====
.proc _updateVramFast
	ldx #0
	beq moreData
addrLoop:
	lda _vramUpdates,X			; VRAM address high-byte
	sta PPUADDR
	inx
	lda _vramUpdates,X			; VRAM address low-byte
	sta PPUADDR
	inx
	ldy _vramUpdates,X			; data length
	inx
dataLoop:
	lda _vramUpdates,X			; data byte
	sta PPUDATA
	inx
	dey
	bne dataLoop
moreData:
	cpx _vramUpdateIndex		; if X < vramUpdateIndex
	bcc addrLoop				; then keep looping
	lda #0
	sta _vramUpdateIndex		; reset vramUpdateIndex to zero
	rts
.endproc


; ====== FamiTone Section ======
.if(FAMITONE_ENABLE) 
.export _initFamiTone
.export _initFamiToneSfx
.import _FamiToneInit
.import _FamiToneMusicData
.import _FamiToneSfxInit
.import _FamiToneSfxData

; ==== initFamiTone() ====
.proc _initFamiTone
	jsr _get_tv		; returns 0 for NTSC, 1 for PAL
	eor #1			; but FamiTone expects 0 for PAL, so flip it
	ldx #<_FamiToneMusicData	; low-byte
	ldy #>_FamiToneMusicData	; high-byte
	jsr _FamiToneInit
	rts
.endproc

; ==== initFamiToneSfx () ====
.proc _initFamiToneSfx
	ldy #<_FamiToneSfxData		; low-byte
	ldy #>_FamiToneSfxData		; high-byte
	jsr _FamiToneSfxInit
	rts
.endproc

.endif
; ====== End FamiTone Section ======



