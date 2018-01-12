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

// Constants
#define MaxInvalidCells (32)

// Global variables
unsigned char deck[40];
unsigned char cardsBeingMoved[MaxColumnHeight];
unsigned char columnCard[8 * MaxColumnHeight];
unsigned char freecellCard[4];
unsigned char foundationCard[4]; // There are only 3 foundations, but having 4 allocated makes some programming easier.
unsigned char invalidCell[MaxInvalidCells]; // for marking where to redraw cards
unsigned char invalidCellCount = 0;
unsigned char cursorX = 1;
unsigned char cursorY= 6;
unsigned char originatingCellX, originatingCellY; // used when moving cards
unsigned char cursorDidMove = 1;

// Function prototypes
unsigned int locationWithCell(unsigned char x, unsigned char y);
void moveCursorToCell(unsigned char x, unsigned char y);
unsigned char columnRowAtCursor(void);

void handleDPad(unsigned char joypad);
void handleButtons(unsigned char joypad);
void pickUpCardsAtCursor(void);
void dropCardsAtCursor(void);
void returnCardsToOrigin(void);
void clearCardsBeingMoved(void);
unsigned char numberOfCardsBeingMoved(void);
unsigned char columnHeight(unsigned char col);

void invalidateCell (unsigned char col, unsigned char row);
void drawInvalidCells(void);

void startNewGame(void);
void shuffleDeck(void);
void beep(unsigned int noteCode);

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
		setScreenVisible(0);
		// Update nametable here to avoid glithces
		//drawHexByte(cursorX, 0, 24); // debugging
		//drawHexByte(cursorY, 3, 24); 
		drawInvalidCells(); 
		if (attributeTableNeedsUpdate != 0) {
			refreshAttributeTable();
		}
		refreshOAM(); // also resets scroll position
		
		if (cursorDidMove != 0) {
			moveCursorToCell(cursorX, cursorY);
			cursorDidMove = 0;
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

// == locationWithCell() ==
unsigned int locationWithCell(unsigned char x, unsigned char y) {
	if (y > 1) {
		++y;
	}
	x = x * 24 + 8;
	y = y * 16;
	return x | y << 8;
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

// == columnRowAtCursor() ==
// Returns the row of the card at the cursor, taking in to account the height of the bottom-most card. If cursor is at top row, returns 0. If no valid card is at cursor, returns 255.
unsigned char columnRowAtCursor(void) {
	unsigned char col = cursorX - 1;
	unsigned char row = cursorY- 2;
	unsigned char height;
	
	if (cursorY == 1) {
		return 0;
	}
	
	height = columnHeight(col);
	if (row <= height) {
		if (row == height) {
			--row; // Selecting bottom half of bottom-most card counts as selecting the card.
		}
		return row;
	} else {
		return 255;
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
	} else if ((joypad & JoySelect) != 0) {
		// Cancel card movement.
		returnCardsToOrigin();
	} else if ((joypad & (JoyButtonB | JoyButtonA)) != 0) {
		// Handle A and B buttons the same.
		if (cardsBeingMoved[0] >= 40) {
			// If cards are being not moved, pick up cards at cursor.
			pickUpCardsAtCursor();
		} else {
			// Otherwise drop cards at cursor.
			dropCardsAtCursor();
		}
	}
}

// == pickUpCardsAtCursor() ==
void pickUpCardsAtCursor(void) {
	unsigned char col = cursorX - 1;
	unsigned char row = columnRowAtCursor();
	unsigned char validCard = 0;
	unsigned char index;
	unsigned int cardLocation;
	unsigned char height;

	clearCardsBeingMoved();
	
	if (cursorY == 1) { // Top row: only single card selection
		if (col < 3) { // Can only pick up cards from freecells, not from foundations.
			cardsBeingMoved[0] = freecellCard[col];
			cardLocation = locationWithCell(cursorX, cursorY);
			originatingCellX = cursorX;
			originatingCellY = cursorY;
			invalidateCell(cursorX, cursorY);
			validCard = 1;
		}
	} else if (cursorY >= 2) { // Tableau 
		height = columnHeight(col);
		// This code only allows picking up the bottom-most card in a column. In the real game, stacks of cards may be picked up if they are in the correct order.
		if (row == height - 1) {
			// Last card in column
			// Move card from column to cards being moved
			index = col * MaxColumnHeight + row;
			cardsBeingMoved[0] = columnCard[index];
			columnCard[index] = 255;
			cardLocation = locationWithCell(cursorX, row + 2);
			originatingCellX = cursorX; // allows drops back at original location to cancel a move
			originatingCellY = row + 2;
			
			// Invalidate both the card being picked up and the one above it, to make sure the whole card is updated.
			invalidateCell(col + 1, row + 2); // make sure to draw the empty cell first
			invalidateCell(col + 1, row + 1); // draw the card left behind
			validCard = 1;
		}
		
	}
	
	if (validCard) {
		beep(NoteA4);
		
		// Update the card sprite with the card being moved.
		setCardSprite(cardsBeingMoved, cardLocation & 0xFF, cardLocation >> 8);
		resetScrollPosition();
	} else {
		// No valid card to select
		beep(NoteA3);
	}
}

// == dropCardsAtCursor() ==
void dropCardsAtCursor(void) {
	unsigned char moveCount = numberOfCardsBeingMoved();
	unsigned char col = cursorX - 1;
	unsigned char validMove = 0;
	unsigned char moveCard = cardsBeingMoved[0];
	unsigned char height, bottomCard, i;
	
	// todo: handle cancel in case of drop back at originating cell
	
	if (cursorY == 1) { // Top row
		if (moveCount == 1) {
			if (col < 3) {
				if (freecellCard[col] >= 40) { // Make sure cell is empty
					validMove = 1;
					freecellCard[col] = moveCard;
					cardsBeingMoved[0] = 255;
					invalidateCell(cursorX, cursorY);
				}
			} else if (col >= 5) {
				bottomCard = foundationCard[col-5];
				if (moveCard < 27) {	// Only rank cards may move to foundation
					if (bottomCard < 40) {
						if ((moveCard / 9 == bottomCard / 9) && (moveCard == bottomCard + 1)) {
							validMove = 1; // Only cards in ascending order can go on foundation.
						}
					} else {
						if (moveCard % 9 == 0) { 
							validMove = 1;// Only rank 1 cards can move to empty foundation.
						}
					}
				}
				if (validMove) {
					foundationCard[col-5] = moveCard;
					cardsBeingMoved[0] = 255;
					invalidateCell(cursorX, cursorY);
				}
			} else { 
				// Cannot move to space between cells and foundations.
				// The flower card should move to its place automatically.
				validMove = 0;
			}
		}
	} else {
		// Dropping cards anywhere in a column will be interpreted as dropping them on the bottom-most card.
		height = columnHeight(col);
		if (height == 0) {
			validMove = 1; // Can always drop a card in an empty column
		} else {
			// Check if cards can be placed on the bottom card of the column.
			bottomCard = columnCard[col * MaxColumnHeight + height - 1];
			if ((bottomCard < 27) && (bottomCard / 9 != moveCard / 9) && ((bottomCard % 9) == (moveCard % 9) + 1) && (height + moveCount < MaxColumnHeight) ) { 
				// Only match rank cards of different suits where the bottom card is one more than the moved card. Also, make sure that the move does not cause a column to exceed its max height.
				validMove = 1;
			}
		}
		if (validMove) { // Move cards to column
			beep(NoteA4);
			for (i=0; i<moveCount; ++i) {
				columnCard[col * MaxColumnHeight + height + i] = cardsBeingMoved[i];
				cardsBeingMoved[i] = 255;
				invalidateCell(col + 1, height + i + 2);
			}
		}
	}
	
	if (validMove) { // Remove card sprite
		setCardSprite(0, 0, 0);
	} else {
		beep(NoteA3);
	}
}

// == returnCardsToOrigin() ==
void returnCardsToOrigin(void) {
	unsigned char col = originatingCellX - 1;
	unsigned char row = originatingCellY - 2;
	unsigned char i;
	
	if (cardsBeingMoved[0] >= 40) {
		return;
	}
	
	beep(NoteA3);

	// originatingCellX and Y use 1,1 as the top left coordinate.
	if (originatingCellY == 1) {
		// Only the 3 freecells are valid origins
		if (originatingCellX <= 3) {
			// Should be only one card being moved.
			freecellCard[originatingCellX - 1] = cardsBeingMoved[0];
			cardsBeingMoved[0] = 255;
			invalidateCell(originatingCellX, originatingCellY);
		}
	} else if (originatingCellY > 1) {
		i = 0;
		while (cardsBeingMoved[i] < 40 && i < MaxColumnHeight) {
			columnCard[col * MaxColumnHeight + row + i] = cardsBeingMoved[i];
			cardsBeingMoved[i] = 255;
			invalidateCell(originatingCellX, originatingCellY + i);
			++i;
		}
	}
	
	// Remove card sprite
	setCardSprite(0, 0, 0);
}

// == clearCardsBeingMoved() ==
void clearCardsBeingMoved(void) {
	unsigned char i;
	for (i=0; i<MaxColumnHeight; ++i) {
		cardsBeingMoved[i] = 255;
	}
}

// == numberOfCardsBeingMoved() ==
unsigned char numberOfCardsBeingMoved(void) {
	unsigned char count = 0;
	unsigned char i;
	for (i=0; i<MaxColumnHeight; ++i) {
		if (cardsBeingMoved[i] < 40) {
			++count;
		} else {
			break;
		}
	}
	return count;
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

// == invalidateCell() ==
void invalidateCell (unsigned char col, unsigned char row) {
	if (invalidCellCount < MaxInvalidCells) {
		--col;
		--row;
		invalidCell[invalidCellCount] = (col << 4) | row;
		++invalidCellCount;
	}
}

// == drawInvalidCells() ==
void drawInvalidCells(void) {
	unsigned char card = 255;
	unsigned char i;
	unsigned int location;
	unsigned char col, row, cell;
	unsigned char x, y;
	
	for (i=0; i<invalidCellCount; ++i) {
		cell = invalidCell[i];
		invalidCell[i] = 255;
		if (cell < 255) {
			col = cell >> 4;
			row = cell & 0x0F;
			location = locationWithCell(col + 1, row + 1);
		
			// Get card value
			if (row == 0) {
				if (col < 3) {
					card = freecellCard[col];
				} else if (col >= 5) {
					card = foundationCard[col - 5];
				}
			} else {
				card = columnCard[col * MaxColumnHeight + row - 1];
			}
			
			// Draw card
			x = (location & 0xFF) / 8;
			y = (location >> 8) / 8;
			if (card < 40) {
				drawCard (card, x, y);
			} else {
				if (row == 0) {
					drawPlaceholder (x, y);
				} else {
					eraseRect(x, y, 3, 4);
				}
			}
		}
	}
	invalidCellCount = 0;
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
	
	refreshAttributeTable();
	setCardSprite(0, 0, 0);
	resetScrollPosition();
	waitvsync();
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
void beep(unsigned int noteCode) {
	APU.status = 0x0F; // enable pulse, triangle, and noise channels.
	
	// time = ( CPU_clock / (16 * frequency) ) - 1
	// time = ( 1789773 / (16 * 1000) ) - 1
	// time = 110
	APU.pulse[0].control = 0x87;
	APU.pulse[0].ramp = 0x00;
	APU.pulse[0].period_low = noteCode & 0xFF;
	APU.pulse[0].len_period_high = 0x10 | ((noteCode >> 8) & 0x07);
}

