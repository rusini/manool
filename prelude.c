// -include prelude.c --- stuff to appear at the *top* of each TU written in C

/*    Copyright (C) 2018-2025 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


// Standard/Mode + Extensions

# if __STDC_VERSION__ < 201710/*C17*/ || !__STDC_HOSTED__ || !__GNUC__/*gcc/clang/icx...*/ || !__STRICT_ANSI__/*-std=cNN*/
   # error "Unsupported C compiler or compiler mode"
# endif
# if __STDC_VERSION__ > 201710
   # warning "Compiling with a later C standard may break compatibility"
# endif

// (undesirable) defaults on modern Ubuntu
# if __SSP__ || __SSP_ALL__ || __SSP_STRONG__
   # warning "Please recompile with -fno-stack-protector"
# endif
# if __CET__
   # warning "Please recompile with -fcf-protection=none"
# endif
# ifdef _FORTIFY_SOURCE
   # warning "Please recompile with -U_FORTIFY_SOURCE"
# endif

# if __NO_MATH_ERRNO__ // mostly useless but disabling it may cause compatibility issues with third-party libraries
   _Static_assert(0, "Please do not use -fno-math-errno");
# endif

// Feature-Test Macros
# define _GNU_SOURCE // just ignored on many platforms not using glibc
// use -D_{FILE_OFFSET,TIME}_BITS=64 consistently with how third-party SOs are compiled!

# include <limits.h>
# include <float.h>
# include <stddef.h>
# include <stdint.h>

// Integer/Pointer Properties --- these checks are both complete and nonredundant

_Static_assert(
   CHAR_BIT == 8,
   "The target ISA shall be octet-addressable" );
_Static_assert(
   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ | __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__,
   "The target ISA shall use a consistent endianness (LE or BE)" );
_Static_assert(
   SCHAR_MIN == -0x80,
   "Unsupported `signed char` properties for the target ABI"
); // provably 8-bit 2's-complement representation using full range and no padding

_Static_assert(
   sizeof(int) == 4 &&
   INT_MAX     == +0x7FFFFFFFl &&
   INT_MIN + 1 == -0x7FFFFFFFl,
   "Unsupported `int` properties for the target ABI"
); // provably 32-bit 2's complement representation using full range and no padding
_Static_assert(
   sizeof(long) == 8 | sizeof(long) == 4 &&
   LONG_MAX     == +(sizeof(long) == 8 ? 0x7FFFFFFFFFFFFFFFll : 0x7FFFFFFFl) &&
   LONG_MIN + 1 == -(sizeof(long) == 8 ? 0x7FFFFFFFFFFFFFFFll : 0x7FFFFFFFl),
   "Unsupported `long` properties for the target ABI"
); // provably 64- or 32-bit 2's-complement representation using full range and no padding
_Static_assert(
   sizeof(long long) == 8 &&
   LLONG_MIN + 1 == -0x7FFFFFFFFFFFFFFFll,
   "Unsupported `long long` properties for the target ABI"
); // provably 64-bit 2's-complement representation using full range and no padding
_Static_assert(
   sizeof(short) == 2 &&
   SHRT_MIN + 1 == -0x7FFF,
   "Unsupported `short` properties for the target ABI"
); // provably 16-bit 2's-complement representation using full range and no padding

_Static_assert(
   sizeof(unsigned) == 4 &&
   UINT_MAX == 0xFFFFFFFF,
   "Unsupported `unsigned` properties for the target ABI"
); // provably 32-bit representation w/ no padding
_Static_assert(
   sizeof(unsigned long) == 8 | sizeof(unsigned long) == 4 &&
   ULONG_MAX == (sizeof(unsigned long) == 8 ? 0xFFFFFFFFFFFFFFFF : 0xFFFFFFFF),
   "Unsupported `unsigned long` properties for the target ABI"
); // provably 64-bit or 32-bit representation w/ no padding
_Static_assert(
   sizeof(unsigned long long) == 8,
   "Unsupported `unsigned long long` properties for the target ABI"
); // provably 64-bit representation w/ no padding
_Static_assert(
   sizeof(unsigned short) == 2,
   "Unsupported `unsigned short` properties for the target ABI"
); // provably 16-bit representation w/ no padding
_Static_assert(
   sizeof(void *) == 8 | sizeof(void *) == 4,
   "Unsupported object pointer size for the target ABI" );

_Static_assert(
   sizeof(long) == sizeof(void *),
   "The target platform shall use LP64 or ILP32 data model" );
# ifndef __INTPTR_TYPE__
   _Static_assert(false, "Roundtrip conversion between `void *` and `long` is unavailable on the target");
# endif
# ifndef __UINTPTR_TYPE__
   _Static_assert(false, "Roundtrip conversion between `void *` and `unsigned long` is unavailable on the target");
# endif

_Static_assert(
   sizeof(size_t) == sizeof(unsigned long) &&
   SIZE_MAX == ULONG_MAX &&
   sizeof(ptrdiff_t) == sizeof(long) &&
   PTRDIFF_MIN == LONG_MIN && PTRDIFF_MAX == LONG_MAX,
   "`size_t`/`ptrdiff_t` shall be consistent with `unsigned long`/`long`" );

// FP Properties --- these checks are nonredundant but cannot be made 100% complete

// __STDC_IEC_559__ (either defined by the driver or libc) is unreliable on gcc/clang!

# ifdef __FLOAT_WORD_ORDER__
   _Static_assert( __FLOAT_WORD_ORDER__ == __BYTE_ORDER__,
      "The target shall use a FP endianness consistent with the rest of the ISA" );
# endif

# if __FAST_MATH__ || __FINITE_MATH_ONLY__
   _Static_assert(0, "Noncompliant or unsupported math mode");
# endif

# pragma STDC FENV_ACCESS OFF // provided for completeness and might be unimplemented
# pragma STDC FP_CONTRACT OFF // ditto
_Static_assert(
   FLT_EVAL_METHOD == 0 | FLT_EVAL_METHOD == 1, // relaxed for C unlike C++
   "Unsupported FP evaluation mode" );

_Static_assert(
   sizeof(double)  ==     8 &&
   FLT_RADIX       ==     2 &&
   DBL_MANT_DIG    ==    53 &&
   DBL_MAX_EXP     == +1024 &&
   DBL_MIN_EXP     == -1021 &&
   DBL_HAS_SUBNORM == 1     &&
   __DBL_HAS_INFINITY__ && __DBL_HAS_QUIET_NAN__, // have to resort to gcc-specific macros
   "The `double` type shall have IEEE754 format"
); // highly likely IEEE754 binary64 format --- assuming it
_Static_assert(
   sizeof(float)   ==     4 &&
   FLT_RADIX       ==     2 &&
   FLT_MANT_DIG    ==    24 &&
   FLT_MAX_EXP     ==  +128 &&
   FLT_MIN_EXP     ==  -125 &&
   FLT_HAS_SUBNORM == 1     &&
   __FLT_HAS_INFINITY__ && __FLT_HAS_QUIET_NAN__, // have to resort to gcc-specific macros
   "The `float` type shall have IEEE754 format"
); // highly likely IEEE754 binary64 format --- assuming it

// Lock-Free Atomic Operations (using gcc builtins)

_Static_assert(
   __GCC_ATOMIC_LONG_LOCK_FREE    == 2 && // typically needed for refcounting using "machine words"
   __GCC_ATOMIC_INT_LOCK_FREE     == 2 && // general counting using the "default" type
   __GCC_ATOMIC_POINTER_LOCK_FREE == 2,   // similar to `long`
   "The target lacks support for core lock-free atomic operations" );

// OS/libc Personality --- relies on some assumptions

# if !__linux__ && !__FreeBSD__
   # include <features.h>
   # if !(__GLIBC__ > 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ >= 25)
      _Static_assert(false, "Unsupported target libc (AKA C runtime) detected");
   # endif
# endif
