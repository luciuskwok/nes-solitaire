// main.c

/* Build commands:

/Applications/Emulators/cc65-master/bin/cl65 -O -t nes -C nes.cfg main.c screen.c sound.c cards.c data.s util.s famitone2.s -o test.nes
open test.nes

*/

#include <time.h>
//#include <peekpoke.h>
#include "screen.h"
#include "cards.h"
#include "famitone2.h"
#include "util.h"
#include "constants.h"
#include <nes.h>

// Global variables
unsigned char cursorX = 1;
unsigned char cursorY= 6;
unsigned char cursorDidMove = 1;

// Function prototypes
void handleDPad(unsigned char joypad);
void handleButtons(unsigned char joypad);
void startMenu(void);
void resumeGame(void);
void moveCursorToCell(unsigned char x, unsigned char y);
void startNewGame(void);

// == main() ==
void main (void) {
	unsigned char joypad = 0;
	unsigned char previousJoypad = 0;
	unsigned char padTimer = 0;
	unsigned char padChanged;

	initScreen();
// 	initFamiTone();
// 	FamiToneMusicPlay(0);
	
	// Enable sound
	APU.status = 0x0F; // enable pulse, triangle, and noise channels.

	// Wait for start button or any other button.
	while ((joypad & 0xF0) == 0) {
		refreshScreen();
		joypad = readJoypad();
	}
	
	startNewGame();
	
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
				if ((joypad & 0xF0) != 0 && padChanged && (autoMoveNextFrame == 0)) {
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
		returnCardsToOrigin();
		startMenu();
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

// == startMenu() ==
void startMenu(void) {
	unsigned char selectedMenuItem = 2;
	unsigned char previousJoypad = 0;
	unsigned char joypad;
	const unsigned char left = 10;
	
	// Show a menu after user presses the start button.
	drawButton("NEW GAME", left, 11, 12);
	refreshScreen();
	drawButton("CONTINUE", left, 14, 12);
	refreshScreen();
	
	while (readJoypad() != 0) {
		// Wait for joypad button release
	}
	cursorDidMove = 1;
	
	// Loop while user selects a menu item.
	while (1) {
		refreshScreen();
		
		if (cursorDidMove) {
			// Move cursor to point at one of the options
			cursorDidMove = 0;
			movePointerTo(127, 76 + 24 * selectedMenuItem);
		}

		// Update joypad and move pointer.
		joypad = readJoypad();
		if (joypad != previousJoypad) {
			// Directions
			if ((joypad & JoyUp) != 0) {
				--selectedMenuItem;
				cursorDidMove = 1;
			}
			if ((joypad & JoyDown) != 0) {
				++selectedMenuItem;
				cursorDidMove = 1;
			}
			selectedMenuItem = (selectedMenuItem > 1)? selectedMenuItem : 1;
			selectedMenuItem = (selectedMenuItem < 2)? selectedMenuItem : 2;
			
			// Buttons
			if ((joypad & JoyStart) != 0) { // Start: cancel and return to game
				resumeGame();
				break;
			} else if ((joypad & (JoyButtonB | JoyButtonA)) != 0) { // Make selection
				if (selectedMenuItem == 1) {
					startNewGame();
					break;
				} else if (selectedMenuItem == 2) {
					resumeGame();
					break;
				}
			}
		}
		previousJoypad = joypad;
	}
}

// == resumeGame() ==
void resumeGame(void) {
	unsigned char i;
	
	// Erase text
	for (i=0; i<6; ++i) {
		drawString("            ", 10, 12+i);
		refreshScreen();
	}
	
	drawAllColumns();
	cursorDidMove = 1;
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
	unsigned char i;
	unsigned char card;
	unsigned char col, row;
	
	seedrandom(clock());
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
	
	// Erase entire screen and draw the placeholder row
	waitvsync();
	hideScreen();
	updateScreenForNewGame();
	setCardSprite(0, 0, 0);
	showScreen();
	
	// Place and draw cards into columns at the same time, showing each card one frame at a time.
	for (i=0; i<40; ++i) {
		col = i % 8;
		row = i / 8;
		card = deck[i];
		columnCard[col * MaxColumnHeight + row] = card;
		drawCardAtCell (card, col + 1, row + 2);
		refreshScreen();
	}
	
	cursorDidMove = 1;
	autoMoveNextFrame  = 1;
}


