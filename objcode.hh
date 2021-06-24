// objcode.hh -- run-time "assembler" for building up object code (kind of relocatable machine code), which is emitted
// by a JIT compiler code generator, and eventually loading it into target address as executable segment

/*    Copyright (C) 2021 Alexey Protasov (AKA Alex or rusini)

   This is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this software.  If not, see <https://www.gnu.org/licenses/>.  */


# ifndef RSN_INCLUDED_OBJCODE
# define RSN_INCLUDED_OBJCODE

# include <new>       // bad_alloc
# include <cassert>
# include <cstdlib>   // free, realloc
# include <cstring>   // memcpy
# include <limits>
# include <vector>
# include <algorithm> // max/min

# include "rusini0.hh"

namespace rsn {

   class objcode /*object code*/ { // with relocations suitable for the target ISA
   public:
      objcode() = default;
      objcode(objcode &&) = delete; // non-copyable and even non-movable
   private: // Internal Helper Types ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class _sect/*ion*/ {
      public:
         unsigned char *pc = {};         // section program-counter for code/data emission
         const unsigned char *base = {}; // start of buffer
         int res = 0, alloc = 0;         // requested and actual buffer size, in bytes (not exceeding 1 << max_segm_size_p2)
         int align = 1;                  // alignment requirements accumulated so far, a power of two in bytes (not exceeding 1 << cacheline_size_p2)
         const bool is_rodata;           // whether the section contains text (code) or read-only data
      public: // standard operations and construction
         RSN_INLINE _sect(_sect &&rhs) noexcept // only move-constructible - only ~_sect() is applicable after moving out
            : pc(rhs.pc), base(rhs.base), res(rhs.res), alloc(rhs.alloc), align(rhs.align), is_rodata(rhs.is_rodata) { rhs.base = {}; }
         RSN_INLINE ~_sect()
            { if (RSN_UNLIKELY(base)) std::free(const_cast<unsigned char *>(base)); } // own fast/slow path split
      public:
         RSN_INLINE explicit constexpr _sect(decltype(is_rodata) is_rodata) noexcept: is_rodata(is_rodata) {} // non-aggregate
      public: // opaque, first-class ID of a section in context of a specific (assumed) instance of objcode
         class id { // (this lacks an explicit reference to that instance to fully identify a section)
            friend objcode;
            int sn;
            RSN_INLINE explicit constexpr id(decltype(sn) sn) noexcept: sn(sn) {}
         public:
            explicit id() = default;
            static const id unspec;
         };
      public: // helper stuff
         struct fixup { // AKA relocation records - specific to x86 and x86-64 ISAs (suitable for x86 and all code models for x86-64)
            enum { plus_label_quad, plus_label_long, plus_label_minus_next_addr_long, plus_label_minus_next_addr_byte, minus_next_addr_long } kind;
            int sect/*s/n*/, offset;
            int label/*s/n*/; // relevant unless kind == minus_next_addr_long
         };
      };
      class _label {
      public:
         int sect/*s/n*/, offset;
      public: // opaque, first-class ID of a label in context of a specific (assumed) instance of objcode
         class id { // (this lacks an explicit reference to that instance to fully identify a label)
            friend objcode;
            int sn;
            RSN_INLINE explicit constexpr id(decltype(sn) sn) noexcept: sn(sn) {}
         public:
            explicit id() = default;
            static const id unspec;
         };
      };
   private: // data size nomenclature (specific to AT&T assembly language for x86 and x86-64 ISAs)
      struct RSN_PACK x86byte { unsigned char      _; };
      struct RSN_PACK x86word { unsigned short     _; };
      struct RSN_PACK x86long { unsigned           _; };
      struct RSN_PACK x86quad { unsigned long long _; };
   public:
      // Symbolic Addresses ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class label: public _label::id { // fully identifies a label
      public:
         objcode &owner; // makes the whole containing class non-assignable (that is, reference-alike itself), which is tolerable (otherwise, ID could be used instead)
         RSN_INLINE constexpr label(decltype(owner) owner, id _) noexcept: owner(owner), id(_) {}
      };
      // Program Text and (RO)Data Sections ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class sect/*ion*/: public _sect::id { // fully identifies a section
      public:
         objcode &owner; // makes the whole containing class non-assignable (that is, reference-alike itself), which is tolerable (otherwise, ID could be used instead)
         RSN_INLINE constexpr sect(decltype(owner) owner, id _) noexcept: owner(owner), id(_) {}
      public: // assembly memory allocation
         RSN_INLINE auto reserve(int size) const {
            assert(size >= 0);
            if (RSN_LIKELY((unsigned)owner._sects[sn].res + size <= owner._sects[sn].alloc))
               owner._sects[sn].res += size; // fast path
            else [](auto &sect, auto size)RSN_NOINLINE {
               if (RSN_UNLIKELY((unsigned)sect.res + size > 1 << max_segm_size_p2)) throw std::bad_alloc{};
               auto pc = (int)(sect.pc - sect.base);
               auto res = sect.res + size;
               auto base = static_cast<unsigned char *>(std::realloc(const_cast<unsigned char *>(sect.base),
                  (unsigned)std::min(res + res / 2, 1 << max_segm_size_p2)));
               if (RSN_UNLIKELY(!base)) throw std::bad_alloc{};
               sect.base = base, sect.pc = base + pc, sect.alloc = std::min(res + res / 2, 1 << max_segm_size_p2);
               sect.res = res;
            }(owner._sects[sn], size); // slow path
            return *this;
         }
      public: // appending section contents (specific to x86 and x86-64 ISAs)
         RSN_INLINE auto b(decltype(x86byte::_) val = {}) const noexcept
            { assert(size() + sizeof(x86byte) <= reserved()); ((x86byte *)owner._sects[sn].pc)->_ = val, owner._sects[sn].pc += sizeof(x86byte); return *this; }
         RSN_INLINE auto w(decltype(x86word::_) val = {}) const noexcept
            { assert(size() + sizeof(x86word) <= reserved()); ((x86word *)owner._sects[sn].pc)->_ = val, owner._sects[sn].pc += sizeof(x86word); return *this; }
         RSN_INLINE auto l(decltype(x86long::_) val = {}) const noexcept
            { assert(size() + sizeof(x86long) <= reserved()); ((x86long *)owner._sects[sn].pc)->_ = val, owner._sects[sn].pc += sizeof(x86long); return *this; }
         RSN_INLINE auto q(decltype(x86quad::_) val = {}) const noexcept
            { assert(size() + sizeof(x86quad) <= reserved()); ((x86quad *)owner._sects[sn].pc)->_ = val, owner._sects[sn].pc += sizeof(x86quad); return *this; }
         // sometimes it's convenient to store in BE format (for instruction encoding)
         RSN_INLINE auto sw(decltype(x86word::_) val) const noexcept { return w(__builtin_bswap16(val)); }
         RSN_INLINE auto sl(decltype(x86long::_) val) const noexcept { return l(__builtin_bswap32(val)); }
         RSN_INLINE auto sq(decltype(x86quad::_) val) const noexcept { return q(__builtin_bswap64(val)); }
         // misc convenience helpers for the above
         template<typename Type> RSN_INLINE auto l(Type *val) const noexcept { return l(reinterpret_cast<unsigned long>(val)); } // for 32-bit code models
         template<typename Type> RSN_INLINE auto q(Type *val) const noexcept { return q(reinterpret_cast<unsigned long>(val)); } // for 64-bit code models
         template<int Size> RSN_INLINE auto b(const char (&val)[Size]) const noexcept { for (int _ = 0; _ < Size; ++_) b(val[_]); return *this; }
      public:
         // symbolic and relative addresses
         RSN_INLINE auto q (class label label, decltype(x86quad::_) offset = 0) const { // for 64-bit code models
            return owner._fixups.push_back({_sect::fixup::plus_label_quad, sn,
               (int)(owner._sects[sn].pc - owner._sects[sn].base), label.sn}), q(offset);
         }
         RSN_INLINE auto l (class label label, decltype(x86long::_) offset = 0) const { // for 32-bit code models
            return owner._fixups.push_back({_sect::fixup::plus_label_long, sn,
               (int)(owner._sects[sn].pc - owner._sects[sn].base), label.sn}), l(offset);
         }
         RSN_INLINE auto rl(class label label, decltype(x86long::_) offset = 0) const {
            return owner._fixups.push_back({_sect::fixup::plus_label_minus_next_addr_long, sn,
               (int)(owner._sects[sn].pc - owner._sects[sn].base), label.sn}), l(offset); }
         RSN_INLINE auto rb(class label label, decltype(x86byte::_) offset = 0) const {
            return owner._fixups.push_back({_sect::fixup::plus_label_minus_next_addr_byte, sn,
               (int)(owner._sects[sn].pc - owner._sects[sn].base), label.sn}), b(offset);
         }
         RSN_INLINE auto rl(decltype(x86long::_) val) const { // for 32-bit code models
            return owner._fixups.push_back({_sect::fixup::minus_next_addr_long, sn,
               (int)(owner._sects[sn].pc - owner._sects[sn].base)}), l(val);
         }
         // convenience helpers for the above
         template<typename Type> RSN_INLINE auto rl(Type *val) const { return rl(reinterpret_cast<unsigned long>(val)); } // for 32-bit code models
      public: // address alignment (specific to x86 and x86-64 ISAs)
         RSN_NOINLINE auto align(int boundary, int max = 1 << cacheline_size_p2) const noexcept {
            assert(__builtin_popcount(boundary) == 1 && boundary <= 1 << cacheline_size_p2);
            assert(max >= 0 && (max <= boundary || max == 1 << cacheline_size_p2));
            assert(size() + std::min(boundary - 1, max) <= reserved());
            auto pad_size = (int)(owner._sects[sn].base - owner._sects[sn].pc & boundary - 1);
            if (RSN_LIKELY(pad_size > max)) return *this;
            if (RSN_UNLIKELY(owner._sects[sn].align < boundary)) owner._sects[sn].align = boundary;
            for (auto _ = pad_size / 10; _; --_) sw(0x662E).sq(0x0F1F84'00000000'00);
            switch (pad_size % 10) {
            case 0: return *this;
            case 1: return b(0x90);
            case 2: return sw(0x6690);
            case 3: return b(0x0F).sw(0x1F'00);
            case 4: return sl(0x0F1F40'00);
            case 5: return b(0x0F).sl(0x1F44'0000);
            case 6: return sw(0x660F).sl(0x1F44'0000);
            case 7: return b(0x0F).sw(0x1F80).sl(0x00000000);
            case 8: return sq(0x0F1F84'00000000'00);
            case 9: return b(0x66).sq(0x0F1F84'00000000'00);
            }
            RSN_UNREACHABLE();
         }
      public: // defining (placing) labels
         RSN_INLINE auto label(class label label, int offset = 0) const noexcept
            { assert(&label.owner == &owner); owner._labels[label.sn] = {sn, (int)(owner._sects[sn].pc - owner._sects[sn].base) + offset}; return *this; }
         // convenience helpers for the above
         RSN_INLINE auto label(int offset = 0) const { auto label = owner.label(); this->label(label, offset); return label; }
      public: // misc operations
         RSN_INLINE auto size() const noexcept { return (int)(owner._sects[sn].pc - owner._sects[sn].base); }
         RSN_INLINE auto reserved() const noexcept { return owner._sects[sn].res; }
      };
      // Target Memory Segment for Object Code Loading /////////////////////////////////////////////////////////////////////////////////////////////////////////
      class segm/*ent*/ { // dynamically allocated
      public:
         static long max_total_used, max_total_phys; // maximum totals without/with overhead, respectively
      public: // standard operations and primary constructors
         constexpr segm() noexcept: _base{}, _size{} {}
         RSN_INLINE segm(segm &&rhs) noexcept: _base(rhs._base), _size(rhs._size) { rhs._base = {}; } // movable-only
         RSN_INLINE ~segm() { if (RSN_UNLIKELY(_base)) _free(); }
         RSN_INLINE auto &operator=(segm &&rhs) noexcept { swap(rhs); return *this; } // movable-only
         RSN_INLINE void swap(segm &rhs) noexcept { std::swap(_base, rhs._base), std::swap(_size, rhs._size); }
      public:
         RSN_INLINE explicit segm(int size) { if (RSN_UNLIKELY(size)) _alloc(size); else _base = {}, _size = {}; }
      public: // access to contents
         template<typename Type> RSN_INLINE explicit operator Type *() const noexcept { return reinterpret_cast<Type *>(_base); }
         RSN_INLINE auto size() const noexcept { return _base ? _size : 0 /*branchless*/; }
      public: // misc operations
         RSN_INLINE segm(const objcode &rhs): segm(rhs.size()) { rhs.load(static_cast<unsigned char *>(*this)); }
         RSN_INLINE explicit segm(const segm &rhs): segm(rhs.size()) { _memcpy(static_cast<void *>(*this), static_cast<const void *>(rhs), size()); } // explicit-only
      private: // internal representation
         unsigned char *_base; int _size;
      private: // internal helper stuff
         void _alloc(int), _free() noexcept;
      };
      RSN_INLINE segm load() const { return *this; }
   public: /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      RSN_INLINE auto text()   { return sect(false); }
      RSN_INLINE auto rodata() { return sect(true); }
   public:
      RSN_INLINE class label label() {
         if (RSN_UNLIKELY((decltype(label::sn))_labels.size() == std::numeric_limits<decltype(label::sn)>::max())) throw std::bad_alloc{};
         _labels.emplace_back(); return {*this, label::id{(decltype(label::sn))_labels.size() - 1}};
      }
   public: // misc operations
      RSN_INLINE class sect sect(bool is_rodata) {
         if (RSN_UNLIKELY((decltype(sect::sn))_sects.size() == std::numeric_limits<decltype(sect::sn)>::max())) throw std::bad_alloc{};
         _sects.emplace_back(is_rodata); return {*this, sect::id{(decltype(sect::sn))_sects.size() - 1}};
      }
   public:
      int size() const;
      void load(unsigned char *) const;
   private: // internal representation
      std::vector<_sect>        _sects;
      std::vector<_sect::fixup> _fixups;
      std::vector<_label>       _labels;
   private: // internal helper stuff
      static constexpr auto
         cacheline_size_p2 =  6 /*64 B*/,  // for CPU L#i/L#d caches (typically 64 B for x86/x86-64 CPUs and many others)
         page_size_p2      = 12 /*4 KiB*/; // for MMU paging (typically 4 KiB for x86/x86-64 CPUs and many others)
      static constexpr auto
         max_segm_size_p2 = // maximum size of an executable segment
         # if __x86_64__ && __SIZEOF_POINTER__ == __SIZEOF_LONG_LONG__
            30 /*1 GiB*/
         # elif __i386__ || __x86_64__ && __SIZEOF_POINTER__ == __SIZEOF_INT__
            24 /*16 MiB*/
         # elif __AARCH64EL__ || __ARMEL__
            20 /*1 MiB*/
         # else
            # error "Unsupported or not tested target ISA or ABI"
            page_size_p2
         # endif
            ;
      static_assert(max_segm_size_p2 >= page_size_p2);
      static_assert(max_segm_size_p2 < std::numeric_limits<int>::digits);
   private:
      RSN_INLINE static void *_memcpy(void *lhs, const void *rhs, int size) noexcept
         { if (RSN_LIKELY(size)) std::memcpy(lhs, rhs, (unsigned)size); return lhs; }
   };

   RSN_INLINE inline void swap(objcode::segm &lhs, objcode::segm &rhs) noexcept { lhs.swap(rhs); }

} // namespace rsn::mnl

constexpr rsn::objcode::sect::id  rsn::objcode::sect::id::unspec(-1);
constexpr rsn::objcode::label::id rsn::objcode::label::id::unspec(-1);

# endif // # ifndef RSN_INCLUDED_OBJCODE
