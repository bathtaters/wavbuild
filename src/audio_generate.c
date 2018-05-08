/*
SIMPLE AUDIO GENERATORS
(Use with 'BUILD WAVE FILE')
*/

#include "audio_generate.h"

float * silence(int sampleCount) {
	return (float *)calloc(sampleCount, sizeof(float));
}

float * sine(int sampleCount, double maxValue, double sampleFreq) {
	float * audio = malloc(sampleCount * sizeof(float));
	int s, c; double value = 0.0, step = 0.0;
	if(maxValue && sampleFreq) {
		step = 2 * M_PI / sampleFreq; }
	
	for(s = 0; s < sampleCount; s++) {
		audio[s] = (float)(maxValue * sin((float)value));
		value += step;
		if(value > 2 * M_PI) {
			value -= 2 * M_PI; }
	}
	return audio;
}

float * triangle(int sampleCount, double maxValue, double sampleFreq) {
	float * audio = malloc(sampleCount * sizeof(float));
	int s, c; double value = 0.0, step = 0.0;
	if(maxValue && sampleFreq) {
		step = 4 * maxValue / sampleFreq; }
	
	for(s = 0; s < sampleCount; s++) {
		audio[s] = (float)value;
		value += step;
		if(value > maxValue || value < -maxValue) {
			value = (value > 0 ? 2 : -2) * maxValue - value;
			step = -step;
		}
	}
	return audio;
}

float * sawtooth(int sampleCount, double maxValue, double sampleFreq) {
	float * audio = malloc(sampleCount * sizeof(float));
	int s, c; double value = 0.0, step = 0.0;
	if(maxValue && sampleFreq) {
		step = 2 * maxValue / sampleFreq; }
	
	for(s = 0; s < sampleCount; s++) {
		audio[s] = (float)value;
		value += step;
		if(value > maxValue) {
			value -= 2 * maxValue; }
	}
	return audio;
}

float * square(int sampleCount, double maxValue, double sampleFreq) {
	float * audio = malloc(sampleCount * sizeof(float));
	int step = 1, s, c;
		
	audio[0] = 0;
	for(s = 1; s < sampleCount; s++) {
		audio[s] = (float)maxValue;
		if( s - 1 >= step * sampleFreq / 2.0 ) {
			maxValue = -maxValue;
			step++;
		}
	}
	return audio;
}

