/*
BUILD WAVE FILE Header
*/

#ifndef WAV_BUILD_H
#define WAV_BUILD_H

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "wav_universal.h"
#include "ltc_generator.h"
#include "audio_generators.h"
#define VERSION_STR "0.3"

// Default options
#define DEF_BITRATE 16
#define DEF_SAMPRATE 44100
#define DEF_FILE "./out.wav"
#define DEF_SECS 60
#define DEF_CHANS 1
#define DEF_GEN GEN_SILENCE // WAVE function
#define DEF_VOL DB_TO_FLOAT(-6.0) // 0.0 - 1.0 (negative will invert)
#define DEF_FREQ 1000 // Hz
#define DEF_INVERT 0

// Settings
#define ENABLE_META 0
#define UNSIGNED_CUTOFF 8 // Data-format becomes signed after...
#define USE_DS64 0xFFFFFFFF

// Validity Arrays
#define BITRATE_LEN 4
#define BITRATE_ARR { 8, 16, 24, 32 }

#define SAMPRATE_LEN 10
#define SAMPRATE_ARR {\
	11025, 22050, 44100, 88200, 8000,\
	32000, 48000, 64000, 96000, 192000\
}

// Chunk Entry Indexes
#define WAVECOUNT ( 6 + 2 * ENABLE_META )
enum Wave_Chunk {
	ID_ENTRY   = 0,
	SIZE_ENTRY = 1,
	WAVE_ENTRY = 2,
	DS64_ENTRY = 3,
	BEXT_ENTRY = 4,
	XML_ENTRY  = 5,
	FMT_ENTRY  = (4 + (ENABLE_META ? 2 : 0)),
	DATA_ENTRY = (5 + (ENABLE_META ? 2 : 0))
};
#define DS64COUNT 10 // Total entries
enum DS64_Chunk {
	DS64_WAVE_SIZE    = 2,
	DS64_WAVE_SIZE_H  = 3,
	DS64_DATA_SIZE    = 4,
	DS64_DATA_SIZE_H  = 5,
	DS64_SAMP_COUNT   = 6,
	DS64_SAMP_COUNT_H = 7,
	DS64_TABLE_LEN    = 8,
	DS64_TABLE_CHUNK  = 9
};
#define FMTCOUNT 8 // Total entries
enum Format_Chunk {
	FMT_FORMAT = 2,
	FMT_CHANNELS = 3,
	FMT_SAMPLERATE = 4,
	FMT_BYTERATE = 5,
	FMT_BLOCKSIZE = 6,
	FMT_BITRATE = 7
};
#define DATACOUNT 2 // Total entries


// Additional enums
enum AudioFormat {
	NO_FORMAT	= 0x00,
	PCM_FORMAT	= 0x01
};


// Chunk Constants

// Parent Constants
#define FILEID intStr("RIFF")
#define WAVEID intStr("WAVE")
// DS64 Constants
#define DS64ID intStr("ds64")
// Format Constants
#define FMTID  intStr("fmt ")
#define AUDIOFMT	PCM_FORMAT
// Data Constants
#define DATAID intStr("data")
// Bext Constants
#define BEXTID intStr("bext")
#define BEXTSIZE 858
#define BNAME 's'
#define BVAL '='
#define BBLANK 0
// XML Constants
#define XMLID intStr("iXML")
#define NL "\r\n"
#define XMLSIZE 5226
#define XBLANK ' '

// Macro Functions
#define U64_LO_INT(num) (uint32_t)( num       & 0xFFFFFFFF)
#define U64_HI_INT(num) (uint32_t)((num >> 4) & 0xFFFFFFFF)
#define SET_U64_ENTRIES(ofChunk, lowIndex, setValue) \
	ofChunk.entry[lowIndex]   = getEntryV(4, U64_LO_INT(setValue));\
	ofChunk.entry[lowIndex+1] = getEntryV(4, U64_HI_INT(setValue))
#define MAX_BITS(b) ((1 << (b)) - 1) // b = b - 1 for signed
#define CLIP(in,max) ((in) >= 0 ? ((in) > (max) ? (max) : (in)) : ((in) < -(max + 1) ? -(max + 1) : (in)))
#define VOL_TO_VAL(v,b) CLIP((int)round(MAX_BITS(b - 1) * v), MAX_BITS(b - 1))
#define DB_TO_FLOAT(db)  pow(10.0, db / 20.0)
#define FLOAT_TO_DB(amp) (log10(fabs(amp)) * 20.0)

// TypeDefs
typedef struct ChunkEntry ChunkEntry;

typedef struct Chunk {
	int count;
	ChunkEntry * entry;
} Chunk;

struct ChunkEntry {
	uint8_t isChunk:1;
	uint8_t byteCount:3;
	union {
		uint32_t val; 
		Chunk chunk;
	};
};

typedef struct ChanData {
	Generator generator;
	GenSetting setting;
	double volume;
	double offset;
} ChanData;

typedef struct WaveFile {
	Chunk hdr;
	ChanData * chan;
} WaveFile;

typedef struct WaveData {
	char * fname;
	WaveGens generator;
	int bitRate;
	int sampleRate;
	int channels;
	double seconds;
	double volume;
	double frequency;
	Timecode * startTC;
} WaveData;

// Function Prototypes
WaveData * getWaveDefault(void);
int makeWave(WaveData * data);
void freeWave(WaveData * data);

#endif