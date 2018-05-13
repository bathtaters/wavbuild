/*
LTC AUDIO GENERATOR
(Use with 'BUILD WAVE FILE')
*/

#include "ltc_generator.h"


// HELPER FUNCITONS

// BIT FUNCTIONS

uint8_t getBit(uint8_t * bytes, int offset) {
	return (bytes[offset/8] >> (offset%8)) & 1; 
}

void setBit(uint8_t * bytes, int offset, uint8_t value) {
	if(value) { bytes[offset/8] = bytes[offset/8] |  (1 << (offset%8)); }
	else      { bytes[offset/8] = bytes[offset/8] & ~(1 << (offset%8)); }
}
uint8_t onesEParity(LtcFrame data, int bitCount) {
	uint8_t isOdd = 0;

	for(int b = 0; b < bitCount / 8; b++) {
		if(bitCount < (b+1) * 8)
			{ data.bytes[b] &= (0xFF >> (8 - (bitCount % 8))); }
		while(data.bytes[b] != 0) {
		  data.bytes[b] &= data.bytes[b] - 1;
		  isOdd = !isOdd;
		}
	}
	return isOdd;
}


// LTC FUNCTIONS

LtcFrame getLtc(Timecode * inTC) {
	uint8_t * tcArr = getTCarr(inTC);
	LtcFrame data;
	
	// TC & User bits
	for(int b = 0; b < 4; b++) {
		data.bytes[b*2]  =  ((tcArr[3-b]%10) & 0xF) | ((inTC->userBits[3-b] << 4) & 0xF0);
		data.bytes[b*2+1] = ((tcArr[3-b]/10) & 0xF) |  (inTC->userBits[3-b]       & 0xF0);
	}
	free(tcArr);
	
	// Sync-word
	data.bytes[8] = (uint8_t)( SYNCWORD       & 0xFF);
	data.bytes[9] = (uint8_t)((SYNCWORD >> 8) & 0xFF);
	
	// Standard bits
	setBit(data.bytes, 10, inTC->isDropFrame);	// DropFrame
	setBit(data.bytes, 11, 0); 					// Color-framing bit
	setBit(data.bytes, 58, inTC->tcClockFmt); 	// BGF1
	
	// FPS-specific bits (Includes even-parity bit)
	setBit(data.bytes, inTC->frameRate == 25 ? 27 : 43, inTC->userBitsFmt & 1);	// BGF0
	setBit(data.bytes, inTC->frameRate == 25 ? 43 : 59, inTC->userBitsFmt & 2);	// BGF2
	setBit(data.bytes, inTC->frameRate == 25 ? 59 : 27, onesEParity(data, 64));	// Even-parity
	
	return data;
}






// MAIN FUNCTION
float ltcGenerator(double second, GenSetting in) {
	static int fps;
	static double bps;
	static LtcFrame currentFrame;
	static double frameBound, bitBound, flipBound;
	static float value = -LTC_START_VAL;
	
	if(second == 0.0) {
		// Initialize statics
		fps = in.tc->frameRate;
		if(fps > 30) { fps /= 2; }
		bps = 80.0 * fps;
		flipBound = 0.5 / bps;
	}
	if(second >= frameBound) {
		currentFrame = getLtc(in.tc);
		in.tc->frames++;
		while(frameBound <= second)
			{ frameBound += 1.0 / fps; }
	}
	if(second >= bitBound) {
		value = -value;
		while(bitBound <= second)
			{ bitBound += 1.0 / bps; }
	}
	if(second >= flipBound) {
		if(getBit(currentFrame.bytes, floor((second -  floor(second * fps) / fps) * bps)))
			{ value = -value; }
		while(flipBound <= second)
			{ flipBound += 1.0 / bps; }
	}
	
	return value;
}