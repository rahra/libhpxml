CC			= gcc
CFLAGS	= -Wall

libhpxml.a: libhpxml.o bstring.o
	ar cr libhpxml.a libhpxml.o bstring.o

bstring.o: bstring.c bstring.h

libhpxml.o: libhpxml.c libhpxml.h bstring.h

clean:
	rm -f *.o libhpxml.a

.PHONY: clean

