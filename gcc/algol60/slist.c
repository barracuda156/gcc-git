/*
 * Copyright (c) 2006 Petr Machata
 * All rights reserved.
 */

#ifndef SELF_TEST

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "slist.h"
#include "meta.h"

static char const* private_slist_signature = "slist";

typedef struct struct_slist_node_t slist_node_t;

struct struct_slist_node_t
{
  // Link points to the next node in list.  When stored in pool, this
  // points to the next node in pool.
  slist_node_t * link;
  void * object;
};

struct struct_slist_t
{
  char const* signature;
  slist_node_t * head;
  slist_node_t * tail;
#ifndef NDEBUG
  void * (*test)(void * obj, void * user);
  void * test_userdata;
#endif
};

struct struct_slist_it_t
{
  // This is used as a pointer to the next iterator for iterator pool.
  // Normally it points to the node the iterator is "at".
  slist_node_t * pointee;
};

// Pools.  These are used for time-efficient memory management.
static slist_node_t * node_pool = NULL;
static slist_it_t * it_pool = NULL;


static slist_t *
private_alloc_slist (void)
{
  slist_t * ret = malloc (sizeof (slist_t));
  if (ret == NULL)
    return NULL;

  ret->signature = private_slist_signature;
  ret->head = NULL;
  ret->tail = NULL;
#ifndef NDEBUG
  ret->test = NULL;
  ret->test_userdata = NULL;
#endif

  return ret;
}

static slist_node_t *
private_alloc_node (void)
{
  if (node_pool == NULL)
    return malloc (sizeof (slist_node_t));
  else
    {
      slist_node_t * node = node_pool;
      node_pool = node_pool->link;
      return node;
    }
}

static slist_it_t *
private_alloc_it (void)
{
  if (it_pool == NULL)
    return malloc (sizeof (slist_it_t));
  else
    {
      slist_it_t * it = it_pool;
      it_pool = (void *)it_pool->pointee;
      return it;
    }
}

static void
private_dispose_node (slist_node_t * node)
{
  node->link = node_pool;
  node_pool = node;
}

static void
private_dispose_it (slist_it_t * it)
{
  it->pointee = (void*)it_pool;
  it_pool = it;
}


void *
adapt_test (void * obj, void * test)
{
  void * (*xtest)(void *) = test;
  return xtest (obj);
}

slist_t *
new_slist (void)
{
  return private_alloc_slist ();
}

slist_t *
new_slist_typed (void* (*test)(void * obj, void * user), void * userdata)
{
#ifndef NDEBUG
  assert (test != NULL);

  slist_t * ret = private_alloc_slist ();
  if (ret == NULL)
    return NULL;

  ret->test = test;
  ret->test_userdata = userdata;

  return ret;
#else
  return new_slist ();
#endif
}

static void
private_push_elements (slist_t * self, unsigned num, va_list * ap,
		       void (*slist_push) (slist_t *, void *))
{
  assert (num > 0);
  do
    (*slist_push) (self, va_arg(*ap, void *));
  while (--num);
}

#ifndef NDEBUG
static void
private_test_element (slist_t * list, void * object)
{
  if (list->test != NULL)
    assert (list->test (object, list->test_userdata));
}
#else
# define private_test_element(LIST,OBJECT)
#endif

slist_t *
new_slist_from (unsigned num, ...)
{
  slist_t * ret = new_slist ();
  if (num > 0)
    {
      va_list ap;
      va_start(ap, num);
      private_push_elements (ret, num, &ap, slist_pushback);
      va_end(ap);
    }
  return ret;
}

slist_t *
new_slist_typed_from (void* (*test)(void * obj, void * user),
		      void * userdata, unsigned num, ...)
{
  assert (test != NULL);
  slist_t * ret = new_slist_typed (test, userdata);
  if (num > 0)
    {
      va_list ap;
      va_start(ap, num);
      private_push_elements (ret, num, &ap, slist_pushback);
      va_end(ap);
    }
  return ret;
}

static void
private_copy_nodes (slist_t * dest, slist_t const * slist,
		    void (*slist_push) (slist_t *, void *))
{
  slist_node_t * node = slist->head;
  for (; node != NULL; node = node->link)
    (*slist_push) (dest, node->object);
}

slist_t *
clone_slist (slist_t const * slist)
{
  assert (slist != NULL);
  slist_t * ret = new_slist ();
  private_copy_nodes (ret, slist, slist_pushback);
  return ret;
}

slist_t *
clone_slist_typed (slist_t const * slist,
		   void* (*test)(void * obj, void * user),
		   void * userdata)
{
#ifndef NDEBUG
  assert (slist != NULL);
  slist_t * ret = new_slist_typed (test, userdata);
  private_copy_nodes (ret, slist, slist_pushback);
  return ret;
#else
  return clone_slist (slist);
#endif
}

void
slist_set_type (slist_t * self, void* (*test)(void * obj, void * user), void * userdata)
{
#ifndef NDEBUG
  assert (self != NULL);
  assert (test != NULL);

  self->test = test;
  self->test_userdata = userdata;

  slist_node_t * node = self->head;
  for (; node != NULL; node = node->link)
    private_test_element (self, node->object);
#endif
}

void
delete_slist (slist_t * list)
{
  if (list != NULL)
    {
      slist_node_t * node = list->head;

      for (; node != NULL; )
	{
	  slist_node_t * next = node->link;
	  private_dispose_node (node);
	  node = next;
	}

      free (list);
    }
}

slist_t *
a60_as_slist (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_slist_signature);
#endif
  return (slist_t *)obj;
}

void
slist_pushback (slist_t * list, void * object)
{
  assert (list != NULL);
  private_test_element (list, object);

  slist_node_t * node = private_alloc_node ();

  node->object = object;
  node->link = NULL;

  if (list->tail == NULL)
    {
      list->tail = node;
      assert (list->head == NULL);
      list->head = node;
    }
  else
    {
      list->tail->link = node;
      list->tail = node;
    }
}

void
slist_pushfront (slist_t * list, void * object)
{
  assert (list != NULL);
  private_test_element (list, object);

  slist_node_t * node = private_alloc_node ();
  node->object = object;

  if (list->tail == NULL)
    {
      list->tail = node;
      assert (list->head == NULL);
      list->head = node;
      node->link = NULL;
    }
  else
    {
      node->link = list->head;
      list->head = node;
    }
}

void *
slist_popfront (slist_t * list)
{
  assert (list != NULL);

  slist_node_t * node = list->head;
  void * object = node->object;

  list->head = node->link;
  if (list->head == NULL)
    list->tail = NULL;
  private_dispose_node (node);

  return object;
}

void *
slist_front (slist_t * list)
{
  assert (list != NULL);
  return list->head->object;
}

void *
slist_back (slist_t * list)
{
  assert (list != NULL);
  return list->tail->object;
}

int
slist_empty (slist_t * list)
{
  assert (list != NULL);
  assert ((list->head == NULL) == (list->tail == NULL));
  return (list->head == NULL);
}

int
slist_length (slist_t * list)
{
  assert (list != NULL);

  slist_node_t * node = list->head;
  int length = 0;
  for (; node != NULL; node = node->link)
    length++;

  return length;
}

void
slist_append (slist_t * list, slist_t * other)
{
  assert (list != NULL);
  assert (other != NULL);

  if (other->head == NULL)
    return;

  assert (list->head != other->head);

#ifndef NDEBUG
  if (list->test != NULL)
    {
      slist_node_t * node = list->head;
      for (; node != NULL; node = node->link)
	private_test_element (list, node->object);
    }
#endif

  if (list->head == NULL)
    {
      list->head = other->head;
      list->tail = other->tail;
    }
  else
    {
      list->tail->link = other->head;
      list->tail = other->tail;
    }

  other->head = NULL;
  other->tail = NULL;
}

void
slist_each (
  slist_t * list,
  void (*fn)(void *, void *),
  void * userdata)
{
  assert (list != NULL);
  assert (fn != NULL);

  slist_node_t * node = list->head;
  for (; node != NULL; node = node->link)
    {
      void * obj = node->object;
      (*fn) (obj, userdata);
    }
}

void
slist_map (
  slist_t * list,
  void* (*fn)(void * /*object*/, void * /*userdata*/),
  void * userdata)
{
  assert (list != NULL);
  assert (fn != NULL);

  slist_node_t * node = list->head;
  for (; node != NULL; node = node->link)
    node->object = (*fn) (node->object, userdata);
}

slist_t *
slist_filter (
   slist_t * list,
   int (*pred)(slist_t * /*list*/, void * /*label*/, void * /*userdata*/),
   void * userdata)
{
  assert (list != NULL);
  assert (pred != NULL);

  slist_t * ret = new_slist ();
  slist_node_t * node = list->head;
  for (; node != NULL; node = node->link)
    {
      if ((*pred) (list, node->object, userdata))
	slist_pushback (ret, node->object);
    }

  return ret;
}


static slist_it_t *
private_create_slist_iter (slist_node_t * pointee)
{
  slist_it_t * ret = private_alloc_it ();
  if (ret == NULL)
    return NULL;

  ret->pointee = pointee;

  return ret;
}

slist_it_t *
slist_iter (slist_t * list)
{
  assert (list != NULL);
  return private_create_slist_iter (list->head);
}

slist_it_t *
new_slist_it (void)
{
  return private_create_slist_iter (NULL);
}

slist_it_t *
clone_slist_it (slist_it_t const * it)
{
  assert (it != NULL);
  return private_create_slist_iter (it->pointee);
}

int
slist_it_has (slist_it_t * it)
{
  assert (it != NULL);
  return it->pointee != NULL;
}

void *
slist_it_get (slist_it_t * it)
{
  assert (it != NULL);
  return it->pointee->object;
}

void
slist_it_put (slist_it_t * it, void * object)
{
  assert (it != NULL);
  it->pointee->object = object;
}

void *
slist_it_get_next (slist_it_t * it)
{
  assert (it != NULL);
  void * ret = slist_it_get (it);
  slist_it_next (it);
  return ret;
}

void *
slist_it_erase_after (slist_it_t * it)
{
  assert (it != NULL);
  assert (slist_it_has (it));
  assert (it->pointee->link != NULL);

  slist_node_t * tmp = it->pointee->link;
  void * ret = tmp->object;
  it->pointee->link = it->pointee->link->link;
  private_dispose_node (tmp);
  return ret;
}

void
slist_it_next (slist_it_t * it)
{
  assert (it != NULL);
  it->pointee = it->pointee->link;
}

void
slist_it_reset (slist_it_t * it, slist_t * list)
{
  assert (it != NULL);
  assert (list != NULL);
  it->pointee = list->head;
}

void
slist_it_reset_it (slist_it_t * it, slist_it_t const * other)
{
  assert (it != NULL);
  if (other == NULL)
    it->pointee = NULL;
  else
    it->pointee = other->pointee;
}

void
delete_slist_it (slist_it_t * it)
{
  private_dispose_it (it);
}

#else /* SELF_TEST */

#include "slist.h"
#include <stdio.h>
#include <assert.h>

void
incctr (void * obj, void * userdata)
{
  (*(int*)userdata)++;
}

int
odd (void * obj, void * userdata)
{
  return (int)obj % 2;
}

void *
num_0_to_9 (void * obj)
{
  int number = (int)obj;
  if (number >=0 && number <= 9)
    return (void*)1;
  else
    return NULL;
}

int
main (void)
{
  slist_t * l1 = new_slist ();
  slist_t * l2 = new_slist ();
  slist_t * l3 = new_slist_typed (adapt_test, num_0_to_9);
  slist_t * l4 = new_slist_typed (adapt_test, num_0_to_9);
  slist_t * lists[] = {
    l1, l3, l2, l4, NULL
  };
  char const* names[] = {
    "ordinary list", "typed list", "other ordinary list", "other typed list", NULL
  };

  slist_t ** lit = lists;
  char const* * nit = names;

  for (; *lit != NULL; ++lit, ++nit)
    {
      slist_t * l = *lit;
      printf (" + %s\n", *nit);
      assert (a60_as_slist (l));
      assert (slist_empty (l));

      for (int i = 0; i < 10; ++i)
	{
	  slist_pushback (l, (void*)i);
	  assert (!slist_empty (l));
	  assert (slist_back (l) == (void*)i);
	}

      int ctr = 0;
      slist_each (l, incctr, &ctr);
      assert (ctr == 10);
      int sum = 0;
      for (slist_it_t * it = slist_iter (l); slist_it_has (it); slist_it_next (it))
	sum += (int)slist_it_get (it);
      assert (sum == 45);

      ctr = 0;
      slist_t * l2 = slist_filter (l, odd, NULL);
      assert (a60_as_slist (l2));
      slist_each (l, incctr, &ctr);
      slist_each (l2, incctr, &ctr);
      assert (ctr == 15);

      while (!slist_empty (l))
	{
	  void * fr = slist_front (l);
	  void * fr2 = slist_popfront (l);
	  assert (fr == fr2);
	  slist_pushfront (l2, l2);
	  --ctr;
	}
      assert (ctr == 5);

      ctr = 0;
      slist_each (l, incctr, &ctr);
      assert (ctr == 0);

      ctr = 0;
      slist_each (l2, incctr, &ctr);
      assert (ctr == 15);

      delete_slist (l);
      delete_slist (l2);
    }

  assert (*nit == NULL);

  printf (" + empty list\n");
  slist_it_t * it = slist_iter (new_slist ());
  for (; slist_it_has (it); slist_it_next (it))
    assert (!"empty list shouldn't execute this");
  delete_slist_it (it);

  printf ("All passed.\n");
  return 0;
}

#endif
