/* Fucntion entier:

    It is understood that transfer functions between any pair of
    quantities and expressions my be defined.  Among the standard
    functions it is recommended that there be one, namely entier (E),
    which ``transfers'' an expression of real type to one of integer
    type, and assigns to it the value which is the largest integer not
    greater than the value of E. */

int __a60__entier_PirQ (double r);
double __a60__entier_PriQ (int r);

int
__a60__entier_PirQ (double r)
{
  return (int)r;
}

double
__a60__entier_PriQ (int r)
{
  return (double)r;
}
