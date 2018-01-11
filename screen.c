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
extern const unsigned char CardPlaceholderData[];
extern const unsigned char CardPlaceholderDataSize;

// Function Prototypes
void drawTestPattern(void);

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
	
	// Clear sprite area
	setScreenVisible(0);
	index8 = 0;
	do {
		spriteAreaPtr[index8] = 0;
		++index8;
	} while (index8 != 0);

	// Clear screen RAM
	PPU.vram.address = 0x20;
	PPU.vram.address = 0x00;
	for (index16=0; index16<0x1000; ++index16) {
		PPU.vram.data = 0x00;
	}

	// Load the palette
	PPU.vram.address = 0x3F;
	PPU.vram.address = 0x00;
	for (index8=0; index8<PaletteDataSize; ++index8) {
		PPU.vram.data = PaletteData[index8];
	}
	
	// Draw screen
	drawCardPlaceholders();
	drawString("PRESS START", 10, 11);
		
	// Reset scroll position and turn screen on
	resetScrollPosition();
	setScreenVisible(1);
}

// == setScreenVisible() ==
void setScreenVisible(unsigned char on) {
	PPU.control = (on != 0)? 0x80 : 0x00; // enable NMI, use nametable 0
	PPU.mask = (on != 0)? 0x1E : 0x00;  // turn on screen
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
	setScreenVisible(1);
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

// == drawCard() ==
void drawCard (unsigned char card, unsigned char x, unsigned char y) {
	unsigned int address = 0x2000 + y * 32 + x;
	unsigned char i = 0;
	
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = 0xA1 + card % 9; // handle cards other than numbered ones
	PPU.vram.data = (card < 27)? (0xB4 + card / 9) : (0xB2); 
	PPU.vram.data = 0xB3; 
	
	address += 32; // next line
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = 0xC1; 
	PPU.vram.data = 0xC2; 
	PPU.vram.data = 0xC3; 

	address += 32; // next line
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = 0xC1; 
	PPU.vram.data = 0xC2; 
	PPU.vram.data = 0xC3; 

	address += 32; // next line
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = 0xD1; 
	PPU.vram.data = 0xD2; 
	PPU.vram.data = 0xD3; 
}

// == drawCardPlaceholders() ==
void drawCardPlaceholders(void) {
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

// == drawTestPattern() ==
void drawTestPattern(void) {
	unsigned char i;
	
	PPU.vram.address = 0x20; // Start 6 lines down
	PPU.vram.address = 0xC0;
	for (i=0; i<128; ++i) {
		PPU.vram.data = i;
	}
}

// == drawString() ==
void drawString(const char *string, unsigned char x, unsigned char y) {
	unsigned int address = 0x2000 + y * 32 + x;
	unsigned char i = 0;
	
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	while (string[i] != 0) {
		PPU.vram.data = string[i];
		++i;
		if (i == 0) break;
	}
}

// == stringWithByte() ==
void stringWithByte(unsigned char byte, char outString[]) {
	unsigned char index = 0;
	unsigned char length;
	unsigned char digit;
	
	// Generate digits in reverse order
	do {
		digit = byte % 10;
		outString[index] = 0x30 + digit;
		++index;
		byte = byte / 10;
	} while (byte > 0);
	
	// Add zero terminator
	outString[index] = 0x00;
	
	// Reverse the string
	length = index - 1;
	for (index = 0; index < length; ++index, --length) {
		digit = outString[index];
		outString[index] = outString[length];
		outString[length] = digit;
	}
}


