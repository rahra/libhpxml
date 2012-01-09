#include <stdio.h>
#include <stdlib.h>

#include "bstring.h"
#include "libhpxml.h"


int main(int argc, char *argv[])
{
   hpx_ctrl_t *ctl;
   hpx_tag_t *tag;
   bstring_t b;
   size_t lno;

   // initialize control structure, stdin, 100MB buffer
   if ((ctl = hpx_init(0, 100*1024*1024)) == NULL)
      perror("hpx_init"), exit(EXIT_FAILURE);
   // initialize tag structure with maximum 16 attributes
   if ((tag = hpx_tm_create(16)) == NULL)
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

