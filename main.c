// main.c

/* Build commands:

/Applications/Emulators/cc65-master/bin/cl65 -O -t nes -C nes.cfg main.c screen.c data.s util.s -o test.nes
open test.nes

*/

#include <nes.h>
#include <time.h>
//#include <peekpoke.h>
#include "screen.h"

// Constants
#define JoyUp (0x08)
#define JoyDown (0x04)
#define JoyLeft (0x02)
#define JoyRight (0x01)
#define JoyStart (0x10)
#define JoySelect (0x20)
#define JoyButtonB (0x40)
#define JoyButtonA (0x80)
#define MaxColumnHeight (12)

// Global variables
unsigned char deck[40];
unsigned char columns[8 * MaxColumnHeight];
unsigned char freecell[4];
unsigned char foundation[3];

// Function prototypes
void shuffleDeck(void);
void startNewGame(void);

// Assembly functions
unsigned char __fastcall__ readJoypad(void);
unsigned char __fastcall__ pseudorandom(void);
void __fastcall__ seedrandom(unsigned int x);

// == main() ==
void main (void) {
	unsigned char x = 0x7F;
	unsigned char y= 0x77;
	unsigned char joypad = 0;
	
	initScreen();
	
	// Wait for start button
	while ((joypad & 0xF0) == 0) {
		joypad = readJoypad();
	}
	
	startNewGame();
	
	while (1) {
		waitvsync();
		refreshOAM();
		moveSpriteTo(x, y);
		
		// Update joypad and move sprite position. Actual move will happen after next VBL.
		joypad = readJoypad();
		if ((joypad & JoyLeft) != 0) {
			--x;
		}
		if ((joypad & JoyRight) != 0) {
			++x;
		}
		if ((joypad & JoyUp) != 0) {
			--y;
		}
		if ((joypad & JoyDown) != 0) {
			++y;
		}
	}
}

// == startNewGame() ==
void startNewGame(void) {
	unsigned int seed = clock();
	unsigned char i;
	unsigned char card;
	unsigned char col, row;

	seedrandom(seed);
	shuffleDeck();
	
	// Clear out tableau and cells
	for (i=0; i<4; ++i) {
		freecell[i] = 255;
	}
	for (i=0; i<3; ++i) {
		foundation[i] = 255;
	}
	for (i=0; i<8 * MaxColumnHeight; ++i) {
		columns[i] = 255;
	}

	// Draw entire screen
	waitvsync();
	setScreenVisible(0);
	drawCardPlaceholders();

	// Place and draw cards into columns at the same time.
	for (i=0; i<40; ++i) {
		col = i % 8;
		row = i / 8;
		card = deck[i];
		columns[col * MaxColumnHeight + row] = card;
		drawCard (card, 3 * col + 4, 2 * row + 6);
	}
	
	resetScrollPosition();
	setScreenVisible(1);
}

// == shuffleDeck() ==
void shuffleDeck(void) {
	// Fisher-Yates shuffle, inside-out method.
	unsigned char i, j;
	for (i=0; i<40;++i) {
		j = pseudorandom() % (i + 1);
		deck[i] = deck[j];
		deck[j] = i;
	}
}
