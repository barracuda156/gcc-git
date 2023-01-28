#ifndef _AL60L_SYMTAB_H_
#define _AL60L_SYMTAB_H_

///
/// \file a60_symtab.h
///
/// Symtab represents a collection of symbols.  Symtabs are used by
/// containers and function symbols to store symbols declared in their
/// bodies.
///

#include "a60_symtab.i"
#include "symbol.i"
#include "label.i"
#include "type.i"
#include "statement.i"
#include "estring.i"
#include "logger.i"
#include "cursor.i"
#include "pd.h"
#include "visitor.i"
#include "slist.i"

// Labels shall be added to most enclosing *block*.  Block is a symtab
// that either holds some symbols (i.e. there already variables), or
// is explicitly marked as block symtab (such as symtab at function
// level toplevel block).
typedef enum enum_a60_symtab_kind_t
{
  a60_stk_ordinary,
  a60_stk_block,
}
a60_symtab_kind_t;

/// Create new symtab with optional parental symtab `parent'.
a60_symtab_t * a60_new_symtab (a60_symtab_kind_t kind)
  ATTRIBUTE_MALLOC;

/// Create a copy of this symtab.  Each symbol is cloned, too.
a60_symtab_t * a60_clone_symtab (a60_symtab_t * self)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_MALLOC;

/// Answer the kind of this symtab.
a60_symtab_kind_t a60_symtab_kind (a60_symtab_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Set the parental symtab.
void a60_symtab_set_parent (a60_symtab_t * self, a60_symtab_t * parent)
  ATTRIBUTE_NONNULL(1);

/// Answer the parent of this symtab.
a60_symtab_t * a60_symtab_parent (a60_symtab_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Delete this symtab.
void a60_delete_symtab (a60_symtab_t * self);

/// Convert void* to statement, if it is statement, or abort.
a60_symtab_t * a60_as_symtab (void * ptr)
  ATTRIBUTE_NONNULL(1);

typedef enum enum_a60_symtab_entry_kind_t
{
  sek_internal,
  sek_ordinary
}
a60_symtab_entry_kind_t;

/// Add given symbol to the container and return 0.  It there already
/// is other symbol with the same name, do nothing and return -1.
/// `internal' is sek_internal for internal symbols (internal symbols
/// may be overloaded) and sek_ordinary for ordinary symbols (which
/// may not).
int a60_symtab_add_symbol (a60_symtab_t * self, symbol_t * sym, a60_symtab_entry_kind_t internal)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Remove the symbol from symtab.  The symbol must be present in
/// symtab.
symbol_t * a60_symtab_erase_symbol (a60_symtab_t * self, symbol_t * sym)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Answer non-zero if the symtab is empty.  Answer zero if there is
/// at least one symbol in table.
int a60_symtab_empty (a60_symtab_t const * self)
  ATTRIBUTE_NONNULL(1);

/// Resolve all symbols in the symbol table.  This calls
/// symbol_resolve_symbols for each symbol in table.
void a60_symtab_resolve_symbols (a60_symtab_t * self, container_t * context, logger_t * log)
  ATTRIBUTE_NONNULL(1);

typedef symbol_t * (* a60_symtab_missing_handler_t) (a60_symtab_t * self, label_t const * lbl, type_t const * atype, logger_t * log, cursor_t * cursor, void * data);

/// Set a missing symbol handler for this symtab.  When a symbol is
/// looked up in this symtab, and it is not found, this handler is
/// invoked.  To protect the system from random overwrites of missing
/// symbol handlers, it is only possible to set a handler once.  The
/// handler has to be cleared up with a60_symtab_unset_missing_handler
/// before any subsequent set.
///
/// When the symbol lookup is invoked directly on this symtab, or
/// recursive lookup procedure performs lookup indirectly in this
/// symtab (i.e. one of the functions a60_symtab_find_name_rec,
/// a60_symtab_find_name, a60_symtab_find_name_rec_add_undefined is
/// invoked) and the symbol is not found, the handler is invoked as
/// follows:
///   void * ret = handler (symtab, lbl, atype, log, cursor, data);
/// With arguments being:
///   SYMTAB: a symbol table where the handler was set
///   LBL:    is a name of a symbol being looked up.  It is a constant
///           pointer and may not be tinkered with inside the handler.
///   ATYPE, LOG, CURSOR: passed from a lookup call.  Some may be NULL.
///   DATA: data pointer passed in this call.  E.g. associated container.
///
/// The return value of handler is interpreted as follows:
///   NULL:      continue looking for symbol in superblock if applicable,
///              report error if not found
///   (void*)-1: stop looking for symbol, don't report errors, don't
///              adjust symtabs
///   other:     use this pointer as a looked up symbol value
///
/// Implicit behavior (i.e. when there is no handler set) is the same
/// as if the handler was:
///   void * handler (void * self, void * data) { return NULL; }
void a60_symtab_set_missing_handler (a60_symtab_t * self, a60_symtab_missing_handler_t handler, void * data)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Unset missing symbol handler.
void a60_symtab_unset_missing_handler (a60_symtab_t * self)
  ATTRIBUTE_NONNULL(1);

/// Look up given symbol in the table.  Answer first symbol that
/// matches restriction `atype', or first symbol with matching name,
/// if its type is NULL (in that case type can't be checked).  Return
/// NULL if not found.
symbol_t * a60_symtab_find_name (a60_symtab_t * self, label_t const * lbl, type_t const * atype)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// Lookup given symbol in the table, and if it fails, recursively
/// in all parental tables. Answer first symbol that matches
/// restriction `atype', or first symbol with matching name, if its
/// type is NULL (in that case type can't be checked). Return NULL if
/// not found.
symbol_t * a60_symtab_find_name_rec (a60_symtab_t * self, label_t const * lbl, type_t const * atype)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

/// See if given symbol with requested matching type is already
/// defined somewhere in the scope.  If it's not, create new
/// definition in most enclosing scope.  Answer either found, or newly
/// created symbol.
///
/// Note that when missing handler (see a60_symtab_set_missing_handler)
/// answers (void*)-1, the symbol table will NOT be affected, and NULL
/// will be returned.
symbol_t * a60_symtab_find_name_rec_add_undefined (a60_symtab_t * self, label_t const * lbl, type_t * atype, logger_t * log, cursor_t * cursor)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2)
  ATTRIBUTE_NONNULL(3)
  ATTRIBUTE_NONNULL(4);

/// Populate given symtab with algol internal functions.
void a60_symtab_toplev_define_internals (a60_symtab_t * self)
  ATTRIBUTE_NONNULL(1);

/// Rewrite the table into string.  `buf' is a string into which the
/// dump should be done. If NULL, new string will be allocated.
/// Either passed-in, or newly allocated buffer is returned.
estring_t * a60_symtab_to_str (a60_symtab_t const * self, estring_t * buf)
  ATTRIBUTE_NONNULL(1);

/// Call a given callback at each symbol of symbol table.  Callback
/// will be passed visited symbol as a first argument, and `data' as a
/// second argument.  Return value of callback is ignored.
void a60_symtab_each (a60_symtab_t * self, callback_t callback, void * data)
  ATTRIBUTE_NONNULL(1)
  ATTRIBUTE_NONNULL(2);

#endif//_AL60L_SYMTAB_H_
