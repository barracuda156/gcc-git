#include "config.h"
#include "system.h"
#include "gcc.h"

#ifndef MATH_LIBRARY
#define MATH_LIBRARY "-lm"
#endif

#ifndef LIBGA60
#define LIBGA60 "-lga60"
#endif


/* This is called before processing source files.  Command line
   arguments may be tweaked here. */
void
lang_specific_driver (int *in_argc, const char *const **in_argv,
		      int *in_added_libraries)
{
  /* 0  means we should not link in libga60
     1  means we should link in libga60 */
  int library = 1;

  /* By default, we throw on the math library if we have one.  */
  int need_math = (MATH_LIBRARY[0] != '\0');

  /* True if we should add -shared-libgcc to the command-line.  */
  int shared_libgcc = 1;

  /* Used to track options that take arguments. */
  const char *quote = NULL;

  /* Number of arguments. */
  int argc = *in_argc;

  /* The argument list.  */
  const char *const *argv = *in_argv;

  /* The total number of arguments with the new stuff.  */
  int num_args = 1;

  /* The number of libraries added in.  */
  int added_libraries = *in_added_libraries;

  int i, j;

  for (i = 1; i < argc; i++)
    {
      /* If the previous option took an argument, we swallow it here.  */
      if (quote)
	{
	  quote = NULL;
	  continue;
	}

      if (argv[i][0] == '\0' || argv[i][1] == '\0')
	continue;

      if (argv[i][0] == '-')
	{
	  if (strcmp (argv[i], "-nostdlib") == 0
	      || strcmp (argv[i], "-nodefaultlibs") == 0)
	    library = 0;
	  else if (strcmp (argv[i], MATH_LIBRARY) == 0)
	    need_math = 0;
	  else if (strcmp (argv[i], "-Xlinker") == 0)
	    quote = argv[i];
	  else if (((argv[i][2] == '\0'
		     && strchr ("bBVDUoeTuIYmLiA", argv[i][1]) != NULL)
		    || strcmp (argv[i], "-Tdata") == 0))
	    quote = argv[i];
	  else if ((argv[i][2] == '\0'
		    && strchr ("cSEM", argv[i][1]) != NULL)
		   || strcmp (argv[i], "-MM") == 0
		   || strcmp (argv[i], "-fsyntax-only") == 0)
	    {
	      /* Don't specify libraries if we won't link, since that would
		 cause a warning.  */
	      library = 0;
	    }
	  else if (DEFAULT_WORD_SWITCH_TAKES_ARG (&argv[i][1]))
	    i++;
	}
      else
	{
	  // a filename...
	}
    }

  if (quote)
    fatal ("argument to '%s' missing\n", quote);

  /* If we know we don't have to do anything, bail now.  */
  if (library == 0)
    return;

  /* There's no point adding -shared-libgcc if we don't have a shared
     libgcc.  */
#ifndef ENABLE_SHARED_LIBGCC
  shared_libgcc = 0;
#endif

  /* Make sure to have room for the trailing NULL argument.  */
  num_args = argc + library + need_math + shared_libgcc + 1;
  char const * * arglist = XNEWVEC (char const *, num_args);

  i = 0;
  j = 0;

  /* Copy the 0th argument, i.e., the name of the program itself.  */
  arglist[i++] = argv[j++];

  char const * saw_math = 0;
  char const * saw_libc = 0;

  /* NOTE: We start at 1 now, not 0.  */
  while (i < argc)
    {
      arglist[j] = argv[i];

      /* Make sure -lga60 is before the math library, since libga60
	 itself uses those math routines. */

      if (!saw_math && (strcmp (argv[i], MATH_LIBRARY) == 0) && library)
	{
	  --j;
	  saw_math = argv[i];
	}

      if (!saw_libc && (strcmp (argv[i], "-lc") == 0) && library)
	{
	  --j;
	  saw_libc = argv[i];
	}

      i++;
      j++;
    }

  /* Add `-lga60' if we haven't already done so.  */
  if (library)
    {
      arglist[j] = LIBGA60;
      if (arglist[j][0] != '-' || arglist[j][1] == 'l')
	added_libraries++;
      j++;
    }

  if (saw_math)
    arglist[j++] = saw_math;
  else if (library && need_math)
    {
      arglist[j] = MATH_LIBRARY;
      if (arglist[j][0] != '-' || arglist[j][1] == 'l')
	added_libraries++;
      j++;
    }

  if (saw_libc)
    arglist[j++] = saw_libc;
  if (shared_libgcc)
    arglist[j++] = "-shared-libgcc";

  arglist[j] = NULL;

  *in_argc = j;
  *in_argv = arglist;
  *in_added_libraries = added_libraries;
}

/* Called before linking.  Returns 0 on success and -1 on failure.  */
int
lang_specific_pre_link (void)
{
  return 0;
}

/* Number of extra output files that lang_specific_pre_link may
   generate.  */
int lang_specific_extra_outfiles = 0;
