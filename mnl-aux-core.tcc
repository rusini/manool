// mnl-aux-core.tcc -- MANOOL core

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "mnl-aux-mnl0.hh"

# include <typeinfo>
# include <cstdint>     // uintptr_t
# include <cstdlib>     // size_t, rand
# include <cmath>       // various...
# include <cstring>     // memcpy, memmove
# include <limits>      // numeric_limits
# include <type_traits> // various...
# include <utility>     // move, pair, make_pair, initializer_list, rel_ops
# include <memory>      // shared_ptr
# include <array>
# include <string>
# include <vector>
# include <map>
# include <set>
# if MNL_WITH_MULTITHREADING
   # include <mutex>
# endif

namespace MNL_AUX_UUID {
   namespace aux {
      using std::uintptr_t; // <cstdint>
      using std::size_t; using std::rand; // <cstdlib>
      using std::abs; using std::isinf; // <cmath>
      using std::memcpy; using std::memmove; // <cstring>
      using std::move; using std::pair; using std::make_pair; using std::initializer_list; // <utility>
      using std::shared_ptr; // <memory>
      using std::string; using std::vector; using std::array; using std::map; using std::set;
   }
   /* For atomics and memory order refer to:
      - http://en.cppreference.com/w/cpp/atomic/memory_order
      - http://preshing.com
      - http://www.boost.org/doc/libs/1_66_0/doc/html/atomic/usage_examples.html  */

// Utilities for Static Initialization /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# if false // most straightforward - triggers a compiler crash in clang++
   # define MNL_AUX_EARLY(...) []() noexcept->auto &{ \
      static constexpr auto init = []() noexcept{ return (__VA_ARGS__); }; \
      return ::mnl::aux::_early<init>; \
   }()
   namespace aux { template<const auto &Init> extern const auto _early = Init(); }
# else // works with both g++ and clang++
   # define MNL_AUX_EARLY(...) []() noexcept->auto &{ \
      static constexpr auto init_lambda = []() noexcept{ return (__VA_ARGS__); }; \
      static decltype(init_lambda()) (*const init_func)() noexcept = init_lambda; \
      return ::mnl::aux::_early<init_func>; \
   }()
   namespace aux { template<const auto &Init> extern const auto _early = Init(); }
# endif


   # define MNL_AUX_INIT(...) []()noexcept->const decltype(__VA_ARGS__) &{ \
      struct traits { MNL_INLINE static decltype(__VA_ARGS__) _() noexcept { return (__VA_ARGS__); } }; \
      return ::mnl::aux::_init<traits>::_; \
   }() // end # define MNL_AUX_INIT
namespace aux {
   template<typename Traits> struct _init { static const decltype(Traits::_()) _; };
   template<typename Traits> const decltype(Traits::_()) _init<Traits>::_ = Traits::_();
} // namespace aux
   # define MNL_AUX_RAND(VAL) MNL_AUX_INIT(::mnl::aux::_rand<VAL>())
namespace aux {
   template<typename Val> MNL_NOINLINE Val _rand() noexcept // TODO: conv to bool is problematic
      { static_assert(std::is_unsigned<Val>::value, "std::is_unsigned<Val>::value"); return (unsigned long)rand() * rand() * rand() * rand(); } // 60 bits
} // namespace aux

// Preliminary Declarations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   namespace aux { namespace pub { class val; } }
   template<typename> class box;
   namespace aux { template<int> struct _record; }
   namespace aux { namespace pub { template<bool> struct range; } }

   namespace aux { namespace pub { struct loc/*ation in source code*/ { shared_ptr<const string> origin; pair<int, int> _start, _final; }; } }
   namespace aux { template<size_t Argc> using args = array<val, Argc>; } // assume any g++-like compiler relaxes ISO/IEC 14882:2011 S8.5.1 P11 as in C++14

// class sym ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   class sym/*bol*/ {
   public: // Standard operations
      MNL_INLINE sym() noexcept: sym{decltype(rep){}} {}
      MNL_INLINE sym(const sym &rhs) noexcept: sym{rhs.rep} {}
      // [1] no sym(sym &&) noexcept;
      MNL_INLINE ~sym() { release(); }
      MNL_INLINE sym &operator=(const sym &rhs) noexcept { rhs.addref(), release(), rep = rhs.rep; return *this; }
      MNL_INLINE sym &operator=(sym &&rhs) noexcept { swap(rhs); return *this; }
      MNL_INLINE void swap(sym &rhs) noexcept { std::swap(rep, rhs.rep); }
      MNL_INLINE friend bool operator==(const sym &lhs, const sym &rhs) noexcept { return lhs.rep == rhs; }
      MNL_INLINE friend bool operator< (const sym &lhs, const sym &rhs) noexcept { auto mask = MNL_AUX_RAND(unsigned); return (lhs ^ mask) < (rhs ^ mask); }
      MNL_INLINE explicit operator bool() const noexcept { return rep; }
   public: // Construction and extraction via conversion
      sym(string), sym(const char *);
      explicit sym(decltype(nullptr));
      MNL_INLINE explicit operator const string &() const noexcept { return inverse[rep]->first; } // no sync required
      MNL_INLINE explicit operator const char *() const noexcept { return ((const string &)*this).c_str(); }
   public: // Functional application (and Repl)
      static constexpr int max_argc = 999;
   // Essential for performance
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         val operator()(const val &self, int argc, Val [], val *argv_out = {}) const; // argv_out[-1] corresponds to self; !argc < !argv
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         val operator()(val &&self,      int argc, Val [], val *argv_out = {}) const; // ditto
   // Essential for metaprogramming
      // For one argument
      val operator()(const val &) const, operator()(val &&) const;
      // For two arguments
      val operator()(const val &, const val &) const, operator()(const val &, val &&) const;
      val operator()(val &&, const val &) const, operator()(val &&, val &&) const;
      // For multiple arguments
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         val operator()(int argc, Val [], val *argv_out = {}) const;
      val sym::operator()(const val &a0, val a1, val a2) const, sym::operator()(val &&a0, val a1, val a2) const;
   // Convenience
      template<std::size_t Argc> val operator()(const val &self, std::array<val, Argc>, val *args_out = {}) const;
      template<std::size_t Argc> val operator()(val &&self, std::array<val, Argc>, val *args_out = {}) const;
      template<std::size_t Argc> val operator()(std::array<val, Argc>, val *args_out = {}) const;
   public: // Scalar IDs
      enum class id: int;
      static constexpr enum id id(const char *) noexcept;
      MNL_INLINE operator enum id() const noexcept { return static_cast<enum id>(rep); }
      template<enum id> static const sym from_id;
   private: // Concrete representation
      enum: unsigned short {} rep;
      static std::map<std::string, const decltype(rep)> dict;
      static decltype(dict)::const_iterator inverse[];
      static std::vector<decltype(rep)> pool;
      MNL_IF_WITH_MT(static std::mutex mutex;)
      static /*atomic*/ long rc[];
      MNL_INLINE explicit sym(decltype(rep) rep) noexcept: rep(rep) { addref(); }
      MNL_INLINE operator decltype(rep)() const noexcept { return rep; }
   private: // Implementation helpers
      MNL_INLINE void addref()  const noexcept { addref(rep); }
      MNL_INLINE void release() const noexcept { release(rep); }
      static void free(decltype(rep)) noexcept;
   private:
      MNL_INLINE static void addref(decltype(rep) rep) noexcept
         { MNL_IF_WITHOUT_MT(++rc[rep]) MNL_IF_WITH_MT(__atomic_add_fetch(&rc[rep], 1, __ATOMIC_RELAXED)); }
      MNL_INLINE static void release(decltype(rep) rep) noexcept
         { if (MNL_UNLIKELY(! MNL_IF_WITHOUT_MT(--rc[rep]) MNL_IF_WITH_MT(__atomic_sub_fetch(&rc[rep], 1, __ATOMIC_RELAXED)) )) free(rep); }
   public: // Related stuff
      template<typename = class code> class tab/*le*/; // do not: move "= class code" to the definition below (compiler bug)
      friend val;
      friend box<string>; friend box<vector<val>>; // direct access to "enum rep" members required, for performance reasons
      template<int> friend struct aux::_record;    // ditto
      friend range<false>; friend range<true>;     // ditto
   };
   MNL_INLINE inline void swap(sym &lhs, sym &rhs) noexcept { lhs.swap(rhs); }
   bool operator==(const sym &, const sym &) noexcept, operator<(const sym &, const sym &) noexcept;
   MNL_INLINE inline bool operator!=(const sym &lhs, const sym &rhs) noexcept { return std::rel_ops::operator!=(lhs, rhs); }
   MNL_INLINE inline bool operator> (const sym &lhs, const sym &rhs) noexcept { return std::rel_ops::operator> (lhs, rhs); }
   MNL_INLINE inline bool operator<=(const sym &lhs, const sym &rhs) noexcept { return std::rel_ops::operator<=(lhs, rhs); }
   MNL_INLINE inline bool operator>=(const sym &lhs, const sym &rhs) noexcept { return std::rel_ops::operator>=(lhs, rhs); }

   template<typename Val> class sym::tab {
   public: // Standard operations
      tab() = default;
      tab(const tab &rhs): rep(rhs.rep), undef(rhs.undef) { addref(); }
      MNL_INLINE tab(tab &&rhs) noexcept(std::is_nothrow_move_constructible<Val>::value): rep(move(rhs.rep)), undef((move)(rhs.undef)) { rhs.rep.clear(); }
      ~tab() noexcept { release(); }
      tab &operator=(const tab &rhs) { if (&rhs == this) return *this; release(); rep = rhs.rep, undef = rhs.undef; addref(); return *this; }
      MNL_INLINE tab &operator=(tab &&rhs) { swap(rhs); return *this; }
      MNL_INLINE void swap(tab &rhs) { using std::swap; swap(rep, rhs.rep), swap(undef, rhs.undef); }
   public: // Specialized construction
      MNL_INLINE explicit tab(Val undef) noexcept(std::is_nothrow_move_constructible<Val>::value): undef((move)(undef)) {}
      MNL_INLINE tab(initializer_list<pair<sym, Val>> il) { update(il); }
      MNL_INLINE tab(Val undef, initializer_list<pair<sym, Val>> il): undef((move)(undef)) { update(il); }
   public: // Queries and incremental updates
      MNL_INLINE typename std::conditional<std::is_same<Val, bool>::value, bool, const Val &>::type operator[](const sym &key) const noexcept {
         return MNL_LIKELY(key < (int)rep.size()) ? rep[key] : undef;
      }
      void update(const sym &key, Val val) {
         if (key >= (int)rep.size())
            rep.resize(key + 1, undef);
         if (rep[key] == undef) {
            if ((rep[key] = (move)(val)) != undef) sym::addref(key);
         } else {
            if ((rep[key] = (move)(val)) == undef) sym::release(key);
         }
      }
   public: // Compatibility/convenience stuff
      void clear()         { release(); rep.clear(); }
      void shrink_to_fit() { rep.shrink_to_fit(); }
   public:
      MNL_NOINLINE void update(initializer_list<pair<sym, Val>> il) { for (auto &&kv: il) update(kv); }
      MNL_INLINE   void update(pair<const sym &, Val> kv) { update(kv.first, (move)(kv.second)); }
   private: // Concrete representation
      vector<typename std::conditional<std::is_same<Val, bool>::value, unsigned char, Val>::type> rep;
      Val undef{};
   private: // Implementation helpers
      void addref()  const noexcept
         { for (int sn = 0; sn < (int)rep.size(); ++sn) if (MNL_UNLIKELY(rep[sn] != undef)) sym::addref (static_cast<decltype(sym::rep)>(sn)); }
      void release() const noexcept
         { for (int sn = 0; sn < (int)rep.size(); ++sn) if (MNL_UNLIKELY(rep[sn] != undef)) sym::release(static_cast<decltype(sym::rep)>(sn)); }
   };
}} // namespace aux::pub

MNL_INLINE constexpr auto aux::pub::sym::id(const char *lab/*el*/) noexcept->enum id
// not intended to be called in a non-constexpr context but should you do, it might produce a run-time value
// (enum id)(sym)lab (as well as for not well-known symbols)
# if __has_builtin(__builtin_is_constant_evaluated)
   if (__builtin_is_constant_evaluated())
# endif
   {  constexpr auto eq/*ual*/ = [](auto lhs, auto rhs) constexpr MNL_NOINLINE{ // constexpr replacement for std::strcmp
         while (MNL_LIKELY(*lhs == *rhs) && MNL_LIKELY(*lhs)) ++lhs, ++rhs;
         return *lhs == *rhs;
      };
      int count = {};
   # define MNL_ID(LAB) if (MNL_UNLIKELY(eq(lab, LAB))) return static_cast<enum id>(count); ++count;
      # include "wks.tcc"
   # undef MNL_ID
   }
   return (sym)lab; // not meant to introduce new symbols, since it aborts on exceptions
}

template<enum id Id> const aux::pub::sym aux::pub::sym::from_id{static_cast<decltype(rep)>(Id)};


   # define MNL_SYM(TXT)  MNL_AUX_INIT(::mnl::sym(TXT))
   # define MNL_DISP(...) MNL_AUX_INIT(::mnl::aux::disp({__VA_ARGS__}))
namespace aux { MNL_NOINLINE inline sym::tab<signed char> disp(initializer_list<const char *> il) {
   sym::tab<signed char> res; int val = 0; for (auto &&key: il) res.update(key, ++val); return res;
}}

// class val ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Design decisions:
   - Use a lightweight (trivial) variant of `val` for more efficient interaction across routine boundaries? - NO
   - Use NaN coding? - YES
   - Use HL (bit-fields) or LL (bitwise ops) to describe the representation (in case of NaN coding)? - bit-fields
   - Expose as raw bit patterns (in case of using bitwise ops) for potentially more efficient op implementation? - N/A, NO
   - Which order for tag/value? - directly compatible with IEEE 754
*/

template<typename> class box;

namespace aux { namespace pub {
   typedef val ast; // val when used as an Abstract Syntax Tree - for documentation purposes

   class val/*ue*/ {
   public: // Standard operations
      MNL_INLINE val(decltype(nullptr) = {}) noexcept: rep{0xFFF8 + 0b000} {}
      MNL_INLINE val(const val &rhs) noexcept: rep(rhs.rep) { hold(); }
      MNL_INLINE val(val &&rhs) noexcept: rep(rhs.rep) { rhs.rep = {0xFFF8 + 0b000}; }
      MNL_INLINE ~val() { unhold(); }
      MNL_INLINE val &operator=(const val &rhs) noexcept { rhs.hold(), unhold(), rep = rhs.rep; return *this; }
      MNL_INLINE val &operator=(val &&rhs) noexcept { unhold(), rep = rhs.rep, rhs.rep = {0xFFF8 + 0b000}; return *this; }
      MNL_INLINE void swap(val &rhs) noexcept { using std::swap; swap(rep, rhs.rep); }
      MNL_INLINE explicit operator bool() const noexcept { return *this != nullptr; }
   public: // Construction -- Implicit conversion (to) -- implicit conversion disabled for `dat`
      MNL_INLINE val(long long dat) noexcept: rep{0xFFF8 + 0b001, dat} {} // min_i48 .. max_i48
      MNL_INLINE val(int dat) noexcept:       val((long long)dat) {}
      MNL_INLINE val(double dat) noexcept: rep(dat) { if (rep.tag() >= 0xFFF8 + 0b000) __builtin_unreachable(); } // excl. inf and nan
      MNL_INLINE val(float dat) noexcept: rep{0xFFF8 + 0b010, dat} {} // ditto
      MNL_INLINE val(const sym &dat) noexcept: rep{0xFFF8 + 0b110, dat} {}
      MNL_INLINE val(bool dat) noexcept: rep{dat | 0xFFF8 + 0b100} {}
      MNL_INLINE val(unsigned dat) noexcept: rep{0xFFF8 + 0b010, dat} {}
      MNL_INLINE val(char dat) noexcept:     val((unsigned)(unsigned char)dat) {}
      template<typename Dat> val(Dat dat): rep{0xFFF8 + 0b111, (void *)(root *)new box<Dat>{std::move(dat)}} {}
      val(const char *);
      MNL_INLINE val(char *dat): val((const char *)dat) {}
      struct boxable {};
   public: // Extraction
      template<typename Dat = decltype(nullptr)> MNL_INLINE friend bool test(const val &rhs) noexcept
         { return rhs.test<Dat>(); }
      template<typename Dat = decltype(nullptr)> MNL_INLINE friend Dat  cast(const val &rhs) noexcept(std::is_nothrow_copy_constructible<Dat>::value)
         { return rhs.cast<Dat>(); }
   public: // Misc essentials
      val default_invoke(const sym &op, int argc, val argv[]);
      long rc /*reference counter*/() const noexcept;
      int default_order(const val &) const noexcept; // actually from MANOOL API
   private: // Concrete representation
      static_assert(sizeof(double) == 8);

   # if !__STDC_IEC_559__ | (defined(__FLOAT_WORD_ORDER__) & __FLOAT_WORD_ORDER__ != __BYTE_ORDER__)
   // 1. __STDC_IEC_559__ is defined AFTER #including some standard header on clang (nonconforming)
   // 2. just ASSUME "uniform" FP endianness for clang and/or other compilers where __FLOAT_WORD_ORDER__ is absent
      # error "Unsupported FP interchange format"
   # endif
      class MNL_ALIGN(8) rep { // bit-layout management - for IEEE 754 FP representation and uniform FP endianness
      # if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
         unsigned short _tag;
      # endif
         union {
            MNL_PACK signed long long _int: 48;
            MNL_PACK std::uintptr_t _ptr: sizeof(void *) == 4 ? 32 : sizeof(void *) == 8 ? 48 : 0;
            sym _sym; // standard-layout struct
         }; // TODO: maybe using "union trick" affects less the optimizer?
      # if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
         unsigned short _tag;
      # elif __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
         # error "Unsupported mixed endianness"
      # endif
      public:
         MNL_INLINE rep() noexcept /*unused*/ {}
         MNL_INLINE ~rep() {}
         MNL_INLINE rep(const rep &rhs) noexcept { copy(rhs); }
         MNL_INLINE rep &operator=(const rep &rhs) noexcept { copy(rhs); return *this; }
      public:
         template<typename Dat> explicit rep(unsigned tag, Dat) noexcept;
         explicit rep(unsigned tag, long long) noexcept;
         explicit rep(unsigned tag, void *) noexcept;
         MNL_INLINE explicit rep(unsigned tag) noexcept: _tag(tag) {}
         explicit rep(double) noexcept;
         explicit rep(unsigned tag, const sym &) noexcept;
      public:
         MNL_INLINE rep &operator=(unsigned tag) noexcept { _tag = tag; return *this; }
         MNL_INLINE unsigned tag() const noexcept { return _tag; }
         template<typename Dat> Dat dat() const noexcept;
      private: // assume memmove copies the union representation AND transfers its active member, if any exists
         MNL_INLINE void copy(const rep &rhs) noexcept { // TODO: memmove introduces aliasing issues (with escaped ptrs)!!!
            std::memmove(this, &rhs, sizeof *this); // updates sym::rep (AND rep::tag at once), in case of _sym (corner case of ISO/IEC 14882:2011)
         }
      } rep;
      static_assert(sizeof rep == 8, "sizeof rep == 8");                                                             // paranoid check
      static_assert(std::is_standard_layout<decltype(rep)>::value, "std::is_standard_layout<decltype(rep)>::value"); // ditto
      MNL_INLINE explicit val(decltype(rep) rep) noexcept: rep(rep) {}
   private: // Implementation helpers
      void addref() const noexcept, release() const noexcept;
      template<typename     = decltype(nullptr)> bool test() const noexcept;
      template<typename Dat = decltype(nullptr)> Dat  cast() const noexcept(std::is_nothrow_copy_constructible<Dat>::value);
      //MNL_IF_CLANG(public:)
      class root; template<typename> friend class mnl::box; // TODO: should we friend box, which could be partially user-customizable?
   public: // Functional application (and Repl)
      static constexpr int max_argc = sym::max_argc;
      static constexpr auto max_i48 = (1ll << 48 - 1) - 1, min_i48 = -max_i48;
   // Essential for performance
      // For one argument
         MNL_INLINE val operator()(const val &arg0) const & { return _apply(*this, arg0); }
         MNL_INLINE val operator()(val &&arg0) const & { return _apply(*this, _mv(arg0)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(const Sym &arg0) const & { return _apply(*this, arg0); }
         MNL_INLINE val operator()(const val &arg0) && { return _apply(_mv(*this), arg0); }
         MNL_INLINE val operator()(val &&arg0) && { return _apply(_mv(*this), _mv(arg0)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(const Sym &arg0) && { return _apply(_mv(*this), arg0); }
      // For one argument
         MNL_INLINE val fetch(const val &key0) const & { return _fetch(*this, key0); }
         MNL_INLINE val fetch(val &&key0) const & { return _fetch(*this, _mv(key0)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val fetch(const Sym &key0) const & { return _fetch(*this, key0); }
         MNL_INLINE val fetch(const val &key0) && { return _fetch(_mv(*this), key0); }
         MNL_INLINE val fetch(val &&key0) && { return _fetch(_mv(*this), _mv(key0)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val fetch(const Sym &key0) && { return _fetch(_mv(*this), key0); }
      // For two arguments
         MNL_INLINE val operator()(const val &arg0, const val &arg1) const & { return _apply(*this, arg0, arg1); }
         MNL_INLINE val operator()(const val &arg0, val &&arg1) const & { return _apply(*this, arg0, _mv(arg1)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(const val &arg0, const Sym &arg1) const & { return _apply(*this, arg0, arg1); }
         MNL_INLINE val operator()(val &&arg0, const val &arg1) const & { return _apply(*this, _mv(arg0), arg1); }
         MNL_INLINE val operator()(val &&arg0, val &&arg1) const & { return _apply(*this, _mv(arg0), _mv(arg1)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(val &&arg0, const Sym &arg1) const & { return _apply(*this, _mv(arg0), arg1); }
         MNL_INLINE val operator()(const val &arg0, const val &arg1) && { return _apply(_mv(*this), arg0, arg1); }
         MNL_INLINE val operator()(const val &arg0, val &&arg1) && { return _apply(_mv(*this), arg0, _mv(arg1)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(const val &arg0, const Sym &arg1) && { return _apply(_mv(*this), arg0, arg1); }
         MNL_INLINE val operator()(val &&arg0, const val &arg1) && { return _apply(_mv(*this), _mv(arg0), arg1); }
         MNL_INLINE val operator()(val &&arg0, val &&arg1) && { return _apply(_mv(*this), _mv(arg0), _mv(arg1)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(val &&arg0, const Sym &arg1) && { return _apply(_mv(*this), _mv(arg0), arg1); }
      // For multiple arguments
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(int argc, Val argv[]) const & { return _apply(*this, argc, argv); }
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(int argc, Val argv[]) && { return _apply(_mv(*this), argc, argv); }
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val fetch(int argc, Val argv[]) const & { return _fetch(*this, argc, argv); }
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val fetch(int argc, Val argv[]) && { return _fetch(_mv(*this), argc, argv); }
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(int argc, Val argv[], val *argv_out) const & { return _apply(*this, argc, argv, argv_out); }
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
         MNL_INLINE val operator()(int argc, Val argv[], val *argv_out) && { return _apply(_mv(*this), argc, argv, argv_out); }
         // argv_out[-1] corresponds to self; !argc < !argv
      // For two arguments
         [[nodiscard]] MNL_INLINE val repl(const val &key0, const val &value) && { return _repl(_mv(*this), key0, value); }
         [[nodiscard]] MNL_INLINE val repl(const val &key0, val &&value) && { return _repl(_mv(*this), key0, _mv(value)); }
         [[nodiscard]] MNL_INLINE val repl(val &&key0, const val &value) && { return _repl(_mv(*this), _mv(key0), value); }
         [[nodiscard]] MNL_INLINE val repl(val &&key0, val &&value) && { return _repl(_mv(*this), _mv(key0), _mv(value)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         [[nodiscard]] MNL_INLINE val repl(const Sym &key0, const val &value) && { return _repl(_mv(*this), key0, value); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         [[nodiscard]] MNL_INLINE val repl(const Sym &key0, val &&value) && { return _repl(_mv(*this), key0, _mv(value)); }
      // For three arguments
         [[nodiscard]] MNL_INLINE val repl(const val &key0, const val &key1, const val &value) && { return _repl(_mv(*this), key0, key1, value); }
         [[nodiscard]] MNL_INLINE val repl(const val &key0, const val &key1, val &&value) && { return _repl(_mv(*this), key0, key1, _mv(value)); }
         [[nodiscard]] MNL_INLINE val repl(const val &key0, val &&key1, const val &value) && { return _repl(_mv(*this), key0, _mv(key1), value); }
         [[nodiscard]] MNL_INLINE val repl(const val &key0, val &&key1, val &&value) && { return _repl(_mv(*this), key0, _mv(key1), _mv(value)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         [[nodiscard]] MNL_INLINE val repl(const val &key0, const Sym &key1, const val &value) && { return _repl(_mv(*this), key0, key1, value); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         [[nodiscard]] MNL_INLINE val repl(const val &key0, const Sym &key1, val &&value) && { return _repl(_mv(*this), key0, key1, _mv(value)); }
         [[nodiscard]] MNL_INLINE val repl(val &&key0, const val &key1, const val &value) && { return _repl(_mv(*this), _mv(key0), key1, value); }
         [[nodiscard]] MNL_INLINE val repl(val &&key0, const val &key1, val &&value) && { return _repl(_mv(*this), _mv(key0), key1, _mv(value)); }
         [[nodiscard]] MNL_INLINE val repl(val &&key0, val &&key1, const val &value) && { return _repl(_mv(*this), _mv(key0), _mv(key1), value); }
         [[nodiscard]] MNL_INLINE val repl(val &&key0, val &&key1, val &&value) && { return _repl(_mv(*this), _mv(key0), _mv(key1), _mv(value)); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         [[nodiscard]] MNL_INLINE val repl(val &&key0, const Sym &key1, const val &value) && { return _repl(_mv(*this), _mv(key0), key1, value); }
      template<class Sym, std::enable_if_t<std::is_same_v<Sym, sym>, decltype(nullptr)> = decltype(nullptr){}>
         [[nodiscard]] MNL_INLINE val repl(val &&key0, const Sym &key1, val &&value) && { return _repl(_mv(*this), _mv(key0), key1, _mv(value)); }
      // For multiple arguments
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
      [[nodiscard]] MNL_INLINE val repl(int argc, Val argv[]) &&
         { return _repl(_mv(*this), argc, argv); }
      template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
      [[nodiscard]] MNL_INLINE val repl(int argc, Val argv[], val *argv_out) &&
         { return _repl(_mv(*this), argc, argv, argv_out); }
   // Metaprogramming
         MNL_INLINE val operator()(val a0, val a1, val a2) const & { val argv[] = {_mv(a0), _mv(a1), _mv(a2)}; return (*this)(std::size(argv), argv); }
         MNL_INLINE val operator()(val a0, val a1, val a2) &&   { val argv[] = {_mv(a0), _mv(a1), _mv(a2)}; return _mv(*this)(std::size(argv), argv); }
   // Convenience
      template<std::size_t Argc> MNL_INLINE val operator()(std::array<val, Argc> args) const &
         { return (*this)(Argc, args.data()); }
      template<std::size_t Argc> MNL_INLINE val operator()(std::array<val, Argc> args) &&
         { return _mv(*this)(Argc, args.data()); }
      template<std::size_t Argc> MNL_INLINE val fetch(std::array<val, Argc> args) const &
         { return (*this).fetch(Argc, args.data()); }
      template<std::size_t Argc> MNL_INLINE val fetch(std::array<val, Argc> args) &&
         { return _mv(*this).fetch(Argc, args.data()); }
      template<std::size_t Argc> MNL_INLINE val operator()(std::array<val, Argc> args, val *args_out) const &
         { return (*this)(Argc, args.data(), args_out); }
      template<std::size_t Argc> MNL_INLINE val operator()(std::array<val, Argc> args, val *args_out) &&
         { return _mv(*this)(Argc, args.data(), args_out); }
      template<std::size_t Argc> [[nodiscard]] MNL_INLINE val repl(std::array<val, Argc> args) &&
         { return _mv(*this).repl(Argc, args.data()); }
      template<std::size_t Argc> [[nodiscard]] MNL_INLINE val repl(std::array<val, Argc> args, val *args_out) &&
         { return _mv(*this).repl(Argc, args.data(), args_out); }
      // For no arguments
      MNL_INLINE val operator()() const &
         { return (*this)(0, (val *)nullptr); }
      MNL_INLINE val operator()() &&
         { return _mv(*this)(0, (val *)nullptr); }
      // Alternative notation
      MNL_INLINE val invoke(const sym *op, int argc, val argv[], val *argv_out = {}) const &
         { return op(*this, argc, argv, argv_out); }
      MNL_INLINE val invoke()(const sym *op, int argc, val argv[], val *argv_out = {}) &&
         { return op(_mv(*this), argc, argv, argv_out); }
      template<std::size_t Argc> MNL_INLINE val invoke(const sym *op, std::array<val, Argc> args, val *args_out = {}) const &
         { return op(*this, Argc, args.data(), args_out); }
      template<std::size_t Argc> MNL_INLINE val invoke()(const sym *op, std::array<val, Argc> args, val *args_out = {}) &&
         { return op(_mv(*this), Argc, args.data(), args_out); }
   // Even more convenience -- tracing counterparts
      ...
   private: // Implementation of the above
      // Apply/Fetch
      template<typename Self, typename Arg0>                static val _apply(Self &&, Arg0 &&);
      template<typename Self, typename Key0>                static val _fetch(Self &&, Key0 &&);
      template<typename Self, typename Arg0, typename Arg1> static val _apply(Self &&, Arg0 &&, Arg1 &&);
      template<typename Self>                               static val _apply(Self &&, int argc, val []);
      template<typename Self>                               static val _fetch(Self &&, int argc, val []);
      template<typename Self>                               static val _apply(Self &&, int argc, val [], val *argv_out);
      // Repl
      template<typename Self, typename Key0, typename Val>                static val _repl(Self &&, Key0 &&, Val &&);
      template<typename Self, typename Key0, typename Key1, typename Val> static val _repl(Self &&, Key0 &&, Key1 &&, Val &&);
      template<typename Self>                                             static val _repl(Self &&, int argc, val []);
      template<typename Self>                                             static val _repl(Self &&, int argc, val [], val *argv_out);
   private: // Implementation of sym::operator()
      template<typename Self> static MNL_HOT val _invoke(Self &&, const sym &, int, val [], val *); // Self == const val & | Self == val
      friend val sym::operator()(const val &, int, val [], val *) const, sym::operator()(val &&, int, val [], val *) const;

   private:
      template<typename Arg> static MNL_INLINE decltype(auto) _mv(Arg &&arg) noexcept { return std::move(arg); }

   public: // Convenience -- Direct comparison with other types
      bool operator==(decltype(nullptr)) const noexcept, operator==(const sym &) const noexcept;
      MNL_INLINE bool operator!=(decltype(nullptr)) const noexcept { return !(*this == nullptr); }
      MNL_INLINE bool operator!=(const sym &rhs) const noexcept { return !(*this == rhs); }
   public: // Convenience -- Working with ASTs
      val(vector<ast>, loc);
      bool is_list() const noexcept;
      vector<ast>::const_iterator begin() const noexcept, end() const noexcept;
      vector<ast>::const_reverse_iterator rbegin() const noexcept, rend() const noexcept;
      bool empty() const noexcept;
      long size() const noexcept;
      const ast &operator[](long) const noexcept, &at(long) const /*unused*/;
      const ast &front() const noexcept, &back() const noexcept /*unused*/;
      const loc &_loc(const loc &) const noexcept;
      // list subranges:
      typedef struct { vector<ast>::const_iterator _begin, _end;
         MNL_INLINE vector<ast>::const_iterator begin() const noexcept { return _begin; }
         MNL_INLINE vector<ast>::const_iterator end() const noexcept { return _end; }
      } vector_const_iterator_range, vci_range;
      vci_range  operator+(long) const noexcept;
      typedef struct { vector<ast>::const_reverse_iterator _begin, _end;
         MNL_INLINE vector<ast>::const_reverse_iterator begin() const noexcept { return _begin; }
         MNL_INLINE vector<ast>::const_reverse_iterator end() const noexcept { return _end; }
      } vector_const_reverse_iterator_range, vcri_range;
      vcri_range operator-(long) const noexcept;
   public: // Related stuff
      //friend sym;
      friend val sym::operator()(int, val [], val *) const; // essential

      friend class proc_Min; friend class proc_Max;
   };
   MNL_INLINE inline void swap(val &lhs, val &rhs) noexcept { lhs.swap(rhs); }
   // defined in friend declarations above:
   template<typename>     bool test(const val &) noexcept; // TODO: disallow conv by using templ param
   template<typename Dat> Dat  cast(const val &) noexcept(std::is_nothrow_copy_constructible<Dat>::value);

   // Fake test/case for non-val inputs
   template<typename Dat> MNL_INLINE inline bool test(const typename std::remove_cv<typename std::remove_reference<
      typename std::enable_if<!std::is_same<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type, val>::value, Dat>::type
      >::type>::type &) noexcept { return true; }
   template<typename Dat> MNL_INLINE inline Dat  cast(const typename std::remove_cv<typename std::remove_reference<
      typename std::enable_if<!std::is_same<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type, val>::value, Dat>::type
      >::type>::type &dat) noexcept(std::is_nothrow_copy_constructible<Dat>::value) { return dat; }
}} // namespace aux::pub

// Forward-declared as members of class sym ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
   MNL_INLINE inline val sym::operator()(const val &self, int argc, Val argv[], val *argv_out) const
      { return val::_invoke(self, *this, argc, argv, argv_out); }
   template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
   MNL_INLINE inline val sym::operator()(val &&self, int argc, Val argv[], val *argv_out) const
      { return val::_invoke(std::move(self), *this, argc, argv, argv_out); }

   // For one argument
   MNL_INLINE inline val sym::operator()(const val &arg0) const { return (*this)(arg0, 0, (val *)nullptr); }
   MNL_INLINE inline val sym::operator()(val &&arg0) const { return (*this)(std::move(arg0), 0, (val *)nullptr); }
   // For two arguments
   MNL_INLINE inline val sym::operator()(const val &arg0, const val &arg1) const { return (*this)(arg0, (val)arg1); }
   MNL_INLINE inline val sym::operator()(const val &arg0, val &&arg1) const { return (*this)(arg0, 1, &arg1); }
   MNL_INLINE inline val sym::operator()(val &&arg0, const val &arg1) const { return (*this)(std::move(arg0), (val)arg1); }
   MNL_INLINE inline val sym::operator()(val &&arg0, val &&arg1) const { return (*this)(std::move(arg0), 1, &arg1); }
   // For multiple arguments
   template<class Val, std::enable_if_t<std::is_same_v<Val, val>, decltype(nullptr)> = decltype(nullptr){}>
   MNL_INLINE inline val sym::operator()(int argc, Val argv[], val *argv_out) const {
      if (MNL_UNLIKELY(!argc)) err_InvalidInvocation(); // argc != 0 check may be eliminated due to inlining
      return (*this)(std::move(*argv), --argc, argc ? ++argv : nullptr, argv_out + !!argv_out); // relies on C++17 eval order
   }
   MNL_INLINE inline val sym::operator()(const val &a0, val a1, val a2) const
      { val argv[] = {std::move(a1), std::move(a2)}; return (*this)(a0, std::size(argv), argv); }
   MNL_INLINE inline val sym::operator()(val &&a0, val a1, val a2) const
      { val argv[] = {std::move(a1), std::move(a2)}; return (*this)(std::move(a0), std::size(argv), argv); }

   template<std::size_t Argc> MNL_INLINE inline val sym::operator()(const val &self, std::array<val, Argc> args, val *args_out) const
      { return (*this)(self, Argc, args.data(), args_out); }
   template<std::size_t Argc> MNL_INLINE inline val sym::operator()(val &&self, std::array<val, Argc> args, val *args_out) const
      { return (*this)(std::move(self), Argc, args.data(), args_out); }
   template<std::size_t Argc> MNL_INLINE inline val sym::operator()(std::array<val, Argc> args, val *args_out) const
      { return (*this)(Argc, args.data(), args_out); }

// Bit-Layout Management -- Data Write /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE inline val::rep::rep(unsigned tag, Dat dat) noexcept: _tag(tag) {
      static_assert(std::is_pod<Dat>::value, "std::is_pod<Dat>::value");
      static_assert(sizeof dat <= 6, "sizeof dat <= 6");
      static_assert(!std::is_pointer<Dat>::value, "!std::is_pointer<Dat>::value");
   # if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      memcpy(this, &dat, sizeof dat);
   # else // __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      memcpy(reinterpret_cast<unsigned char *>(this) + sizeof _tag, &dat, sizeof dat);
   # endif
   }
   MNL_INLINE inline val::rep::rep(unsigned tag, long long dat) noexcept
      : _tag(tag), _int(dat) {}
   MNL_INLINE inline val::rep::rep(unsigned tag, void *dat) noexcept
      : _tag(tag), _ptr(reinterpret_cast<uintptr_t>(dat)) {}
   MNL_INLINE inline val::rep::rep(double dat) noexcept
      { memcpy(this, &dat, sizeof dat); }
   MNL_INLINE inline val::rep::rep(unsigned tag, const sym &dat) noexcept
      : _tag(tag), _sym(dat) {}

// Bit-Layout Management -- Data Read //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE inline Dat val::rep::dat() const noexcept {
      Dat dat;
      static_assert(std::is_pod<Dat>::value, "std::is_pod<Dat>::value");
      static_assert(sizeof dat <= 6, "sizeof dat <= 6");
      static_assert(!std::is_pointer<Dat>::value, "!std::is_pointer<Dat>::value");
   # if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      memcpy(&dat, this, sizeof dat); return dat;
   # else // __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      memcpy(&dat, reinterpret_cast<const unsigned char *>(this) + sizeof _tag, sizeof dat); return dat;
   # endif
   }
   template<> MNL_INLINE inline long long val::rep::dat() const noexcept
      { return _int; }
   template<> MNL_INLINE inline void *val::rep::dat() const noexcept
      { return reinterpret_cast<void *>(_ptr); }
   template<> MNL_INLINE inline double val::rep::dat() const noexcept
      { double dat; memcpy(&dat, this, sizeof dat); return dat; }
   template<> MNL_INLINE inline const sym &val::rep::dat() const noexcept
      { return _sym; }

// class Template box //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   class val::root {
   protected:
      MNL_INLINE explicit root(const std::byte *tag) noexcept: _tag(reinterpret_cast<std::uintptr_t>(tag)) {}
      virtual ~root() = default;
   private:
      root(const root &) = delete;
      root &operator=(const root &) = delete; // would be implicitly deleted anyway
   protected:
      bool shared() const noexcept { return MNL_IF_WITHOUT_MT(_rc) MNL_IF_WITH_MT(__atomic_load_n(&_rc, __ATOMIC_RELAXED)) != 1; }
   private:
      const unsigned _tag; // assume 64-bit small/medium code model or x32 ABI or 32-bit ISA
      MNL_NOTE(atomic) long _rc = 1;
   private: // 50 VMT entries (+dtor)
      MNL_NOINLINE virtual val _invoke(const val &self, const sym &op, int argc, val [], val *argv_out = {}) = 0; // argv_out[-1] corresponds to self
      MNL_NOINLINE virtual val _invoke(val &&self,      const sym &op, int argc, val [], val *argv_out = {}) = 0; // ditto
   private:
      // For one argument (6 VMT entries)
      MNL_HOT virtual val _apply(const val &self, const val &) = 0;
      MNL_HOT virtual val _apply(const val &self, val &&) = 0;
      MNL_HOT virtual val _apply(const val &self, const sym &) = 0;
      MNL_HOT virtual val _apply(val &&self, const val &) = 0;
      MNL_HOT virtual val _apply(val &&self, val &&) = 0;
      MNL_HOT virtual val _apply(val &&self, const sym &) = 0;
      // For one argument (6 VMT entries)
      MNL_HOT virtual val _fetch(const val &self, const val &) = 0;
      MNL_HOT virtual val _fetch(const val &self, val &&) = 0;
      MNL_HOT virtual val _fetch(const val &self, const sym &) = 0;
      MNL_HOT virtual val _fetch(val &&self, const val &) = 0;
      MNL_HOT virtual val _fetch(val &&self, val &&) = 0;
      MNL_HOT virtual val _fetch(val &&self, const sym &) = 0;
      // For two arguments (12 VMT entries)
      MNL_HOT virtual val _apply(const val &self, const val &, const val &) = 0;
      MNL_HOT virtual val _apply(const val &self, const val &, val &&) = 0;
      MNL_HOT virtual val _apply(const val &self, const val &, const sym &) = 0;
      MNL_HOT virtual val _apply(const val &self, val &&, const val &) = 0;
      MNL_HOT virtual val _apply(const val &self, val &&, val &&) = 0;
      MNL_HOT virtual val _apply(const val &self, val &&, const sym &) = 0;
      MNL_HOT virtual val _apply(val &&self, const val &, const val &) = 0;
      MNL_HOT virtual val _apply(val &&self, const val &, val &&) = 0;
      MNL_HOT virtual val _apply(val &&self, const val &, const sym &) = 0;
      MNL_HOT virtual val _apply(val &&self, val &&, const val &) = 0;
      MNL_HOT virtual val _apply(val &&self, val &&, val &&) = 0;
      MNL_HOT virtual val _apply(val &&self, val &&, const sym &) = 0;
      // For multiple arguments (4 VMT entries)
      MNL_HOT virtual val _apply(const val &self, int argc, val []) = 0;
      MNL_HOT virtual val _apply(val &&self,      int argc, val []) = 0;
      MNL_HOT virtual val _fetch(const val &self, int argc, val []) = 0;
      MNL_HOT virtual val _fetch(val &&self,      int argc, val []) = 0;
      // For two arguments (6 VMT entries)
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const sym &, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const sym &, val &&) = 0;
      // For three arguments (12 VMT entries)
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, const val &, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, const val &, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, val &&, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, val &&, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, const sym &, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, const val &, const sym &, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, const val &, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, const val &, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, val &&, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, val &&, val &&) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, const sym &, const val &) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, val &&, const sym &, val &&) = 0;
      // For multiple arguments (2 VMT entries)
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, int argc, val []) = 0;
      [[nodiscard]] MNL_HOT virtual val _repl(val &&self, int argc, val [], val *argv_out) = 0; // see above about argv_out
   public: // Who can use the private/protected members directly?
      friend val;
   };
   template<typename Dat> class box final: val::root {
      Dat dat;
      MNL_INLINE explicit box(Dat &&dat): root{&_tag}, dat(std::move(dat)) {}
      ~box() = default;
   private:
      static constexpr std::byte _tag{};
      friend val; // to directly use Dat, ctor, dtor, and _tag
   private: // 50 VMT entries (+dtor)
      MNL_NOINLINE val _invoke(const val &self, const sym &op, int argc, val argv[], val *argv_out = {}) override
         { return invoke(self, op, argv, argv_out); }
      MNL_NOINLINE val _invoke(val &&self,      const sym &op, int argc, val argv[], val *argv_out = {}) override
         { return invoke(_mv(self), op, argv, argv_out); }
   private:
      // For one argument (6 VMT entries)
      MNL_HOT val _apply(const val &self, const val &arg0) override { return apply(self, arg0); }
      MNL_HOT val _apply(const val &self, val &&arg0) override { return apply(self, _mv(arg0)); }
      MNL_HOT val _apply(const val &self, const sym &arg0) override { return apply(self, arg0); }
      MNL_HOT val _apply(val &&self, const val &arg0) override { return apply(_mv(self), arg0); }
      MNL_HOT val _apply(val &&self, val &&arg0) override { return apply(_mv(self), _mv(arg0)); }
      MNL_HOT val _apply(val &&self, const sym &arg0) override { return apply(_mv(self), arg0); }
      // For one argument (6 VMT entries)
      MNL_HOT val _fetch(const val &self, const val &arg0) override { return fetch(self, arg0); }
      MNL_HOT val _fetch(const val &self, val &&arg0) override { return fetch(self, _mv(arg0)); }
      MNL_HOT val _fetch(const val &self, const sym &arg0) override { return fetch(self, arg0); }
      MNL_HOT val _fetch(val &&self, const val &arg0) override { return fetch(_mv(self), arg0); }
      MNL_HOT val _fetch(val &&self, val &&arg0) override { return fetch(_mv(self), _mv(arg0)); }
      MNL_HOT val _fetch(val &&self, const sym &arg0) override { return fetch(_mv(self), arg0); }
      // For two arguments (12 VMT entries)
      MNL_HOT val _apply(const val &self, const val &arg0, const val &arg1) override { return apply(self, arg0, arg1); }
      MNL_HOT val _apply(const val &self, const val &arg0, val &&arg1) override { return apply(self, arg0, _mv(arg1)); }
      MNL_HOT val _apply(const val &self, const val &arg0, const sym &arg1) override { return apply(self, arg0, arg1); }
      MNL_HOT val _apply(const val &self, val &&arg0, const val &arg1) override { return apply(self, _mv(arg0), arg1); }
      MNL_HOT val _apply(const val &self, val &&arg0, val &&arg1) override { return apply(self, _mv(arg0), _mv(arg1)); }
      MNL_HOT val _apply(const val &self, val &&arg0, const sym &arg1) override { return apply(self, _mv(arg0), arg1); }
      MNL_HOT val _apply(val &&self, const val &arg0, const val &arg1) override { return apply(_mv(self), arg0, arg1); }
      MNL_HOT val _apply(val &&self, const val &arg0, val &&arg1) override { return apply(_mv(self), arg0, _mv(arg1)); }
      MNL_HOT val _apply(val &&self, const val &arg0, const sym &arg1) override { return apply(_mv(self), arg0, arg1); }
      MNL_HOT val _apply(val &&self, val &&arg0, const val &arg1) override { return apply(_mv(self), _mv(arg0), arg1); }
      MNL_HOT val _apply(val &&self, val &&arg0, val &&arg1) override { return apply(_mv(self), _mv(arg0), _mv(arg1)); }
      MNL_HOT val _apply(val &&self, val &&arg0, const sym &arg1) override { return apply(_mv(self), _mv(arg0), arg1); }
      // For multiple arguments (4 VMT entries)
      MNL_HOT val _apply(const val &self, int argc, val argv[]) override { return apply(self, argc, argv); }
      MNL_HOT val _apply(val &&self, int argc, val argv[]) override { return apply(_mv(self), argc, argv); }
      MNL_HOT val _fetch(const val &self, int argc, val argv[]) override { return fetch(self, argc, argv); }
      MNL_HOT val _fetch(val &&self, int argc, val argv[]) override { return fetch(_mv(self), argc, argv); }
      // For two arguments (6 VMT entries)
      MNL_HOT val _repl(val &&self, const val &key0, const val &value) override { return repl(_mv(self), key0, value); }
      MNL_HOT val _repl(val &&self, const val &key0, val &&value) override { return repl(_mv(self), key0, _mv(value)); }
      MNL_HOT val _repl(val &&self, val &&key0, const val &value) override { return repl(_mv(self), _mv(key0), value); }
      MNL_HOT val _repl(val &&self, val &&key0, val &&value) override { return repl(_mv(self), _mv(key0), _mv(value)); }
      MNL_HOT val _repl(val &&self, const sym &key0, const val &value) override { return repl(_mv(self), key0, value); }
      MNL_HOT val _repl(val &&self, const sym &key0, val &&value) override { return repl(_mv(self), key0, _mv(value)); }
      // For three arguments (12 VMT entries)
      MNL_HOT val _repl(val &&self, const val &key0, const val &key1, const val &value) override { return repl(_mv(self), key0, key1, value); }
      MNL_HOT val _repl(val &&self, const val &key0, const val &key1, val &&value) override { return repl(_mv(self), key0, key1, _mv(value)); }
      MNL_HOT val _repl(val &&self, const val &key0, val &&key1, const val &value) override { return repl(_mv(self), key0, _mv(key1), value); }
      MNL_HOT val _repl(val &&self, const val &key0, val &&key1, val &&value) override { return repl(_mv(self), key0, _mv(key1), _mv(value)); }
      MNL_HOT val _repl(val &&self, const val &key0, const sym &key1, const val &value) override { return repl(_mv(self), key0, key1, value); }
      MNL_HOT val _repl(val &&self, const val &key0, const sym &key1, val &&value) override { return repl(_mv(self), key0, key1, _mv(value)); }
      MNL_HOT val _repl(val &&self, val &&key0, const val &key1, const val &value) override { return repl(_mv(self), _mv(key0), key1, value); }
      MNL_HOT val _repl(val &&self, val &&key0, const val &key1, val &&value) override { return repl(_mv(self), _mv(key0), key1, _mv(value)); }
      MNL_HOT val _repl(val &&self, val &&key0, val &&key1, const val &value) override { return repl(_mv(self), _mv(key0), _mv(key1), value); }
      MNL_HOT val _repl(val &&self, val &&key0, val &&key1, val &&value) override { return repl(_mv(self), _mv(key0), _mv(key1), _mv(value)); }
      MNL_HOT val _repl(val &&self, val &&key0, const sym &key1, const val &value) override { return repl(_mv(self), _mv(key0), key1, value); }
      MNL_HOT val _repl(val &&self, val &&key0, const sym &key1, val &&value) override { return repl(_mv(self), _mv(key0), key1, _mv(value)); }
      // For multiple arguments (2 VMT entries)
      MNL_HOT val _repl(val &&self, int argc, val argv[]) override
         { return repl(_mv(self), argc, argv); }
      MNL_HOT val _repl(val &&self, int argc, val argv[], val *argv_out) override
         { if (!argv_out) __builtin_unreachable(); return repl(_mv(self), argc, argv, argv_out); }
   private:
      template<typename Arg> static MNL_INLINE decltype(auto) _mv(Arg &&arg) noexcept { return std::move(arg); }
   private: // User-specializable
      template<typename Self> MNL_INLINE val invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out)
         { return dat.invoke(std::forward<Self>(self), op, argc, argv, argv_out); static_assert(std::is_base_v<boxable, decltype(dat)>); }
   private:
      template<typename Self, typename Arg0> MNL_INLINE val apply(Self &&self, Arg0 &&arg0)
         { return apply_or_fetch<0>(std::forward<Self>(self), std::forward<Arg0>(arg0)); }
      template<typename Self, typename Arg0> MNL_INLINE val fetch(Self &&self, Key0 &&key0)
         { return apply_or_fetch<1>(std::forward<Self>(self), std::forward<Key0>(key0)); }
      template<bool Op, typename Self, typename Arg0> MNL_INLINE val apply_or_fetch(Self &&self, Arg0 &&arg0)
         { return default_apply_or_fetch<Op>(std::forward<Self>(self), std::forward<Arg0>(arg0)); }
      template<typename Self, typename Arg0, typename Arg1> MNL_INLINE val apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1)
         { return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)); }
      template<typename Self> MNL_INLINE val apply(Self &&self, int argc, val argv[])
         { return apply_or_fetch<0>(std::forward<Self>(self), argc, argv); }
      template<typename Self> MNL_INLINE val fetch(Self &&self, int argc, val argv[])
         { return apply_or_fetch<1>(std::forward<Self>(self), argc, argv); }
      template<bool Op, typename Self> MNL_INLINE val apply_or_fetch(Self &&self, int argc, val argv[])
         { return default_apply_or_fetch<Op>(std::forward<Self>(self), argc, argv); }
      template<typename Key0, typename Val> MNL_INLINE val repl(val &&self, Key0 &&key0, Val &&value)
         { return default_repl(std::move(self), std::forward<Key0>(key0), std::forward<Val>(value)); }
      template<typename Key0, typename Key1, typename Val> MNL_INLINE val repl(val &&self, Key0 &&key0, Key1 &&key1, Val &&value)
         { return default_repl(std::move(self), std::forward<Key0>(key0), std::forward<Key1>(key1), std::forward<Val>(value)); }
      MNL_INLINE val repl(val &&self, int argc, val argv[], val *argv_out = {})
         { return default_repl(std::move(self), argc, argv, argv_out); }
   private: // Utilities for forwarding to "invoke"
      template< typename Self, typename Arg0, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val> &&
         std::is_same_v<Arg0, const val &> | std::is_same_v<Arg0, val> | std::is_same_v<Arg0, const sym &>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_apply(Self &&self, Arg0 &&arg0)
         { return default_apply_or_fetch<0>(std::forward<Self>(self), std::forward<Arg0>(arg0)); }
      template< typename Self, typename Arg0, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val> &&
         std::is_same_v<Arg0, const val &> | std::is_same_v<Arg0, val> | std::is_same_v<Arg0, const sym &>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_fetch(Self &&self, Arg0 &&arg0)
         { return default_apply_or_fetch<1>(std::forward<Self>(self), std::forward<Arg0>(arg0)); }
      template< bool Op, typename Self, typename Arg0, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val> &&
         std::is_same_v<Arg0, const val &> | std::is_same_v<Arg0, val> | std::is_same_v<Arg0, const sym &>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_apply_or_fetch(Self &&self, Arg0 &&arg0) {
         return _invoke(std::forward<Self>(self), sym::from_id<sym::id(Op ? "Apply" : "Fetch")>, 1,
            &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Arg0, val>, val &, val>)arg0));
      }
      template<typename Self, typename Arg0, typename Arg1, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val> &&
         std::is_same_v<Arg0, const val &> | std::is_same_v<Arg0, val> &&
         std::is_same_v<Arg1, const val &> | std::is_same_v<Arg1, val> | std::is_same_v<Arg1, const sym &>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1) {
         val argv[] = {std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)};
         return _invoke(std::forward<Self>(self), sym::from_id<sym::id("Apply")>, std::size(argv), argv);
      }
      template< typename Self, typename Arg0, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_apply(Self &&self, int argc, val argv[])
         { default_apply_or_fetch<0>(std::forward<Self>(self), argc, argv); }
      template< typename Self, typename Arg0, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_fetch(Self &&self, int argc, val argv[])
         { default_apply_or_fetch<1>(std::forward<Self>(self), argc, argv); }
      template< bool Op, typename Self, typename Arg0, std::enable_if_t<
         std::is_same_v<Self, const val &> | std::is_same_v<Self, val>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_apply_or_fetch(Self &&self, int argc, val argv[]) {
         return _invoke(std::forward<Self>(self), sym::from_id<sym::id(Op ? "Apply" : "Fetch")>, argc, argv);
      }
      template<typename Self, typename Key0, typename Val, std::enable_if_t<
         std::is_same_v<Self, val> &&
         std::is_same_v<Key0, const val &> | std::is_same_v<Key0, val> | std::is_same_v<Key0, const sym &> &&
         std::is_same_v<Val,  const val &> | std::is_same_v<Val,  val>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_repl(Self &&self, Key0 &&key0, Val &&value) {
         val argv[] = {std::forward<Key0>(key0), std::forward<Val>(value)};
         return _invoke(std::move(self), sym::from_id<sym::id("Repl")>, std::size(argv), argv);
      }
      template<typename Self, typename Key0, typename Key1, typename Val, std::enable_if_t<
         std::is_same_v<Self, val> &&
         std::is_same_v<Key0, const val &> | std::is_same_v<Key0, val> &&
         std::is_same_v<Key1, const val &> | std::is_same_v<Key1, val> | std::is_same_v<Key1, const sym &> &&
         std::is_same_v<Val,  const val &> | std::is_same_v<Val,  val>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_repl(Self &&self, Key0 &&key0, Key1 &&key1, Val &&value) {
         val argv[] = {std::forward<Key0>(key0), std::forward<Key1>(key1), std::forward<Val>(value)};
         return _invoke(std::move(self), sym::from_id<sym::id("Repl")>, std::size(argv), argv);
      }
      template<typename Self, std::enable_if_t<
         std::is_same_v<Self, val>,
         decltype(nullptr) > = decltype(nullptr){} >
      MNL_INLINE val default_repl(Self &&self, int argc, val argv[], val *argv_out) {
         return _invoke(std::move(self), sym::from_id<sym::id("Repl")>, argc, argv, argv_out);
      }
   };

   template<> val box<std::string>::invoke(val &&, const sym &, int, val [], val *);
   template<> val box<std::pair<std::vector<ast>, loc>>::invoke(val &&, const sym &, int, val [], val *);
   template<> val box<std::vector<val>>::invoke(val &&, const sym &, int, val [], val *);
   template<> inline box<std::vector<val>>::~box() { while (!dat.empty()) dat.pop_back(); }

   MNL_NOINLINE inline val::val(const char *dat): val((string)dat) {} // postponed definition because the complete type box<std::string> was needed
   // postponed definitions because the complete types box<std::vector<ast>>, box<std::pair<std::vector<ast>, loc>> were needed:
   MNL_INLINE inline ast::val(vector<ast> first, loc second)
      : ast(make_pair(move(first), move(second))) {}
   MNL_INLINE inline bool ast::is_list() const noexcept
      { return test<vector<ast>>() || test<pair<vector<ast>, loc>>(); }
   MNL_INLINE inline std::vector<ast>::const_iterator ast::begin() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).begin(); }
   MNL_INLINE inline std::vector<ast>::const_iterator ast::end() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).end(); }
   MNL_INLINE inline std::vector<ast>::const_reverse_iterator ast::rbegin() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).rbegin(); }
   MNL_INLINE inline std::vector<ast>::const_reverse_iterator ast::rend() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).rend(); }
   MNL_INLINE inline bool ast::empty() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).empty(); }
   MNL_INLINE inline long ast::size() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).size(); }
   MNL_INLINE inline const ast &ast::operator[](long sn) const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first)[sn]; }
   MNL_INLINE inline const ast &ast::at(long sn) const
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).at(sn); }
   MNL_INLINE inline const ast &ast::front() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).front(); }
   MNL_INLINE inline const ast &ast::back() const noexcept
      { return (test<vector<ast>>() ? cast<const vector<ast> &>() : cast<const pair<vector<ast>, loc> &>().first).back(); }
   MNL_INLINE inline const loc &ast::_loc(const loc &_loc) const noexcept
      { return test<pair<vector<ast>, loc>>() ? cast<const pair<vector<ast>, loc> &>().second : _loc; }
   MNL_INLINE inline ast::vci_range  ast::operator+(long sn) const noexcept { return test<vector<ast>>() ?
      vci_range {cast<const vector<ast> &>().begin() + sn, cast<const vector<ast> &>().end()} :
      vci_range {cast<const pair<vector<ast>, loc> &>().first.begin() + sn, cast<const pair<vector<ast>, loc> &>().first.end()}; }
   MNL_INLINE inline ast::vcri_range ast::operator-(long sn) const noexcept { return test<vector<ast>>() ?
      vcri_range{cast<const vector<ast> &>().rbegin(), cast<const vector<ast> &>().rend() - sn} :
      vcri_range{cast<const pair<vector<ast>, loc> &>().first.rbegin(), cast<const pair<vector<ast>, loc> &>().first.rend() - sn}; }

// Resource Management Helpers /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_INLINE inline void val::hold() const noexcept {
      if (MNL_UNLIKELY(rep.tag() >= 0xFFF8 + 0b110))
      if (MNL_UNLIKELY(rep.tag() == 0xFFF8 + 0b110)) // Sym
         rep.dat<const sym &>().hold();
      else // BoxPtr (fallback)
         MNL_IF_WITHOUT_MT(++static_cast<root *>(rep.dat<void *>())->_rc)
         MNL_IF_WITH_MT(__atomic_add_fetch(&static_cast<root *>(rep.dat<void *>())->_rc, 1, __ATOMIC_RELAXED));
   }
   MNL_INLINE inline void val::unhold() const noexcept {
      if (MNL_UNLIKELY(rep.tag() >= 0xFFF8 + 0b110))
      if (MNL_UNLIKELY(rep.tag() == 0xFFF8 + 0b110)) // Sym
         rep.dat<const sym &>().unhold();
      else // BoxPtr (fallback)
      if (MNL_UNLIKELY(!
         MNL_IF_WITHOUT_MT(--static_cast<root *>(rep.dat<void *>())->_rc)
         MNL_IF_WITH_MT(__atomic_sub_fetch(&static_cast<root *>(rep.dat<void *>())->_rc, 1, __ATOMIC_ACQ_REL)) ))
         delete static_cast<root *>(rep.dat<void *>());
   }

// val Extractors //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE inline bool val::is() const noexcept {
      return MNL_LIKELY(rep.tag() == 0xFFF8 + 0b111) && static_cast<const root *>(rep.dat<void *>())->_tag ==
         (decltype(root::_tag))reinterpret_cast<std::uintptr_t>(&box<std::decay_t<Dat>>::_tag);
   }
   template<typename Dat> MNL_INLINE inline Dat val::as() const noexcept(std::is_nothrow_copy_constructible_v<Dat>) {
      return static_cast<box<std::decay_t<Dat>> *>(static_cast<root *>(rep.dat<void *>()))->dat;
   }
   MNL_INLINE inline long val::rc() const noexcept
      { return static_cast<const root *>(rep.dat<void *>())->rc(); }

   // Fake test/cast for val outputs
   template<> MNL_INLINE inline bool val::test<const val &>() const noexcept { return true; }
   template<> MNL_INLINE inline bool val::test<val>() const noexcept         { return test<const val &>(); }
   template<> MNL_INLINE inline const val &val::cast() const noexcept { return *this; }
   template<> MNL_INLINE inline val        val::cast() const noexcept { return cast<const val &>(); }

   template<> MNL_INLINE inline bool val::test<decltype(nullptr)>() const noexcept { return rep.tag() == 0b000 - 8; }
   template<> MNL_INLINE inline decltype(nullptr) val::cast() const noexcept { return nullptr; }
   MNL_INLINE inline bool val::operator==(decltype(nullptr)) const noexcept { return test<>(); }

   template<> MNL_INLINE inline bool val::test<long long>() const noexcept { return rep.tag() == 0b001 - 8; }
   template<> MNL_INLINE inline bool val::test<int>() const noexcept       { return test<long long>(); }
   template<> MNL_INLINE inline long long val::cast() const noexcept { return rep.dat<long long>(); }
   template<> MNL_INLINE inline int       val::cast() const noexcept { return cast<long long>(); }
   template<> MNL_INLINE inline auto val::test<const long long &>() const noexcept { return test<long long>(); }
   template<> MNL_INLINE inline auto val::cast<const long long &>() const noexcept { return cast<long long>(); }
   template<> MNL_INLINE inline auto val::test<const int &>() const noexcept { return test<int>(); }
   template<> MNL_INLINE inline auto val::cast<const int &>() const noexcept { return cast<int>(); }

   template<> MNL_INLINE inline bool val::test<double>() const noexcept { return rep.tag() < 0xFFF8 + 0b000; }
   template<> MNL_INLINE inline double val::cast() const noexcept { return rep.dat<double>(); }
   template<> MNL_INLINE inline auto val::test<const double &>() const noexcept { return test<double>(); }
   template<> MNL_INLINE inline auto val::cast<const double &>() const noexcept { return cast<double>(); }

   template<> MNL_INLINE inline bool val::test<float>() const noexcept { return rep.tag() == 0xFFF8 + 0b010; }
   template<> MNL_INLINE inline float val::cast() const noexcept { return rep.dat<float>(); }

   template<> MNL_INLINE inline bool val::test<const sym &>() const noexcept { return rep.tag() == 0xFFF8 + 0b110; }
   template<> MNL_INLINE inline bool val::test<sym>() const noexcept         { return test<const sym &>(); }
   template<> MNL_INLINE inline const sym &val::cast() const noexcept { return rep.dat<const sym &>(); }
   template<> MNL_INLINE inline sym        val::cast() const noexcept { return cast<const sym &>(); }
   MNL_INLINE inline bool val::operator==(const sym &rhs) const noexcept { return test<sym>() && cast<const sym &>() == rhs; }

   template<> MNL_INLINE inline bool val::test<bool>() const noexcept { return rep.tag() == 0xFFF8 + 0b100 | rep.tag() == 0xFFF8 + 0b101; }
   template<> MNL_INLINE inline bool val::cast() const noexcept { return rep.tag() & true; }

   template<> MNL_INLINE inline bool val::test<unsigned>() const noexcept { return rep.tag() == 0xFFF8 + 0b011; }
   template<> MNL_INLINE inline bool val::test<char>() const noexcept     { return test<unsigned>(); }
   template<> MNL_INLINE inline unsigned val::cast() const noexcept { return rep.dat<unsigned>(); }
   template<> MNL_INLINE inline char     val::cast() const noexcept { return cast<unsigned>(); }

   template<> MNL_INLINE inline bool val::test<const char *>() const noexcept { return test<std::string>; }
   template<> MNL_INLINE inline const char *val::cast() const noexcept { return cast<const std::string &>().c_str(); }

// Signals, Exceptions, and Invocation Traces //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   extern MNL_IF_WITH_MT(thread_local) pair<sym, val>                  sig_state;
   extern MNL_IF_WITH_MT(thread_local) vector<pair<loc, const char *>> sig_trace;
}} // namespace aux::pub
   # define MNL_ERR(...) ::mnl::error([&]()noexcept->const ::mnl::sym &{ return (__VA_ARGS__); })
   namespace aux { MNL_NORETURN void error(const sym &); }
   namespace aux { namespace pub { template<typename Err> MNL_NORETURN void error(const Err &err) { aux::error(err()); } } }
   # define MNL_ERR_LOC(LOC, ...) ::mnl::error((LOC), [&]()noexcept->const ::mnl::sym &{ return (__VA_ARGS__); })
   namespace aux { MNL_NORETURN void error(const loc &, const sym &); }
   namespace aux { namespace pub { template<typename Err> MNL_NORETURN void error(const loc &loc, const Err &err) { aux::error(loc, err()); } } }
namespace aux { namespace pub {
   MNL_NORETURN void trace_execute (const loc &);
   MNL_NORETURN void trace_exec_in (const loc &);
   MNL_NORETURN void trace_exec_out(const loc &);
}} // namespace aux::pub

// Translation Infrastructure //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   ast parse(const string &, string origin = "<anonymous>");
   MNL_INLINE inline ast parse(string &&source, string origin = "<anonymous>") // parse won't take advantage of source's resources, but
      { auto res = parse(source, move(origin)); source.clear(); return res; } // this is to release its resources earlier, which is useful in practice
   typedef ast form; // AST when it is to be compiled as a whole - for documentation purposes





   class code { // compiled entity
   public: // Standard operations
      code() = default;
      MNL_INLINE code(const code &rhs) noexcept: rep(rhs.rep) { hold(); }
      MNL_INLINE code(code &&rhs) noexcept: rep(rhs.rep) { rhs.rep = {}; }
      MNL_INLINE ~code() { unhold(); }
      MNL_INLINE code &operator=(const code &rhs) noexcept { rhs.hold(), unhold(), rep = rhs.rep; return *this; }
      MNL_INLINE code &operator=(code &&rhs) noexcept { swap(rhs); return *this; }
      MNL_INLINE void swap(code &rhs) noexcept { using std::swap; swap(rep, rhs.rep); }
      MNL_INLINE friend bool operator==(const code &lhs, const code &rhs) noexcept { return lhs.rep == rhs.rep; }
      MNL_INLINE explicit operator bool() const noexcept { return rep; }
   public: // Construction -- Implicit conversion (to) + Compilation/execution operations
      template<typename Dat> code(Dat dat): rep(new box<Dat>{std::move(dat)}) {}
      MNL_INLINE code compile(const form &form, const loc &loc) && { return rep->compile(std::move(*this), form, loc); }
      template<bool fast_sig = bool{}, bool nores = bool{}>
      MNL_INLINE auto execute() const { return rep->execute(std::bool_constant<fast_sig>{}, std::bool_constant<nores>{}); }
      MNL_INLINE void exec_in(const val &value) const { rep->exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const      { rep->exec_in(std::move(value)); }
      MNL_INLINE val  exec_out() const { return rep->exec_out(); }
      MNL_INLINE bool is_rvalue() const noexcept { return rep->is_rvalue(); }
      MNL_INLINE bool is_lvalue() const noexcept { return rep->is_lvalue(); } // implies is_rvalue()
   public:
      MNL_INLINE code compile(const form &form, const loc &loc) const & // for API completeness
         { if (*this); else __builtin_unreachable(); return ((code)*this).compile(form, loc); }
   public: // Extraction
      template<typename Dat> MNL_INLINE friend bool is(const code &rhs) noexcept
         { return rhs.rep->tag == (decltype(root::tag))reinterpret_cast<std::uintptr_t>(&box<std::remove_cv_t<std::remove_reference_t<Dat>>>::tag); }
      template<typename Dat> MNL_INLINE friend Dat  as(const code &rhs) noexcept
         { return static_cast<box<std::remove_cv_t<std::remove_reference_t<Dat>>> *>(rhs.rep)->dat; }


      template<typename Dat> MNL_INLINE friend bool test(const code &rhs) noexcept
         { return typeid(*rhs.rep) == typeid(box<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type>); }
      template<typename Dat> MNL_INLINE friend Dat  cast(const code &rhs) noexcept
         { return static_cast<box<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type> *>(rhs.rep)->dat; }

   public: // Required bases for Dat
      struct nonvalue; struct rvalue; struct lvalue;
   private: // Concrete representation
      class root {
      public:
         const unsigned tag; // assume 64-bit small/medium code model or x32 ABI or 32-bit ISA
         MNL_NOTE(atomic) long rc = 1;
      protected:
         MNL_INLINE explicit root(const std::byte *tag) noexcept: tag(reinterpret_cast<std::uintptr_t>(tag)) {}
         virtual ~root() = default;
      private:
         root(const root &) = delete;
         root &operator=(const root &) = delete; // would be implicitly deleted anyway
      public:
         virtual code compile(code &&self, const form &, const loc &) const = 0;
         MNL_HOT virtual val  execute(std::false_type, std::false_type) const = 0, 
         MNL_HOT virtual void execute(std::false_type, std::true_type)  const = 0, 
         MNL_HOT virtual val  execute(std::true_type,  std::false_type) const = 0;
         MNL_HOT virtual void execute(std::true_type,  std::true_type)  const = 0;
         MNL_HOT virtual void exec_in(const val &) const = 0, exec_in(val &&) const = 0;
         MNL_HOT virtual val  exec_out() const = 0;
         MNL_PURE virtual bool is_rvalue() const noexcept = 0;
         MNL_PURE virtual bool is_lvalue() const noexcept = 0; // shall imply is_rvalue()
      } *rep = {};
      template<class Dat> class box final: public root {
      public:
         const Dat dat;
         static_assert(std::is_base_of_v<nonvalue, Dat>);
         explicit box(Dat &&dat) noexcept: root{&tag}, dat(std::move(dat)) {}
         static std::byte tag; // custom RTTI
      private:
         code compile(code &&self, const form &form, const loc &loc) const override { return dat.compile(std::move(self), form, loc); }
         MNL_HOT val  execute(std::false_type, std::false_type) const override { return dat.execute<>(); }
         MNL_HOT void execute(std::false_type, std::true_type)  const override { (void)dat.execute<bool{}, true>(); }
         MNL_HOT val  execute(std::true_type,  std::false_type) const override { return dat.execute<true>(); }
         MNL_HOT void execute(std::true_type,  std::true_type)  const override { (void)dat.execute<true, true>(); }
         MNL_HOT void exec_in(const val &value) const override { dat.exec_in(value); }
         MNL_HOT void exec_in(val &&value)      const override { dat.exec_in(std::move(value)); }
         MNL_HOT val  exec_out() const override { return dat.exec_out(); }
         bool is_rvalue() const noexcept override { return dat.is_rvalue(); }
         bool is_lvalue() const noexcept override { return dat.is_lvalue(); } // shall imply is_rvalue()
      };
   private: // Implementation helpers
      MNL_INLINE void hold() const noexcept
         { if (MNL_LIKELY(rep)) MNL_IF_WITHOUT_MT(++rep->rc) MNL_IF_WITH_MT(__atomic_add_fetch(&rep->rc, 1, __ATOMIC_RELAXED)); }
      MNL_INLINE void unhold() const noexcept
         { if (MNL_LIKELY(rep) && MNL_UNLIKELY(! MNL_IF_WITHOUT_MT(--rep->rc) MNL_IF_WITH_MT(__atomic_sub_fetch(&rep->rc, 1, __ATOMIC_ACQ_REL)) )) delete rep; }
   private: // Support for <expr># expressions
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) { return self.default_invoke(op, argc, argv); }
      friend mnl::box<code>;
   };
   template<typename Dat> std::byte code::box<Dat>::tag;
   MNL_INLINE inline void swap(code &lhs, code &rhs) noexcept { lhs.swap(rhs); }
   bool operator==(const code &, const code &) noexcept;
   MNL_INLINE inline bool operator!=(const code &lhs, const code &rhs) noexcept { return std::rel_ops::operator!=(lhs, rhs); }



   struct code::nonvalue {
      static code compile(code &&self, const form &, const loc &) = delete;
      template<bool fast_sig = bool{}, bool nores = bool{}> MNL_INLINE static val execute() noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static void exec_in(const val &) noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static val  exec_out() noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static bool is_rvalue() noexcept { return {}; }
      MNL_INLINE static bool is_lvalue() noexcept { return {}; }
   };
   struct code::rvalue: code::nonvalue {
      MNL_INLINE static code compile(code &&self, const form &form, const loc &loc) { return aux::compile_apply(std::move(self), form, loc); }
      template<bool fast_sig = bool{}, bool nores = bool{}> static val execute() = delete;
      MNL_INLINE static bool is_rvalue() noexcept { return true; }
   };
   struct code::lvalue: code::rvalue {
      static void exec_in(const val &) = delete;
      static val exec_out() = delete;
      MNL_INLINE static bool is_lvalue() noexcept { return true; }
   };




   class code /*compiled entity*/ {
   public: // Standard operations
      code() = default;
      MNL_INLINE code(const code &rhs) noexcept: rep(rhs.rep) { addref(); }
      MNL_INLINE code(code &&rhs) noexcept: rep(rhs.rep) { rhs.rep = {}; }
      MNL_INLINE ~code() { release(); }
      MNL_INLINE code &operator=(const code &rhs) noexcept { rhs.addref(), release(), rep = rhs.rep; return *this; }
      MNL_INLINE code &operator=(code &&rhs) noexcept { swap(rhs); return *this; }
      MNL_INLINE void swap(code &rhs) noexcept { using std::swap; swap(rep, rhs.rep); }
      MNL_INLINE friend bool operator==(const code &lhs, const code &rhs) noexcept { return lhs.rep == rhs.rep; }
      MNL_INLINE explicit operator bool() const noexcept { return rep; }
   public: // Construction -- Implicit conversion (to) + Compilation/execution operations
      template<typename Dat> code(Dat dat): rep(new box<Dat>{(move)(dat)}) {}
      MNL_INLINE code compile(const form &form, const loc &loc) && { return rep->compile(move(*this), form, loc); }
      MNL_INLINE val  execute(bool fast_sig = {}) const { return rep->execute(fast_sig); }
      MNL_INLINE void exec_in(val &&val) const { rep->exec_in(move(val)); }
      MNL_INLINE val  exec_out() const { return rep->exec_out(); }
      MNL_INLINE bool is_rvalue() const noexcept { return rep->is_rvalue(); }
      MNL_INLINE bool is_lvalue() const noexcept { return rep->is_lvalue(); } // implies is_rvalue()
   public: // Extraction
      template<typename Dat> MNL_INLINE friend bool test(const code &rhs) noexcept
         { return typeid(*rhs.rep) == typeid(box<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type>); }
      template<typename Dat> MNL_INLINE friend Dat  cast(const code &rhs) noexcept
         { return static_cast<box<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type> *>(rhs.rep)->dat; }
   private: // Concrete representation
      class root { public:
         /*atomic*/ long rc = 1;
         virtual ~root() = default;
         virtual code compile(code &&self, const form &, const loc &) const = 0;
         virtual val  execute(bool fast_sig) const = 0;
         virtual void exec_in(val &&) const = 0;
         virtual val  exec_out() const = 0;
         virtual bool is_rvalue() const noexcept = 0;
         virtual bool is_lvalue() const noexcept = 0; // shall imply is_rvalue()
      } *rep{};
      template<typename Dat> class box final: public root { public:
         const Dat dat;
         explicit box(Dat dat): dat((move)(dat)) {}
         code compile(code &&self, const form &form, const loc &loc) const override { return dat.compile(move(self), form, loc); }
         val  execute(bool fast_sig) const override { return dat.execute(fast_sig); }
         void exec_in(val &&val) const override { dat.exec_in(move(val)); }
         val  exec_out() const override { return dat.exec_out(); }
         bool is_rvalue() const noexcept override { return dat.is_rvalue(); }
         bool is_lvalue() const noexcept override { return dat.is_lvalue(); } // shall imply is_rvalue()
      };
   private: // Implementation helpers
      MNL_INLINE void addref() const noexcept
         { if (MNL_LIKELY(rep)) MNL_IF_WITHOUT_MT(++rep->rc) MNL_IF_WITH_MT(__atomic_add_fetch(&rep->rc, 1, __ATOMIC_RELAXED)); }
      MNL_INLINE void release() const noexcept
         { if (MNL_LIKELY(rep) && MNL_UNLIKELY(! MNL_IF_WITHOUT_MT(--rep->rc) MNL_IF_WITH_MT(__atomic_sub_fetch(&rep->rc, 1, __ATOMIC_ACQ_REL)) )) delete rep; }
   private: // Support for <expr># expressions
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) { return self.default_invoke(op, argc, argv); }
      friend mnl::box<code>;
   };
   MNL_INLINE inline void swap(code &lhs, code &rhs) noexcept { lhs.swap(rhs); }
   bool operator==(const code &, const code &) noexcept;
   MNL_INLINE inline bool operator!=(const code &lhs, const code &rhs) noexcept { return std::rel_ops::operator!=(lhs, rhs); }
   template<typename>     bool test(const code &) noexcept;
   template<typename Dat> Dat  cast(const code &) noexcept;

   extern MNL_IF_WITH_MT(thread_local) sym::tab<> symtab;
   code compile(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});
   MNL_NORETURN void err_compile(const char *msg, const loc &);
}} // namespace aux::pub

// Primitive Operations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux {

   using std::isinf;
   constexpr auto err_Overflow = []() MNL_INLINE { MNL_ERR(MNL_SYM("Overflow")); }; // to enable basic block merging (esp. in hot section)

   // I48 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // 48-bit addition, subtraction
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat> _add(Dat lhs, Dat rhs)
      { long long res = lhs + rhs; if (MNL_LIKELY(res >= val::min_i48 & res <= val::max_i48)) return res; err_Overflow(); } // for hot paths
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat> _sub(Dat lhs, Dat rhs)
      { long long res = lhs - rhs; if (MNL_LIKELY(res >= val::min_i48 & res <= val::max_i48)) return res; err_Overflow(); } // for hot paths
   // 48-bit multiplication
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat> _mul(Dat lhs, Dat rhs) {
      long long res; bool ok = !__builtin_mul_overflow(lhs, rhs, &res);
      if (MNL_LIKELY(ok) && MNL_LIKELY(res >= val::min_i48 & res <= val::max_i48)) return res;
      err_Overflow(); // for hot paths
   }
   // 48-bit (symmetric-range) negation (unary minus), absolute value (magnitude)
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat> _neg(Dat arg) { return -arg; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat> _abs(Dat arg) { using std::abs; return abs(arg); }

   // F64, F32 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // IEEE754 addition, subtraction, multiplication
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat> _add(Dat lhs, Dat rhs)
      { auto res = lhs + rhs; if (MNL_LIKELY(!isinf(res))) return res; err_Overflow(); } // for hot paths
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat> _sub(Dat lhs, Dat rhs)
      { auto res = lhs - rhs; if (MNL_LIKELY(!isinf(res))) return res; err_Overflow(); } // for hot paths
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat> _mul(Dat lhs, Dat rhs)
      { auto res = lhs * rhs; if (MNL_LIKELY(!isinf(res))) return res; err_Overflow(); } // for hot paths
   // negation (unary minus), absolute value (magnitude)
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat> _neg(Dat arg)
      { return -arg; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat> _abs(Dat arg)
      { using std::abs; return abs(arg); }

   // U32, Bool ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // 32-bit modular (wraparound) addition, subtraction, multiplication
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat> _add(Dat lhs, Dat rhs) { return lhs + rhs; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat> _sub(Dat lhs, Dat rhs) { return lhs - rhs; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat> _mul(Dat lhs, Dat rhs) { return lhs * rhs; }
   // boolean/logical and bitwise "and", "or", "xor" (exclusive-"or")
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned> | std::is_same_v<Dat, bool>, Dat> _and(Dat lhs, Dat rhs)
      { return lhs & rhs; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned> | std::is_same_v<Dat, bool>, Dat> _or (Dat lhs, Dat rhs)
      { return lhs | rhs; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned> | std::is_same_v<Dat, bool>, Dat> _xor(Dat lhs, Dat rhs)
      { return lhs ^ rhs; }
   // negation (2's-complement), absolute value (identity), bitwise "not" (complement)
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat> _neg(Dat arg) { return -arg; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat> _abs(Dat arg) { return +arg; }
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat> _not(Dat arg) { return ~arg; }
   // boolean/logical "not" (negation)
   template<typename Dat> MNL_INLINE inline std::enable_if_t<std::is_same_v<Dat, bool>, Dat> _not(Dat arg) { return !arg; }

} // namespace aux

   // val::Apply/Fetch/Repl ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Apply/Fetch

   template<typename Self, typename Arg0>
   MNL_INLINE inline val val::_apply(Self &&self, Arg0 &&arg0) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _apply(std::forward<Self>(self), std::forward<Arg0>(arg0));
      if (MNL_LIKELY(is<sym>(self))) // Sym
         return as<const sym &>(self)(std::forward<Arg0>(arg0));
      err_UnrecognizedOperation();
   }
   template<typename Self, typename Key0>
   MNL_INLINE inline val val::_fetch(Self &&self, Key0 &&key0) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _fetch(std::forward<Self>(self), std::forward<Key0>(key0));
      err_UnrecognizedOperation();
   }
   template<typename Self, typename Arg0, typename Arg1>
   MNL_INLINE inline val val::_apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _apply(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
      if (MNL_LIKELY(is<sym>(self))) // Sym
         return as<const sym &>(self)(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
      err_UnrecognizedOperation();
   }
   template<typename Self>
   MNL_INLINE inline val val::_apply(Self &&self, int argc, val argv[]) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _apply(std::forward<Self>(self), argc, argv);
      if (MNL_LIKELY(is<sym>(self))) // Sym
         return as<const sym &>(self)(argc, argv);
      err_UnrecognizedOperation();
   }
   template<typename Self>
   MNL_INLINE inline val val::_fetch(Self &&self, int argc, val argv[]) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _fetch(std::forward<Self>(self), argc, argv);
      err_UnrecognizedOperation();
   }
   template<typename Self>
   MNL_INLINE inline val val::_apply(Self &&self, int argc, val argv[], val *argv_out) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _invoke(std::forward<Self>(self), sym::from_id<sym::id("Apply")>, argc, argv, argv_out);
      if (MNL_LIKELY(is<sym>(self))) // Sym
         return as<const sym &>(self)(argc, argv, argv_out);
      err_UnrecognizedOperation();
   }

   // Repl

   template<typename Self, typename Key0, typename Val>
   MNL_INLINE inline val val::_repl(Self &&self, Key0 &&key0, Val &&value) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _repl(std::forward<Self>(self), std::forward<Key0>(key0), std::forward<Val>(value));
      err_UnrecognizedOperation();
   }
   template<typename Self, typename Key0, typename Key1, typename Val>
   MNL_INLINE inline val val::_repl(Self &&self, Key0 &&key0, Key1 &&key1, Val &&value) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _repl(std::forward<Self>(self), std::forward<Key0>(key0), std::forward<Key1>(key1), std::forward<Val>(value));
      err_UnrecognizedOperation();
   }
   template<typename Self>
   MNL_INLINE inline val val::_repl(Self &&self, int argc, val argv[]) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _repl(std::forward<Self>(self), argc, argv);
      err_UnrecognizedOperation();
   }
   template<typename Self>
   MNL_INLINE inline val val::_repl(Self &&self, int argc, val argv[], val *argv_out) {
      if (MNL_LIKELY(self.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->
            _repl(std::forward<Self>(self), argc, argv, argv_out);
      err_UnrecognizedOperation();
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   struct val::ops { // empty (aggregate), for code organization and access control
   private:
      static constexpr auto err_UnrecognizedOperation = []() MNL_INLINE{ MNL_ERR(MNL_SYM("UnrecognizedOperation")); };
      static constexpr auto err_TypeMismatch          = []() MNL_INLINE{ MNL_ERR(MNL_SYM("TypeMismatch"));          };
   private:
      static constexpr auto err_numeric = [](const val &lhs) MNL_INLINE{
         MNL_ERR(is<long long>(lhs) || is<double>(lhs) || is<float>(lhs) || is<unsigned>(lhs) ?
            MNL_SYM("TypeMismatch") : MNL_SYM("UnrecognizedOperation", &lhs);
      };
      static constexpr auto err_bitwise = [](const val &lhs) MNL_INLINE{
         MNL_ERR(test<unsigned>(lhs) ?
            MNL_SYM("TypeMismatch") : MNL_SYM("UnrecognizedOperation", &lhs);
      };
   private:
      template<enum sym::id Id> class val::ops::_op { // surrogate used instead of a sym
      private:
         explicit _op() = default;
         friend ops;
      public:
         MNL_INLINE operator const sym &() const noexcept { return sym::from_id<Id>; }
      public:
         MNL_INLINE val operator()(const val  &lhs, const val  &rhs) const { return _apply(          lhs ,           rhs ); } // argument
         MNL_INLINE val operator()(const val  &lhs,       val &&rhs) const { return _apply(          lhs , std::move(rhs)); } // implicit
         MNL_INLINE val operator()(      val &&lhs, const val  &rhs) const { return _apply(std::move(lhs),           rhs ); } // conversion
         MNL_INLINE val operator()(      val &&lhs,       val &&rhs) const { return _apply(std::move(lhs), std::move(rhs)); } // enabled
      private:
         template<typename Lhs, typename Rhs, std::enable_if_t< // SFINAE requirements in internal function for disambiguation
            std::is_same_v<std::decay_t<Lhs>, val> && std::is_same_v<std::decay_t<Rhs>, val>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE static val _apply(Lhs &&lhs, Rhs &&rhs) {
            if (bool{});
            else if constexpr (Id == sym::id("==") | Id == sym::id("<>"))
               switch (lhs.rep.tag()) MNL_NOTE(jumptable) {
               default             /*F64*/:               return (*this)(as<double>(lhs),      rhs);
               case 0xFFF8 + 0b111 /*BoxPtr (fallback)*/: return static_cast<root *>(lhs.rep.template dat<void *>())->_invoke(std::forward<Lhs>(lhs),
                  *this, 1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs));
               case 0xFFF8 + 0b000 /*Nil*/:               return (*this)(nullptr               rhs);
               case 0xFFF8 + 0b001 /*I48*/:               if (std::memcmp(&lhs, &rhs, sizeof(val))) return Id == sym::id("<>"); else
                  { if (is<long long>(rhs) && as<long long>(rhs) == as<long long>(lhs)); else __builtin_unreachable(); return Id == sym::id("=="); }
               case 0xFFF8 + 0b010 /*F32*/:               return (*this)(as<float>(lhs),       rhs);
               case 0xFFF8 + 0b110 /*Sym*/:               return (*this)(as<const sym &>(lhs), rhs);
               case 0xFFF8 + 0b100 /*Bool/False*/:        return (*this)(false,                rhs);
               case 0xFFF8 + 0b101 /*Bool/True*/:         return (*this)(true,                 rhs);
               case 0xFFF8 + 0b011 /*U32*/:               return (*this)(as<unsigned>(lhs),    rhs);
               }
            else if constexpr (
               Id == sym::id("+") | Id == sym::id("-" ) | Id == sym::id("*") |
               Id == sym::id("<") | Id == sym::id("<=") | Id == sym::id(">") | Id == sym::id(">=" ) )
               switch (lhs.rep.tag()) MNL_NOTE(jumptable) {
               case 0xFFF8 + 0b000: case 0xFFF8 + 0b100: case 0xFFF8 + 0b101: case 0xFFF8 + 0b110:
                  err_UnrecognizedOperation();
               case 0xFFF8 + 0b111 /*BoxPtr (fallback)*/: return static_cast<root *>(lhs.rep.template dat<void *>())->_invoke(std::forward<Lhs>(lhs),
                  *this, 1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs));
               case 0xFFF8 + 0b001 /*I48*/:               return (*this)(as<long long>(lhs),   rhs);
               default             /*F64*/:               return (*this)(as<double>(lhs),      rhs);
               case 0xFFF8 + 0b010 /*F32*/:               return (*this)(as<float>(lhs),       rhs);
               case 0xFFF8 + 0b011 /*U32*/:               return (*this)(as<unsigned>(lhs),    rhs);
               }
            else if constexpr (Id == sym::id("Xor") | Id == sym::id("&") | Id == sym::id("|"))
               if (bool{}); // TODO: we can also use if-return w/o else
               else if (MNL_UNLIKELY(is<unsigned>(lhs()))) // U32
                  return (*this)(as<unsigned>(lhs), rhs);
               else if (MNL_LIKELY(is<bool>(lhs()))) // Bool
                  return (*this)(as<bool>(lhs), rhs);
               else if (MNL_LIKELY(lhs.rep.tag() == 0xFFF8 + 0b111)) // BoxPtr (fallback)
                  return static_cast<root *>(lhs.rep.template dat<void *>())->_invoke(std::forward<Lhs>(lhs),
                     *this, 1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs));
               else
                  err_UnrecognizedOperation();
            else
               return ((const sym &)*this)(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs)); // for completeness
         }
      public:
      // "specializations" derivable by scalar value propagation in _apply; not absolutely necessary, but used in _apply as part of its architecture
         // core numeric
         template< typename Lhs, class Rhs, std::enable_if_t<
            std::is_same_v<Lhs, long long> | std::is_same_v<Lhs, double> | std::is_same_v<Lhs, float> | std::is_same_v<Lhs, unsigned> &&
            std::is_same_v<Rhs, val>, decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE auto operator()(Lhs lhs, const Rhs &rhs) const noexcept(Id == sym::id("==") | Id == sym::id("<>")) { // conversion disabled
            if (bool{});
            else if constexpr (Id == sym::id("==")) return  MNL_LIKELY(is<Lhs>(rhs)) && lhs == as<decltype(lhs)>(rhs);
            else if constexpr (Id == sym::id("<>")) return !MNL_LIKELY(is<Lhs>(rhs)) || lhs != as<decltype(lhs)>(rhs);
            else if constexpr (
               Id == sym::id("+") | Id == sym::id("-" ) | Id == sym::id("*") |
               Id == sym::id("<") | Id == sym::id("<=") | Id == sym::id(">") | Id == sym::id(">=") ||
               std::is_same_v<Lhs, unsigned> && Id == sym::id("Xor") | Id == sym::id("&") | Id == sym::id("|") ) {
               { if (MNL_LIKELY(is<Lhs>(rhs))) return _apply(lhs, as<decltype(lhs)>(rhs)); err_TypeMismatch(); }
            else
               return ((const sym &)*this)(lhs, rhs); // for completeness
         }
         template< typename Lhs, class Rhs, std::enable_if_t< // for completeness
            std::is_same_v<Lhs, int> &&
            std::is_same_v<Rhs, val>, decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE auto operator()(Lhs lhs, const Rhs &rhs) const noexcept(noexcept((*this)((long long)lhs, rhs)))
            { return (*this)((long long)lhs, rhs); }
         // Sym, string
         template< class Lhs, class Rhs, std::enable_if_t<
            std::is_same_v<Lhs, sym> | std::is_same_v<Lhs, std::string> &&
            std::is_same_v<Rhs, val>, decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE auto operator()(const Lhs &lhs, const Rhs &rhs) const noexcept(Id == sym::id("==") | Id == sym::id("<>")) {
            if (bool{});
            else if constexpr (Id == sym::id("==")) return  MNL_LIKELY(is<Lhs>(rhs)) && lhs == as<decltype(lhs)>(rhs);
            else if constexpr (Id == sym::id("<>")) return !MNL_LIKELY(is<Lhs>(rhs)) || lhs != as<decltype(lhs)>(rhs);
            else return ((const sym &)*this)(lhs, rhs); // for completeness
         }
         template< typename Lhs, class Rhs, std::enable_if_t< // for completeness
            std::is_same_v<Lhs, char> &&
            std::is_same_v<Rhs, val>, decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE auto operator()(const Lhs *lhs, const Rhs &rhs) const noexcept(Id == sym::id("==") | Id == sym::id("<>")) {
            if (bool{});
            else if constexpr (Id == sym::id("==")) return  MNL_LIKELY(is<std::string>(rhs)) && lhs == as<const std::string &>(rhs);
            else if constexpr (Id == sym::id("<>")) return !MNL_LIKELY(is<std::string>(rhs)) || lhs != as<const std::string &>(rhs);
            else return ((const sym &)*this)(lhs, rhs); // for completeness
         }
         // Nil
         template< typename Lhs, class Rhs, std::enable_if_t<
            std::is_same_v<Lhs, decltype(nullptr)> &&
            std::is_same_v<Rhs, val>, decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE auto operator()(Lhs lhs, const Rhs &rhs) const noexcept(Id == sym::id("==") | Id == sym::id("<>")) {
            if (bool{});
            else if constexpr (Id == sym::id("==")) return  is<>(rhs);
            else if constexpr (Id == sym::id("<>")) return !is<>(rhs);
            else return ((const sym &)*this)(lhs, rhs); // for completeness
         }
         // Bool
         template< typename Lhs, class Rhs, std::enable_if_t<
            std::is_same_v<Lhs, bool> &&
            std::is_same_v<Rhs, val>, decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE auto operator()(Lhs lhs, const Rhs &rhs) const noexcept(Id == sym::id("==") | Id == sym::id("<>")) {
            if (bool{});
            else if constexpr (Id == sym::id("==" )) return rhs.rep.tag() == (lhs | 0xFFF8 + 0b100);
            else if constexpr (Id == sym::id("<>" )) return rhs.rep.tag() != (lhs | 0xFFF8 + 0b100);
            else if constexpr (
               Id == sym::id("Xor") | Id == sym::id("&") | Id == sym::id("|") )
               { if (MNL_LIKELY(is<Lhs>(rhs))) return _apply(lhs, as<decltype(lhs)>(rhs)); err_TypeMismatch(); }
            else
               return ((const sym &)*this)(lhs, rhs); // for completeness
         }
      public:
      // "specializations" not derivable by scalar value propagation in _apply; necessary for performance reasons
         // core numeric
         template< typename Lhs, typename Rhs, std::enable_if_t<
            std::is_same_v<std::remove_const_t<std::remove_reference_t<Lhs>>, val> &&
            std::is_same_v<Rhs, long long> | std::is_same_v<Rhs, double> | std::is_same_v<Rhs, float> | std::is_same_v<Rhs, unsigned>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE val operator()(Lhs &&lhs, Rhs rhs) const {
            if (bool{});
            else if constexpr (Id == sym::id("==")) {
               if (MNL_LIKELY(is<Rhs>(lhs))) return as<decltype(rhs)>(lhs) == rhs;
               if (MNL_LIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) return false;
            }
            else if constexpr (Id == sym::id("<>")) {
               if (MNL_LIKELY(is<Rhs>(lhs))) return as<decltype(rhs)>(lhs) != rhs;
               if (MNL_LIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) return true;
            }
            else if constexpr (
               Id == sym::id("+") | Id == sym::id("-" ) | Id == sym::id("*") |
               Id == sym::id("<") | Id == sym::id("<=") | Id == sym::id(">") | Id == sym::id(">=") ) {
               if (MNL_LIKELY(is<Rhs>(lhs))) return _apply(as<decltype(rhs)>(lhs), rhs);
               if (MNL_UNLIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) err_numeric(lhs);
            }
            else if constexpr (
               std::is_same_v<Rhs, unsigned> && Id == sym::id("Xor") | Id == sym::id("&") | Id == sym::id("|") ) {
               if (MNL_LIKELY(is<Rhs>(lhs))) return _apply(as<decltype(rhs)>(lhs), rhs);
               if (MNL_UNLIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) err_bitwise(lhs);
            }
            else
               return ((const sym &)*this)(std::forward<Lhs>(lhs), rhs); // for completeness
            return static_cast<root *>(lhs.rep.template dat<void *>())->_invoke(
               std::forward<Lhs>(lhs), *this, 1, &const_cast<val &>((const val &)rhs));
         }
         template< typename Lhs, typename Rhs, std::enable_if_t< // for completeness
            std::is_same_v<std::remove_const_t<std::remove_reference_t<Lhs>>, val> &&
            std::is_same_v<Rhs, int>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE val operator()(Lhs &&lhs, Rhs rhs) const
            { return (*this)(std::forward<Lhs>(lhs), (long long)rhs); }
         // Sym
         template< typename Lhs, class Rhs, std::enable_if_t<
            std::is_same_v<std::remove_const_t<std::remove_reference_t<Lhs>>, val> &&
            std::is_same_v<Rhs, sym>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE val operator()(Lhs &&lhs, const Rhs &rhs) const {
            if (bool{});
            else if constexpr (Id == sym::id("==")) {
               if (MNL_LIKELY(is<Rhs>(lhs))) return as<decltype(rhs)>(lhs) == rhs;
               if (MNL_LIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) return false;
            }
            else if constexpr (Id == sym::id("<>")) {
               if (MNL_LIKELY(is<Rhs>(lhs))) return as<decltype(rhs)>(lhs) != rhs;
               if (MNL_LIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) return true;
            }
            else
               return ((const sym &)*this)(std::forward<Lhs>(lhs), rhs); // for completeness
            return static_cast<root *>(lhs.rep.template dat<void *>())->_invoke(
               std::forward<Lhs>(lhs), *this, 1, &const_cast<val &>((const val &)rhs));
         }
         // Nil
         template< typename Lhs, typename Rhs, std::enable_if_t<
            std::is_same_v<std::remove_const_t<std::remove_reference_t<Lhs>>, val> &&
            std::is_same_v<Rhs, decltype(nullptr)>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE val operator()(Lhs &&lhs, Rhs rhs) const {
            if (bool{});
            else if constexpr (Id == sym::id("==")) {
               if (MNL_LIKELY(is<>(lhs))) return true;
               if (MNL_LIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) return false;
            }
            else if constexpr (Id == sym::id("<>")) {
               if (MNL_LIKELY(is<>(lhs))) return false;
               if (MNL_LIKELY(lhs.rep.tag() != 0xFFF8 + 0b111)) return true;
            }
            else
               return ((const sym &)*this)(std::forward<Lhs>(lhs), rhs); // for completeness
            return static_cast<root *>(lhs.rep.template dat<void *>())->_invoke(
               std::forward<Lhs>(lhs), *this, 1, &const_cast<val &>((const val &)nullptr));
      public:
         MNL_INLINE val operator()(const val  &arg) const { return _apply(          arg ); }
         MNL_INLINE val operator()(      val &&arg) const { return _apply(std::move(arg)); }
      private:
         template<typename Arg, std::enable_if_t<
            std::is_same_v<std::decay_t<Arg>, val>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE static val _apply(Arg &&arg) {
            if (bool{});
            else if constexpr (Id == sym::id("-") | Id == sym::id("Abs"))
               switch (arg.rep.tag()) MNL_NOTE(jumptable) {
               case 0xFFF8 + 0b000: case 0xFFF8 + 0b100: case 0xFFF8 + 0b101: case 0xFFF8 + 0b110:
                  err_UnrecognizedOperation();
               case 0xFFF8 + 0b111 /*BoxPtr (fallback)*/:
                  return static_cast<root *>(arg.rep.template dat<void *>())->_invoke(std::forward<Arg>(arg), *this, 0, {});
               case 0xFFF8 + 0b001 /*I48*/: return _apply(as<long long>(arg));
               default             /*F64*/: return _apply(as<double>(arg));
               case 0xFFF8 + 0b010 /*F32*/: return _apply(as<float>(arg));
               case 0xFFF8 + 0b011 /*U32*/: return _apply(as<unsigned>(arg));
               }
            else if constexpr (Id == sym::id("~"))
               if (bool{});
               else if (MNL_UNLIKELY(is<unsigned>(arg))) // U32
                  return _apply(as<unsigned>(arg));
               else if (MNL_LIKELY(is<bool>(arg))) // Bool
                  return _apply(as<bool>(arg));
               else if (MNL_LIKELY(arg.rep.tag() == 0xFFF8 + 0b111)) // BoxPtr (fallback)
                  return static_cast<root *>(arg.rep.template dat<void *>())->_invoke(std::forward<Arg>(arg), *this, 0, {});
               else
                  err_UnrecognizedOperation();
            else if constexpr (Id == sym::id("Size") | Id == sym::id("Elems") | Id == sym::id("Keys"))
               if (bool{});
               else if (MNL_LIKELY(arg.rep.tag() == 0xFFF8 + 0b111)) // BoxPtr (fallback)
                  return static_cast<root *>(arg.rep.template dat<void *>())->_invoke(std::forward<Arg>(arg), *this, 0, {});
               else
                  err_UnrecognizedOperation();
            else
               return ((const sym &)*this)(std::forward<Arg>(arg)); // for completeness
         }
      private:
         template<typename Dat, std::enable_if_t<
            std::is_same_v<Dat, long long> | std::is_same_v<Dat, double> | std::is_same_v<Dat, float> |
            std::is_same_v<Dat, unsigned>  | std::is_same_v<Dat, bool>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE static auto _apply(Dat lhs, Dat rhs) {
            if constexpr (Id == sym::id( "+" )) return (_add)(lhs, rhs); // use internal helpers
            if constexpr (Id == sym::id( "-" )) return (_sub)(lhs, rhs);
            if constexpr (Id == sym::id( "*" )) return (_mul)(lhs, rhs);
            if constexpr (Id == sym::id( "<" )) return lhs <  rhs;       // rely on ADL-enabled lookup
            if constexpr (Id == sym::id( "<=")) return lhs <= rhs;
            if constexpr (Id == sym::id( ">" )) return lhs >  rhs;
            if constexpr (Id == sym::id( ">=")) return lhs >= rhs;
            if constexpr (Id == sym::id( "&" )) return (_and)(lhs, rhs);
            if constexpr (Id == sym::id( "|" )) return (_or )(lhs, rhs);
            if constexpr (Id == sym::id("Xor")) return (_xor)(lhs, rhs);
         }
         template<typename Dat, std::enable_if_t<
            std::is_same_v<Dat, long long> | std::is_same_v<Dat, double> | std::is_same_v<Dat, float> |
            std::is_same_v<Dat, unsigned>  | std::is_same_v<Dat, bool>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE static auto _apply(Dat arg) {
            if constexpr (Id == sym::id( "-" )) return (_neg)(arg);
            if constexpr (Id == sym::id("Abs")) return (_abs)(arg);
            if constexpr (Id == sym::id( "~" )) return (_not)(arg);
         }
      public:
         MNL_INLINE val operator()(const val  &target, int argc, val argv[], val *argv_out = 0) { return _apply(          target , argc, argv, argv_out); }
         MNL_INLINE val operator()(      val &&target, int argc, val argv[], val *argv_out = 0) { return _apply(std::move(target), argc, argv, argv_out); }
      public:
         template<typename Target, std::enable_if_t<
            std::is_same_v<std::decay_t<Target>, val>,
            decltype(nullptr) > = decltype(nullptr){} >
         MNL_INLINE static val _apply(Target &&target, int argc, val argv[], val *argv_out) {
            if (bool{});
            else if constexpr (Id == sym::id("Apply") | Id == sym::id("Fetch")) {
               if (MNL_LIKELY(target.rep.tag() == 0x7FF8 + 0b111)) // BoxPtr (fallback)
                  return static_cast<root *>(target.rep.template dat<void *>())->
                     _invoke(std::forward<Target>(target), *this, argc, argv, argv_out);
               if (MNL_LIKELY(target.rep.tag() == 0x7FF8 + 0b110)) // Sym
                  return cast<const sym &>(target)(argc, argv, argv_out);
               err_UnrecognizedOperation();
            }
            else
               return ((const sym &)*this)(std::forward<Target>(target), argc, argv, argv_out); // for completeness
         }
      };
   public:
      template<enum sym::id Id, std::enable_if_t<
         Id == sym::id( "+" ) | Id == sym::id( "-" ) | Id == sym::id( "*" ) |
         Id == sym::id( "==") | Id == sym::id( "<>") |
         Id == sym::id( "<" ) | Id == sym::id( "<=") | Id == sym::id( ">" ) | Id == sym::id(">=") |
         Id == sym::id( "&" ) | Id == sym::id( "|" ) | Id == sym::id("Xor") |
         Id == sym::id( "-" ) | Id == sym::id("Abs") | Id == sym::id( "~" ),
         decltype(nullptr) > = decltype(nullptr){} >
      static constexpr _op<Id> op{};
   };
   namespace aux::pub {
      template<enum sym::id Id> constexpr auto op<Id> = val::ops::op<Id>;
   }

namespace aux { namespace pub {
   class proc_Min { MNL_INLINE static inline val invoke(val &&, const sym &, int, val [], val *); friend box<proc_Min>; };
   class proc_Max { MNL_INLINE static inline val invoke(val &&, const sym &, int, val [], val *); friend box<proc_Max>; };
}} // namespace aux::pub
   extern template class box<proc_Min>;
   extern template class box<proc_Max>;

// Convenience Routines ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   template<typename Dat> MNL_INLINE inline Dat safe_cast(const val &rhs)
      { if (MNL_LIKELY(test<Dat>(rhs))) return cast<Dat>(rhs); MNL_ERR(MNL_SYM("TypeMismatch")); }
   template<typename Dat> MNL_INLINE inline Dat safe_cast(const loc &loc, const val &rhs)
      { if (MNL_LIKELY(test<Dat>(rhs))) return cast<Dat>(rhs); MNL_ERR_LOC(loc, MNL_SYM("TypeMismatch")); }
}} // namespace aux::pub

// Record Composite ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   class record_descr/*iptor*/ { // TODO: record_sign/*ature*/, profile?
   public: // Standard operations
      MNL_INLINE record_descr() noexcept: rep(store.end()) {}
      record_descr(const record_descr &) noexcept;
      MNL_INLINE record_descr(record_descr &&rhs) noexcept: rep(rhs.rep) { rhs.rep = store.end(); }
      ~record_descr();
      record_descr &operator=(const record_descr &) noexcept;
      MNL_INLINE record_descr &operator=(record_descr &&rhs) noexcept { swap(rhs); return *this; }
      MNL_INLINE void swap(record_descr &rhs) noexcept { using std::swap; swap(rep, rhs.rep); }
   public: // Misc public operations
      record_descr(set<sym> /*items*/), record_descr(initializer_list<const char *>); // precond: items.size() <= (unsigned char)-1
      MNL_INLINE const set<sym> &items() const noexcept { return rep->first; }
      MNL_INLINE const sym::tab<unsigned char> &tab() const noexcept { return rep->second.first; }
      MNL_INLINE int operator[](const sym &id) const noexcept { return tab()[id]; }
      MNL_INLINE bool has(const sym &id) const noexcept { return (*this)[id] != (unsigned char)-1; }
      MNL_INLINE friend bool operator==(const record_descr &lhs, const record_descr &rhs) noexcept { return lhs.rep == rhs.rep; }
      friend int order(const record_descr &, const record_descr &) noexcept;
   private: // Concrete representation and implementation helpers
      static map<set<sym>, pair<const sym::tab<unsigned char>, /*atomic*/ long>> store;
      decltype(store)::iterator rep;
      MNL_IF_WITH_MT(static std::mutex mutex;)
      MNL_INLINE inline void addref() const noexcept, release() const noexcept;
   };
   MNL_INLINE inline void swap(record_descr &lhs, record_descr &rhs) noexcept { lhs.swap(rhs); }
   bool operator==(const record_descr &, const record_descr &) noexcept;
   int  order     (const record_descr &, const record_descr &) noexcept;
   MNL_INLINE inline bool operator!=(const record_descr &lhs, const record_descr &rhs) noexcept { return std::rel_ops::operator!=(lhs, rhs); }
}} // namespace aux::pub
   # define MNL_RECORD_DESCR(...) MNL_AUX_INIT(::mnl::record_descr({__VA_ARGS__}))
namespace aux {
   template<int Size = 0> struct _record { // too large Size implies more copying at creation time!
      static_assert(Size >= 0 && Size <= 12, "Size >= 0 && Size <= 12");
      record_descr descr;
      typename std::conditional<Size >= 1 && Size <= 12, val [Size >= 1 && Size <= 12 ? Size : 1], vector<val>>::type items;
      void swap(_record &rhs) noexcept { std::swap(*this, rhs); } // default swap is nice here
   public:
      MNL_INLINE val &operator[](int sn) noexcept { return items[sn]; }
      MNL_INLINE const val &operator[](int sn) const noexcept { return items[sn]; }
      MNL_INLINE val &operator[](const sym &id) noexcept { return (*this)[descr[id]]; }
      MNL_INLINE const val &operator[](const sym &id) const noexcept { return (*this)[descr[id]]; }
      MNL_INLINE bool has(const sym &id) const noexcept { return descr.has(id); }
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int, val [], val *);
      friend box<_record>;
   };
} // namespace aux
   template<> MNL_INLINE inline box<aux::_record<>>::~box() { while (!dat.items.empty()) dat.items.pop_back(); }
   extern template class box<aux::_record<0x1>>;
   extern template class box<aux::_record<0x2>>;
   extern template class box<aux::_record<0x3>>;
   extern template class box<aux::_record<0x4>>;
   extern template class box<aux::_record<0x5>>;
   extern template class box<aux::_record<0x6>>;
   extern template class box<aux::_record<0x7>>;
   extern template class box<aux::_record<0x8>>;
   extern template class box<aux::_record<0x9>>;
   extern template class box<aux::_record<0xA>>;
   extern template class box<aux::_record<0xB>>;
   extern template class box<aux::_record<0xC>>;
   extern template class box<aux::_record<>>;
namespace aux { namespace pub {
   template<int Size> using record = _record<(Size >= 1 && Size <= (unsigned char)-1 ? Size <= 12 ? Size : 0 : -1)>;
   // invariant: size(r.items) == size(r.descr.items())
}} // namespace aux::pub

// I48 Range ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   template<bool /*Rev[erse]*/ = false> struct range {
      long long lo, hi;
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int, val [], val *);
      friend box<range>;
   };
}} // namespace aux::pub
   extern template class box<range<>>;
   extern template class box<range<true>>;

} // namespace MNL_AUX_UUID
