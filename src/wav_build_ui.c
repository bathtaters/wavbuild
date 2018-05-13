/*
BUILD WAVE FILE UI
Error checking and user-input sanitizing
*/

#include "wav_build_ui.h"

// Globals
extern const char * GEN_NAMES[];
extern const int BITRATE_VALID[BITRATE_LEN];
extern const int SAMPRATE_VALID[SAMPRATE_LEN];
extern const int TC_FPS_VALID[TC_FPS_LEN];
extern const int TC_PULLED[TC_PULLED_LEN];
extern const int TC_PULLDOWN_VALID[TC_PULLDOWN_LEN];
extern const int TC_DF_VALID[TC_DF_LEN];
char * xchars = NULL;
int getErr = 0;
const int NDF_VALID[NDF_SIZE] = NDF_ARR;
static int verbose = DEF_VERBOSITY;
static struct option long_opts[] = LONG_OPTIONS;



// GENERIC FUNCTIONS

int arrayMatch(int val, const int * array, int len) {
	for(int i = 0; i < len; i++) {
		if(array[i] == val) { return 1; }
	}
	return 0;
}

int strscmp(char const *str, int count, ...) {
	if(!str) { return 1; } // Null ptr
	char const *a, *b;
	va_list arglist;
	va_start(arglist, count);
	
	for (int i = 0; i < count; i++) {
		a = str;
		b = va_arg(arglist, char const *);
		for (; ; a++, b++) {
			if(!*a) {
				va_end(arglist);
				return 1;
			}
			if(tolower(*a) - tolower(*b) != 0) {
				break;
			}
		}
	}
	va_end(arglist);
	return 0;
}



// ERROR CHECKING

int validGenerator(int generator) {
	for(WaveGens g = GEN_FIRST; g <= GEN_LAST; g++) {
		if(generator == g) return 1;
	}
	return 0;
}

int invalidTC(Timecode * tc) {
	int ret = 0;
	char err[TC_ERR_LEN] = "\0";
	char *e = err + 1;
		
	if(tc->frames > MAX_SEC * tc->frameRate) {
		sprintf(--e,"Frame count exceeds maximum (%d). ", MAX_SEC * tc->frameRate);
		ret++; 
	}
	if(!arrayMatch(tc->frameRate, &TC_FPS_VALID[0], TC_FPS_LEN)) {
		while(*e) { e++; }
		sprintf(--e,"Invalid frame rate. ");
		ret++;
	}
	else {
		if(tc->isPullDown && !arrayMatch(tc->frameRate, &TC_PULLDOWN_VALID[0], TC_PULLDOWN_LEN)) {
			while(*e) { e++; }
			sprintf(--e,"Invalid frame rate for pulldown. ");
			ret++;
		}
		if(tc->isDropFrame && !arrayMatch(tc->frameRate, &TC_DF_VALID[0], TC_DF_LEN)) {
			while(*e) { e++; }
			sprintf(--e,"Invalid frame rate for drop-frame. ");
			ret++;
		}
	}
	
	if(ret) { printf("ERROR: %s (%s)\n", err, getTCstr(tc)); }
	return ret;
}

int errorCheck(WaveData * data) {
	int i,ret = 0;
	
	if(!data) { return 1; }
	
	if(!validGenerator(data->generator)) {
		printf("ERROR: Invalid generator selected. (%d)\n", data->generator);
		ret++;
	}
	if(!arrayMatch(data->bitRate, &BITRATE_VALID[0], BITRATE_LEN)) {
		printf("ERROR: Invalid bit rate. (%d)\n", data->bitRate);
		ret++;
	}
	if(!arrayMatch(data->sampleRate, &SAMPRATE_VALID[0], SAMPRATE_LEN)) {
		printf("ERROR: Invalid sample rate. (%d)\n", data->sampleRate);
		ret++;
	}
	if(data->channels > MAX_CHANNELS || data->channels < 0) {
		printf("ERROR: Channel count is out of range. (%d channels)\n", data->channels);
		ret++;
	}
	if(data->seconds < 0.0) {
		printf("ERROR: Negative audio length not allowed. (%.2f seconds)\n", data->seconds);
		ret++;
	}
	if(data->frequency < 0.0 && data->generator != GEN_TIMECODE) {
		printf("ERROR: Negative frequency not allowed. (%.2f Hz)\n", data->frequency);
		ret++;
	}
	if(data->volume < -1.0 || data->volume > 1.0) {
		printf("ERROR: Invalid volume. Must be between 1.0 and -1.0. (%.4f)\n", data->volume);
		ret++;
	}
	if(data->generator == GEN_TIMECODE) {
		ret += invalidTC(data->startTC);
	}
	if(!ret && !access(data->fname, F_OK)) {
		printf("%s exists. ", data->fname);
		if(access(data->fname, R_OK|W_OK)) {
			printf("Permission to modify denied.\n");
			ret++;
		}
		else {
			printf("Would you like to overwrite (y/n)? ");
			for(int c = 0; c != 'y' && c != 'Y'; ) {
				c = getchar();
				if(c == 'n' || c == 'N')
					{ ret++; break; }
			}
		}
	}
	if(ret) { printf("File not saved.\n"); }
	return ret;
}




// SCREEN OUTPUT
void printOpt(const char * opt1, const char * opt2, const char * detail, int isDefault, const char * defText) {
	if(opt1) {
		printf(" -%s",opt1);
		if(strlen(opt1) < TAB_LEN) { putchar('\t'); }
		if(opt2) { printf("-%s",opt2); }
		if(!opt2 || strlen(opt2) < TAB_LEN) { putchar('\t'); }
	}
	else if(opt2) {
		printf("\t\"%s\"",opt2);
		if(strlen(opt2) < TAB_LEN) { putchar('\t'); }
	}
	else { printf("\t\t"); }
	if(detail) { printf("%s",detail); }
	if(isDefault == -1) { printf(" [Default]"); }
	else if(isDefault || defText) {
		printf(" [Default: ");
		if(isDefault) { printf("%d",isDefault); }
		if(defText) { printf("%s",defText); }
		printf("]");
	}
	putchar('\n');
}

void printUsage(void) {
	printf("Usage: "WAV_BUILD_FILENAME" -<option> <value> ... -o <file>\n");
}

void printOptions(void) {
	printUsage();
	
	printf("\nWav Build Options:\n");
	printOpt("o","out","Output file <filepath>",0,"\""DEF_FILE"\"");
	printOpt("h","help","Display list of commands",0,0);
	
	printf("\nAudio Options:\n");
	printOpt("b","br","Bit-rate <8|16|24|32> bits per sample",DEF_BITRATE,"-bit");
	printOpt("s","sr","Sample rate <11025-192k> Hz",DEF_SAMPRATE," Hz");
	printOpt("d","dur","Duration <(hh:)mm:ss(.ms|:ff)> (Or append with <h|m|s|ms|f>)",DEF_SECS," secs");
	
	printf("\nGenerator Options:\n");
	printOpt("g","gen","Select generator <Silence|Sine|Sawtooth|Triangle|Square|LTC>",0,GEN_NAMES[DEF_GEN]);
	printOpt("v","vol","Volume <Percentage|Amplitude (Max=1)|dBFS (Max=0)>",FLOAT_TO_DB(DEF_VOL)," dBFS");
	printOpt("q","freq","Generator frequency <Hz> (Not used for LTC)",DEF_FREQ," Hz");
	printOpt("inv","invert","Invert generator wave (Vertical flip)",-DEF_INVERT,0);
	
	printf("\nTimecode Options: (For LTC)\n");
	printOpt("t","tc","Timecode of first frame (See \"-dur\" for format, or <Now> for current time)",0,0);
	printOpt("f","fps","Frame Rate <23|24|25|29|30|50|59|60> frames per second",DEF_FPS," fps");
	printOpt("df","drop","Force drop-frame counting (Only in 29.97/30)",-DEF_DROPFRM,0);
	printOpt("nd","ndf","Force non-drop-frame counting (Only with 29.97/30)",-(!DEF_DROPFRM),0);
	printOpt("pd","pull","Force pull-down of integer fps (Only with 24/30)",-DEF_PULLDWN,0);
	
	printf("\nUser-bits Options: (For LTC)\n");
	printOpt("u","ub","User-bits <u1:u2:u3:u4> (Also <4-char string> or <Today> for current date)",0,0);
	printOpt("<base>",0,"Input format of user-bits <-hex|-dec|-oct|-bin|-str>",0,NAME_BYTEFMT(DEF_BYTE_FORMAT));
	
	printf("\nTimecode/User-Bits Format:\n");
	printOpt("m","fmt","Force TC/UB format <format options (see below)>",0,0);
	printOpt(0,"none","Unspecified TC/UB", -(!DEF_TC_FMT && !DEF_UB_FMT),0);
	printOpt(0,"tc","Unspecified Timecode only", -(!DEF_TC_FMT && DEF_UB_FMT),0);
	printOpt(0,"clock","Timecode is clock-time", -(DEF_TC_FMT),0);
	printOpt(0,"ub","Unspecified User-Bits only", -(DEF_TC_FMT && !DEF_UB_FMT),0);
	printOpt(0,"date","User-bits are date", -(DEF_UB_FMT==ub_date),0);
	printOpt(0,"str","User-bits are ASCII text", -(DEF_UB_FMT==ub_text),0);
	printOpt(0,"page","User-bits are page/line", -(DEF_UB_FMT==ub_page),0);
	printf("\n");
}

void printWaveData(WaveData * data) {
	printf("Saving file \"%s\"\n", data->fname);
	printf("   Duration: %.2f seconds\t", data->seconds);
	printf("Generator: %s\n", GEN_NAMES[data->generator]);
	printf("   Bit-rate: %d bits\t\t", data->bitRate);
	printf("Volume: %.2f dBFS%s\n", FLOAT_TO_DB(data->volume),
		(data->volume < 0.0) ? " (Inverse)" : "");
	printf("   Sample Rate: %d Hz\t", data->sampleRate);
	if(data->generator != GEN_TIMECODE)
		{ printf("Frequency: %.2f Hz\n   ", data->frequency); }
	printf("Channel Count: %d channels\n", data->channels);
	if(data->generator == GEN_TIMECODE)
		{ printf("   LTC Start: %s\n", getTCstr(data->startTC)); }
}













// GENERIC DATA EXTRACTORS

int getInt(char * num, int size, int isSigned) {
	long int result;
	char *end, *x = xchars = realloc(xchars, strlen(num)+1);
	getErr = 0;
	*x = '\0';
	
	result = strtol(num, &end, 10);
	if(!result && end == num) { getErr = 1; return 0; }
	
	for( ; *end; end++) {
		switch(tolower(*end)) {
			case 'k': result *= 1000; break;
			default: *x++ = *end;
		}
	}
	*x = '\0';
	result = CLIP(result, (int)MAX_BITS((size * 8) - isSigned) );
	
	return (int)result;
}

double getFloat(char * num) {
	double result;
	char *end, *x = xchars = realloc(xchars, strlen(num)+1);
	getErr = 0;
	*x = '\0';
	
	result = strtod(num, &end);
	if(!result && end == num) { getErr = 1; return 0; }
	
	for( ; *end; end++) {
		switch(tolower(*end)) {
			case 'k': result *= 1000; break;
			default: *x++ = *end;
		}
	}
	*x = '\0';
	result = CLIP(result, DBL_MAX); // WARNING
	
	return result;
}

int * getIntArr(char * str, int size, char * seperators, int base, int rightjust) {
	int * arr = malloc(size * sizeof(int));
	int i;
	if(base < 2) { return NULL; } // Invalid Base
	
	// Extract integers
	char *c = str, *d, *x = xchars = realloc(xchars, strlen(str)+1);
	*x = '\0';
	for(i = 0; *c; c++) {
		if(isdigit(*c)) {
			arr[i++] = (int)strtoul(c, &c, base);
			if(i >= size) { break; }
		}
		else {
			for(d = seperators; *d; d++) {
				if(*c == *d) { break; }
			}
			if(!*d){ *x++ = *c; }
		}
	}
	*x = '\0';
	
	// Shift and zero incomplete array
	if(rightjust) {
		int j = size - i;
		if(j > 0) {
			for(i = size - 1; i >= 0; i--) {
				if(i-j>=0)	{ arr[i] = arr[i - j]; }
				else 		{ arr[i] = 0; }
			}
		}
	}
	
	return arr;
}

int isValidPath(char * filepath) {
	char *c;
	int hasExt = -1;
	for(c = &filepath[strlen(filepath)]; c > filepath; c--) {
		if(*c == PATH_SEP) { break; }
		else if(*c == PATH_EXT) { hasExt = 1; }
	}
	if(c == filepath) { return 2 * hasExt; } // Curr directory
	
	*c = '\0';
	if(!access(filepath, W_OK|R_OK)) {
		*c = PATH_SEP;
		return 1 * hasExt;
	}
	*c = PATH_SEP;
	return 0;
}

char * appendExt(char * filepath) {
	char * newPath = malloc((strlen(filepath) + 1) * sizeof(char) + sizeof(DEF_FILE_EXT));
	sprintf(newPath,"%s%c%s",filepath,PATH_EXT,DEF_FILE_EXT);
	return newPath;
}





// EXTRACTING PARAMETERS

double getDuration(char * lenStr, Timecode * tc) {
	double mult = 1.0, div = 1.0;
	int end = 0, isArr = 0, decimal = 0;
	char *c;
	for(c = lenStr; *c; c++) {
		if(*c == ' ') { continue; }
		if(end) { return -1; } // Additional garbage
		if(isdigit(*c)) { continue; }
		if(*c == '.') {
			if(decimal) { return -1; }
			decimal++;
			continue;
		}
		switch(tolower(*c)) {
			case ':':
			case ';':
				if(decimal) { return -1; }
				isArr++;
				break;
			case 'f':
				if(!tc) { return -1; } // No framerate
				div = tc->frameRate;
				end++;
				break;
			case 'h':
				mult *= 60.0;
			case 'm':
				if(tolower(c[1]) == 's')
					{ div = 100.0; c++; }
				else
					{ mult *= 60.0; }
			case 's':
				end++;
				break;
			default:
				return -1;
		}
		if(isArr + decimal > 3) { return -1; } // To many array spots
		if(isArr && (mult-1 || div-1)) { return -1; } // Array + letter
	}
	
	if(mult-1 && div-1) { return -1; } // Input has '...hs'
	
	double secs;
	if(isArr) {
		// Get array
		int *lenArr = getIntArr(lenStr, 4, ".;:", (int)dec, 1);
		if(!lenArr || strlen(xchars) > 0) { return -1; }
		// ADD GET_TIME IF THIS FAILS
		
		// Get seconds from array
		/*  isArr+decimal values:
				1+0	   mm:ss
				1+1	   mm:ss.ms
				2+0	hh:mm:ss
				2+1	hh:mm:ss.ms
				3+0	hh:mm:ss:ff   */
		if(isArr < 3) {
			secs = (double)getTCarrFrames(
				lenArr,
				(decimal ? SEC_TO_MS : MIN_TO_SEC), 0 // "frame" duration
			) / (double)(decimal ? SEC_TO_MS : 1); // convert ms to s
		}
		else
			{ secs = (double)getTCarrFrames(lenArr, tc->frameRate, 0); }
		
		if(secs < 0) { return secs; }
		return secs;
	}
	
	// Just number
	secs = getFloat(lenStr);
	if(secs < 0) { return -1; }
	return secs * mult / div;
}

WaveGens getGenerator(char * gen) {
	// WARNING ON BLANK
	if(strscmp(gen, 3, "", "silence", "none"))
		return GEN_SILENCE;
	if(strscmp(gen, 2, "sine", "tone"))
		return GEN_SINE;
	if(strscmp(gen, 1, "triangle"))
		return GEN_TRIANGLE;
	if(strscmp(gen, 2, "sawtooth", "saw"))
		return GEN_SAWTOOTH;
	if(strscmp(gen, 1, "square"))
		return GEN_SQUARE;
	if(strscmp(gen, 3, "tc", "ltc", "timecode"))
		return GEN_TIMECODE;
	if(strscmp(gen, 1, "debug", "header", "blank"))
		return GEN_HEADER;
	return -1;
}


int setFramerate(char * fpsStr, Timecode * tc) {
	// Check for pulldown
	int isFloat = 0;
	char *c;
	for(c = fpsStr; *c; c++) {
		if(*c == '.' && !isFloat) { isFloat++; }
		else if(!isdigit(*c)) { break; }
	}
	
	// Check for drop frame
	int ndf[3] = { 0, 0, 0 };
	for( ; *c; c++) {
		switch(tolower(*c)) {
			case 'n':
				if(ndf[0]||ndf[1]||ndf[2]) { return -1; }
				ndf[0]++;
				break;
			case 'd':
				if(ndf[1]||ndf[2]) { return -1; }
				ndf[1]++;
				break;
			case 'f':
				if(ndf[2]) { return -1; }
				ndf[2]++;
			case ' ':
				break;
			default:
				return -1;
		}
	}
	
	// Get/Set framerate
	int fps = getInt(fpsStr, 2, 0);
	if(fps < 1) { return -1; }
	for(c = xchars; *c; c++) {
		if(!arrayMatch((int)tolower(*c), NDF_VALID, NDF_SIZE))
			{ return -1; }
	}
	if(arrayMatch(fps, &TC_PULLED[0], TC_PULLED_LEN)) {
		// For rates like '23, 29.97, etc'
		isFloat = 1;
		fps++;
	}
	tc->frameRate = (unsigned char)fps;
	
	// Set flags
	if(isFloat)	{ tc->isPullDown = 1; }
	else		{ tc->isPullDown = 0; }
	if(ndf[0]) 		{ tc->isDropFrame = 0; }
	else if(ndf[1])	{ tc->isDropFrame = 1; }
	return 0;
}


int setWaveTimecode(char * TCstr, ByteFormat format, Timecode * tc) {
	int f;
	
	// Set current clock-time
	if(strscmp(TCstr, 1, "now")) {
		if(setAsCurrentTime(tc)) { return -1; }
		return 0;
	}
	
	// Get array
	int *TCarr = getIntArr(TCstr, 4, ";:", (int)format, 1);
	if(!TCarr || strlen(xchars) > 0) {
		// Get using audioLen format
		double secs = getDuration(TCstr, tc);
		f = (int)round(secs * tc->frameRate);
	}
	else {
		// Set TC to array
		f = getTCarrFrames(TCarr, tc->frameRate, tc->isDropFrame);
	}
	if(f < 0) { return f; } // Error
	tc->frames = f;
	return 0;
}


int setWaveUserBits(char * ubStr, ByteFormat format, Timecode * tc) {
	// Set current date
	if(strscmp(ubStr, 1, "today")) {
		if(setAsCurrentDate(tc)) { return -1; }
		return 0;
	}
	
	// Determine if it's a string
	char *c;
	int count = 0;
	for(c = ubStr; *c; c++) {
		if(!isdigit(*c)) { count++; }
	}
	if(count > 4) { format = chr; } // Read as String
	
	// Get integer arr
	int *ub = malloc(4 * sizeof(int));
	if(format != chr) {	
		ub = getIntArr(ubStr, 4, " :;,/-_.", (int)format, 0);
		if(ub && strlen(xchars) < 1) {
			setUserBitsArr(tc, ub);
			free(ub);
			return 0;
		}
		// WARNING
	}
	
	// Get string, set first 4 chars
	int u = 0;
	for(c = ubStr; *c; c++) {
		if(*c < 0x21 || *c > 0x7E) { continue; } // WARNING
		if(u > 3) { break; }
		ub[u++] = *c;
	}
	// WARNING IF !*c
	
	setUserBitsArr(tc, ub);
	tc->userBitsFmt = ub_text;
	free(ub);
	return 0;
}

int setTCFormat(char * fmtStr, Timecode * tc) {
	// WARNING ON BLANK
	if(strscmp(fmtStr, 4, "", "none", "unspec", "unspecified")) {
		tc->tcClockFmt = tc_unspecified;
		tc->userBitsFmt = ub_unspecified;
	}
	else if(strscmp(fmtStr, 2, "tc", "timecode"))
		tc->tcClockFmt = tc_unspecified;
	else if(strscmp(fmtStr, 2, "ub", "userbits"))
		tc->userBitsFmt = ub_unspecified;
	else if(strscmp(fmtStr, 2, "clock", "clocktime"))
		tc->tcClockFmt = tc_clocktime;
	else if(strscmp(fmtStr, 3, "dt", "date", "day"))
		tc->userBitsFmt = ub_date;
	else if(strscmp(fmtStr, 3, "pg", "page", "line"))
		tc->userBitsFmt = ub_page;
	else if(strscmp(fmtStr, 5, "str", "string", "text", "ascii", "word"))
		tc->userBitsFmt = ub_text;
	else
		return -1; // None found
	return 0;
}





// MAIN FUNCTIONS

WaveData * getWaveFromArgs(int argc, char **argv) {
	WaveData * data = getWaveDefault();
	int c, temp, lo, help = 0, inv = -1, df = -1, pd = -1;
	double tempf;
	char *ub = NULL, *tc = NULL, *fmt = NULL, *dur = NULL;
	ByteFormat read_as = (DEF_UB_FMT == ub_text ? chr : DEF_BYTE_FORMAT);
    
    // GetOpts from user-input
	while((c = getopt_long_only(argc, argv, SHORT_OPTIONS, long_opts, &lo)) != -1) {
		switch (c) {
			
			case 'o':
				temp = isValidPath(optarg);
				if(temp) {
					if(temp < 0) { optarg = appendExt(optarg); }
					data->fname = malloc((strlen(optarg) + 1) * sizeof(char));
					strcpy(data->fname, optarg);
				}
				else { printf("'%s' is an invalid path.\n", optarg); }
				break;
				
			case 'g':
				temp = getGenerator(optarg);
				if(temp < 0) { printf("'%s' generator not recognized.\n", optarg); }
				else { data->generator = temp; }
				break;
			
			case 'b':
				temp = getInt(optarg, 2, 0);
				if(temp < 1)
					{ printf("Bit-rate must be a positive integer ('%s').\n", optarg); }
				else if(strlen(xchars) > 0)
					{ printf("Bit-rate contains invalid characters ('%s').\n", xchars); }
				else { data->bitRate = temp; }
				break;
			
			case 's':
				tempf = getFloat(optarg);
				if(tempf < 1.0)
					{ printf("Sample rate must be a positive number ('%s').\n", optarg); }
				else if(strlen(xchars) > 0)
					{ printf("Sample rate contains invalid characters ('%s').\n", xchars); }
				else {
					if(tempf < 1000.0) { tempf *= 1000.0; }
					data->sampleRate = (int)round(tempf);
				}
				break;

			case 'c':
				temp = getInt(optarg, 2, 0);
				if(temp < 1)
					{ printf("Channel count must be a positive integer ('%s').\n", optarg); }
				else if(strlen(xchars) > 0)
					{ printf("Channel count contains invalid characters ('%s').\n", xchars); }
				else { data->channels = temp; }
				break;
			
			case 'v':
				tempf = getFloat(optarg);
				if(getErr)
					{ printf("Volume must be in amplitude, percentage or dBFS (%s).\n", optarg); }
				if(tempf > 100.0)
					{ printf("Volume exceeds maximum range (%s).\n", optarg); }
				else if(strlen(xchars) > 0)
					{ printf("Volume contains invalid characters ('%s').\n", xchars); }
				else {
					if(tempf > 1.0) { tempf = tempf / 100.0; }
					else if(tempf < 0.0) { tempf = DB_TO_FLOAT(tempf); }
					data->volume = tempf;
				}
				break;
			
			case 'q':
				tempf = getFloat(optarg);
				if(tempf < 1.0) { printf("Frequency must be a positive number ('%s').\n", optarg); }
				else if(strlen(xchars) > 0)
					{ printf("Frequency contains invalid characters ('%s').\n", xchars); }
				else { data->frequency = tempf; }
				break;
			
			case 'f':
				if(setFramerate(optarg, data->startTC))
					{ printf("Framerate not recognized ('%s').\n",optarg); }
				break;
				
			// Must be set after loop
			case 'l': // Same as 'd'
			case 'd': dur = optarg; break;
			case 't': tc  = optarg; break;
			case 'u': ub  = optarg; break;
			case 'm': fmt = optarg; break;
			case 'h': help = 1; break;
			
			// Quick-set: Byte-format, drop-frame, pull-down
			case 0x80: read_as = hex; break;
			case 0x81: read_as = dec; break;
			case 0x82: read_as = oct; break;
			case 0x83: read_as = bin; break;
			case 0x84: read_as = chr; break;
			case 0x90: df = 0; break;
			case 0x91: df = 1; break;
			case 0x92: pd = 1; break;
			case 0xA0: inv = (inv < 0 ? 1 : !inv); break;
			
			// Unrecognized
			case 0:
			case '?':
				break;
			default:
				printf(
					"ERROR: Unexpected value while setting options. (getopt: 0x%X='%s').\n",
					c, optarg
				);
				return NULL;
		}
	}
	if(ub && setWaveUserBits(ub, read_as, data->startTC))
		{ printf("User-bits not recognized ('%s').\n",ub); }
	if(df != -1)
		{ data->startTC->isDropFrame = (df ? 1 : 0); }
	if(pd != -1)
		{ data->startTC->isPullDown = (pd ? 1 : 0); }
	if(tc && setWaveTimecode(tc, dec, data->startTC))
		{ printf("Timecode not recognized ('%s').\n",tc); }
	if(fmt && setTCFormat(fmt, data->startTC))
		{ printf("Format argument not recognized ('%s').\n",fmt); }
		
	if(df != -1) // Force if changed by TC/Format
		{ data->startTC->isDropFrame = (df ? 1 : 0); }
	
	if(dur) {	
		tempf = getDuration(dur, data->startTC);
		if(tempf < 0) { printf("Audio length not recognized ('%s').\n", dur); }
		else { data->seconds = tempf; }
	}
	
	if(inv < 0) { inv = DEF_INVERT; }
	if(inv) { data->volume = -(data->volume); }
	
	if(verbose) {
		// PRINT ALL OPTIONS HERE
		printf("Verbose mode enabled.\n");
	}
	
	// Print unrecognized arguments
	if(optind < argc) {
		printf("WARNING: Skipped unknown options: ");
		while(optind < argc) {
			printf("%s ", argv[optind++]); }
		putchar('\n');
	}
	
	if(help || optind < 2) { printOptions(); return NULL; }
		
	return data;
}

int main(int argc, char **argv) {
	printf(WAV_BUILD_HEADER"\n");
	WaveData * data = getWaveFromArgs(argc, argv);
	int err = errorCheck(data);
	if(!err) { printWaveData(data); }
	return err ? err : makeWave(data);
}