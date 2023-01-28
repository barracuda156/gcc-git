#include "visitor-impl.h"
#include "visitor.h"
#include "meta.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

visitor_t *
a60_build_generic_visitor (char const* const* signature, int count, ...)
{
  int visitor_size = count + A60_IFDEBUG (1, 0);
  void const* * ret = (void const* *)malloc (visitor_size * sizeof (void *));
  void const* * it = ret;
  va_list ap;

#ifndef NDEBUG
  *it++ = (void *)signature;
#endif
  va_start(ap, count);
  while (count--)
    *it++ = va_arg (ap, void const*);
  va_end(ap);

  return (visitor_t*)ret;
}

void *
a60_visitor_dispatch (visitor_t const * visitor, void const * dispatch_on, void * self, void * data)
{
  return (a60_visitor_pick (visitor, dispatch_on))(self, data);
}

callback_t
a60_visitor_pick (visitor_t const * visitor, void const * dispatch_on)
{
  visitable_t * visitable = (visitable_t *)dispatch_on;
  int kind = visitable->kind;

#ifndef NDEBUG
  /* check and skip signature */
  if (visitable->signature != **(char const* const* *)visitor)
    {
      fprintf (stderr,
	       "error: visitor %p for `%s' dispatches over `%s'.\n",
	       (void*)visitor, **(char const* const* *)visitor, visitable->signature);
      abort ();
    }
  kind += 1;
#endif

  return ((callback_t*)visitor)[kind];
}
