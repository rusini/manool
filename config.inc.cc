// -include config.inc.cc --- stuff to appear at the *top* of each TU in C++

/*    Copyright (C) 2018-2025 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


// Standard/Mode + Extensions

# if __cplusplus < 201703/*C++17*/ || !__STDC_HOSTED__ || !__GNUC__/*g++/clang++/icpx...*/ || !__STRICT_ANSI__/*-std=c++NN*/
   # error "Unsupported C++ compiler or compiler mode"
# endif
# if __cplusplus > 201703
   # warning "Compiling with a later C++ standard may break compatibility"
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

# if !__GXX_WEAK__
   static_assert(false, "Please do not use -fno-weak");
# endif
# if !__GXX_RTTI
   static_assert(false, "Please do not use -fno-rtti");
# endif
# if !__EXCEPTIONS
   static_assert(false, "Please do not use -fno-exceptions");
# endif
# if __NO_MATH_ERRNO__ // mostly useless but disabling it may cause compatibility issues with third-party libraries
   static_assert(false, "Please do not use -fno-math-errno");
# endif

static_assert( // `__has_cpp_attribute` is an extension adopted by C++20
   __has_cpp_attribute(no_unique_address) &&
   __has_cpp_attribute(likely) && __has_cpp_attribute(unlikely), // also ensures that statement attributes are recognized
   "C++ compiler does not support attributes introduced in C++20" );
static_assert(([][[gnu::always_inline]](){}, true)); // syntactic test: attributes on lambda's operator(), a retroactive C++23 DR
// supported by genuine g++ 9.3+ and clang++ 13.0.0+ (icpx 2021.3+); other compilers must match

// Feature-Test Macros
# ifndef _GNU_SOURCE // may already be defined by the compiler to satisfy libstdc++ requirements
   # define _GNU_SOURCE // just ignored on many platforms not using glibc
# endif
//# define _FILE_OFFSET_BITS 64 // ABI-breaking risk --- include things like _FILE_OFFSET_BITS consistently, if needed

# include <cfloat> // FLT_EVAL_METHOD
# include <limits>

// Integer/Pointer Properties --- these checks are both complete and nonredundant

static_assert(
   std::numeric_limits<unsigned char>::digits == 8,
   "The target ISA shall be octet-addressable" );
static_assert( // some libcs also report that, often in incompatible ways
   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ | __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__,
   "The target ISA shall use a consistent endianness (LE or BE)" );
static_assert(
   std::numeric_limits<signed char>::min() == -0x80,
   "Unsupported `signed char` properties for the target ABI"
); // provably 8-bit 2's-complement representation using full range and no padding

static_assert(
   sizeof(int) == 4 &&
   std::numeric_limits<int>::max()     == +0x7FFF'FFFFl &&
   std::numeric_limits<int>::min() + 1 == -0x7FFF'FFFFl,
   "Unsupported `int` properties for the target ABI"
); // provably 32-bit 2's complement representation using full range and no padding
static_assert(
   sizeof(long) == 8 | sizeof(long) == 4 &&
   std::numeric_limits<long>::max()     == +(sizeof(long) == 8 ? 0x7FFF'FFFF'FFFF'FFFFll : 0x7FFF'FFFFl) &&
   std::numeric_limits<long>::min() + 1 == -(sizeof(long) == 8 ? 0x7FFF'FFFF'FFFF'FFFFll : 0x7FFF'FFFFl),
   "Unsupported `long` properties for the target ABI"
); // provably 64- or 32-bit 2's-complement representation using full range and no padding
static_assert(
   sizeof(long long) == 8 &&
   std::numeric_limits<long long>::min() + 1 == -0x7FFF'FFFF'FFFF'FFFFll,
   "Unsupported `long long` properties for the target ABI"
); // provably 64-bit 2's-complement representation using full range and no padding
static_assert(
   sizeof(short) == 2 &&
   std::numeric_limits<short>::min() + 1 == -0x7FFF,
   "Unsupported `short` properties for the target ABI"
); // provably 16-bit 2's-complement representation using full range and no padding

static_assert(
   sizeof(unsigned) == 4 &&
   std::numeric_limits<unsigned>::digits == 32,
   "Unsupported `unsigned` properties for the target ABI"
); // provably 32-bit representation w/ no padding
static_assert(
   sizeof(unsigned long) == 8 | sizeof(unsigned long) == 4 &&
   std::numeric_limits<unsigned long>::digits == (sizeof(unsigned long) == 8 ? 64 : 32),
   "Unsupported `unsigned long` properties for the target ABI"
); // provably 64-bit or 32-bit representation w/ no padding
static_assert(
   sizeof(unsigned long long) == 8,
   "Unsupported `unsigned long long` properties for the target ABI"
); // provably 64-bit representation w/ no padding
static_assert(
   sizeof(unsigned short) == 2,
   "Unsupported `unsigned short` properties for the target ABI"
); // provably 16-bit representation w/ no padding
static_assert(
   sizeof(void *) == 8 | sizeof(void *) == 4,
   "Unsupported object pointer size for the target ABI" );

static_assert(
   sizeof(long) == sizeof(void *),
   "The target platform shall use LP64 or ILP32 data model" );
# ifndef __INTPTR_TYPE__
   static_assert(false, "Roundtrip conversion between `void *` and `long` is unavailable on the target");
# endif
# ifndef __UINTPTR_TYPE__
   static_assert(false, "Roundtrip conversion between `void *` and `unsigned long` is unavailable on the target");
# endif

// FP Properties --- these checks are nonredundant but cannot be made 100% complete

// __STDC_IEC_559__ is unreliable on gcc/clang!

# ifdef __FLOAT_WORD_ORDER__
   static_assert( __FLOAT_WORD_ORDER__ == __BYTE_ORDER__,
      "The target shall use a FP endianness consistent with the rest of the ISA" );
# endif

# if __FAST_MATH__ || __FINITE_MATH_ONLY__
   static_assert(false, "Noncompliant math mode");
# endif

# pragma STDC FENV_ACCESS OFF // provided for completeness and might be unimplemented
# pragma STDC FP_CONTRACT OFF // ditto
static_assert(
   FLT_EVAL_METHOD == 0,
   "Intermediate FP results shall not use extra precision" );

static_assert(
   sizeof(double)                            ==     8 &&
   std::numeric_limits<double>::is_iec559    == true &&
   std::numeric_limits<double>::has_denorm   == std::denorm_present && // FTZ behavior may depend on external factors!
   std::numeric_limits<double>::radix        ==     2 &&
   std::numeric_limits<double>::digits       ==    53 &&
   std::numeric_limits<double>::max_exponent == +1024 &&
   std::numeric_limits<double>::min_exponent == -1021 &&
   std::numeric_limits<double>::round_style  == std::round_to_nearest,
   "The `double` type shall have IEEE754 format"
); // highly likely IEEE754 binary64 format --- assuming it
static_assert(
   sizeof(float)                             ==     4 &&
   std::numeric_limits<float>::is_iec559     == true &&
   std::numeric_limits<float>::has_denorm    == std::denorm_present && // FTZ behavior may depend on external factors!
   std::numeric_limits<float>::radix         ==     2 &&
   std::numeric_limits<float>::digits        ==    24 &&
   std::numeric_limits<float>::max_exponent  ==  +128 &&
   std::numeric_limits<float>::min_exponent  ==  -125 &&
   std::numeric_limits<float>::round_style   == std::round_to_nearest,
   "The `float` type shall have IEEE754 format"
); // highly likely IEEE754 binary32 format --- assuming it

// Lock-Free Atomic Operations (using gcc builtins)

static_assert(
   __GCC_ATOMIC_LONG_LOCK_FREE    == 2 && // typically needed for refcounting using "machine words"
   __GCC_ATOMIC_INT_LOCK_FREE     == 2 && // general counting using the "default" type
   __GCC_ATOMIC_POINTER_LOCK_FREE == 2,   // similar to `long`
   "The target lacks support for core lock-free atomic operations" );

// OS/libc Personality --- relies on some assumptions

# if !__linux__ && !__FreeBSD__
   # include <features.h>
   # if !(__GLIBC__ > 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ >= 25)
      static_assert(false, "Unsupported target libc (AKA C runtime) detected");
   # endif
# endif

#if !__ELF__
  static_assert(false, "Only ELF-based targets are supported for C++ plugin functionality");
#endif
