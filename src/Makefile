#/* Copyright 2011-2024 Bernhard R. Fischer, 4096R/8E24F29D <bf@abenteuerland.at>
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

# Define either HAVE_MADVISE or HAVE_POSIX_MADVISE typically with a
# configure script in a real project. If you define neither of them, libhpxml
# does still work.
CC			= gcc
CFLAGS	= -Wall -DWITH_MMAP -DHAVE_MADVISE

libhpxml.a: libhpxml.o bstring.o
	ar cr libhpxml.a libhpxml.o bstring.o

bstring.o: bstring.c bstring.h

libhpxml.o: libhpxml.c libhpxml.h bstring.h

clean:
	rm -f *.o libhpxml.a

.PHONY: clean

