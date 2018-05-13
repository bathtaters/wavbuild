/*
SIMPLE AUDIO GENERATORS
(Use with 'BUILD WAVE FILE')
*/

#include "audio_generators.h"

float silenceGenerator(double second, GenSetting none) {
	return 0.0;
}

float sineGenerator(double second, GenSetting freq) {
	double value = 2 * M_PI * fmod(second * freq.dec, 1.0);
	return (float)sin((float)value);
}

float triangleGenerator(double second, GenSetting freq) {
	double value = 4 * second * freq.dec;
	int offset = 2 * ((int)ceil((value + 1)/2) - 1);
	if(offset % 4) { value = -value; }
	else { offset = -offset; }
	return (float)(value + (double)offset);
}

float sawtoothGenerator(double second, GenSetting freq) {
	double value = 2 * second * freq.dec;
	double offset = -2 * ((int)ceil((value + 1)/2) - 1);
	return value + offset;
}

float squareGenerator(double second, GenSetting freq) {
	return (int)floor(2 * second * freq.dec) % 2 ? -1.0 : 1.0;
}		
