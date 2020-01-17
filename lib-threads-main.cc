// lib-threads-main.cc

/*    Copyright (C) 2018, 2019 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# include "config.tcc"

# ifdef MNL_WITH_MULTITHREADING

# include <stdlib.h>
# include <stdio.h>
# include <pthread.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <utility>
# include <cstdlib>
# include <cstdio>
# include <thread>

# include "manool.hh"

extern "C" mnl::code mnl_main() {
   using std::_Exit;
   using std::fputs; using std::fflush;
   using std::move;
   using std::thread;
   using mnl::sym; using mnl::val; using mnl::test; using mnl::cast; using mnl::safe_cast;
   using mnl::make_lit; using mnl::expr_export;
   using mnl::stk_limit;

   static ::pthread_mutexattr_t mutex_attr;
   static bool mutex_attr_ready; // TODO: it seems that we will be called exactly once per each ::dlopen
   if (!mutex_attr_ready) {
      if (::pthread_mutexattr_init(&mutex_attr)) MNL_ERR(MNL_SYM("SystemError")); // TODO: finalization needed
      ::pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
      mutex_attr_ready = true;
   }

   static thread_local long long lock_count; // numeric overflow possible after tenths of years of continuous Mutex Acquire - ignore it

   struct mutex {
      ::pthread_mutex_t rep;
   public:
      mutex() {
         if (MNL_UNLIKELY(::pthread_mutex_init(&rep, &mutex_attr))) MNL_ERR(MNL_SYM("SystemError"));
      }
      ~mutex() {
         if (MNL_UNLIKELY(::pthread_mutex_trylock(&rep)) || MNL_UNLIKELY(!(::pthread_mutex_unlock(&rep), ::pthread_mutex_unlock(&rep))))
            fputs("MANOOL: FATAL ERROR: Mutex falls out of scope while its ownership is still held\n", stderr),
               fflush(stderr), _Exit(EXIT_FAILURE);
         ::pthread_mutex_destroy(&rep);
      }
   public:
      mutex(mutex &&): mutex{} {}
      mutex &operator=(const mutex &) = delete;
   public:
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         switch (MNL_DISP("Acquire", "Release")[op]) {
         case 1:
            if (MNL_LIKELY(argc == 0) || MNL_UNLIKELY(argc == 1) && !MNL_LIKELY(safe_cast<bool>(argv[0]))) { // Acquire[M], Acquire[M; False]
               if (MNL_UNLIKELY(::pthread_mutex_lock(&rep))) MNL_ERR(MNL_SYM("MutexAcquireError"));
               ++lock_count; return {};
            }
            if (MNL_LIKELY(argc == 1)) // Acquire[True] - nonblocking
            switch (::pthread_mutex_trylock(&rep)) {
            case 0:     return ++lock_count, true;
            case EBUSY: return false;
            default:    MNL_ERR(MNL_SYM("MutexAcquireError"));
            }
            MNL_ERR(MNL_SYM("InvalidInvocation"));
         case 2:
            if (MNL_LIKELY(argc == 0) || MNL_UNLIKELY(argc == 1) && !MNL_LIKELY(safe_cast<bool>(argv[0]))) { // Release[M], Release[M; False]
               if (MNL_UNLIKELY(::pthread_mutex_unlock(&rep))) MNL_ERR(MNL_SYM("MutexReleaseError"));
               --lock_count; return {};
            }
            if (MNL_LIKELY(argc == 1)) { // Release[True] - complete
               long long count = 1;
               if (MNL_UNLIKELY(::pthread_mutex_unlock(&rep))) MNL_ERR(MNL_SYM("MutexReleaseError"));
               --lock_count; while (!::pthread_mutex_unlock(&rep)) ++count, --lock_count; return count;
            }
            MNL_ERR(MNL_SYM("InvalidInvocation"));
         }
         return self.default_invoke(op, argc, argv);
      }
   };

   struct cond {
      val cond_mutex;
      ::pthread_cond_t rep;
      ::pthread_mutex_t aux_mutex;
   public:
      cond(val cond_mutex): cond_mutex(move(cond_mutex)) {
         if (MNL_UNLIKELY(::pthread_cond_init(&rep, {})) || MNL_UNLIKELY(::pthread_mutex_init(&aux_mutex, {}))) MNL_ERR(MNL_SYM("SystemError"));
      }
      ~cond() {
         ::pthread_mutex_destroy(&aux_mutex), ::pthread_cond_destroy(&rep); // no errors expected
      }
   public:
      cond(cond &&rhs): cond{move(rhs.cond_mutex)} {}
      cond &operator=(const cond &) = delete;
   public:
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         switch (MNL_DISP("Wait", "Notify")[op]) {
         case 1:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            {  long long count = 1;
               ::pthread_mutex_lock(&aux_mutex);
               if (MNL_UNLIKELY(::pthread_mutex_unlock(&cast<mutex &>(cond_mutex).rep))) MNL_ERR(MNL_SYM("CondWaitError"));
               while (!::pthread_mutex_unlock(&cast<mutex &>(cond_mutex).rep)) ++count; // complete unlock
               ::pthread_cond_wait(&rep, &aux_mutex); // no errors expected
               ::pthread_mutex_unlock(&aux_mutex);
               while (count--) ::pthread_mutex_lock(&cast<mutex &>(cond_mutex).rep);
            }
            return {};
         case 2:
            if (MNL_UNLIKELY(argc == 0)) return pthread_cond_signal(&rep), nullptr;
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return (MNL_LIKELY(safe_cast<bool>(argv[0])) ? ::pthread_cond_broadcast : ::pthread_cond_signal)(&rep), nullptr;
         }
         return self.default_invoke(op, argc, argv);
      }
   };

   struct proc_StartThread {
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) const {
         if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         {  static struct _ { MNL_INLINE _() noexcept {
               struct ::rlimit rl; ::getrlimit(RLIMIT_STACK, &rl);
               if (MNL_UNLIKELY(rl.rlim_cur == RLIM_INFINITY)) // maybe specific to Linux NPTL
                  fputs("MANOOL: WARNING: 'ulimit -s unlimited' is not recommended for multithreaded code\n", stderr);
            }} _;
         }
         auto start = move(argv[0]);
         thread{[=]{
            stk_limit(::strtoll(::getenv("MNL_STACK"), {}, {}));
            try { move(start)(); } MNL_CATCH_UNEXPECTED
            if (MNL_UNLIKELY(lock_count))
               fputs("MANOOL: FATAL ERROR: Terminating thread abandons a mutex while still holding ownership\n", stderr), fflush(stderr), _Exit(EXIT_FAILURE);
         }}.detach();
         return {};
      }
   };
   struct proc_MakeMutex {
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) const {
         if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return mutex{};
      }
   };
   struct proc_MakeCond {
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) const {
         if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<mutex>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return cond{move(argv[0])};
      }
   };

   return expr_export{
      {"StartThread", make_lit(proc_StartThread{})},
      {"MakeMutex",   make_lit(proc_MakeMutex{})},
      {"MakeCond",    make_lit(proc_MakeCond{})},
   };
}

# else
# include "manool.hh"
extern "C" mnl::code mnl_main() { return mnl::expr_export{}; }
# endif // # ifdef MNL_WITH_MULTITHREADING
