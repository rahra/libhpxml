CC			= gcc
CFLAGS	= -Wall -Isrc
SVNVER	= $(shell svnversion | tr -d -c '[:digit:]')
DISTDST	= libhpxml-r$(SVNVER)

all: example

example: example.o libhpxml.a

example.o: example.c

libhpxml.a:
	make -C src
	cp src/libhpxml.a .

clean:
	rm -f libhpxml.a
	make -C src clean

dist:
	mkdir -p $(DISTDST)/src
	cp Makefile example.c $(DISTDST)
	cp src/Makefile src/bstring.c src/bstring.h src/libhpxml.c src/libhpxml.h $(DISTDST)/src
	tar cvfj $(DISTDST).tbz2 $(DISTDST)


.PHONY: clean dist

