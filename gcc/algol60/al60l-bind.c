/// This file contains the binding between the GCC part and Al60l
/// parsing library.

#include "config.h" /* system.h needs to know if it's safe to include
		       <string.h>/<strings.h> stuff; this is
		       host-specific, and config.h knows it */

#include "system.h" /* stdlib, strings... system includes... for some
		       reason they can't be included directly, has
		       something to do with host/build/target
		       trichotomy. */

#include "coretypes.h" /* contains definition of `tree', required by
			  tree.h */

#include "tree.h"   /* treecodes used in tree.def, here required to be
		       able to call actual tree-building functions */

#include "real.h"   /* methods for real number manipulations, such as
		       build_real */

#include "tree-gimple.h" /* alloc_stmt_list, append_to_statement_list */
#include "toplev.h" /* rest_of_decl_compilation, pedwarn */

#include "algol-tree.h"
#include "al60l-bind.h"

#include "util.h"
#include "meta.h"

#include "slist.h"
#include "estring.h"
#include "boundspair.h"
#include "label.h"
#include "symbol.h"
#include "type.h"
#include "statement.h"
#include "expression.h"
#include "desig-expr.h"
#include "for-elmt.h"
#include "visitor.h"
#include "a60_symtab.h"
#include "cursor.h"

struct struct_al60l_bind_state_t
{
  /// Function declarations stack.  As the layers of nesting are
  /// uncovered, new decls are pushed on and popped off this stack.
  /// slist_front always yields the most enclosing decl.
  slist_t * fundecls;

  /// Result declarations stack.  This is always kept in sync with
  /// fundecls, n-th resultdecl from top of this stack matches n-th
  /// fundecl from top of fundecl stack.
  slist_t * resultdecls;

  /// Stack of subblocks on binding levels.  This is slist of
  /// tree lists.
  slist_t * subblocks;

  /// Stack of vars on binding levels.  This is slist of
  /// tree chains.
  slist_t * vars;

  /// Stack of statements on binding levels.  This is slist of
  /// tree chains.
  slist_t * stmts;

  /// For translation of for elements.  Stack of loop constructs.
  /// Each for-loop translation pushes a statement_t * to front.
  slist_t * for_statements;

  visitor_t * statement_build_generic;  ///< statement->GENERIC builder visitor.
  visitor_t * expression_build_generic;  ///< expression->GENERIC builder visitor.
  visitor_t * desig_expr_build_generic;  ///< desig_expr->GENERIC builder visitor.
  visitor_t * type_build_generic;  ///< type->GENERIC builder visitor.
  visitor_t * symbol_decl_build_generic; ///< symbol->GENERIC, decl builder visitor.
  visitor_t * symbol_decl_for_type; ///< (symbol,type)->GENERIC, decl builder visitor.
  visitor_t * symbol_init_build_generic; ///< symbol->GENERIC, decl builder visitor.
  visitor_t * symbol_init_for_type; ///< (symbol,type)->GENERIC, init builder visitor.
  visitor_t * symbol_var_switch_init_elmt_build_generic; ///< desig_expr->GENERIC, builder for switch dispatch auxiliary code.
  visitor_t * for_elmt_build_generic; ///< for_elmt->GENERIC builder visitor.
};

#ifndef IN_GCC
# error It makes no sense to build this file outside the GCC
#endif

// ------------------------------------
//   CALLBACK DECLARATIONS
// ------------------------------------

// Statement callbacks.

static void * stmt_dummy_build_generic (statement_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_assign_build_generic (statement_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_call_build_generic (statement_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_cond_build_generic (statement_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_for_build_generic (statement_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_goto_build_generic (statement_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_container_build_generic (container_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * stmt_toplev_build_generic (container_t * self, void * data)
  ATTRIBUTE_NONNULL (1);


// Expression callbacks.

static void * expr_int_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_real_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_string_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_bool_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_idref_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_if_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_binary_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_unary_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_call_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * expr_subscript_build_generic (expression_t * self, void * data)
  ATTRIBUTE_NONNULL (1);


// Designational Expression callbacks.

static void * desig_expr_label_build_generic (desig_expr_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * desig_expr_if_build_generic (desig_expr_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * desig_expr_switch_build_generic (desig_expr_t * self, void * data)
  ATTRIBUTE_NONNULL (1);


// Type callbacks.

static void * type_unknown_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_any_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_own_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_void_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_int_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_real_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_string_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_bool_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_label_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_switch_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_array_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);

static void * type_proc_build_generic (type_t * self, void * data)
  ATTRIBUTE_NONNULL (1);


// Declaration builder callbacks.

static void * symbol_var_decl_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_fun_decl_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_formparm_decl_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_invalid_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_ordinary_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_label_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_switch_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_proc_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);


// Initializator builder callbacks.

static void * symbol_var_init_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_fun_init_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_formparm_init_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_init_omit (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_init_error (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);

static void * symbol_var_switch_init_build_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);


// Helper callbacks for symbol_var_switch_init_build_generic.

static void * switch_label_init_build_generic (desig_expr_t * de, void * _state)
  ATTRIBUTE_NONNULL (1);

static void * switch_compute_init_build_generic (desig_expr_t * de, void * _state)
  ATTRIBUTE_NONNULL (1);


// For Element callbacks.

static void * for_elmt_expr_build_generic (for_elmt_t * for_elmt, void * data)
  ATTRIBUTE_NONNULL (1);

static void * for_elmt_until_build_generic (for_elmt_t * for_elmt, void * data)
  ATTRIBUTE_NONNULL (1);

static void * for_elmt_while_build_generic (for_elmt_t * for_elmt, void * data)
  ATTRIBUTE_NONNULL (1);


/// Special function, not callback.  Returns GENERIC for given builtin
/// declaration.
static void * builtin_decl_get_generic (symbol_t * sym, void * data)
  ATTRIBUTE_NONNULL (1);


// ------------------------------------
//   AUXILIARIES
// ------------------------------------

al60l_bind_state_t *
new_bind_state (void)
{
  al60l_bind_state_t * ret = xmalloc (sizeof (al60l_bind_state_t));
  if (ret == NULL)
    return NULL;

  jmp_buf buf;
  if (setjmp (buf) == 0)
    {
      guard_ptr (buf, 1, ret->fundecls = new_slist ());
      guard_ptr (buf, 1, ret->resultdecls = new_slist ());
      guard_ptr (buf, 1, ret->subblocks = new_slist ());
      guard_ptr (buf, 1, ret->vars = new_slist ());
      guard_ptr (buf, 1, ret->stmts = new_slist ());
      guard_ptr (buf, 1, ret->for_statements = new_slist_typed (adapt_test, a60_as_statement));
      guard_ptr (buf, 1,
          ret->statement_build_generic = new_visitor_stmt (
	      a60_stmt_callback (stmt_dummy_build_generic),
	      a60_stmt_callback (stmt_assign_build_generic),
	      a60_stmt_callback (stmt_call_build_generic),
	      a60_stmt_callback (stmt_cond_build_generic),
	      a60_stmt_callback (stmt_for_build_generic),
	      a60_stmt_callback (stmt_goto_build_generic),
	      a60_cont_callback (stmt_container_build_generic),
	      a60_cont_callback (stmt_toplev_build_generic)
	  ));
      guard_ptr (buf, 1,
          ret->expression_build_generic = new_visitor_expr (
	      a60_expr_callback (expr_int_build_generic),
	      a60_expr_callback (expr_real_build_generic),
	      a60_expr_callback (expr_string_build_generic),
	      a60_expr_callback (expr_bool_build_generic),
	      a60_expr_callback (expr_idref_build_generic),
	      a60_expr_callback (expr_if_build_generic),
	      a60_expr_callback (expr_binary_build_generic),
	      a60_expr_callback (expr_unary_build_generic),
	      a60_expr_callback (expr_call_build_generic),
	      a60_expr_callback (expr_subscript_build_generic)
	  ));
      guard_ptr (buf, 1,
          ret->desig_expr_build_generic = new_visitor_desig_expr (
	      a60_desig_expr_callback (desig_expr_label_build_generic),
	      a60_desig_expr_callback (desig_expr_if_build_generic),
	      a60_desig_expr_callback (desig_expr_switch_build_generic)
	  ));
      guard_ptr (buf, 1,
          ret->type_build_generic = new_visitor_type (
	      a60_type_callback (type_unknown_build_generic),
	      a60_type_callback (type_unknown_build_generic),
	      a60_type_callback (type_any_build_generic),
	      a60_type_callback (type_own_build_generic),
	      a60_type_callback (type_void_build_generic),
	      a60_type_callback (type_int_build_generic),
	      a60_type_callback (type_real_build_generic),
	      a60_type_callback (type_string_build_generic),
	      a60_type_callback (type_bool_build_generic),
	      a60_type_callback (type_label_build_generic),
	      a60_type_callback (type_switch_build_generic),
	      a60_type_callback (type_array_build_generic),
	      a60_type_callback (type_proc_build_generic)
	  ));
      guard_ptr (buf, 1,
	   ret->symbol_decl_build_generic = new_visitor_symbol (
	      a60_symbol_callback (symbol_var_decl_build_generic),
	      a60_symbol_callback (symbol_fun_decl_build_generic),
	      a60_symbol_callback (symbol_formparm_decl_build_generic)
	   ));
      guard_ptr (buf, 1,
          ret->symbol_decl_for_type = new_visitor_type (
	      a60_symbol_callback (symbol_var_invalid_build_generic),
	      a60_symbol_callback (symbol_var_invalid_build_generic),
	      a60_symbol_callback (symbol_var_invalid_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_label_build_generic),
	      a60_symbol_callback (symbol_var_switch_build_generic),
	      a60_symbol_callback (symbol_var_ordinary_build_generic),
	      a60_symbol_callback (symbol_var_proc_build_generic)
	  ));
      guard_ptr (buf, 1,
	   ret->symbol_init_build_generic = new_visitor_symbol (
	      a60_symbol_callback (symbol_var_init_build_generic),
	      a60_symbol_callback (symbol_fun_init_build_generic),
	      a60_symbol_callback (symbol_formparm_init_build_generic)
	   ));
      guard_ptr (buf, 1,
          ret->symbol_init_for_type = new_visitor_type (
	      a60_symbol_callback (symbol_var_init_error),
	      a60_symbol_callback (symbol_var_init_error),
	      a60_symbol_callback (symbol_var_init_error),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_switch_init_build_generic),
	      a60_symbol_callback (symbol_var_init_omit),
	      a60_symbol_callback (symbol_var_init_omit)
	  ));
      guard_ptr (buf, 1,
          ret->symbol_var_switch_init_elmt_build_generic = new_visitor_desig_expr (
	      a60_desig_expr_callback (switch_label_init_build_generic),
	      a60_desig_expr_callback (switch_compute_init_build_generic),
	      a60_desig_expr_callback (switch_compute_init_build_generic)
	  ));
      guard_ptr (buf, 1,
          ret->for_elmt_build_generic = new_visitor_for_elmt (
	      a60_for_elmt_callback (for_elmt_expr_build_generic),
	      a60_for_elmt_callback (for_elmt_until_build_generic),
	      a60_for_elmt_callback (for_elmt_while_build_generic)
	  ));
      return ret;
    }
  else
    {
      delete_bind_state (ret);
      return NULL;
    }
}

void
delete_bind_state (al60l_bind_state_t * _state)
{
  al60l_bind_state_t * state = (void*)_state;
  if (state != NULL)
    {
      delete_slist (state->fundecls);
      delete_slist (state->resultdecls);
      delete_slist (state->subblocks);
      delete_slist (state->vars);
      delete_slist (state->stmts);
      free (state);
    }
}

void
bind_state_push_block (al60l_bind_state_t * state)
{
  gcc_assert (state != NULL);
  slist_pushfront (state->subblocks, NULL_TREE);
  slist_pushfront (state->vars, NULL_TREE);
  slist_pushfront (state->stmts, alloc_stmt_list ());
}

void
bind_state_discard_block (al60l_bind_state_t * state)
{
  gcc_assert (state != NULL);
  gcc_assert (!slist_empty (state->subblocks));
  gcc_assert (!slist_empty (state->vars));
  gcc_assert (!slist_empty (state->stmts));
  slist_popfront (state->subblocks);
  slist_popfront (state->vars);
  slist_popfront (state->stmts);
}

tree
bind_state_build_block (al60l_bind_state_t * state)
{
  // Collect the subblocks that might have been added during
  // translation of container statements.
  tree subblocks = slist_popfront (state->subblocks);
  if (subblocks)
    subblocks = nreverse (subblocks);
  tree vars = slist_popfront (state->vars);
  tree stmts = slist_popfront (state->stmts);

  // Create new block and make it a superblock of all subblocks.
  tree block = build_block (vars, subblocks,
			    /*supercontext*/NULL_TREE,
			    /*chain*/NULL_TREE);
  if (subblocks)
    {
      tree subblock_node;
      for (subblock_node = subblocks; subblock_node != NULL_TREE;
	   subblock_node = TREE_CHAIN (subblock_node))
	{
	  tree subblock = TREE_VALUE (subblock_node);
	  BLOCK_SUPERCONTEXT (subblock) = block;
	}
    }

  // Then add the block to super's subblocks.
  slist_it_t * it = slist_iter (state->subblocks);
  tree super_subs = slist_it_get (it);
  super_subs = tree_cons (NULL_TREE, block, super_subs);
  slist_it_put (it, super_subs);
  delete_slist_it (it);

  // Finally return the binding expression that represents our block.
  tree bind = build3 (BIND_EXPR, void_type_node,
		      BLOCK_VARS (block), stmts, block);
  TREE_USED (block) = 1;

  return bind;
}

void
bind_state_add_decl (al60l_bind_state_t * state, tree decl)
{
  // Add variable to the chain.  This really has to be chain of
  // variables, not a list of conses.
  slist_it_t * it = slist_iter (state->vars);
  tree vars = slist_it_get (it);
  TREE_CHAIN (decl) = vars;
  slist_it_put (it, decl);
}

void
bind_state_add_stmt (al60l_bind_state_t * state,
		     tree stmt)
{
  tree stmts = slist_front (state->stmts);
  append_to_statement_list (stmt, &stmts);
}

void
bind_state_push_function (al60l_bind_state_t * state, tree resultdecl, tree decl)
{
  gcc_assert (state != NULL);
  slist_pushfront (state->fundecls, decl);
  slist_pushfront (state->resultdecls, resultdecl);
  bind_state_push_block (state);
}

tree
bind_state_enclosing_decl (al60l_bind_state_t const * state)
{
  gcc_assert (state != NULL);
  gcc_assert (!slist_empty (state->fundecls));
  return slist_front (state->fundecls);
}

tree
bind_state_enclosing_resultdecl (al60l_bind_state_t const * state)
{
  gcc_assert (state != NULL);
  gcc_assert (!slist_empty (state->resultdecls));
  return slist_front (state->resultdecls);
}

void
bind_state_pop_function (al60l_bind_state_t * state)
{
  gcc_assert (state != NULL);
  gcc_assert (!slist_empty (state->fundecls));
  gcc_assert (!slist_empty (state->resultdecls));
  slist_popfront (state->fundecls);
  slist_popfront (state->resultdecls);
  bind_state_discard_block (state);
}


// ------------------------------------
//   STATEMENT
// ------------------------------------

static void
private_set_location (tree node, cursor_t * cursor)
{
  if (cursor == NULL)
    return;

  location_t loc;
  cursor_to_loc (cursor, &loc);
  SET_EXPR_LOCATION (node, loc);
}

tree
stmt_build_generic (statement_t * statement, al60l_bind_state_t * state)
{
  tree ret = a60_visitor_dispatch (state->statement_build_generic, statement, statement, state);
  private_set_location (ret, stmt_cursor (statement));
  return ret;
}

void *
stmt_dummy_build_generic (statement_t * self ATTRIBUTE_UNUSED,
			  void * state ATTRIBUTE_UNUSED)
{
  return build_empty_stmt ();
}

void *
stmt_assign_build_generic (statement_t * self, void * data)
{
  slist_t * lhss = stmt_assign_lhss (self);
  gcc_assert (!slist_empty (lhss));

  tree ret = NULL_TREE;
  expression_t * rhs = stmt_assign_rhs (self);
  tree tt = type_build_generic (expr_type (rhs), data);
  tree rhst = build1 (SAVE_EXPR, tt,
		      expr_build_generic (rhs, data));

  if (slist_front (lhss) == slist_back (lhss))
    {
      // Simple case expr := expr.
      tree op1 = expr_build_generic (slist_front (lhss), data);
      ret = build2 (MODIFY_EXPR, void_type_node, op1, rhst);
    }
  else
    {
      // Complex cases expr1 := expr2 := ... := expr are handled with
      // special care.  At first, the address of each LHS expression
      // is extracted to temporary variable.  Then each temporary is
      // dereferenced and assigned RHS.  RRA60 requires the array
      // subscripts to be evaluated before the first assignment is
      // done, and this is our way to do it without having to look
      // into LHS expressions.
      //
      // @TODO: More optimal solution would be to check if given LHS
      // is array access expression.  Even more optimal would be to
      // check if the array access expression uses some LHS assigned
      // earlier and only compute address then.

      al60l_bind_state_t * state = data;
      bind_state_push_block (state);

      // List of dereferences of LHS address temporaries.
      slist_t * derefs = new_slist ();

      // evaluate LHS references
      slist_it_t * it = slist_iter (lhss);
      for (; slist_it_has (it); slist_it_next (it))
	{
	  // compute type of pointer to LHS
	  tree expr = expr_build_generic (slist_it_get (it), data);
	  tree base_type = TREE_TYPE (expr);
	  tree pointer_type = build_pointer_type (base_type);
	  expr = build1 (ADDR_EXPR, pointer_type, expr);

	  // assign the pointer to the temporary
	  tree tmp_decl = create_tmp_var_raw (pointer_type, NULL);
	  tree tmp_init = build2 (INIT_EXPR, void_type_node, tmp_decl, expr);
	  TREE_SIDE_EFFECTS (tmp_init) = 1;
	  TREE_USED (tmp_init) = 1;

	  bind_state_add_decl (state, tmp_decl);
	  bind_state_add_stmt (state, tmp_init);

	  // remember the reference to temporary for later assignments
	  expr = build1 (INDIRECT_REF, base_type, tmp_decl);
	  slist_pushback (derefs, expr);
	}

      // build assignments
      slist_it_reset (it, derefs);
      for (; slist_it_has (it); slist_it_next (it))
	{
	  tree tgt = slist_it_get (it);
	  tree assign = build2 (MODIFY_EXPR, void_type_node, tgt, rhst);
	  TREE_SIDE_EFFECTS (assign) = 1;
	  TREE_USED (assign) = 1;
	  bind_state_add_stmt (state, assign);
	}

      ret = bind_state_build_block (state);

      delete_slist_it (it);
      delete_slist (derefs);
    }
  return ret;
}

void *
stmt_call_build_generic (statement_t * self, void * state)
{
  return expr_build_generic (stmt_call_call (self), state);
}

void *
stmt_cond_build_generic (statement_t * self, void * state)
{
  tree cond = expr_build_generic (stmt_cond_cond (self), state);
  tree stmtt = stmt_build_generic (stmt_cond_ifclause (self), state);
  statement_t * elseclause = stmt_cond_elseclause (self);
  tree stmtf = elseclause ? stmt_build_generic (elseclause, state) : NULL_TREE;
  tree ret  = build3 (COND_EXPR, void_type_node, cond, stmtt, stmtf);
  return ret;
}

void *
stmt_for_build_generic (statement_t * self, void * _state)
{
  // Variable sharing: as per spec, variable declared `own' retain
  // their value between exit from block and entry to the same block.
  // Other variables have undefined values on block entry.  We want to
  // handle owns specifically for this reason, because the value has
  // to be shared across copied blocks.
  // We don't this, but it has to be done as part of `own' handling.
  //
  // Label sharing: local labels can be copied with the rest of the
  // code, the gotos will then lead into the same iteration.
  // Non-local labels are OK, because they will appear outside the
  // toplevel wrapper block.  Labels before the for statements itself
  // should be also ok, because whole thing is compiled into a single
  // tree node, and goto will then lead before that node.

  al60l_bind_state_t * state = _state;

  /*

    This might come in handy when we start handling `own' in for
    bodies.  Just don't forget that this only peels the outermost
    block's variables, and actually it's necessary to dive into the
    sctructure and pick each `own', and make all copies of particular
    own's `extra' point to the same declaration tree.

  if (container (for_body))
    {
      container_t * for_body_c = container (for_body);
      slist_t * for_body_symtab = container_symtab (for_body_c);
      slist_it_t * it = slist_iter (for_body_symtab);
      for (; slist_it_has (it); slist_it_next (it))
	{
	  symbol_t * sym = slist_it_get (it);
	  if (!types_same (symbol_type (sym), type_label ()))
	    {
	      symbol_t * erased = container_erase_symbol (for_body_c, sym);
	      gcc_assert (erased == sym);
	      int st = container_add_symbol (topwrap, sym, sek_ordinary);
	      gcc_assert (st == 0);
	    }
	}
      delete_slist_it (it);
    }
  */

  // Translate for elements...
  slist_pushfront (state->for_statements, self);
  bind_state_push_block (state);

  slist_it_t * it = slist_iter (stmt_for_elmts (self));
  for (; slist_it_has (it); slist_it_next (it))
    {
      for_elmt_t * elmt = slist_it_get (it);
      tree t = a60_visitor_dispatch (state->for_elmt_build_generic, elmt, elmt, state);
      bind_state_add_stmt (state, t);
    }
  delete_slist_it (it);

  tree ret = bind_state_build_block (state);
  slist_popfront (state->for_statements);

  return ret;
}

void *
stmt_goto_build_generic (statement_t * self, void * _state)
{
  al60l_bind_state_t * state = _state;
  tree target = desig_expr_build_generic (stmt_goto_target (self), state);
  tree ret = build1 (GOTO_EXPR, void_type_node, target);
  return ret;
}

static tree
private_label_build_generic (container_t * context ATTRIBUTE_UNUSED,
			     symbol_t * lbl,
			     al60l_bind_state_t * state)
{
  tree decl = symbol_extra (lbl);
  gcc_assert (decl != NULL);
  DECL_CONTEXT (decl) = slist_front (state->fundecls);
  tree ret = build1 (LABEL_EXPR, void_type_node, decl);
  return ret;
}

static void *
private_stmt_container_build_decl_for_symbol (symbol_t * sym, void * _state)
{
  // This is called from stmt_container_build_generic for each symbol
  // in symbol table.
  al60l_bind_state_t * state = _state;
  tree decl = symbol_decl_build_generic (sym, state);
  if (decl)
    {
      symbol_set_extra (sym, decl);
      bind_state_add_decl (state, decl);
    }
  return NULL;
}

static void *
private_stmt_container_build_init_for_symbol (symbol_t * sym, void * _state)
{
  // This is called from stmt_container_build_generic for each symbol
  // in symbol table.
  al60l_bind_state_t * state = _state;
  tree init = symbol_init_build_generic (sym, state);
  bind_state_add_stmt (state, init);
  return NULL;
}

void *
stmt_container_build_generic (container_t * self, void * _state)
{
  al60l_bind_state_t * state = _state;
  slist_it_t * it;

  // Open new subblock level.  All substatements that are containers
  // will add their bind_exprs to the _front of state->subblocks.
  bind_state_push_block (state);

  a60_symtab_each (container_symtab (self),
		   a60_symbol_callback (private_stmt_container_build_decl_for_symbol),
		   _state);

  a60_symtab_each (container_symtab (self),
		   a60_symbol_callback (private_stmt_container_build_init_for_symbol),
		   _state);

  it = slist_iter (container_stmts (self));
  for (; slist_it_has (it); slist_it_next (it))
    {
      statement_t * st = slist_it_get (it);
      slist_t * labels = stmt_labels (st);
      slist_it_t * jt = slist_iter (labels);
      for (; slist_it_has (jt); slist_it_next (jt))
	{
	  symbol_t * l = slist_it_get (jt);
	  tree label = private_label_build_generic (self, l, state);
	  bind_state_add_stmt (state, label);
	}
      delete_slist_it (jt);

      tree stmt = stmt_build_generic (st, state);
      bind_state_add_stmt (state, stmt);
    }
  delete_slist_it (it);

  return bind_state_build_block (state);
}

static void *
private_stmt_toplev_build_decl_for_symbol (symbol_t * sym, void * _state)
{
  // This is called from stmt_toplev_build_generic for each symbol
  // in symbol table.
  al60l_bind_state_t * state = _state;
  symbol_set_extra (sym, builtin_decl_get_generic (sym, state));
  return NULL;
}

void *
stmt_toplev_build_generic (container_t * self, void * _state)
{
  // Process builtins first.
  a60_symtab_each (container_symtab (self),
		   a60_symbol_callback (private_stmt_toplev_build_decl_for_symbol),
		   _state);

  // The toplev block should contain only one node: the actual program
  // node.  Pick it up, and dispatch on it.  Don't do the rest of
  // bookkeeping that we do with ordinary blocks (as in
  // stmt_container_build_generic), the toplev node is merely a
  // container for internal declarations.
  gcc_assert (slist_length (container_stmts (self)) == 1);
  return stmt_build_generic (a60_as_statement (slist_front (container_stmts (self))), _state);
}

// ------------------------------------
//   EXPRESSION
// ------------------------------------

tree
expr_build_generic (expression_t * expression, al60l_bind_state_t * state)
{
  return (tree)a60_visitor_dispatch (state->expression_build_generic, expression, expression, state);
}

void *
expr_int_build_generic (expression_t * self, void * data ATTRIBUTE_UNUSED)
{
  tree ret = build_int_cst (integer_type_node, expr_int_value (self));
  return ret;
}

void *
expr_real_build_generic (expression_t * self, void * data)
{
  REAL_VALUE_TYPE real;
  type_t * t = expr_type (self);
  tree ttt = type_build_generic (t, data);

  real_from_string3 (&real, estr_cstr (expr_real_value (self)), TYPE_MODE (ttt));

  if (REAL_VALUE_ISINF (real))
    pedwarn ("floating constant exceeds range of %qT", ttt);

  tree ret = build_real (ttt, real);
  return ret;
}

void *
expr_string_build_generic (expression_t * self, void * data ATTRIBUTE_UNUSED)
{
  estring_t const * s = expr_string_value (self);
  int len = estr_length (s) + 1; // +1 for trailing zero
  tree ret = build_string_literal (len, estr_cstr (s));
  private_set_location (ret, expr_cursor (self));
  return ret;
}

void *
expr_bool_build_generic (expression_t * self, void * data ATTRIBUTE_UNUSED)
{
  tree ret = build_int_cst (boolean_type_node, expr_bool_value (self));
  return ret;
}

void *
expr_idref_build_generic (expression_t * self, void * data ATTRIBUTE_UNUSED)
{
  // see if the idref was resolved
  symbol_t * sym = expr_symbol (self);
  gcc_assert (sym != NULL);

  // see if it has declaration, and answer it, if it has
  tree decl = symbol_extra (sym);
  gcc_assert (decl);
  return decl;
}

void *
expr_if_build_generic (expression_t * self, void * data)
{
  type_t * t = expr_type (self);
  tree ttt  = type_build_generic (t, data);
  tree cond = expr_build_generic (expr_if_cond (self), data);
  tree expt = expr_build_generic (expr_if_trueb (self), data);
  tree expf = expr_build_generic (expr_if_falseb (self), data);
  tree ret  = build3 (COND_EXPR, ttt, cond, expt, expf);
  private_set_location (ret, expr_cursor (self));
  return ret;
}

static tree
private_expr_build_binary_generic (expression_t * self, void * data, int op)
{
  type_t * t = expr_type (self);
  tree ttt = type_build_generic (t, data);
  tree op1 = expr_build_generic (expr_binary_left (self), data);
  tree op2 = expr_build_generic (expr_binary_right (self), data);
  tree ret = build2 (op, ttt, op1, op2);
  private_set_location (ret, expr_cursor (self));
  return ret;
}

static tree
private_expr_build_unary_generic (expression_t * self, void * data, int op)
{
  type_t * t = expr_type (self);
  tree ttt = type_build_generic (t, data);
  tree op1 = expr_build_generic (expr_unary_operand (self), data);
  tree ret = build1 (op, ttt, op1);
  private_set_location (ret, expr_cursor (self));
  return ret;
}

static void *
private_expr_build_ardiv (expression_t * self, void * data)
{
  tree ret =  private_expr_build_binary_generic (self, data, RDIV_EXPR);
  // int / int also yields real.  We must cast the operands explicitly
  // to allow for this.
  if (types_same (expr_type (expr_binary_left (self)), type_int ()))
    {
      tree tmp = TREE_OPERAND (ret, 0);
      TREE_OPERAND (ret, 0) = build1 (FLOAT_EXPR, TREE_TYPE (ret), tmp);
    }

  if (types_same (expr_type (expr_binary_right (self)), type_int ()))
    {
      tree tmp = TREE_OPERAND (ret, 1);
      TREE_OPERAND (ret, 1) = build1 (FLOAT_EXPR, TREE_TYPE (ret), tmp);
    }

  private_set_location (ret, expr_cursor (self));
  return ret;
}

static void *
private_expr_build_apow (expression_t * self, void * data)
{
  // Arithmetic power expressions are translated to function call into
  // runtime library.  The resulting symbol is named `pow`, and is
  // mangled during the translation of funcall to reflect the types of
  // the operands.

  type_t * tl = expr_type (expr_binary_left (self));
  type_t * tr = expr_type (expr_binary_right (self));

  static type_t * types[4] = {}; // NULLs by default
  static int initialized = 0;

  if (! initialized)
    {
      types[0] = type_proc_int_int_int ();
      types[1] = type_proc_real_int_real ();
      types[2] = type_proc_real_real_int ();
      types[3] = type_proc_real_real_real ();
      initialized = 1;
    }

  // sl, sr are indices used to compute
  // index into types array
  int sl = (types_same (tl, type_int ()) ? 0x00
	    : (gcc_assert (types_same (tl, type_real ())), 0x02));
  int sr = (types_same (tr, type_int ()) ? 0x00
	    : (gcc_assert (types_same (tr, type_real ())), 0x01));
  int typeidx = sl & sr; // index into types array

  label_t * l = new_label (new_estring_from ("pow"));
  slist_t * args = new_slist_from (2,
				   expr_binary_left (self),
				   expr_binary_right (self));
  cursor_t * c = expr_cursor (self);

  // Don't forget to do what resolve would do for us.  We can omit
  // typechecking, and subexpression resolving has already been done
  // by this point.
  symbol_t * sym = new_symbol_var (l);
  symbol_set_type (sym, types[typeidx]); // function type
  symbol_set_hidden (sym, 1);
  symbol_set_extra (sym, builtin_decl_get_generic (sym, data));
  expression_t * e = new_expr_call_sym (c, l, args, sym);

  tree ret = expr_build_generic (e, data);
  private_set_location (ret, expr_cursor (self));
  return ret;
}

static void *
private_expr_build_limp (expression_t * self, void * data)
{
  // `a => b` translates as `or (not (a), b)`
  // @@@TODO: btw, this is candidate for custom tree node!
  cursor_t * c = expr_cursor (self);
  expression_t * e1 = new_expr_unary (c, euk_not, expr_binary_left (self));
  expression_t * e2 = new_expr_binary (c, ebk_lor, e1, expr_binary_right (self));
  tree ret = expr_build_generic (e2, data);
  private_set_location (ret, expr_cursor (self));
  return ret;
}

void *
expr_binary_build_generic (expression_t * self, void * data)
{
  static int treecode[] = {
    [ebk_aadd] = PLUS_EXPR,  [ebk_asub] = MINUS_EXPR,
    [ebk_amul] = MULT_EXPR,  [ebk_aidiv] = TRUNC_DIV_EXPR,
    [ebk_req] = EQ_EXPR,     [ebk_rneq] = NE_EXPR,
    [ebk_rlt] = LT_EXPR,     [ebk_rlte] = LE_EXPR,
    [ebk_rgt] = GT_EXPR,     [ebk_rgte] = GE_EXPR,
    [ebk_leq] = EQ_EXPR,     [ebk_lor] = TRUTH_OR_EXPR,
    [ebk_land] = TRUTH_AND_EXPR
  };

  expr_binop_t op = expr_binary_op (self);
  switch (op)
    {
    case ebk_aadd:    case ebk_asub:    case ebk_amul:
    case ebk_aidiv:   case ebk_req:     case ebk_rneq:
    case ebk_rlt:     case ebk_rlte:    case ebk_rgt:
    case ebk_rgte:    case ebk_leq:     case ebk_lor:
    case ebk_land:
      return private_expr_build_binary_generic (self, data, treecode[op]);

    case ebk_ardiv:
      return private_expr_build_ardiv (self, data);
    case ebk_apow:
      return private_expr_build_apow (self, data);
    case ebk_limp:
      return private_expr_build_limp (self, data);
    };
  gcc_unreachable ();
}

void *
expr_unary_build_generic (expression_t * self, void * data)
{
  expr_unop_t op = expr_unary_op (self);
  switch (op)
    {
    case euk_uminus:
      return private_expr_build_unary_generic (self, data, NEGATE_EXPR);
    case euk_not:
      return private_expr_build_unary_generic (self, data, TRUTH_NOT_EXPR);
    };
  gcc_unreachable ();
}

void *
expr_call_build_generic (expression_t * self, void * state)
{
  symbol_t * sym = expr_symbol (self);
  gcc_assert (sym != NULL); // i.e. resolve_symbols was called already

  tree proc_decl = symbol_extra (sym);
  gcc_assert (proc_decl != NULL);

  tree arg_list = NULL_TREE;
  slist_it_t * it = slist_iter (expr_call_args (self));
  for (; slist_it_has (it); slist_it_next (it))
    {
      expression_t * expr = slist_it_get (it);
      tree arg_expr = expr_build_generic (expr, state);
      arg_list = tree_cons (NULL_TREE, arg_expr, arg_list);
    }
  delete_slist_it (it);
  arg_list = nreverse (arg_list);

  tree ret = build_function_call_expr (proc_decl, arg_list);
  TREE_SIDE_EFFECTS (ret) = 1;
  TREE_USED (ret) = 1;

  private_set_location (ret, expr_cursor (self));
  return ret;
}

void *
expr_subscript_build_generic (expression_t * self, void * data)
{
  symbol_t * sym = expr_symbol (self);
  tree ret = symbol_extra (sym);
  type_t * t = symbol_type (sym);
  gcc_assert (ret != NULL);
  gcc_assert (t != NULL);

  slist_it_t * it = slist_iter (expr_subscript_indices (self));
  for (; slist_it_has (it); slist_it_next (it))
    {
      expression_t * idx_expr = slist_it_get (it);
      tree idx_tree = expr_build_generic (idx_expr, data);
      t = type_host (t);
      tree type = type_build_generic (t, data);
      ret = build4 (ARRAY_REF, type, ret, idx_tree, NULL_TREE, NULL_TREE);
    }
  delete_slist_it (it);

  private_set_location (ret, expr_cursor (self));
  return ret;
}

static void
private_mangle_for_type (type_t const * t, estring_t * ret)
{
  if (types_same (t, type_int ()))
    estr_push (ret, 'i');
  else if (types_same (t, type_real ()))
    estr_push (ret, 'r');
  else if (types_same (t, type_string ()))
    estr_push (ret, 's');
  else if (types_same (t, type_void ()))
    estr_push (ret, 'v');
  else if (types_same (t, type_bool ()))
    estr_push (ret, 'b');
  else if (types_same (t, type_label ()))
    estr_push (ret, 'l');
  else if (type_is_own (t))
    private_mangle_for_type (type_host (t), ret);
  else if (type_is_array (t))
    {
      estr_push (ret, 'A');
      private_mangle_for_type (type_host (t), ret);
    }
  else if (type_is_proc (t))
    {
      estr_push (ret, 'P');
      private_mangle_for_type (t_proc_return_type (t), ret);
      slist_t * a = t_proc_arg_types (t);
      slist_it_t * it = slist_iter (a);
      for (; slist_it_has (it); slist_it_next (it))
	private_mangle_for_type (slist_it_get (it), ret);
      delete_slist_it (it);
      estr_push (ret, 'Q');
    }
  else
    gcc_unreachable ();
}

static estring_t *
private_builtin_asm_name (symbol_t const * sym)
{
  estring_t const * orig_name = label_id (symbol_label (sym));
  estring_t * ret = new_estring_fmt ("__a60__%s_", estr_cstr (orig_name));
  private_mangle_for_type (symbol_type (sym), ret);
  return ret;
}

void *
builtin_decl_get_generic (symbol_t * sym, void * _state)
{
  type_t * t = symbol_type (sym);

  int own = 0;
  if (type_is_own (t))
    {
      own = 1;
      t = type_host (t);
    }

  // Builtin symbols, defined inside libga60, are mangled
  label_t * asm_label = new_label (private_builtin_asm_name (sym));
  sym = clone_symbol_with_name (sym, asm_label);

  tree decl = symbol_decl_build_generic (sym, _state);

  // Builtins have file scope.
  DECL_CONTEXT (decl) = NULL_TREE;

  // In a VAR_DECL or FUNCTION_DECL, nonzero means external reference:
  // do not allocate storage, and refer to a definition elsewhere.
  DECL_EXTERNAL (decl) = 1; // all builtins are external

  // In a VAR_DECL, FUNCTION_DECL, NAMESPACE_DECL or TYPE_DECL,
  // nonzero means name is to be accessible from outside this module.
  TREE_PUBLIC (decl) = 1;

  // In a VAR_DECL, nonzero means allocate static storage.
  // In a FUNCTION_DECL, nonzero if function has been defined.
  if (type_is_proc (t))
    TREE_STATIC (decl) = 0; // builtin functions are undefined
  else
    TREE_STATIC (decl) = own;

  rest_of_decl_compilation (decl, /*top_level=*/1, /*at_end=*/0);

  return decl;
}


// ------------------------------------
//   DESIGNATIONAL EXPRESSION
// ------------------------------------

tree
desig_expr_build_generic (desig_expr_t * desig_expr, al60l_bind_state_t * state)
{
  return (tree)a60_visitor_dispatch (state->desig_expr_build_generic, desig_expr, desig_expr, state);
}

void *
desig_expr_label_build_generic (desig_expr_t * self,
				void * _state ATTRIBUTE_UNUSED)
{
  symbol_t * sym = desig_expr_symbol (self);
  tree decl = symbol_extra (sym);
  TREE_USED (decl) = 1;
  tree ret = build1 (ADDR_EXPR, ptr_type_node, decl);

  return ret;
}

void *
desig_expr_if_build_generic (desig_expr_t * self,
			     void * _state)
{
  al60l_bind_state_t * state = _state;

  desig_expr_t * bt = desig_expr_if_trueb (self);
  desig_expr_t * bf = desig_expr_if_falseb (self);
  expression_t * cond = desig_expr_if_cond (self);

  tree tbt = convert (ptr_type_node, desig_expr_build_generic (bt, state));
  tree tbf = convert (ptr_type_node, desig_expr_build_generic (bf, state));
  tree tcond = expr_build_generic (cond, state);

  tree ret  = build3 (COND_EXPR, ptr_type_node, tcond, tbt, tbf);

  return ret;
}

void *
desig_expr_switch_build_generic (desig_expr_t * self, void * _state)
{
  al60l_bind_state_t * state = _state;

  symbol_t * sym = desig_expr_symbol (self);
  tree tsym = symbol_extra (sym);

  expression_t * index = desig_expr_switch_index (self);
  tree tindex = expr_build_generic (index, state);

  tree ret = build4 (ARRAY_REF, ptr_type_node, tsym, tindex, NULL_TREE, NULL_TREE);
  return ret;
}


// ------------------------------------
//   SYMBOL -- both DECL & INIT
// ------------------------------------

tree
symbol_decl_build_generic (symbol_t * symbol, al60l_bind_state_t * state)
{
  return (tree)a60_visitor_dispatch (state->symbol_decl_build_generic, symbol, symbol, state);
}

tree
symbol_init_build_generic (symbol_t * symbol, al60l_bind_state_t * state)
{
  return (tree)a60_visitor_dispatch (state->symbol_init_build_generic, symbol, symbol, state);
}

void *
symbol_var_decl_build_generic (symbol_t * sym, void * _state)
{
  al60l_bind_state_t * state = _state;
  type_t const * sym_type = symbol_type (sym);
  return (tree)a60_visitor_dispatch (state->symbol_decl_for_type, sym_type, sym, state);
}

void *
symbol_fun_decl_build_generic (symbol_t * sym ATTRIBUTE_UNUSED,
			       void * _state ATTRIBUTE_UNUSED)
{
  // Do nothing for now.
  // @TODO Do the right thing.
  return NULL_TREE;
}

void *
symbol_formparm_decl_build_generic (symbol_t * sym ATTRIBUTE_UNUSED,
				    void * _state ATTRIBUTE_UNUSED)
{
  // Do nothing for now.
  // @TODO Do the right thing.
  return NULL_TREE;
}


void *
symbol_var_init_build_generic (symbol_t * sym, void * _state)
{
  al60l_bind_state_t * state = _state;
  type_t const * sym_type = symbol_type (sym);
  return (tree)a60_visitor_dispatch (state->symbol_init_for_type, sym_type, sym, state);
}

void *
symbol_fun_init_build_generic (symbol_t * sym ATTRIBUTE_UNUSED,
			       void * _state ATTRIBUTE_UNUSED)
{
  // Do nothing for now.
  // @TODO Do the right thing.
  return NULL_TREE;
}

void *
symbol_formparm_init_build_generic (symbol_t * sym ATTRIBUTE_UNUSED,
				    void * _state ATTRIBUTE_UNUSED)
{
  // Do nothing for now.
  // @TODO Do the right thing.
  return NULL_TREE;
}


void *
symbol_var_init_omit (symbol_t * sym ATTRIBUTE_UNUSED,
		  void * _state ATTRIBUTE_UNUSED)
{
  // do nothing.
  return NULL_TREE;
}

void *
symbol_var_init_error (symbol_t * sym ATTRIBUTE_UNUSED,
		   void * _state ATTRIBUTE_UNUSED)
{
  // This is called for `unknown', `implicit' and `any' metatypes.
  gcc_unreachable ();
}


void *
symbol_var_ordinary_build_generic (symbol_t * sym, void * data)
{
  // Handle only regular symbols
  label_t const * lbl = symbol_label (sym);
  tree id = get_identifier (estr_cstr (label_id (lbl)));
  tree tt = type_build_generic (symbol_type (sym), data);
  return build_decl (VAR_DECL, id, tt);
}

void *
symbol_var_invalid_build_generic (symbol_t * sym ATTRIBUTE_UNUSED,
			 void * data ATTRIBUTE_UNUSED)
{
  gcc_assert (!"You shouldn't ask for GENERIC of `unknown', `implicit' or `any'.");
  gcc_unreachable ();
}

void *
symbol_var_label_build_generic (symbol_t * sym, void * data ATTRIBUTE_UNUSED)
{
  label_t const * lbl = symbol_label (sym);
  tree id = get_identifier (estr_cstr (label_id (lbl)));
  return build_decl (LABEL_DECL, id, void_type_node);
}

void *
symbol_var_proc_build_generic (symbol_t * sym, void * data)
{
  type_t * t = symbol_type (sym);
  tree fn_type = type_proc_build_generic (t, data);
  char const* name = estr_cstr (label_id (symbol_label (sym)));
  tree fn_decl = build_fn_decl (name, fn_type);

  /*
  // build declaration parameters
  tree param_decls = NULL_TREE;

  char buf[15];
  int num = 0;
  slist_it_t * it = slist_iter (ast_as (t_proc, sym->type)->arg_types);
  for (; slist_it_has (it); slist_it_next (it))
    {
      type * t = slist_it_get (it);
      tree argt = type_build_generic (t, data);
      snprintf (buf, sizeof (buf), ".arg.%d", num);
      tree parm = build_decl (PARM_DECL, get_identifier (buf), argt);
      DECL_CONTEXT (parm) = fn_decl;
      param_decls = tree_cons (NULL_TREE, parm, param_decls);
      ++num;
    }
  delete_slist_it (it);
  param_decls = nreverse (param_decls);

  DECL_ARGUMENTS (fn_decl) = param_decls;
  */

  return fn_decl;
}

//
// COMPILATION OF COMPUTED GOTO
// ----------------------------
//
// 'begin' 'comment' variable `x' comes from outer scope;
//   'integer' y;
//   'switch' r := a, b, c
//   'switch' s := d, r[x], 'if' x > 1 'then' r[x] 'else' d
//
//   y := x + 1;
//   'goto' s[y];
//
//   a: puts (`a');
//   b: puts (`b');
//   c: puts (`c');
//   d: puts (`d');
// 'end';
//
// Is translated like this:
//
// {
//   void * .sw.r[3];      /* switch declaration */
//   void * .sw.s[3];      /* switch declaration */
//
//   {                     /* init of switch R */
//     goto .init;
//     .init:
//       sw.s = {&&a, &&b, &&c};
//   }
//
//   {                     /* init of switch S */
//     goto .init;
//     .sw.s.2:
//       goto *.sw.r[x];
//     .sw.s.3:
//       goto (x > 1 ? *.sw.r[x] : d);
//     .init:
//       sw.s = {&&d, &&.sw.s.2, &&.sw.s.3};
//   }
//
//   y = x + 1;            /* start of normal code */
//   goto *.sw.s[y];
//
//   a: puts ("a");
//   b: puts ("b");
//   c: puts ("c");
//   d: puts ("d");
// }

void *
symbol_var_switch_build_generic (symbol_t * sym, void * _state)
{
  // Build declaration node for switch.  Switches are handled as
  // arrays of void pointers, each pointer pointing to one
  // designational expression.

  al60l_bind_state_t * state = _state;

  label_t const * lbl = symbol_label (sym);
  tree id = get_identifier (estr_cstr (label_id (lbl)));

  slist_t * switchlist = t_switch_switchlist (symbol_type (sym));
  long len = slist_length (switchlist);
  tree high_bound = expr_build_generic (new_expr_int (NULL, len), state);
  tree array_range = build_range_type (integer_type_node, integer_one_node, high_bound);
  tree array_type = build_array_type (ptr_type_node, array_range);

  return build_decl (VAR_DECL, id, array_type);
}

void *
symbol_var_switch_init_build_generic (symbol_t * sym, void * _state)
{
  al60l_bind_state_t * state = _state;
  tree sym_decl = symbol_extra (sym);

  bind_state_push_block (state);

  slist_t * switchlist = t_switch_switchlist (symbol_type (sym));

  tree init_decl = create_artificial_label ();
  bind_state_add_decl (state, init_decl);

  tree goto_init = build1 (GOTO_EXPR, void_type_node, init_decl);
  bind_state_add_stmt (state, goto_init);

  slist_t * init_exprs = new_slist ();
  slist_it_t * it = slist_iter (switchlist);
  long idx = 1;
  for (; slist_it_has (it); slist_it_next (it))
    {
      tree index = build_int_cst (integer_type_node, idx);
      desig_expr_t *de = a60_as_desig_expr (slist_it_get (it));
      tree value = a60_visitor_dispatch (state->symbol_var_switch_init_elmt_build_generic, de, de, state);

      tree element = build4 (ARRAY_REF, ptr_type_node, sym_decl, index, NULL_TREE, NULL_TREE);
      tree elt_init = build2 (INIT_EXPR, ptr_type_node, element, value);
      slist_pushback (init_exprs, elt_init);

      idx++;
    }

  tree init_lbl  = build1 (LABEL_EXPR, void_type_node, init_decl);
  bind_state_add_stmt (state, init_lbl);

  slist_it_reset (it, init_exprs);
  for (; slist_it_has (it); slist_it_next (it))
    bind_state_add_stmt (state, slist_it_get (it));

  delete_slist_it (it);
  delete_slist (init_exprs);

  tree ret = bind_state_build_block (state);
  return ret;
}

void *
switch_label_init_build_generic (desig_expr_t * de, void * _state)
{
  // The case of simple label is optimized: no artificial label is
  // created, and the jump is done directly, not through dispatch as
  // in case of `if' and `switch'-style computed goto.

  al60l_bind_state_t * state = _state;
  return desig_expr_build_generic (de, state);
}

void *
switch_compute_init_build_generic (desig_expr_t * de, void * _state)
{
  // This callback is used both for `if' and `switch'-style computed goto.
  // We dispatch the computation of goto to the DE->GENERIC builder.

  al60l_bind_state_t * state = _state;

  tree tmp_lbl_decl = create_artificial_label ();
  tree tmp_label = build1 (LABEL_EXPR, void_type_node, tmp_lbl_decl);
  bind_state_add_decl (state, tmp_lbl_decl);
  bind_state_add_stmt (state, tmp_label);

  tree target = desig_expr_build_generic (de, state);
  tree jump = build1 (GOTO_EXPR, void_type_node, target);
  bind_state_add_stmt (state, jump);

  return build1 (ADDR_EXPR, ptr_type_node, tmp_lbl_decl);
}


// ------------------------------------
//   TYPE
// ------------------------------------

tree
type_build_generic (type_t * type, al60l_bind_state_t * state)
{
  return (tree)a60_visitor_dispatch (state->type_build_generic, type, type, state);
}

void *
type_unknown_build_generic (type_t * self ATTRIBUTE_UNUSED,
			    void * data ATTRIBUTE_UNUSED)
{
  gcc_assert (!"You shouldn't ask for GENERIC of `unknown' or `implicit'.");
  gcc_unreachable ();
}

void *
type_any_build_generic (type_t * self ATTRIBUTE_UNUSED,
			void * data ATTRIBUTE_UNUSED)
{
  gcc_assert (!"You shouldn't ask for GENERIC of `any'.");
  gcc_unreachable ();
}

void *
type_own_build_generic (type_t * self, void * data)
{
  tree ret = type_build_generic (type_host (self), data);
  TREE_STATIC (ret) = 1;
  return ret;
}

void *
type_int_build_generic (type_t * self ATTRIBUTE_UNUSED,
			void * data ATTRIBUTE_UNUSED)
{
  return integer_type_node;
}

void *
type_void_build_generic (type_t * self ATTRIBUTE_UNUSED,
			 void * data ATTRIBUTE_UNUSED)
{
  return void_type_node;
}

void *
type_real_build_generic (type_t * self ATTRIBUTE_UNUSED,
			 void * data ATTRIBUTE_UNUSED)
{
  return double_type_node;
}

void *
type_string_build_generic (type_t * self ATTRIBUTE_UNUSED,
			   void * data ATTRIBUTE_UNUSED)
{
  return string_type_node;
}

void *
type_bool_build_generic (type_t * self ATTRIBUTE_UNUSED,
			 void * data ATTRIBUTE_UNUSED)
{
  return boolean_type_node;
}

void *
type_label_build_generic (type_t * self ATTRIBUTE_UNUSED,
			  void * data ATTRIBUTE_UNUSED)
{
  gcc_unreachable ();
}

void *
type_switch_build_generic (type_t * self ATTRIBUTE_UNUSED,
			   void * data ATTRIBUTE_UNUSED)
{
  // This is handled in symbol_var_switch_build_generic.
  gcc_unreachable ();
}

void *
type_array_build_generic (type_t * self, void * data)
{
  tree emtt = type_build_generic (type_host (self), data);
  boundspair_t * bp = t_array_bounds (self);
  gcc_assert (bp != NULL);
  expression_t * h = boundspair_hi (bp);
  expression_t * l = boundspair_lo (bp);
  tree lowb = expr_build_generic (l, data);
  tree highb = expr_build_generic (h, data);
  tree arridxt = type_build_generic (type_int (), data);
  tree arrbdst = build_range_type (arridxt, lowb, highb);
  tree ret = build_array_type (emtt, arrbdst);
  return ret;
}

void *
type_proc_build_generic (type_t * self, void * data)
{
  tree fn_ret_type = type_build_generic (t_proc_return_type (self), data);
  tree param_types = NULL_TREE;
  slist_it_t * it = slist_iter (t_proc_arg_types (self));
  for (; slist_it_has (it); slist_it_next (it))
    {
      tree t1 = type_build_generic (slist_it_get (it), data);
      param_types = tree_cons (NULL_TREE, t1, param_types);
    }
  delete_slist_it (it);
  param_types = nreverse (param_types);

  tree fn_type = build_function_type (fn_ret_type, param_types);
  return fn_type;
}



// ------------------------------------
//   FOR ELEMENT
// ------------------------------------

void *
for_elmt_expr_build_generic (for_elmt_t * for_elmt, void * _state)
{
  //   for i := A do <body>
  //
  // is translated like this:
  //
  //   i := expr;
  //   <body>;

  al60l_bind_state_t * state = _state;
  statement_t * parent_for_stmt = a60_as_statement (slist_front (state->for_statements));
  statement_t * for_body = stmt_for_body (parent_for_stmt);
  expression_t * loop_ctrl_var = stmt_for_variable (parent_for_stmt);

  bind_state_push_block (state);

  expression_t * rhs = for_elmt_expr_expr (for_elmt);

  tree lhs_tree = expr_build_generic (loop_ctrl_var, state);
  tree rhs_tree = expr_build_generic (rhs, state);
  tree assign_tree = build2 (MODIFY_EXPR, void_type_node, lhs_tree, rhs_tree);

  bind_state_add_stmt (state, assign_tree);
  bind_state_add_stmt (state, stmt_build_generic (for_body, state));

  tree ret = bind_state_build_block (state);
  return ret;
}

void *
for_elmt_until_build_generic (for_elmt_t * for_elmt, void * _state)
{
  //   for i := A step B until C do <body>;
  //
  // is translated like this:
  //
  //   i := A;
  //   LOOP_EXPR (void, {
  //    1: EXIT_EXPR (void, ((i - C) * (if B > 0 then 1 else if B < 0 then -1 else 0)) > 0);
  //    2: <body>;
  //    3: i := i + B;
  //   })

  al60l_bind_state_t * state = _state;
  statement_t * parent_for_stmt = a60_as_statement (slist_front (state->for_statements));
  statement_t * for_body = stmt_for_body (parent_for_stmt);
  expression_t * loop_ctrl_var = stmt_for_variable (parent_for_stmt);

  bind_state_push_block (state);

  expression_t * start = for_elmt_until_start (for_elmt);
  expression_t * stop = for_elmt_until_stop (for_elmt);
  expression_t * step = for_elmt_until_step (for_elmt);
  cursor_t * csr = expr_cursor (step);

  // initialization element
  {
    tree lhs_tree = expr_build_generic (loop_ctrl_var, state);
    expression_t * rhs = start;
    tree rhs_tree = expr_build_generic (rhs, state);
    tree assign_tree = build2 (MODIFY_EXPR, void_type_node, lhs_tree, rhs_tree);
    bind_state_add_stmt (state, assign_tree);
  }

  // this block is a body of the endless loop
  bind_state_push_block (state);

  // #1
  {
    expression_t * i_minus_C = new_expr_binary (csr, ebk_asub, loop_ctrl_var, stop);
    // if B < 0 then -1 else 0
    expression_t * expr2 = new_expr_if (csr,
					new_expr_binary (csr, ebk_rlt, step,
							 new_expr_int (csr, 0)),
					new_expr_int (csr, -1),
					new_expr_int (csr, 0));
    // if B > 0 then 1 else <expr2>
    expression_t * sgn_B = new_expr_if (csr,
					new_expr_binary (csr, ebk_rgt, step,
							 new_expr_int (csr, 0)),
					new_expr_int (csr, 1),
					expr2);
    // whole exit condition
    expression_t * cond = new_expr_binary (csr, ebk_rgt,
					   new_expr_binary (csr, ebk_amul, i_minus_C, sgn_B),
					   new_expr_int (csr, 0));
    expr_resolve_symbols (cond, stmt_parent (parent_for_stmt), (logger_t*)0xdeadbeef); // logger shouldn't be necessary
    tree cond_tree = expr_build_generic (cond, state);
    tree break_tree = build1 (EXIT_EXPR, void_type_node, cond_tree);
    bind_state_add_stmt (state, break_tree);
  }

  // #2
  {
    bind_state_add_stmt (state, stmt_build_generic (for_body, state));
  }

  // #3
  {
    tree lhs_tree = expr_build_generic (loop_ctrl_var, state);
    expression_t * rhs = new_expr_binary (csr, ebk_aadd, loop_ctrl_var, step);
    tree rhs_tree = expr_build_generic (rhs, state);
    tree assign_tree = build2 (MODIFY_EXPR, void_type_node, lhs_tree, rhs_tree);
    bind_state_add_stmt (state, assign_tree);
  }

  // and build the loop body & the loop itself
  tree endless_body = bind_state_build_block (state);
  tree stmt = build1 (LOOP_EXPR, void_type_node, endless_body);
  bind_state_add_stmt (state, stmt);

  tree ret = bind_state_build_block (state);
  return ret;
}

void *
for_elmt_while_build_generic (for_elmt_t * for_elmt, void * _state)
{
  //   for i := E while F do <body>;
  //
  // is translated like this:
  //
  //   LOOP_EXPR (void, {
  //    1: i := E;
  //    2: EXIT_EXPR (void, !F);
  //    3: <body>;
  //   })

  al60l_bind_state_t * state = _state;
  statement_t * parent_for_stmt = a60_as_statement (slist_front (state->for_statements));
  statement_t * for_body = stmt_for_body (parent_for_stmt);
  expression_t * loop_ctrl_var = stmt_for_variable (parent_for_stmt);

  bind_state_push_block (state);

  // this block is a body of the LOOP_EXPR
  bind_state_push_block (state);

  // #1
  tree lhs_tree = expr_build_generic (loop_ctrl_var, state);
  expression_t * rhs = for_elmt_while_expr (for_elmt);
  tree rhs_tree = expr_build_generic (rhs, state);
  tree assign_tree = build2 (MODIFY_EXPR, void_type_node, lhs_tree, rhs_tree);
  bind_state_add_stmt (state, assign_tree);

  // #2
  expression_t * cond = for_elmt_while_cond (for_elmt);
  cond = new_expr_unary (expr_cursor (cond), euk_not, cond);
  tree cond_tree = expr_build_generic (cond, state);
  tree break_tree = build1 (EXIT_EXPR, void_type_node, cond_tree);
  bind_state_add_stmt (state, break_tree);

  // #3
  bind_state_add_stmt (state, stmt_build_generic (for_body, state));

  // build the loop body & the loop itself
  tree endless_body = bind_state_build_block (state);
  tree stmt = build1 (LOOP_EXPR, void_type_node, endless_body);
  bind_state_add_stmt (state, stmt);

  tree ret = bind_state_build_block (state);
  return ret;
}
