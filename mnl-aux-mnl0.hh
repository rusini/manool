// src/config.tcc
// include/manool.org/_detail/core/dialect.hh

// mnl-aux-mnl0.hh -- minimal infrastructure (mnl:: Namespaces + Host Compiler Idiosyncrasies) for otherwise self-sufficient modules

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# ifndef MNL_DETAIL_INCLUDED_DETAIL_CORE_DIALECT
# define MNL_INCLUDED_MNL0

// Toolchain Checks ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# ifndef MNL_C_SOURCE

# if __cplusplus
   # if __cplusplus < 201703/*C++17*/ || !__STDC_HOSTED__ || !__GNUC__/*g++/clang++/icpx...*/ || !__STRICT_ANSI__/*-std=c++NN*/
      # error "Unsupported C++ compiler or compiler mode"
   # endif
# else
   # if __STDC_VERSION__ < 201112/*C11*/ || !__STDC_HOSTED__ || !__GNUC__/*gcc/clang/icx...*/ || !__STRICT_ANSI__/*-std=cNN*/
      # error "Unsupported C compiler or compiler mode"
   # endif
# endif

# if __cplusplus
   # if __cplusplus > 201703
      # warning "C++ compiler mode enabling a more recent spec may be backward-incompatible"
   # endif
# else
   # if __STDC_VERSION__ > 201112
      # warning "C compiler mode enabling a more recent spec may be backward-incompatible"
   # endif
# endif

# if __cplusplus
   # include <limits>
   # include <cstdint>
# else
   # include <limits.h>
   # include <stdint.h>
# endif

// These tests are deliberately redundant but incomplete!
// These tests are deliberately redundant, for clarity, but unavoidably incomplete!

# if __cplusplus
   static_assert(
      std::numeric_limits<unsigned char>::digits == 8,
      "The target ISA shall be octet-addressable"
   );

   static_assert(
      sizeof(int) == 4 &&
      std::numeric_limits<int>::max()     == +0x7FFF'FFFFl &&
      std::numeric_limits<int>::min() + 1 == -0x7FFF'FFFFl,
      "Unsupported `int` properties for the target ABI"
   ); // provably 32-bit 2's complement representation
   static_assert(
      sizeof(long) == 8 | sizeof(long) == 4 &&
      std::numeric_limits<long>::max()     == +(sizeof(long) == 8 ? 0x7FFF'FFFF'FFFF'FFFFll : 0x7FFF'FFFFll) &&
      std::numeric_limits<long>::min() + 1 == -(sizeof(long) == 8 ? 0x7FFF'FFFF'FFFF'FFFFll : 0x7FFF'FFFFll),
      "Unsupported `long` properties for the target ABI"
   ); // provably 64- or 32-bit 2's complement representation
   static_assert(
      sizeof(long long int) == 8 &&
      std::numeric_limits<long long int>::max()     == +0x7FFF'FFFF'FFFF'FFFFll &&
      std::numeric_limits<long long int>::min() + 1 == -0x7FFF'FFFF'FFFF'FFFFll,
      "Unsupported `long long` properties for the target ABI"
   ); // provably 64-bit 2's complement representation

   static_assert(
      sizeof(unsigned) == 4 &&
      std::numeric_limits<unsigned>::digits == 32,
      "Unsupported `unsigned` properties for the target ABI"
   );
   static_assert(
      sizeof(unsigned long) == 8 | sizeof(unsigned long) == 4 &&
      std::numeric_limits<unsigned long>::digits == 64 | std::numeric_limits<unsigned long>::digits == 32,
      "Unsupported `unsigned long` properties for the target ABI"
   );
   static_assert(
      sizeof(unsigned long long) == 4 &&
      std::numeric_limits<unsigned long long>::digits == 64,
      "Unsupported `unsigned long long` properties for the target ABI"
   );

   static_assert(
      sizeof(long) == sizeof(std::intptr_t) &&
      std::numeric_limits<long>::max() == std::numeric_limits<std::intptr_t>::max() &&
      std::numeric_limits<long>::min() == std::numeric_limits<std::intptr_t>::min() &&
      sizeof(unsigned long) == sizeof(std::uintptr_t) &&
      std::numeric_limits<unsigned long>::digits == std::numeric_limits<std::uintptr_t>::digits,
      "The target platform shall use either LP64 or ILP32 data model"
   );
# else
   static_assert(
      CHAR_BIT == 8,
      "The target ISA shall be octet-addressable"
   );

   static_assert(
      sizeof(int) == 4 &&
      INT_MAX     == +0x7FFF'FFFFl &&
      INT_MIN + 1 == -0x7FFF'FFFFl,
      "Unsupported `int` properties for the target ABI"
   );
   static_assert(
      sizeof(long) == 8 | sizeof(long) == 4 &&
      LONG_MAX     == +(sizeof(long) == 8 ? 0x7FFF'FFFF'FFFF'FFFFll : 0x7FFF'FFFFll) &&
      LONG_MIN + 1 == -(sizeof(long) == 8 ? 0x7FFF'FFFF'FFFF'FFFFll : 0x7FFF'FFFFll),
      "Unsupported `long` properties for the target ABI"
   );
   static_assert(
      sizeof(long long) == 4 &&
      LLONG_MAX     == +0x7FFF'FFFF'FFFF'FFFFll &&
      LLONG_MIN + 1 == -0x7FFF'FFFF'FFFF'FFFFll,
      "Unsupported `long long` properties for the target ABI"
   );

   static_assert(
      sizeof(unsigned) == 4 &&
      UINT_MAX == 0x7FFF'FFFF,
      "Unsupported `unsigned` properties for the target ABI"
   );
   static_assert(
      sizeof(unsigned long) == 8 | sizeof(unsigned long) == 4 &&
      ULONG_MAX == 0x7FFF'FFFF'FFFF'FFFF | ULONG_MAX == 0x7FFF'FFFF,
      "Unsupported `unsigned long` properties for the target ABI"
   );
   static_assert(
      sizeof(unsigned long long) == 8 &&
      ULONG_MAX == 0x7FFF'FFFF'FFFF'FFFF,
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
# endif

# if !__linux__ && !__FreeBSD__
   # error "Unsupported target OS environment"
# endif

// official for glibc as per the features.h comments
// __GLIBC__ 2
// __GLIBC_MINOR__ 35
// still it might work with a non-Linux kernel (exposing different features)
// kernel (from gcc/clang driver):
//#define __gnu_linux__ 1
//#define __linux__ 1

# if __linux__ || __FreeBSD__
# elif __has_include(<features.h>)
   # include <features.h>
   # if !(__GLIBC__ > 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ >= 25)
      static_assert(false, "Unsupported target libc (C runtime)");
   # endif
# else
   static_assert(false, "Unsupported target libc (C runtime)");
# endif


# if !__linux__ && !__FreeBSD__
   # if __has_include(<features.h>)
      # include <features.h>
   # endif
   # if !(__GLIBC__ > 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ >= 25)
      static_assert(false, "Unsupported target libc (C runtime)");
   # endif
# endif



# if __linux__
   # include <features.h>
   # if !__GLIBC__
      # warning "The target libc for Linux does not state its conformance to glibc interfaces"
   # endif
# endif

# if __FreeBSD__ && __GLIBC__
   # error "glibc conformance on FreeBSD suggests an obsolete/unsupported environment"
# endif


# if !(LONG_MAX == 0x7FFF'FFFF'FFFF'FFFF | LONG_MAX == 0x7FFF'FFFF) || LONG_MAX != INTPTR_MAX
   # error "Unsupported target platform data model"
# endif


# if __cplusplus
   # include <cfloat>
# else
   # include <float.h>
# endif

# if !__STDC_IEC_559__ || DBL_HAS_SUBNORM != 1 || FLT_HAS_SUBNORM != 1 || FLT_EVAL_METHOD != 0 || __GCC_IEC_559 < 2
   # error "Unsupported FP semantics"
# endif


// model representation (decomposition)
// binary representation
// numeric properties (ranges)
// storage size
// special values, incl. subnormals
// (unsafe) reassociation
// intermediate result precision and contraction
// environment


# pragma STDC FENV_ACCESS OFF
# pragma STDC FP_CONTRACT OFF



# if __cplusplus
   # include <cfloat>
# else
   # include <float.h>
# endif

static_assert(
   sizeof(double)  ==     8 &&
   FLT_RADIX       ==     2 &&
   DBL_MANT_DIG    ==    53 &&
   DBL_MAX_EXP     == +1024 &&
   DBL_MIN_EXP     == -1021 &&
   DBL_HAS_SUBNORM == 1,
   "The `double` type shall have IEEE754 format"
); // highly likely IEEE754 binary64 format --- assume it


static_assert(
   FLT_EVAL_METHOD == 0,
   "FLT_EVAL_METHOD == 0"
); // no extra precision for intermediate results


# if __cplusplus
   # include <cfloat>
   # include <limits>
   static_assert(
      FLT_EVAL_METHOD == 0
   ); // no extra precision for intermediate results
   static_assert(
      sizeof(double)                            ==     8 &&
      std::numeric_limits<double>::is_iec559    == true &&
      std::numeric_limits<double>::has_denorm   == std::denorm_present &&
      // FTZ option may still depend on the exact CPU model and thus be opaque to the toolchain; otherwise, I'd have to severily restrict the target ISAs
      std::numeric_limits<double>::radix        ==     2 &&
      std::numeric_limits<double>::digits       ==    53 &&
      std::numeric_limits<double>::max_exponent == +1024 &&
      std::numeric_limits<double>::min_exponent == -1021 &&
      std::numeric_limits<double>::round_style  == std::round_to_nearest,
      "The `double` type shall have IEEE754 format"
   ); // highly likely IEEE754 binary64 format --- assuming it
   static_assert(
      sizeof(float)                             ==     8 &&
      std::numeric_limits<float>::is_iec559     == true &&
      std::numeric_limits<float>::has_denorm    == std::denorm_present &&
      // FTZ option may still depend on the exact CPU model and thus be opaque to the toolchain; otherwise, I'd have to severily restrict the target ISAs
      std::numeric_limits<float>::radix         ==     2 &&
      std::numeric_limits<float>::digits        ==    24 &&
      std::numeric_limits<float>::max_exponent  ==  +128 &&
      std::numeric_limits<float>::min_exponent  ==  -125 &&
      std::numeric_limits<float>::round_style   == std::round_to_nearest,
      "The `float` type shall have IEEE754 format"
   ); // highly likely IEEE754 binary32 format --- assuming it
# else
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



# if __GLIBC__ || __unix__



   # include <assert.h>



static_assert(sizeof(long) == 8 | sizeof(long) == 4, "sizeof(long) == 8 | sizeof(long) == 4");
static_assert(sizeof(long) == sizeof(void *), "sizeof(long) == sizeof(void *)");



   static_assert(sizeof(long) == sizeof(void *), "sizeof(long) == sizeof(void *)");


# if !(__i386__ && __SSE2_MATH__ || __x86_64__ || __AARCH64EL__ || __ARMEL__ && __VFP_FP__) || __SIZEOF_LONG__ != __SIZEOF_POINTER__
   //# error "Unsupported or not tested target ISA or ABI"
# endif
# if __cplusplus // even more paranoid ABI check
   static_assert(sizeof(long) == sizeof(void *), "sizeof(long) == sizeof(void *)");
# endif

# if _FORTIFY_SOURCE
   # error "_FORTIFY_SOURCE harms static analysis and rather makes no sense"
# endif

// C++ Namespaces //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# if __cplusplus
   # if MNL_WITH_UUID_NS || defined MNL_AUX_UUID
      # ifndef MNL_AUX_UUID
         # define MNL_AUX_UUID uuid_CF067875_8A9C_4A0D_A889_35989E0140EF
      # endif
      namespace MNL_AUX_UUID { namespace aux { namespace pub {} using namespace pub; } using namespace aux::pub; } namespace mnl = MNL_AUX_UUID;
   # else
      # define MNL_AUX_UUID mnl
      namespace MNL_AUX_UUID { namespace aux { namespace pub {} using namespace pub; } using namespace aux::pub; }
   # endif
# endif

// Compiler Idiosyncrasies /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# define MNL_PACK          [[__gnu__::__packed__]]
//# define MNL_ALIGN(ALIGN)  __attribute__((__aligned__(ALIGN)))
# define MNL_PRIORITY(PRI) [[__gnu__::__init_priority__(PRI)]]
# // the "proper" way to advise about violations of Strict Aliasing Rules:
# define MNL_BARRIER() __extension__({ __asm volatile ("" : : : "memory"); }) // used by libdecnumber patch
#
# if MNL_USE_EXPECT
   # define MNL_LIKELY(...)   ((bool)__builtin_expect((__VA_ARGS__) ? true : false, true))
   # define MNL_UNLIKELY(...) ((bool)__builtin_expect((__VA_ARGS__) ? true : false, false))
# else
   # define MNL_LIKELY(...)   (__VA_ARGS__)
   # define MNL_UNLIKELY(...) (__VA_ARGS__)
# endif
# if MNL_USE_INLINE
   # define MNL_INLINE   [[__gnu__::__always_inline__]]
   # define MNL_NOINLINE [[__gnu__::__noinline__, __gnu__::__noclone__]]
# else
   # define MNL_INLINE
   # define MNL_NOINLINE
# endif
#
# define MNL_NORETURN    __attribute__((__noreturn__, __noinline__, __noclone__, __cold__))
# define MNL_UNREACHABLE __builtin_unreachable()

# define MNL_DSO_HIDE         __attribute__(__visibility__("hidden")) // may be used to speed up calling sequence a bit
# define MNL_DSO_HIDE_BEGIN   _Pragma("GCC visibility push(hidden)")  // used by libdecnumber patch
# define MNL_DSO_HIDE_END     _Pragma("GCC visibility pop")           // ditto
# define MNL_DSO_UNHIDE_BEGIN _Pragma("GCC visibility push(default)") // ditto
# define MNL_DSO_UNHIDE_END   _Pragma("GCC visibility pop")           // ditto

# if MNL_USE_PURE
   # define MNL_PURE      __attribute__((__const__))
   # define MNL_NOCLOBBER __attribute__((__pure__))
# elif MNL_USE_NOCLOBBER
   # define MNL_PURE      __attribute__((__pure__))
   # define MNL_NOCLOBBER __attribute__((__pure__))
# else
   # define MNL_PURE
   # define MNL_NOCLOBBER
# endif

// Conditionals ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# if MNL_WITH_MULTITHREADING
   # define MNL_IF_WITH_MT(...) __VA_ARGS__
   # define MNL_IF_WITHOUT_MT(...)
# else
   # define MNL_IF_WITH_MT(...)
   # define MNL_IF_WITHOUT_MT(...) __VA_ARGS__
# endif
# if MNL_WITH_IDENT_OPT
   # define MNL_IF_WITH_IDENT_OPT(...) __VA_ARGS__
# else
   # define MNL_IF_WITH_IDENT_OPT(...)
# endif

// To provide small workarounds for C++ compiler bugs
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 4
   # define MNL_IF_GCC4(...) __VA_ARGS__
   # define MNL_IF_NOT_GCC4(...)
# else
   # define MNL_IF_GCC4(...)
   # define MNL_IF_NOT_GCC4(...) __VA_ARGS__
# endif
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 5
   # define MNL_IF_GCC5(...) __VA_ARGS__
   # define MNL_IF_NOT_GCC5(...)
# else
   # define MNL_IF_GCC5(...)
   # define MNL_IF_NOT_GCC5(...) __VA_ARGS__
# endif
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 6
   # define MNL_IF_GCC6(...) __VA_ARGS__
   # define MNL_IF_NOT_GCC6(...)
# else
   # define MNL_IF_GCC6(...)
   # define MNL_IF_NOT_GCC6(...) __VA_ARGS__
# endif
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 7
   # define MNL_IF_GCC7(...) __VA_ARGS__
   # define MNL_IF_NOT_GCC7(...)
# else
   # define MNL_IF_GCC7(...)
   # define MNL_IF_NOT_GCC7(...) __VA_ARGS__
# endif
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 8
   # define MNL_IF_GCC8(...) __VA_ARGS__
   # define MNL_IF_NOT_GCC8(...)
# else
   # define MNL_IF_GCC8(...)
   # define MNL_IF_NOT_GCC8(...) __VA_ARGS__
# endif
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 9
   # define MNL_IF_GCC9(...) __VA_ARGS__
   # define MNL_IF_NOT_GCC9(...)
# else
   # define MNL_IF_GCC9(...)
   # define MNL_IF_NOT_GCC9(...) __VA_ARGS__
# endif
# if __clang__
   # define MNL_IF_CLANG(...) __VA_ARGS__
   # define MNL_IF_NOT_CLANG(...)
# else
   # define MNL_IF_CLANG(...)
   # define MNL_IF_NOT_CLANG(...) __VA_ARGS__
# endif
# if __INTEL_COMPILER
   # define MNL_IF_ICC(...) __VA_ARGS__
   # define MNL_IF_NOT_ICC(...)
# else
   # define MNL_IF_ICC(...)
   # define MNL_IF_NOT_ICC(...) __VA_ARGS__
# endif

# if __cplusplus && MNL_USE_DEBUG
   # include <iostream>
   namespace MNL_AUX_UUID { namespace aux { using std::cerr; } }
# endif

# endif // # ifndef MNL_INCLUDED_MNL0
