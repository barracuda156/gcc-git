#include "meta.h"

#include <stdlib.h>
#include <stdio.h>

void
a60_check_access (void * obj, char const* requested_signature)
{
  char const * signature = *((char const* const*)obj);
  if (signature != requested_signature)
    {
      fprintf (stderr,
	       "Invalid access: object %p of type %p (%s) accessed as type %p ",
	       obj, requested_signature, requested_signature, signature);
      /* split message to print at least something, should the
	 following sigsegv: */
      fprintf (stderr, "(%s).\n", signature);
      abort ();
    }
}
