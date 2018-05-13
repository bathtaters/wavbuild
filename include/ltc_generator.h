/*
LTC AUDIO GENERATOR Header
(Use with 'BUILD WAVE FILE')
*/

#ifndef LTC_GENERATE_H
#define LTC_GENERATE_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "wav_universal.h"

#define SYNCWORD 0xBFFC
#define LTC_START_VAL 1.0

typedef struct LtcFrame {
	uint8_t bytes[10];
} LtcFrame;

float ltcGenerator(double second, GenSetting in);

#endif