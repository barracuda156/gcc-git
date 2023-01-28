#ifndef _AL60L_BOUNDSPAIR_H_
#define _AL60L_BOUNDSPAIR_H_

///
/// \file boundspair.h
///
/// Small module that takes care of bounds pairs.  Bounds pairs are
/// AST nodes that represent array bounds.  Note that we handle
///

#include "pd.h"
#include "boundspair.i"
#include "expression.i"

/// Create new bounds pair.  `hi` and `lo` are bounds expressions, and
/// they can be NULL if wished.
boundspair_t * new_boundspair (expression_t * lo, expression_t * hi)
  ATTRIBUTE_MALLOC;

/// Delete the bounds pair.  This doesn't delete the bounds
/// expressions themselves.
void delete_boundspair (boundspair_t * self);

/// Convert void* to boundspair, if it is boundspair, or return NULL.
boundspair_t * boundspair (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Answer upper bound.
expression_t * boundspair_hi (boundspair_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer lower bound.
expression_t * boundspair_lo (boundspair_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Set upper bound.
void boundspair_set_hi (boundspair_t * self, expression_t * bound)
  ATTRIBUTE_NONNULL(1);

/// Set lower bound.
void boundspair_set_lo (boundspair_t * self, expression_t * bound)
  ATTRIBUTE_NONNULL(1);

#endif//_AL60L_BOUNDSPAIR_H_
