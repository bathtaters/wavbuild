# wavbuild
Audio File Sound Wave &amp; LTC Generator (My first C program)


Help page:
Wav Build (Version 0.1) (c) 2018 by Nice Sound.
Usage: wavbuild -<option> <value> ... -o <file>

Wav Build Options:
 -o	-out	Output file <filepath> [Default: "./out.wav"]
 -h	-help	Display list of commands

Audio Options:
 -b	-br	Bit-rate <8|16|24|32> bits per sample [Default: 16-bit]
 -s	-sr	Sample rate <11025-192k> Hz [Default: 44100 Hz]
 -d	-dur	Duration <(hh:)mm:ss(.ms|:ff)> (Or append with <h|m|s|ms|f>) [Default: 60 secs]

Generator Options:
 -g	-gen	Select generator <Silence|Sine|Sawtooth|Triangle|Square|LTC> [Default: Silence]
 -v	-vol	Volume <Percentage|Amplitude (Max=1)|dBFS (Max=0)> [Default: -6 dBFS]
 -q	-freq	Generator frequency <Hz> (Not used for LTC) [Default: 1000 Hz]
 -inv	-invertInvert generator wave (Vertical flip)

Timecode Options: (For LTC)
 -t	-tc	Timecode of first frame (See "-dur" for format, or <Now> for current time)
 -f	-fps	Frame Rate <23|24|25|29|30|50|59|60> frames per second [Default: 24 fps]
 -df	-drop	Force drop-frame counting (Only in 29.97/30)
 -nd	-ndf	Force non-drop-frame counting (Only with 29.97/30) [Default]
 -pd	-pull	Force pull-down of integer fps (Only with 24/30)

User-bits Options: (For LTC)
 -u	-ub	User-bits <u1:u2:u3:u4> (Also <4-char string> or <Today> for current date)
 -<base>	Input format of user-bits <-hex|-dec|-oct|-bin|-str> [Default: Hex]

Timecode/User-Bits Format:
 -m	-fmt	Force TC/UB format <format options (see below)>
	"none"	Unspecified TC/UB [Default]
	"tc"	Unspecified Timecode only
	"clock"	Timecode is clock-time
	"ub"	Unspecified User-Bits only
	"date"	User-bits are date
	"str"	User-bits are ASCII text
	"page"	User-bits are page/line
