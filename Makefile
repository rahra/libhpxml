#/* Copyright 2011-2025 Bernhard R. Fischer, 4096R/8E24F29D <bf@abenteuerland.at>
# *
# * This file is part of libhpxml.
# *
# * Libhpxml is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, version 3 of the License.
# *
# * Libhpxml is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with libhpxml. If not, see <http://www.gnu.org/licenses/>.
# */

CC			= gcc
CFLAGS	= -Wall -Isrc -O2
SVNVER	= 1.2.0
DISTDST	= libhpxml-r$(SVNVER)

all: example example2 xml2xml

example: example.o libhpxml.a

example2: example2.o libhpxml.a

xml2xml: xml2xml.o libhpxml.a

libhpxml.a:
	make -C src
	cp src/libhpxml.a .

clean:
	rm -f libhpxml.a *.o
	make -C src clean

dist:
	mkdir -p $(DISTDST)/src
	cp Makefile example.c $(DISTDST)
	cp src/Makefile src/bstring.c src/bstring.h src/libhpxml.c src/libhpxml.h $(DISTDST)/src
	tar cvfj $(DISTDST).tbz2 $(DISTDST)


.PHONY: clean dist

