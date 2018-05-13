/*
GENERIC TIMECODE Functions Header
*/

#include "timecode.h"

// Globals
const int TC_FPS_VALID[TC_FPS_LEN] = TC_FPS_ARR;
const int TC_PULLED[TC_PULLED_LEN] = TC_PULLED_ARR;
const int TC_PULLDOWN_VALID[TC_PULLDOWN_LEN] = TC_PULLDOWN_ARR;
const int TC_DF_VALID[TC_DF_LEN] = TC_DF_ARR;



// TIME FUNCTIONS
uint8_t * getTCarr(Timecode * tc) {
	uint8_t * outTC = malloc(4 * sizeof(uint8_t));
	int fcount = tc->frames;
	int convert[4] = TC_CONV_ARR(tc->frameRate);
	if(tc->isDropFrame) {
		if(tc->frameRate == 30) {
			fcount += (DF29_DPM * (DF29_MSKIP - 1)) * (fcount / DF29_FPM10) + 
						DF29_DPM * ((fcount % DF29_FPM10) / DF29_FPM);
		}
		// else DF behavior is not defined
	}
	for(int t = 3; t >= 0; t--) {
		outTC[t] = (uint8_t)(fcount % convert[t]);
		fcount /= convert[t];
	}
	// if(fcount) dayCarry = TRUE;
	return outTC;
}

int getTCarrFrames(int * TCarr, unsigned char frameRate, int isDrop) {
	int convert[4] = TC_CONV_ARR(frameRate);
	int frames = 0;
	// if(TCarr[0] / convert[0]) dayCarry = TRUE;
	TCarr[0] = TCarr[0] % convert[0];
	for(int t = 0; t < 4; t++) {
		frames *= convert[t];
		frames += TCarr[t];
    }
    if(isDrop) {
    	if(frameRate == 30) {
			int minutes = frames / (convert[3] * convert[2]);
			frames -= DF29_DPM * (minutes - (minutes / DF29_MSKIP));
		}
		// else DF behavior is not defined
    }
    return frames;
}

int getTCframes(int hh, int mm, int ss, int ff, unsigned char frameRate, int isDrop) {
	int tcArr[4] = {hh, mm, ss, ff};
	int f = getTCarrFrames((int *)tcArr, frameRate, isDrop);
    return f;
}

int setAsCurrentTime(Timecode * tc) {
	int f;
	time_t tvar = time(NULL);
	struct tm currtime = *localtime(&tvar);
	
	f = getTCframes(
		currtime.tm_hour,
		currtime.tm_min,
		currtime.tm_sec,
		0,
		tc->frameRate, tc->isDropFrame
	);
	if(f < 0) { return f; }
	tc->frames = f;
	tc->tcClockFmt = tc_clocktime;
	return 0;
}


// USER-BIT FUNCTIONS
int decAsHex(int num) {
	// Output hexadecimal with same digits as decimal
	char decStr[INT_MAX_DIGITS];
	sprintf(decStr,"%d",num);
	return (int)strtol(&decStr[0],NULL,16);
}
	

int setUserBits(Timecode * tc, int ub0, int ub1, int ub2, int ub3) {
	// Invalid number
	if(ub0 < CHAR_MIN || ub0 > UCHAR_MAX) { return 10; }
	if(ub1 < CHAR_MIN || ub1 > UCHAR_MAX) { return 11; }
	if(ub2 < CHAR_MIN || ub2 > UCHAR_MAX) { return 12; }
	if(ub3 < CHAR_MIN || ub3 > UCHAR_MAX) { return 13; }
	
	// Set 'em
	tc->userBits[0] = (unsigned char)ub0;
	tc->userBits[1] = (unsigned char)ub1;
	tc->userBits[2] = (unsigned char)ub2;
	tc->userBits[3] = (unsigned char)ub3;
	
	return 0;
}

int setUserBitsArr(Timecode * tc, int * ubArr) {
	return setUserBits(tc, ubArr[0], ubArr[1], ubArr[2], ubArr[3]);
}

int setAsCurrentDate(Timecode * tc) {
	time_t tvar = time(NULL);
	struct tm currtime = *localtime(&tvar);
	
	setUserBits(tc,
		decAsHex(currtime.tm_mon + 1),
		decAsHex(currtime.tm_mday),
		decAsHex((currtime.tm_year) % 100),
		currtime.tm_isdst
	);
	tc->userBitsFmt = ub_date;
	return 0;
}


// TIMECODE Functions

char * getTCstr(Timecode * tc) {
	char * tstr = malloc(TC_STR_LEN * sizeof(char));
	uint8_t *t = getTCarr(tc);
	unsigned char *u = &tc->userBits[0];
	
	sprintf(tstr,
		"%02d:%02d:%02d:%02d (%02X %02X %02X %02X) @ %.2f%s fps",
		*t,*(t+1),*(t+2),*(t+3),
		*u,*(u+1),*(u+2),*(u+3),
		(float)tc->frameRate / (tc->isPullDown ? 1.001 : 1.0),
		tc->isDropFrame ? "DF" : ""
	);
	
	free(t);
	return tstr;
}

Timecode * getTCdefault(void) {
	Timecode * tc = malloc(sizeof(Timecode));
	tc->frames 		= DEF_FRAMECOUNT;
	tc->frameRate   = DEF_FPS;
	tc->isPullDown  = DEF_PULLDWN;
	tc->isDropFrame = DEF_DROPFRM;
	tc->tcClockFmt  = DEF_TC_FMT;
	tc->userBitsFmt = DEF_UB_FMT;
	setUserBits(tc, DEF_UB);
	return tc;
}