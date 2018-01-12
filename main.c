// main.c

/* Build commands:

/Applications/Emulators/cc65-master/bin/cl65 -O -t nes -C nes.cfg main.c screen.c cards.c data.s util.s -o test.nes
open test.nes

*/

#include <time.h>
//#include <peekpoke.h>
#include "screen.h"
#include "cards.h"
#include "util.h"
#include "constants.h"
#include <nes.h>

// Global variables
unsigned char cursorX = 1;
unsigned char cursorY= 6;
unsigned char cursorDidMove = 1;
unsigned char debugValue1 = 0, debugValue2 = 0;

// Function prototypes
void handleDPad(unsigned char joypad);
void handleButtons(unsigned char joypad);
void moveCursorToCell(unsigned char x, unsigned char y);

void startNewGame(void);
void shuffleDeck(void);

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
	autoMoveNextFrame  = 1;
	
	while (1) {
		refreshScreen();
		
		if (cursorDidMove) {
			cursorDidMove = 0;
			moveCursorToCell(cursorX, cursorY);
		}
		
		if (autoMoveNextFrame) {
			autoMoveNextFrame = 0;
			autoMoveCards();
		}
		
		// Update joypad and move sprite position. Actual move will happen after next VBL.
		joypad = readJoypad();
		padChanged = (joypad != previousJoypad)? 1 : 0;
		previousJoypad = joypad;
		
		if ((padChanged != 0) || (padTimer == 0)) {
			
			if ((joypad & 0x0F) != 0) {
				// Handle D-pad
				// Repeat delay is different for initial and subsequent ones.
				padTimer = padChanged? 20 : 7; 
				handleDPad (joypad);
			} else {
				// Reset padTimer when D-pad is released
				padTimer = 0;
				if ((joypad & 0xF0) != 0 && padChanged) {
					// Handle buttons only if D-pad isn't active, and with no repeat
					handleButtons (joypad);
				}
			}
		}
		if (padTimer != 0) {
			--padTimer;
		}
	}
}

// == handleDPad() ==
void handleDPad(unsigned char joypad) {
	cursorX -= ((joypad & JoyLeft) != 0)? 1 : 0;
	cursorX += ((joypad & JoyRight) != 0)? 1 : 0;
	cursorY -= ((joypad & JoyUp) != 0)? 1 : 0;
	cursorY += ((joypad & JoyDown) != 0)? 1 : 0;

	// Limit cursor position
	cursorX = (cursorX > 1)? cursorX : 1;
	cursorX = (cursorX <8)? cursorX : 8;
	cursorY = (cursorY > 1)? cursorY : 1;
	cursorY = (cursorY < 13)? cursorY : 13;
	cursorDidMove = 1;
}

// == handleButtons() ==
void handleButtons(unsigned char joypad) {
	if ((joypad & JoyStart) != 0) {
		// Handle menu: Start New Game, Exit to Title.
		// todo: show menu instead of just starting over
		startNewGame();
		autoMoveNextFrame = 1;
	} else if ((joypad & JoySelect) != 0) {
		// Cancel card movement.
		returnCardsToOrigin();
	} else if ((joypad & (JoyButtonB | JoyButtonA)) != 0) {
		// Handle A and B buttons the same.
		if (cardsBeingMoved[0] >= 40) {
			// If cards are being not moved, pick up cards at cursor.
			pickUpCardsAtCursor(cursorX, cursorY);
		} else {
			// Otherwise drop cards at cursor.
			dropCardsAtCursor(cursorX, cursorY);
		}
	}
}

// == moveCursorToCell() ==
void moveCursorToCell(unsigned char x, unsigned char y) {
	unsigned int loc = locationWithCell (x, y);
	x = loc & 0xFF;
	y = loc >> 8;
	if (y < 20) {
		y += 7;
	}
	movePointerTo(x + 11, y + 8);
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
	
	// Clear drawing list
	for (i=0; i<9 * MaxColumnHeight; ++i) {
		invalidCell[i] = 255;
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
		drawCard (card, 3 * col + ColumnsOffsetX, 2 * row + ColumnsOffsetY);
	}
	
	// Erase rows below columns
	eraseRect(4, 18, 24, 10);
	
	refreshAttributeTable();
	setCardSprite(0, 0, 0);
	resetScrollPosition();
	waitvsync();
	setScreenVisible(1);
}


