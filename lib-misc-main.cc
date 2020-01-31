// lib-misc-main.cc -- Interesting facilities to experiment with

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"

# include <stdlib.h>
# include <time.h>

# include "manool.hh"
# include "mnl-lib-ieee754-dec.hh"

extern "C" mnl::code mnl_main() {
   using mnl::sym; using mnl::val; using mnl::test; using mnl::cast;
   using mnl::make_lit; using mnl::expr_export;
   using mnl::dec;

   struct proc_Random { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      static MNL_IF_WITH_MT(thread_local) unsigned short state[]
         {(unsigned short)(::time({}) >> 16), (unsigned short)::time({}), (unsigned short)::clock()};
      return ::erand48(state);
   }};
   struct proc_Clock  { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      struct ::timespec ts;
      ::clock_gettime(
   # if __linux__
      CLOCK_MONOTONIC_RAW
   # elif __FreeBSD__
      CLOCK_MONOTONIC_PRECISE
   # else
      # error "Either __linux__ or __FreeBSD__ is required"
   # endif
      , &ts);
      char buf[sizeof "18446744073709551616.111222333"];
      return (dec<128>)(sprintf(buf, "%llu.%09ld", (unsigned long long)ts.tv_sec, ts.tv_nsec), buf);
   }};
   struct proc_Delay  { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
      struct ::timespec ts
         {::time_t(cast<long long>(argv[0]) / 1000000000llu), long(cast<long long>(argv[0]) % 1000000000llu)};
      ::nanosleep(&ts, {}); return {};
   }};
   return expr_export{
      {"Random", make_lit(proc_Random{})},
      {"Clock",  make_lit(proc_Clock{})},
      {"Delay",  make_lit(proc_Delay{})},
   };
}
