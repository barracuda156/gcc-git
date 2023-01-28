#ifndef _AL60L_DESIG_EXPR_H_
#define _AL60L_DESIG_EXPR_H_

///
/// \file desig-expr.h
///
/// Designational expressions are used to denote the target of a goto
/// statement.  They generally come in one of three shapes: simple
/// label; conditional expression; or switch expression.  This module
/// handles designational expressions.
///

#include "desig-expr.i"
#include "cursor.i"
#include "label.i"
#include "expression.i"
#include "logger.i"
#include "estring.i"
#include "statement.i"
#include "symbol.i"
#include "visitor.i"
#include "pd.h"

/// Create a designational expression representing simple label.
desig_expr_t * new_desig_expr_label (cursor_t * location, label_t * lbl)
  ATTRIBUTE_MALLOC;

/// Create a conditional designational expression.
desig_expr_t * new_desig_expr_if (cursor_t * location, expression_t * cond, desig_expr_t * exp_t, desig_expr_t * exp_f)
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_NONNULL(3)
  ATTRIBUTE_NONNULL(4)
  ATTRIBUTE_MALLOC;

/// Create a switch designational expression.
desig_expr_t * new_desig_expr_switch (cursor_t * location, label_t * lbl, expression_t * index)
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_NONNULL(3)
  ATTRIBUTE_MALLOC;

/// Create a copy of subtree starting at given designational
/// expression. Cursor is shared, other components are recursively
/// cloned.
desig_expr_t * clone_desig_expr (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_MALLOC;

/// Convert void* to designational expression, if it is designational
/// expression, or abort.
desig_expr_t * a60_as_desig_expr (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Dump the designational expression to string.  Returns 'buf', or
/// newly allocated buffer if 'buf' was NULL.  Contents of non-NULL
/// 'buf' is overwritten.
estring_t * desig_expr_to_str (desig_expr_t const * self, estring_t * buf)
  ATTRIBUTE_NONNULL(1);

/// Resolve symbols used in this designational expression.  By the
/// time this gets called, symtabs should be already filled.
/// Typechecking should be also performed here.
void desig_expr_resolve_symbols (desig_expr_t * self, container_t * context, logger_t * logger)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(3);

/// Answer the cursor associated with this designational expression.
cursor_t * desig_expr_cursor (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer condition of if-style designational expression.
/// Assert if it's not if-expression.
expression_t * desig_expr_if_cond (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer true branch of if-style designational expression.
/// Assert if it's not if-expression.
desig_expr_t * desig_expr_if_trueb (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer false branch of if-style designational expression.
/// Assert if it's not if-expression.
desig_expr_t * desig_expr_if_falseb (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the symbol associated with this designational expression.
/// Valid for label-style and switch-style designational expressions.
symbol_t * desig_expr_symbol (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Answer the index expression of switch-style designational
/// expression.  Assert if it's not designational expression.
expression_t * desig_expr_switch_index (desig_expr_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Construct Designation Expression visitor.  Arguments are the
/// functions that should be called when the dispatched object's kind
/// matches its respective argument.
visitor_t * new_visitor_desig_expr (
    callback_t desig_expr_label,
    callback_t desig_expr_if,
    callback_t desig_expr_switch
)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2)
  ATTRIBUTE_NONNULL (3)
;

/// For conversion of function prototype to callback.
callback_t a60_desig_expr_callback (void *(*cb)(desig_expr_t *, void *))
  ATTRIBUTE_NONNULL (1);

#endif//_AL60L_DESIG_EXPR_H_
