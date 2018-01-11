// main.c

/* Build commands:

/Applications/Emulators/cc65-master/bin/cl65 -O -t nes -C nes.cfg main.c screen.c data.s util.s -o test.nes
open test.nes

*/

#include <nes.h>
#include <time.h>
//#include <peekpoke.h>
#include "screen.h"
#include "constants.h"

// Global variables
unsigned char deck[40];
unsigned char cardsBeingMoved[MaxColumnHeight];
unsigned char columnCard[8 * MaxColumnHeight];
unsigned char freecellCard[4];
unsigned char foundationCard[4]; // There are only 3 foundations, but having 4 allocated makes some programming easier.
unsigned char cursorX = 1;
unsigned char cursorY= 6;

// Function prototypes
void moveCursorTo(unsigned char x, unsigned char y);
void handleClick(void);
unsigned char columnHeight(unsigned char col);
void startNewGame(void);
void shuffleDeck(void);

// Assembly functions
unsigned char __fastcall__ readJoypad(void);
unsigned char __fastcall__ pseudorandom(void);
void __fastcall__ seedrandom(unsigned int x);

// == main() ==
void main (void) {
	unsigned char joypad = 0;
	unsigned char previousJoypad = 0;
	unsigned char padTimer = 0;
	unsigned char padChanged;
	
	initScreen();
	
	// Wait for start button or any other button.
	while ((joypad & 0xF0) == 0) {
		joypad = readJoypad();
	}
	
	startNewGame();
	
	while (1) {
		waitvsync();
		refreshOAM();
		moveCursorTo(cursorX, cursorY);
		
		// Update joypad and move sprite position. Actual move will happen after next VBL.
		joypad = readJoypad();
		padChanged = (joypad != previousJoypad)? 1 : 0;
		if ((padChanged) || (padTimer == 0)) {
			padTimer = padChanged? 20 : 7; // Repeat delay is different for initial and subsequent ones.
			previousJoypad = joypad;
		
			// Handle D-pad
			cursorX -= ((joypad & JoyLeft) != 0)? 1 : 0;
			cursorX += ((joypad & JoyRight) != 0)? 1 : 0;
			cursorY -= ((joypad & JoyUp) != 0)? 1 : 0;
			cursorY += ((joypad & JoyDown) != 0)? 1 : 0;
			
			// Limit cursor position
			cursorX = (cursorX > 1)? cursorX : 1;
			cursorX = (cursorX <8)? cursorX : 8;
			cursorY = (cursorY > 1)? cursorY : 1;
			cursorY = (cursorY < 13)? cursorY : 13;
			moveCursorTo(cursorX, cursorY);
			
			// Handle buttons only if D-pad isn't active
			if ((joypad | 0x0F) == 0) {
				if ((joypad & JoyStart) != 0) {
					// Handle menu: Start New Game, Exit to Title.
				} else if ((joypad & (JoyButtonB | JoyButtonA)) != 0) {
					// Handle A and B buttons the same.
					handleClick();
				}
			}
		}
		if (padTimer != 0) {
			--padTimer;
		}
	}
}

// == moveCursorTo() ==
void moveCursorTo(unsigned char x, unsigned char y) {
	unsigned char yOffset = (y < 2)? 10 : 22;
	moveSpriteTo(x * 24 + 20, y * 16 + yOffset);
}

// == handleClick() ==
void handleClick(void) {
	unsigned char col, row, height;
	
	if (cursorY == 1) { // Top row: only single card selection
		// todo
	} else { // Tableau 
		col = cursorX - 1;
		row = cursorY - 2;
		height = columnHeight(col);
		if ((row >= height - 1) && (row <= height)) {
			// Last card in column
			
		}
	}
}

// == columnHeight() ==
unsigned char columnHeight(unsigned char col) {
	unsigned char *ptr = columnCard + col * MaxColumnHeight;
	unsigned char result = 0;
	unsigned char row;

	if (col < 8) {
		for (row=0; row<MaxColumnHeight; ++row) {
			if (ptr[row] < 40) {
				++result;
			} else {
				break;
			}
		}
	}
	return result;
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
		freecellCard[i] = 255;
		foundationCard[i] = 255;
	}
	for (i=0; i<8 * MaxColumnHeight; ++i) {
		columnCard[i] = 255;
	}
	for (i=0; i<MaxColumnHeight; ++i) {
		cardsBeingMoved[i] = 255;
	}

	// Draw entire screen
	waitvsync();
	setScreenVisible(0);
	drawPlaceholderRow();

	// Place and draw cards into columns at the same time.
	for (i=0; i<40; ++i) {
		col = i % 8;
		row = i / 8;
		card = deck[i];
		columnCard[col * MaxColumnHeight + row] = card;
		drawCard (card, 3 * col + 4, 2 * row + 6);
	}
	
	refreshAttributeTable();
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
