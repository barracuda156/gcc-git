/*
 * Copyright (c) 2006 Petr Machata
 * All rights reserved.
 */

#ifndef _AL60L_PARSER_H_
#define _AL60L_PARSER_H_

#include "pd.h"
#include "lexer.h"
#include "statement.i"
#include "logger.i"
#include <stdio.h>

typedef struct struct_parser_t { } parser_t;

/// Allocate new parser, fetching tokens from the given lexer.  If
/// `manage' is non-zero, the lexer will be released after the parse.
parser_t * new_parser (lexer_t * lexer, int manage)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_MALLOC;


/// Destroy the parser.
void delete_parser (parser_t * parser);

/// Convert void* to parser, if it is parser, or abort.
parser_t * a60_as_parser (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Fire a parser.  Answer the AST tree of parsed file, or NULL when
/// there were errors.
statement_t * parser_parse (parser_t * parser)
  ATTRIBUTE_NONNULL(1);

/// Controls what logging messages get printed.
void parser_set_logging (parser_t * parser, debug_level_t filter)
  ATTRIBUTE_NONNULL(1);

/// Get underlying logging device.
logger_t const* parser_log (parser_t * parser)
  ATTRIBUTE_NONNULL(1);

#endif//_AL60L_PARSER_H_
