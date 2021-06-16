// objcode.hh -- run-time "assembler" for building up object code (kind of relocatable machine code, which is emitted
// by a JIT compiler code generator) and eventually loading it into a target address as executable segment

/*    Copyright (C) 2021 Alexey Protasov (AKA Alex or rusini)

   This is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this software.  If not, see <https://www.gnu.org/licenses/>.  */


# include <cstdint>
# include <cstdlib>
# include <cstring>
# include <cstdio>
# include <vector>
# include <limits>
# include <algorithm>
# include <mutex>

# include <new> // bad_alloc

#include <sys/mman.h>

# include "rusini0.hh"

namespace rsn {

   class objcode /*object code*/ { // with relocations suitable for the target ISA
   private: // important magnitudes
      static constexpr auto
         cacheline_size_p2 =  6 /*64 B*/,                 // for CPU L#I/L#D caches (64 B for x86/x86-64 CPUs and many others)
         page_size_p2      = 12 /*4 KiB*/,                // for MMU paging (4 KiB for x86/x86-64 CPUs and many others)
         max_segm_size_p2  = page_size_p2 + 18 /*1 GiB*/; // maximum size of an executable segment
         /* max_segm_size_p2 maxval is half of positive space of int numbers
            (good for x86/x86-64 ISAs but ought to be much lower for RISC ISAs) */
      static_assert(max_segm_size_p2 < std::numeric_limits<int>::digits);
   private: // data size nomenclature (specific to AT&T assembly language for x86/x86-64 ISAs)
      struct RSN_PACK x86byte { unsigned char      _; };
      struct RSN_PACK x86word { unsigned short     _; };
      struct RSN_PACK x86long { unsigned           _; };
      struct RSN_PACK x86quad { unsigned long long _; };
   private: // Internal Helper Types ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class _sect/*ion*/ {
      public: // persistent members
         unsigned char *pc = {};         // section program counter for code emission
         const unsigned char *base = {}; // start of buffer
         int res = 0, alloc = 0;         // requested and actual buffer size (up to 1 << max_size_p2)
         int align = 1;                  // alignment requirements accumulated so far (up to 1 << cacheline_size_p2)
         const bool is_rodata;           // whether the section contains text (code) or read-only data
      public: // construction and standard operations
         RSN_INLINE _sect(bool is_rodata) noexcept: is_rodata(is_rodata) {}
         RSN_INLINE _sect(_sect &&rhs) noexcept: base(rhs.base), pc(rhs.pc), res(rhs.res), alloc(rhs.alloc), align(rhs.align), is_rodata(rhs.is_rodata) { rhs.base = {}; }
         RSN_INLINE ~_sect() { if (RSN_UNLIKELY(base)) std::free(const_cast<unsigned char *>(base)); } // own fast/slow path split
      public: // helper stuff
         struct fixup { // AKA relocation records - specific to x86 and x86-64 ISAs (suitable for x86 and all data and code models for x86-64)
            enum { plus_label_quad, plus_label_long, plus_label_minus_next_addr_long, plus_label_minus_next_addr_byte, minus_next_addr_long } kind;
            int sect/*s/n*/, offset;
            int label/*s/n*/; // kind != minus_next_addr_long
         };
      };
      struct _label {
         int sect/*s/n*/, offset;
      };
   public: // Symbolic Addresses ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class label {
         int sn;
         friend objcode;
      public:
         label(label &&) = default;
      public:
         RSN_INLINE explicit label(objcode &owner): sn(RSN_LIKELY(owner.labels.reserve((int)owner.labels.size() + 1),
            (int)owner.labels.size() != std::numeric_limits<int>::max()) ? owner.labels.size() : throw std::bad_alloc{}) { owner.labels.emplace_back(); }
      };
   public: // Program Text and (RO)Data Sections ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class sect/*ion*/ {
      public:
         objcode &owner;
      public: // assembly memory allocation
         RSN_INLINE auto &reserve(int size) const {
            //assert(size >= 0);
            if (RSN_LIKELY((unsigned)owner.sects[sn].res + size <= owner.sects[sn].alloc))
               owner.sects[sn].res += size; // fast path
            else [](auto &sect, auto size) RSN_NOINLINE{
               if (RSN_UNLIKELY((unsigned)sect.res + size > 1 << max_segm_size_p2)) throw std::bad_alloc{};
               static_assert(max_segm_size_p2 < std::numeric_limits<int>::digits);
               auto res = sect.res + size;
               auto pc = (int)(sect.pc - sect.base);
               auto alloc = std::max(std::min(sect.alloc + (sect.alloc + 2 - 1 >> 1), 1 << max_segm_size_p2), res);
               auto base = static_cast<unsigned char *>(std::realloc(const_cast<unsigned char *>(sect.base), alloc));
               if (RSN_UNLIKELY(!base)) throw std::bad_alloc{};
               sect.base = base, sect.pc = base + pc, sect.alloc = alloc;
               sect.res = res;
            }(owner.sects[sn], size); // slow path
            return *this;
         }
      public: // appending section contents (specific to x86 and x86-64 ISAs)
         RSN_INLINE auto &b(decltype(x86byte::_) val) const noexcept
            { ((x86byte *)owner.sects[sn].pc)->_ = val, owner.sects[sn].pc += sizeof(x86byte); return *this; }
         RSN_INLINE auto &w(decltype(x86word::_) val) const noexcept
            { ((x86word *)owner.sects[sn].pc)->_ = val, owner.sects[sn].pc += sizeof(x86word); return *this; }
         RSN_INLINE auto &l(decltype(x86long::_) val) const noexcept
            { ((x86long *)owner.sects[sn].pc)->_ = val, owner.sects[sn].pc += sizeof(x86long); return *this; }
         RSN_INLINE auto &q(decltype(x86quad::_) val) const noexcept
            { ((x86quad *)owner.sects[sn].pc)->_ = val, owner.sects[sn].pc += sizeof(x86quad); return *this; }
         // sometimes it's convenient to store in BE format (for instruction encoding)
         RSN_INLINE auto &sw(decltype(x86word::_) val) const noexcept { return w(__builtin_bswap16(val)); }
         RSN_INLINE auto &sl(decltype(x86long::_) val) const noexcept { return l(__builtin_bswap32(val)); }
         RSN_INLINE auto &sq(decltype(x86quad::_) val) const noexcept { return q(__builtin_bswap64(val)); }
         // misc convenience helpers for the above
         template<typename Type> RSN_INLINE auto &l(Type *val) const noexcept { return l(reinterpret_cast<unsigned long>(val)); } // for 32-bit code models
         template<typename Type> RSN_INLINE auto &q(Type *val) const noexcept { return q(reinterpret_cast<unsigned long>(val)); } // for 64-bit code models
      public:
         // symbolic and relative addresses
         RSN_INLINE auto &q (const label &label, decltype(x86quad::_) offset = 0) const { // for 64-bit code models
            return owner.fixups.push_back({_sect::fixup::plus_label_quad, sn,
               (int)(owner.sects[sn].pc - owner.sects[sn].base), label.sn}), q(offset);
         }
         RSN_INLINE auto &l (const label &label, decltype(x86long::_) offset = 0) const { // for 32-bit code models
            return owner.fixups.push_back({_sect::fixup::plus_label_long, sn,
               (int)(owner.sects[sn].pc - owner.sects[sn].base), label.sn}), l(offset);
         }
         RSN_INLINE auto &rl(const label &label, decltype(x86long::_) offset = 0) const {
            return owner.fixups.push_back({_sect::fixup::plus_label_minus_next_addr_long, sn,
               (int)(owner.sects[sn].pc - owner.sects[sn].base), label.sn}), l(offset); }
         RSN_INLINE auto &rb(const label &label, decltype(x86byte::_) offset = 0) const {
            return owner.fixups.push_back({_sect::fixup::plus_label_minus_next_addr_byte, sn,
               (int)(owner.sects[sn].pc - owner.sects[sn].base), label.sn}), b(offset);
         }
         RSN_INLINE auto &rl(decltype(x86long::_) val) const {
            return owner.fixups.push_back({_sect::fixup::minus_next_addr_long, sn,
               (int)(owner.sects[sn].pc - owner.sects[sn].base)}), l(val);
         }
         // convenience helpers for the above
         template<typename Type> RSN_INLINE auto &rl(Type *val) const { return rl(reinterpret_cast<unsigned long>(val)); } // for 32-bit code models
      public: // address alignment (specific to x86 and x86-64 ISAs)
         RSN_INLINE auto &align(int boundary, int max = 1 << cacheline_size_p2) const noexcept {
            auto pad_size = (int)(owner.sects[sn].base - owner.sects[sn].pc & boundary - 1);
            if (RSN_LIKELY(pad_size > max)) return *this;
            if (RSN_UNLIKELY(owner.sects[sn].align < boundary)) owner.sects[sn].align = boundary;
            for (auto _ = pad_size / 10; _; --_) sw(0x662E).sq(0x0F1F840000000000);
            switch (pad_size % 10) {
            case 0: return *this;
            case 1: return b(0x90);
            case 2: return sw(0x6690);
            case 3: return b(0x0F).sw(0x1F00);
            case 4: return sl(0x0F1F4000);
            case 5: return b(0x0F).sl(0x1F440000);
            case 6: return sw(0x660F).sl(0x1F440000);
            case 7: return b(0x0F).sw(0x1F80).sl(0x00000000);
            case 8: return sq(0x0F1F840000000000);
            case 9: return b(0x66).sq(0x0F1F840000000000);
            }
            RSN_UNREACHABLE();
         }
      public: // defining (placing) labels
         RSN_INLINE auto &label(class label &label, int offset = 0) const noexcept
            { owner.labels[label.sn] = {sn, (int)(owner.sects[sn].pc - owner.sects[sn].base) + offset}; return *this; }
         // convenience helper for the above
         RSN_INLINE auto pc(int offset = 0) const { auto res = owner.label(); label(res, offset); return res; }
      private: // internal representation
         int sn;
      public: // construction
         //sect(sect &&rhs) = default;
      public:
         RSN_INLINE explicit sect(objcode &owner, bool is_rodata = false): owner(owner), sn(RSN_LIKELY(owner.sects.reserve((int)owner.sects.size() + 1),
            (int)owner.sects.size() != std::numeric_limits<int>::max()) ? owner.sects.size() : throw std::bad_alloc{}) { owner.sects.emplace_back(is_rodata); }
      };
   public: /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RSN_INLINE sect text() { return sect(*this); }
      RSN_INLINE sect rodata() { return sect(*this, true); }
      RSN_INLINE class label label() { typedef class label _label; return _label(*this); }
      RSN_INLINE sect text(int size) { auto res = text(); res.reserve(size); return res; }
      RSN_INLINE sect rodata(int size) { auto res = rodata(); res.reserve(size); return res; }
   public:
      class segm/*ent*/ { // represents target memory segment for object code loading
      public: // standard operations
         RSN_INLINE segm() noexcept: _base{}, _size(0) {}
         RSN_INLINE segm(segm &&rhs) noexcept: _base(rhs._base), _size(rhs._size) { rhs._base = {}; }
         RSN_INLINE ~segm() { if (RSN_UNLIKELY(_base)) _free(); }
         RSN_INLINE auto &operator=(segm &&rhs) noexcept { swap(rhs); return *this; }
         RSN_INLINE void swap(segm &rhs) noexcept { using std::swap; swap(_base, rhs._base), swap(_size, rhs._size); }
      public: // misc constructors
         RSN_INLINE explicit segm(int size) { if (RSN_UNLIKELY(size)) _alloc(size); else _base = {}, _size = 0; }
         RSN_INLINE segm(const objcode &oc): segm(oc.size()) { oc.load((unsigned char *)(*this)); }
      public: // access to contents
         template<typename T> RSN_INLINE explicit operator T *() const noexcept { return reinterpret_cast<T *>(_base); }
         RSN_INLINE int size() const noexcept { return _size; }
      private: // internal representation
         unsigned char *_base; int _size;
      private: // implementation helpers
         void _alloc(int), _free() noexcept;
      private:
         static long max_total_used, max_total_phys;
      };
      RSN_INLINE inline segm load() const { return segm(*this); }
   public:
      int size() const noexcept;
      void load(unsigned char *) const noexcept;
   private: // internal representation
      std::vector<_sect> sects;
      std::vector<_sect::fixup> fixups;
      std::vector<_label> labels;
   };

} // namespace rsn::mnl

namespace rsn { RSN_INLINE inline void swap(objcode::segm &lhs, objcode::segm &rhs) noexcept { lhs.swap(rhs); } }

