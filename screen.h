// screen.h

#ifndef SCREEN_H
#define SCREEN_H

// Globals
extern unsigned char attributeTableNeedsUpdate;
extern unsigned char debugValue1, debugValue2;

// Function prototypes
void initScreen(void);
void refreshScreen(void);
void addVramUpdate(unsigned int address, unsigned char length, const unsigned char *data);

void setScreenVisible(unsigned char on);
void resetScrollPosition(void);
void refreshOAM(void);

void setColorAttribute(unsigned char color, unsigned char x, unsigned char y);

void movePointerTo(unsigned char x, unsigned char y);
void setCardSprite(unsigned char *cards, unsigned char x, unsigned char y);
void animateCardSprite(unsigned char fromX, unsigned char fromY, unsigned char toX, unsigned char toY, unsigned char duration);
unsigned char getCardTilesAndColor (unsigned char card, unsigned char tiles[12]);

void drawCard (unsigned char card, unsigned char x, unsigned char y);
void drawPlaceholder(unsigned char x, unsigned char y);
void eraseHalfCardArea (unsigned char x, unsigned char y);

void updateScreenForNewGame(void);

void drawString(const char *string, unsigned char x, unsigned char y);
void stringWithByte(unsigned char byte, char outString[]);
void drawHexByte (unsigned char byte, unsigned char x, unsigned char y);

#endif