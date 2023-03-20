IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)
OUTDIR=./

ODIR=src
LDIR =./lib

LIBS=-lm

_DEPS = wav_universal.h wav_build_ui.h wav_build.h audio_generators.h ltc_generator.h timecode.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = wav_build_ui.o wav_build.o audio_generators.o ltc_generator.o timecode.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)wavbuild: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 


