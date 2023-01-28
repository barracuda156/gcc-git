#include "coretypes.h"
#include "tree.h"

/* Language-specific identifier information */
struct lang_identifier GTY(())
{
  struct tree_identifier common;
};


/* Language-specific type information.  */
struct lang_type GTY(())
{
  char junk; /* dummy field to ensure struct is not empty */
};


/* Language-specific declaration information.  */
struct lang_decl GTY(())
{
  char junk; /* dummy field to ensure struct is not empty */
};

struct language_function GTY(())
{
  char junk; /* dummy field to ensure struct is not empty */
};

/* The resulting tree type.  */
union lang_tree_node 
  GTY((desc ("TREE_CODE (&%h.generic) == IDENTIFIER_NODE"),
       chain_next ("(union lang_tree_node *)TREE_CHAIN (&%h.generic)")))
{
  union tree_node GTY ((tag ("0"), 
			desc ("tree_node_structure (&%h)"))) 
    generic;
  struct lang_identifier GTY ((tag ("1"))) identifier;
};


#include "tm.h"     /* machine dependent defines of BITS_PER_UNIT and
		       UNITS_PER_WORD */

/* These are used to build types for various sizes. */
#ifndef MAX_BITS_PER_WORD
#define MAX_BITS_PER_WORD  BITS_PER_WORD
#endif

/* This variable keeps a table for types for each precision so that we only 
   allocate each of them once. Signed and unsigned types are kept separate.  */
static GTY(()) tree signed_and_unsigned_types[MAX_BITS_PER_WORD + 1][2];

bool algol60_init (void);
void algol60_finish (void);
unsigned int algol60_init_options (unsigned int, const char **);
int algol60_handle_option (size_t scode, const char *arg, int value);
bool algol60_post_options (const char **pfilename);
void algol60_parse_file (int debug);
bool algol60_mark_addressable (tree exp);
tree algol60_lang_signed_type (tree type_node);
tree algol60_lang_unsigned_type (tree type_node);
tree algol60_lang_signed_or_unsigned_type (int unsignedp, tree type_node);
tree algol60_lang_type_for_mode (enum machine_mode mode, int unsignedp);
tree algol60_lang_type_for_size (unsigned precision, int unsignedp);
void algol60_expand_function (tree fndecl);

tree pushdecl (tree);
tree getdecls (void);
int global_bindings_p (void);
void insert_block (tree);
tree builtin_function (const char *, tree, int, enum built_in_class, const char *, tree);

extern tree string_type_node;
