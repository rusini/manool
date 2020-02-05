// mnl-misc-memmgm.hh -- general memory management

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# ifndef MNL_INCLUDED_MEMMGM
# define MNL_INCLUDED_MEMMGM

# include "mnl-aux-mnl0.hh"

# include <exception> // exception
# include <new>       // bad_alloc

namespace MNL_AUX_UUID {

namespace aux {
   extern MNL_IF_WITH_MT(thread_local) long long stk_lim;
   MNL_NORETURN void err_stk_overflow();
} // namespace aux

namespace aux { namespace pub {
   void stk_limit(long long) noexcept, heap_limit(long long) noexcept /*NOT MT-safe!*/; // precond: size >= 0, size < 1ll << 47
   long long stk_reserve(long long) noexcept, heap_reserve(long long) noexcept;         // ditto
   MNL_INLINE inline void stk_check() { unsigned long _; if (MNL_UNLIKELY(reinterpret_cast<long long>(&_) < stk_lim)) err_stk_overflow(); }
   class stack_overflow: public std::exception { const char *what() const noexcept override; };
   class heap_exhausted: public std::bad_alloc { const char *what() const noexcept override; };
}} // namespace aux::pub

} // namespace MNL_AUX_UUID

#endif // # ifndef MNL_INCLUDED_MEMMGM
