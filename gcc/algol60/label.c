#include <assert.h>
#include <stdlib.h>
#include "meta.h"

#include "label.h"
#include "estring.h"
#include "boundspair.h"
#include "slist.h"
#include "visitor-impl.h"

static char const * private_label_signature = "label";

struct struct_label_t
{
  visitable_t base;

  estring_t const * id;
  slist_t * arr_bd_list;
  cursor_t * cursor;
};

label_t *
new_label (estring_t const * id)
{
  assert (id != NULL);

  label_t * ret = malloc (sizeof (label_t));
#ifndef NDEBUG
  ret->base.signature = private_label_signature;
#endif
  ret->id = id;
  ret->arr_bd_list = NULL;
  ret->cursor = NULL;
  return ret;
}

void
delete_label (label_t * self)
{
  if (self != NULL)
    {
      delete_slist (self->arr_bd_list);
      free (self);
    }
}

label_t *
a60_as_label (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_label_signature);
#endif
  return (label_t *)obj;
}

estring_t const *
label_id (label_t const * self)
{
  assert (self != NULL);
  return self->id;
}

estring_t *
label_to_str (label_t const * self, estring_t * buf)
{
  assert (self != NULL);
  if (buf == NULL)
    buf = new_estring ();
  estr_assign (buf, self->id);
  return buf;
}

int
label_eq (label_t const * lhs, label_t const * rhs)
{
  assert (lhs != NULL);
  assert (rhs != NULL);
  return !estr_compare (lhs->id, rhs->id);
}

slist_t *
label_add_boundspairs (label_t * self)
{
  assert (self != NULL);
  assert (self->arr_bd_list == NULL);
  self->arr_bd_list = new_slist_typed (adapt_test, boundspair);
  return self->arr_bd_list;
}

slist_t *
label_add_boundspairs_with (label_t * self, slist_t * bps)
{
  assert (self != NULL);
  assert (bps != NULL);
  slist_t * list = label_add_boundspairs (self);
  slist_it_t * it = slist_iter (bps);
  for (; slist_it_has (it); slist_it_next (it))
    slist_pushback (list, slist_it_get (it));
  delete_slist_it (it);
  return list;
}

void
label_remove_boundspairs (label_t * self)
{
  assert (self != NULL);
  assert (self->arr_bd_list != NULL);
  delete_slist (self->arr_bd_list);
  self->arr_bd_list = NULL;
}

slist_t *
label_boundspairs (label_t const * self)
{
  assert (self != NULL);
  return self->arr_bd_list;
}

void
label_set_cursor (label_t * self, cursor_t * cursor)
{
  assert (self != NULL);
  assert (self->cursor == NULL);
  self->cursor = cursor;
}

cursor_t *
label_cursor (label_t const * self)
{
  assert (self != NULL);
  return self->cursor;
}
