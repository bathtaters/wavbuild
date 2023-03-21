/*
BUILD WAVE FILE
*/

#include "wav_build.h"

// Globals
const int BITRATE_VALID[BITRATE_LEN] = BITRATE_ARR;
const int SAMPRATE_VALID[SAMPRATE_LEN] = SAMPRATE_ARR;
const WaveGens SETTING_FREQ[GEN_FREQ_LEN] = GEN_FREQ_SET;
const WaveGens SETTING_TC[GEN_TC_LEN] = GEN_TC_SET;
const char * GEN_NAMES[] = GEN_NAMES_ARR;
Generator GEN_FUNCTIONS[] = GEN_FUNCT_ARR;

// Helper Functions

// Convert str to int (little-endian)
uint32_t intStr(char str[5]) {
	uint32_t num = 0;
	for(int c=0; c < 4; c++) {
		num += str[c] << (8 * c);
	}
	return num;
}

ChunkEntry getEntryB() {
	ChunkEntry entry = {
		(uint8_t)0,
		(uint8_t)0,
		{ (uint32_t)0 }
	};
	return entry;
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

uint64_t getChunkSize(Chunk chunk) {
	int size = 0, chunkSize;
	for(int e = 0; e < chunk.count; e++) {
		if(chunk.entry[e].isChunk) {
			chunkSize = chunk.entry[e].chunk.entry[SIZE_ENTRY].val;
			// WHAT TO DO IF 64-bit VALUE
			size += 8 + chunkSize; // Add ID + SIZE entries
		}
		else {
			size += chunk.entry[e].byteCount;
		}
	}
	return size;
}

WaveData * getWaveDefault(void) {
	WaveData * data = malloc(sizeof(WaveData));
	if (!data) { return NULL; }

	data->fname = malloc(sizeof(DEF_FILE) + sizeof(char));
	if (!data->fname) { return NULL; }
	strcpy(data->fname, DEF_FILE);

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


Chunk dataChunk(uint64_t size) {
	ChunkEntry *entries = malloc(DATACOUNT * sizeof(ChunkEntry));
	Chunk data = { DATACOUNT, entries };
	entries[ID_ENTRY] = getEntryV(4, DATAID);
	
	if(size >= UINT32_MAX) { size = USE_DS64; }
	entries[SIZE_ENTRY] = getEntryV(4, (int)size);
	
	return data;
}


Chunk fmtChunk(int bitRate, int sampleRate, int channelCount) {
	ChunkEntry *entries = malloc(FMTCOUNT * sizeof(ChunkEntry));
	Chunk fmt = { FMTCOUNT, entries };
	
	entries[ID_ENTRY] = getEntryV(4, FMTID);
	entries[SIZE_ENTRY] = getEntryB();
	entries[FMT_FORMAT] = getEntryV(2, AUDIOFMT);
	entries[FMT_CHANNELS] = getEntryV(2, channelCount);
	entries[FMT_SAMPLERATE] = getEntryV(4, sampleRate);
	entries[FMT_BYTERATE] = getEntryV(4, sampleRate * bitRate * channelCount / 8);
	entries[FMT_BLOCKSIZE] = getEntryV(2, channelCount * bitRate / 8);
	entries[FMT_BITRATE] = getEntryV(2, bitRate);
	
	entries[SIZE_ENTRY] = getEntryV(4, (int)getChunkSize(fmt) - 4); // Don't count ID
	return fmt;
}

Chunk ds64Chunk(uint64_t waveSize, uint64_t dataSize, uint64_t sampleCount) {
	ChunkEntry *entries = malloc(DS64COUNT * sizeof(ChunkEntry));
	Chunk ds64 = { DS64COUNT, entries };
	
	entries[ID_ENTRY] =  getEntryV(4, DS64ID);
	entries[SIZE_ENTRY] = getEntryB();
	SET_U64_ENTRIES(ds64, DS64_WAVE_SIZE,  waveSize);
	SET_U64_ENTRIES(ds64, DS64_DATA_SIZE,  dataSize);
	SET_U64_ENTRIES(ds64, DS64_SAMP_COUNT, sampleCount);
	entries[DS64_TABLE_LEN] = getEntryV(4, 0);
	entries[DS64_TABLE_CHUNK] = getEntryB();
	
	entries[SIZE_ENTRY] = getEntryV(4, (int)getChunkSize(ds64) - 4); // Don't count ID
	return ds64;
}

Chunk hdrChunk(int bitRate, int sampleRate, int channelCount, uint64_t channelLen) {
	ChunkEntry *entries = malloc(WAVECOUNT * sizeof(ChunkEntry));
	Chunk wave = { WAVECOUNT, entries };
	
	uint64_t dataSize = channelLen * channelCount * bitRate / 8;
	entries[ID_ENTRY] = getEntryV(4, FILEID);
	entries[SIZE_ENTRY] = getEntryB();
	entries[DS64_ENTRY] = getEntryB();
	entries[WAVE_ENTRY] = getEntryV(4, WAVEID);
	if(ENABLE_META) {
		entries[BEXT_ENTRY] = getEntryC(bextChunk());
		entries[XML_ENTRY]  = getEntryC(xmlChunk());
	}
	entries[FMT_ENTRY] = getEntryC(fmtChunk(bitRate, sampleRate, channelCount));
	entries[DATA_ENTRY] = getEntryC(dataChunk(dataSize));
	
	uint64_t totalSize = getChunkSize(wave) - 4;
	if(totalSize >= UINT32_MAX) {
		if(entries[DATA_ENTRY].chunk.entry[SIZE_ENTRY].val != USE_DS64)
			{ dataSize = 0; }
		entries[DS64_ENTRY] = getEntryC(ds64Chunk(0, dataSize, 0));
		totalSize += entries[DS64_ENTRY].chunk.entry[SIZE_ENTRY].val + 8;
		SET_U64_ENTRIES(entries[DS64_ENTRY].chunk, DS64_WAVE_SIZE, totalSize);
		totalSize = USE_DS64;
	}
	
	entries[SIZE_ENTRY] = getEntryV(4, (int)totalSize);
	return wave;
}


// WRITE FUNCTIONS

void freeChunk(Chunk chunk) {
	for(int e = 0; e < chunk.count; e++) {
		if(chunk.entry[e].isChunk)
			{ freeChunk(chunk.entry[e].chunk); }
	}
	if(chunk.entry) { 
		free(chunk.entry);
		chunk.entry = NULL;
	}
}

void writeChunk(Chunk chunk, FILE * f) {
	for(int e = 0; e < chunk.count; e++) {
		if(chunk.entry[e].isChunk)
			{ writeChunk(chunk.entry[e].chunk, f); }
		else if(chunk.entry[e].byteCount)
			{ fwrite((uint8_t *)&chunk.entry[e].val, 1, chunk.entry[e].byteCount, f); }
	}
}

void writeChannels(WaveFile wav, FILE *f) {
	int s, c, val;
	int br = wav.hdr.entry[FMT_ENTRY].chunk.entry[FMT_BITRATE].val;
	int sr = wav.hdr.entry[FMT_ENTRY].chunk.entry[FMT_SAMPLERATE].val;
	int cCount = wav.hdr.entry[FMT_ENTRY].chunk.entry[FMT_CHANNELS].val;
	int sCount = wav.hdr.entry[DATA_ENTRY].chunk.entry[SIZE_ENTRY].val * 8 / cCount / br;
	for(s = 0; s < sCount; s++) {
		for(c = 0; c < cCount; c++) {
			val = VOL_TO_VAL(
				wav.chan[c].generator(
					wav.chan[c].offset + (double)s/sr,
					wav.chan[c].setting
				) * wav.chan[c].volume,
				br
			);
			if(br <= UNSIGNED_CUTOFF)
				{ val += MAX_BITS(br - 1) + 1; } // Shift signed to unsigned
			fwrite((void *)&val, br/8, 1, f);
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
	writeChannels(wav, f);
	
	fclose(f);
	return 0;
}



// CHANNEL FUNCTIONS
GenSetting getGenSetting(WaveData * data) {
	GenSetting setting;
	int i;
	
	// Frequency
	for(i = 0; i < GEN_FREQ_LEN; i++) {
		if(data->generator == SETTING_FREQ[i]) {
			setting.dec = data->frequency;
			return setting;
		}
	}
	// Timecode
	for(i = 0; i < GEN_TC_LEN; i++) {
		if(data->generator == SETTING_TC[i]) {
			setting.tc = data->startTC;
			return setting;
		}
	}
	// Default
	setting.num = 0;
	return setting;
}

Generator getGenFunc(WaveGens genID) {
	if(genID < GEN_FIRST || genID > GEN_LAST) { return NULL; }
	return GEN_FUNCTIONS[genID];
}

ChanData getChan(WaveGens generator, GenSetting setting, double volume, double offset) {
	ChanData channel = { getGenFunc(generator), setting, volume, offset };
	return channel;
}

ChanData * getMonoChans(int chanCount, WaveGens generator, GenSetting setting, double volume, double offset) {
	ChanData * channels = malloc(chanCount * sizeof(ChanData));
	for(int c = 0; c < chanCount; c++) {
		channels[c] = getChan(generator, setting, volume, offset);
	}
	return channels;
}


// WAVE FUNCTIONS
WaveFile getWaveFile(WaveData * data) { // MONO ONLY!
	WaveFile wav = {
		hdrChunk(
			data->bitRate,
			data->sampleRate,
			data->channels,
			data->seconds * data->sampleRate
		),
		getMonoChans(
			data->channels,
			data->generator,
			getGenSetting(data),
			data->volume,
			0.0 // offset
		)
	};
	return wav;
}

void freeWaveFile(WaveFile file) {
	freeChunk(file.hdr);
	free(file.chan);
}

// MAIN FUNCTION
// audioData = { {val of chan1 Samples}, {val of chan2 Samples}, ... }
// val is float in range [-1.0, 1.0]
int makeWave(WaveData * data) {		
	WaveFile wav = getWaveFile(data);
	int ret = saveWave(wav, data->fname);
	freeWaveFile(wav);
	return ret;
}

void freeWave(WaveData * data) {
	if (data->fname) { free(data->fname); }
	if (data->startTC) { free(data->startTC); }
	free(data);
}