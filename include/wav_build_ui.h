/*
BUILD WAVE FILE UI Header
*/

// Use "wavbuild -h" for list of commands
// Make with: sudo gcc wav_build_ui.c wav_build.c audio_generate.c ltc_generate.c timecode.c -o /usr/local/bin/wavbuild


#ifndef WAV_BUILD_UI_H
#define WAV_BUILD_UI_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "wav_build.h"

// Frontend Constants
#define WAV_BUILD_FILENAME "wavbuild"
#define WAV_BUILD_HEADER "Wav Builder (Version "VERSION_STR") 2018 by bathtaters."
#define DEF_FILE_EXT "wav"
#define MAX_CHANNELS 0xFFFF
#define TC_ERR_LEN 200
#define SEC_TO_MS 100
#define PATH_SEP '/'
#define PATH_EXT '.'
#define TAB_LEN 6

// xchar validity
#define NDF_SIZE 3
#define NDF_ARR { 'n', 'd', 'f' }

// Typedefs

typedef enum ByteFormat {
	chr = -1,
	bin = 2,
	oct = 8,
	dec = 10,
	hex = 16
} ByteFormat;
#define DEF_BYTE_FORMAT hex
#define NAME_BYTEFMT(bf)\
	( bf==chr ? "String" : \
	( bf==bin ? "Binary" : \
	( bf==oct ? "Octal"  : \
	( bf==dec ? "Decimal": \
	( bf==hex ? "Hex"    : \
				"Unknown" )))))

// User Input Constants
#define DEF_VERBOSITY 0
#define SHORT_OPTIONS "o:g:b:s:c:l:d:v:q:t:f:u:m:h"
#define LONG_OPTIONS {\
	  {"verb",		no_argument,		&verbose,	1},\
	  {"verbose",	no_argument,		&verbose,	1},\
	  {"brief",		no_argument,		&verbose,	0},\
	  {"help",		no_argument,		0,	'h'},\
	  {"output",	required_argument,	0, 	'o'},\
	  {"out",		required_argument,	0, 	'o'},\
	  {"generator",	required_argument,	0, 	'g'},\
	  {"gen",		required_argument,	0, 	'g'},\
	  {"channels",	required_argument,	0, 	'c'},\
	  {"chan",		required_argument,	0, 	'c'},\
	  {"samples",	required_argument,	0, 	's'},\
	  {"sr",		required_argument,	0, 	's'},\
	  {"bits",		required_argument,	0, 	'b'},\
	  {"br",		required_argument,	0, 	'b'},\
	  {"length",	required_argument,	0, 	'd'},\
	  {"secs",		required_argument,	0, 	'd'},\
	  {"seconds",	required_argument,	0, 	'd'},\
	  {"duration",	required_argument,	0, 	'd'},\
	  {"dur",		required_argument,	0, 	'd'},\
	  {"volume",	required_argument,	0, 	'v'},\
	  {"vol",		required_argument,	0, 	'v'},\
	  {"frequency",	required_argument,	0, 	'q'},\
	  {"freq",		required_argument,	0, 	'q'},\
	  {"timecode",	required_argument,	0, 	't'},\
	  {"tc",		required_argument,	0, 	't'},\
	  {"framerate",	required_argument,	0, 	'f'},\
	  {"fps",		required_argument,	0, 	'f'},\
	  {"userbits",	required_argument,	0, 	'u'},\
	  {"ub",		required_argument,	0, 	'u'},\
	  {"format",	required_argument,	0, 	'm'},\
	  {"fmt",		required_argument,	0, 	'm'},\
	  {"hexadecimal", no_argument,	 	0, 	0x80},\
	  {"hex",		no_argument,	 	0, 	0x80},\
	  {"decimal",	no_argument, 		0, 	0x81},\
	  {"dec",		no_argument, 		0, 	0x81},\
	  {"octal",		no_argument,	 	0, 	0x82},\
	  {"oct",		no_argument,	 	0, 	0x82},\
	  {"binary",	no_argument,	 	0, 	0x83},\
	  {"bin",		no_argument,	 	0, 	0x83},\
	  {"text",		no_argument,		0, 	0x84},\
	  {"string",	no_argument,		0, 	0x84},\
	  {"str",		no_argument,		0, 	0x84},\
	  {"nondrop",	no_argument, 		0, 	0x90},\
	  {"ndf",		no_argument, 		0, 	0x90},\
	  {"nd",		no_argument, 		0, 	0x90},\
	  {"dropframe",	no_argument, 		0, 	0x91},\
	  {"drop",		no_argument, 		0, 	0x91},\
	  {"df",		no_argument, 		0, 	0x91},\
	  {"pulldown",	no_argument, 		0, 	0x92},\
	  {"pulled",	no_argument, 		0, 	0x92},\
	  {"pull",		no_argument, 		0, 	0x92},\
	  {"pd",		no_argument, 		0, 	0x92},\
	  {"invert",	no_argument, 		0, 	0xA0},\
	  {"inverse",	no_argument, 		0, 	0xA0},\
	  {"inv",		no_argument, 		0, 	0xA0},\
	  {0, 			0,					0,	0}\
}

// Function Prototypes
int invalidTC(Timecode * tc);
int errorCheck(WaveData * data);
WaveData * getWaveFromArgs(int argc, char **argv);

#endif