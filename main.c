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
void moveCursorToCell(unsigned char x, unsigned char y);
void handleClick(void);
void clearCardsBeingMoved(void);
unsigned char columnHeight(unsigned char col);
void startNewGame(void);
void shuffleDeck(void);
void beep(void);

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
	beep();
	
	while (1) {
		waitvsync();
		refreshOAM();
		moveCursorToCell(cursorX, cursorY);
		
		// Update joypad and move sprite position. Actual move will happen after next VBL.
		joypad = readJoypad();
		padChanged = (joypad != previousJoypad)? 1 : 0;
		previousJoypad = joypad;
		
		if ((padChanged != 0) || (padTimer == 0)) {
			
			if ((joypad & 0x0F) != 0) {
				// Handle D-pad
				beep();
				// Repeat delay is different for initial and subsequent ones.
				padTimer = padChanged? 20 : 7; 
		
				cursorX -= ((joypad & JoyLeft) != 0)? 1 : 0;
				cursorX += ((joypad & JoyRight) != 0)? 1 : 0;
				cursorY -= ((joypad & JoyUp) != 0)? 1 : 0;
				cursorY += ((joypad & JoyDown) != 0)? 1 : 0;
			
				// Limit cursor position
				cursorX = (cursorX > 1)? cursorX : 1;
				cursorX = (cursorX <8)? cursorX : 8;
				cursorY = (cursorY > 1)? cursorY : 1;
				cursorY = (cursorY < 13)? cursorY : 13;
				moveCursorToCell(cursorX, cursorY);
			} else if ((joypad & 0xF0) != 0) {
				// Handle buttons only if D-pad isn't active
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

// == moveCursorToCell() ==
void moveCursorToCell(unsigned char x, unsigned char y) {
	unsigned char yOffset = (y < 2)? 10 : 22;
	movePointerTo(x * 24 + 20, y * 16 + yOffset);
}

// == handleClick() ==
void handleClick(void) {
	unsigned char col, row, height, index, x, y;
	
	if (cursorY == 1) { // Top row: only single card selection
		// todo
	} else if (cursorY >= 2) { // Tableau 
		setScreenVisible(0);
		
		col = cursorX - 1;
		row = cursorY - 2;
		
		// Debugging
		drawHexByte (col, 0, 22);
		drawHexByte (row, 3, 22);

		height = columnHeight(col);
		if ((row >= height - 1) && (row <= height)) {
			// Last card in column
			row = (row > height - 1)? row : height - 1;
			clearCardsBeingMoved();
			index = col * MaxColumnHeight + row;
			cardsBeingMoved[0] = columnCard[index];
			
			// Remove card from column and draw changes
			columnCard[index] = 255;
			
			// Erase bottom half of card that was removed
			x = 3 * col + ColumnsOffsetX;
			y = 2 * (row + 1) + ColumnsOffsetY;
			eraseRect (x, y, 3, 2);
			// Redraw the card above the removed card, unless the column is empty.
			if (row > 0) {
				--row;
				y -= 4;
				index = col * MaxColumnHeight + row;
				drawCard (columnCard[index], x, y);
			}
		}
		
		resetScrollPosition();
		setScreenVisible(1);
	}
}

// == clearCardsBeingMoved() ==
void clearCardsBeingMoved(void) {
	unsigned char i;
	for (i=0; i<MaxColumnHeight; ++i) {
		cardsBeingMoved[i] = 255;
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
		drawCard (card, 3 * col + ColumnsOffsetX, 2 * row + ColumnsOffsetY);
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

// == beep() ==
void beep(void) {
	APU.status = 0x0F; // enable pulse, triangle, and noise channels.
	
	// time = ( CPU_clock / (16 * frequency) ) - 1
	// time = ( 1789773 / (16 * 1000) ) - 1
	// time = 110
	APU.pulse[0].control = 0x87;
	APU.pulse[0].ramp = 0x00;
	APU.pulse[0].period_low = 0x6E;
	APU.pulse[0].len_period_high = 0x10;
}

