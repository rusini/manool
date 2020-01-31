// misc-memmgm.cc -- General Memory Management

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-misc-memmgm.hh"

# include <stdlib.h> // ::malloc, ::free
# if __linux__
   # include <malloc.h> // ::malloc_usable_size
# elif __FreeBSD__
   # include <malloc_np.h> // ::malloc_usable_size
# else
   # error "Either __linux__ or __FreeBSD__ is required"
# endif
# include <cerrno>
# include <cstdlib>  // size_t, _Exit, EXIT_FAILURE
# include <cstdio>   // stderr, fprintf, fflush, perror

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::size_t;  using std::_Exit; // <cstdlib>
      using std::fprintf; using std::fflush; using std::perror; // <cstdio>
   }

   MNL_IF_WITH_MT(thread_local) long long aux::stk_lim;
namespace aux { namespace {
   MNL_IF_WITH_MT(thread_local) long long stk_res;

   /*atomic*/ size_t heap_use;
   long long heap_lim = 1ll << 47;
   MNL_IF_WITH_MT(thread_local) long long heap_res;
}} // namespace aux::<unnamed>

   void pub::stk_limit (long long size) noexcept {
      unsigned long _; stk_lim = reinterpret_cast<long long>(&_) - size + stk_res;
   }
   void pub::heap_limit(long long size) noexcept {
      heap_lim = (long long)heap_use + size;
   }
   long long pub::stk_reserve (long long size) noexcept { // TODO: this probably should be inline
      auto saved_size = stk_res; (stk_lim -= stk_res) += size, stk_res = size; return saved_size;
   }
   long long pub::heap_reserve(long long size) noexcept { // TODO: this probably should be inline
      auto saved_size = heap_res; heap_res = size; return saved_size;
   }
   void aux::err_stk_overflow() {
      if (stk_res) stk_lim -= stk_res, stk_res = 0, throw stack_overflow{};
      fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Stack overflow (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
   }

namespace aux { namespace {
   MNL_INLINE inline void *_new(size_t size) {
      if (MNL_UNLIKELY(!size)) size = 1;
      auto ptr = ::malloc(size); // Note: the VM pages are still not committed here
      if (MNL_UNLIKELY(!ptr)) {
         if (size <= 1ll << 47 && (long long) MNL_IF_WITH_MT(__atomic_load_n(&heap_use, __ATOMIC_RELAXED))
            MNL_IF_WITHOUT_MT(heap_use) + (long long)size + (long long)sizeof(size_t) <= heap_lim - heap_res)
            perror("MANOOL: FATAL ERROR: Cannot malloc"), fflush(stderr), _Exit(EXIT_FAILURE);
         if (heap_res) heap_res = 0, throw heap_exhausted{};
         fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Heap exhausted (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
      }
   # if MNL_WITH_MULTITHREADING
      size = ::malloc_usable_size(ptr) + sizeof(size_t);
      auto _heap_use = __atomic_load_n(&heap_use, __ATOMIC_RELAXED);
      do if (MNL_UNLIKELY((long long)(_heap_use + size) > heap_lim - heap_res)) {
         ::free(ptr);
         if (heap_res) heap_res = 0, throw heap_exhausted{};
         fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Heap exhausted (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
      } while (MNL_UNLIKELY(!__atomic_compare_exchange_n(&heap_use, &_heap_use, _heap_use + size, {}, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
   # else
      if (MNL_UNLIKELY((long long)(heap_use + (size = ::malloc_usable_size(ptr) + sizeof(size_t))) > heap_lim - heap_res)) {
         ::free(ptr);
         if (heap_res) heap_res = 0, throw heap_exhausted{};
         fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Heap exhausted (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
      }
      heap_use += size;
   # endif // # if MNL_WITH_MULTITHREADING
      return ptr;
   }
   MNL_INLINE inline void *_new(size_t size, const std::nothrow_t &) noexcept {
      if (MNL_UNLIKELY(!size)) size = 1;
      auto ptr = ::malloc(size); // Note: the VM pages are still not committed here
      if (MNL_UNLIKELY(!ptr)) {
         if (size <= 1ll << 47 && (long long) MNL_IF_WITH_MT(__atomic_load_n(&heap_use, __ATOMIC_RELAXED))
            MNL_IF_WITHOUT_MT(heap_use) + (long long)size + (long long)sizeof(size_t) <= heap_lim - heap_res)
            perror("MANOOL: FATAL ERROR: Cannot malloc"), fflush(stderr), _Exit(EXIT_FAILURE);
         if (heap_res) return heap_res = 0, nullptr;
         fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Heap exhausted (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
      }
   # if MNL_WITH_MULTITHREADING
      size = ::malloc_usable_size(ptr) + sizeof(size_t);
      auto _heap_use = __atomic_load_n(&heap_use, __ATOMIC_RELAXED);
      do if (MNL_UNLIKELY((long long)(_heap_use + size) > heap_lim - heap_res)) {
         ::free(ptr);
         if (heap_res) return heap_res = 0, nullptr;
         fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Heap exhausted (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
      } while (MNL_UNLIKELY(!__atomic_compare_exchange_n(&heap_use, &_heap_use, _heap_use + size, {}, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));
   # else
      if (MNL_UNLIKELY((long long)(heap_use + (size = ::malloc_usable_size(ptr) + sizeof(size_t))) > heap_lim - heap_res)) {
         ::free(ptr);
         if (heap_res) return heap_res = 0, nullptr;
         fprintf(stderr, "MANOOL: FATAL ERROR: %s\n", "Heap exhausted (and no reservation is in effect)"), fflush(stderr), _Exit(EXIT_FAILURE);
      }
      heap_use += size;
   # endif // # if MNL_WITH_MULTITHREADING
      return ptr;
   }
   MNL_INLINE inline void _delete(void *ptr) noexcept {
      if (MNL_UNLIKELY(!ptr)) return;
      auto size = ::malloc_usable_size(ptr) + sizeof(size_t);
      ::free(ptr), MNL_IF_WITH_MT(__atomic_sub_fetch(&heap_use, size, __ATOMIC_RELAXED))MNL_IF_WITHOUT_MT(heap_use -= size);
   }
}} // namespace aux::<unnamed>

   const char *stack_overflow::what() const noexcept { return "mnl::stack_overflow"; }
   const char *heap_exhausted::what() const noexcept { return "mnl::heap_exhausted"; }

} // namespace MNL_AUX_UUID


void *operator new(std::size_t size) { return mnl::_new(size); }
void *operator new(std::size_t size, const std::nothrow_t &) noexcept { return mnl::_new(size, std::nothrow); }
void operator delete(void *ptr) noexcept { mnl::_delete(ptr); }
void operator delete(void *ptr, const std::nothrow_t &) noexcept { mnl::_delete(ptr); }

void *operator new[](std::size_t size) { return mnl::_new(size); }
void *operator new[](std::size_t size, const std::nothrow_t &) noexcept { return mnl::_new(size, std::nothrow); }
void operator delete[](void *ptr) noexcept { mnl::_delete(ptr); }
void operator delete[](void *ptr, const std::nothrow_t &) noexcept { mnl::_delete(ptr); }

