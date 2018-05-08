/*
LTC AUDIO GENERATOR Header
(Use with 'BUILD WAVE FILE')
*/

#ifndef LTC_GENERATE_H
#define LTC_GENERATE_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "timecode.h"

#define SYNCWORD		 0xBFFC 	// Little-endian

typedef struct LtcFrame {
	uint8_t bytes[8];
} LtcFrame;

float * ltcChannel(Timecode * tc, int frameCount, double maxValue, int sampleRate);

#endif