/*
 * Copyright (c) 2006 Petr Machata
 * All rights reserved.
 */

#ifndef _AL60L_DEBUG_H_
#define _AL60L_DEBUG_H_

#include <stdio.h>
#include "pd.h"

/// The description of the severity of log message.  Used for
/// filtering.
typedef enum enum_debug_level_t
{
  ll_filter_nothing,    ///< Logical value for `filter nothing'.
  ll_debug = ll_filter_nothing,    ///< Debug level information
  ll_info,              ///< General progress information
  ll_warning,           ///< Compilation warnings
  ll_error,             ///< Compilation errors
  ll_fatal_error,       ///< Fatal errors, usually leading to compiler halt
  ll_filter_all,
  debug_level_t_count = ll_filter_all
} debug_level_t;

#include "cursor.i"
#include "logger.i"

/// Allocate new debug log context.  The default output stream is
/// stderr, the default threshold level is ll_warning.
logger_t * new_logger (char const* name)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(1);

/// Destroy the log.
void delete_logger (logger_t * logger);

/// Convert void* to logger, if it is logger, or return NULL.
logger_t * logger (void * ptr)
  ATTRIBUTE_NONNULL(1);


/// This function is used for logging compiler messages.  Depending on
/// designated severity and source and effective filtering rules, the
/// function may or may not actually print anything to screen (or
/// file).  E.g. with level=ll_debug and filter set to ll_warning, the
/// message will be swallowed.
///
/// Old-style printf is used under the hoods, so the format string is
/// perfectly printf-like.
///
/// \arg logger The logger to which the message should be sent.
/// \arg level Severity of message.
/// \arg format_string printf-like formatting string.  The arguments
/// following this one are objects that should be formatted.
///
/// \return Like printf, the function returns a number of characters
/// written.  If an error is encountered, a negative number is
/// answered.  Filtered out messages return 0.
int log_printf (logger_t * logger, debug_level_t level,
		char const* format_string, ...)
  ATTRIBUTE_PRINTF(3,4)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(3);

/// Like log_printf, but also include formatted information about the
/// cursor.  Cursor may be NULL, in which case the information isn't
/// printed.
int log_printfc (logger_t * logger, debug_level_t level, cursor_t * cursor,
		 char const* format_string, ...)
  ATTRIBUTE_PRINTF(4,5)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(4);


/// Use this function to set up message filtering based on their
/// severity in given source.  Filtering level is ll_warning by
/// default.
///
/// \arg logger The logger for which the filter should be set up.
///
/// \arg level The lowest severity that the message from given source
/// has to have to get displayed.  Application will print out all
/// messages with severity higher or the same as provided level.
void log_set_filter (logger_t * logger, debug_level_t level)
  ATTRIBUTE_NONNULL(1);


/// Set the stream where the output should be sent.
///
/// \arg logger The logger for which the stream should be set.  NULL
/// means don't output debug messages at all.
///
/// \arg stream The stream where unfiltered output will appear on.
void log_set_stream (logger_t * logger, FILE * stream)
  ATTRIBUTE_NONNULL(1);


/// Answer count of messages with a severity higher than or same as
/// given level.
///
/// \arg logger The logger for which the count should be given.
///
/// \arg level The lower bound of severity of counted messages.
///
/// \return Number of messages.
int log_count_messages (logger_t const* logger, debug_level_t level)
  ATTRIBUTE_NONNULL(1);

#endif //_AL60L_DEBUG_H_
