// screen.h

#ifndef SCREEN_H
#define SCREEN_H

// Globals
extern unsigned char attributeTableNeedsUpdate;

// Function prototypes
void initScreen(void);
void setScreenVisible(unsigned char on);
void resetScrollPosition(void);
void refreshOAM(void);

void setColorAttribute(unsigned char color, unsigned char x, unsigned char y);
void refreshAttributeTable(void);

void movePointerTo(unsigned char x, unsigned char y);
void setCardSprite(unsigned char *cards, unsigned char x, unsigned char y);
unsigned char getCardTilesAndColor (unsigned char card, unsigned char tiles[12]);

void drawCard (unsigned char card, unsigned char x, unsigned char y);
void drawPlaceholder(unsigned char x, unsigned char y);
void drawPlaceholderRow(void);
void eraseRect (unsigned char x, unsigned char y, unsigned char width, unsigned char height);

void drawString(const char *string, unsigned char x, unsigned char y);
void stringWithByte(unsigned char byte, char outString[]);
void drawHexByte (unsigned char byte, unsigned char x, unsigned char y);

#endif