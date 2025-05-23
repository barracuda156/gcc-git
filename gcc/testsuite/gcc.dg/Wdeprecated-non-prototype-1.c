/* { dg-do compile } */
/* { dg-options "-std=gnu17 -Wdeprecated-non-prototype" } */

void f1 ();
void f2 (); /* { dg-note "declared here" } */
void f3 (...);

void
g ()
{
  f1 ();
  f2 (1); /* { dg-warning "does not allow arguments for function" } */
  f3 (1);
}

void
f1 ()
{
}

void
f2 (int i) /* { dg-warning "does not allow defining parameters for function" } */
{
}
