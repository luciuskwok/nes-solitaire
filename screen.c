// screen.c

#include <nes.h>
#include "screen.h"
#include "cards.h"
#include "constants.h"


// Constants
#define PointerSpriteIndex (1)
#define CardSpriteIndex (5)
#define AttributeTableAddress (0x2C30)
const unsigned char PointerSprite_Tile[] = { 0xEE, 0xEF, 0xFE, 0xFF };

// Global variables
unsigned char vramUpdates[255];
unsigned char vramUpdateIndex = 0;
unsigned char attributeTableShadow[64]; // Copy of the attribute table in the nametable for easier modifications.
unsigned char *spriteAreaPtr = (unsigned char *)0x0200;
unsigned char debugValue1 = 0, debugValue2 = 0;

// Extern
extern const unsigned char PaletteData[];
extern const unsigned char PaletteDataSize;
extern const unsigned char FaceDownCardTileData[];
extern const unsigned char PlaceholderTileData[];
extern const unsigned char PlaceholderRowData[];
extern const unsigned char PlaceholderRowDataSize;

// Function Prototypes
void updateVram(void);
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
		attributeTableShadow[index8] = 0;
	}

	// Load the palette
	PPU.vram.address = 0x3F;
	PPU.vram.address = 0x00;
	for (index8=0; index8<PaletteDataSize; ++index8) {
		PPU.vram.data = PaletteData[index8];
	}
	
	// Draw screen
	updateScreenForNewGame();
	drawString("PRESS START", 10, 11);
		
	// Finalize and turn screen on
	resetScrollPosition();
	setScreenVisible(1);
}

// == refreshScreen() ==
void refreshScreen(void) {
	waitvsync();
	setScreenVisible(0);
	if (vramUpdateIndex > 0) {
		updateVram();
	}
	refreshOAM(); // also resets scroll position
}

// == updateVram() ==
void updateVram(void) {
	// Internal format of an update: 
	// address: 2 bytes
	// length: 1 byte
	// data: number of bytes according to length
	unsigned char index = 0;
	unsigned char remain;
	while (index < vramUpdateIndex) {
		PPU.vram.address = vramUpdates[index];
		PPU.vram.address = vramUpdates[++index];
		remain = vramUpdates[++index];
		while (remain > 0) {
			PPU.vram.data = vramUpdates[++index];
			--remain;
		}
		++index;
	}
	vramUpdateIndex = 0; // Reset to zero
}

// == updateVram() ==
void addVramUpdate(unsigned int address, unsigned char length, const unsigned char *data) {
	unsigned char i = 0;
	if (vramUpdateIndex <= 252 - length) { // Check for enough space in vramUpdates buffer to store data
		vramUpdates[vramUpdateIndex] = (address >> 8);
		vramUpdates[++vramUpdateIndex] = (address & 0xFF);
		vramUpdates[++vramUpdateIndex] = length;
		while (i < length) {
			vramUpdates[++vramUpdateIndex] = data[i];
			++i;
		}
		++vramUpdateIndex;
	}
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
	PPU.vram.address = 0;
	PPU.vram.address = 0;
	PPU.scroll = 0;
	PPU.scroll = 0;
	setScreenVisible(1);
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
	
	value = attributeTableShadow[offset];
	value = value & (~mask);
	value = value | (color & mask);
	attributeTableShadow[offset] = value;
	
	addVramUpdate(AttributeTableAddress + offset, 1, &attributeTableShadow[offset]);
}

// == movePointerTo() ==
void movePointerTo(unsigned char x, unsigned char y) {
	unsigned char i;
	SpriteInfo *sprite;
	
	for (i=0; i<4; ++i) {
		sprite = (SpriteInfo *)(spriteAreaPtr + 4 * (i + PointerSpriteIndex));
		sprite->x = x + (i % 2) * 8;
		sprite->y = y + (i / 2) * 8 - 1;
		sprite->tile = PointerSprite_Tile[i];
		sprite->attributes = 0x00;
	}
}

// == setCardSprite() ==
// The cards parameter may be nil for an empty sprite.
void setCardSprite(unsigned char *cards, unsigned char x, unsigned char y) {
	unsigned char topCard = cards[0];
	unsigned char color = 0;
	unsigned char i;
	unsigned char tile[12];
	SpriteInfo *sprite;
	
	if (cards != 0) {
		color = getCardTilesAndColor (topCard, tile);
		if (cards[1] < 40) { // Modify bottom row of tiles to show that more cards are being moved.
			tile[9] = 0xF1;
			tile[10] = 0xF2;
			tile[11] = 0xF3;
		}
	}
	
	for (i=0; i<12; ++i) { // 3 wide by 4 high
		sprite = (SpriteInfo *)(spriteAreaPtr + 4 * (i + CardSpriteIndex));
		sprite->x = x + (i % 3) * 8;
		sprite->y = y + (i / 3) * 8 - 1;
		sprite->tile = (cards != 0)? tile[i] : 0x20;
		sprite->attributes = 0x01;
	}
	
	// Set sprite color
	if (cards != 0) {
		i = color * 4 + 1;
		PPU.vram.address = 0x3F;
		PPU.vram.address = 0x15;
		PPU.vram.data = PaletteData[i];
		PPU.vram.data = PaletteData[++i];
		PPU.vram.data = 0x31; // light blue
	}
}

// == animateCardSprite() ==
void animateCardSprite(unsigned char fromX, unsigned char fromY, unsigned char toX, unsigned char toY, unsigned char duration) {
	int t = 0;
	int dx =(int) toX - (int) fromX;
	int dy = (int) toY - (int) fromY;
	unsigned char i;
	int x, y;
	SpriteInfo *sprite;
	
	waitvsync();
	refreshOAM();
	
	while (t <= duration) {
		x = fromX + (dx * t / (int) duration);
		y = fromY + (dy * t / (int) duration);
		for (i=0; i<12; ++i) { // 3 wide by 4 high
			sprite = (SpriteInfo *)(spriteAreaPtr + 4 * (i + CardSpriteIndex));
			sprite->x = x + (i % 3) * 8;
			sprite->y = y + (i / 3) * 8 - 1;
		}
		++t;
		waitvsync();
		refreshOAM();
	}
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
		cardSuit = cardColor + 1;
	} else if (card < FlowerCard) { // "Color" card
		cardColor = (card - FirstSpecialCard) / 4; // There are 4 of each "color" card.
		cardValue = 9 + cardColor;
	} else { // Flower card or Face-Down card.
		cardColor = 1; 
		cardValue = 12;
	}
	
	if (card < FaceDownCard) {
		tiles[0] = 0xA1 + cardValue;
		tiles[1] = 0xCA + cardSuit;
		tiles[2] = 0xC3;
	
		tiles[3] = 0xD1; 
		tiles[4] = 0xD2; 
		tiles[5] = 0xD3; 

		tiles[6] = 0xD1; 
		tiles[7] = 0xD2; 
		tiles[8] = 0xD3; 

		tiles[9] = 0xE1; 
		tiles[10] = 0xE2; 
		tiles[11] = 0xE3; 
	} else {
		for (cardValue = 0; cardValue < 12; ++cardValue) {
			tiles[cardValue] = FaceDownCardTileData[cardValue];
		}
	}

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

	if (address < AttributeTableAddress) {
		addVramUpdate(address, 3, tiles);
	}
	
	address += 32; // next line
	if (address < AttributeTableAddress) {
		addVramUpdate(address, 3, tiles + 3);
	}

	address += 32; // next line
	if (address < AttributeTableAddress) {
		addVramUpdate(address, 3, tiles + 6);
	}

	address += 32; // next line
	if (address < AttributeTableAddress) {
		addVramUpdate(address, 3, tiles + 9);
	}

	// Update the attribute table
	setColorAttribute(color, x, y);
	if ((x % 2) == 1) { 
		// For odd values of x, also set the color attribute of the suit tile, because it falls on a different megatile.
		setColorAttribute(color, x + 1, y);
	}
}

// == eraseHalfCardArea() ==
void eraseHalfCardArea (unsigned char x, unsigned char y) {
	unsigned char blanks[3] = { 0x20, 0x20, 0x20 }; // ASCII space char
	unsigned int address = 0x2000 + y * 32 + x;
	unsigned char row;

	for (row=0; row<2; ++row) {
		addVramUpdate(address, 3, blanks);
		address += 32; // next line
	}
}

// == updateScreenForNewGame() ==
void updateScreenForNewGame(void) {
	unsigned int index16;
	unsigned char index8;
	
	// Set nametable cells
	PPU.vram.address = 0x20; // Start 2 lines down
	PPU.vram.address = 0x40;
	for (index8=0; index8<PlaceholderRowDataSize; ++index8) {
		PPU.vram.data = PlaceholderRowData[index8];
	}
	
	// Erase the remaining space.
	PPU.vram.address = 0x20; // Start 6 lines down
	PPU.vram.address = 0xC0;
	for (index16=0; index16<768; ++index16) {
		PPU.vram.data = 0x20;
	}
	
	// Fill attribute cells with color 2 (binary 10).
	PPU.vram.address = 0x23;
	PPU.vram.address = 0xC0;
	for (index8=0; index8<64; ++index8) {
		attributeTableShadow[index8] = 0xAA;
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

// == drawHexByte() ==
void drawHexByte (unsigned char byte, unsigned char x, unsigned char y) {
	unsigned int address = 0x2000 + y * 32 + x;
	unsigned char text[2];
	
	text[0] = hexChar(byte >> 4);;
	text[1] = hexChar(byte);
	addVramUpdate(address, 2, text);
}

// == hexChar() ==
unsigned char hexChar(unsigned char value) {
	value = value & 0x0F;
	return (value < 10)? 0x30 + value : 0x37 + value;
}

