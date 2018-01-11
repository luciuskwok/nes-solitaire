// main.c

/* Build commands:

/Applications/Emulators/cc65-master/bin/cl65 -O -t nes -C nes.cfg main.c screen.c data.s util.s -o test.nes
open test.nes

*/

#include <nes.h>
//#include <peekpoke.h>
#include "screen.h"

// Constants
#define JoyUp (0x08)
#define JoyDown (0x04)
#define JoyLeft (0x02)
#define JoyRight (0x01)
#define JoyStart (0x10)
#define JoySelect (0x20)
#define JoyButtonB (0x40)
#define JoyButtonA (0x80)


// Global variables
unsigned char deck[40];

// Function prototypes
unsigned char joypadStatus(void);
void shuffleDeck(void);
unsigned char pseudorandom(void);

// == main() ==
void main (void) {
	unsigned char x = 0x7F;
	unsigned char y= 0x77;
	unsigned char joypad = 0;
	
	initScreen();
	
	while (1) {
		waitvsync();
		refreshOAM();
		moveSpriteTo(x, y);
		
		// Update joypad and move sprite position. Actual move will happen after next VBL.
		joypad = joypadStatus();
		if ((joypad & JoyLeft) != 0) {
			--x;
		}
		if ((joypad & JoyRight) != 0) {
			++x;
		}
		if ((joypad & JoyUp) != 0) {
			--y;
		}
		if ((joypad & JoyDown) != 0) {
			++y;
		}
	}
}

// == joypadStatus() ==
unsigned char joypadStatus(void) {
	unsigned char result = 0;
	unsigned char i;
	JOYPAD[0] = 1; // Set strobe bit. The joypad buttons will be continuously reloaded, and only the Button A result will be returned.
	JOYPAD[0] = 0; // Clear strobe bit. This allows all 8 buttons to be read.
	for (i=0; i<8; ++i) {
		result = result << 1;
		result = result | (JOYPAD[0] & 0x01); // Set bit 0 of result to bit 0 of joypad
	}
	return result;
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

unsigned char pseudorandom(void) {
	return 1;
}