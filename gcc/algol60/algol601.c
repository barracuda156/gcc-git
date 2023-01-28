/* Algol 60 Compiler almost main (algol601)
   Called by GCC's toplev.c*/

/* system defines... required by later includes */
#include "config.h"
#include "system.h"

#include "ggc.h" /* contains `struct ggc_root_tab' type required by
		    gtype-algol.h */

#include "coretypes.h"
#include "tree.h"
#include "algol-tree.h"
#include "debug.h"
#include "gtype-algol60.h" /* Contains gt_* garbage collecting
			      definitions. */
