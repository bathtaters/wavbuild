IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)
OUTDIR=/usr/local/bin/

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = wav_build_ui.h wav_build.h audio_generate.h ltc_generate.h timecode.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = wav_build_ui.o wav_build.o audio_generate.o ltc_generate.o timecode.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)wavbuild: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 


