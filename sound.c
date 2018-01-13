// sound.c

#include "sound.h"
#include <nes.h>


// == playPulse() ==
void playPulse(unsigned int period, unsigned char len) {
	// time = ( CPU_clock / (16 * frequency) ) - 1
	// time = ( 1789773 / (16 * 1000) ) - 1
	// time = 110
	APU.pulse[0].control = 0x87;
	APU.pulse[0].ramp = 0x00;
	APU.pulse[0].period_low = period;
	APU.pulse[0].len_period_high = (len << 4) | ((period >> 8) & 0x07);
}

// == playTriangle() ==
void playTriangle(unsigned int period, unsigned char len) {
	APU.triangle.counter = 0x01;
	APU.triangle.period_low = period;
	APU.triangle.len_period_high = (len << 4) | ((period >> 8) & 0x07);
}

// == playNoise() == 
void playNoise(unsigned char period, unsigned char len) {
	APU.noise.control = 0x07;
	APU.noise.period = period & 0x0F;
	APU.noise.len = len << 4;
}


