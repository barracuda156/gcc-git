#ifdef __POWERPC__ // No support for issignaling in math.h on Darwin PPC

int isnansf (float x)       { return __builtin_issignaling (x) ? 1 : 0; }
int isnans  (double x)      { return __builtin_issignaling (x) ? 1 : 0; }
int isnansl (long double x) { return __builtin_issignaling (x) ? 1 : 0; }

#else

#define _GNU_SOURCE
#include <math.h>
#include <float.h>

int isnansf (float x)       { return issignaling (x) ? 1 : 0; }
int isnans  (double x)      { return issignaling (x) ? 1 : 0; }
int isnansl (long double x) { return issignaling (x) ? 1 : 0; }

#endif
