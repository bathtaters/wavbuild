/*
SIMPLE AUDIO GENERATORS Header
(Use with 'BUILD WAVE FILE')
*/
#ifndef AUDIO_GENERATE_H
#define AUDIO_GENERATE_H

#include <stdlib.h>
#include <math.h>
#include "wav_universal.h"

float silenceGenerator(double second, GenSetting none);
float sineGenerator(double second, GenSetting frequency);
float triangleGenerator(double second, GenSetting frequency);
float sawtoothGenerator(double second, GenSetting frequency);
float squareGenerator(double second, GenSetting frequency);

#endif