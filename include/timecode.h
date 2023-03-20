/*
GENERIC TIMECODE Functions Header
*/

#ifndef TIMECODE_H
#define TIMECODE_H

#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// Constants
#define MAX_HRS 			24
#define MIN_TO_SEC 			60
#define TC_CONV_ARR(fps) 	{ MAX_HRS,  MIN_TO_SEC,  MIN_TO_SEC,  fps }
#define MAX_FRAMES(fps)  	( MAX_HRS * MIN_TO_SEC * MIN_TO_SEC * fps )
#define MAX_SEC 		 	MAX_FRAMES(1)
#define TC_STR_LEN 			40
#define INT_MAX_DIGITS 		11

// Drop Frame parameters
// 30/29.97DF
#define DF29_DPM   2  	 /* Frames dropped per minute  */
#define DF29_MSKIP 10 	 /* Skip drop every 10 minutes */
#define DF29_FPM   1798  /* Frames per minute  getTCframes(0,  1, 0, 0, 30, 1) */
#define DF29_FPM10 17982 /* Frames per 10 mins getTCframes(0, 10, 0, 0, 30, 1) */

// Defaults
#define DEF_FRAMECOUNT 	getTCframes(0, 0, 0, 0, DEF_FPS, DEF_PULLDWN)
#define DEF_FPS 		24
#define DEF_UB 			0x00, 0x00, 0x00, 0x00
#define DEF_PULLDWN 	0 
#define DEF_DROPFRM 	0
#define DEF_TC_FMT 		tc_unspecified
#define DEF_UB_FMT 		ub_unspecified

// Validity Arrays
#define TC_FPS_LEN 5
#define TC_FPS_ARR {\
	24, 25, 30, 50, 60 /*, 48, 72, 100, 120 */\
}

#define TC_PULLED_LEN 3
#define TC_PULLED_ARR { 23, 29, 59 }

#define TC_PULLDOWN_LEN 3
#define TC_PULLDOWN_ARR { 24, 30, 60 }

#define TC_DF_LEN 1
#define TC_DF_ARR { 30 }

// Enums
typedef enum TCFormat {
	tc_unspecified = 0,
	tc_clocktime = 1
} TCFormat;

typedef enum UBFormat {
	ub_unspecified = 0,
	ub_text = 1,
	ub_date = 2,
	ub_page = 3
} UBFormat;


// Structs
typedef struct Timecode {
	unsigned int	frames;				// frames since midnight
	unsigned char 	userBits[4];		// { uBit bytes }
	unsigned char 	frameRate;			// integer fps
	unsigned char 	isPullDown:1;		// bool (true = 23.976/29.97)
	unsigned char 	isDropFrame:1;		// bool
	TCFormat		tcClockFmt:1;		// unspecified/clock
	UBFormat	 	userBitsFmt:2;		// unspec/text/date/page
} Timecode;


// Function Prototypes
uint8_t * getTCarr(Timecode * tc);
int getTCarrFrames(int * TCarr, unsigned char frameRate, int isDrop);
int getTCframes(int hh, int mm, int ss, int ff, unsigned char frameRate, int isDrop);
int setUserBits(Timecode * tc, int ub1, int ub2, int ub3, int ub4);
int setUserBitsArr(Timecode * tc, int * ubArr);
int setAsCurrentTime(Timecode * tc);
int setAsCurrentDate(Timecode * tc);
char * getTCstr(Timecode * tc);
Timecode * getTCdefault(void);

#endif