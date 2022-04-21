CC=gcc
FLAGS= -g -Wall -Wno-implicit -Wno-unused
LIBS = -lm

OBS = segment.o streamer.o metharm.o globals.o main.o
HEADERS = polyph.h

polyph: $(OBS)
	$(CC) $(CFLAGS) $(OBS) $(LIBS) -o polyph

$(OBS): $(HEADERS)

clean:
	rm -f *.o *~
	/bin/rm polyph
