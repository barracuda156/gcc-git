/* FPU-related code for PowerPC.
   Copyright (C) 2023 Free Software Foundation, Inc.
   Contributed by Sergey Fedorov <vital.had@gmail.com>

This file is part of the GNU Fortran runtime library (libgfortran).

Libgfortran is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

Libgfortran is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
the GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version 3.1,
as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively. If not, see
<http://www.gnu.org/licenses/>. */

#include <fenv.h>

/* FP exception flags */
#define FE_INVALID_SNAN 0x01000000
#define FE_INVALID_ISI  0x00800000
#define FE_INVALID_IDI  0x00400000
#define FE_INVALID_ZDZ  0x00200000
#define FE_INVALID_IMZ  0x00100000
#define FE_INVALID_XVC  0x00080000
#define FE_INVALID_SOFT 0x00000400  /* May not be supported. */
#define FE_INVALID_SQRT 0x00000200  /* May not be supported. */
#define FE_INVALID_CVI  0x00000100
#define FE_NO_EXCEPT    0xC1FFFFFF
/* Values in fenv_private.h and fp_regs.h are inconsistent: the former omits VX_SOFT,
   but includes VX_SQRT, it is reverse in fp_regs.h. It is unclear which is correct,
   if any. Here we assume both to be supported. FreeBSD also adds FE_INVALID in, which is
   arguably wrong. Notice, that implementation of feraiseexcept uses FE_INVALID_SOFT. */
#define FE_ALL_INVALID  0x01F80300
#define FE_NO_INVALID   0xFE07FCFF
#define FE_ALL_FLAGS    0xFFF80300
#define FE_NO_FLAGS     0x0007FCFF
#define FE_NO_ENABLES   0xFFFFFF07

#define FE_ALL_RND      0x00000003
#define FE_NO_RND       0xFFFFFFFC

#define FE_EXCEPT_SHIFT 22
#define EXCEPT_MASK     FE_ALL_EXCEPT >> FE_EXCEPT_SHIFT
/* Floating-point exception summary (FX) bit. */
#define FE_SET_FX       0x80000000
#define FE_CLR_FX       0x7FFFFFFF
/* From libm it appears that FE_INVALID_SOFT is unsupported.
    Glibc also uses FE_INVALID_SNAN for that case. */
#define SET_INVALID     0x01000000

/* Check we can actually store the FPU state in the allocated size. */
_Static_assert (sizeof(fenv_t) <= (size_t) GFC_FPE_STATE_BUFFER_SIZE,
    "GFC_FPE_STATE_BUFFER_SIZE is too small");

/*  Macros to get or set environment flags doubleword  */
#define      FEGETENVD(x) ({ __label__ L1, L2; L1: (void)&&L1; \
                    asm volatile ("mffs %0" : "=f" (x)); \
                    L2: (void)&&L2; })

#define      FESETENVD(x) ({ __label__ L1, L2; L1: (void)&&L1; \
                    asm volatile("mtfsf 255,%0" : : "f" (x)); \
                    L2: (void)&&L2; })

/*  Macros to get or set environment flags doubleword in their own dispatch group  */
#define      FEGETENVD_GRP(x) ({ __label__ L1, L2; L1: (void)&&L1; \
                    asm volatile ("mffs %0" : "=f" (x)); \
                    L2: (void)&&L2; __NOOP; __NOOP; __NOOP; })

#define      FESETENVD_GRP(x) ({ __label__ L1, L2; __NOOP; __NOOP; __NOOP; L1: (void)&&L1; \
                    asm volatile ("mtfsf 255,%0" : : "f" (x)); \
                    L2: (void)&&L2;})

typedef union {
    struct {
        unsigned int hi;
        fenv_t       lo;
    } i;
    double           d;
} hexdouble;

#define HEXDOUBLE(hi, lo) {{ hi, lo }}


void fegetexcept (fexcept_t *flagp, int excepts)
{
    _fegetexceptflag (flagp, excepts);
}

int feclearexcept (int excepts)
{
    hexdouble ifpscr;
    uint32_t mask;

    mask = excepts & FE_ALL_EXCEPT; 
    if ((excepts & FE_INVALID) != 0 ) mask |= FE_ALL_INVALID;
    FEGETENVD_GRP(ifpscr.d);
    mask = ~mask;
    //if ((excepts & FE_INVALID) != 0) mask &= FE_NO_INVALID;
    ifpscr.i.lo &= mask;
    if ((ifpscr.i.lo & FE_ALL_EXCEPT) == 0)
        ifpscr.i.lo &= FE_CLR_FX;
    FESETENVD_GRP(ifpscr.d);
    return 0;
}

int feraiseexcept (int excepts)
{
    uint32_t mask;
    hexdouble ifpscr;

    mask = excepts & FE_ALL_EXCEPT;

    FEGETENVD_GRP(ifpscr.d);

    if ((mask & FE_INVALID) != 0)
    {
        ifpscr.i.lo |= SET_INVALID;
        mask &= ~FE_INVALID;
    }

    if ((mask & (FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO | FE_INEXACT)) != 0)
        ifpscr.i.lo |= mask;

    FESETENVD_GRP(ifpscr.d);
    return 0;
}

int fetestexcept (int excepts)
{
    hexdouble temp;

    FEGETENVD_GRP(temp.d);
    return (int) ((excepts & FE_ALL_EXCEPT) & temp.i.lo);
}

int feholdexcept (fenv_t *envp)
{
    hexdouble ifpscr;

    FEGETENVD_GRP(ifpscr.d);
    *envp = ifpscr.i.lo;
    ifpscr.i.lo &= (FE_NO_FLAGS & FE_NO_ENABLES);
    FESETENVD_GRP(ifpscr.d);
    return 0;
}

static inline int feenableexcept(int mask)
{
    hexdouble ifpscr;
    fenv_t oldmask;

    FEGETENVD_GRP(ifpscr.d);
    oldmask = ifpscr.i.lo;
    ifpscr.i.lo |= (mask & FE_ALL_EXCEPT) >> FE_EXCEPT_SHIFT;
    FESETENVD_GRP(ifpscr.d);
    return ((oldmask & EXCEPT_MASK) << FE_EXCEPT_SHIFT);
}

static inline int fedisableexcept(int mask)
{
    hexdouble ifpscr;
    fenv_t oldmask;

    FEGETENVD_GRP(ifpscr.d);
    oldmask = ifpscr.i.lo;
    ifpscr.i.lo &= ~((mask & FE_ALL_EXCEPT) >> FE_EXCEPT_SHIFT);
    FESETENVD_GRP(ifpscr.d);
    return ((oldmask & EXCEPT_MASK) << FE_EXCEPT_SHIFT);
}

void fesetexcept (fexcept_t *flagp, int excepts)
{
    _fesetexceptflag (flagp, excepts);
}

int fegetround (void)
{
    hexdouble temp;

    FEGETENVD_GRP(temp.d);
    return (int) (temp.i.lo & FE_ALL_RND);
}

int fesetround (int round)
{   
    if ((round & FE_NO_RND))
        return 1;
    else
    {
        hexdouble temp;

        FEGETENVD_GRP(temp.d);
        temp.i.lo = (temp.i.lo & FE_NO_RND) | round;
        FESETENVD_GRP(temp.d);
        return 0;
    }
}

int fegetenv (fenv_t *envp)
{
    hexdouble temp;

    FEGETENVD_GRP(temp.d);
    *envp = temp.i.lo;
    return 0;
}

int fesetenv (const fenv_t *envp)
{
    hexdouble temp;

    temp.i.lo = *envp;
    FESETENVD_GRP(temp.d);
    return 0;
}

int feupdateenv (const fenv_t *envp)
{
    int newexc;
    hexdouble temp;

    FEGETENVD_GRP(temp.d);
    newexc = temp.i.lo & FE_ALL_EXCEPT;
    temp.i.lo = *envp;
    FESETENVD_GRP(temp.d);
    feraiseexcept(newexc);
    return 0;
}


int get_fpu_trap_exceptions (void)
{
  int exceptions = fegetexcept();
  int res = 0;

  if (exceptions & FE_INVALID) res |= GFC_FPE_INVALID;
  if (exceptions & FE_DIVBYZERO) res |= GFC_FPE_ZERO;
  if (exceptions & FE_OVERFLOW) res |= GFC_FPE_OVERFLOW;
  if (exceptions & FE_UNDERFLOW) res |= GFC_FPE_UNDERFLOW;
  if (exceptions & FE_INEXACT) res |= GFC_FPE_INEXACT;

  return res;
}

void set_fpu (void)
{
  if (options.fpe & GFC_FPE_DENORMAL)
    estr_write ("Fortran runtime warning: Floating point 'denormal operand' "
            "exception not supported.\n");

  set_fpu_trap_exceptions (options.fpe, 0);
}

void set_fpu_trap_exceptions (int trap, int notrap)
{
  unsigned int mode_set = 0, mode_clr = 0;

  if (trap & GFC_FPE_INVALID)
    mode_set |= FE_INVALID;
  if (notrap & GFC_FPE_INVALID)
    mode_clr |= FE_INVALID;

  if (trap & GFC_FPE_ZERO)
    mode_set |= FE_DIVBYZERO;
  if (notrap & GFC_FPE_ZERO)
    mode_clr |= FE_DIVBYZERO;

  if (trap & GFC_FPE_OVERFLOW)
    mode_set |= FE_OVERFLOW;
  if (notrap & GFC_FPE_OVERFLOW)
    mode_clr |= FE_OVERFLOW;

  if (trap & GFC_FPE_UNDERFLOW)
    mode_set |= FE_UNDERFLOW;
  if (notrap & GFC_FPE_UNDERFLOW)
    mode_clr |= FE_UNDERFLOW;

  if (trap & GFC_FPE_INEXACT)
    mode_set |= FE_INEXACT;
  if (notrap & GFC_FPE_INEXACT)
    mode_clr |= FE_INEXACT;

  /* Clear stalled exception flags. */
  feclearexcept (FE_ALL_EXCEPT);

  feenableexcept (mode_set);
  fedisableexcept (mode_clr);
}

int get_fpu_except_flags (void)
{
  int result, set_excepts;
  result = 0;
  set_excepts = fetestexcept (FE_ALL_EXCEPT);

  if (set_excepts & FE_INVALID)
    result |= GFC_FPE_INVALID;
  if (set_excepts & FE_DIVBYZERO)
    result |= GFC_FPE_ZERO;
  if (set_excepts & FE_OVERFLOW)
    result |= GFC_FPE_OVERFLOW;
  if (set_excepts & FE_UNDERFLOW)
    result |= GFC_FPE_UNDERFLOW;
  if (set_excepts & FE_INEXACT)
    result |= GFC_FPE_INEXACT;

  return result;
}

void set_fpu_except_flags (int set, int clear)
{
  unsigned int exc_set = 0, exc_clr = 0;

  if (set & GFC_FPE_INVALID)
    exc_set |= FE_INVALID;
  else if (clear & GFC_FPE_INVALID)
    exc_clr |= FE_INVALID;

  if (set & GFC_FPE_ZERO)
    exc_set |= FE_DIVBYZERO;
  else if (clear & GFC_FPE_ZERO)
    exc_clr |= FE_DIVBYZERO;

  if (set & GFC_FPE_OVERFLOW)
    exc_set |= FE_OVERFLOW;
  else if (clear & GFC_FPE_OVERFLOW)
    exc_clr |= FE_OVERFLOW;

  if (set & GFC_FPE_UNDERFLOW)
    exc_set |= FE_UNDERFLOW;
  else if (clear & GFC_FPE_UNDERFLOW)
    exc_clr |= FE_UNDERFLOW;

  if (set & GFC_FPE_INEXACT)
    exc_set |= FE_INEXACT;
  else if (clear & GFC_FPE_INEXACT)
    exc_clr |= FE_INEXACT;

  feclearexcept (exc_clr);
  feraiseexcept (exc_set);
}

void get_fpu_state (void *state)
{
  fegetenv (state);
}

void set_fpu_state (void *state)
{
  fesetenv (state);
}

int get_fpu_rounding_mode (void)
{
  int rnd_mode;
  rnd_mode = fegetround();

  switch (rnd_mode)
    {
      case FE_TONEAREST:
        return GFC_FPE_TONEAREST;
      case FE_UPWARD:
        return GFC_FPE_UPWARD;
      case FE_DOWNWARD:
        return GFC_FPE_DOWNWARD;
      case FE_TOWARDZERO:
        return GFC_FPE_TOWARDZERO;
      default:
        return 0; /* Should be unreachable. */
    }
}

void set_fpu_rounding_mode (int round)
{
  int rnd_mode;

  switch (round)
    {
    case GFC_FPE_TONEAREST:
      rnd_mode = FE_TONEAREST;
      break;
    case GFC_FPE_UPWARD:
      rnd_mode = FE_UPWARD;
      break;
    case GFC_FPE_DOWNWARD:
      rnd_mode = FE_DOWNWARD;
      break;
    case GFC_FPE_TOWARDZERO:
      rnd_mode = FE_TOWARDZERO;
      break;
    default:
      return; /* Should be unreachable. */
    }

  fesetround (rnd_mode);
}

int support_fpu_flag (int flag)
{
  if (flag & GFC_FPE_DENORMAL)
    return 0;

  return 1;
}

int support_fpu_trap (int flag)
{
  if (flag & GFC_FPE_DENORMAL)
    return 0;

  return 1;
}

int support_fpu_rounding_mode(int mode __attribute__((unused)))
{
  return 1;
}

/* The following are not supported. */

int support_fpu_underflow_control(int kind __attribute__((unused)))
{
  return 0;
}

int get_fpu_underflow_mode(void)
{
  return 0;
}

void set_fpu_underflow_mode(int gradual __attribute__((unused)))
{
}
