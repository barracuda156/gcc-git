#ifndef AL60L_GCC_BIND_H
#define AL60L_GCC_BIND_H

#include "statement.i"
#include "expression.i"
#include "desig-expr.i"
#include "type.i"
#include "symbol.i"

/// \file
/// Al60l binder contains the glue between Al60l parsing library and
/// GCC.  The code that does AST-to-GENERIC translation is here.

/// \note
/// The GCC framework necessary to setup types isn't included here.
/// Instead, it's assumed that the unit that needs to use binder will
/// also need to use GCC framework, and will have to include it anyway.

typedef struct struct_al60l_bind_state_t al60l_bind_state_t;

/// Create new state for al60l binder.
al60l_bind_state_t * new_bind_state (void)
  ATTRIBUTE_MALLOC;

/// Destroy the state.
void delete_bind_state (al60l_bind_state_t * state);

/// Push new scope.
void bind_state_push_block (al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1);

/// Remove inner scope.  The discard means no tree is answered as part
/// of this function.  push function currently uses this, because no
/// real function support is in place ATM.  It will go away, and I
/// presume also this function will become unused.
void bind_state_discard_block (al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1);

/// Pop inner scope and build tree (BIND_EXPR) from it.
tree bind_state_build_block (al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1);

/// Add declaration to inner scope.
void bind_state_add_decl (al60l_bind_state_t * state, tree decl)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Add statement to inner scope.
void bind_state_add_stmt (al60l_bind_state_t * state, tree stmt)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Push new function context to the state.  `resultdecl' is tree
/// representing the value that holds return value of function.  `decl' is
/// the declaration of the function.  New block is pushed
/// automatically as part of function push.
void bind_state_push_function (al60l_bind_state_t * state, tree resultdecl, tree decl)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_NONNULL(3);

/// Get decl of most enclosing function.
tree bind_state_enclosing_decl (al60l_bind_state_t const* state)
  ATTRIBUTE_NONNULL(1);

/// Get resultdecl of most enclosing function.
tree bind_state_enclosing_resultdecl (al60l_bind_state_t const* state)
  ATTRIBUTE_NONNULL(1);

/// Pop one function context off the stack.
/// This *discards* the block that was pushed as part of function
/// push.  This will go away eventually, as proper function handling
/// is implemented (during function support I expect).
void bind_state_pop_function (al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1);

/// Build GENERIC from statement.
tree stmt_build_generic (statement_t * statement, al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Build GENERIC from expression.
tree expr_build_generic (expression_t * expression, al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Build GENERIC from designational expression.
tree desig_expr_build_generic (desig_expr_t * desig_expr, al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Build GENERIC from type.
tree type_build_generic (type_t * type, al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Return GENERIC decl for given symbol depending on symbol's kind
/// (variable, function, etc.).
tree symbol_decl_build_generic (symbol_t * symbol, al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Return GENERIC initialization for given symbol depending on
/// symbol's kind & type.
tree symbol_init_build_generic (symbol_t * symbol, al60l_bind_state_t * state)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

#endif//AL60L_GCC_BIND_H
