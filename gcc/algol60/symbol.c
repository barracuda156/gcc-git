#include <stdlib.h>
#include <assert.h>
#include "meta.h"

#include "symbol.h"
#include "a60_symtab.h"
#include "label.h"
#include "type.h"
#include "statement.h"
#include "estring.h"
#include "visitor.h"
#include "logger.h"
#include "visitor-impl.h"

static char const * private_symbol_signature = "symbol";

typedef struct struct_sym_var_t
{
}
sym_var_t;

typedef struct struct_sym_func_t
{
  /// List typed with a60_as_symbol, contains symbols that are
  /// unresolved in function body, and as such form implicit
  /// parameters of the function.
  slist_t * implied_params;
}
sym_func_t;

typedef struct struct_sym_formparm_t
{
  parmconv_t convention;
}
sym_formparm_t;

typedef enum enum_symbol_kind_t
{
  sk_var,
  sk_func,
  sk_formparm,
}
symbol_kind_t;

struct struct_symbol_t
{
  // @TODO: see if it makes sense to define ordinary symbols with proc
  // type...  Perhaps yes, in that case see if there are some parts
  // down there that could be moved to union...
  visitable_t base;
  label_t const * label;
  statement_t * stmt;
  type_t * type;
  int hidden;
  void * extra;

  union {
    sym_var_t sym_var;
    sym_func_t sym_func;
    sym_formparm_t sym_formparm;
  };
};

static symbol_t *
private_alloc_symbol (label_t const * label, symbol_kind_t kind)
{
  symbol_t * ret = malloc (sizeof (symbol_t));
#ifndef NDEBUG
  ret->base.signature = private_symbol_signature;
#endif
  ret->base.kind = kind;
  ret->label = label;
  ret->stmt = NULL;
  ret->type = NULL;
  ret->hidden = 0;
  ret->extra = NULL;
  return ret;
}

symbol_t *
new_symbol_var (label_t const * name)
{
  assert (name != NULL);

  symbol_t * ret = private_alloc_symbol (name, sk_var);
  return ret;
}

symbol_t *
new_symbol_func (label_t const * name)
{
  assert (name != NULL);

  symbol_t * ret = private_alloc_symbol (name, sk_func);
  ret->sym_func.implied_params = NULL;
  return ret;
}

symbol_t *
new_symbol_formparm (label_t const * name, type_t * type, parmconv_t convention)
{
  assert (name != NULL);
  assert (type != NULL);

  symbol_t * ret = private_alloc_symbol (name, sk_formparm);
  ret->type = type;
  ret->sym_formparm.convention = convention;
  return ret;
}

symbol_t *
clone_symbol (symbol_t const * self)
{
  return clone_symbol_with_name (self, self->label);
}

symbol_t *
clone_symbol_with_name (symbol_t const * self, label_t const * label)
{
  assert (self != NULL);
  assert (label != NULL);

  symbol_t * ret = private_alloc_symbol (label, self->base.kind);
  ret->stmt = self->stmt;
  ret->type = self->type;
  ret->hidden = self->hidden;
  ret->extra = self->extra;

  switch (self->base.kind)
    {
    case sk_var:
      break;

    case sk_func:
      break;
    }

  return ret;
}

void
delete_symbol (symbol_t * self)
{
  if (self != NULL)
    {
      free (self);

      switch (self->base.kind)
	{
	case sk_var:
	  break;

	case sk_func:
	  break;
	}
    }
}

symbol_t *
a60_as_symbol (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_symbol_signature);
#endif
  return (symbol_t *)obj;
}

label_t const *
symbol_label (symbol_t const * self)
{
  assert (self != NULL);
  return self->label;
}

estring_t *
symbol_to_str (symbol_t const * self, estring_t * buf)
{
  assert (self != NULL);
  return label_to_str (self->label, buf);
}

struct parse_context_s
{
  symbol_t * function;
  a60_symtab_t * func_symtab;
};

static symbol_t *
private_function_add_implicit (a60_symtab_t * symtab ATTRIBUTE_UNUSED,
			       label_t const * label,
			       type_t const * atype ATTRIBUTE_UNUSED,
			       logger_t * log,
			       cursor_t * cursor,
			       void * _pc)
{
  static estring_t * buf = NULL, * buf2 = NULL;
  struct parse_context_s * pc = _pc;

  log_printfc (log, ll_warning, cursor,
	      "function `%s' has an implicit parameter `%s'",
	      estr_cstr (buf = label_to_str (symbol_label (pc->function), buf)),
	      estr_cstr (buf2 = label_to_str (label, buf2)));

  symbol_t * symbol = new_symbol_var (label);
  a60_symtab_add_symbol (pc->func_symtab, symbol, sek_internal);

  // These symbols are handled special on instantiation.  No errors
  // are reported if one of expression operand is `implicit'.
  symbol_set_type (symbol, type_implicit ());
  symbol_set_extra (symbol, cursor);

  return symbol;
}

void
symbol_resolve_symbols (symbol_t * self, container_t * context, logger_t * log)
{
  switch (self->base.kind)
    {
    case sk_var:
      type_resolve_symbols (self->type, context, log);
      return;

    case sk_func:
      {
	statement_t * stmt = self->stmt;
	container_t * context = a60_as_container (stmt);
	a60_symtab_t * symtab = container_symtab (context);
	struct parse_context_s pc = {self, symtab};

	a60_symtab_set_missing_handler (symtab,
					private_function_add_implicit,
					&pc);
	stmt_resolve_symbols (stmt, log);
	a60_symtab_unset_missing_handler (symtab);
      }
      return;

    case sk_formparm:
      // Nothing to be done here.
      return;
    }
}

void
symbol_set_type (symbol_t * self, type_t * type)
{
  assert (self != NULL);
  assert (type != NULL);
  self->type = type;
}

type_t *
symbol_type (symbol_t const * self)
{
  assert (self != NULL);
  return self->type;
}

void
symbol_set_stmt (symbol_t * self, statement_t * stmt)
{
  assert (self != NULL);
  assert (stmt != NULL);
  self->stmt = stmt;
}

statement_t *
symbol_stmt (symbol_t const * self)
{
  assert (self != NULL);
  return self->stmt;
}

void
symbol_set_hidden (symbol_t * self, int hidden)
{
  assert (self != NULL);
  self->hidden = hidden;
}

int
symbol_hidden (symbol_t const * self)
{
  assert (self != NULL);
  return self->hidden;
}

void
symbol_set_extra (symbol_t * self, void * extra)
{
  assert (self != NULL);
  self->extra = extra;
}

void *
symbol_extra (symbol_t const * self)
{
  assert (self != NULL);
  return self->extra;
}

visitor_t *
new_visitor_symbol (
    callback_t symbol_var,
    callback_t symbol_fun,
    callback_t symbol_formparm
)
{
  return a60_build_generic_visitor (
      A60_IFDEBUG (&private_symbol_signature, NULL), 3,
      symbol_var,
      symbol_fun,
      symbol_formparm
  );
}

callback_t
a60_symbol_callback (void *(*cb)(symbol_t *, void *))
{
  return (callback_t)cb;
}
