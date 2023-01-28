/*
 * Copyright (c) 2006 Petr Machata
 * All rights reserved.
 */

#ifndef SELF_TEST

#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char *
a_strdup (char const* str)
{
  char * ret = malloc (strlen (str) + 1);
  strcpy (ret, str);
  return ret;
}

void
guard_ptr (jmp_buf env, int fail_signal, void * ptr)
{
  assert (fail_signal != 0);
  if (ptr == NULL)
    longjmp (env, fail_signal);
}

void
guard_int (jmp_buf env, int fail_signal, int errcode)
{
  assert (fail_signal != 0);
  if (errcode != 0)
    longjmp (env, fail_signal);
}

void *
tmpbuild (void * buf, void* (*buf_creator)(void))
{
  if (buf)
    return buf;
  else
    return buf_creator ();
}


#else /* SELF_TEST */

#include "util.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
test_a_strdup (void)
{
  printf ("strdup\n");

  char const* specimen = "this is a string\n";
  char * copy1 = a_strdup (specimen);
  char * copy2 = a_strdup (copy1);

  assert (strcmp (copy1, copy2) == 0);
  assert (strcmp (copy1, specimen) == 0);

  free (copy1);
  assert (strcmp (copy2, specimen) == 0);
  free (copy2);

  return 0;
}

int
test_guards (void)
{
  // check int guard
  {
    printf ("int guard\n");
    int x = 0, e = 0;
    jmp_buf env;
    if ((e = setjmp (env)) == 0)
      {
	guard_int (env, 1, 0);
	x = 4; // must hit this
	guard_int (env, 9, -1);
	assert (0); // shouldn't get there
      }
    else
      {
	assert (e == 9);
      }
    assert (x == 4);
  }

  // check ptr guard
  {
    printf ("ptr guard\n");
    int x = 0, e = 0;
    jmp_buf env;
    if ((e = setjmp (env)) == 0)
      {
	guard_ptr (env, 1, &test_guards);
	x = -7; // must hit this
	guard_ptr (env, 16, NULL);
	assert (0); // shouldn't get there
      }
    else
      {
	assert (e == 16);
      }
    assert (x == -7);
  }

  return 0;
}

void *
allocate ()
{
  return malloc (1024);
}

int
main (void)
{
  assert (test_a_strdup () == 0);
  assert (test_guards () == 0);

  printf ("tmpbuild\n");
  void * ptr = tmpbuild (NULL, allocate);
  assert (ptr != NULL);
  void * ptr2 = tmpbuild (ptr, allocate);
  assert (ptr == ptr2);

  printf ("All passed.\n");
  return 0;
}

#endif
