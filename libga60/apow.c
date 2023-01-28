/* Very straightforward definition for to-be algol library.  This
   should be rewritten to use more optimal computation methods,
   e.g. see how gfortran does it. */

#include "config.h"

#ifdef HAVE_STDLIB_H
void abort (void);

int __a60__pow_PiiiQ (int a, int i);
double __a60__pow_PrriQ (double a, int i);

int
__a60__pow_PiiiQ (int a, int i)
{
  if (i > 0)
    {
      int result = a;
      while (--i)
	result *= a;
      return result;
    }
  else if (i == 0)
    {
      // Algol spec leaves it undefined what happens if you do `0**0'.
      if (a != 0)
	return 1;
      else
	abort ();
    }
  else /* i < 0 */
    {
      if (a == 0)
	abort ();
      else if (a == 1)
	return 1;
      else if (a == -1)
	return -1;
      else
	return 0;
    }
}

double
__a60__pow_PrriQ (double a, int i)
{
  if (i > 0)
    {
      double result = a;
      while (--i)
	result *= a;
      return result;
    }
  else if (i == 0)
    {
      // Algol spec leaves it undefined what happens if you do `0**0'.
      if (a != 0)
	return 1;
      else
	abort ();
    }
  else /* i < 0 */
    {
      if (a == 0)
	abort ();
      else
	return (double)1.0 / __a60__pow_PrriQ (a, -i);
    }
}

# ifdef HAVE_MATH_H

double exp (double);
double log (double);

double __a60__pow_PrrrQ (double a, double r);
double __a60__pow_PrirQ (int a, double r);

double
__a60__pow_PrrrQ (double a, double r)
{
  if (a > 0)
    return exp (r * log (a));
  else if (a == 0)
    {
      if (r > 0)
	return 0.0;
      else
	abort ();
    }
  else /* a < 0 */
    abort ();
}

double
__a60__pow_PrirQ (int a, double r)
{
  return __a60__pow_PrrrQ ((double)a, r);
}
# endif

#endif
