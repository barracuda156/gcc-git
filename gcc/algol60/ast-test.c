#include "ast-tab.h"
#include <stdlib.h>
#include <assert.h>

int
main (void)
{
  printf (" + ast sanity\n");
  statement * c1 = stmt_block_create ();
  assert (ast_as (stmt_block, c1));
  assert (ast_as (container, c1));
  assert (ast_as (statement, c1));
  assert (!ast_as (stmt_dummy, c1));
  container_add_stmt (ast_as (container, c1), stmt_dummy_create ());
  container_add_stmt (ast_as (container, c1), stmt_dummy_create ());
  statement * toplev = stmt_toplev_create ();
  container_add_stmt (ast_as (container, toplev), c1);
  statement_dump (toplev, stdout, 0);

  label * l = label_int_create (4);
  assert (ast_as (label, l));
  assert (!ast_as (statement, l));
  assert (!ast_as (symbol, l));

  printf (" + basic types\n");
  type* t1a = type_int ();
  type* t1b = type_int ();
  assert (t1a == t1b);
  assert (types_same (t1a, t1b));
  assert (types_match (t1a, t1b));

  type* t2a = type_real ();
  type* t2b = type_real ();
  assert (t2a == t2b);
  assert (types_same (t2a, t2b));
  assert (types_match (t2a, t2b));
  assert (t1a != t2a);
  assert (!types_same (t1a, t2a));
  assert (!types_match (t1a, t2a));

  type* t3a = type_bool ();
  type* t3b = type_bool ();
  assert (t3a == t3b);
  assert (types_same (t3a, t3b));
  assert (types_match (t3a, t3b));
  assert (t1a != t3a);
  assert (!types_same (t1a, t3a));
  assert (!types_match (t1a, t3a));
  assert (t2a != t3a);
  assert (!types_same (t2a, t3a));
  assert (!types_match (t2a, t3a));

  type* t4a = type_label ();
  type* t4b = type_label ();
  assert (t4a == t4b);
  assert (types_same (t4a, t4b));
  assert (types_match (t4a, t4b));
  assert (t1a != t4a);
  assert (!types_same (t1a, t4a));
  assert (!types_match (t1a, t4a));
  assert (t2a != t4a);
  assert (!types_same (t2a, t4a));
  assert (!types_match (t2a, t4a));
  assert (t3a != t4a);
  assert (!types_same (t3a, t4a));
  assert (!types_match (t3a, t4a));

  type* t5a = type_string ();
  type* t5b = type_string ();
  assert (t5a == t5b);
  assert (types_same (t5a, t5b));
  assert (types_match (t5a, t5b));
  assert (t1a != t5a);
  assert (!types_same (t1a, t5a));
  assert (!types_match (t1a, t5a));
  assert (t2a != t5a);
  assert (!types_same (t2a, t5a));
  assert (!types_match (t2a, t5a));
  assert (t3a != t5a);
  assert (!types_same (t3a, t5a));
  assert (!types_match (t3a, t5a));
  assert (t4a != t5a);
  assert (!types_same (t4a, t5a));
  assert (!types_match (t4a, t5a));

  printf (" + arrays\n");
  type* t6a = t_array_create (type_int ());
  type* t6b = t_array_create (t1a);
  assert (types_same (t6a, t6b));
  assert (types_match (t6a, t6b));
  assert (t1a != t6a);
  assert (!types_same (t1a, t6a));
  assert (!types_match (t1a, t6a));
  assert (t2a != t6a);
  assert (!types_same (t2a, t6a));
  assert (!types_match (t2a, t6a));
  assert (t3a != t6a);
  assert (!types_same (t3a, t6a));
  assert (!types_match (t3a, t6a));
  assert (t4a != t6a);
  assert (!types_same (t4a, t6a));
  assert (!types_match (t4a, t6a));
  assert (t5a != t6a);
  assert (!types_same (t5a, t6a));
  assert (!types_match (t5a, t6a));

  type* t7a = t_array_create (t_array_create (type_int ()));
  type* t7b = t_array_create (t_array_create (t1a));
  type* t7c = t_array_create (t6a);
  type* t7d = t_array_create (t6b);
  assert (types_same (t7a, t7b));
  assert (types_match (t7a, t7b));
  assert (types_same (t7a, t7c));
  assert (types_match (t7a, t7c));
  assert (types_same (t7a, t7d));
  assert (types_match (t7a, t7d));
  assert (!types_same (t1a, t7a));
  assert (!types_match (t1a, t7a));
  assert (!types_same (t2a, t7a));
  assert (!types_match (t2a, t7a));
  assert (!types_same (t3a, t7a));
  assert (!types_match (t3a, t7a));
  assert (!types_same (t4a, t7a));
  assert (!types_match (t4a, t7a));
  assert (!types_same (t5a, t7a));
  assert (!types_match (t5a, t7a));
  assert (!types_same (t6a, t7a));
  assert (!types_match (t6a, t7a));

  printf (" + any\n");
  type* t8a = type_any ();
  type* t8b = type_any ();
  assert (types_same (t8a, t8b));
  assert (types_match (t8a, t8b));
  assert (!types_same (t1a, t8a));
  assert (types_match (t1a, t8a));
  assert (!types_same (t2a, t8a));
  assert (types_match (t2a, t8a));
  assert (!types_same (t3a, t8a));
  assert (types_match (t3a, t8a));
  assert (!types_same (t4a, t8a));
  assert (types_match (t4a, t8a));
  assert (!types_same (t5a, t8a));
  assert (types_match (t5a, t8a));
  assert (!types_same (t6a, t8a));
  assert (types_match (t6a, t8a));
  assert (!types_same (t7a, t8a));
  assert (types_match (t7a, t8a));

  // this test is slightly different from the rest, it checks whether
  // 'array of any' will match 'array of int' and 'array of array of
  // int'
  type* t81a = t_array_create (t8a);
  type* t81b = t_array_create (type_any ());
  type* t81c = t_array_create (type_int ());
  type* t81d = t_array_create (t_array_create (type_int ()));
  assert (types_same (t81a, t81b));
  assert (!types_same (t81a, t81c));
  assert (!types_same (t81a, t81d));
  assert (!types_same (t81c, t81d));

  assert (types_match (t8a, t81a)); // a,b,c,d matches ANY
  assert (types_match (t8a, t81b));
  assert (types_match (t8a, t81c));
  assert (types_match (t8a, t81d));
  assert (types_match (t81a, t81c)); // array of any matches array of int
  assert (types_match (t81a, t81d)); // array of any matches array of array
  assert (!types_match (t81a, t1a)); // array of any doesn't match basic
  assert (!types_match (t81a, t2a));
  assert (!types_match (t81a, t3a));
  assert (!types_match (t81a, t4a));
  assert (!types_match (t81a, t5a));

  printf (" + unknown\n");
  type* t9a = type_unknown ();
  type* t9b = type_unknown ();
  assert (types_same (t9a, t9b));
  assert (!types_match (t9a, t9b)); // don't match yourself
  assert (!types_same (t1a, t9a));
  assert (!types_match (t1a, t9a));
  assert (!types_same (t2a, t9a));
  assert (!types_match (t2a, t9a));
  assert (!types_same (t3a, t9a));
  assert (!types_match (t3a, t9a));
  assert (!types_same (t4a, t9a));
  assert (!types_match (t4a, t9a));
  assert (!types_same (t5a, t9a));
  assert (!types_match (t5a, t9a));
  assert (!types_same (t6a, t9a));
  assert (!types_match (t6a, t9a));
  assert (!types_same (t7a, t9a));
  assert (!types_match (t7a, t9a));
  assert (!types_same (t8a, t9a));
  assert (types_match (t8a, t9a)); // match ANY

  printf (" + own\n");
  type* tA1a = t_own_create (t8a);
  type* tA1b = t_own_create (type_int ());
  type* tA1c = t_own_create (t_array_create (type_int ()));
  type* tA1d = t_own_create (t81a);
  assert (types_match (tA1a, t8a));
  assert (types_match (tA1b, t_own_create (type_int ())));
  assert (types_match (tA1c, t_own_create (t_array_create (type_int ()))));
  assert (types_match (tA1d, t_own_create (t81a)));
  assert (!types_same (tA1a, t8a));
  assert (!types_same (tA1b, t_array_create (type_int ())));
  assert (!types_same (tA1c, t_array_create (t_array_create (type_int ()))));
  assert (!types_same (tA1d, t81a));

  printf ("All passed.\n");
  return 0;
}
