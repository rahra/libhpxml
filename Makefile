#/* Copyright 2011 Bernhard R. Fischer, 2048R/5C5FFD47 <bf@abenteuerland.at>
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

