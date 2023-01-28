#ifndef VISITOR_IMPL_H_GUARD
#define VISITOR_IMPL_H_GUARD

#include "visitor.h"

typedef
struct struct_visitable_t
{
#ifndef NDEBUG
  char const * signature;
#endif
  unsigned int kind;
}
visitable_t;

visitor_t * a60_build_generic_visitor (char const* const* signature, int count, ...);

#endif /* VISITOR_IMPL_H_GUARD */
