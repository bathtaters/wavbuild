// DEBUG FUNCTIONS
#include "wav_build.h"
#include <stdio.h>

// DEBUG
Timecode quickTC(int frames, int fps);

// WAV_BUILD_UI FUNCTIONS

void printArgs(int argc, char **argv) {
	for(int i=0; i < argc; i++) {
		printf("arg:%02d = '%s'\n",i,argv[i]);
	}
}

void printWaveData(WaveData * data) {
	printf("File: '%s'\n", data->fname);
	printf("   Bit-rate: %d bits\n", data->bitRate);
	printf("   Sample Rate: %d Hz\n", data->sampleRate);
	printf("   Channel Count: %d channels\n", data->channels);
	printf("   Generator: %s\n", GEN_NAMES[data->generator]);
	printf("   Audio Length: %.2f seconds\n", data->seconds);
	printf("   Audio Volume: %.2f\n", data->volume);
	printf("   Audio Frequency: %.2f Hz\n", data->frequency);
	printf("   Timecode: %s\n", getTCstr(data->startTC));
}



/* LTC_GENERATE FUCNTIONS */
Timecode quickTC(int frames, int fps) {
	Timecode tc = {
		frames,
		DEF_UB,
		fps,
		DEF_PULLDWN,
		DEF_DROPFRM,
		DEF_TC_FMT,
		DEF_UB_FMT
	};
	return tc;
}

void printBin(uint8_t * start, int count) {
	printf("Binary:");
	for(int i=0; i < count*8; i++) {
		printf("%d",getBit(start,i));
		//if(!(i%4)) printf(" ");
	}
	printf("\n");
}




// WAV_BUILD.C FUNCTIONS

#define BIN_PAT "%c%c%c%c %c%c%c%c %c%c%c%c %c%c%c%c"
#define BIN_BYTE(byte)  \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x0800 ? '1' : '0'), \
  (byte & 0x0400 ? '1' : '0'), \
  (byte & 0x0200 ? '1' : '0'), \
  (byte & 0x0100 ? '1' : '0'), \
  (byte & 0x0080 ? '1' : '0'), \
  (byte & 0x0040 ? '1' : '0'), \
  (byte & 0x0020 ? '1' : '0'), \
  (byte & 0x0010 ? '1' : '0'), \
  (byte & 0x0008 ? '1' : '0'), \
  (byte & 0x0004 ? '1' : '0'), \
  (byte & 0x0002 ? '1' : '0'), \
  (byte & 0x0001 ? '1' : '0')
#define BADCHR ' '
#define STR_PAT "'%c%c%c%c'"
#define STR_BYTE(byte)  \
  (((byte >> 24) & 0xFF) > 0x1F ? (byte >> 24) & 0xFF : BADCHR), \
  (((byte >> 16) & 0xFF) > 0x1F ? (byte >> 16) & 0xFF : BADCHR), \
  (((byte >>  8) & 0xFF) > 0x1F ? (byte >>  8) & 0xFF : BADCHR), \
  (((byte >>  0) & 0xFF) > 0x1F ? (byte >>  0) & 0xFF : BADCHR)
  
void printChunk(Chunk in) {
	printf("---Chunk %08X---\n   Entries: %d\n", (int)&in, in.entryCount);
	for(int i=0; i < in.entryCount; i++) {
		printf("Entry %03d @ %08X (Size: %d): ",i,(int)&in.entries[i],in.entries[i].byteCount);
		if(in.entries[i].isChunk) {
			printf("Nested Chunk @ %#X\n",(int)&in.entries[i].value.chunk);
			printChunk(in.entries[i].value.chunk);
		}
		else {
			printf("Value: %10d 0x%08X "BIN_PAT" "STR_PAT"\n",
				in.entries[i].value.num,
				in.entries[i].value.num,
				BIN_BYTE(in.entries[i].value.num),
				STR_BYTE(in.entries[i].value.num)
			);
		}
	}
	printf("~~~End Chunk %08X~~~\n", (int)&in);
}

void displayData(int chanCount, int sampCount, int ** dataArr) {
	printf("DISPLAYING %d channels w/ %d samples.\n",chanCount,sampCount);
	printf("DATAARR: %d %d\n", (int)dataArr, *((int *)dataArr+1));
	int s,c,b;
	for(c = 0; c < chanCount; c++) {
		printf("CHANNEL %02d:\n",c);
		for(s = 0; s < sampCount; s++) {
			printf("%04d:",s);
			printf(" %06d",dataArr[c][s]);
			//for(b = 0; b < DEF_BITRATE/8; b++) {
				//printf(" %03d", (dataArr[c][s] >> (8*b)) & 0xFF);
			//}
			printf("\n");
		}
	}
}