// cards.c

#include "cards.h"
#include "screen.h"
#include "util.h"
#include "constants.h"
#include <nes.h>

// Constants

// Globals
unsigned char deck[40];
unsigned char cardsBeingMoved[MaxColumnHeight];
unsigned char columnCard[8 * MaxColumnHeight];
unsigned char freecellCard[4];
unsigned char foundationCard[4]; // There are only 3 foundations, but having 4 allocated makes some programming easier.
unsigned char originatingCellX, originatingCellY; // used when moving cards
unsigned char autoMoveNextFrame = 0; 

// Function Prototypes
void autoMoveCardFromColumnToFoundation(unsigned char fromCol, unsigned char toFou);

void clearCardsBeingMoved(void);
unsigned char numberOfCardsBeingMoved(void);
unsigned char columnHeight(unsigned char col);
unsigned char topMovableRowAtColumn(unsigned char col);

unsigned char consolidateHonors(unsigned char moveCard, unsigned char curX, unsigned char curY);
unsigned char isMatchingHonor(unsigned char card1, unsigned char card2);
void animateCardFromOriginTo(unsigned char curX, unsigned char curY);

void drawColumnBottom(unsigned char col, unsigned char blankRows);
unsigned char columnRowAtCursor(unsigned char curX, unsigned char curY);
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


// == autoMoveCards() ==
void autoMoveCards(void) {
	unsigned char highLimit = 255;
	unsigned char searchCard, card;
	unsigned char col, colHeight, fou;

	// Find the lowest card in the foundations
	for (fou=0; fou<3; ++fou) {
		card = foundationCard[fou];
		if (card >= 40) {
			card =0;
		} else {
			card = card % 9;
		}
		highLimit = (card < highLimit)? card : highLimit;
	}
	++highLimit; // Set highLimit to one more than the lowest card found.
	
	// Compare the foundation card to each card in freecells and columns
	for (fou=0; fou<3; ++fou) {
		searchCard = foundationCard[fou];
		if (searchCard < 40) {
			if (searchCard % 9 != 8) { // For valid cards that are not the "9" card, set searchCard to the card we're looking for.
				++searchCard;
			}
		} else { // For empty foundations, look for the "1" card of the matching suit in order.
			searchCard = fou * 9;
		}
		
		if ((searchCard % 9) <= highLimit) {
			// Check the freecells
			for (col=0; col<3; ++col) {
				card = freecellCard[col];
				if (card < 40) {
					if (card == FlowerCard) {
						autoMoveCardFromColumnToFoundation(col + 8, 3);
						autoMoveNextFrame = 1;
					} else if (card == searchCard) {
						autoMoveCardFromColumnToFoundation(col + 8, fou);
						autoMoveNextFrame = 1;
					}
				}
			}
		
			// Check the tableau columns
			for (col=0; col<8; ++col) {
				colHeight = columnHeight(col);
				if (colHeight > 0) {
					card = columnCard[col * MaxColumnHeight + colHeight - 1];
					if (card == FlowerCard) {
						autoMoveCardFromColumnToFoundation(col, 3);
						autoMoveNextFrame = 1;
					} else if (card == searchCard) {
						autoMoveCardFromColumnToFoundation(col, fou);
						autoMoveNextFrame = 1;
					}
				}
			}
			
		} // end if (searchCard...)
		
	} // end for (fou...)
}


// == autoMoveCardToFoundation() ==
// fromCol: 0 to 7 for tableau columns, 8 to 10 for freecells
// toFou: 0 to 2 for foundations, and 3 for flower card
void autoMoveCardFromColumnToFoundation(unsigned char fromCol, unsigned char toFou) {
	unsigned char moveCard;
	unsigned char colHeight, index;
	
	if (fromCol < 8) {
		colHeight = columnHeight(fromCol);
		index = fromCol * MaxColumnHeight + colHeight - 1;
		moveCard = columnCard[index];
		columnCard[index] = 255;
		drawColumnBottom(fromCol, 1);
		originatingCellX = fromCol + 1;
		originatingCellY = colHeight + 1;
	} else {
		index = fromCol - 8;
		moveCard = freecellCard[index];
		freecellCard[index] = 255;
		originatingCellX = index + 1;
		originatingCellY = 1;
		drawCardAtCell(255, originatingCellX, originatingCellY); // draw placeholder
	}
	
	// Redraw screen to remove card being moved
	refreshScreen();

	// Animate card being moved
	cardsBeingMoved[0] = moveCard;
	cardsBeingMoved[1] = 255;
	setCardSprite(cardsBeingMoved, 0, 0);
	animateCardFromOriginTo(toFou + 6, 1);
	cardsBeingMoved[0] = 255;
	setCardSprite(0, 0, 0);

	// Place card at destination after animation finishes. 
	foundationCard[toFou] = moveCard;
	drawCardAtCell (moveCard, toFou + 6, 1);
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
			drawCardAtCell(255, curX, curY); // draw placeholder
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
				}
				originatingCellX = curX; // allows drops back at original location to cancel a move
				originatingCellY = row + 2;
				cardLocation = locationWithCell(originatingCellX, originatingCellY);
			
				drawColumnBottom (col, colHeight - row);
				validCard = 1;
			}
		}
		
	}
	
	if (validCard) {
		//beep(Note_A4);
		
		// Update the card sprite with the card being moved.
		setCardSprite(cardsBeingMoved, cardLocation & 0xFF, cardLocation >> 8);
		resetScrollPosition();
	} else {
		// No valid card to select
		beep(Note_A3);
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
	unsigned char destinationX, destinationY;
	
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
				bottomCard = freecellCard[col];
				if (bottomCard > 40) { 
					// Always allow dropping on an empty freecell
					validMove = 1;
					freecellCard[col] = moveCard;
				} else if ((27 <= bottomCard) && (bottomCard < 39) && (27 <= moveCard) && (moveCard < 39)) { 
					// Handle dropping an Honor card onto another Honor card.
					if ((bottomCard - 27) / 4 == (moveCard - 27) / 4) { // same color.
						validMove = consolidateHonors(moveCard, curX, curY); 
						moveCard = FaceDownCard; // draw face-down card at destination cell
					}
				}
			} else if (col >= 5) {
				bottomCard = foundationCard[col-5];
				if (moveCard < 27) {	 // Only rank cards may move to foundation
					if (bottomCard < 40) {
						if ((moveCard / 9 == bottomCard / 9) && (moveCard == bottomCard + 1)) {
							validMove = 1; // Only cards in ascending order can go on foundation.
						}
					} else if (moveCard % 9 == 0) { 
						validMove = 1;// Only rank 1 cards can move to empty foundation.
					}
				}
				if (validMove) {
					foundationCard[col-5] = moveCard;
				}
			} else { 
				// Cannot move to space between cells and foundations.
				// The flower card should move to its place automatically.
				validMove = 0;
			}
		}
		if (validMove) {
			cardsBeingMoved[0] = 255;
			destinationX = curX;
			destinationY = curY;
			drawCardAtCell (moveCard, curX, curY);
		}
	} else {
		// Dropping cards anywhere in a column will be interpreted as dropping them on the bottom-most card.
		height = columnHeight(col);
		if (height == 0) {
			validMove = 1; // Can always drop a card in an empty column
		} else {
			// Check if cards can be placed on the bottom card of the column.
			bottomCard = columnCard[col * MaxColumnHeight + height - 1];
			if ((bottomCard < 27) && (moveCard < 27) && (bottomCard / 9 != moveCard / 9) && ((bottomCard % 9) == (moveCard % 9) + 1) && (height + moveCount <= MaxColumnHeight) ) { 
				// Only match rank cards of different suits where the bottom card is one more than the moved card. Also, make sure that the move does not cause a column to exceed its max height.
				validMove = 1;
			}
		}
		if (validMove) { // Move cards to column
			for (i=0; i<moveCount; ++i) {
				moveCard = cardsBeingMoved[i];
				columnCard[col * MaxColumnHeight + height + i] = moveCard;
				cardsBeingMoved[i] = 255;
				drawCardAtCell(moveCard, col + 1, height + i + 2);
			}
			destinationX = col + 1;
			destinationY = height + 2;
		}
	}
	
	if (validMove) { // Animate card sprite
		if (originatingCellX > 0) {
			animateCardFromOriginTo(destinationX, destinationY);
		}
		setCardSprite(0, 0, 0); // then remove the sprite
		autoMoveNextFrame = 1; // Always auto-move cards after user makes a valid move.
	} else {
		beep(Note_A3);
	}
}

// == consolidateHonors() ==
// Check if all the honor cards are available, except for the card being moved, which has already been taken off the screen. Then move the cards one by one to the foundation cell, except for the last one, which will be moved by the calling function.
unsigned char consolidateHonors(unsigned char moveCard, unsigned char curX, unsigned char curY) {
	unsigned char foundCount = 0;
	unsigned char originX[4]; // coordinates for moving the other 3 cards
	unsigned char originY[4];
	unsigned char col, colHeight, x, y;
	
	// Save originatingCell coordinate to restore for last animation
	originX[3] = originatingCellX;
	originY[3] = originatingCellY;
	
	// Freecells
	for (col=0; col<3; ++col) {
		if (isMatchingHonor(moveCard, freecellCard[col])) {
			originX[foundCount] = col + 1;
			originY[foundCount] = 1;
			++foundCount;
		}
	}
	
	// Columns
	for (col=0; col<8; ++col) {
		colHeight = columnHeight(col);
		if (colHeight > 0) {
			if (isMatchingHonor(moveCard, columnCard[col * MaxColumnHeight + colHeight - 1])) {
				originX[foundCount] = col + 1;
				originY[foundCount] = colHeight + 1;
				++foundCount;
			}
		}
	}
	
	debugValue1 = foundCount;
	
	// return if card count isn't right
	if (foundCount < 3) {
		return 0;
	}
	
	// Animate selected card
	animateCardFromOriginTo(curX, curY);
	
	// Animate other 3 cards (skipping the card already at cursor)
	for (col=0; col<3; ++col) {
		x = originX[col];
		y = originY[col];
		if ((x != curX) || (y != curY)) {
			// Remove card from screen
			if (y == 1) { // Freecell
				freecellCard[x - 1] = 255;
				drawCardAtCell(255, x, y);
			} else { // Column
				columnCard[(x - 1) * MaxColumnHeight + (y - 2)] = 255;
				drawColumnBottom(x-1, 1);
			}
			refreshScreen();
			
			originatingCellX = x;
			originatingCellY = y;
			animateCardFromOriginTo(curX, curY);
		}
	}
	
	// Set the freecell to a face-down card
	freecellCard[curX - 1] = FaceDownCard;
	
	// Disable card-move animation on return
	originatingCellX = 0;
	
	return 1; // is a valid move
}

unsigned char isMatchingHonor(unsigned char card1, unsigned char card2) {
	if ((27 <= card1) && (card2 < 39) && (27 <= card2) && (card2 < 39)) {
		card1 = (card1 - 27) / 4;
		card2 = (card2 - 27) / 4;
		return (card1 == card2)? 1 : 0;
	}
	return 0;
}

// == returnCardsToOrigin() ==
void returnCardsToOrigin(void) {
	unsigned char col = originatingCellX - 1;
	unsigned char row = originatingCellY - 2;
	unsigned char i, moveCard;
	
	if (cardsBeingMoved[0] >= 40) {
		return;
	}

	// originatingCellX and Y use 1,1 as the top left coordinate.
	if (originatingCellY == 1) {
		// Only the 3 freecells are valid origins
		if (originatingCellX <= 3) {
			// Should be only one card being moved.
			moveCard = cardsBeingMoved[0];
			freecellCard[originatingCellX - 1] = moveCard;
			cardsBeingMoved[0] = 255;
			drawCardAtCell (moveCard, originatingCellX, originatingCellY);
		}
	} else if (originatingCellY > 1) {
		i = 0;
		while (cardsBeingMoved[i] < 40 && i < MaxColumnHeight) {
			moveCard = cardsBeingMoved[i];
			columnCard[col * MaxColumnHeight + row + i] = moveCard;
			cardsBeingMoved[i] = 255;
			drawCardAtCell (moveCard, originatingCellX, originatingCellY + i);
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


// == animateCardFromOriginTo() ==
void animateCardFromOriginTo(unsigned char curX, unsigned char curY) {
	unsigned int startLocation = locationWithCell(originatingCellX, originatingCellY);
	unsigned int endLocation = locationWithCell(curX, curY);
	unsigned char startX = (startLocation & 0xFF);
	unsigned char startY = (startLocation >> 8);
	unsigned char endX = (endLocation & 0xFF);
	unsigned char endY = (endLocation >> 8);
	
	//beep(Note_A4);
	animateCardSprite (startX, startY, endX, endY, 8); // duration=60 for testing, 8 for production.
}

// == drawCardAtCell() ==
void drawCardAtCell(unsigned char card, unsigned char row, unsigned char col) {
	unsigned int location = locationWithCell(row, col);
	unsigned char x = (location & 0x00FF) / 8;
	unsigned char y = (location & 0xFF00) >> 11;
	
	drawHexByte (x, 0, 29);
	drawHexByte (y, 3, 29);
	
	if (card <= 40) {
		drawCard (card, x, y);
	} else {
		if (row == 1) {
			drawPlaceholder (x, y);
		} else {
			eraseHalfCardArea (x, y);
			eraseHalfCardArea (x, y + 2);
		}
	}
}

// == drawColumnBottom() ==
void drawColumnBottom(unsigned char col, unsigned char blankRows) {
	unsigned char colHeight = columnHeight(col);
	unsigned char cellX = col + 1;
	unsigned char cellY = colHeight + 1;
	unsigned char card = 255;
	unsigned int location = locationWithCell(cellX, cellY);
	unsigned char tileX = (location & 0x00FF) / 8;
	unsigned char tileY = (location & 0xFF00) >> 11;
	unsigned char i;
	
	if (colHeight > 0) {
		card = columnCard[col * MaxColumnHeight + colHeight - 1];
	}
	drawCardAtCell (card, cellX, cellY);

	tileY += 4;
	for (i=0; i<blankRows; ++i) {
		eraseHalfCardArea(tileX, tileY);
		tileY += 2;
	}
}

// == locationWithCell() ==
unsigned int locationWithCell(unsigned char x, unsigned char y) {
	if (y > 1) {
		++y;
	}
	if (x == 9 && y == 1) {
		x = 120;
	} else {
		x = x * 24 + 8;
	}
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


