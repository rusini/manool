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

rsn::objcode::segm::segm(const objcode &oc)
   : segm(oc.size()) { oc.load(static_cast<unsigned char *>(*this)); }

rsn::objcode::segm::segm(int size) {
   if (RSN_LIKELY(!size)) { base = {}, size_p2 = -1; return; } // trivial path - zero size is acceptable but results in zero address (and special size_p2)
   if (RSN_UNLIKELY(size > 1u << max_segm_size_p2)) throw std::bad_alloc{}; // redundant sanity check "not above nor negative"
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   static constexpr auto mmap_cached = [](int size) RSN_INLINE{
      assert(size > 0), assert(size <= 1 << mmap_threshold_p2);
      assert(size % (1 << page_size_p2) == 0);
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      static constexpr auto max_total_mmap_req = 54 /*slightly above 3 GiB*/;
      static int           total_mmap_req = 0; // cumulative number of ::mmap requests
      static int           total_mmap_pag = 0; // cumulative number of ::mmap-ed pages - normally grows by a factor (e.g., 3 / 2) with each succesful request
      static unsigned char *mmap_base     = 0; // starting address of the current ::mmap-ed block (aligned to page size)
      static long          mmap_size      = 0; // size of the current ::mmap-ed block, in bytes (multiple of page size)
      static long          munmap_size    = 0; // size of the last ::munmap-ed block; ditto
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if (RSN_LIKELY(size <= mmap_size)) { auto base = mmap_base; mmap_base += size, mmap_size -= size; return base; } // fast path
      if (RSN_UNLIKELY(mmap_size)) ::munmap(mmap_base, munmap_size = mmap_size);
      auto total_req = total_mmap_req;
      auto total_pag = RSN_UNLIKELY(!mmap_total_pag) ? 1 : mmap_total_pag + (mmap_total_pag + (2 - 1) >> 1) /*ceiling division by 2*/;
      while (total_req < max_total_mmap_req && ((long)total_pag - mmap_total_pag << page_size_p2) + munmap_size < size)
         ++total_req, total_pag = total_pag + (total_pag + (2 - 1) >> 1) /*ceiling division by 2*/;
      if (RSN_UNLIKELY((mmap_size = (long)total_pag - mmap_total_pag << page_size_p2) + munmap_size) < size)) mmap_size = 0, throw std::bad_alloc{};
      unsigned char *base = ::mmap(mmap_base, mmap_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, {});
      if (RSN_UNLIKELY(base == MAP_FAILED)) mmap_size = 0, throw std::bad_alloc{};
      mmap_total_req = total_req, mmap_total_pag = total_pag, mmap_base = base + size, mmap_size -= size, munmap_size = 0;
      return base;
   };
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   size_p2 = std::numeric_limits<decltype(size)>::digits + 1 - __builtin_clz(std::max(size, 1 << min_size_p2) - 1);
   if (RSN_LIKELY(free[size_p2 - min_size_p2])) // fast path
      free[size_p2 - min_size_p2] = static_cast<const meta *>(base = free[size_p2 - min_size_p2])->next, this->size_p2 = size_p2;
   else // slower path
   if (RSN_UNLIKELY(size_p2 >= page_size_p2))
      base = mmap(1 << size_p2), this->size_p2 = size_p2;
   else {
      auto base = this->base = mmap(1 << page_size_p2); this->size_p2 = size_p2;
      for (auto _ = 1 << page_size_p2 - size_p2; --_;) static_cast<meta *>(base += 1 << size_p2)->next =
         free[size_p2 - min_size_p2], free[size_p2 - min_size_p2] = base;
   }
}

