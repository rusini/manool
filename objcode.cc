// objcode.cc -- run-time assembler to build up object code (kind of relocatable machine code,
// which is emitted by a JIT compiler code generator) and eventually load it into RAM for execution

/*    Copyright (C) 2021 Alexey Protasov (AKA Alex or rusini)

   This is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this software.  If not, see <https://www.gnu.org/licenses/>.  */


# include "objcode.hh"

int rsn::objcode::size() const noexcept {
   int pc = 0;
   bool has_rodata = false;
   for (const auto &sect: sects) if (RSN_UNLIKELY(sect.is_rodata)) has_rodata = true; else {
      if (RSN_UNLIKELY((unsigned)(pc = pc + sect.align - 1 & -sect.align) +
         (int)(sect.pc - sect.base) > 1 << max_segm_size_p2)) return -1;
      pc += (int)(sect.pc - sect.base);
   }
   if (RSN_LIKELY(!has_rodata)) return pc;
   pc = pc + ((1 << cacheline_size_p2) - 1) & -(1 << cacheline_size_p2);
   for (const auto &sect: sects) if (RSN_UNLIKELY(sect.is_rodata)) {
      if (RSN_UNLIKELY((unsigned)(pc = pc + sect.align - 1 & -sect.align) +
         (int)(sect.pc - sect.base) > 1 << max_segm_size_p2)) return -1;
      pc += (int)(sect.pc - sect.base);
   }
   return pc;
}

void rsn::objcode::load(unsigned char *RSN_RESTRICT base) const noexcept {
   [&]() noexcept RSN_INLINE{ // transfer contents of sections to target load address
      int pc = 0;
      bool has_rodata = false;
      for (auto &sect: sects) if (RSN_UNLIKELY(sect.is_rodata)) has_rodata = true; else {
         int size = sect.pc - sect.base;
         sect.load_base = static_cast<unsigned char *>(std::memcpy(base +
            (unsigned)(pc = pc + sect.align - 1 & -sect.align), sect.base, (unsigned)size)), pc += size;
      }
      if (RSN_LIKELY(!has_rodata)) return;
      pc = pc + (1 << cacheline_size_p2) - 1 & -(1 << cacheline_size_p2);
      for (auto &sect: sects) if (RSN_UNLIKELY(sect.is_rodata)) {
         int size = sect.pc - sect.base;
         sect.load_base = static_cast<unsigned char *>(std::memcpy(base +
            (unsigned)(pc = pc + sect.align - 1 & -sect.align), sect.base, (unsigned)size)), pc += size;
      }
   }();
   for (auto fixup: fixups) switch (fixup.kind) { // apply fixup relocations to run-time memory contents
      case _sect::fixup::plus_label_quad: // useful for 64-bit ABIs
         reinterpret_cast<x86quad *>(sects[fixup.sect].load_base + fixup.offset)->_ +=
            reinterpret_cast<unsigned long>(sects[labels[fixup.label].sect].load_base + labels[fixup.label].offset);
         continue;
      case _sect::fixup::plus_label_long: // useful for 32-bit ABIs
         reinterpret_cast<x86long *>(sects[fixup.sect].load_base + fixup.offset)->_ +=
            reinterpret_cast<unsigned long>(sects[labels[fixup.label].sect].load_base + labels[fixup.label].offset);
         continue;
      case _sect::fixup::plus_label_minus_next_addr_long:
         reinterpret_cast<x86long *>(sects[fixup.sect].load_base + fixup.offset)->_ +=
            reinterpret_cast<unsigned long>(sects[labels[fixup.label].sect].load_base + labels[fixup.label].offset) -
            reinterpret_cast<unsigned long>(sects[fixup.sect].load_base + fixup.offset + sizeof(x86long));
         continue;
      case _sect::fixup::plus_label_minus_next_addr_byte:
         reinterpret_cast<x86byte *>(fixup.sects[fixup.sect].load_base + fixup.offset)->_ +=
            reinterpret_cast<unsigned long>(sects[labels[fixup.label].sect].load_base + labels[fixup.label].offset) -
            reinterpret_cast<unsigned long>(sects[fixup.sect].load_base + fixup.offset + sizeof(x86byte));
         continue;
      case _sect::fixup::minus_next_addr_long: // useful for 32-bit ABIs
         reinterpret_cast<x86long *>(sects[fixup.sect].load_base + fixup.offset)->_ -=
            reinterpret_cast<unsigned long>(sects[fixup.sect].load_base + fixup.offset + sizeof(x86long));
         continue;
      default: RSN_UNREACHABLE();
   }
}

namespace rsn {
   namespace {
      static constexpr auto
         min_size_p2    = 1 + 6      /*128 B - two cache lines       */,
         threshold_1_p2 = 1 + 2 + 10 /*  8 KiB - up to ~14x overhead */, // if size is above, use ::madvise to release unneeded physical storage
         threshold_2_p2 = 8 + 10   /*256 KiB - up to ~16 Ki mmaps  */; // if size is above, delegate to ::mmap/::munmap directly
      unsigned char *free[threshold_2_p2 - min_size_p2 + 1];
      struct free { unsigned char *next; };
      long total_used, total_phys;
      std::mutex mutex;
   }
}

void rsn::objcode::segm::_alloc(int size) {
   if (RSN_UNLIKELY(size > 1u << max_segm_size_p2)) // redundant sanity check "not above nor negative"
      throw std::bad_alloc{};
   static constexpr auto mmap = [](int size) RSN_INLINE{
      static unsigned char *mmap_base;
      static int mmap_size;
      if (RSN_UNLIKELY(size > mmap_size)) [](int size) RSN_NOINLINE{
         static int munmap_size;
         if (RSN_UNLIKELY(mmap_size)) ::munmap(mmap_base, munmap_size = mmap_size);
         static constexpr auto mmap_delta = sizeof(void *) == sizeof(long long) ? 24 << 10 << 10 /*MiB*/ :
            sizeof(void *) == sizeof(int) ? 384 << 10 /*KiB*/ : -1;
         static_assert(mmap_delta % (1 << page_size_p2) == 0);
         auto base = (unsigned char *)::mmap(mmap_base, (mmap_size = RSN_UNLIKELY(size <= munmap_size) ?
            munmap_size : (size - munmap_size + mmap_delta - 1) / mmap_delta * mmap_delta + munmap_size),
            PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, {});
         if (RSN_UNLIKELY(base == MAP_FAILED)) mmap_size = 0, throw std::bad_alloc{};
         munmap_size = 0, mmap_base = base;
      }(size); // slow path
      auto base = mmap_base;
      return mmap_base += size, mmap_size -= size, base; // fast path
   };
   if (RSN_LIKELY(size <= 1 << threshold_1_p2)) {
      static_assert(threshold_1_p2 >= page_size_p2);
      int populate_size;
      auto size_p2 = std::numeric_limits<unsigned>::digits - __builtin_clz(std::max(size, 1 << min_size_p2) - 1);
      static_assert(min_size_p2 >= cacheline_size_p2); static_assert(min_size_p2 < page_size_p2);
      RSN_IF_WITH_MT([&](auto) RSN_INLINE){
         if (RSN_UNLIKELY(total_used + size > max_total_used))
            throw std::bad_alloc{};
         if (RSN_LIKELY(free[size_p2 - min_size_p2])) { // fast path
            populate_size = 0;
            free[size_p2 - min_size_p2] = reinterpret_cast<const struct free *>(_base = free[size_p2 - min_size_p2])->next;
         } else
         if (RSN_UNLIKELY(size_p2 >= page_size_p2)) {
            if (RSN_UNLIKELY(total_phys + (populate_size = 1 << size_p2) > max_total_phys)) throw std::bad_alloc{};
            _base = mmap(1 << size_p2); total_phys += populate_size;
         } else {
            populate_size = 0;
            //if (RSN_UNLIKELY(total_phys + (1 << page_size_p2)) throw std::bad_alloc{};
            auto base = _base = mmap(1 << page_size_p2);
            for (auto _ = 1 << page_size_p2 - size_p2; --_;) ((struct free *)(base += 1 << size_p2))->next =
               free[size_p2 - min_size_p2], free[size_p2 - min_size_p2] = base;
            total_phys += 1 << page_size_p2;
         }
         total_used += _size = size;
      }RSN_IF_WITH_MT((std::lock_guard(mutex));)
      if (RSN_UNLIKELY(populate_size > 1 << page_size_p2)) ::madvise(_base, size, MADV_WILLNEED);
   } else
   if (RSN_LIKELY(size <= 1 << threshold_2_p2)) {
      static_assert(threshold_2_p2 >= page_size_p2);
      int populate_size;
      auto size_p2 = std::numeric_limits<unsigned>::digits - __builtin_clz(size - 1);
      RSN_IF_WITH_MT([&](auto) RSN_INLINE){
         if (RSN_UNLIKELY(total_used + size > max_total_used))
            throw std::bad_alloc{};
         if (RSN_LIKELY(free[size_p2 - min_size_p2])) { // fast path
            if (RSN_UNLIKELY(total_phys + (populate_size = size - 1 & -(1 << page_size_p2)) > max_total_phys)) throw std::bad_alloc{};
            free[size_p2 - min_size_p2] = reinterpret_cast<const struct free *>(_base = free[size_p2 - min_size_p2])->next;
            total_phys += populate_size;
         } else {
            if (RSN_UNLIKELY(total_phys + (populate_size = size + (1 << page_size_p2) - 1 & -(1 << page_size_p2)) > max_total_phys)) throw std::bad_alloc{};
            _base = mmap(1 << size_p2); total_phys += populate_size;
         }
         total_used += _size = size;
      }RSN_IF_WITH_MT((std::lock_guard(mutex));)
      if (RSN_UNLIKELY(populate_size > 1 << page_size_p2)) ::madvise(_base, size, MADV_WILLNEED);
   } else RSN_IF_WITH_MT([&](auto) RSN_INLINE){
      if ( RSN_UNLIKELY(total_used + size > max_total_used) ||
           RSN_UNLIKELY(total_phys + (size + (1 << page_size_p2) - 1 & -(1 << page_size_p2)) > max_total_phys) ) throw std::bad_alloc{};
      if ( RSN_UNLIKELY((_base = (unsigned char *)(::mmap({}, size, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, {}))) == MAP_FAILED) ) throw std::bad_alloc{};
      total_phys += size + (1 << page_size_p2) - 1 & -(1 << page_size_p2), total_used += _size = size;
   }RSN_IF_WITH_MT((std::lock_guard(mutex)));
}

void rsn::objcode::segm::_free() noexcept {
   if (RSN_LIKELY(_size <= 1 << threshold_1_p2)) {
      static_assert(threshold_1_p2 >= page_size_p2);
      auto size_p2 = std::numeric_limits<unsigned>::digits - __builtin_clz(std::max(_size, 1 << min_size_p2) - 1);
      static_assert(min_size_p2 >= cacheline_size_p2); static_assert(min_size_p2 < page_size_p2);
      RSN_IF_WITH_MT((void)std::lock_guard(mutex),)
         ((struct free *)(_base))->next = free[size_p2 - min_size_p2], free[size_p2 - min_size_p2] = _base,
         total_used -= 1 << size_p2;
   } else
   if (RSN_LIKELY(_size <= 1 << threshold_2_p2)) {
      static_assert(threshold_2_p2 >= threshold_1_p2);
      auto size_p2 = std::numeric_limits<unsigned>::digits - __builtin_clz(_size - 1);
      ::madvise(_base + (1 << page_size_p2), (1 << size_p2) - (1 << page_size_p2), MADV_DONTNEED),
      RSN_IF_WITH_MT((void)std::lock_guard(mutex),)
         ((struct free *)(_base))->next = free[size_p2 - min_size_p2], free[size_p2 - min_size_p2] = _base,
         total_used -= 1 << size_p2, total_phys -= (1 << size_p2) - (1 << page_size_p2);
   } else {
      auto size = _size + (1 << page_size_p2) - 1 & -(1 << page_size_p2);
      RSN_IF_WITH_MT((void)std::lock_guard(mutex),)
         ::munmap(_base, _size), total_used -= size, total_phys -= size;
   }
}

long rsn::objcode::segm::max_total_used = 256 * 1024 * 1024, rsn::objcode::segm::max_total_phys = 768 * 1024 * 1024;

