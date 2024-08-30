#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bstring.h"
#include "libhpxml.h"


int is_doctype(const hpx_tag_t *tag)
{
   return tag->type == HPX_ATT && tag->tag.len >= 8 && isspace(tag->tag.buf[7]) && !strncasecmp(tag->tag.buf, "DOCTYPE", 7);
}


int proc_subset(hpx_tag_t *tag)
{
   hpx_ctrl_t _ctl, *ctl = &_ctl;
   int i, sqcnt;
   bstring_t b;
   long lno;

   printf("===== BEGIN PROCESS SUBSET=====\n");

   b = tag->tag;

   // skip until opening tag '['
   for (; b.len && *b.buf != '['; b.buf++, b.len--);

   // no data or start tag
   if (b.len < 2 || *b.buf != '[')
      return -1;

   // find end tag ']'
   for (i = 0, sqcnt = 0; i < b.len; i++)
   {
      if (b.buf[i] == '[')
      {
         sqcnt++;
      }
      else if (b.buf[i] == ']')
      {
         sqcnt--;
         if (!sqcnt)
            break;
      }
   }

   // no closing tag or too short?
   if (i >= b.len || i < 2)
      return -1;

   //printf("i = %d, len = %d, \"%.*s\"\n", i, b.len, b.len, b.buf);

   // remove enclosing square brackets
   b.len = i - 2;
   b.buf++;

   //printf("i = %d, len = %d, \"%.*s\"\n", i, b.len, b.len, b.buf);

   hpx_init_membuf(ctl, b.buf, b.len);

   // loop as long as XML elements are available
   while (hpx_get_elem(ctl, &b, NULL, &lno) > 0)
   {
      // parse XML element
      if (!hpx_process_elem(b, tag))
      {
         // element successfully parsed, do something with it
         // ...
         // ...

         printf("[%ld] type=%d, name=%.*s, nattr=%d\n", lno, tag->type, tag->tag.len, tag->tag.buf, tag->nattr);
      }
      else
         printf("[%ld] ERROR in element: %.*s\n", lno, b.len, b.buf);
   }

   printf("===== END PROCESS SUBSET =====\n");
   return 0;
}


int main(int argc, char *argv[])
{
   hpx_ctrl_t *ctl;
   hpx_tag_t *tag;
   bstring_t b;
   long lno;

   // initialize control structure, stdin, 100MB buffer
   if ((ctl = hpx_init(0, 100*1024*1024)) == NULL)
      perror("hpx_init"), exit(EXIT_FAILURE);
   // initialize tag structure with maximum 16 attributes
   if ((tag = hpx_tm_create(64)) == NULL)
      perror("hpx_tm_create"), exit(EXIT_FAILURE);

   // loop as long as XML elements are available
   while (hpx_get_elem(ctl, &b, NULL, &lno) > 0)
   {
      // parse XML element
      if (!hpx_process_elem(b, tag))
      {
         // element successfully parsed, do something with it
         // ...
         // ...

         printf("[%ld] type=%d, name=%.*s, nattr=%d\n", lno, tag->type, tag->tag.len, tag->tag.buf, tag->nattr);

         // process doctype subtypes
         if (is_doctype(tag))
            proc_subset(tag);
      }
      else
         printf("[%ld] ERROR in element: %.*s\n", lno, b.len, b.buf);
   }

   if (!ctl->eof)
      perror("hpx_get_elem"), exit(EXIT_FAILURE);

   hpx_tm_free(tag);
   hpx_free(ctl);

   exit(EXIT_SUCCESS);
}

