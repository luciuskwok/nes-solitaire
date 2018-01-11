// screen.h

#ifndef SCREEN_H
#define SCREEN_H

// Function prototypes
void initScreen(void);
void setScreenVisible(unsigned char on);
void resetScrollPosition(void);
void refreshOAM(void);

void setColorAttribute(unsigned char color, unsigned char x, unsigned char y);
void refreshAttributeTable(void);

void moveSpriteTo(unsigned char x, unsigned char y);

void drawCard (unsigned char card, unsigned char x, unsigned char y);
void drawPlaceholder(unsigned char x, unsigned char y);
void drawPlaceholderRow(void);

void drawString(const char *string, unsigned char x, unsigned char y);
void stringWithByte(unsigned char byte, char outString[]);


#endif