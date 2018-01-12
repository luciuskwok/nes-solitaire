// cards.c

#include "cards.h"
#include "screen.h"
#include "util.h"
#include "constants.h"
#include <nes.h>

// Constants
#define MaxInvalidCells (32)

// Globals
unsigned char deck[40];
unsigned char cardsBeingMoved[MaxColumnHeight];
unsigned char columnCard[8 * MaxColumnHeight];
unsigned char freecellCard[4];
unsigned char foundationCard[4]; // There are only 3 foundations, but having 4 allocated makes some programming easier.
unsigned char originatingCellX, originatingCellY; // used when moving cards
unsigned char invalidCell[MaxInvalidCells]; // for marking where to redraw cards
unsigned char invalidCellCount = 0;

// Function Prototypes
void beep(unsigned int noteCode);


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

// == locationWithCell() ==
unsigned int locationWithCell(unsigned char x, unsigned char y) {
	if (y > 1) {
		++y;
	}
	x = x * 24 + 8;
	y = y * 16;
	return x | y << 8;
}

// == columnRowAtCursor() ==
// Returns the row of the card at the cursor, taking in to account the height of the bottom-most card. If cursor is at top row, returns 0. If no valid card is at cursor, returns 255.
unsigned char columnRowAtCursor(unsigned char curX, unsigned char curY) {
	unsigned char col = curX - 1;
	unsigned char row = curY- 2;
	unsigned char height;
	
	if (curY == 1) {
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

// == pickUpCardsAtCursor() ==
void pickUpCardsAtCursor(unsigned char curX, unsigned char curY) {
	unsigned char col = curX - 1;
	unsigned char row = columnRowAtCursor(curX, curY);
	unsigned char validCard = 0;
	unsigned char index;
	unsigned int cardLocation;
	unsigned char colHeight;
	unsigned char topMovableRow;
	unsigned char *colPtr;

	clearCardsBeingMoved();
	
	if (curY == 1) { // Top row: only single card selection
		if ((col < 3) && (freecellCard[col] < 40)) { // Can only pick up cards from freecells, not from foundations, and only if a card is there.
			cardsBeingMoved[0] = freecellCard[col];
			freecellCard[col] = 255;
			cardLocation = locationWithCell(curX, curY);
			originatingCellX = curX;
			originatingCellY = curY;
			invalidateCell(curX, curY);
			validCard = 1;
		}
	} else if (curY >= 2) { // Tableau 
		colHeight = columnHeight(col);
		if (colHeight > 0) {
			topMovableRow = topMovableRowAtColumn(col);
			
			if (topMovableRow <= row && row < colHeight) {
				// Move card from column to cards being moved
				colPtr = columnCard + col * MaxColumnHeight;
				for (index = row; index < colHeight; ++index) {
					cardsBeingMoved[index - row] = colPtr[index];
					colPtr[index] = 255;
					invalidateCell (curX, index + 2); // erase the area that the old card used
				}
				originatingCellX = curX; // allows drops back at original location to cancel a move
				originatingCellY = row + 2;
				cardLocation = locationWithCell(originatingCellX, originatingCellY);
			
				if (row > 0) { // Redraw the card left behind, if any
					invalidateCell(originatingCellX, originatingCellY - 1); 
				}
				validCard = 1;
			}
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
void dropCardsAtCursor(unsigned char curX, unsigned char curY) {
	unsigned char moveCount = numberOfCardsBeingMoved();
	unsigned char col = curX - 1;
	unsigned char validMove = 0;
	unsigned char isCancellingMove = 0;
	unsigned char moveCard = cardsBeingMoved[0];
	unsigned char height, bottomCard, i;
	
	// Handle drop on originating cell as a cancel
	if (curX == originatingCellX) {
		if ((curY == 1) && (originatingCellY == 1)) {
			isCancellingMove = 1;
		} else if ((curY > 1) && (originatingCellY > 1)) {
			isCancellingMove = 1; // Handle drops anywhere in the originating cell as a cancel
		}
		if (isCancellingMove) {
			returnCardsToOrigin();
			return;
		}
	}
	
	if (curY == 1) { // Top row
		if (moveCount == 1) {
			if (col < 3) {
				if (freecellCard[col] > 40) { // Make sure cell is empty
					validMove = 1;
					freecellCard[col] = moveCard;
					cardsBeingMoved[0] = 255;
					invalidateCell(curX, curY);
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
					invalidateCell(curX, curY);
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

// == topMovableRowAtColumn() ==
unsigned char topMovableRowAtColumn(unsigned char col) {
	unsigned char *colPtr = columnCard + col * MaxColumnHeight;
	signed char row = MaxColumnHeight - 1;
	unsigned char lowerCard = 255;
	unsigned char card;
	
	while (row > 0) {
		card = colPtr[row];
		if ((card < 40) && (lowerCard < 40)) {
			if ((card >= 27) || (lowerCard >= 27)) {
				return row + 1; // Non-rank cards are not a valid sequence
			}
			if (card / 9 == lowerCard / 9) {
				return row + 1; // Matching suits are not a valid sequence
			}
			if ((card % 9) != (lowerCard % 9) + 1) {
				return row + 1;
			}
		}
		--row;
		lowerCard = card;
	}
	return 0;
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


