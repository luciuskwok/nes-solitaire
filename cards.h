// cards.h

#ifndef CARDS_H
#define CARDS_H

// Globals
extern unsigned char attributeTableNeedsUpdate;
extern unsigned char deck[];
extern unsigned char cardsBeingMoved[];
extern unsigned char columnCard[];
extern unsigned char freecellCard[];
extern unsigned char foundationCard[]; // There are only 3 foundations, but having 4 allocated makes some programming easier.
extern unsigned char originatingCellX, originatingCellY; // used when moving cards
extern unsigned char invalidCell[]; // for marking where to redraw cards
extern unsigned char invalidCellCount;

// Functions
void shuffleDeck(void);

void invalidateCell (unsigned char col, unsigned char row);
void drawInvalidCells(void);

unsigned int locationWithCell(unsigned char x, unsigned char y);
unsigned char columnRowAtCursor(unsigned char curX, unsigned char curY);
void pickUpCardsAtCursor(unsigned char curX, unsigned char curY);
void dropCardsAtCursor(unsigned char curX, unsigned char curY);
void returnCardsToOrigin(void);
void clearCardsBeingMoved(void);
unsigned char numberOfCardsBeingMoved(void);
unsigned char columnHeight(unsigned char col);
unsigned char topMovableRowAtColumn(unsigned char col);


#endif
