/*
 * Copyright (c) 2005,2006 Petr Machata
 * All rights reserved.
 */

#ifndef _AL60L_CURSOR_H_
#define _AL60L_CURSOR_H_

#include "cursor.i"
#include "pd.h"

/// Create new cursor.  The cursor points to the beginning of file
/// with given name.
cursor_t * new_cursor (char const* filename, int line)
  ATTRIBUTE_MALLOC;

/// Create new cursor by copying given other cursor.
cursor_t * clone_cursor (cursor_t * cursor)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(1);

/// Destroy given cursor.
void delete_cursor (cursor_t * cursor);

/// Convert void* to cursor, if it is cursor, or abort.
cursor_t * a60_as_cursor (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Move the point the cursor points to `offset' characters ahead.
void cursor_move (cursor_t * cursor, int offset)
  ATTRIBUTE_NONNULL(1);

/// Simulate tab movement with the cursor.
/// \arg tabsize Cursor column is rounded up to multiple of this
/// number.
void cursor_tab (cursor_t * cursor, int tabsize)
  ATTRIBUTE_NONNULL(1);

/// Move the point to the new line.  This adds one to offset,
/// increases line number, and sets column number to zero.
void cursor_nl (cursor_t * cursor)
  ATTRIBUTE_NONNULL(1);

/// Answer offset.  The first character of file has the offset of
/// zero.
int cursor_offset (cursor_t * cursor)
  ATTRIBUTE_NONNULL(1);

/// Answer line number.  The first line has the number zero.
int cursor_line (cursor_t * cursor)
  ATTRIBUTE_NONNULL(1);

/// Answer column name.  Zero is the first column.
int cursor_column (cursor_t * cursor)
  ATTRIBUTE_NONNULL(1);

/// Answer name of pointed-to file.
char const* cursor_file (cursor_t * cursor)
  ATTRIBUTE_NONNULL(1);

/// Render cursor into a string.  The returned string is static, so
/// handle with care.
char const* cursor_to_str (cursor_t * cursor)
  ATTRIBUTE_NONNULL(1);

#ifdef IN_GCC
/// Convert cursor to location_t when in GCC.  `locptr' is pointer to
/// location_t where the result should be stored.  It's void* instead
/// of location_t* to keep away from GCC headers (mainly to get rid of
/// GCC poisoning mechanism).
void cursor_to_loc (cursor_t const * cursor, void * locptr)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);
#endif

#endif //_AL60L_CURSOR_H_
