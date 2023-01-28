#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "statement.h"
#include "cursor.h"
#include "slist.h"
#include "expression.h"
#include "desig-expr.h"
#include "logger.h"
#include "symbol.h"
#include "label.h"
#include "type.h"
#include "estring.h"
#include "boundspair.h"
#include "for-elmt.h"
#include "meta.h"
#include "a60_symtab.h"
#include "visitor-impl.h"

static char const * private_statement_signature = "statement";

typedef struct struct_stmt_dummy_rep_t
{
}
stmt_dummy_rep_t;

typedef struct struct_stmt_assign_rep_t
{
  expression_t * rhs;
  slist_t * lhss;
}
stmt_assign_rep_t;

typedef struct struct_stmt_call_rep_t
{
  expression_t * call;
}
stmt_call_rep_t;

typedef struct struct_stmt_cond_rep_t
{
  expression_t * cond;
  statement_t * ifclause;
  statement_t * elseclause;
}
stmt_cond_rep_t;

typedef struct struct_stmt_for_rep_t
{
  expression_t * variable;
  slist_t * elmts;
  statement_t * body;
}
stmt_for_rep_t;

typedef struct struct_stmt_goto_rep_t
{
  desig_expr_t * target;
}
stmt_goto_rep_t;

/// Used for blocks as well as for toplevs.
typedef struct struct_container_rep_t
{
  slist_t * statements;
  a60_symtab_t * symtab;
}
container_rep_t;

typedef enum enum_stmt_kind_t
{
  sk_dummy,
  sk_assign,
  sk_call,
  sk_cond,
  sk_for,
  sk_goto,
  sk_block,
  sk_toplev,
}
stmt_kind_t;

struct struct_statement_t
{
  visitable_t base;

  cursor_t * cursor;
  container_t * parent;
  slist_t * labels;
  union {
    stmt_dummy_rep_t dummy;
    stmt_assign_rep_t assign;
    stmt_call_rep_t call;
    stmt_cond_rep_t cond;
    stmt_for_rep_t afor;
    stmt_goto_rep_t agoto;
    container_rep_t block; ///< Used by both block and toplev.
  };
};

static void *
private_check_symbol_label (void * ptr, void * data ATTRIBUTE_UNUSED)
{
  symbol_t * sym = a60_as_symbol (ptr);
  if (!types_same (symbol_type (sym), type_label ()))
    sym = NULL;
  return sym;
}

static statement_t *
private_new_statement (stmt_kind_t kind, cursor_t * cursor, container_t * parent)
{
  statement_t * ret = malloc (sizeof (statement_t));
#ifndef NDEBUG
  ret->base.signature = private_statement_signature;
#endif
  ret->base.kind = kind;
  ret->cursor = cursor;
  ret->parent = parent;
  ret->labels = new_slist_typed (private_check_symbol_label, NULL);
  return ret;
}

statement_t *
new_stmt_dummy (cursor_t * cursor)
{
  return private_new_statement (sk_dummy, cursor, NULL);
}

static void *
private_check_expr_lvalue (void * ptr, void * data ATTRIBUTE_UNUSED)
{
  expression_t * expr = a60_as_expression (ptr);
  if (expr && !expr_is_lvalue (expr))
    expr = NULL;
  return expr;
}

statement_t *
new_stmt_assign (cursor_t * cursor, slist_t * lhss, expression_t * rhs)
{
  assert (lhss != NULL);
  assert (rhs != NULL);

  statement_t * ret = private_new_statement (sk_assign, cursor, NULL);
  slist_set_type (lhss, private_check_expr_lvalue, NULL);
  ret->assign.lhss = lhss;
  ret->assign.rhs = rhs;

  return ret;
}

statement_t *
new_stmt_call (cursor_t * cursor, expression_t * call)
{
  assert (call != NULL);

  statement_t * ret = private_new_statement (sk_call, cursor, NULL);
  ret->call.call = call;
  return ret;
}

statement_t *
new_stmt_cond (cursor_t * cursor, expression_t * cond,
	       statement_t * ifclause, statement_t * elseclause)
{
  assert (cond != NULL);
  assert (ifclause != NULL);

  statement_t * ret = private_new_statement (sk_cond, cursor, NULL);
  ret->cond.cond = cond;
  ret->cond.ifclause = ifclause;
  ret->cond.elseclause = elseclause;
  return ret;
}

statement_t *
new_stmt_for (cursor_t * cursor, expression_t * variable, slist_t * elmts, statement_t * body)
{
  assert (variable != NULL);
  assert (elmts != NULL);
  assert (body != NULL);

  statement_t * ret = private_new_statement (sk_for, cursor, NULL);
  ret->afor.variable = variable;
  slist_set_type (elmts, adapt_test, a60_as_for_elmt);
  ret->afor.elmts = elmts;
  ret->afor.body = body;
  return ret;
}

statement_t *
new_stmt_goto (cursor_t * cursor, desig_expr_t * target)
{
  assert (target != NULL);

  statement_t * ret = private_new_statement (sk_goto, cursor, NULL);
  ret->agoto.target = target;
  return ret;
}

static container_t *
private_new_container (stmt_kind_t kind, cursor_t * cursor, a60_symtab_t * symtab)
{
  statement_t * ret = private_new_statement (kind, cursor, NULL);
  ret->block.statements = new_slist_typed (adapt_test, a60_as_statement);
  ret->block.symtab = symtab;
  return (container_t *)ret;
}

container_t *
new_stmt_block (cursor_t * cursor, a60_symtab_t * symtab)
{
  return private_new_container (sk_block, cursor, symtab);
}

container_t *
new_stmt_toplev (cursor_t * cursor, a60_symtab_t * symtab)
{
  return private_new_container (sk_toplev, cursor, symtab);
}

static void *
private_symtab_pick_labels (symbol_t * sym, void * _labels)
{
  slist_t * labels = a60_as_slist (_labels);
  if (types_same (symbol_type (sym), type_label ()))
    slist_pushback (labels, sym);
  return NULL;
}

statement_t *
clone_statement (statement_t const * self)
{
  assert (self != NULL);

  statement_t * ret = private_new_statement (self->base.kind, self->cursor, NULL);

  switch (self->base.kind)
    {
    case sk_dummy:
      break;

    case sk_assign:
      ret->assign.lhss = clone_slist (self->assign.lhss);
      slist_map (ret->assign.lhss, adapt_test, clone_expression);
      ret->assign.rhs = clone_expression (self->assign.rhs);
      break;

    case sk_call:
      ret->call.call = clone_expression (self->call.call);
      break;

    case sk_cond:
      ret->cond.cond = clone_expression (self->cond.cond);
      ret->cond.ifclause = clone_statement (self->cond.ifclause);
      ret->cond.elseclause
	= ret->cond.elseclause ? clone_statement (self->cond.elseclause) : NULL;
      break;

    case sk_for:
      ret->afor.variable = clone_expression (self->afor.variable);
      ret->afor.elmts = clone_slist (self->afor.elmts);
      slist_map (ret->afor.elmts, adapt_test, clone_for_elmt);
      ret->afor.body = clone_statement (self->afor.body);
      break;

    case sk_goto:
      ret->agoto.target = clone_desig_expr (self->agoto.target);
      break;

    case sk_block:
    case sk_toplev:
      {
	ret->block.symtab = a60_clone_symtab (self->block.symtab);
	ret->block.statements = new_slist_typed (adapt_test, a60_as_statement);

	// extract labels from the list
	slist_t * labels = new_slist ();
	a60_symtab_each (ret->block.symtab,
			 a60_symbol_callback (private_symtab_pick_labels),
			 labels);
	slist_it_t * jt = slist_iter (labels);

	// clone statement list and retarget labels in one pass
	slist_it_t * it = slist_iter (self->block.statements);
	for (; slist_it_has (it); slist_it_next (it))
	  {
	    statement_t * stmt = slist_it_get (it);
	    statement_t * clone = clone_statement (stmt);
	    container_add_stmt (a60_as_container (ret), clone);

	    slist_it_reset (jt, labels);
	    for (; slist_it_has (jt); slist_it_next (jt))
	      {
		symbol_t * sym = slist_it_get (jt);
		statement_t * target = symbol_stmt (sym);
		if (target == stmt)
		  {
		    symbol_set_stmt (sym, clone);
		    stmt_add_label (clone, sym);
		  }
	      }
	  }

	delete_slist_it (jt);
	delete_slist_it (it);
	delete_slist (labels);
      }
      break;
    };

  return ret;
}

container_t *
clone_container (container_t const * self)
{
  return a60_as_container (clone_statement (a60_as_statement ((container_t *)self)));
}

statement_t *
a60_as_statement (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_statement_signature);
#endif
  return (statement_t *)obj;
}

container_t *
a60_as_container (void * obj)
{
#ifndef NDEBUG
  statement_t * st = a60_as_statement (obj);
  assert (st->base.kind == sk_block
	  || st->base.kind == sk_toplev);
#endif
  return (container_t *)obj;
}

static char const *
padding (int level)
{
  static char const* padding =
    "                                                            "
    "                                                            "
    "                                                            "
    "                                                            "
    "                                                            ";
  if (level > 300)
    level = 300;
  return padding + 300 - level;
}


static void private_stmt_dump (statement_t const * self, estring_t * buf, int level);

static void
private_dump_container (statement_t const * self ATTRIBUTE_UNUSED,
			estring_t * buf ATTRIBUTE_UNUSED,
			int level ATTRIBUTE_UNUSED)
{
  /*
  estring_t * buf0 = new_estring ();

  // dump all variables but labels and hidden symbols
  slist_it_t * it = slist_iter (self->block.symtab);
  for (; slist_it_has (it); slist_it_next (it))
    {
      symbol_t * sym = slist_it_get (it);
      type_t const * symtype = symbol_type (sym);
      if (types_same (symtype, type_label ())
	  || symbol_hidden (sym))
	continue;

      type_to_str_canon (symtype, buf0);
      estr_append_cstr (buf, padding (level));
      estr_append (buf, buf0);
      estr_push (buf, ' ');
      estr_append (buf, label_id (symbol_label (sym)));
      if (type_is_own (symtype))
	symtype = type_host (symtype);
      if (type_is_array (symtype))
	{
	  estr_push (buf, '[');
	  int first = 1;
	  for (; type_is_array (symtype); symtype = type_host (symtype))
	    {
	      boundspair_t * bp = t_array_bounds (symtype);
	      if (first)
		first = 0;
	      else
		estr_push (buf, ',');
	      expr_to_str (boundspair_lo (bp), buf0);
	      estr_append (buf, buf0);
	      estr_push (buf, ':');
	      expr_to_str (boundspair_hi (bp), buf0);
	      estr_append (buf, buf0);
	    }
	  estr_push (buf, ']');
	}
      estr_append_cstr (buf, ";\n");
    }

  slist_it_reset (it, self->block.statements);

  // dump all statemenets
  for (;slist_it_has (it);)
    {
      statement_t * stmt = slist_it_get (it);

      // look for any labels pointing to this command
      slist_it_t * lt = slist_iter (stmt_labels (stmt));
      for (; slist_it_has (lt); slist_it_next (lt))
	{
	  symbol_t * sym = slist_it_get (lt);
	  estr_append_cstr (buf, padding (level));
	  estr_append (buf, label_id (symbol_label (sym)));
	  estr_append_cstr (buf, ":\n");
	}
      delete_slist_it (lt);

      slist_it_next (it);

      // dump the statement itself.  Don't dump it, if there are no
      // more statements in container (i.e. `stmt' is last statement)
      // and `stmt' is dummy statement.
      if (slist_it_has (it)
	  || ((statement_t *)stmt)->base.kind != sk_dummy)
	private_stmt_dump (stmt, buf, level);
      else
	break;
    }
  delete_slist_it (it);

  delete_estring (buf0);
  */
}

static void
private_dump_assign (statement_t const * self, estring_t * buf, int level)
{
  slist_it_t * it = slist_iter (self->assign.lhss);
  estring_t * buf0 = new_estring ();
  estr_append_cstr (buf, padding (level));
  for (; slist_it_has (it); slist_it_next (it))
    {
      expression_t * expr = slist_it_get (it);
      estr_append (buf, expr_to_str (expr, buf0));
      estr_append_cstr (buf, " := ");
    }
  estr_append (buf, expr_to_str (self->assign.rhs, buf0));
  estr_append_cstr (buf, ";\n");
  delete_estring (buf0);
  delete_slist_it (it);
}

static void
private_stmt_dump (statement_t const * self, estring_t * buf, int level)
{
  switch (self->base.kind)
    {
    case sk_dummy:
      return;

    case sk_assign:
      private_dump_assign (self, buf, level);
      return;

    case sk_call:
      {
	estring_t * buf0 = expr_to_str (self->call.call, NULL);
	estr_append_cstr (buf, padding (level));
	estr_append (buf, buf0);
	estr_append_cstr (buf, ";\n");
	delete_estring (buf0);
	return;
      }

    case sk_cond:
      {
	estr_append_cstr (buf, padding (level));
	estr_append_cstr (buf, "'if' ");
	estring_t * buf0 = expr_to_str (self->cond.cond, NULL);
	estr_append (buf, buf0);
	delete_estring (buf0);
	estr_append_cstr (buf, " 'then'\n");
	private_stmt_dump (self->cond.ifclause, buf, level + 1);
	if (self->cond.elseclause)
	  {
	    estr_append_cstr (buf, padding (level));
	    estr_append_cstr (buf, "'else'\n");
	    private_stmt_dump (self->cond.elseclause, buf, level + 1);
	  }
	return;
      }

    case sk_for:
      {
	estr_append_cstr (buf, padding (level));
	estr_append_cstr (buf, "'for' ");
	estring_t * buf0 = expr_to_str (self->afor.variable, NULL);
	estr_append (buf, buf0);
	estr_append_cstr (buf, " := ");

	slist_it_t * it = slist_iter (self->afor.elmts);
	int first = 1;
	for (; slist_it_has (it); slist_it_next (it))
	  {
	    for_elmt_t * elmt = slist_it_get (it);
	    if (first)
	      first = 0;
	    else
	      estr_append_cstr (buf, ", ");
	    for_elmt_to_str (elmt, buf0);
	    estr_append (buf, buf0);
	  }
	delete_slist_it (it);
	delete_estring (buf0);

	estr_append_cstr (buf, " 'do'\n");
	private_stmt_dump (self->afor.body, buf, level + 1);
	return;
      }

    case sk_goto:
      {
	estr_append_cstr (buf, padding (level));
	estr_append_cstr (buf, "'goto' ");
	estring_t * buf0 = desig_expr_to_str (self->agoto.target, NULL);
	estr_append (buf, buf0);
	delete_estring (buf0);
	return;
      }

    case sk_block:
      estr_append_cstr (buf, padding (level));
      estr_append_cstr (buf, "'begin'\n");
      private_dump_container (self, buf, level+1);
      estr_append_cstr (buf, padding (level));
      estr_append_cstr (buf, "'end';\n");
      return;

    case sk_toplev:
      private_dump_container (self, buf, level);
      return;
    };

  assert (!"Should never get there!");
}

estring_t *
stmt_to_str (statement_t const * self, estring_t * buf)
{
  assert (self != NULL);

  if (buf == NULL)
    buf = new_estring ();
  else
    estr_clear (buf);

  private_stmt_dump (self, buf, 0);
  return buf;
}

static void
private_resolve_symbols_assign (statement_t * self, logger_t * log)
{
  expr_resolve_symbols (self->assign.rhs, self->parent, log);
  slist_it_t * it = slist_iter (self->assign.lhss);
  for (; slist_it_has (it); slist_it_next (it))
    expr_resolve_symbols (slist_it_get (it), self->parent, log);

  slist_it_reset (it, self->assign.lhss);

  type_t * tt = expr_type (self->assign.rhs);
  for (; slist_it_has (it); slist_it_next (it))
    {
      expression_t * lhs = slist_it_get (it);
      type_t * t1 = expr_type (lhs);

      // If one of the operands is `unknown', then the appropriate
      // reporting action has already been taken.  If one of the
      // operands is `implicit' then the symbol is yet to be resolved.
      if (!type_is_unknown (tt) && !type_is_unknown (t1)
	  && !type_is_implicit (tt) && !type_is_implicit (t1)
	  && !types_match (tt, t1))
	{
	  estring_t * s1 = expr_to_str (lhs, NULL);
	  estring_t * s2 = expr_to_str (self->assign.rhs, NULL);
	  estring_t * s3 = type_to_str (t1, NULL);
	  estring_t * s4 = type_to_str (tt, NULL);
	  log_printfc (log, ll_error, self->cursor,
		       "type mismatch in assignment %s := %s (%s doesn't match %s)",
		       estr_cstr (s1), estr_cstr (s2), estr_cstr (s3), estr_cstr (s4));
	  delete_estring (s1);
	  delete_estring (s2);
	  delete_estring (s3);
	  delete_estring (s4);
	}

      // Shouldn't be necessary to check this, since we now have
      // strongly typed slist that only holds lvalue expressions, and
      // this error is reported during syntax analysis.  But better
      // safe than sorry.
      if (!expr_is_lvalue (lhs))
	{
	  estring_t * s1 = expr_to_str (lhs, NULL);
	  log_printfc (log, ll_error, self->cursor,
		      "%s has to be an lvalue",
		      estr_cstr (s1));
	  delete_estring (s1);
	}
    }
  delete_slist_it (it);
}

static void
private_resolve_symbols_block (statement_t * self, logger_t * log)
{
  assert (a60_as_container (self));
  a60_symtab_resolve_symbols (self->block.symtab, a60_as_container (self), log);
  slist_it_t * it = slist_iter (self->block.statements);
  for (; slist_it_has (it); slist_it_next (it))
    stmt_resolve_symbols (slist_it_get (it), log);
  delete_slist_it (it);
}

void
stmt_resolve_symbols (statement_t * self, logger_t * log)
{
  assert (self != NULL);
  assert (log != NULL);

  switch (self->base.kind)
    {
    case sk_dummy: return;

    case sk_assign:
      private_resolve_symbols_assign (self, log);
      return;

    case sk_call:
      expr_resolve_symbols (self->call.call, self->parent, log);
      return;

    case sk_cond:
      {
	expr_resolve_symbols (self->cond.cond, self->parent, log);

	stmt_set_parent (self->cond.ifclause, self->parent);
	stmt_resolve_symbols (self->cond.ifclause, log);
	if (self->cond.elseclause)
	  {
	    stmt_set_parent (self->cond.elseclause, self->parent);
	    stmt_resolve_symbols (self->cond.elseclause, log);
	  }

	type_t * t1 = expr_type (self->cond.cond);
	if (!type_is_unknown (t1) && !type_is_implicit (t1)
	    && !types_match (t1, type_bool ()))
	  {
	    estring_t * s1 = expr_to_str (self->cond.cond, NULL);
	    estring_t * s2 = type_to_str (t1, NULL);
	    log_printfc (log, ll_error, self->cursor,
			 "type mismatch in conditional `%s' (%s should be Boolean)",
			 estr_cstr (s1), estr_cstr (s2));
	    delete_estring (s2);
	    delete_estring (s1);
	  }
	return;
      }

    case sk_for:
      {
	expr_resolve_symbols (self->afor.variable, self->parent, log);
	slist_it_t * it = slist_iter (self->afor.elmts);
	for (; slist_it_has (it); slist_it_next (it))
	  for_elmt_resolve_symbols (slist_it_get (it), self->afor.variable,
				    self->parent, log);
	delete_slist_it (it);
	stmt_set_parent (self->afor.body, self->parent);
	stmt_resolve_symbols (self->afor.body, log);
	return;
      }

    case sk_goto:
      desig_expr_resolve_symbols (self->agoto.target, self->parent, log);
      return;

    case sk_block:
    case sk_toplev:
      private_resolve_symbols_block (self, log);
      return;
    };

  assert (!"Should never get there!");
}

cursor_t *
stmt_cursor (statement_t const * self)
{
  assert (self != NULL);
  return self->cursor;
}

container_t *
stmt_parent (statement_t const * self)
{
  assert (self != NULL);
  return self->parent;
}

container_t *
container_parent (container_t const * _self)
{
  assert (_self != NULL);
  A60_USER_TO_REP (statement, self, const *);
  assert (self->base.kind == sk_block || self->base.kind == sk_toplev);
  return self->parent;
}

void
stmt_set_parent (statement_t * self, container_t * parent)
{
  assert (self != NULL);
  if (self->base.kind == sk_block || self->base.kind == sk_toplev)
    container_set_parent ((container_t *)self, parent);
  else
    self->parent = parent;
}

void
container_set_parent (container_t * _self, container_t * parent)
{
  assert (_self != NULL);
  A60_USER_TO_REP (statement, self, *);
  assert (self->base.kind == sk_block || self->base.kind == sk_toplev);

  self->parent = parent;
  a60_symtab_set_parent (self->block.symtab, ((statement_t*)parent)->block.symtab);
}

void
container_add_stmt (container_t * _self, statement_t * stmt)
{
  assert (_self != NULL);
  assert (stmt != NULL);
  A60_USER_TO_REP (statement, self, *);
  assert (self->base.kind == sk_block || self->base.kind == sk_toplev);

  slist_pushback (self->block.statements, stmt);
  stmt_set_parent (stmt, _self);
}

slist_t *
stmt_assign_lhss (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_assign);
  return self->assign.lhss;
}

expression_t *
stmt_assign_rhs (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_assign);
  return self->assign.rhs;
}

expression_t *
stmt_call_call (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_call);
  return self->call.call;
}

expression_t *
stmt_cond_cond (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_cond);
  return self->cond.cond;
}

statement_t *
stmt_cond_ifclause (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_cond);
  return self->cond.ifclause;
}

statement_t *
stmt_cond_elseclause (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_cond);
  return self->cond.elseclause;
}

expression_t *
stmt_for_variable (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_for);
  return self->afor.variable;
}

slist_t *
stmt_for_elmts (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_for);
  return self->afor.elmts;
}

statement_t *
stmt_for_body (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_for);
  return self->afor.body;
}

desig_expr_t *
stmt_goto_target (statement_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == sk_goto);
  return self->agoto.target;
}

void
stmt_add_label (statement_t * self, symbol_t * label)
{
  assert (self != NULL);
  assert (label != NULL);
  slist_pushback (self->labels, label);
}

slist_t *
stmt_labels (statement_t const * self)
{
  assert (self != NULL);
  return self->labels;
}

a60_symtab_t *
container_symtab (container_t const * _self)
{
  assert (_self != NULL);
  A60_USER_TO_REP (statement, self, *);
  assert (self->base.kind == sk_block || self->base.kind == sk_toplev);
  return self->block.symtab;
}

slist_t *
container_stmts (container_t const * _self)
{
  assert (_self != NULL);
  A60_USER_TO_REP (statement, self, *);
  assert (self->base.kind == sk_block || self->base.kind == sk_toplev);
  return self->block.statements;
}

visitor_t *
new_visitor_stmt (
    callback_t stmt_dummy,
    callback_t stmt_assign,
    callback_t stmt_call,
    callback_t stmt_cond,
    callback_t stmt_for,
    callback_t stmt_goto,
    callback_t stmt_block,
    callback_t stmt_toplev
)
{
  return a60_build_generic_visitor (
      A60_IFDEBUG (&private_statement_signature, NULL), 8,
      stmt_dummy,
      stmt_assign,
      stmt_call,
      stmt_cond,
      stmt_for,
      stmt_goto,
      stmt_block,
      stmt_toplev
  );
}

callback_t
a60_stmt_callback (void *(*cb)(statement_t *, void *))
{
  return (callback_t)cb;
}

callback_t
a60_cont_callback (void *(*cb)(container_t *, void *))
{
  return (callback_t)cb;
}
