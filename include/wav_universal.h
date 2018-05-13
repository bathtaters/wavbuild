/* Universal Constants */

#ifndef WAV_UNIVERSAL_H
#define WAV_UNIVERSAL_H

#include "timecode.h"

typedef enum WaveGens {
	GEN_SILENCE = 0,
	GEN_SINE = 1,
	GEN_TRIANGLE = 2,
	GEN_SAWTOOTH = 3,
	GEN_SQUARE = 4,
	GEN_TIMECODE = 5,
	GEN_HEADER = 0xFFFF
} WaveGens;
#define GEN_FIRST GEN_SILENCE
#define GEN_LAST  GEN_TIMECODE
#define GEN_FUNCT_ARR {\
	&silenceGenerator,\
	&sineGenerator,\
	&triangleGenerator,\
	&sawtoothGenerator,\
	&squareGenerator,\
	&ltcGenerator\
}
#define GEN_NAMES_ARR {\
	"Silence",\
	"Sine Wave",\
	"Triangle",\
	"Sawtooth",\
	"Square",\
	"Timecode",\
	"Header Only"\
}
#define GEN_FREQ_LEN 4
#define GEN_FREQ_SET { GEN_SINE, GEN_TRIANGLE, GEN_SAWTOOTH, GEN_SQUARE }
#define GEN_TC_LEN 	1
#define GEN_TC_SET	{ GEN_TIMECODE }

typedef union GenSetting {
	int num;
	double dec;
	Timecode * tc;
} GenSetting;

typedef float (*Generator)(double, GenSetting);

#endif