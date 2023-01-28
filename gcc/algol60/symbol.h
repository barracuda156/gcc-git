#ifndef _AL60L_SYMBOL_H_
#define _AL60L_SYMBOL_H_

///
/// \file symbol.h
///
/// Symbol is an AST node that represents a symbol in a program.
/// Symbols are used to track variables and functions in Algol
/// programs.  Each symbol has a label, that denotes its name, and
/// several optional features, such as assigned type, statement, etc.
///

#include "symbol.i"
#include "label.i"
#include "type.i"
#include "statement.i"
#include "estring.i"
#include "visitor.i"
#include "logger.i"
#include "pd.h"

typedef enum enum_parmconv_t {
  pc_byname,
  pc_byvalue
}
parmconv_t;

/// Create new variable given its name.
symbol_t * new_symbol_var (label_t const * name)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_MALLOC;

/// Create new function given its name.
/// Use symbol_set_stmt to assign a body & symtab to function.
symbol_t * new_symbol_func (label_t const * name)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_MALLOC;

/// Create new formal parameter.
/// `type' may be `t_any' for by-name parameters withou a type
/// specified.
symbol_t * new_symbol_formparm (label_t const * name, type_t * type, parmconv_t convention)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_MALLOC;

/// Create new symbol by cloning other symbol.
symbol_t * clone_symbol (symbol_t const * self)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_MALLOC;

/// Create new symbol by cloning other symbol, only with other name.
symbol_t * clone_symbol_with_name (symbol_t const * self, label_t const * name)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_MALLOC;

/// Destroy the symbol.  `self' can be NULL.
void delete_symbol (symbol_t * self);

/// Convert void* to symbol, if it is symbol, or abort.
symbol_t * a60_as_symbol (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Get symbol's label.
label_t const * symbol_label (symbol_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Render symbol into string.  Buf may be NULL, new one would be
/// allocated.  Returns either buf, or newly allocated buffer if buf
/// was NULL.
estring_t * symbol_to_str (symbol_t const * self, estring_t * buf);

/// Resolve symbols in and typecheck this symbol.
void symbol_resolve_symbols (symbol_t * self, container_t * context, logger_t * log)
  ATTRIBUTE_NONNULL(1);

/// Set the type of the symbol.  Type must not be NULL.
void symbol_set_type (symbol_t * self, type_t * type)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Get the type of the symbol.
type_t * symbol_type (symbol_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Set the statement associated with the symbol.  Statement must not
/// be NULL.
/// @TODO: see if it's possible to move this to ordinary symbol... or
/// perhaps to dedicated `label'-kind symbol.
void symbol_set_stmt (symbol_t * self, statement_t * stmt)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Get the statement associated with the symbol.
statement_t * symbol_stmt (symbol_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Make the symbol hidden / visible.
void symbol_set_hidden (symbol_t * self, int hidden)
  ATTRIBUTE_NONNULL(1);

/// Answer if the symbol is hidden.
int symbol_hidden (symbol_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Assign extra to given symbol.
void symbol_set_extra (symbol_t * self, void * extra)
  ATTRIBUTE_NONNULL(1);

/// Get symbol's extra.
void * symbol_extra (symbol_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Construct Symbol visitor.  Arguments are the functions that
/// should be called when the dispatched object's kind matches its
/// respective argument.
visitor_t * new_visitor_symbol (
    callback_t symbol_var,
    callback_t symbol_fun,
    callback_t symbol_formparm
)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2)
  ATTRIBUTE_NONNULL (3)
;

/// For conversion of function prototype to callback.
callback_t a60_symbol_callback (void *(*cb)(symbol_t *, void *))
  ATTRIBUTE_NONNULL (1);

#endif//_AL60L_SYMBOL_H_
