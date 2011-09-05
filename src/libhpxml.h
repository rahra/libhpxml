#ifndef INPLACE_H
#define INPLACE_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "bstring.h"


#define IS_XML1CHAR(x) (isalpha(x) || (x == '_') || (x == ':'))
#define IS_XMLCHAR(x) (isalpha(x) || isdigit(x) || (x == '.') || (x == '-') || (x == '_') || (x == ':'))

#define hpx_init_simple() hpx_init(0, 10*1024*1024)


typedef struct hpx_ctrl
{
   //! data buffer containing pointer and number of bytes in buffer
   bstring_t buf;
   //! file descriptor of input file
   int fd;
   //! flag set if eof
   int eof;
   //! total length of buffer
   int len;
   //! current working position
   int pos;
   //! flag to deter if next element is in or out of tag
   int in_tag;
   //! flag set if data should be read from file
   int empty;
} hpx_ctrl_t;

typedef struct hpx_attr
{
   bstring_t name;
   bstring_t value;
   char delim;
} hpx_attr_t;

typedef struct hpx_tag
{
   bstring_t tag;
   int type;
   size_t line;
   int nattr;
   int mattr;
   hpx_attr_t attr[];
} hpx_tag_t;

typedef struct hpx_tree
{
   hpx_tag_t *tag;
   int nsub;
   int msub;
   struct hpx_tree *subtag[];
} hpx_tree_t;

enum
{
   HPX_ILL, HPX_OPEN, HPX_SINGLE, HPX_CLOSE, HPX_LITERAL, HPX_ATT, HPX_INSTR, HPX_COMMENT
};


size_t hpx_lineno(void);
void hpx_tm_free(hpx_tag_t *t);
hpx_tag_t *hpx_tm_create(int n);
int hpx_process_elem(bstring_t b, int in, hpx_tag_t *p);
hpx_ctrl_t *hpx_init(int fd, int len);
void hpx_free(hpx_ctrl_t *ctl);
int hpx_get_elem(hpx_ctrl_t *ctl, bstring_t *b, int *in_tag, size_t *lno);
int hpx_fprintf_tag(FILE *f, const hpx_tag_t *p);
int hpx_tree_resize(hpx_tree_t **tl, int n);

#endif

