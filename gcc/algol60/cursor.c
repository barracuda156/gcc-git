/*
 * Copyright (c) 2005,2006 Petr Machata
 * All rights reserved.
 */

#ifndef SELF_TEST

#include "cursor.h"
#include "util.h"
#include "visitor-impl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char const* private_cursor_signature = "cursor";

struct struct_cursor_t
{
  visitable_t base;

  int offset;
  int line;
  int column;
  char * filename;
  int fnlen;
};

cursor_t *
new_cursor (char const* filename, int line)
{
  cursor_t * ret = malloc (sizeof (cursor_t));

#ifndef NDEBUG
  ret->base.signature = private_cursor_signature;
#endif
  ret->offset = ret->column = 0;
  ret->line = line;

  ret->fnlen = strlen (filename);
  ret->filename = a_strdup (filename);

  return ret;
}

cursor_t *
clone_cursor (cursor_t * cursor)
{
  cursor_t * ret = new_cursor (cursor->filename, cursor->line);

  ret->offset = cursor->offset;
  ret->column = cursor->column;

  return ret;
}

void
delete_cursor (cursor_t * cursor)
{
  if (cursor != NULL)
    {
      free (cursor->filename);
      free (cursor);
    }
}

cursor_t *
a60_as_cursor (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_cursor_signature);
#endif
  return (cursor_t *)obj;
}

void
cursor_move (cursor_t * cursor, int offset)
{
  cursor->offset += offset;
  cursor->column += offset;
}

void
cursor_tab (cursor_t * cursor, int tabsize)
{
  cursor->offset++;
  cursor->column = ((cursor->column + 1) / tabsize + 1) * tabsize;
}

void
cursor_nl (cursor_t * cursor)
{
  cursor->offset++;
  cursor->line++;
  cursor->column = 0;
}

int
cursor_offset (cursor_t * cursor)
{
  return cursor->offset;
}

int
cursor_line (cursor_t * cursor)
{
  return cursor->line;
}

int
cursor_column (cursor_t * cursor)
{
  return cursor->column;
}

char const*
cursor_file (cursor_t * cursor)
{
  return cursor->filename;
}

char const*
cursor_to_str (cursor_t * cursor)
{
  static char * buf = NULL;
  static int bufsize = 0;

  if (bufsize < cursor->fnlen + 64
      || buf == NULL)
    {
      bufsize = cursor->fnlen + 64;
      buf = realloc (buf, bufsize);
    }

  if (cursor->fnlen == 0)
    // don't display empty filename
    sprintf (buf, "%d", cursor->line);
  else
    sprintf (buf, "%s:%d", cursor->filename, cursor->line);

  return buf;
}

#ifdef IN_GCC
// We nedd to have this at the very bottom of the definitions,
// otherwise our memory management would get poisoned.
# include "system.h"
# include "coretypes.h"
# include "limits.h"
# include "input.h"

void
cursor_to_loc (cursor_t const * cursor, void * locptr)
{
  gcc_assert (cursor != NULL);
  gcc_assert (locptr != NULL);
  location_t * loc = locptr;
  memset (loc, 0, sizeof (location_t));
  loc->file = cursor->filename;
  loc->line = cursor->line;
}
#endif

#else /* SELF_TEST */

#include "cursor.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int
main (void)
{
  cursor_t * c = new_cursor ("<stdin>", 0);
  assert (cursor (c));
  cursor_t * c1 = NULL;
  int line = 0;
  static char * str = "0123456701234\nhallo\tworld\nhi\tworld\nhallo   world\n";

  for (char * it = str; *it; ++it)
    {
      assert (str [cursor_offset (c)] == *it);
      assert (cursor_line (c) == line);
      if (*it == '\n')
	{
	  printf ("nl:  %s (%d)\n", cursor_to_str (c), cursor_offset (c));

	  // all lines have the same length
	  if (c1 == NULL)
	    {
	      c1 = new_cursor_copy (c);
	      assert (cursor (c1));
	    }
	  else
	    assert (cursor_column (c) == cursor_column (c1));

	  cursor_nl (c);
	  ++line;
	}
      else if (*it == '\t')
	{
	  printf ("tab: %s (%d) -> ", cursor_to_str (c), cursor_offset (c));
	  cursor_tab (c, 8);
	  printf ("%s (%d)\n", cursor_to_str (c), cursor_offset (c));
	  assert (cursor_column (c) % 8 == 0);
	}
      else
	cursor_move (c, 1);
    }

  int i = cursor_column (c);
  int j = cursor_offset (c);
  int k = cursor_line (c);
  cursor_move (c, 5);
  assert (cursor_column (c) == i+5);
  assert (cursor_offset (c) == j+5);
  assert (cursor_line (c) == k);

  assert (strcmp (cursor_file (c), "<stdin>") == 0);
  assert (strcmp (cursor_file (c1), "<stdin>") == 0);

  delete_cursor (c);
  assert (strcmp (cursor_file (c1), "<stdin>") == 0);

  delete_cursor (c1);

  printf ("All passed.\n");
  return 0;
}

#endif
