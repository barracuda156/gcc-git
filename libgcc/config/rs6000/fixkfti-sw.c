/* Software floating-point emulation, convert IEEE quad to 128bit signed
   integer.

   Copyright (C) 2016-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Steven Munroe (munroesj@linux.vnet.ibm.com)
   Code is based on the main soft-fp library written by:
	   Uros Bizjak (ubizjak@gmail.com).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file into
   combinations with other programs, and to distribute those
   combinations without any restriction coming from the use of this
   file.  (The Lesser General Public License restrictions do apply in
   other respects; for example, they cover modification of the file,
   and distribution when not linked into a combine executable.)

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef _ARCH_PPC64
#include "soft-fp.h"
#include "quad-float128.h"

TItype
__fixkfti_sw (TFtype a)
{
  FP_DECL_EX;
  FP_DECL_Q (A);
  UTItype r;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_Q (A, a);
  FP_TO_INT_Q (r, A, TI_BITS, 1);
  FP_HANDLE_EXCEPTIONS;

  return r;
}
#endif
