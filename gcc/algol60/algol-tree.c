#include "config.h" /* system.h needs to know if it's safe to include
		       <string.h>/<strings.h> stuff; this is
		       host-specific, and config.h knows it */

#include "system.h" /* stdlib, strings... system includes... for some
		       reason they can't be included directly, has
		       something to do with host/build/target
		       trichotomy. */

#include "coretypes.h" /* contains definition of `tree', required by
			  tree.h */

#include "tree.h"   /* treecodes used in tree.def, required to define
		       tree_code_* arrays */

#include "algol-tree.h"


#include "ggc.h"             /* for ggc_root_tab */
#include "debug.h"
//#include "gt-algol60-algol-tree.h" /* garbage collecting definitions */


#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) TYPE,

const enum tree_code_class tree_code_type[] = {
#include "tree.def"
  tcc_exceptional
};
#undef DEFTREECODE

#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) LENGTH,

const unsigned char tree_code_length[] = {
#include "tree.def"
  0
};
#undef DEFTREECODE

#define DEFTREECODE(SYM, NAME, TYPE, LEN) NAME,

const char *const tree_code_name[] = {
#include "tree.def"
  "@@dummy"
};
#undef DEFTREECODE


/* Record a decl-node X as belonging to the current lexical scope.
   Check for errors (such as an incompatible declaration for the same
   name already seen in the same scope). */
tree
pushdecl (tree x)
{
  return x;
}

/* Return the list of declarations of the current level. */
tree
getdecls (void)
{
  return NULL;
}

/* Nonzero if we are currently in the global binding level.  */
int
global_bindings_p (void)
{
  return 0;/*current_binding_level == global_binding_level;*/
}

/* Insert BLOCK at the end of the list of subblocks of the current
   binding level.  This is used when a BIND_EXPR is expanded, to
   handle the BLOCK node inside the BIND_EXPR.  */
void
insert_block (tree block ATTRIBUTE_UNUSED)
{
  gcc_unreachable ();
  /*
  TREE_USED (block) = 1;
  current_binding_level->blocks
    = chainon (current_binding_level->blocks, block);
  */
}

/* Return a definition for a builtin function named NAME and whose
   data type is TYPE.  TYPE should be a function type with argument
   types.  FUNCTION_CODE tells later passes how to compile calls to
   this function.  See tree.h for its possible values.

   If LIBRARY_NAME is nonzero, use that for DECL_ASSEMBLER_NAME, the
   name to be called if we can't opencode the function.  If ATTRS is
   nonzero, use that for the function's attribute list.

   copied from gcc/c-decl.c */
tree
builtin_function (const char *name,
		  tree type ATTRIBUTE_UNUSED,
		  int function_code ATTRIBUTE_UNUSED,
		  enum built_in_class cl ATTRIBUTE_UNUSED,
		  const char *library_name,
		  tree attrs ATTRIBUTE_UNUSED)
{
  tree decl = build_decl (FUNCTION_DECL, get_identifier (name), type);
  DECL_EXTERNAL (decl) = 1;
  TREE_PUBLIC (decl) = 1;
  if (library_name)
    SET_DECL_ASSEMBLER_NAME (decl, get_identifier (library_name));
  pushdecl (decl);
  DECL_BUILT_IN_CLASS (decl) = cl;
  DECL_FUNCTION_CODE (decl) = function_code;

  /* Possibly apply some default attributes to this built-in function.  */
  if (attrs)
    decl_attributes (&decl, attrs, ATTR_FLAG_BUILT_IN);
  else
    decl_attributes (&decl, NULL_TREE, 0);

  return decl;
}

tree string_type_node;
