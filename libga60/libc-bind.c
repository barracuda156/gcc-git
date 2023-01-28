/* define puts and exit as libga60 internals, before some more general
   mechanism for including other libraries, such as libc, is
   invented. */

#include "config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>

int puts (char const *);

int __a60__puts_PisQ (char const * s);
int
__a60__puts_PisQ (char const * s)
{
  return puts (s);
}

void __a60__out_PviQ (int number);
void
__a60__out_PviQ (int number)
{
  printf ("%d", number);
}

void __a60__out_PvrQ (double number);
void
__a60__out_PvrQ (double number)
{
  printf ("%f", number);
}

void __a60__out_PvsQ (char * number);
void
__a60__out_PvsQ (char * str)
{
  printf ("%s", str);
}

void __a60__out_PvbQ (int boolean);
void
__a60__out_PvbQ (int boolean)
{
  printf ("%s", (boolean & 0x1) ? "true" : "false");
}
#endif

#ifdef HAVE_STDLIB_H
void exit (int);

void __a60__exit_PviQ (int status);

void
__a60__exit_PviQ (int status)
{
  exit (status);
}
#endif
