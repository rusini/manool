// mnl-aux-mnl0.hh -- Minimal Infrastructure (mnl:: namespaces + host compiler idiosyncrasies) for otherwise self-sufficient modules

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# pragma once

// Toolchain Checks ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# ifndef MNL_C_SOURCE
   # if __cplusplus < 201103/*C++11*/ || !__STDC_HOSTED__ || !__GNUC__/*g++/clang++/icpc...*/ || !__STRICT_ANSI__/*-std=c++NN*/
      # error "Unsupported C++ compiler or compiler mode"
   # endif
# else
   # if __STDC_VERSION__ < 199901/*C99*/ || !__STDC_HOSTED__ || !__GNUC__/*gcc/clang/icc...*/ || !__STRICT_ANSI__/*-std=cNN*/
      # error "Unsupported C compiler or compiler mode"
   # endif
# endif
# if !(__i386__ && __SSE2_MATH__ || __x86_64__ || __AARCH64EL__ || __ARMEL__ && __VFP_FP__) || __SIZEOF_LONG__ != __SIZEOF_POINTER__
   # error "Unsupported or not tested target ISA or ABI"
# endif
# if __cplusplus // even more paranoid ABI check
   static_assert(sizeof(long) == sizeof(void *), "sizeof(long) == sizeof(void *)");
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
# define MNL_PACK          __attribute__((__packed__))
# define MNL_ALIGN(ALIGN)  __attribute__((__aligned__(ALIGN)))
# define MNL_PRIORITY(PRI) __attribute__((__init_priority__(PRI)))
# // the "proper" way to advise about violations of Strict Aliasing Rules:
# define MNL_BARRIER() __extension__({ __asm volatile ("" : : : "memory"); }) // used by libdecnumber patch
#
# if MNL_USE_EXPECT
   # define MNL_LIKELY(...)   (!__builtin_expect(!(__VA_ARGS__), 0))
   # define MNL_UNLIKELY(...) (!__builtin_expect(!(__VA_ARGS__), 1))
# else
   # define MNL_LIKELY(...)   (__VA_ARGS__)
   # define MNL_UNLIKELY(...) (__VA_ARGS__)
# endif
# if MNL_USE_INLINE
   # define MNL_INLINE   __attribute__((__always_inline__))
   # define MNL_NOINLINE __attribute__((__noinline__))
# else
   # define MNL_INLINE
   # define MNL_NOINLINE
# endif
#
# define MNL_NORETURN      __attribute__((__noreturn__, __noinline__, __cold__))
# define MNL_UNREACHABLE() __builtin_unreachable()

# define MNL_DSO_HIDE         __attribute__(__visibility__("hidden")) // may be used to speed up calling sequence a bit
# define MNL_DSO_HIDE_BEGIN   _Pragma("GCC visibility push(hidden)")  // used by libdecnumber patch
# define MNL_DSO_HIDE_END     _Pragma("GCC visibility pop")           // ditto
# define MNL_DSO_UNHIDE_BEGIN _Pragma("GCC visibility push(default)") // ditto
# define MNL_DSO_UNHIDE_END   _Pragma("GCC visibility pop")           // ditto

# if MNL_USE_PURE
   # define MNL_PURE      __attribute__((__const__)) // currently have no use in MANOOL codebase but might be handy for plugin writers
   # define MNL_NOCLOBBER __attribute__((__pure__))  // ditto
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
# if !__clang__ && !__INTEL_COMPILER && __GNUC__ == 8
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
