// cards.h

#ifndef CARDS_H
#define CARDS_H

// Globals
extern unsigned char deck[];
extern unsigned char cardsBeingMoved[];
extern unsigned char columnCard[];
extern unsigned char freecellCard[];
extern unsigned char foundationCard[]; // There are only 3 foundations, but having 4 allocated makes some programming easier.
extern unsigned char originatingCellX, originatingCellY; // used when moving cards
extern unsigned char autoMoveNextFrame;

// Functions
void shuffleDeck(void);
unsigned char isGameOver(void);
void autoMoveCards(void);

void pickUpCardsAtCursor(unsigned char curX, unsigned char curY);
void dropCardsAtCursor(unsigned char curX, unsigned char curY);
void returnCardsToOrigin(void);

void drawCardAtCell(unsigned char card, unsigned char col, unsigned char row);
unsigned int locationWithCell(unsigned char x, unsigned char y);
void drawAllColumns(void);

#endif
