/*
  \item[sign (E)] for the sign of the value of E (+1 for E$>$0, 0 for E$=$0,
        -1 for E$<$0)
*/

int __a60__sign_PirQ (double value);
int __a60__sign_PiiQ (int value);

int
__a60__sign_PirQ (double value)
{
  return (value > 0) ? 1 : ((value < 0) ? -1 : 0);
}

int
__a60__sign_PiiQ (int value)
{
  return (value > 0) ? 1 : ((value < 0) ? -1 : 0);
}
