CC			= gcc
CFLAGS	= -Wall -Isrc

all: example

example: example.o libhpxml.a

example.o: example.c

libhpxml.a:
	make -C src
	cp src/libhpxml.a .

clean:
	rm -f libhpxml.a
	make -C src clean

.PHONY: clean

