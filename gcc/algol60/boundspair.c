#include <stdlib.h>
#include <assert.h>
#include "meta.h"

#include "boundspair.h"

static char const * private_boundspair_signature = "boundspair";

struct struct_boundspair_t
{
  char const * signature;
  expression_t * lobound;
  expression_t * hibound;
};


boundspair_t *
new_boundspair (expression_t * lo, expression_t * hi)
{
  boundspair_t * ret = malloc (sizeof (boundspair_t));
  ret->signature = private_boundspair_signature;
  ret->hibound = hi;
  ret->lobound = lo;
  return ret;
}

void
delete_boundspair (boundspair_t * self)
{
  free (self);
}

boundspair_t *
boundspair (void * ptr)
{
  A60_CHECKED_CONVERSION (boundspair, ptr);
}

expression_t *
boundspair_hi (boundspair_t const * self)
{
  assert (self != NULL);
  return self->hibound;
}

expression_t *
boundspair_lo (boundspair_t const * self)
{
  assert (self != NULL);
  return self->lobound;
}

void
boundspair_set_hi (boundspair_t * self, expression_t * bound)
{
  assert (self != NULL);
  self->hibound = bound;
}

void
boundspair_set_lo (boundspair_t * self, expression_t * bound)
{
  assert (self != NULL);
  self->lobound = bound;
}
