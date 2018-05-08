/*
SIMPLE AUDIO GENERATORS Header
(Use with 'BUILD WAVE FILE')
*/
#ifndef AUDIO_GENERATE_H
#define AUDIO_GENERATE_H

#include <stdlib.h>
#include <math.h>

float * silence(int sampleCount);
float * sine(int sampleCount, double maxValue, double sampleFreq);
float * triangle(int sampleCount, double maxValue, double sampleFreq);
float * sawtooth(int sampleCount, double maxValue, double sampleFreq);
float * square(int sampleCount, double maxValue, double sampleFreq);

#endif