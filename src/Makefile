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
CFLAGS	= -Wall -DWITH_MMAP

libhpxml.a: libhpxml.o bstring.o
	ar cr libhpxml.a libhpxml.o bstring.o

bstring.o: bstring.c bstring.h

libhpxml.o: libhpxml.c libhpxml.h bstring.h

clean:
	rm -f *.o libhpxml.a

.PHONY: clean

