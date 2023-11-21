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
      MNL_INLINE ~sym() { release(); }
      MNL_INLINE sym &operator=(const sym &rhs) noexcept { rhs.addref(), release(), rep = rhs.rep; return *this; }
      MNL_INLINE sym &operator=(sym &&rhs) noexcept { swap(rhs); return *this; }
      MNL_INLINE void swap(sym &rhs) noexcept { using std::swap; swap(rep, rhs.rep); }
      MNL_INLINE friend bool operator==(const sym &lhs, const sym &rhs) noexcept { return lhs.rep == rhs; }
      MNL_INLINE friend bool operator< (const sym &lhs, const sym &rhs) noexcept { auto mask = MNL_AUX_RAND(unsigned); return (lhs ^ mask) < (rhs ^ mask); }
      MNL_INLINE explicit operator bool() const noexcept { return rep; }
   public: // Construction and extraction via conversion
      sym(string), sym(const char *);
      explicit sym(decltype(nullptr));
      MNL_INLINE explicit operator const string &() const noexcept { return inverse[rep]->first; } // no sync required
      MNL_INLINE explicit operator const char *() const noexcept { return ((const string &)*this).c_str(); }
   public: // Functional application
      static constexpr auto max_argc = 999;
      val operator()(int argc, val argv[], val *argv_out = {}) const; // essential
      val operator()(const val &arg, val *arg_out = {}) const, operator()(val &&arg, val *arg_out = {}) const;
      template<size_t Argc> val operator()(args<Argc> &&, val *args_out = {}) const;
      // ...and their tracing counterparts:
      val operator()(const loc &, int, val [], val * = {}) const;
      val operator()(const loc &, const val &, val * = {}) const, operator()(const loc &, val &&, val * = {}) const;
      template<size_t Argc> val operator()(const loc &, args<Argc> &&, val * = {}) const;

      template<typename ...It> MNL_INLINE val operator()(It &&...it) const & { return _apply(*this, std::forward<It>(it) ...); }
      template<typename ...It> MNL_INLINE val repl(It &&...it) const & { return _repl(*this, std::forward<It>(it) ...); }
      template<typename ...It> MNL_INLINE val operator()(const loc &loc, It &&...it) const & { return _apply(loc, *this, std::forward<It>(it) ...); }
      template<typename ...It> MNL_INLINE val repl(const loc &loc, It &&...it) const & { return _repl(loc, *this, std::forward<It>(it) ...); }

   public:




   public: // Essential for performance (argv_out[-1] corresponds to self; !argc => !argv && !argv_out)
      val operator()(const val &self, int argc, val argv[], val *argv_out = {}) const;
      val operator()(val &&self, int argc, val argv[], val *argv_out = {}) const;

   public: // Essential for metaprogramming
      // For one argument
      val operator()(const val &) const, operator()(val &&) const;
      // For two arguments
      val operator()(const val &, const val &) const, operator()(const val &, val &&) const;
      val operator()(val &&, const val &) const, operator()(val &&, val &&) const;
      // For multiple arguments
      val operator()(int argc, val [], val *argv_out = {}) const;
   public: // Convenience
      template<std::size_t Argc> val operator()(const val &self, std::array<val, Argc>, val *args_out = {}) const;
      template<std::size_t Argc> val operator()(val &&self, std::array<val, Argc>, val *args_out = {}) const;
      template<std::size_t Argc> val operator()(std::array<val, Argc>, val *args_out = {}) const;

   private: // Concrete representation
      enum rep: unsigned short;
      enum rep rep; // "enum" is required as per ISO/IEC 14882:2011 S3.3.7 P1, although some C++ compilers do not issue any diagnostic message
      static map<string, const decltype(rep)> dict;
      static decltype(dict)::const_iterator inverse[];
      static vector<decltype(rep)> pool;
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
   public:
      template<typename Target, std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
         friend val _apply(Target &&, int argc, val [], val *argv_out);
   public: // Related stuff
      template<typename = class code> class tab/*le*/; // do not: move "= class code" to the definition below (compiler bug)
      friend val;                                  // addref/release
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


   extern template val sym::operator()(const val &self, int argc, val argv[], val *argv_out = {}) const &; // really needed?
   extern template val sym::operator()(val &&self, int argc, val argv[], val *argv_out = {}) const &;


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

   # define MNL_SYM(TXT)  MNL_AUX_INIT(::mnl::sym(TXT))
   # define MNL_DISP(...) MNL_AUX_INIT(::mnl::aux::disp({__VA_ARGS__}))
namespace aux { MNL_NOINLINE inline sym::tab<signed char> disp(initializer_list<const char *> il) {
   sym::tab<signed char> res; int val = 0; for (auto &&key: il) res.update(key, ++val); return res;
}}

// class val ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux::pub {
   typedef val ast; // val when used as an Abstract Syntax Tree - for documentation purposes

   class val/*ue*/ {
   public: // Standard operations
      MNL_INLINE val(decltype(nullptr) = {}) noexcept: rep{rep::tag_nil} {}
      MNL_INLINE val(const val &rhs) noexcept: rep(rhs.rep) { addref(); }
      MNL_INLINE val(val &&rhs) noexcept: rep(rhs.rep) { rhs.rep = {rep::tag_nil}; }
      MNL_INLINE ~val() { release(); }
      MNL_INLINE val &operator=(const val &rhs) noexcept { rhs.addref(), release(), rep = rhs.rep; return *this; }
      MNL_INLINE val &operator=(val &&rhs) noexcept { release(), rep = rhs.rep, rhs.rep = {rep::tag_nil}; return *this; }
      MNL_INLINE void swap(val &rhs) noexcept { std::swap(rep, rhs.rep); }
      MNL_INLINE explicit operator bool() const noexcept { return *this != nullptr; }
   public: // using swap for (faster) assignment
      //template<typename Rhs> MNL_INLINE val &assign(Rhs &&rhs) noexcept(noexcept(*this = std::forward<Rhs>(rhs))) { return *this = std::forward<Rhs>(rhs); }
      //MNL_INLINE val &assign(val &&rhs) noexcept { swap(rhs); return *this; }
   public: // Construction -- Implicit conversion (to)
      MNL_INLINE val(long long dat) noexcept: rep{rep::tag_i48, dat} {} // valid range: min_i48 .. max_i48
      MNL_INLINE val(int dat) noexcept:       val((long long)dat) {}
      MNL_INLINE val(double dat) noexcept: rep(dat) { assume_f64(); }
      MNL_INLINE val(float dat) noexcept: rep{0x7FFCu, dat} {}
      MNL_INLINE val(const sym &dat) noexcept: rep{0x7FFBu, dat} {}
      MNL_INLINE val(sym &&dat) noexcept: rep{0x7FFBu, std::move(dat)} {}
      MNL_INLINE val(bool dat) noexcept: rep{0x7FFEu | dat} {}
      MNL_INLINE val(unsigned dat) noexcept: rep{0x7FFDu, dat} {}
      MNL_INLINE val(char dat) noexcept:     val((unsigned)(unsigned char)dat) {}
      template<typename Dat, std::enable_if_t<!std::is_same_v<Dat, val *>, int{}>
         val(Dat dat) noexcept(std::is_nothrow_constructible_v<Dat>): rep{0x7FF8u, (void *)(root *)new box<Dat>{std::move(dat)}} {}
      val(const char *);
      MNL_INLINE val(char *dat): val((const char *)dat) {}
   public: // Extraction
      template<typename Dat = decltype(nullptr)> MNL_INLINE friend bool test(const val &rhs) noexcept
         { return rhs.test<Dat>(); }
      template<typename Dat = decltype(nullptr)> MNL_INLINE friend Dat  cast(const val &rhs) noexcept(std::is_nothrow_copy_constructible<Dat>::value)
         { return rhs.cast<Dat>(); }
   public: // Misc essentials
      static constexpr auto max_argc = sym::max_argc;
      val default_invoke(const sym &op, int argc, val argv[]);
      long rc /*reference counter*/() const noexcept;
      int default_order(const val &) const noexcept; // actually from MANOOL API
   private: // Concrete representation
   public:
      static_assert(sizeof(double) == 8, "sizeof(double) == 8");
      class MNL_ALIGN(8) rep { // bit-layout management - IEEE 754 FP representation and uniform FP endianness are assumed (and NOT checked)
         static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__"); // support for BE can be added on-demand
         union {
            MNL_PACK signed long long _int: 48;
            MNL_PACK uintptr_t _ptr: sizeof(void *) == 4 ? 32 : sizeof(void *) == 8 ? 48 : 0;
            sym _sym; // standard-layout struct
         };
         unsigned short _tag;
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
         explicit rep(unsigned tag, sym &&) noexcept;
      public:
         MNL_INLINE rep &operator=(unsigned tag) noexcept { _tag = tag; return *this; }
         MNL_INLINE unsigned tag() const noexcept { return _tag; }
         template<typename Dat> Dat dat() const noexcept;
      private:
         MNL_INLINE void copy(const rep &rhs) noexcept { // assume memcpy copies the union representation AND its active member, if any exists
         # if __clang__ || __GNUC__ >= 5 && !__INTEL_COMPILER
            memmove
         # else
            __builtin_memcpy // technically !!!UB!!! when &rhs == this but shouldn't be an issue for our target environments
         # endif
            (this, &rhs, sizeof *this); // updates sym::rep (AND rep::tag at once), in case of _sym (corner case of ISO/IEC 14882:2011)
         }
         static constexpr decltype(_tag) tag_base = 0x7FF8,
            tag_box = tag_base | 0x0, tag_nil = tag_base | 0x1, tag_i48 = tag_base | 0x2, tag_f32 = tag_base | 0x4,
            tag_sym = tag_base | 0x3, tag_false = tag_base | 0x6, tag_true = tag_false | true, tag_u32 = tag_base | 0x5;
         static_assert(!(tag_false & true));
      } rep;
      static_assert(sizeof rep == 8, "sizeof rep == 8");                                                             // paranoid check
      static_assert(std::is_standard_layout<decltype(rep)>::value, "std::is_standard_layout<decltype(rep)>::value"); // ditto
      MNL_INLINE explicit val(decltype(rep) rep) noexcept: rep(rep) {}
   private: // Implementation helpers
      void addref() const noexcept, release() const noexcept;
      void assume_f64() const noexcept;
      template<typename     = decltype(nullptr)> bool test() const noexcept;
      template<typename Dat = decltype(nullptr)> Dat  cast() const noexcept(std::is_nothrow_copy_constructible<Dat>::value);
   public:
      class root;
   public: // Convenience -- Functional application - !argc => !argv && !argv_out
      template<typename ...It> MNL_INLINE val operator()(It &&...it) const & { return aux::_apply(*this, std::forward<It>(it) ...); }
      template<typename ...It> MNL_INLINE val operator()(It &&...it) && { return aux::_apply(std::move(*this), std::forward<It>(it) ...); }
      template<typename ...It> MNL_NODISCARD MNL_INLINE val repl(It &&...it) const & { return aux::_repl(*this, std::forward<It>(it) ...); }
      template<typename ...It> MNL_NODISCARD MNL_INLINE val repl(It &&...it) && { return aux::_repl(std::move(*this), std::forward<It>(it) ...); }
   public:
      template<typename ...It> MNL_INLINE val operator()(void trace(const loc &), const loc &loc, It &&...it) const &
         { try { return (*this)(std::forward<It>(it) ...); } catch (...) { trace(loc); } }
      template<typename ...It> MNL_INLINE val operator()(void trace(const loc &), const loc &loc, It &&...it) &&
         { try { return std::move(*this)(std::forward<It>(it) ...); } catch (...) { trace(loc); } }
      template<typename ...It> MNL_NODISCARD MNL_INLINE val repl(void trace(const loc &), const loc &loc, It &&...it) const &
         { try { return (*this).repl(std::forward<It>(it) ...); } catch (...) { trace(loc); } }
      template<typename ...It> MNL_NODISCARD MNL_INLINE val repl(void trace(const loc &), const loc &loc, It &&...it) &&
         { try { return std::move(*this).repl(std::forward<It>(it) ...); } catch (...) { trace(loc); } }


   public:
      template<typename Arg0 = val,
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &> || std::is_same_v<Arg0, const sym &>, int> = int{}>
      MNL_INLINE val operator()(Arg0 &&arg0) const & { return _apply(*this, std::forward<Arg0>(arg0)); }
      template<typename Arg0 = val,
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &> || std::is_same_v<Arg0, const sym &>, int> = int{}>
      MNL_INLINE val operator()(Arg0 &&arg0) && { return _apply(std::move(*this), std::forward<Arg0>(arg0)); }
      template<typename Arg0 = val, typename Arg1 = val,
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>,                                      int> = int{}>
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &> || std::is_same_v<Arg1, const sym &>, int> = int{}>
      MNL_INLINE val operator()(Arg0 &&arg0) const & { return _apply(*this, std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)); }
      template<typename Arg0 = val, typename Arg1 = val,
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>,                                      int> = int{}>
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &> || std::is_same_v<Arg1, const sym &>, int> = int{}>
      MNL_INLINE val operator()(Arg0 &&arg0) && { return _apply(std::move(*this), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1))); }



   public: // Fundamental for performance
      MNL_INLINE val operator()(int argc, val argv[], val *argv_out = {}) const & // argv_out[-1] corresponds to self
         { return _apply(*this, argc, argv, argv_out); }
      MNL_INLINE val operator()(int argc, val argv[], val *argv_out = {}) &&      // ditto
         { return _apply(_mv(*this), argc, argv, argv_out); }
   public:
      // For one argument
      MNL_INLINE val operator()(const val &arg0) const & { return _apply(*this, arg0); }
      MNL_INLINE val operator()(val &&arg0) const & { return _apply(*this, _mv(arg0)); }
      MNL_INLINE val operator()(const sym &arg0) const & { return _apply(*this, arg0); }
      MNL_INLINE val operator()(const val &arg0) && { return _apply(_mv(*this), arg0); }
      MNL_INLINE val operator()(val &&arg0) && { return _apply(_mv(*this), _mv(arg0)); }
      MNL_INLINE val operator()(const sym &arg0) && { return _apply(_mv(*this), arg0); }
      // For two arguments
      MNL_INLINE val operator()(const val &arg0, const val &arg1) const & { return _apply(*this, arg0, arg1); }
      MNL_INLINE val operator()(const val &arg0, val &&arg1) const & { return _apply(*this, arg0, _mv(arg1)); }
      MNL_INLINE val operator()(const val &arg0, const sym &arg1) const & { return _apply(*this, arg0, arg1); }
      MNL_INLINE val operator()(val &&arg0, const val &arg1) const & { return _apply(*this, _mv(arg0), arg1); }
      MNL_INLINE val operator()(val &&arg0, val &&arg1) const & { return _apply(*this, _mv(arg0), _mv(arg1)); }
      MNL_INLINE val operator()(val &&arg0, const sym &arg1) const & { return _apply(*this, _mv(arg0), arg1); }
      MNL_INLINE val operator()(const val &arg0, const val &arg1) && { return _apply(_mv(*this), arg0, arg1); }
      MNL_INLINE val operator()(const val &arg0, val &&arg1) && { return _apply(_mv(*this), arg0, _mv(arg1)); }
      MNL_INLINE val operator()(const val &arg0, const sym &arg1) && { return _apply(_mv(*this), arg0, arg1); }
      MNL_INLINE val operator()(val &&arg0, const val &arg1) && { return _apply(_mv(*this), _mv(arg0), arg1); }
      MNL_INLINE val operator()(val &&arg0, val &&arg1) && { return _apply(_mv(*this), _mv(arg0), _mv(arg1)); }
      MNL_INLINE val operator()(val &&arg0, const sym &arg1) && { return _apply(_mv(*this), _mv(arg0), arg1); }
   public:
      // For two arguments
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, const val &arg1) && { return _repl(_mv(*this), arg0, arg1); }
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, val &&arg1) && { return _repl(_mv(*this), arg0, _mv(arg1)); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, const val &arg1) && { return _repl(_mv(*this), _mv(arg0), arg1); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, val &&arg1) && { return _repl(_mv(*this), _mv(arg0), _mv(arg1)); }
      MNL_NODISCARD MNL_INLINE val repl(const sym &arg0, const val &arg1) && { return _repl(_mv(*this), arg0, arg1); }
      MNL_NODISCARD MNL_INLINE val repl(const sym &arg0, val &&arg1) && { return _repl(_mv(*this), arg0, _mv(arg1)); }
      // For three arguments
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, const val &arg1, const val &arg2) && { return _repl(_mv(*this), arg0, arg1, arg2); }
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, const val &arg1, val &&arg2) && { return _repl(_mv(*this), arg0, arg1, _mv(arg2)); }
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, val &&arg1, const val &arg2) && { return _repl(_mv(*this), arg0, _mv(arg1), arg2); }
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, val &&arg1, val &&arg2) && { return _repl(_mv(*this), arg0, _mv(arg1), _mv(arg2)); }
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, const sym &arg1, const val &arg2) && { return _repl(_mv(*this), arg0, arg1, arg2); }
      MNL_NODISCARD MNL_INLINE val repl(const val &arg0, const sym &arg1, val &&arg2) && { return _repl(_mv(*this), arg0, arg1, _mv(arg2)); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, const val &arg1, const val &arg2) && { return _repl(_mv(*this), _mv(arg0), arg1, arg2); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, const val &arg1, val &&arg2) && { return _repl(_mv(*this), _mv(arg0), arg1, _mv(arg2)); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, val &&arg1, const val &arg2) && { return _repl(_mv(*this), _mv(arg0), _mv(arg1), arg2); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, val &&arg1, val &&arg2) && { return _repl(_mv(*this), _mv(arg0), _mv(arg1), _mv(arg2)); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, const sym &arg1, const val &arg2) && { return _repl(_mv(*this), _mv(arg0), arg1, arg2); }
      MNL_NODISCARD MNL_INLINE val repl(val &&arg0, const sym &arg1, val &&arg2) && { return _repl(_mv(*this), _mv(arg0), arg1, _mv(arg2)); }
   public: // Convenience
      template<std::size_t Argc> MNL_INLINE val operator()(std::array<val, Argc> args, val *args_out = {}) const &
         { return (*this)(Argc, args.data(), args_out); }
      template<std::size_t Argc> MNL_INLINE val operator()(std::array<val, Argc> args, val *args_out = {}) &&
         { return _mv(*this)(Argc, args.data(), args_out); }
      // For no arguments
      MNL_INLINE val operator()() const &
         { return (*this)(0, {}); }
      MNL_INLINE val operator()() &&
         { return _mv(*this)(0, {}); }
      // Alternative notation
      MNL_INLINE val invoke(const sym *op, int argc, val argv[], val *argv_out = {}) const &
         { return op(*this, argc, argv, argv_out); }
      MNL_INLINE val invoke()(const sym *op, int argc, val argv[], val *argv_out = {}) &&
         { return op(_mv(*this), argc, argv, argv_out); }
      template<std::size_t Argc> MNL_INLINE val invoke(const sym *op, std::array<val, Argc> args, val *args_out = {}) const &
         { return op(*this, Argc, args.data(), args_out); }
      template<std::size_t Argc> MNL_INLINE val invoke()(const sym *op, std::array<val, Argc> args, val *args_out = {}) &&
         { return op(_mv(*this), Argc, args.data(), args_out); }

   private: // Implementation of the above
      // _apply
      template<typename Target>                               static val _apply(Target &&, int argc, val [], val *argv_out);
      template<typename Target, typename Arg0>                static val _apply(Target &&, Arg0 &&);
      template<typename Target, typename Arg0, typename Arg1> static val _apply(Target &&, Arg0 &&, Arg1 &&);
      // _repl
      template<typename Target, typename Arg0, typename Arg1>                static val _repl(Target &&, Arg0 &&, Arg1 &&);
      template<typename Target, typename Arg0, typename Arg1, typename Arg2> static val _repl(Target &&, Arg0 &&, Arg1 &&, Arg2 &&);
   private: // Implementation of sym::operator()
      template<typename Self> static val _invoke(Self &&, const sym &op, int argc, val [], val *argv_out); // Self == const val & || Self == val
      friend val sym::operator()(const val &, int, val [], val *) const, sym::operator()(val &&, int, val [], val *) const;


   public: // TODO: not needed

      template<typename Arg1, typename Arg2> MNL_INLINE val operator()(It &&...it) const & { return aux::_apply(*this, std::forward<It>(it) ...); }
      template<typename ...It> MNL_INLINE val operator()(It &&...it) && { return aux::_apply(std::move(*this), std::forward<It>(it) ...); }


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
   public:
      // apply/repl
      template<typename Target,
         std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
         friend val _apply(Target &&, int argc, val [], val *argv_out);
      template<typename Target,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
         friend val _apply(Target &&, int argc, val [], val *argv_out);
      template<typename Target, typename Arg0,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{}>
         friend val _apply(Target &&, Arg0 &&);
      template<typename Target, typename Arg0, typename Arg1,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
         friend val _apply(Target &&, Arg0 &&, Arg1 &&);
      template<typename Target,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
         friend val _repl(Target &&, int argc, val []);
      template<typename Target, typename Arg0, typename Arg1,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
         friend val _repl(Target &&, Arg0 &&, Arg1 &&);
      template<typename Target, typename Arg0, typename Arg1, typename Arg2,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg2, val> || std::is_same_v<Arg2, const val &>, int> = int{}>
         friend val _repl(Target &&, Arg0 &&, Arg1 &&, Arg2 &&);
      // op
   # define MNL_M(ID) \
      template<typename Lhs, typename Rhs, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
         friend val ID(Lhs &&, Rhs &&); \
   // end # define MNL_M(ID)
      MNL_M(_eq) MNL_M(_ne) MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge)
      MNL_M(_add) MNL_M(_sub) MNL_M(_mul) MNL_M(_xor)
   # undef MNL_M
   # define MNL_M(ID) \
      template<typename Rhs, \
         std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
         friend val ID(Rhs &&); \
   // end # define MNL_M(ID)
      MNL_M(_neg) MNL_M(_abs) MNL_M(_not)
   # undef MNL_M
      // op (const)
   # define MNL_M(ID) \
      template<typename Lhs, typename Dat, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}> \
         friend val ID(Lhs &&, Dat); \
   // end # define MNL_M(ID)
      MNL_M(_eq) MNL_M(_ne) MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge)
      MNL_M(_add) MNL_M(_sub) MNL_M(_mul)
   # undef MNL_M
   # define MNL_M(ID) \
      template<typename Lhs, typename Dat, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Dat, const sym &>, int> = int{}> \
         friend val ID(Lhs &&, Dat); \
      template<typename Lhs, typename Dat, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Dat, decltype(nullptr)>, int> = int{}> \
         friend val ID(Lhs &&, Dat); \
   // end # define MNL_M(ID)
      MNL_M(_eq) MNL_M(_ne)
   # undef MNL_M
   public:
      friend class proc_Min; friend class proc_Max;
   };
   MNL_INLINE inline void swap(val &lhs, val &rhs) noexcept { lhs.swap(rhs); }
   // defined in friend declarations above:
   template<typename>     bool test(const val &) noexcept;
   template<typename Dat> Dat  cast(const val &) noexcept(std::is_nothrow_copy_constructible<Dat>::value);

   // Forward-declared as members of class sym /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_INLINE inline val sym::operator()(const val &self, int argc, val argv[], val *argv_out = {}) const
      { return val::_invoke(self, *this, argc, argv, argv_out); }
   MNL_INLINE inline val sym::operator()(val &&self, int argc, val argv[], val *argv_out = {}) const
      { return val::_invoke(std::move(self), *this, argc, argv, argv_out); }
   // For one argument
   MNL_INLINE inline val sym::operator()(const val &arg0) const { return (*this)(arg0, 0, {}); }
   MNL_INLINE inline val sym::operator()(val &&arg0) const { return (*this)(std::move(arg0), 0, {}); }
   // For two arguments
   MNL_INLINE inline val sym::operator()(const val &arg0, const val &arg1) const { return (*this)(arg0, (val)arg1); }
   MNL_INLINE inline val sym::operator()(const val &arg0, val &&arg1) const { return (*this)(arg0, 1, &arg1); }
   MNL_INLINE inline val sym::operator()(val &&arg0, const val &arg1) const { return (*this)(std::move(arg0), (val)arg1); }
   MNL_INLINE inline val sym::operator()(val &&arg0, val &&arg1) const { return (*this)(std::move(arg0), 1, &arg1); }
   // For multiple arguments
   MNL_INLINE inline val sym::operator()(int argc, val argv[], val *argv_out = {}) const {
      if (MNL_UNLIKELY(!argc)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      return (*this)(std::move(*argv), argc - 1, argv + 1, argv_out + !!argv_out);                          // TODO: here, argc might become 0, but not argv!
   }
   // Convenience
   template<std::size_t Argc> MNL_INLINE inline val sym::operator()(const val &self, std::array<val, Argc> args, val *args_out) const
      { return (*this)(self, Argc, args.data(), args_out); }
   template<std::size_t Argc> MNL_INLINE inline val sym::operator()(val &&self, std::array<val, Argc> args, val *args_out) const
      { return (*this)(std::move(self), Argc, args.data(), args_out); }
   template<std::size_t Argc> MNL_INLINE inline val sym::operator()(std::array<val, Argc> args, val *args_out) const
      { return (*this)(Argc, args.data(), args_out); }





   /* val sym::operator()(int argc, val argv[], val *argv_out = {}) const; // essential form */
   MNL_INLINE inline val sym::operator()(const val &arg, val *arg_out) const { return (*this)(val(arg), arg_out); }
   MNL_INLINE inline val sym::operator()(val &&arg, val *arg_out) const { return (*this)(1, &arg, arg_out); }
   template<size_t Argc> MNL_INLINE inline val sym::operator()(args<Argc> &&args, val *args_out) const { return (*this)((int)Argc, args.data(), args_out); }
   // ...and their tracing counterparts:
   MNL_INLINE inline val sym::operator()(const loc &loc, int argc, val argv[], val *argv_out) const
      { try { return (*this)(argc, argv, argv_out); } catch (...) { MNL_NORETURN void trace_execute(const mnl::loc &); trace_execute(loc); } }
   MNL_INLINE inline val sym::operator()(const loc &loc, const val &arg, val *arg_out) const
      { try { return (*this)(arg, arg_out); } catch (...) { MNL_NORETURN void trace_execute(const mnl::loc &); trace_execute(loc); } }
   MNL_INLINE inline val sym::operator()(const loc &loc, val &&arg, val *arg_out) const
      { try { return (*this)(move(arg), arg_out); } catch (...) { MNL_NORETURN void trace_execute(const mnl::loc &); trace_execute(loc); } }
   template<size_t Argc> MNL_INLINE inline val sym::operator()(const loc &loc, args<Argc> &&args, val *args_out) const
      { try { return (*this)(move(args), args_out); } catch (...) { MNL_NORETURN void trace_execute(const mnl::loc &); trace_execute(loc); } }

   // Fake test/case for non-val inputs
   template<typename Dat> MNL_INLINE inline bool test(const typename std::remove_cv<typename std::remove_reference<
      typename std::enable_if<!std::is_same<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type, val>::value, Dat>::type
      >::type>::type &) noexcept { return true; }
   template<typename Dat> MNL_INLINE inline Dat  cast(const typename std::remove_cv<typename std::remove_reference<
      typename std::enable_if<!std::is_same<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type, val>::value, Dat>::type
      >::type>::type &dat) noexcept(std::is_nothrow_copy_constructible<Dat>::value) { return dat; }
} // namespace aux::pub

// Bit-Layout Management -- Data Write /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE inline val::rep::rep(unsigned tag, Dat dat) noexcept: _tag(tag) {
      static_assert(std::is_pod<Dat>::value, "std::is_pod<Dat>::value");
      static_assert(sizeof dat <= 6, "sizeof dat <= 6");
      static_assert(!std::is_pointer<Dat>::value, "!std::is_pointer<Dat>::value");
      memcpy(this, &dat, sizeof dat);
   }
   MNL_INLINE inline val::rep::rep(unsigned tag, long long dat) noexcept
      : _tag(tag), _int(dat) {}
   MNL_INLINE inline val::rep::rep(unsigned tag, void *dat) noexcept
      : _tag(tag), _ptr(reinterpret_cast<uintptr_t>(dat)) {}
   MNL_INLINE inline val::rep::rep(double dat) noexcept
      { memcpy(this, &dat, sizeof dat); }
   MNL_INLINE inline val::rep::rep(unsigned tag, const sym &dat) noexcept
      : _tag(tag), _sym(dat) {}
   MNL_INLINE inline val::rep::rep(unsigned tag, sym &&dat) noexcept
      : _tag(tag), _sym(std::move(dat)) {}

// Bit-Layout Management -- Data Read //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE inline Dat val::rep::dat() const noexcept {
      Dat dat;
      static_assert(std::is_pod<Dat>::value, "std::is_pod<Dat>::value");
      static_assert(sizeof dat <= 6, "sizeof dat <= 6");
      static_assert(!std::is_pointer<Dat>::value, "!std::is_pointer<Dat>::value");
      memcpy(&dat, this, sizeof dat); return dat;
   }
   template<> MNL_INLINE inline long long val::rep::dat() const noexcept
      { return _int; }
   template<> MNL_INLINE inline void *val::rep::dat() const noexcept
      { return reinterpret_cast<void *>(_ptr); }
   template<> MNL_INLINE inline double val::rep::dat() const noexcept
      { double dat; memcpy(&dat, this, sizeof dat); return dat; }
   template<> MNL_INLINE inline const sym &val::rep::dat() const noexcept
      { return _sym; }
   template<> MNL_INLINE inline sym &val::rep::dat() noexcept // new!!!
      { return _sym; }

// class Template box //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   class val::root {
   protected:
      root() = default;
      virtual ~root() = default;
   private:
      root(const root &) = delete;
      root &operator=(const root &) = delete;
   protected:
      long rc() const noexcept { return MNL_IF_WITHOUT_MT(_rc) MNL_IF_WITH_MT(__atomic_load_n(&_rc, __ATOMIC_RELAXED)); }
   private:
      /*atomic*/ long _rc = 1;
   private: // To be called instead of virtual functions directly (just more consistent naming)
      template<typename ...Items> MNL_INLINE val invoke(Items &&...items) { return _invoke(std::forward<Items>(items) ...); }
      template<typename ...Items> MNL_INLINE val apply(Items &&...items) { return _apply(std::forward<Items>(items) ...); }
      template<typename ...Items> MNL_INLINE val repl(Items &&...items) { return _repl(std::forward<Items>(items) ...); }
   private: // Helpers
      template<typename Self, typename Arg0> MNL_INLINE val _repl(Self &&self, Arg0 &&arg0, const val &arg1)
         { return _repl(std::forward<Self>(self), std::forward<Arg0>(arg0), (val)arg1); }
      template<typename Self, typename Arg0, typename Arg1> MNL_INLINE val _repl(Self &&self, Arg0 &&arg0, Arg1 &&arg1, const val &arg2)
         { return _repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), (val)arg2); }
   private: // 38 VMT entries
      MNL_NOINLINE MNL_HOT virtual val _invoke(const val &self, const sym &op, int argc, val [], val *argv_out) = 0; // argv_out[-1] corresponds to self
      MNL_NOINLINE MNL_HOT virtual val _invoke(val &&self, const sym &op, int argc, val [], val *argv_out) = 0;      // ditto
   private:
      // For one argument (6 VMT entries)
      MNL_HOT virtual val _apply(const val &self, const val &) = 0;
      MNL_HOT virtual val _apply(const val &self, val &&) = 0;
      MNL_HOT virtual val _apply(const val &self, const sym &) = 0;
      MNL_HOT virtual val _apply(val &&self, const val &) = 0;
      MNL_HOT virtual val _apply(val &&self, val &&) = 0;
      MNL_HOT virtual val _apply(val &&self, const sym &) = 0;
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
   private:
      // For two arguments (6 VMT entries)
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const sym &, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const sym &, val &&) = 0;
      // For three arguments (12 VMT entries)
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, const val &, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, const val &, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, val &&, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, val &&, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, const sym &, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, const val &, const sym &, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, const val &, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, const val &, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, val &&, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, val &&, val &&) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, const sym &, const val &) = 0;
      MNL_NODISCARD MNL_HOT virtual val _repl(val &&self, val &&, const sym &, val &&) = 0;
   public: // Friendship
      friend sym;
      friend val;
   public:
      // apply/repl
      template<typename Target,
         std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
         friend val _apply(Target &&, int argc, val [], val *argv_out);
      template<typename Target,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
         friend val _apply(Target &&, int argc, val [], val *argv_out);
      template<typename Target, typename Arg0,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{}>
         friend val _apply(Target &&, Arg0 &&);
      template<typename Target, typename Arg0, typename Arg1,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
         friend val _apply(Target &&, Arg0 &&, Arg1 &&);
      template<typename Target,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
         friend val _repl(Target &&, int argc, val []);
      template<typename Target, typename Arg0, typename Arg1,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
         friend val _repl(Target &&, Arg0 &&, Arg1 &&);
      template<typename Target, typename Arg0, typename Arg1, typename Arg2,
         std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{},
         std::enable_if_t<std::is_same_v<Arg2, val> || std::is_same_v<Arg2, const val &>, int> = int{}>
         friend val _repl(Target &&, Arg0 &&, Arg1 &&, Arg2 &&);
      // op
   # define MNL_M(ID) \
      template<typename Lhs, typename Rhs, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
         friend val ID(Lhs &&, Rhs &&); \
   // end # define MNL_M(ID)
      MNL_M(_eq) MNL_M(_ne) MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge)
      MNL_M(_add) MNL_M(_sub) MNL_M(_mul) MNL_M(_xor)
   # undef MNL_M
   # define MNL_M(ID) \
      template<typename Rhs, \
         std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
         friend val ID(Rhs &&); \
   // end # define MNL_M(ID)
      MNL_M(_neg) MNL_M(_abs) MNL_M(_not)
   # undef MNL_M
      // op (const)
   # define MNL_M(ID) \
      template<typename Lhs, typename Dat, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}> \
         friend val ID(Lhs &&, Dat); \
   // end # define MNL_M(ID)
      MNL_M(_eq) MNL_M(_ne) MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge)
      MNL_M(_add) MNL_M(_sub) MNL_M(_mul)
   # undef MNL_M
   # define MNL_M(ID) \
      template<typename Lhs, typename Dat, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Dat, const sym &>, int> = int{}> \
         friend val ID(Lhs &&, Dat); \
      template<typename Lhs, typename Dat, \
         std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
         std::enable_if_t<std::is_same_v<Dat, decltype(nullptr)>, int> = int{}> \
         friend val ID(Lhs &&, Dat); \
   // end # define MNL_M(ID)
      MNL_M(_eq) MNL_M(_ne)
   };
   template<typename Dat> class box final: val::root {
      Dat dat;
      explicit box(Dat &&dat): dat(std::move(dat)) {}
      ~box() {}
      friend val;
   private: // 38 VMT entries
      MNL_NOINLINE MNL_HOT val _invoke(const val &self, const sym &op, int argc, val argv[], val *argv_out = {}) override
         { return invoke(self, op, argv, argv_out); }
      MNL_NOINLINE MNL_HOT val _invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out = {}) override
         { return invoke(_mv(self), op, argv, argv_out); }
   private:
      // For one argument (6 VMT entries)
      MNL_HOT val _apply(const val &self, const val &arg0) override { return apply(self, arg0); }
      MNL_HOT val _apply(const val &self, val &&arg0) override { return apply(self, _mv(arg0)); }
      MNL_HOT val _apply(const val &self, const sym &arg0) override { return apply(self, arg0); }
      MNL_HOT val _apply(val &&self, const val &arg0) override { return apply(_mv(self), arg0); }
      MNL_HOT val _apply(val &&self, val &&arg0) override { return apply(_mv(self), _mv(arg0)); }
      MNL_HOT val _apply(val &&self, const sym &arg0) override { return apply(_mv(self), arg0); }
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
   private:
      // For two arguments (6 VMT entries)
      MNL_HOT val _repl(val &&self, const val &arg0, const val &arg1) override { return repl(_mv(self), arg0, arg1); }
      MNL_HOT val _repl(val &&self, const val &arg0, val &&arg1) override { return repl(_mv(self), arg0, _mv(arg1)); }
      MNL_HOT val _repl(val &&self, val &&arg0, const val &arg1) override { return repl(_mv(self), _mv(arg0), arg1); }
      MNL_HOT val _repl(val &&self, val &&arg0, val &&arg1) override { return repl(_mv(self), _mv(arg0), _mv(arg1)); }
      MNL_HOT val _repl(val &&self, const sym &arg0, const val &arg1) override { return repl(_mv(self), arg0, arg1); }
      MNL_HOT val _repl(val &&self, const sym &arg0, val &&arg1) override { return repl(_mv(self), arg0, _mv(arg1)); }
      // For three arguments (12 VMT entries)
      MNL_HOT val _repl(val &&self, const val &arg0, const val &arg1, const val &arg2) override { return repl(_mv(self), arg0, arg1, arg2); }
      MNL_HOT val _repl(val &&self, const val &arg0, const val &arg1, val &&arg2) override { return repl(_mv(self), arg0, arg1, _mv(arg2)); }
      MNL_HOT val _repl(val &&self, const val &arg0, val &&arg1, const val &arg2) override { return repl(_mv(self), arg0, _mv(arg1), arg2); }
      MNL_HOT val _repl(val &&self, const val &arg0, val &&arg1, val &&arg2) override { return repl(_mv(self), arg0, _mv(arg1), _mv(arg2)); }
      MNL_HOT val _repl(val &&self, const val &arg0, const sym &arg1, const val &arg2) override { return repl(_mv(self), arg0, arg1, arg2); }
      MNL_HOT val _repl(val &&self, const val &arg0, const sym &arg1, val &&arg2) override { return repl(_mv(self), arg0, arg1, _mv(arg2)); }
      MNL_HOT val _repl(val &&self, val &&arg0, const val &arg1, const val &arg2) override { return repl(_mv(self), _mv(arg0), arg1, arg2); }
      MNL_HOT val _repl(val &&self, val &&arg0, const val &arg1, val &&arg2) override { return repl(_mv(self), _mv(arg0), arg1, _mv(arg2)); }
      MNL_HOT val _repl(val &&self, val &&arg0, val &&arg1, const val &arg2) override { return repl(_mv(self), _mv(arg0), _mv(arg1), arg2); }
      MNL_HOT val _repl(val &&self, val &&arg0, val &&arg1, val &&arg2) override { return repl(_mv(self), _mv(arg0), _mv(arg1), _mv(arg2)); }
      MNL_HOT val _repl(val &&self, val &&arg0, const sym &arg1, const val &arg2) override { return repl(_mv(self), _mv(arg0), arg1, arg2); }
      MNL_HOT val _repl(val &&self, val &&arg0, const sym &arg1, val &&arg2) override { return repl(_mv(self), _mv(arg0), arg1, _mv(arg2)); }
   private:
      //template<typename Rhs> MNL_INLINE auto _mv(Rhs &&rhs) noexcept { return std::move(rhs); }
      MNL_INLINE val &&_mv(Rhs &rhs) noexcept { return std::move(rhs); }
   private: // User-specializable
      template<typename Self> MNL_INLINE val invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out)
         { return dat.invoke(std::forward<Self>(self), op, argc, argv, argv_out); }
      template<typename Self, typename Arg0> MNL_INLINE val apply(Self &&self, Arg0 &&arg0)
         { return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0)); }
      template<typename Self, typename Arg0, typename Arg1> MNL_INLINE val apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1)
         { return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)); }
      template<typename Self, typename Arg0> MNL_INLINE val repl(Self &&self, Arg0 &&arg0, val &&arg1)
         { return default_repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::move(arg1)); }
      template<typename Self, typename Arg0, typename Arg1> MNL_INLINE val repl(Self &&self, Arg0 &&arg0, Arg1 &&arg1, val &&arg2)
         { return default_repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::move(arg2)); }
   private: // Utilities for forwarding to "invoke"
      template<typename Self, typename Arg0>
      MNL_INLINE val default_apply(Self &&self, Arg0 &&arg0) {
         return _invoke(std::forward<Self>(self), MNL_SYM("Apply"), 1,
            &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Arg0, val>, val &, val>)arg0));
      }
      template<typename Self, typename Arg0, typename Arg1>
      MNL_INLINE val default_apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1) {
         val argv[] = {std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)};
         return _invoke(std::forward<Self>(self), MNL_SYM("Apply"), std::size(argv), argv);
      }
      template<typename Self, typename Arg0>
      MNL_INLINE val default_repl(Self &&self, Arg0 &&arg0, val &&arg1) {
         val argv[] = {std::forward<Arg0>(arg0), std::move(arg1)};
         return _invoke(std::forward<Self>(self), MNL_SYM("Repl"), std::size(argv), argv);
      }
      template<typename Self, typename Arg0, typename Arg1>
      MNL_INLINE val default_repl(Self &&self, Arg0 &&arg0, Arg1 &&arg1, val &&arg2) {
         val argv[] = {std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::move(arg2)};
         return _invoke(std::forward<Self>(self), MNL_SYM("Repl"), std::size(argv), argv);
      }
   };
   template<> class box<decltype(nullptr)>; // to be left incomplete to improve diagnostics
   template<> class box<long long>;         // ditto
   template<> class box<int>;               // ditto
   template<> class box<double>;            // ditto
   template<> class box<float>;             // ditto
   template<> class box<sym>;               // ditto
   template<> class box<bool>;              // ditto
   template<> class box<unsigned>;          // ditto
   template<> class box<char>;              // ditto

   template<> val box<std::string>::invoke(val &&, const sym &, int, val [], val *);
   template<> val box<std::vector<val>>::invoke(val &&, const sym &, int, val [], val *);
   template<> val box<std::pair<std::vector<ast>, loc>>::invoke(val &&, const sym &, int, val [], val *);
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
# if 0
   MNL_INLINE inline void val::addref() const noexcept {
      switch (rep.tag()) {
      case 0x7FFBu: rep.dat<const sym &>().addref(); return;
      case 0x7FF8u: MNL_IF_WITHOUT_MT(++static_cast<root *>(rep.dat<void *>())->_rc)
         MNL_IF_WITH_MT(__atomic_add_fetch(&static_cast<root *>(rep.dat<void *>())->_rc, 1, __ATOMIC_RELAXED));
      }
   }
   MNL_INLINE inline void val::release() const noexcept {
      switch (rep.tag()) {
      case 0x7FFBu: rep.dat<const sym &>().release(); return;
      case 0x7FF8u: if (MNL_UNLIKELY(! MNL_IF_WITHOUT_MT(--static_cast<root *>(rep.dat<void *>())->_rc)
         MNL_IF_WITH_MT(__atomic_sub_fetch(&static_cast<root *>(rep.dat<void *>())->_rc, 1, __ATOMIC_ACQ_REL)) )) delete static_cast<root *>(rep.dat<void *>());
      }
   }
# else // alternative implementation
   MNL_INLINE inline void val::addref() const noexcept {
      if (MNL_UNLIKELY(rep.tag() == 0x7FF8u)) // should improve branch prediction compared to a switch
         MNL_IF_WITHOUT_MT(++static_cast<root *>(rep.dat<void *>())->_rc)
         MNL_IF_WITH_MT(__atomic_add_fetch(&static_cast<root *>(rep.dat<void *>())->_rc, 1, __ATOMIC_RELAXED));
      else
      if (MNL_UNLIKELY(rep.tag() == 0x7FFBu))
         rep.dat<const sym &>().addref();
   }
   MNL_INLINE inline void val::release() const noexcept {
      if (MNL_UNLIKELY(rep.tag() == 0x7FF8u)) {
         if (!static_cast<root *>(rep.dat<void *>())) MNL_UNREACHABLE(); // performance hack
         if (MNL_UNLIKELY(!
            MNL_IF_WITHOUT_MT(--static_cast<root *>(rep.dat<void *>())->_rc)
            MNL_IF_WITH_MT(__atomic_sub_fetch(&static_cast<root *>(rep.dat<void *>())->_rc, 1, __ATOMIC_ACQ_REL)) ))
            delete static_cast<root *>(rep.dat<void *>());
      } else
      if (MNL_UNLIKELY(rep.tag() == 0x7FFBu)) {
         rep.dat<const sym &>().release();
      }
   }
# endif

// val Extractors //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE inline bool val::test() const noexcept {
      return MNL_LIKELY(rep.tag() == 0x7FF8u) &&
         typeid(*static_cast<root *>(rep.dat<void *>())) == typeid(box<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type>);
   }
   template<typename Dat> MNL_INLINE inline Dat val::cast() const noexcept(std::is_nothrow_copy_constructible<Dat>::value) {
      return static_cast<box<typename std::remove_cv<typename std::remove_reference<Dat>::type>::type> *>(static_cast<root *>(rep.dat<void *>()))->dat;
   }
   MNL_INLINE inline long val::rc() const noexcept
      { return static_cast<const root *>(rep.dat<void *>())->rc(); }

   template<> MNL_INLINE inline bool val::test<std::vector>() const noexcept
      { return MNL_LIKELY(rep.tag() == 0x7FF8u) && static_cast<root *>(rep.dat<void *>())->type == root::array; }
   template<> MNL_INLINE inline bool val::test<const std::vector &>() const noexcept
      { return test<std::vector>(); }
   template<> MNL_INLINE inline bool val::test<std::vector &>() const noexcept
      { return test<std::vector>(); }

   // Fake test/cast for val outputs
   template<> MNL_INLINE inline bool val::test<const val &>() const noexcept { return true; }
   template<> MNL_INLINE inline bool val::test<val>() const noexcept         { return test<const val &>(); }
   template<> MNL_INLINE inline const val &val::cast() const noexcept { return *this; }
   template<> MNL_INLINE inline val        val::cast() const noexcept { return cast<const val &>(); }

   template<> MNL_INLINE inline bool val::test<decltype(nullptr)>() const noexcept { return rep.tag() == 0x7FF9u; }
   template<> MNL_INLINE inline decltype(nullptr) val::cast() const noexcept { return nullptr; }
   MNL_INLINE inline bool val::operator==(decltype(nullptr)) const noexcept { return test<>(); }

   template<> MNL_INLINE inline bool val::test<long long>() const noexcept { return rep.tag() == 0x7FFAu; }
   template<> MNL_INLINE inline bool val::test<int>() const noexcept       { return test<long long>(); }
   template<> MNL_INLINE inline long long val::cast() const noexcept { return rep.dat<long long>(); }
   template<> MNL_INLINE inline int       val::cast() const noexcept { return cast<long long>(); }

   template<> MNL_INLINE inline bool val::is<double>() const noexcept         { return (rep.tag() & 0x7FF8u) != 0x7FF8u; }
   template<> MNL_INLINE inline bool val::is<const double &>() const noexcept { return is<double>(); }
   template<> MNL_INLINE inline double val::as() const noexcept                 { return rep.dat<double>(); }
   template<> MNL_INLINE inline double val::as<const double &>() const noexcept { return as<double>(); }

   template<> MNL_INLINE inline bool val::is<float>() const noexcept         { return rep.tag() == 0x7FFCu; }
   template<> MNL_INLINE inline bool val::is<const float &>() const noexcept { return is<float>(); }
   template<> MNL_INLINE inline float val::cast() const noexcept { return rep.dat<float>(); }

   template<> MNL_INLINE inline bool val::test<const sym &>() const noexcept { return rep.tag() == 0x7FFBu; }
   template<> MNL_INLINE inline bool val::test<sym>() const noexcept         { return test<const sym &>(); }
   template<> MNL_INLINE inline const sym &val::cast() const noexcept { return rep.dat<const sym &>(); }
   template<> MNL_INLINE inline sym        val::cast() const noexcept { return cast<const sym &>(); }
   MNL_INLINE inline bool val::operator==(const sym &rhs) const noexcept { return test<sym>() && cast<const sym &>() == rhs; }

   template<> MNL_INLINE inline bool val::test<bool>() const noexcept { return (rep.tag() & 0x7FFEu) == 0x7FFEu; }
   template<> MNL_INLINE inline bool val::cast() const noexcept { return rep.tag() & 1u; }

   template<> MNL_INLINE inline bool val::test<unsigned>() const noexcept { return rep.tag() == 0x7FFDu; }
   template<> MNL_INLINE inline bool val::test<char>() const noexcept     { return test<unsigned>(); }
   template<> MNL_INLINE inline unsigned val::cast() const noexcept { return rep.dat<unsigned>(); }
   template<> MNL_INLINE inline char     val::cast() const noexcept { return cast<unsigned>(); }

   MNL_INLINE inline void val::assume_f64() const noexcept { // performance hack
      if (
         rep.tag() == 0x7FF8u ||
         mnl::test<>(*this) ||
         mnl::test<long long>(*this) ||
         !mnl::test<double>(*this) ||
         mnl::test<float>(*this) ||
         mnl::test<sym>(*this) ||
         mnl::test<bool>(*this) || rep.tag() == 0x7FFEu || rep.tag() == 0x7FFFu ||
         mnl::test<unsigned>(*this) )
         MNL_UNREACHABLE();
      switch (rep.tag()) case 0x7FF8u: case 0x7FF9u: case 0x7FFAu: case 0x7FFBu: case 0x7FFCu: case 0x7FFDu: case 0x7FFEu: case 0x7FFFu:
         MNL_UNREACHABLE();
   }

// Signals, Exceptions, and Invocation Traces //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace pub {
   inline constexpr class sig_state {
   public:
      MNL_INLINE void raise(const sym &tag) const noexcept
         { pending = true, signal.first = tag; }
      template<typename Arg> MNL_INLINE void raise(const sym &tag, Arg &&arg) const noexcept
         { pending = true, signal.first = tag, signal.second.assign(std::forward<Arg>(arg)); }
      MNL_INLINE void raise(std::pair<sym, val> &&signal) const noexcept
         { pending = true, signal = std::move(signal); }
      MNL_INLINE operator bool() const noexcept
         { return pending; }
      MNL_INLINE const sym &tag() const noexcept
         { return signal.first; }
      MNL_INLINE std::pair<sym, val> cancel() const noexcept
         { std::pair<sym, val> signal; pending = false, signal.swap(this->signal); return signal; }
   private:
      inline static MNL_IF_WITH_MT(thread_local) bool                pending; // inexpensive
      inline static MNL_IF_WITH_MT(thread_local) std::pair<sym, val> signal;  // more expensive
   } sig_state;


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
      template<typename Dat> code(Dat dat): rep(new box<Dat>{std::move(dat)}) {}
      MNL_INLINE code compile(const form &form, const loc &loc) && { return rep->compile(std::move(*this), form, loc); }
      template<bool fast_sig = bool{}, bool nores = bool{}>
      MNL_INLINE auto execute() const { return rep->execute(std::bool_constant<fast_sig>{}, std::bool_constant<nores>{}); }
      MNL_INLINE void exec_in(const val &value) const { rep->exec_in(value); }
      MNL_INLINE void exec_in(val &&value)      const { rep->exec_in(std::move(value)); }
      MNL_INLINE val  exec_out() const { return rep->exec_out(); }
      MNL_INLINE bool is_rvalue() const noexcept { return rep->is_rvalue(); }
      MNL_INLINE bool is_lvalue() const noexcept { return rep->is_lvalue(); } // implies is_rvalue()
   public: // Extraction
      template<typename Dat> MNL_INLINE friend bool test(const code &rhs) noexcept
         { return rhs.rep->tag == &box<std::remove_cv_t<std::remove_reference_t<Dat>>>::tag; }
      template<typename Dat> MNL_INLINE friend Dat  cast(const code &rhs) noexcept
         { return static_cast<box<std::remove_cv_t<std::remove_reference_t<Dat>>> *>(rhs.rep)->dat; }
   public: // Mandatory bases for Dat
      struct nonvalue; struct rvalue; struct lvalue;
   private: // Concrete representation
      class root { public:
         /*atomic*/ long rc = 1;
         const void *const tag; // dynamic type identification
         MNL_INLINE explicit root(decltype(tag) tag) noexcept: tag(tag) {}
         root(root &&) = delete; // can only be constructed/destructed (not movied or copied)
         virtual ~root() = default;
         virtual code compile(code &&self, const form &, const loc &) const = 0;
         virtual val  execute(std::false_type, std::false_type) const = 0;
         virtual void execute(std::false_type, std::true_type)  const = 0;
         virtual val  execute(std::true_type,  std::false_type) const = 0;
         virtual void execute(std::true_type,  std::true_type)  const = 0;
         virtual void exec_in(const val &) const = 0;            // concrete dat (e.g., expr_tv) may have Dat::exec_in(long long) etc. for optimization purposes
         virtual void exec_in(val &&)      const = 0;
         virtual val  exec_out() const = 0;
         virtual bool is_rvalue() const noexcept = 0;
         virtual bool is_lvalue() const noexcept = 0; // shall imply is_rvalue()
      } *rep = {};
      template<typename Dat> class box final: public root { public:
         const Dat dat;
         static constexpr unsigned char tag = {}; // dynamic type identification
         static_assert(std::is_base_of_v<nonvalue, Dat>);
         explicit box(Dat dat) noexcept: root{&tag}, dat(std::move(dat)) {}
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

   template<enum sym::id _> std::enable_if_t<(_, true), void> op;

   template<> inline constexpr auto op<sym::id("+")> = [](){
      struct _ {
         auto operator()(int lhs, const val &rhs) const {
         }
         auto operator()(int lhs, val &&rhs) const {
         }
      };
      return _{};
   }();


   struct code::nonvalue {
      static code compile(code &&self, const form &, const loc &) = delete;
      MNL_INLINE static val execute(bool fast_sig = {}) noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static void exec_nores(bool fast_sig = {}) noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static void exec_in(const val &) noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static val exec_out() noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static bool is_rvalue() noexcept { return false; }
      MNL_INLINE static bool is_lvalue() noexcept { return false; }
   };
   struct code::rvalue: code::nonvalue {
      MNL_INLINE static code compile(code &&self, const form &form, const loc &loc) { return aux::compile_apply(std::move(self), form, loc); }
      static val execute(bool fast_sig = {}) = delete;
      MNL_INLINE static void exec_nores(bool fast_sig = {}) noexcept {} // may still be shadowed
      MNL_INLINE static bool is_rvalue() noexcept { return true; }
   };
   struct code::lvalue: code::rvalue {
      static void exec_in(const val &) = delete;
      static val exec_out() = delete;
      MNL_INLINE static bool is_lvalue() noexcept { return true; }
   };

}} // namespace aux::pub

// Primitive Operations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux { namespace pub { constexpr auto max_i48 = (1ll << 48 - 1) - 1, min_i48 = -max_i48; } }

namespace aux {

   // I48 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, long long>, int> = int{}>
   MNL_INLINE inline Dat _add(Dat lhs, Dat rhs) {
      if (MNL_LIKELY((unsigned long long)(lhs += rhs) - min_i48 <= max_i48 - min_i48)) return lhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, long long>, int> = int{}>
   MNL_INLINE inline Dat _sub(Dat lhs, Dat rhs) {
      if (MNL_LIKELY((unsigned long long)(lhs -= rhs) - min_i48 <= max_i48 - min_i48)) return lhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, long long>, int> = int{}>
   MNL_INLINE inline Dat _mul(Dat lhs, Dat rhs) {
      if (MNL_LIKELY(!__builtin_mul_overflow(lhs, rhs, &lhs)) && MNL_LIKELY((unsigned long long)res - min_i48 <= max_i48 - min_i48)) return lhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }

   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, long long>, int> = int{}>
      MNL_INLINE inline Dat _neg(Dat rhs) noexcept { return -rhs; }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, long long>, int> = int{}>
      MNL_INLINE inline Dat _abs(Dat rhs) noexcept { return std::abs(rhs); }

   // F64, F32 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, double> || std::is_same_v<Dat, float>, int> = int{}>
   MNL_INLINE inline Dat _add(Dat lhs, Dat rhs) {
      if (MNL_LIKELY(!std::isinf(lhs += rhs))) return lhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, double> || std::is_same_v<Dat, float>, int> = int{}>
   MNL_INLINE inline Dat _sub(Dat lhs, Dat rhs) {
      if (MNL_LIKELY(!std::isinf(lhs -= rhs))) return lhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, double> || std::is_same_v<Dat, float>, int> = int{}>
   MNL_INLINE inline Dat _mul(Dat lhs, Dat rhs) {
      if (MNL_LIKELY(!std::isinf(lhs *= rhs))) return lhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }

   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, double> || std::is_same_v<Dat, float>, int> = int{}>
      MNL_INLINE inline Dat _neg(Dat rhs) noexcept { return -rhs; }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, double> || std::is_same_v<Dat, float>, int> = int{}>
      MNL_INLINE inline Dat _abs(Dat rhs) noexcept { return std::abs(rhs); }

   // U32 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, unsigned>, int> = int{}>
      MNL_INLINE inline Dat _add(Dat lhs, Dat rhs) noexcept { return lhs + rhs; }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, unsigned>, int> = int{}>
      MNL_INLINE inline Dat _sub(Dat lhs, Dat rhs) noexcept { return lhs - rhs; }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, unsigned>, int> = int{}>
      MNL_INLINE inline Dat _mul(Dat lhs, Dat rhs) noexcept { return lhs * rhs; }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, unsigned>, int> = int{}>
      MNL_INLINE inline Dat _neg(Dat rhs) noexcept { return -rhs; }
   template<typename Dat, std::enable_if_t<std::is_same_v<Dat, unsigned>, int> = int{}>
      MNL_INLINE inline Dat _abs(Dat rhs) noexcept { return +rhs; }

} // namespace aux

namespace aux { namespace pub {

   // sym::Apply/Repl //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Apply

   template<typename Target,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
   val _apply(Target &&, int argc, val [], val *argv_out = {});
      // extern
   template<typename Target, std::size_t Argc,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, std::array<val, Argc> &&args, val *args_out = {})
      { return target(Argc, args.data(), args_out); }

   template<typename Target,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
   MNL_NORETURN inline val _apply(Target &&)
      { MNL_ERR(MNL_SYM("UnrecognizedOperation")); }
   template<typename Target, typename Arg0,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, Arg0 &&arg0)
      { return target(1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Arg0, val>, val &, val>)arg0)); }
   template<typename Target, typename Arg0, typename Arg1,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, Arg0 &&arg0, Arg1 &&arg1)
      { val argv[] = {std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)}; return target(std::size(argv), argv); }

   // Repl

   template<typename Target,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
   MNL_NODISCARD MNL_NORETURN inline val _repl(Target &&, int argc, val [], val *argv_out = {})
      { MNL_ERR(MNL_SYM("UnrecognizedOperation")); }
   template<typename Target, std::size_t Argc,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{}>
   MNL_NODISCARD MNL_NORETURN inline val _repl(Target &&, std::array<val, Argc> &&, val *args_out = {})
      { MNL_ERR(MNL_SYM("UnrecognizedOperation")); }
   template<typename Target, typename Arg0, typename Arg1,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
   MNL_NODISCARD MNL_NORETURN inline val _repl(Target &&, Arg0 &&, Arg1 &&, val *args_out = {})
      { MNL_ERR(MNL_SYM("UnrecognizedOperation")); }
   template<typename Target, typename Arg0, typename Arg1, typename Arg2,
      std::enable_if_t<std::is_same_v<Target, const sym &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg2, val> || std::is_same_v<Arg2, const val &>, int> = int{}>
   MNL_NODISCARD MNL_NORETURN inline val _repl(Target &&, Arg0 &&, Arg1 &&, Arg2 &&, val *args_out = {})
      { MNL_ERR(MNL_SYM("UnrecognizedOperation")); }

   // val::Apply/Repl //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Apply

   template<typename Target,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, int argc, val argv[], val *argv_out = {}) {
      if (MNL_LIKELY(target.rep.tag() == 0x7FF8u)) // BoxPtr (fallback)
         return static_cast<val::root *>(target.rep.template dat<void *>())->
            invoke(std::forward<Target>(target), MNL_SYM("Apply"), argc, argv, argv_out);
      if (MNL_LIKELY(target.rep.tag() == 0x7FFBu)) // Sym
         return cast<const sym &>(target)(argc, argv, argv_out);
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }
   template<typename Target, std::size_t Argc,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, std::array<val, Argc> &&args, val *args_out = {})
      { return std::forward<Target>(target)(Argc, args.data(), args_out); }

   template<typename Target,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target)
      { return std::forward<Target>(target)(0, {}); }
   template<typename Target, typename Arg0,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, Arg0 &&arg0) {
      if (MNL_LIKELY(target.rep.tag() == 0x7FF8u)) // BoxPtr (fallback)
         return static_cast<val::root *>(target.rep.template dat<void *>())->
            apply(std::forward<Target>(target), std::forward<Arg0>(arg0));
      if (MNL_LIKELY(target.rep.tag() == 0x7FFBu)) // Sym
         return cast<const sym &>(target)(std::forward<Arg0>(arg0));
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }
   template<typename Target, typename Arg0, typename Arg1,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{}>
   MNL_INLINE inline val _apply(Target &&target, Arg0 &&arg0, Arg1 &&arg1) {
      if (MNL_LIKELY(target.rep.tag() == 0x7FF8u)) // BoxPtr (fallback)
         return static_cast<val::root *>(target.rep.template dat<void *>())->
            apply(std::forward<Target>(target), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
      if (MNL_LIKELY(target.rep.tag() == 0x7FFBu)) // Sym
         return cast<const sym &>(target)(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }

   // Repl

   template<typename Target,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _repl(Target &&target, int argc, val argv[]) {
      if (MNL_LIKELY(target.rep.tag() == 0x7FF8u)) // BoxPtr (fallback)
         return static_cast<val::root *>(target.rep.template dat<void *>())->
            invoke(std::forward<Target>(target), MNL_SYM("Repl"), argc, argv);
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }
   template<typename Target, std::size_t Argc,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _repl(Target &&target, std::array<val, Argc> &&args)
      { return std::forward<Target>(target).repl(Argc, args.data()); }

   template< typename Target, typename Arg0, typename Arg1,
      std::enable_if_t<std::is_same_v<Target, const val &> || std::is_same_v<Target, val>,                                      int> = int{},
      std::enable_if_t<std::is_same_v<Arg0,   const val &> || std::is_same_v<Arg0,   val> || std::is_same_v<Arg0, const sym &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg1,   const val &> || std::is_same_v<Arg1,   val>,                                      int> = int{} >
   MNL_INLINE inline val _repl(Target &&target, Arg0 &&arg0, Arg1 &&arg1) {
      if (MNL_LIKELY(target.rep.tag() == 0x7FF8u)) // BoxPtr (fallback)
         return static_cast<val::root *>(target.rep.template dat<void *>())->
            repl(std::forward<Target>(target), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }
   template<typename Target, typename Arg0, typename Arg1, typename Arg2,
      std::enable_if_t<std::is_same_v<Target, val> || std::is_same_v<Target, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg0, val> || std::is_same_v<Arg0, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg1, val> || std::is_same_v<Arg1, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Arg2, val> || std::is_same_v<Arg2, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _repl(Target &&target, Arg0 &&arg0, Arg1 &&arg1, Arg2 &&arg2) {
      if (MNL_LIKELY(target.rep.tag() == 0x7FF8u)) // BoxPtr (fallback)
         return static_cast<val::root *>(target.rep.template dat<void *>())->
            repl(std::forward<Target>(target), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2));
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Lhs, typename Rhs,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _eq(Lhs &&lhs, Rhs &&rhs) {
      switch (lhs.rep.tag()) {
      case 0x7FF8u: return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs),
         MNL_SYM("=="), 1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs));
      case 0x7FF9u: return  test<>(rhs);
      case 0x7FFAu: return  MNL_LIKELY(test<long long>(rhs)) && cast<long long>(lhs) == cast<long long>(rhs);
      default:      return  MNL_LIKELY(test<double>(rhs)) && cast<double>(lhs) == cast<double>(rhs);
      case 0x7FFCu: return  MNL_LIKELY(test<float>(rhs)) && cast<float>(lhs) == cast<float>(rhs);
      case 0x7FFBu: return  MNL_LIKELY(test<sym>(rhs)) && cast<const sym &>(lhs) == cast<const sym &>(rhs);
      case 0x7FFEu: return  rhs.rep.tag() == 0x7FFEu;
      case 0x7FFFu: return  rhs.rep.tag() == 0x7FFFu;
      case 0x7FFDu: return  MNL_LIKELY(test<unsigned>(rhs)) && cast<unsigned>(lhs) == cast<unsigned>(rhs);
      }
   }
   template<typename Lhs, typename Rhs,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _ne(Lhs &&lhs, Rhs &&rhs) {
      switch (lhs.rep.tag()) {
      case 0x7FF8u: return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs),
         MNL_SYM("<>"), 1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs));
      case 0x7FF9u: return !test<>(rhs);
      case 0x7FFAu: return !MNL_LIKELY(test<long long>(rhs)) || cast<long long>(lhs) != cast<long long>(rhs);
      default:      return !MNL_LIKELY(test<double>(rhs)) || cast<double>(lhs) != cast<double>(rhs);
      case 0x7FFCu: return !MNL_LIKELY(test<float>(rhs)) || cast<float>(lhs) != cast<float>(rhs);
      case 0x7FFBu: return !MNL_LIKELY(test<sym>(rhs)) || cast<const sym &>(lhs) != cast<const sym &>(rhs);
      case 0x7FFEu: return  rhs.rep.tag() != 0x7FFEu;
      case 0x7FFFu: return  rhs.rep.tag() != 0x7FFFu;
      case 0x7FFDu: return !MNL_LIKELY(test<unsigned>(rhs)) || cast<unsigned>(lhs) != cast<unsigned>(rhs);
      }
   }
# define MNL_M(ID, OP) \
   template<typename Lhs, typename Rhs, \
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
   MNL_NODISCARD MNL_INLINE inline val ID(Lhs &&lhs, Rhs &&rhs) { \
      if (MNL_LIKELY(test<long long>(lhs))) { \
         if (MNL_UNLIKELY(!test<long long>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<long long>(lhs) OP cast<long long>(rhs); \
      } \
      if (MNL_UNLIKELY(test<unsigned>(lhs))) { \
         if (MNL_UNLIKELY(!test<unsigned>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<unsigned>(lhs) OP cast<unsigned>(rhs); \
      } \
      if (MNL_LIKELY(test<double>(lhs))) { \
         if (MNL_UNLIKELY(!test<double>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<double>(lhs) OP cast<double>(rhs); \
      } \
      if (MNL_UNLIKELY(lhs.rep.tag() == 0x7FF8u)) /* BoxPtr (fallback) */ \
         return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM(#OP), \
            1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs)); \
      if (MNL_LIKELY(test<float>(lhs))) { \
         if (MNL_UNLIKELY(!test<float>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<float>(lhs) OP cast<float>(rhs); \
      } \
      MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
   } \
// end # define MNL_M(ID, OP)
   MNL_M(_lt, <) MNL_M(_le, <=) MNL_M(_gt, >) MNL_M(_ge, >=)
# undef MNL_M

# define MNL_M(ID, SYM) \
   template<typename Lhs, typename Rhs, \
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
   MNL_NODISCARD MNL_INLINE inline val ID(Lhs &&lhs, Rhs &&rhs) { \
      if (MNL_LIKELY(test<long long>(lhs))) { \
         if (MNL_UNLIKELY(!test<long long>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::ID(cast<long long>(lhs), cast<long long>(rhs)); \
      } \
      if (MNL_UNLIKELY(test<unsigned>(lhs))) { \
         if (MNL_UNLIKELY(!test<unsigned>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::ID(cast<unsigned>(lhs), cast<unsigned>(rhs)); \
      } \
      if (MNL_LIKELY(test<double>(lhs))) { \
         if (MNL_UNLIKELY(!test<double>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::ID(cast<double>(lhs), cast<double>(rhs)); \
      } \
      if (MNL_UNLIKELY(lhs.rep.tag() == 0x7FF8u)) /* BoxPtr (fallback) */ \
         return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM(SYM), \
            1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs)); \
      if (MNL_LIKELY(test<float>(lhs))) { \
         if (MNL_UNLIKELY(!test<float>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::ID(cast<float>(lhs), cast<float>(rhs)); \
      } \
      MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
   } \
// end # define MNL_M(ID, SYM)
   MNL_M(_add, "+") MNL_M(_sub, "-") MNL_M(_mul, "*")
# undef MNL_M
# define MNL_M(ID, SYM) \
   template<typename Rhs, \
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}> \
   MNL_NODISCARD MNL_INLINE inline val ID(Rhs &&rhs) { \
      if (MNL_LIKELY(test<long long>(lhs))) \
         return aux::ID(cast<long long>(rhs)); \
      if (MNL_UNLIKELY(test<unsigned>(lhs))) \
         return aux::ID(cast<unsigned>(rhs)); \
      if (MNL_LIKELY(test<double>(lhs))) \
         return aux::ID(cast<double>(rhs)); \
      if (MNL_UNLIKELY(test<float>(lhs))) \
         return aux::ID(cast<float>(rhs)); \
      if (MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) /* BoxPtr (fallback) */ \
         return static_cast<val::root *>(rhs.rep.dat<void *>())->invoke(std::forward<Rhs>(rhs), MNL_SYM(SYM), 0, {}); \
      MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
   } \
// end # define MNL_M(ID, SYM)
   MNL_M(_neg, "Neg") MNL_M(_abs, "Abs")
# undef MNL_M
   template<typename Lhs, typename Rhs,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _xor(Lhs &&lhs, Rhs &&rhs) {
      switch (lhs.rep.tag()) {
      case 0x7FF8u: // BoxPtr (fallback)
         return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("Xor"),
            1, &const_cast<val &>((const val &)(std::conditional_t<std::is_same_v<Rhs, val>, val &, val>)rhs));
      default:
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFEu: // Bool/False
         if (MNL_UNLIKELY(!test<bool>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return val{decltype(val::rep){rhs.rep.tag()}};
      case 0x7FFFu: // Bool/True
         if (MNL_UNLIKELY(!test<bool>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return val{decltype(val::rep){rhs.rep.tag() ^ 1}};
      case 0x7FFDu: // U32
         if (MNL_UNLIKELY(!test<unsigned>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return cast<unsigned>(lhs) ^ cast<unsigned>(rhs);
      }
   }
   template<typename Rhs,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _not(Rhs &&rhs) { \
      switch (rhs.rep.tag()) {
      case 0x7FF8u: // BoxPtr (fallback)
         return static_cast<val::root *>(rhs.rep.dat<void *>())->invoke(std::forward<Rhs>(rhs), MNL_SYM("~"), 0, {});
      case 0x7FF9u: case 0x7FFBu:
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFEu: return true;
      case 0x7FFFu: return false;
      case 0x7FFDu: return ~cast<unsigned>(rhs);
      case 0x7FFAu: return aux::_neg(cast<long long>(rhs)); // Neg(ation)
      default:      return aux::_neg(cast<double>(rhs));    // Neg(ation)
      case 0x7FFCu: return aux::_neg(cast<float>(rhs));     // Neg(ation)
      }
   }

   // I48, F64, F32, U32 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Lhs, typename Dat,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _eq(Lhs &&lhs, Dat rhs) {
      if (MNL_LIKELY(test<Dat>(lhs))) return cast<Dat>(lhs) == rhs;
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return false;
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("=="), 1, &const_cast<val &>((const val &)rhs));
   }
   template<typename Lhs, typename Dat,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _ne(Lhs &&lhs, Dat rhs) {
      if (MNL_LIKELY(test<Dat>(lhs))) return cast<Dat>(lhs) != rhs;
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return true;
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("<>"), 1, &const_cast<val &>((const val &)rhs));
   }
# define MNL_M(ID, OP) \
   template<typename Lhs, typename Dat, \
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}> \
   MNL_NODISCARD MNL_INLINE inline val ID(Lhs &&lhs, Dat rhs) { \
      if (MNL_LIKELY(test<Dat>(lhs))) return cast<Dat>(lhs) OP rhs; \
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return [&]()MNL_NORETURN{ return ID(std::forward<Lhs>(lhs), (val)rhs); }(); \
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM(#OP), 1, &const_cast<val &>((const val &)rhs)); \
   } \
// end # define MNL_M(ID, OP)
   MNL_M(_lt, <) MNL_M(_le, <=) MNL_M(_gt, >) MNL_M(_ge, >=)
# undef MNL_M
# define MNL_M(ID, SYM) \
   template<typename Lhs, typename Dat, \
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{}, \
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}> \
   MNL_NODISCARD MNL_INLINE inline val ID(Lhs &&lhs, Dat rhs) { \
      if (MNL_LIKELY(test<Dat>(lhs))) return aux::ID(cast<Dat>(lhs), rhs); \
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return [&]()MNL_NORETURN{ return ID(std::forward<Lhs>(lhs), (val)rhs); }(); \
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM(SYM), 1, &const_cast<val &>((const val &)rhs)); \
   } \
// end # define MNL_M(ID, SYM)
   MNL_M(_add, "+") MNL_M(_sub, "-") MNL_M(_mul, "*")
# undef MNL_M

   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool _eq(Dat lhs, Rhs &&rhs) noexcept
      { return  MNL_LIKELY(test<Dat>(rhs)) && lhs == cast<Dat>(rhs); }
   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool _ne(Dat lhs, Rhs &&rhs) noexcept
      { return !MNL_LIKELY(test<Dat>(rhs)) || lhs != cast<Dat>(rhs); }
# define MNL_M(ID, OP) \
   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool ID(Dat lhs, Rhs &&rhs)
      { if (MNL_LIKELY(test<Dat>(rhs))) return lhs OP cast<Dat>(rhs); MNL_ERR(MNL_SYM("TypeMismatch")); } \
// end # define MNL_M(ID, OP)
   MNL_M(_lt, <) MNL_M(_le, <=) MNL_M(_gt, >) MNL_M(_ge, >=)
# undef MNL_M
# define MNL_M(ID) \
   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, int> || std::is_same_v<Dat, double> || std::is_same_v<Dat, float> || std::is_same_v<Dat, unsigned>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline Dat ID(Dat lhs, Rhs &&rhs)
      { if (MNL_LIKELY(test<Dat>(rhs))) return aux::ID(lhs, cast<Dat>(rhs)); MNL_ERR(MNL_SYM("TypeMismatch")); } \
// end # define MNL_M(ID)
   MNL_M(_add) MNL_M(_sub) MNL_M(_mul)
# undef MNL_M

   // Misc /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Lhs, typename Dat,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, const sym &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _eq(Lhs &&lhs, Dat rhs) {
      if (MNL_LIKELY(test<Dat>(lhs))) return cast<const Dat &>(lhs) == rhs;
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return false;
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("=="), 1, &const_cast<val &>((const val &)rhs));
   }
   template<typename Lhs, typename Dat,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, const sym &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _ne(Lhs &&lhs, Dat rhs) {
      if (MNL_LIKELY(test<Dat>(lhs))) return cast<const Dat &>(lhs) != rhs;
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return true;
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("<>"), 1, &const_cast<val &>((const val &)rhs));
   }
   template<typename Lhs, typename Dat,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, decltype(nullptr)>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _eq(Lhs &&lhs, Dat rhs) {
      if (MNL_LIKELY(test<>(lhs))) return true;
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return false;
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("=="), 1, &const_cast<val &>((const val &)rhs));
   }
   template<typename Lhs, typename Dat,
      std::enable_if_t<std::is_same_v<Lhs, val> || std::is_same_v<Lhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, decltype(nullptr)>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline val _ne(Lhs &&lhs, Dat rhs) {
      if (MNL_LIKELY(test<>(lhs))) return false;
      if (!MNL_LIKELY(lhs.rep.tag() == 0x7FF8u)) return true;
      return static_cast<val::root *>(lhs.rep.template dat<void *>())->invoke(std::forward<Lhs>(lhs), MNL_SYM("<>"), 1, &const_cast<val &>((const val &)rhs));
   }

   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, const sym &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool _eq(Dat lhs, Rhs &&rhs) noexcept
      { return  MNL_LIKELY(test<Dat>(rhs)) && lhs == cast<Dat>(rhs); }
   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, const sym &>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool _ne(Dat lhs, Rhs &&rhs) noexcept
      { return !MNL_LIKELY(test<Dat>(rhs)) || lhs != cast<Dat>(rhs); }
   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, decltype(nullptr)>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool _eq(Dat lhs, Rhs &&rhs) noexcept
      { return  test<>(rhs); }
   template<typename Rhs, typename Dat,
      std::enable_if_t<std::is_same_v<Rhs, val> || std::is_same_v<Rhs, const val &>, int> = int{},
      std::enable_if_t<std::is_same_v<Dat, decltype(nullptr)>, int> = int{}>
   MNL_NODISCARD MNL_INLINE inline bool _ne(Dat lhs, Rhs &&rhs) noexcept
      { return !test<>(rhs); }

   // Tracing //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

# define MNL_M(ID) \
   template<typename ...Items> auto ID(const loc &loc, Items &&...items) noexcept(noexcept(ID(std::forward<Items>(items) ...))) \
      { try { return ID(std::forward<Items>(items) ...); } catch (...) { MNL_NORETURN void trace_execute(const mnl::loc &); trace_execute(loc); } } \
// end # define MNL_M(ID)
   MNL_M(_apply)
   MNL_M(_eq) MNL_M(_ne) MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge)
   MNL_M(_add) MNL_M(_sub) MNL_M(_mul) MNL_M(_neg) MNL_M(_abs) MNL_M(_xor) MNL_M(_not)
# undef MNL_M

}} // namespace aux::pub

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
   class record_descr/*iptor*/ { // TODO: record_sign/*ature*/
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





// part of public interface, like val::operator(), val::repl, etc (kind of argument-simmetric and ADL-able under some circumstances):
// should not have templated params to be ADL-safe (assuming rhs might be an std::type at some point in the future)

namespace aux::mnl {
# define MNL_M(OP1, OP2) \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs,  long long rhs)   { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs,  double    rhs)   { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs,  float     rhs)   { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs,  unsigned  rhs)   { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs,  const val &rhs)  { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs,  val       &&rhs) { return val::OP2(lhs, std::move(rhs)); } \
   MNL_INLINE inline val aux::pub::OP1(val       &&lhs, long long rhs)   { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val       &&lhs, double    rhs)   { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val       &&lhs, float     rhs)   { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val       &&lhs, unsigned  rhs)   { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val       &&lhs, const val &rhs)  { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val       &&lhs, val       &&rhs) { return val::OP2(std::move(lhs), std::move(rhs)); } \
// end MNL_M
   MNL_M(operator+, _add) MNL_M(operator-, _sub) MNL_M(operator*, _mul)
   MNL_M(operator<, _lt) MNL_M(operator<=, _le) MNL_M(operator>, _gt) MNL_M(operator>=, _ge)
# undef MNL_M
# define MNL_M(OP1, OP2) \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, long long rhs)     { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, double    rhs)     { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, float     rhs)     { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, const sym &rhs)    { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, bool      rhs)     { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, decltype(nullptr)) { return val::OP2(lhs, nullptr); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, unsigned  rhs)     { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, const val &rhs)    { return val::OP2(lhs, rhs); } \
   MNL_INLINE inline val aux::pub::OP1(const val &lhs, val       &&rhs)   { return val::OP2(lhs, std::move(rhs)); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, long long rhs)     { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, double    rhs)     { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, float     rhs)     { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, const sym &rhs)    { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, bool      rhs      { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, decltype(nullptr)) { return val::OP2(std::move(lhs), nullptr); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, unsigned  rhs)     { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, const val &rhs)    { return val::OP2(std::move(lhs), rhs); } \
   MNL_INLINE inline val aux::pub::OP1(val &&lhs, val       &&rhs)   { return val::OP2(std::move(lhs), std::move(rhs)); } \
// end MNL_M
   MNL_M(operator==, _eq) MNL_M(operator!=, _ne)
# undef MNL_M

// OR, maybe member functions is more appropriate?? (since the idea here is dispatching by the first argument)

}

   class val {
      ...
   public: // public, but the combination of Dat, Val exists for optimizations, not so as a "public" API
      template<typename Lhs, typename Rhs> static val _add(Lhs &&, Rhs &&); // use if constexpr in the definition


   };












} // namespace MNL_AUX_UUID
