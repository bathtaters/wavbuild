/*
LTC AUDIO GENERATOR
(Use with 'BUILD WAVE FILE')
*/

#include "ltc_generate.h"


// HELPER FUNCITONS

// BIT FUNCTIONS

uint8_t getBit(uint8_t * bytes, int offset) {
	return (bytes[offset/8] >> (offset%8)) & 1; 
}

void setBit(uint8_t * bytes, int offset, uint8_t value) {
	if(value) { bytes[offset/8] = bytes[offset/8] |  (1 << (offset%8)); }
	else      { bytes[offset/8] = bytes[offset/8] & ~(1 << (offset%8)); }
}
uint8_t evenOnes(LtcFrame data) {
	// Is binary 1s count even?
	uint8_t isEven = 1;

	for(int b = 0; b < 8; b++) {
		while(data.bytes[b] != 0) {
		  data.bytes[b] &= data.bytes[b] - 1;
		  isEven = !isEven;
		}
	}
	return isEven;
}


// LTC FUNCTIONS

LtcFrame getLtc(Timecode * inFrame) {
	uint8_t * tc = getTCarr(inFrame);
	LtcFrame data;
	uint8_t * bptr = data.bytes;
	
	// TC & User bits
	for(int b = 0; b < 4; b++) {
		data.bytes[b*2]  =  ((tc[3-b]%10) & 0xF) | ((inFrame->userBits[3-b] << 4) & 0xF0);
		data.bytes[b*2+1] = ((tc[3-b]/10) & 0xF) |  (inFrame->userBits[3-b] & 0xF0);
	}
	free(tc);
	
	// Standard bits
	setBit(bptr, 10, inFrame->isDropFrame); 	// DropFrame
	setBit(bptr, 11, 0); 					// Color-framing bit
	setBit(bptr, 58, inFrame->tcClockFmt); 	// BGF1
	
	// FPS-specific bits (Includes even-parity bit)
	setBit(bptr, inFrame->frameRate == 25 ? 27 : 43, inFrame->userBitsFmt & 1);	// BGF0
	setBit(bptr, inFrame->frameRate == 25 ? 43 : 59, inFrame->userBitsFmt & 2);	// BGF2
	setBit(bptr, inFrame->frameRate == 25 ? 59 : 27, evenOnes(data));			// Even-parity
	
	return data;
}

uint8_t * getLtcData(Timecode * tc) {
	LtcFrame ltc = getLtc(tc);
	uint8_t * data = malloc(10);
	memcpy((void *)data, (void *)ltc.bytes, 8);
	data[8] = SYNCWORD & 0xFF;
	data[9] = (SYNCWORD >> 8) & 0xFF;
	return data;
}






// AUDIO DATA FUNCTIONS

float * FMbytes(int byteCount, float maxValue, double sampleFreq, uint8_t * bytes) {
	int s = 0, c, b;
	int sampleCount = (int)round(byteCount * 8 * sampleFreq + 0.5);
	float *audio = malloc(sampleCount * sizeof(float));
	
	for(b = 0; b < byteCount * 8; b++) {
		for( ; s - (int)round(sampleFreq) < (int)round(b * sampleFreq); s++) {
			if(s == (int)round((b + 0.5) * sampleFreq) && getBit(bytes,b)) {
				maxValue = -maxValue; }
			audio[s] = maxValue;
		}
		maxValue = -maxValue;
	}
	
	for( ; s < sampleCount; s++) {
		audio[s] = 0; } // Zero-out remaining samples
	
	return audio;
}

// MAIN FUNCTION

float * ltcChannel(Timecode * tc, int frameCount, double maxValue, int sampleRate) {
	uint8_t *nextFrame, *ltcBytes = malloc(frameCount * 10);
	
	for(int f=0; f < frameCount; f++, tc->frames++) {
		nextFrame = getLtcData(tc);
		memcpy(ltcBytes + f*10, nextFrame, 10);
		free(nextFrame);
	}
	
	double sampleFreq = (double)sampleRate / (80.0 * tc->frameRate);
	if(tc->frameRate > 30) { sampleFreq *= 2.0; }
	float * audio = FMbytes(frameCount * 10, (float)maxValue, sampleFreq, ltcBytes);
	
	free(ltcBytes);
	return audio;
}
