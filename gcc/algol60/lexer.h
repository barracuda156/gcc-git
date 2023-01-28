/*
 * Copyright (c) 2006 Petr Machata
 * All rights reserved.
 */

#ifndef _AL60L_LEXER_H_
#define _AL60L_LEXER_H_

//for references in parser-tab.h
#include "slist.i"
#include "statement.i"
#include "label.i"
#include "symbol.i"
#include "type.i"
#include "boundspair.i"
#include "expression.i"
#include "desig-expr.h"
#include "estring.i"

// direct includes
#include "parser-tab.h"
#include "pd.h"
#include "estring.h"
#include "logger.h"
#include <stdio.h>

typedef struct struct_lexer_t { } lexer_t;
typedef enum yytokentype token_kind_t;

/// Allocate and return new lexer, operating on given stream `file',
/// whose name is `filename'.  If `manage' is non-zero, it will be
/// closed upon destruction of this lexer.
///
/// \arg file The stream which the lexer should use to fetch tokens.
///
/// \arg filename The name of file underlying the stream.  May be
/// arbitrary descriptor, if the stream doesn originate in file.  Must
/// not be NULL.
///
/// \arg manage Whether the stream should be fclosed when the lexer is
/// destroyed.
lexer_t * new_lexer (FILE * stream, char const* filename, int manage)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Opens the file with given name and returns new stream that
/// operates on that file.  The stream is managed (in sense of
/// new_lexer).  Returns NULL when an error occurs.
lexer_t * new_lexer_filename (char const* filename)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(1);

/// Destroy a lexer.
void delete_lexer (lexer_t * lexer);

/// Convert void* to lexer, if it is lexer, or return NULL.
lexer_t * lexer (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Query the name of parsed stream.
char const * lexer_filename (lexer_t const * lexer)
  ATTRIBUTE_NONNULL(1);

/// Query the name of parsed stream, taking into account preprocessing
/// directives.
char const * lexer_pp_filename (lexer_t const * lexer)
  ATTRIBUTE_NONNULL(1);


/// Fetch new token with given lexer, assign a location of given token,
/// and optionally store any token-related info into val.  Should a
/// token return string values, val->slit will be initialized with
/// new_estring.  You can pick this estring and use it further without
/// cloning, a new one is created each time.
int lexer_tok (lexer_t * lexer, YYSTYPE * val, YYLTYPE * loc)
  ATTRIBUTE_NONNULL(1);

/// Answer the kind of last token
token_kind_t lexer_get_tok_kind (lexer_t * lexer)
  ATTRIBUTE_NONNULL(1);

/// Answer the lexeme that formed last token.
char const* lexer_get_tok_lexeme (lexer_t * lexer)
  ATTRIBUTE_NONNULL(1);

/// Answer the length of last lexeme.
int lexer_get_tok_lexeme_len (lexer_t * lexer)
  ATTRIBUTE_NONNULL(1);

/// Controls what logging messages get printed.  `Messages' controls
/// what error reporting is done.  `Tokens' controls whether debug
/// info about processed tokens gets printed.
void lexer_set_logging (lexer_t * lexer, debug_level_t messages, int tokens)
  ATTRIBUTE_NONNULL(1);

/// Get underlying logging device.
logger_t const* lexer_log (lexer_t * lexer)
  ATTRIBUTE_NONNULL(1);

/// Whether lexer should recognize preprocessing output (linemarks) in
/// input file.
void lexer_set_preprocessed (lexer_t * lexer, int preprocessed)
  ATTRIBUTE_NONNULL(1);

#endif//_AL60L_LEXER_H_
