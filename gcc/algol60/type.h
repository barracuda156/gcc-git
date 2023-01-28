#ifndef _AL60L_TYPE_H_
#define _AL60L_TYPE_H_

///
/// \file type.h
///
/// This module contains methods for Algol type handling.
///

#include "type.i"
#include "estring.i"
#include "expression.i"
#include "statement.i"
#include "logger.i"
#include "slist.i"
#include "boundspair.i"
#include "symbol.i"
#include "visitor.i"
#include "pd.h"

type_t * type_unknown (void); ///< Memoized type.
type_t * type_any (void); ///< Memoized type.
type_t * type_int (void); ///< Memoized type.
type_t * type_void (void); ///< Memoized type.
type_t * type_real (void); ///< Memoized type.
type_t * type_string (void); ///< Memoized type.
type_t * type_bool (void); ///< Memoized type.
type_t * type_label (void); ///< Memoized type.
type_t * type_switch_any (void); ///< Memoized type for matching any switch.
type_t * type_array_any (void); ///< Memoized type.
type_t * type_array_real (void); ///< Memoized type.
type_t * type_proc_void_int (void); ///< Memoized type.
type_t * type_proc_void_real (void); ///< Memoized type.
type_t * type_proc_void_bool (void); ///< Memoized type.
type_t * type_proc_void_string (void); ///< Memoized type.
type_t * type_proc_void_any (void); ///< Memoized type.
type_t * type_proc_int_int (void); ///< Memoized type.
type_t * type_proc_real_int (void); ///< Memoized type.
type_t * type_proc_int_real (void); ///< Memoized type.
type_t * type_proc_real_real (void); ///< Memoized type.
type_t * type_proc_int_string (void); ///< Memoized type.
type_t * type_proc_int_int_int (void); ///< Memoized type.
type_t * type_proc_real_int_real (void); ///< Memoized type.
type_t * type_proc_real_real_int (void); ///< Memoized type.
type_t * type_proc_real_real_real (void); ///< Memoized type.
type_t * type_implicit (void); ///< Memoized type.

/// Create `unknown` type.
type_t  * new_t_unknown (void)
  ATTRIBUTE_MALLOC;

/// Create `any` type.
type_t  * new_t_any (void)
  ATTRIBUTE_MALLOC;

/// Create `int` type.
type_t  * new_t_int (void)
  ATTRIBUTE_MALLOC;

/// Create `void` type.
type_t  * new_t_void (void)
  ATTRIBUTE_MALLOC;

/// Create `real` type.
type_t  * new_t_real (void)
  ATTRIBUTE_MALLOC;

/// Create `string` type.
type_t  * new_t_string (void)
  ATTRIBUTE_MALLOC;

/// Create `bool` type.
type_t  * new_t_bool (void)
  ATTRIBUTE_MALLOC;

/// Create `label` type.
type_t  * new_t_label (void)
  ATTRIBUTE_MALLOC;

/// Create `implicit` type.
type_t  * new_t_implicit (void)
  ATTRIBUTE_MALLOC;

/// Create `switch` type.  The `switchlist` is a list of designational
/// expressions of this given switch type.  It's not taken into
/// account for types_same and types_match relations, it is merely
/// additional information.  `design_expr` type will be imposed on a
/// `switchlist` slist upon this call.  It will NOT be cloned for
/// purposes of storage inside t_switch privates.
type_t  * new_t_switch (slist_t * switchlist)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1);

/// Create `array of host` type.
type_t  * new_t_array (type_t * host)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1);

/// Create `own host` type.
type_t * new_t_own (type_t * host)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1);

/// Create `proc` type, with return type `rettype`, argument types
/// `argtypes`.  `argtypes` will NOT be cloned for purposes of storage
/// inside t_proc privates.
type_t  * new_t_proc (type_t * rettype, slist_t * argtypes)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2);

/// Create `proc` type, with return type `rettype`, and with no
/// constraints on argument types.  In ideal world, there would be
/// type matching finite automaton, and this kind of things would be
/// expressible with some repetition pattern (proc(int->any*)), but
/// this hack will cover most uses just fine.
type_t  * new_t_proc_stub (type_t * rettype)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1);

/// Convert void* to type, if it is type, or abort.
type_t * a60_as_type (void * ptr)
  ATTRIBUTE_NONNULL(1);

/// Set array bounds.  Depending on internal implementation this may
/// or may not return the same type you passed in, so always use the
/// functional idiom:
///  `t = t_array_set_bounds (t, boundspair);`
/// boundspair must not be NULL.
type_t * t_array_set_bounds (type_t * self, boundspair_t * boundspair)
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2);

/// Answer the bounds pair associated with this type, if any.
boundspair_t * t_array_bounds (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Answer the return type of given procedure type.
type_t * t_proc_return_type (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Answer the vector of argument types of given procedure type.
slist_t * t_proc_arg_types (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Answer the switchlist of switch type.
slist_t * t_switch_switchlist (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Return string representation of the type (for debugging and
/// dumping purposes).  Buffer `buf' may be NULL, in which case new
/// buffer will be allocated, or it can be preallocated buffer, in
/// which case it will be overwritten.  Return value is passed-in
/// buffer, or newly allocated buffer it that was NULL.  NULL is
/// returned if something goes wrong (usually allocation).
estring_t * type_to_str (type_t const * self, estring_t * buf)
  ATTRIBUTE_NONNULL (1);

/// Like type_str, but for canonical dumps.  It collapses series of
/// arrays into one array: 'int array' instead of 'int array array
/// array' for three dimensional array.
estring_t * type_to_str_canon (type_t const * self, estring_t * buf)
  ATTRIBUTE_NONNULL (1);

/// Answers 1 or 0, depending on whether the two types are same.
/// Array bounds are not taken into account.
/// Switchlist of switch type is not taken into account.
int types_same (type_t const * lhs, type_t const * rhs)
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2);

/// Answers 1 or 0, depending on whether one type matches the other.
/// This is the same behavior as types_same exhibits, with following
/// exception.  t_any matches any other type, including t_any.  And
/// t_unknown doesn't match any other type, with the exception of
/// t_any, but including t_unknown.  Thus:
///
///  any     ~~ any
///  any     ~~ unknown
///  unknown !~ unknown
///
/// Other rules include `own' handling:
///
///  'own' X ~~ X;   X ~~ 'own' X
///
/// The relation 'matches' is commutative.  It's not transitive:
/// unknown~~any, any~~any, any~~unknown, but unknown!~unknown.
///
/// `implicit` matches like `unknown`.
int types_match (type_t const * lhs, type_t const * rhs)
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2);

/// Answer root type.  Root type is the type at the very end of
/// hosting chain.  E.g. 'own array of array of int' has root type
/// 'int'.  This may also be a procedure or structured type.  This
/// procedure effectively strips any 'own' and 'array' garbage around
/// the type.
type_t * type_get_root (type_t * self)
  ATTRIBUTE_NONNULL (1);

/// Get primitive expression for given type, if there is one.  Not all
/// types allow this, e.g. it's not possible (ATM) to answer primitive
/// for procedure type.
expression_t * expr_primitive_for_type (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Check if given type is metatype (unknown, any, or maybe others in
/// future), or basic type.  Note that array, own, etc. are not
/// metatypes, but merely parametrized basic types.  Array of any is
/// however again metatype.
int is_metatype (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Check if it's `own` type.
int type_is_own (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Check if it's `array` type.
int type_is_array (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Check if given type is `unknown'.
int type_is_unknown (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Check if given type is `implicit'.
int type_is_implicit (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Check if given type is `proc'.
int type_is_proc (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Get hosting type for array or own.  NULL for others.
type_t * type_host (type_t const * self)
  ATTRIBUTE_NONNULL (1);

/// Resolve symbol references in type descriptions.  This is
/// particularly useful for array bounds resolution, but might happen
/// to have more uses.
void type_resolve_symbols (type_t * self, container_t * context, logger_t * log)
  ATTRIBUTE_NONNULL (1);

/// Construct Type visitor.  Arguments are the functions that should
/// be called when the dispatched object's kind matches its respective
/// argument.
visitor_t * new_visitor_type (
    callback_t type_unknown,
    callback_t type_implicit,
    callback_t type_any,
    callback_t type_own,
    callback_t type_void,
    callback_t type_int,
    callback_t type_real,
    callback_t type_string,
    callback_t type_bool,
    callback_t type_label,
    callback_t type_switch,
    callback_t type_array,
    callback_t type_proc
)
  ATTRIBUTE_MALLOC
  ATTRIBUTE_NONNULL (1)
  ATTRIBUTE_NONNULL (2)
  ATTRIBUTE_NONNULL (3)
  ATTRIBUTE_NONNULL (4)
  ATTRIBUTE_NONNULL (5)
  ATTRIBUTE_NONNULL (6)
  ATTRIBUTE_NONNULL (7)
  ATTRIBUTE_NONNULL (8)
  ATTRIBUTE_NONNULL (9)
  ATTRIBUTE_NONNULL (10)
  ATTRIBUTE_NONNULL (11)
  ATTRIBUTE_NONNULL (12)
;

/// For conversion of function prototype to callback.
callback_t a60_type_callback (void *(*cb)(type_t *, void *))
  ATTRIBUTE_NONNULL (1);


#endif//_AL60L_TYPE_H_
