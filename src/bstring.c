/* Copyright 2011-2024 Bernhard R. Fischer, 2048R/5C5FFD47 <bf@abenteuerland.at>
 *
 * This file is part of libhpxml.
 *
 * Libhpxml is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Libhpxml is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libhpxml. If not, see <http://www.gnu.org/licenses/>.
 */

/*! \file bstring.c
 * This file contains all functions for the bstrings. Bstrings are string
 * which are stored as a memory structure bstring_t which contains a pointer to
 * the string an the number of bytes within the string. Bstrings are not
 * necessarily \0-terminated. This speeds up XML processing because it allows
 * to use the strings within the files directly instead of copying them
 * somewhere else.
 *
 * \author Bernhard R. Fischer, <bf@abenteuerland.at>
 * \date 2024/09/12
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include "bstring.h"


/*! Advance bstring_t->buf and decrease bstring_t->len. This function does NOT
 * check if string length >= 1 and if b->buf != NULL which might result in
 * buffer underflows or segfaults.
 * @param b Pointer to bstring_t;
 * @return Length of string.
 */
int bs_advance(bstring_t *b)
{
   b->buf++;
   b->len--;
   return b->len;
}


/*! This function is the same as bs_advance() but takes a bstringl_t structure
 * as an argument instead.
 * @param b Pointer to bstringl_t structure.
 * @return Returns the new length of the string.
 */
int bs_advancel(bstringl_t *b)
{
   b->buf++;
   b->len--;
   return b->len;
}


/*! This function is like bs_advance() but does safety checks on pointers and
 * buffer length.
 * @param b Pointer to bstring_t.
 * @return Length of string.
 */
int bs_advance2(bstring_t *b)
{
   if (b == NULL || b->buf == NULL || b->len < 1)
      return 0;
   
   return bs_advance(b);
}


int bs_nadvance(bstring_t *b, int n)
{
   b->buf += n;
   b->len -= n;
   return b->len;
}


/*! bs_ncmp compares exactly n bytes of b and s. If they are equal, 0 is
 * returned. If they are not equal, the return value of strncmp(3) is returned.
 * If the string length of either is less then n, -2 is returned.
 */
int bs_ncmp(bstring_t b, const char *s, int n)
{
   if ((b.len < n) || ((int) strlen(s) < n))
      return -2;
   return strncmp(b.buf, s, n);
}


/*! This function compares a b_string to a regular C \0-terminated character
 * string.
 * @param b String as bstring_t structure.
 * @param s Pointer to C string.
 * @return The function returns an integer less than, equal, or greater than 0
 * exactly like strcmp(3).
 */
int bs_cmp(bstring_t b, const char *s)
{
   char c;

   // compare characters and return difference if they are not equal
   for (; b.len && *s; (void) bs_advance(&b), s++)
      if ((c = *b.buf - *s))
         return c;

   // strings are equal and of equal length
   if (!b.len && !*s)
      return 0;

   // string s is longer than b
   if (*s)
      return -*s;

   // string s is shorter than b
   return *b.buf;
}


static int char2int(char c)
{
   if (c >= '0' && c <='9')
      return c - '0';
   if (c >= 'A' && c <= 'Z')
      return c - 'A' + 10;
   if (c >= 'a' && c <= 'z')
      return c - 'a' + 10;
   return 256;
}


/*! This function converts a character string into a long. The function behaves
 * exactly like strtol(3) as defined by POSIX.1-2008.
 * @param b Parameter of type bstring_t which shall be converted. If the
 * bstring contains invalid data, i.e. len < 0 or buf points to NULL, the
 * function immediately returns -1 and errno is set to EINVAL.
 * @param endptr If not NULL, this variable will receive a pointer to the
 * characters not converted. Please note that this is a pointer within the
 * bstring b, i.e. it is not necessarily \0-terminated.
 * @param base Conversion base, 0 <= base <= 36. If base is set outside of the
 * valid range, 0 is assumed.
 * @return The function returns the converted value as defined in strtol(3).
 */
long bs_tol2(bstring_t b, char **endptr, int base)
{
   int n = 1, d, conv;
   long l;

   if (b.len < 0 || b.buf == NULL)
   {
      errno = EINVAL;
      return -1L;
   }

   if (base < 0 || base > 36)
      base = 0;

   if (endptr != NULL)
      *endptr = b.buf;

   for (; isspace((unsigned) *b.buf) && b.len; bs_advance(&b));

   if (b.len <= 0) return 0;

   switch (*b.buf)
   {
      case '-':
         n = -1;
         /* fall through */
      case '+':
         bs_advance(&b);
         break;
   }

   if (b.len <= 0) return 0;

   // check for hex
   if ((!base || base == 16) && b.len > 2 && b.buf[0] == '0' && (b.buf[1] == 'x' || b.buf[1] == 'X') && char2int(b.buf[2]) < 16)
   {
      base = 16;
      bs_nadvance(&b, 2);
   }
   // check for octal
   if (!base && b.len > 1 && b.buf[0] == '0' && char2int(b.buf[1]) < 8)
   {
      base = 8;
      bs_advance(&b);
   }
   // if neither, set to dec
   if (!base)
      base = 10;

   // convert digits
   for (l = 0, conv = 0; b.len > 0; bs_advance(&b), conv++)
   {
      d = char2int(*b.buf);
      if (d >= base)
         break;
      l *= base;
      l += n * d;

      // check overflow
      if (n == 1 && l < 0)
      {
         errno = ERANGE;
         l = LONG_MAX;
         b.len = 0;
      }
      // check underflow
      else if (n == -1 && l > 0)
      {
         errno = ERANGE;
         l = LONG_MIN;
         b.len = 0;
      }
   }

   if (conv && endptr != NULL)
      *endptr = b.buf;

   return l;
}


/*! This function converts the string in b into a long integer. Currently, it
 * converts only decimal numbers, i.e. it uses a base of 10.
 * @param b String of type bstring_t.
 * @return The function returns the value of the converted string. The
 * conversion stops at the first character which is not between 0 and 9. Thus,
 * it returns 0 if there is no digit at the beginning of the string.
 * FIXME: This function should be improved to something similar to strtol(3).
 */
long bs_tol(bstring_t b)
{
   int n = 1;
   long l = 0;

   if (b.len && *b.buf == '-')
   {
      (void) bs_advance(&b);
      n = -1;
   }

   for (; b.len && *b.buf >= '0' && *b.buf <= '9'; (void) bs_advance(&b))
   {
      l *= 10;
      l += *b.buf - '0';
   }

   return l * n;
}


double bs_tod(bstring_t b)
{
   int n = 0, e;
   double d = 0.0;

   if (b.len && *b.buf == '-')
   {
      (void) bs_advance(&b);
      n = 1;
   }

   for (e = -1; b.len; (void) bs_advance(&b))
   {
      if (*b.buf == '.')
      {
         e++;
         continue;
      }
      if ((*b.buf < '0') || (*b.buf > '9'))
         break;
      if (e >= 0) e++;
      d *= 10.0;
      d += (double) (*b.buf - '0');
   }
   
   for (; e > 0; e--)
      d /= 10.0;

   if (n)
      return -d;

   return d;
}


#ifdef TEST_BSTRING
#include <stdio.h>

int main(int argc, char *argv[])
{
   char *s[] = {"-47.234hhu djj", "foo bar", " +q", "+23", "-23", "", "+", "-", "0", "00", "0x", "0xg", "0xf", "-0xf", "0xfg", "333", " \t33ab", "9223372036854775807", "9223372036854775808", "-9223372036854775807", "-9223372036854775808", "-9223372036854775809", "922337203685477580700a", "9223372036854775807a", "9223372036854775808a", NULL};
   char *e;
   bstring_t b;
   double d;
   long l;
   int c[3];

   b.buf = s[0];
   b.len = strlen(s[0]);;

   d = bs_tod(b);
   l = bs_tol(b);

   c[0] = bs_cmp(b, "-33");
   c[1] = bs_cmp(b, "-47.234");
   c[2] = bs_cmp(b, "absnn");

   printf("%f, %ld, [%d,%d,%d]\n", d, l, c[0], c[1], c[2]);

   for (int i = 0; s[i] != NULL; i++)
   {
      b.buf = s[i];
      b.len = strlen(s[i]);
      errno = 0;
      l = bs_tol2(b, &e, 0);
      printf("str = \"%.*s\", long = %ld, errno = %d, endptr = \"%.*s\"\n", b.len, b.buf, l, errno, b.len - (int) (e - b.buf), e);
   }

   return 0;
}

#endif

