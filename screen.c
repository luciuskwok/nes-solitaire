// screen.c

#include <nes.h>
#include "screen.h"

// Constants
const unsigned char MetaSprite_X[] = { 0, 8, 0, 8 };
const unsigned char MetaSprite_Y[] = { 0, 0, 8, 8 };
const unsigned char MetaSprite_Tile[] = { 0xEE, 0xEF, 0xFE, 0xFF };
const unsigned char MetaSprite_Attr[] = { 0, 0, 0, 0 };

// Global variables
unsigned char *spriteAreaPtr = (unsigned char *)0x0200;

// Extern
extern const unsigned char PaletteData[];
extern const unsigned char PaletteDataSize;
extern const unsigned char AttributeData[];
extern const unsigned char AttributeDataSize;
extern const unsigned char CardPlaceholderData[];
extern const unsigned char CardPlaceholderDataSize;


// Struct
typedef struct {
	unsigned char y;
	unsigned char tile;
	unsigned char attributes;
	unsigned char x;
}  SpriteInfo;

// == initScreen() ==
void initScreen(void) {
	unsigned int index16;
	unsigned char index8;
	
	// Turn off the screen
	PPU.control = 0;
	PPU.mask = 0;
	
	// Clear sprite area
	index8 = 0;
	do {
		spriteAreaPtr[index8] = 0;
		++index8;
	} while (index8 != 0);

	// Clear screen RAM
	PPU.vram.address = 0x20;
	PPU.vram.address = 0x00;
	for (index16=0; index16<0x1000; ++index16) {
		//PPU.vram.data = 0x00;
		PPU.vram.data = index16 * 256;
	}

	// Load the palette
	PPU.vram.address = 0x3F;
	PPU.vram.address = 0x00;
	for (index8=0; index8<PaletteDataSize; ++index8) {
		PPU.vram.data = PaletteData[index8];
	}
	
	// Load attribute table
	PPU.vram.address = 0x23;
	PPU.vram.address = 0xDA;
	for (index8=0; index8<AttributeDataSize; ++index8) {
		PPU.vram.data = AttributeData[index8];
	}
	
	// Draw screen
	drawCardPlaceholder();
		
	// Reset scroll position
	resetScrollPosition();

	// Turn on the screen
	PPU.control = 0x80; // enable NMI
	PPU.mask = 0x1E; // turn on screen
}

// == drawCardPlaceholder() ==
void drawCardPlaceholder(void) {
	unsigned char i;
	
	// Set nametable cells
	PPU.vram.address = 0x20; // Start 2 lines down
	PPU.vram.address = 0x40;
	for (i=0; i<CardPlaceholderDataSize; ++i) {
		PPU.vram.data = CardPlaceholderData[i];
	}
	
	// Set attribute cells
	PPU.vram.address = 0x23;
	PPU.vram.address = 0xC0;
	for (i=0; i<16; ++i) {
		PPU.vram.data = 0xAA;
	}
	
}

// == resetScrollPosition() ==
void resetScrollPosition(void) {
	// Reset scroll position
	PPU.vram.address = 0;
	PPU.vram.address = 0;
	PPU.scroll = 0;
	PPU.scroll = 0;
}

// == refreshOAM() ==
void refreshOAM(void) {
	PPU.sprite.address = 0;
	APU.sprite.dma = 2;
	PPU.control = 0x80; // enable NMI
	PPU.mask = 0x1E; // turn on screen
	PPU.scroll = 0;
	PPU.scroll = 0;
}

// == moveSpriteTo() ==
void moveSpriteTo(unsigned char x, unsigned char y) {
	unsigned char i;
	SpriteInfo *sprite;
	
	for (i=0; i<4; ++i) {
		sprite = (SpriteInfo *)(spriteAreaPtr + 4 * i);
		sprite->x = MetaSprite_X[i] + x;
		sprite->y = MetaSprite_Y[i] + y;
		sprite->tile = MetaSprite_Tile[i];
		sprite->attributes = MetaSprite_Attr[i];
	}
}




