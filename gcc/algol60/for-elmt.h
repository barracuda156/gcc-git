#ifndef _AL60L_FOR_ELMT_H_
#define _AL60L_FOR_ELMT_H_

///
/// \file for-elmt.h
///
/// Small module that contains functions for handling of for heading
/// elements.  Each such object represents one for list element.  E.g.
///   for i := 1, 2, 4 step 1 until 6 do ...
/// contains three elements: ForExpr elements `1' and `2', and
/// ForUntil element `4 step 1 until 6'.
///

#include "for-elmt.i"
#include "expression.i"
#include "cursor.i"
#include "estring.i"
#include "statement.i"
#include "logger.i"
#include "visitor.i"
#include "pd.h"

/// Create new element that denotes simple immediate value.
///  'for' i := 1 'do' ;
for_elmt_t * new_for_elmt_expr (cursor_t * cursor, expression_t * expr)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(2);

/// Create new element that denotes step-until element.
///  'for' i := start 'step' step 'until' stop 'do' ;
for_elmt_t * new_for_elmt_until (cursor_t * cursor, expression_t * start, expression_t * step, expression_t * stop)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_NONNULL(3)
  ATTRIBUTE_NONNULL(4);

/// Create new element that denotes while element.
///  'for' i := expr 'while' cond 'do' ;
for_elmt_t * new_for_elmt_while (cursor_t * cursor, expression_t * expr, expression_t * cond)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(3)
  ATTRIBUTE_NONNULL(2);

/// Create a copy of subtree starting at given for element.
/// Recursively clones whole subtree.
for_elmt_t * clone_for_elmt (for_elmt_t const * self)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL(1);

/// Convert void* to for element, if it is statement, or abort.
for_elmt_t * a60_as_for_elmt (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Rewrite the element into string in its original shape (as close as
/// possible).  `buf' is a string into which the dump should be done.
/// If NULL, new string will be allocated.  Either passed-in, or newly
/// allocated buffer is returned.
estring_t * for_elmt_to_str (for_elmt_t const * self, estring_t * buf)
  ATTRIBUTE_NONNULL(1);

/// Resolve symbols in and typecheck this for element.  By the time
/// this gets called, symtabs should be already filled.
/// `variable' is an expression denoting the controlling variable of
/// loop.  `context' is a parental container of for statement this
/// elements is part of. `log' is a logger for any warning and/or
/// error messages.
void for_elmt_resolve_symbols (for_elmt_t * self, expression_t * variable, container_t * context, logger_t * log)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Answer the expression of given expr for element.
expression_t * for_elmt_expr_expr (for_elmt_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the start expression of given until for element.
expression_t * for_elmt_until_start (for_elmt_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the step expression of given until for element.
expression_t * for_elmt_until_step (for_elmt_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the stop expression of given until for element.
expression_t * for_elmt_until_stop (for_elmt_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the expression of given while for element.
expression_t * for_elmt_while_expr (for_elmt_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the condition of given while for element.
expression_t * for_elmt_while_cond (for_elmt_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Construct For Element visitor.  Arguments are the functions that
/// should be called when the dispatched object's kind matches its
/// respective argument.
visitor_t * new_visitor_for_elmt (
    callback_t for_elmt_expr,
    callback_t for_elmt_until,
    callback_t for_elmt_while
)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2)
  ATTRIBUTE_NONNULL (3)
;

/// For conversion of function prototype to callback.
callback_t a60_for_elmt_callback (void *(*cb)(for_elmt_t *, void *))
  ATTRIBUTE_NONNULL (1);

#endif//_AL60L_FOR_ELMT_H_
