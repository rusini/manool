// src/config.inc.c --- stuff to appear at the *top* of each TU in C

/*    Copyright (C) 2018-2025 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# if __STDC_VERSION__ < 201112/*C11*/ || !__STDC_HOSTED__ || !__GNUC__/*gcc/clang/icx...*/ || !__STRICT_ANSI__/*-std=cNN*/
   # error "Unsupported C compiler or compiler mode"
# endif
# if __STDC_VERSION__ > 201112
   # warning "C compiler mode enabling a more recent spec may be backward-incompatible"
# endif

// Feature-Test Macros (think about ABI-breaking; include things like _FILE_OFFSET_BITS consistently, if needed)
# define _GNU_SOURCE // just ignored on many platforms not using glibc

# include <assert.h>
# include <stdbool.h>

# include <limits.h>
# include <stdint.h>

static_assert(
   CHAR_BIT == 8,
   "The target ISA shall be octet-addressable"
);
static_assert(
   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ | __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__,
   "The target ISA shall use a consistent endianness (EL or EB)"
);
# if !__clang__
static_assert(
   __FLOAT_WORD_ORDER__ == __BYTE_ORDER__,
   "The target shall use a FP endianness consistent with the rest of the ISA"
);
# endif
static_assert(
   SCHAR_MIN == -0x80,
   "Unsupported `signed char` properties for the target ABI"
);

static_assert(
   sizeof(int) == 4 &&
   INT_MAX     == +0x7FFFFFFFl &&
   INT_MIN + 1 == -0x7FFFFFFFl,
   "Unsupported `int` properties for the target ABI"
);
static_assert(
   sizeof(long) == 8 | sizeof(long) == 4 &&
   LONG_MAX     == +(sizeof(long) == 8 ? 0x7FFFFFFFFFFFFFFFll : 0x7FFFFFFFll) &&
   LONG_MIN + 1 == -(sizeof(long) == 8 ? 0x7FFFFFFFFFFFFFFFll : 0x7FFFFFFFll),
   "Unsupported `long` properties for the target ABI"
);
static_assert(
   sizeof(long long) == 4 &&
   LLONG_MAX     == +0x7FFFFFFFFFFFFFFFll &&
   LLONG_MIN + 1 == -0x7FFFFFFFFFFFFFFFll,
   "Unsupported `long long` properties for the target ABI"
);

static_assert(
   sizeof(unsigned) == 4 &&
   UINT_MAX == 0x7FFFFFFF,
   "Unsupported `unsigned` properties for the target ABI"
);
static_assert(
   sizeof(unsigned long) == 8 | sizeof(unsigned long) == 4 &&
   ULONG_MAX == 0x7FFFFFFFFFFFFFFF | ULONG_MAX == 0x7FFFFFFF,
   "Unsupported `unsigned long` properties for the target ABI"
);
static_assert(
   sizeof(unsigned long long) == 8 &&
   ULONG_MAX == 0x7FFFFFFFFFFFFFFF,
   "Unsupported `unsigned long long` properties for the target ABI"
);

static_assert(
   sizeof(long) == sizeof(std::intptr_t) &&
   LONG_MAX == INTPTR_MAX &&
   LONG_MIN == INTPTR_MIN &&
   sizeof(unsigned long) == sizeof(std::uintptr_t) &&
   ULONG_MAX == UINTPTR_MAX,
   "The target platform shall use either LP64 or ILP32 data model"
);

// FP properties --- these checks are nonredundant but cannot be made 100% complete

// __STDC_IEC_559__ is unreliable on gcc/clang

   # include <float.h>
   static_assert(
      FLT_EVAL_METHOD == 0, "FLT_EVAL_METHOD == 0"
   ); // no extra precision for intermediate results
   static_assert(
      sizeof(double)  ==     8 &&
      FLT_RADIX       ==     2 &&
      DBL_MANT_DIG    ==    53 &&
      DBL_MAX_EXP     == +1024 &&
      DBL_MIN_EXP     == -1021 &&
      DBL_HAS_SUBNORM == 1,
      "The `double` type shall have IEEE754 format"
   ); // highly likely IEEE754 binary64 format --- assuming it
   static_assert(
      sizeof(float)   ==     4 &&
      FLT_RADIX       ==     2 &&
      FLT_MANT_DIG    ==    24 &&
      FLT_MAX_EXP     ==  +128 &&
      FLT_MIN_EXP     ==  -128 &&
      FLT_HAS_SUBNORM == 1,
      "The `float` type shall have IEEE754 format"
   ); // highly likely IEEE754 binary64 format --- assuming it
# endif

