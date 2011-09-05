#ifndef BSTRING_H
#define BSTRING_H


typedef struct bstring
{
   int len;
   char *buf;
} bstring_t;


int bs_advance(bstring_t *);
int bs_advance2(bstring_t *);
int bs_ncmp(bstring_t b, const char *s, int n);
int bs_cmp(bstring_t b, const char *s);
long bs_tol(bstring_t b);
double bs_tod(bstring_t b);

#endif

