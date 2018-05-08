/*
BUILD WAVE FILE
*/

#include "wav_build.h"

// Globals
const int BITRATE_VALID[BITRATE_LEN] = BITRATE_ARR;
const int SAMPRATE_VALID[SAMPRATE_LEN] = SAMPRATE_ARR;
const char * GEN_NAMES[] = GEN_NAMES_ARR;

// Helper Functions

// Convert str to int (little-endian)
uint32_t intStr(char str[5]) {
	uint32_t num = 0;
	for(int c=0; c < 4; c++) {
		num += str[c] << (8 * c);
	}
	return num;
}

ChunkEntry getEntryC(Chunk nestedChunk) {
	ChunkEntry entry = {
		(uint8_t)1,
		(uint8_t)0,
		{ .chunk = nestedChunk }
	};
	return entry;
}
	
ChunkEntry getEntryV(int bytes, int value) {
	ChunkEntry entry = {
		(uint8_t)0,
		(uint8_t)(bytes & 0x7),
		{ (uint32_t)value }
	};
	return entry;
} 

int getChunkSize(Chunk chunk) {
	int size = 0, chunkSize;
	for(int e = 0; e < chunk.entryCount; e++) {
		if(chunk.entries[e].isChunk) {
			chunkSize = chunk.entries[e].value.chunk.entries[SIZE_ENTRY].value.num;
			// WHAT TO DO IF 64-bit VALUE
			size += 8 + chunkSize; // Add ID + SIZE entries
		}
		else {
			size += chunk.entries[e].byteCount;
		}
	}
	return size;
}

WaveData * getWaveDefault(void) {
	WaveData * data = malloc(sizeof(WaveData));
	data->fname = DEF_FILE;
	data->generator = DEF_GEN;
	data->bitRate = DEF_BITRATE;
	data->sampleRate = DEF_SAMPRATE;
	data->channels = DEF_CHANS;
	data->seconds = DEF_SECS;
	data->volume = DEF_VOL;
	data->frequency = DEF_FREQ;
	data->startTC = getTCdefault();
	return data;
}




// CHUNK FUNCTIONS

Chunk blankChunk(void) { Chunk none = { 0, NULL }; return none; }

// TO WRITE!!!
Chunk bextChunk(void) { return blankChunk(); }
Chunk xmlChunk(void) { return blankChunk(); }



Chunk dataChunk(int size) {
	ChunkEntry *entries = malloc(DATACOUNT * sizeof(ChunkEntry));
	entries[ID_ENTRY] = getEntryV(4, DATAID);
	entries[SIZE_ENTRY] = getEntryV(4, size);
	
	Chunk data = { DATACOUNT, entries };
	return data;
}


Chunk fmtChunk(int bitRate, int sampleRate, int channelCount) {
	ChunkEntry *entries = malloc(FMTCOUNT * sizeof(ChunkEntry));
	entries[ID_ENTRY] = getEntryV(4, FMTID);
	entries[FMT_FORMAT] = getEntryV(2, AUDIOFMT);
	entries[FMT_CHANNELS] = getEntryV(2, channelCount);
	entries[FMT_SAMPLERATE] = getEntryV(4, sampleRate);
	entries[FMT_BYTERATE] = getEntryV(4, sampleRate * bitRate * channelCount / 8);
	entries[FMT_BLOCKSIZE] = getEntryV(2, channelCount * bitRate / 8);
	entries[FMT_BITRATE] = getEntryV(2, bitRate);
	
	Chunk fmt = { FMTCOUNT, entries };
	entries[SIZE_ENTRY] = getEntryV(4, getChunkSize(fmt) - 4); // Don't count ID
	return fmt;
}

Chunk hdrChunk(int bitRate, int sampleRate, int channelCount, int channelLen) {
	ChunkEntry *entries = malloc(WAVECOUNT * sizeof(ChunkEntry));
	entries[ID_ENTRY] = getEntryV(4, FILEID);
	entries[WAVE_ENTRY] = getEntryV(4, WAVEID);
	if(ENABLE_META) {
		entries[BEXT_ENTRY] = getEntryC(bextChunk());
		entries[XML_ENTRY]  = getEntryC(xmlChunk());
	}
	entries[FMT_ENTRY] = getEntryC(fmtChunk(bitRate, sampleRate, channelCount));
	entries[DATA_ENTRY] = getEntryC(dataChunk(channelLen * channelCount * bitRate / 8));
	
	Chunk wave = { WAVECOUNT, entries };
	entries[SIZE_ENTRY] = getEntryV(4, getChunkSize(wave) - 4); // Don't count ID
	return wave;
}




// WRITE FUNCTIONS

void writeChunk(Chunk, FILE *);

void writeEntries(ChunkEntry * entryArr, int entryCount, FILE * f) {
	for(int e = 0; e < entryCount; e++) {
		if(entryArr[e].isChunk) {
			writeChunk(entryArr[e].value.chunk, f);
			continue;
		}
		fwrite((uint8_t *)&entryArr[e].value.num, 1, entryArr[e].byteCount, f);
	}
}

void writeChunk(Chunk fromChunk, FILE * f) {
	writeEntries(fromChunk.entries, fromChunk.entryCount, f);
	free(fromChunk.entries);
}

void writeArr(WaveFile wav, FILE *f) {
	int s, c, val;
	int bCount = wav.hdr.entries[FMT_ENTRY].value.chunk.entries[FMT_BITRATE].value.num / 8;
	int cCount = wav.hdr.entries[FMT_ENTRY].value.chunk.entries[FMT_CHANNELS].value.num;
	int sCount = wav.hdr.entries[DATA_ENTRY].value.chunk.entries[SIZE_ENTRY].value.num / cCount / bCount;
	for(s = 0; s < sCount; s++) {
		for(c = 0; c < cCount; c++) {
			val = VOL_TO_VAL(wav.data[c][s], (bCount * 8));
			fwrite((void *)&val, bCount, 1, f);
		}
	}
}

int saveWave(WaveFile wav, char * filename) {
	FILE *f;

	f = fopen(filename, "wb");
	if(!f) {
		printf("Unable to create file.\n");
		return 1;
	}
	
	writeChunk(wav.hdr, f);
	writeArr(wav, f);
	
	fclose(f);
	return 0;
}



// CALL TO OTHER FILES

float * getAudioChannel(WaveData * data) {
	float * chanData;
		
	switch(data->generator) {
		case GEN_SINE :
			chanData = sine(
				data->sampleRate * data->seconds,
				data->volume,
				data->sampleRate / data->frequency
			);
			break;
		case GEN_TRIANGLE :
			chanData = triangle(
				data->sampleRate * data->seconds,
				data->volume,
				data->sampleRate / data->frequency
			);
			break;
		case GEN_SAWTOOTH :
			chanData = sawtooth(
				data->sampleRate * data->seconds,
				data->volume,
				data->sampleRate / data->frequency
			);
			break;
		case GEN_SQUARE :
			chanData = square(
				data->sampleRate * data->seconds,
				data->volume,
				data->sampleRate / data->frequency
			);
			break;
		case GEN_TIMECODE :
			chanData = ltcChannel(
				data->startTC,
				(int)round(data->seconds * data->startTC->frameRate),
				data->volume,
				data->sampleRate
			);
			break;
		case GEN_SILENCE :
			chanData = silence(data->sampleRate * data->seconds);
			break;
		case GEN_HEADER :
			chanData = calloc(1, sizeof(float));
			data->seconds = 0.0;
			break;			
		default:
			free(chanData);
			return NULL;
	};
	
	return chanData;
}



// MAIN FUNCTIONS

WaveFile getWaveFile(WaveData * data, float ** audioData) {
	WaveFile wav = {
		hdrChunk(
			data->bitRate,
			data->sampleRate,
			data->channels,
			data->seconds * data->sampleRate
		),
		audioData
	};
	return wav;
}

// audioData = { {val of chan1 Samples}, {val of chan2 Samples}, ... }
// val is float in range [-1.0, 1.0]
int makeWave(WaveData * data) {
	float ** audioData = malloc(data->channels * sizeof(float *));
	int mono = 1;
	
	audioData[0] = getAudioChannel(data);
	if(!audioData[0]) { return (int)(data->generator); }
	
	if(mono) {
		for(int c = 1; c < data->channels; c++)
			{ audioData[c] = audioData[0]; }
	}
	
	int ret = saveWave(getWaveFile(data, audioData), data->fname);
	
	if(mono) { free(audioData[0]); }
	else {
		for(int c = 0; c < data->channels; c++)
			{ free(audioData[c]); }
	}
	free(audioData);
	
	return ret;
}
