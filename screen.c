// screen.c

#include <nes.h>
#include "screen.h"
#include "constants.h"


// Constants
#define PointerSpriteIndex (1)
#define CardSpriteIndex (5)
const unsigned char PointerSprite_Tile[] = { 0xEE, 0xEF, 0xFE, 0xFF };

// Global variables
unsigned char *spriteAreaPtr = (unsigned char *)0x0200;
unsigned char attributeShadow[64]; // Copy of the attribute table in the nametable for easier modifications.

// Extern
extern const unsigned char PaletteData[];
extern const unsigned char PaletteDataSize;
extern const unsigned char PlaceholderTileData[];
extern const unsigned char PlaceholderTileDataSize;
extern const unsigned char PlaceholderRowData[];
extern const unsigned char PlaceholderRowDataSize;

// Function Prototypes
void drawTestPattern(void);
void placeCardTiles(unsigned char x, unsigned char y, const unsigned char *tiles, unsigned char color);
unsigned char hexChar(unsigned char value);

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
	
	// Clear Attribute shadow table
	for (index8=0; index8<64; ++index8) {
		attributeShadow[index8] = 0;
	}

	// Load the palette
	PPU.vram.address = 0x3F;
	PPU.vram.address = 0x00;
	for (index8=0; index8<PaletteDataSize; ++index8) {
		PPU.vram.data = PaletteData[index8];
	}
	
	// Draw screen
	drawPlaceholderRow();
	drawString("PRESS START", 10, 11);
		
	// Finalize and turn screen on
	refreshAttributeTable();
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

// == setColorAttribute() ==
void setColorAttribute(unsigned char color, unsigned char x, unsigned char y) {
	unsigned char offset = (x / 4) + 8 * (y / 4);
	unsigned char h = (x / 2) % 2;
	unsigned char v = (y / 2) % 2;
	unsigned char shift = (h + 2 * v) * 2;
	unsigned char mask = 0x03;
	unsigned char value;
	
	color = (color << shift);
	mask = (mask << shift);
	
	value = attributeShadow[offset];
	value = value & (~mask);
	value = value | (color & mask);
	attributeShadow[offset] = value;
}

// == refreshAttributeTable() ==
void refreshAttributeTable(void) {
	unsigned char i;
	
	// Load the palette
	PPU.vram.address = 0x23;
	PPU.vram.address = 0xC0;
	for (i=0; i<64; ++i) {
		PPU.vram.data = attributeShadow[i];
	}
}

// == movePointerTo() ==
void movePointerTo(unsigned char x, unsigned char y) {
	unsigned char i;
	SpriteInfo *sprite;
	
	for (i=0; i<4; ++i) {
		sprite = (SpriteInfo *)(spriteAreaPtr + 4 * (i + PointerSpriteIndex));
		sprite->x = x + (i % 2) * 8;
		sprite->y = y + (i / 2) * 8;
		sprite->tile = PointerSprite_Tile[i];
		sprite->attributes = 0x00;
	}
}

// == setCardSprite() ==
void setCardSprite(unsigned char *cards, unsigned char x, unsigned char y) {
	unsigned char topCard = cards[0];
	unsigned char i;
	unsigned char tile[12];
	unsigned char color;
	SpriteInfo *sprite;
	
	color = getCardTilesAndColor (topCard, tile);
	
	for (i=0; i<12; ++i) { // 3 wide by 4 high
		sprite = (SpriteInfo *)(spriteAreaPtr + 4 * (i + CardSpriteIndex));
		sprite->x = x + (i % 3) * 8;
		sprite->y = y + (i / 3) * 8 - 1;
		sprite->tile = tile[i];
		sprite->attributes = 0x01;
	}
	
	// Set sprite color
	i = color * 4 + 1;
	PPU.vram.address = 0x3F;
	PPU.vram.address = 0x15;
	PPU.vram.data = PaletteData[i];
	PPU.vram.data = PaletteData[++i];
	
}

// == getCardTiles() ==
unsigned char getCardTilesAndColor (unsigned char card, unsigned char tiles[12]) {
	// Returns color.
	unsigned char cardSuit = 0;
	unsigned char cardValue;
	unsigned char cardColor;
	
	if (card < FirstSpecialCard) { // Normal rank cards.
		cardValue = card % 9; 
		cardColor = card / 9;
		cardSuit = cardColor + 2;  // add 2 because the suits start 2 after the blank tile.
	} else if (card < FlowerCard) { // "Color" card
		cardColor = (card - FirstSpecialCard) / 4; // There are 4 of each "color" card.
		cardValue = 9 + cardColor;
	} else { // Flower card.
		cardColor = 1; 
		cardValue = 12;
	}

	tiles[0] = 0xA1 + cardValue;
	tiles[1] = 0xB2 + cardSuit;
	tiles[2] = 0xB3;
	
	tiles[3] = 0xC1; 
	tiles[4] = 0xC2; 
	tiles[5] = 0xC3; 

	tiles[6] = 0xC1; 
	tiles[7] = 0xC2; 
	tiles[8] = 0xC3; 

	tiles[9] = 0xD1; 
	tiles[10] = 0xD2; 
	tiles[11] = 0xD3; 

	return cardColor;	
}

// == drawCard() ==
void drawCard (unsigned char card, unsigned char x, unsigned char y) {
	unsigned char tiles[12];
	unsigned char cardColor;
	
	cardColor = getCardTilesAndColor (card, tiles);
	placeCardTiles (x, y, tiles, cardColor);
}

// == drawPlaceholder() ==
void drawPlaceholder(unsigned char x, unsigned char y) {
	placeCardTiles (x, y, PlaceholderTileData, 2);
}

// == placeCardTiles() ==
void placeCardTiles(unsigned char x, unsigned char y, const unsigned char *tiles, unsigned char color) {
	unsigned int address = 0x2000 + y * 32 + x;

	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = *tiles;
	PPU.vram.data = *(++tiles);
	PPU.vram.data = *(++tiles); 
	
	address += 32; // next line
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = *(++tiles); 
	PPU.vram.data = *(++tiles); 
	PPU.vram.data = *(++tiles); 

	address += 32; // next line
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = *(++tiles); 
	PPU.vram.data = *(++tiles); 
	PPU.vram.data = *(++tiles); 

	address += 32; // next line
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = *(++tiles); 
	PPU.vram.data = *(++tiles); 
	PPU.vram.data = *(++tiles); 

	// Update the attribute table
	setColorAttribute(color, x, y);
	if ((x % 2) == 1) { 
		// For odd values of x, also set the color attribute of the suit tile, because it falls on a different megatile.
		setColorAttribute(color, x + 1, y);
	}
}

// == drawPlaceholderRow() ==
void drawPlaceholderRow(void) {
	unsigned char i;
	
	// Set nametable cells
	PPU.vram.address = 0x20; // Start 2 lines down
	PPU.vram.address = 0x40;
	for (i=0; i<PlaceholderRowDataSize; ++i) {
		PPU.vram.data = PlaceholderRowData[i];
	}
	
	// Set attribute cells
	for (i=0; i<16; ++i) {
		attributeShadow[i] = 0xAA;
	}
}

// == eraseRect() ==
void eraseRect (unsigned char x, unsigned char y, unsigned char width, unsigned char height) {
	unsigned int address = 0x2000 + y * 32 + x;
	unsigned char i, j;

	for (i=0; i<height; ++i) {
		PPU.vram.address = (address >> 8);
		PPU.vram.address = (address & 0xFF);
		for (j=0; j<width; ++j) {
			PPU.vram.data = 0x20; // Use ASCII space character to erase
		}
		address += 32; // next line
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

// == drawHexByte() ==
void drawHexByte (unsigned char byte, unsigned char x, unsigned char y) {
	unsigned int address = 0x2000 + y * 32 + x;
	
	PPU.vram.address = (address >> 8);
	PPU.vram.address = (address & 0xFF);
	PPU.vram.data = hexChar(byte >> 4);
	PPU.vram.data = hexChar(byte);
}

// == hexChar() ==
unsigned char hexChar(unsigned char value) {
	value = value & 0x0F;
	return (value < 10)? 0x30 + value : 0x37 + value;
}

