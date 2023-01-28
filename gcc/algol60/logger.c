/*
 * Copyright (c) 2006 Petr Machata
 * All rights reserved.
 */

#ifndef SELF_TEST

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "meta.h"

#include "logger.h"
#include "cursor.h"
#include "util.h"

static char const* private_logger_signature = "logger";

char const* debug_level_str[debug_level_t_count] = {
  "debug",
  "info",
  "warn",
  "error",
  "fatal"
};

struct struct_logger_t
{
  char const* signature;

  char * name;
  FILE * stream;
  int ct[debug_level_t_count];
  debug_level_t threshold;
};

logger_t *
new_logger (char const* name)
{
  assert (name != NULL);

  logger_t * ret = malloc (sizeof (logger_t));
  if (ret == NULL)
    return NULL;

  ret->signature = private_logger_signature;

  ret->name = a_strdup (name);
  ret->stream = stderr;
  memset (ret->ct, 0, sizeof (ret->ct));
  ret->threshold = ll_warning;

  return ret;
}

void
delete_logger (logger_t * logger)
{
  if (logger != NULL)
    {
      free (logger->name);
      free (logger);
    }
}

logger_t *
logger (void * ptr)
{
  A60_CHECKED_CONVERSION(logger, ptr);
}

/// Depending on filtering rules in effect, either filter out or print
/// the message.
static int private_maybe_log_message (logger_t * logger, debug_level_t level,
				      cursor_t * cursor, char const * fmt,
				      va_list * ap);

/// General error reporting function, #ifdefed to either use GCC or
/// our own error reporting.
static int private_log_printfc (logger_t * logger, debug_level_t level,
				cursor_t * cursor, char const * fmt,
				va_list * ap0);

/// Non-GCC error reporting.
static int private_log_printfc_nogcc (logger_t * logger, debug_level_t level,
				      cursor_t * cursor, char const * fmt,
				      va_list * ap);

int
log_printf (logger_t * logger, debug_level_t level,
	    char const* fmt, ...)
{
  assert (logger != NULL);
  assert (fmt != NULL);

  va_list ap;
  va_start (ap, fmt);
  int ret = private_maybe_log_message (logger, level, NULL, fmt, &ap);
  va_end (ap);

  return ret;
}

int
log_printfc (logger_t * logger, debug_level_t level, cursor_t * cursor,
	     char const* fmt, ...)
{
  assert (logger != NULL);
  assert (fmt != NULL);

  va_list ap;
  va_start (ap, fmt);
  int ret = private_maybe_log_message (logger, level, cursor, fmt, &ap);
  va_end (ap);
  return ret;
}

void
log_set_filter (logger_t * logger, debug_level_t level)
{
  assert (logger != NULL);
  logger->threshold = level;
}

void
log_set_stream (logger_t * logger, FILE * stream)
{
  assert (logger != NULL);
  logger->stream = stream;
}

int
log_count_messages (logger_t const * logger, debug_level_t level)
{
  assert (logger != NULL);
  int ret = 0;

  for (; level < debug_level_t_count; ++level)
    ret += logger->ct[level];

  return ret;
}

static int
private_maybe_log_message (logger_t * logger, debug_level_t level,
			   cursor_t * cursor, char const * fmt,
			   va_list * ap)
{
  if (logger->stream != NULL
      && level >= logger->threshold)
    {
      ++ logger->ct[level];
      return private_log_printfc (logger, level, cursor, fmt, ap);
    }
  else
    return 0;
}

static int
private_log_printfc_nogcc (logger_t * logger, debug_level_t level,
			   cursor_t * cursor, char const * fmt, va_list * ap)
{
  int ret = 0;

  if (cursor != NULL)
    ret += fprintf (logger->stream, "%s: ", cursor_to_str (cursor));
  ret += fprintf (logger->stream, "%s: ", debug_level_str[level]);
  ret += vfprintf (logger->stream, fmt, *ap);
  ret += fprintf (logger->stream, "\n");

  return ret;
}

#ifdef IN_GCC
// Use GCC-native error reporting when possible.  This is at the end
// of definitions, to keep us away from GCC memory mangement poisoning.
# include "config.h"
# include "system.h"
# include "coretypes.h"
# include "limits.h"
# include "toplev.h"
# include "diagnostic.h"

static int
private_log_printfc (logger_t * logger, debug_level_t level,
		     cursor_t * cursor, char const * fmt, va_list * ap)
{
  // GCC diagnostics are not intended for debug messages and notes.
  // Fall back to our own reporting in such a case.
  if (level < ll_warning)
    return private_log_printfc_nogcc (logger, level, cursor, fmt, ap);

  // Table to convert from our own severity encoding to GCC's.
  static diagnostic_t gcc_diagnostic_kind_map[] = {
    [ll_debug] = DK_DEBUG,
    [ll_info] = DK_NOTE,
    [ll_warning] = DK_WARNING,
    [ll_error] = DK_ERROR,
    [ll_fatal_error] = DK_FATAL
  };

  diagnostic_info diagnostic;
  location_t loc;
  if (cursor != NULL)
    cursor_to_loc (cursor, &loc);
  else
    memset (&loc, 0, sizeof (loc));
  diagnostic_set_info (&diagnostic, fmt, ap, loc, gcc_diagnostic_kind_map[level]);
  report_diagnostic (&diagnostic);
  return 0;
}

#else
// Outside the GCC, always fall back to our own error reporting.
static int
private_log_printfc (logger_t * logger, debug_level_t level,
		     cursor_t * cursor, char const * fmt, va_list * ap)
{
  return private_log_printfc_nogcc (logger, level, cursor, fmt, ap);
}

#endif


#else /* SELF_TEST */

#include "logger.h"
#include <stdio.h>
#include <assert.h>

int
main (void)
{
  logger_t * log_test = new_logger ("test");
  assert (logger (log_test));

  int ret;

  ret = log_printf (log_test, ll_debug,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret == 0);

  ret = log_printf (log_test, ll_warning,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret > 0);

  ret = log_printf (log_test, ll_error,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret > 0);

  log_set_filter (log_test, ll_error);
  ret = log_printf (log_test, ll_error,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret > 0);

  log_set_filter (log_test, ll_debug);
  ret = log_printf (log_test, ll_error,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret > 0);

  ret = log_printf (log_test, ll_debug,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret > 0);

  log_set_stream (log_test, NULL);
  ret = log_printf (log_test, ll_fatal_error,
		    "This is testing message #%d.",
		    log_count_messages (log_test, 0));
  assert (ret == 0);

  assert (log_count_messages (log_test, 0) == 5);

  delete_logger (log_test);

  printf ("All passed.\n");
  return 0;
}

#endif
