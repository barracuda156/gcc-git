#ifndef _AL60L_LABEL_H_
#define _AL60L_LABEL_H_

///
/// \file label.h
///
/// Label identifies the symbol. Labels are usually string names, but
/// in Algol 60, also numerical labels are possible.  String labels
/// are used to identify variables and functios, numerical labels are
/// used to identify goto targets. Both label kinds are handled the
/// same, the second kind contains numerical identifier inside the
/// string.
///
/// Label can hold list of assigned bounds pairs.  This is used to
/// represent declaration of an array.  Details of handling the array
/// bounds are described in parser.y at block declaration handling
/// code.
///

#include "pd.h"
#include "label.i"
#include "estring.i"
#include "boundspair.i"
#include "slist.i"
#include "cursor.i"

/// Create new label.
label_t * new_label (estring_t const * id)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1);

/// Destroy the label.
void delete_label (label_t * self);

/// Convert void* to label, if it is label, or abort.
label_t * a60_as_label (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Answer the label identifier.
estring_t const * label_id (label_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Render label into string.  Buf may be NULL, new one would be
/// allocated.  Returns either buf, or newly allocated buffer if buf
/// was NULL.
estring_t * label_to_str (label_t const * self, estring_t * buf)
  ATTRIBUTE_NONNULL (1);

/// Answer if the two labels are the same.
int label_eq (label_t const * lhs, label_t const * rhs)
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2);

/// Add boundspair list to the label.  It must not already be present,
/// which is guarded by an assertion.  Added slist is typed:
///   new_slist_typed (adapt_test, boundspair);
/// Created slist is returned, and you are free to manipulate with it
/// directly.
slist_t * label_add_boundspairs (label_t * self)
  ATTRIBUTE_NONNULL (1);

/// Add boundspair list to the label and copy contents of given list
/// inside.
slist_t * label_add_boundspairs_with (label_t * self, slist_t * bps)
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2);

/// Remove boundspair list to the label.  It must be present, which is
/// guarded by an assertion.
void label_remove_boundspairs (label_t * self)
  ATTRIBUTE_NONNULL (1);

/// Request the underlying list of bounds pairs for direct
/// manipulation.  You can add or remove bounds pairs as you wish,
/// label itself doesn't use the list.
slist_t * label_boundspairs (label_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Assign a cursor to the label.  It must not already be present,
/// which is guarded by an assertion.
void label_set_cursor (label_t * self, cursor_t * cursor)
  ATTRIBUTE_NONNULL (1);

/// Get a cursor associated with the label.
cursor_t * label_cursor (label_t const * self)
  ATTRIBUTE_NONNULL (1);

#endif//_AL60L_LABEL_H_
