#include "for-elmt.h"
#include "expression.h"
#include "estring.h"
#include "type.h"
#include "logger.h"
#include "meta.h"
#include "visitor-impl.h"

#include <stdlib.h>
#include <assert.h>

static char const * private_for_elmt_signature = "for_elmt";

typedef enum enum_for_elmt_kind_t
{
  fek_expr,
  fek_until,
  fek_while
}
for_elmt_kind_t;

typedef struct struct_for_elmt_expr_t
{
  expression_t * expr;
}
for_elmt_expr_t;

typedef struct struct_for_elmt_until_t
{
  expression_t * start;
  expression_t * step;
  expression_t * stop;
}
for_elmt_until_t;

typedef struct struct_for_elmt_while_t
{
  expression_t * expr;
  expression_t * cond;
}
for_elmt_while_t;

struct struct_for_elmt_t
{
  visitable_t base;

  cursor_t * cursor;
  union {
    for_elmt_expr_t feexpr;
    for_elmt_while_t fewhile;
    for_elmt_until_t feuntil;
  };
};


static for_elmt_t *
private_new_for_elmt (for_elmt_kind_t kind, cursor_t * cursor)
{
  for_elmt_t * ret = malloc (sizeof (for_elmt_t));
#ifndef NDEBUG
  ret->base.signature = private_for_elmt_signature;
#endif
  ret->base.kind = kind;
  ret->cursor = cursor;
  return ret;
}

for_elmt_t *
new_for_elmt_expr (cursor_t * cursor, expression_t * expr)
{
  assert (expr != NULL);

  for_elmt_t * ret = private_new_for_elmt (fek_expr, cursor);
  ret->feexpr.expr = expr;
  return ret;
}

for_elmt_t *
new_for_elmt_until (cursor_t * cursor, expression_t * start, expression_t * step, expression_t * stop)
{
  assert (start != NULL);
  assert (step != NULL);
  assert (stop != NULL);

  for_elmt_t * ret = private_new_for_elmt (fek_until, cursor);
  ret->feuntil.start = start;
  ret->feuntil.step = step;
  ret->feuntil.stop = stop;
  return ret;
}

for_elmt_t *
new_for_elmt_while (cursor_t * cursor, expression_t * expr, expression_t * cond)
{
  assert (expr != NULL);
  assert (cond != NULL);

  for_elmt_t * ret = private_new_for_elmt (fek_while, cursor);
  ret->fewhile.expr = expr;
  ret->fewhile.cond = cond;
  return ret;
}

for_elmt_t *
clone_for_elmt (for_elmt_t const * self)
{
  assert (self != NULL);

  for_elmt_t * ret = private_new_for_elmt (self->base.kind, self->cursor);

  switch (self->base.kind)
    {
    case fek_expr:
      ret->feexpr.expr = clone_expression (self->feexpr.expr);
      break;

    case fek_until:
      ret->feuntil.start = clone_expression (self->feuntil.start);
      ret->feuntil.step = clone_expression (self->feuntil.step);
      ret->feuntil.stop = clone_expression (self->feuntil.stop);
      break;

    case fek_while:
      ret->fewhile.expr = clone_expression (self->fewhile.expr);
      ret->fewhile.cond = clone_expression (self->fewhile.cond);
      break;
    };

  return ret;
}

for_elmt_t *
a60_as_for_elmt (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_for_elmt_signature);
#endif
  return (for_elmt_t *)obj;
}

estring_t *
for_elmt_to_str (for_elmt_t const * self, estring_t * buf)
{
  assert (self != NULL);

  switch (self->base.kind)
    {
    case fek_expr:
      return expr_to_str (self->feexpr.expr, buf);

    case fek_until:
      {
	buf = expr_to_str (self->feuntil.start, buf);

	estring_t * str = expr_to_str (self->feuntil.step, NULL);
	estr_append_cstr (buf, " 'step' ");
	estr_append (buf, str);

	expr_to_str (self->feuntil.stop, str);
	estr_append_cstr (buf, " 'until' ");
	estr_append (buf, str);
	delete_estring (str);

	return buf;
      }

    case fek_while:
      {
	buf = expr_to_str (self->fewhile.expr, buf);

	estring_t * str = expr_to_str (self->fewhile.cond, NULL);
	estr_append_cstr (buf, " 'while' ");
	estr_append (buf, str);
	delete_estring (str);

	return buf;
      }
    };

  assert (!"Should never get there!");
  return NULL;
}

static void
private_check_elmt_type (char const * what, expression_t * expr, type_t * type, logger_t * log)
{
  type_t * t = expr_type (expr);
  if (!type_is_unknown (t) && !types_match (t, type))
    {
      estring_t * expr_s = expr_to_str (expr, NULL);
      estring_t * t_s = type_to_str (t, NULL);
      estring_t * type_s = type_to_str (type, NULL);
      log_printfc (log, ll_error, expr_cursor (expr),
		   "type mismatch in for statement %s `%s' (%s doesn't match %s)",
		   what, estr_cstr (expr_s), estr_cstr (t_s), estr_cstr (type_s));
      delete_estring (expr_s);
      delete_estring (t_s);
      delete_estring (type_s);
    }
}

void
for_elmt_resolve_symbols (for_elmt_t * self, expression_t * variable, container_t * context, logger_t * log)
{
  assert (self != NULL);
  assert (log != NULL);

  switch (self->base.kind)
    {
    case fek_expr:
      expr_resolve_symbols (self->feexpr.expr, context, log);
      private_check_elmt_type ("expression", self->feexpr.expr, expr_type (variable), log);
      break;

    case fek_until:
      expr_resolve_symbols (self->feuntil.start, context, log);
      expr_resolve_symbols (self->feuntil.step, context, log);
      expr_resolve_symbols (self->feuntil.stop, context, log);
      private_check_elmt_type ("start expression", self->feuntil.start, expr_type (variable), log);
      private_check_elmt_type ("step expression", self->feuntil.step, expr_type (variable), log);
      private_check_elmt_type ("stop expression", self->feuntil.stop, expr_type (variable), log);
      break;

    case fek_while:
      expr_resolve_symbols (self->fewhile.expr, context, log);
      expr_resolve_symbols (self->fewhile.cond, context, log);
      private_check_elmt_type ("while expression", self->fewhile.expr, expr_type (variable), log);
      private_check_elmt_type ("while condition", self->fewhile.cond, type_bool (), log);
      break;
    };
}

expression_t *
for_elmt_expr_expr (for_elmt_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == fek_expr);
  return self->feexpr.expr;
}

expression_t *
for_elmt_until_start (for_elmt_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == fek_until);
  return self->feuntil.start;
}

expression_t *
for_elmt_until_step (for_elmt_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == fek_until);
  return self->feuntil.step;
}

expression_t *
for_elmt_until_stop (for_elmt_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == fek_until);
  return self->feuntil.stop;
}

expression_t *
for_elmt_while_expr (for_elmt_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == fek_while);
  return self->fewhile.expr;
}

expression_t *
for_elmt_while_cond (for_elmt_t const * self)
{
  assert (self != NULL);
  assert (self->base.kind == fek_while);
  return self->fewhile.cond;
}

visitor_t *
new_visitor_for_elmt (
    callback_t for_elmt_expr,
    callback_t for_elmt_until,
    callback_t for_elmt_while
)
{
  return a60_build_generic_visitor (
      A60_IFDEBUG (&private_for_elmt_signature, NULL), 3,
      for_elmt_expr,
      for_elmt_until,
      for_elmt_while
  );
}

callback_t
a60_for_elmt_callback (void *(*cb)(for_elmt_t *, void *))
{
  return (callback_t)cb;
}
