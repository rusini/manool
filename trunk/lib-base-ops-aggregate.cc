// lib-base-ops-aggreg.cc

/*    Copyright (C) 2018, 2019 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-lib-base.hh"
# include "manool.hh"

# include <iterator> // next, prev
# include <mutex> // TODO: provisional!

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::next; using std::prev; // <iterator>
      using std::list;
   }

namespace aux { namespace {

   template<typename Traits /*iterator, fetch*/> class mnl_iter: Traits {
   public:
      MNL_INLINE mnl_iter(val &&base, long size, typename Traits::iterator begin) noexcept: base(move(base)), size(size), cache{0, begin} {}
      MNL_INLINE mnl_iter(mnl_iter &&rhs) noexcept: base(move(rhs.base)), size(rhs.size), cache(rhs.cache) {}
   private:
      val base; long size;
      mutable std::mutex mutex; pair<long, typename Traits::iterator> cache; // TODO: MNL_IF_WITH_MT
   private:
      MNL_INLINE inline val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         switch (MNL_DISP("Apply", "Size", "Elems", "^")[op]) {
         case 1: // Apply
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= size)) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return Traits::fetch((std::lock_guard<std::mutex>{mutex}, cache =
               {(long)cast<long long>(argv[0]), std::next(cache.second, (long)cast<long long>(argv[0]) - cache.first)}).second);
         case 2: // Size
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return (long long)size;
         case 3: // Elems
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return move(self);
         case 4: // ^
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            {  vector<val> res; res.reserve(size);
               auto it = std::prev(cache.second, cache.first);
               for (auto count = size; count; --count) res.push_back(Traits::fetch(it++));
               return res;
            }
         }
         return self.default_invoke(op, argc, argv);
      }
      friend box<mnl_iter>;
   };

   template<typename Iterator> MNL_INLINE inline Iterator seek(long size, Iterator begin, Iterator end, long index)
      { return index < size - index ? std::next(begin, index) : std::prev(end, size - index); }
   template<typename Container> MNL_INLINE inline typename Container::iterator seek(Container &cont, long index)
      { return seek(cont.size(), cont.begin(), cont.end(), index); }
   template<typename Container> MNL_INLINE inline typename Container::const_iterator seek(const Container &cont, long index)
      { return seek(cont.size(), cont.begin(), cont.end(), index); }
   template<typename Container> MNL_INLINE inline typename Container::const_iterator cseek(const Container &cont, long index)
      { return seek(cont.size(), cont.begin(), cont.end(), index); }

}} // namespace aux::<anon>

   template<> val box<dict<val, val>>::invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) {
      switch (MNL_DISP("Apply", "Exists", "Repl", "Delete", "Size", "Keys", "Elems", "==", "<>", "Order", "Succ", "Pred", "Clone", "DeepClone")[op]) {
      case  1: // Apply
         if (MNL_UNLIKELY(argc < 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         {  auto it = dat.find(argv[0]); if (MNL_UNLIKELY(it == dat.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            return MNL_LIKELY(argc == 1) ? it->second : it->second(argc - 1, argv + 1);
         }
      case  2: // Exists
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return dat.find(argv[0]) != dat.end();
      case  3: // Repl
         if (MNL_UNLIKELY(argc < 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(rc() == 1)) {
            if (MNL_LIKELY(argc == 2)) {
               if (MNL_UNLIKELY(argv_out)) { auto it = dat.find(argv[0]); if (MNL_LIKELY(it != dat.end())) argv_out[1].swap(it->second); }
               dat.set(argv[0], move(argv[1]));
            } else {
               auto it = dat.find(argv[0]);
               if (MNL_UNLIKELY(it == dat.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
               it->second = op(argc, (argv[0].swap(it->second), argv), argv_out);
            }
            return move(self);
         } else {
            auto res = dat;
            if (MNL_LIKELY(argc == 2)) {
               if (MNL_UNLIKELY(argv_out)) { auto it = res.find(argv[0]); if (MNL_LIKELY(it != res.end())) argv_out[1].swap(it->second); }
               res.set(argv[0], move(argv[1]));
            } else {
               auto it = res.find(argv[0]);
               if (MNL_UNLIKELY(it == res.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
               it->second = op(argc, (argv[0].swap(it->second), argv), argv_out);
            }
            return res;
         }
      case  4: // Delete
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(rc() == 1)) return dat.unset(argv[0]), move(self);
         { auto res = dat; res.unset(argv[0]); return res; }
      case  5: // Size
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return (long long)dat.size();
      case  6: // Keys
         if (MNL_LIKELY(argc == 0)) {
            struct traits {
               typedef dict<val, val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->first; }
            };
            return mnl_iter<traits>{move(self), (long)dat.size(), dat.cbegin()};
         }
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef dict<val, val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->first; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<>>(argv[0]).hi - (long)cast<range<>>(argv[0]).lo,
               cseek(dat, cast<range<>>(argv[0]).lo)};
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef dict<val, val>::const_reverse_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->first; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<true>>(argv[0]).hi - (long)cast<range<true>>(argv[0]).lo,
               traits::iterator{cseek(dat, cast<range<true>>(argv[0]).hi)}};
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case  7: // Elems
         if (MNL_LIKELY(argc == 0)) {
            struct traits {
               typedef dict<val, val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->second; }
            };
            return mnl_iter<traits>{move(self), (long)dat.size(), dat.cbegin()};
         }
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef dict<val, val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->second; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<>>(argv[0]).hi - (long)cast<range<>>(argv[0]).lo,
               cseek(dat, cast<range<>>(argv[0]).lo)};
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef dict<val, val>::const_reverse_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->second; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<true>>(argv[0]).hi - (long)cast<range<true>>(argv[0]).lo,
               traits::iterator{cseek(dat, cast<range<true>>(argv[0]).hi)}};
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case  8: // ==
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val, val>>(argv[0]))) return false;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const dict<val, val> &>(argv[0])) return true;)
         for (auto lhs = dat.cbegin(), rhs = cast<const dict<val, val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs == cast<const dict<val, val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const dict<val, val> &>(argv[0]).end())) return false;
            if (
               MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{lhs->first,  rhs->first}))) ||
               MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{lhs->second, rhs->second}))) ) return false;
         }
      case  9: // <>
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val, val>>(argv[0]))) return true;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const dict<val, val> &>(argv[0])) return false;)
         for (auto lhs = dat.cbegin(), rhs = cast<const dict<val, val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs != cast<const dict<val, val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const dict<val, val> &>(argv[0]).end())) return true;
            if (
               MNL_UNLIKELY( safe_cast<bool>(op(args<2>{lhs->first,  rhs->first}))) ||
               MNL_UNLIKELY( safe_cast<bool>(op(args<2>{lhs->second, rhs->second}))) ) return true;
         }
      case 10: // Order
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val, val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const dict<val, val> &>(argv[0])) return 0;)
         for (auto lhs = dat.cbegin(), rhs = cast<const dict<val, val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return -(rhs != cast<const dict<val, val> &>(argv[0]).end());
            if (MNL_UNLIKELY(rhs == cast<const dict<val, val> &>(argv[0]).end())) return +1;
            { auto res = safe_cast<long long>(op(args<2>{lhs->first,  rhs->first}));  if (MNL_UNLIKELY(res)) return res; }
            { auto res = safe_cast<long long>(op(args<2>{lhs->second, rhs->second})); if (MNL_UNLIKELY(res)) return res; }
         }
      case 11: // Succ
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         {  auto it = dat.find(argv[0]); if (MNL_UNLIKELY(it == dat.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            if (MNL_UNLIKELY(++it == dat.end())) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return it->first;
         }
      case 12: // Pred
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         {  auto it = dat.find(argv[0]); if (MNL_UNLIKELY(it == dat.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            if (MNL_UNLIKELY(it == dat.begin())) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return (--it)->first;
         }
      case 13: // Clone
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1) ? move(self) : dat;
      case 14: // DeepClone
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return [this]()->val{ val res = dat; for (auto &&el: cast<dict<val, val> &>(res)) el.second = MNL_SYM("DeepClone")(move(el.second)); return res; }();
      }
      return self.default_invoke(op, argc, argv);
   }

   template<> val box<dict<val>>::invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      switch (MNL_DISP("Apply", "Exists", "Repl", "Delete", "Size", "+", "|", "*", "-", "/", "Keys", "Elems", "==", "<>", "Order", "Succ", "Pred", "Clone")[op]) {
      case  1: case  2: // Apply/Exists
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return dat.find(argv[0]) != dat.end();
      case  3: // Repl
         if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<bool>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_LIKELY(rc() == 1)) {
            if (cast<bool>(argv[1])) dat.set(argv[0]); else dat.unset(argv[0]);
            return move(self);
         } else {
            auto res = dat;
            if (cast<bool>(argv[1])) res.set(argv[0]); else res.unset(argv[0]);
            return res;
         }
      case  4: // Delete
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(rc() == 1)) return dat.unset(argv[0]), move(self);
         { auto res = dat; res.unset(argv[0]); return res; }
      case  5: // Size
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return (long long)dat.size();
      case  6: // +
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (dat.size() >= cast<const dict<val> &>(argv[0]).size())
         if (MNL_LIKELY(rc() == 1) || cast<const dict<val> &>(argv[0]).empty()) {
            for (auto &&el: cast<const dict<val> &>(argv[0])) dat.set(el.first);
            return move(self);
         } else {
            auto res = dat;
            for (auto &&el: cast<const dict<val> &>(argv[0])) res.set(el.first);
            return res;
         } else
         if (MNL_LIKELY(argv[0].rc() == 1) || dat.empty()) {
            for (auto &&el: dat) cast<dict<val> &>(argv[0]).set(el.first);
            return move(argv[0]);
         } else {
            auto res = cast<const dict<val> &>(argv[0]);
            for (auto &&el: dat) res.set(el.first);
            return res;
         }
      case  7: // |
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(rc() == 1)) return dat.set(move(argv[0])), move(self);
         { auto res = dat; res.set(move(argv[0])); return res; }
      case  8: // *
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         {  decltype(dat) res;
            if (dat.size() >= cast<const dict<val> &>(argv[0]).size()) {
               for (auto &&el: cast<const dict<val> &>(argv[0])) if (dat.find(el.first) != dat.end()) res.set(el.first);
            } else {
               for (auto &&el: dat) if (cast<const dict<val> &>(argv[0]).find(el.first) != cast<const dict<val> &>(argv[0]).end()) res.set(el.first);
            }
            return res;
         }
      case  9: // -
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (dat.size() >= cast<const dict<val> &>(argv[0]).size())
         if (MNL_LIKELY(rc() == 1) || cast<const dict<val> &>(argv[0]).empty()) {
            for (auto &&el: cast<const dict<val> &>(argv[0])) dat.unset(el.first);
            return move(self);
         } else {
            auto res = dat;
            for (auto &&el: cast<const dict<val> &>(argv[0])) res.unset(el.first);
            return res;
         } else
         if (dat.empty()) return move(self); else {
            decltype(dat) res;
            for (auto &&el: dat) if (cast<const dict<val> &>(argv[0]).find(el.first) == cast<const dict<val> &>(argv[0]).end()) res.set(el.first);
            return res;
         }
      case 10: // /
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (cast<const dict<val> &>(argv[0]).empty()) return move(self);
         if (dat.empty()) return move(argv[0]);
         {  decltype(dat) res;
            for (auto &&el: dat) if (cast<const dict<val> &>(argv[0]).find(el.first) == cast<const dict<val> &>(argv[0]).end()) res.set(el.first);
            for (auto &&el: cast<const dict<val> &>(argv[0])) if (dat.find(el.first) == dat.end()) res.set(el.first);
            return res;
         }
      case 11: case 12: // Keys/Elems
         if (MNL_LIKELY(argc == 0)) {
            struct traits {
               typedef dict<val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->first; }
            };
            return mnl_iter<traits>{move(self), (long)dat.size(), dat.cbegin()};
         }
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef dict<val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->first; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<>>(argv[0]).hi - (long)cast<range<>>(argv[0]).lo,
               cseek(dat, cast<range<>>(argv[0]).lo)};
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef dict<val>::const_reverse_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return it->first; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<true>>(argv[0]).hi - (long)cast<range<true>>(argv[0]).lo,
               traits::iterator{cseek(dat, cast<range<true>>(argv[0]).hi)}};
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case 13: // ==
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) return false;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const dict<val> &>(argv[0])) return true;)
         for (auto lhs = dat.cbegin(), rhs = cast<const dict<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs == cast<const dict<val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const dict<val> &>(argv[0]).end())) return false;
            if (MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{lhs->first, rhs->first})))) return false;
         }
      case 14: // <>
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) return true;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const dict<val> &>(argv[0])) return false;)
         for (auto lhs = dat.cbegin(), rhs = cast<const dict<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs != cast<const dict<val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const dict<val> &>(argv[0]).end())) return true;
            if (MNL_UNLIKELY( safe_cast<bool>(op(args<2>{lhs->first, rhs->first})))) return true;
         }
      case 15: // Order
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<dict<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const dict<val> &>(argv[0])) return 0;)
         for (auto lhs = dat.cbegin(), rhs = cast<const dict<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return -(rhs != cast<const dict<val> &>(argv[0]).end());
            if (MNL_UNLIKELY(rhs == cast<const dict<val> &>(argv[0]).end())) return +1;
            auto res = safe_cast<long long>(op(args<2>{lhs->first, rhs->first})); if (MNL_UNLIKELY(res)) return res;
         }
      case 16: // Succ
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         {  auto it = dat.find(argv[0]); if (MNL_UNLIKELY(it == dat.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            if (MNL_UNLIKELY(++it == dat.end())) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return it->first;
         }
      case 17: // Pred
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         {  auto it = dat.find(argv[0]); if (MNL_UNLIKELY(it == dat.end())) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            if (MNL_UNLIKELY(it == dat.begin())) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return (--it)->first;
         }
      case 18: // Clone
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1) ? move(self) : dat;
      }
      return self.default_invoke(op, argc, argv);
   }

   template<> val box<list<val>>::invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) {
      switch (MNL_DISP("Apply", "Repl", "Size", "+", "|", "Elems", "Keys", "==", "<>", "Order", "Clone", "DeepClone")[op]) {
      case  1: // Apply
         if (MNL_LIKELY(argc == 1)) {
            if (MNL_LIKELY(test<long long>(argv[0]))) { // Sequence[Index]
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return *cseek(dat, cast<long long>(argv[0]));
            }
            if (MNL_LIKELY(test<range<>>(argv[0]))) { // Sequence[Range[Low; High]]
               if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if (MNL_LIKELY(rc() == 1)) return
                  dat.erase(std::prev(dat.end(), (long)dat.size() - (long)cast<range<>>(argv[0]).hi), dat.end()),
                  dat.erase(dat.begin(), std::next(dat.begin(), cast<range<>>(argv[0]).lo)), move(self);
               return list<val>(
                  cseek(dat, cast<range<>>(argv[0]).lo),
                  cseek(dat, cast<range<>>(argv[0]).hi));
            }
            if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Sequence[RevRange[Low; High]] - IMDOOs: all
               if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return list<val>(
                  list<val>::const_reverse_iterator{cseek(dat, cast<range<true>>(argv[0]).hi)},
                  list<val>::const_reverse_iterator{cseek(dat, cast<range<true>>(argv[0]).lo)});
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }
         if (MNL_LIKELY(argc > 1)) { // Sequence[Index; ...]
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return (*cseek(dat, cast<long long>(argv[0])))(argc - 1, argv + 1);
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case  2: // Repl
         if (MNL_LIKELY(argc == 2)) {
            if (MNL_LIKELY(test<long long>(argv[0]))) { // Sequence.Repl[Index; NewElem]
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if (MNL_LIKELY(rc() == 1)) {
                  argv[1].swap(*seek(dat, cast<long long>(argv[0])));
                  if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
                  return move(self);
               }
               auto res = dat;
               argv[1].swap(*seek(res, cast<long long>(argv[0])));
               if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
               return res;
            }
            if (MNL_UNLIKELY(!test<list<val>>(argv[1])))
               MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_LIKELY(test<range<>>(argv[0]))) { // Sequence.Repl[Range[Low; High]; NewSequence]
               if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if (MNL_LIKELY(rc() == 1)) return
                  MNL_LIKELY(argv[1].rc() == 1)
                  ? (void)dat.splice(dat.erase(seek(dat, cast<range<>>(argv[0]).lo), seek(dat, cast<range<>>(argv[0]).hi)),
                     cast<list<val> &>(argv[1]))
                  : (void)dat.insert(dat.erase(seek(dat, cast<range<>>(argv[0]).lo), seek(dat, cast<range<>>(argv[0]).hi)),
                     cast<const list<val> &>(argv[1]).begin(), cast<const list<val> &>(argv[1]).end()), move(self);
               if (MNL_LIKELY(argv[1].rc() == 1)) return
                  cast<list<val> &>(argv[1]).insert(cast<list<val> &>(argv[1]).begin(), dat.cbegin(), cseek(dat, cast<range<>>(argv[0]).lo)),
                  cast<list<val> &>(argv[1]).insert(cast<list<val> &>(argv[1]).end(), cseek(dat, cast<range<>>(argv[0]).hi), dat.cend()), move(argv[1]);
               if (cast<range<>>(argv[0]).lo == cast<range<>>(argv[0]).hi && cast<const list<val> &>(argv[1]).empty()) return move(self);
               if (!cast<range<>>(argv[0]).lo && cast<range<>>(argv[0]).hi == dat.size()) return move(argv[1]);
               list<val> res(cast<const list<val> &>(argv[1]).begin(), cast<const list<val> &>(argv[1]).end());
               res.insert(res.begin(), dat.cbegin(), cseek(dat, cast<range<>>(argv[0]).lo));
               res.insert(res.end(), cseek(dat, cast<range<>>(argv[0]).hi), dat.cend());
               return res;
            }
            if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Sequence.Repl[RevRange[Low; High]; NewSequence]
               if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if (MNL_LIKELY(rc() == 1)) return
                  MNL_LIKELY(argv[1].rc() == 1)
                  ? (void)dat.splice(dat.erase(seek(dat, cast<range<true>>(argv[0]).lo), seek(dat, cast<range<true>>(argv[0]).hi)),
                     (cast<list<val> &>(argv[1]).reverse(), cast<list<val> &>(argv[1])))
                  : (void)dat.insert(dat.erase(seek(dat, cast<range<true>>(argv[0]).lo), seek(dat, cast<range<>>(argv[0]).hi)),
                     cast<const list<val> &>(argv[1]).rbegin(), cast<const list<val> &>(argv[1]).rend()), move(self);
               if (MNL_LIKELY(argv[1].rc() == 1)) return
                  cast<list<val> &>(argv[1]).reverse(),
                  cast<list<val> &>(argv[1]).insert(cast<list<val> &>(argv[1]).begin(), dat.cbegin(), cseek(dat, cast<range<true>>(argv[0]).lo)),
                  cast<list<val> &>(argv[1]).insert(cast<list<val> &>(argv[1]).end(), cseek(dat, cast<range<true>>(argv[0]).hi), dat.cend()), move(argv[1]);
               if (cast<range<true>>(argv[0]).lo == cast<range<true>>(argv[0]).hi && cast<const list<val> &>(argv[1]).empty()) return move(self);
               list<val> res(cast<const list<val> &>(argv[1]).rbegin(), cast<const list<val> &>(argv[1]).rend());
               res.insert(res.begin(), dat.cbegin(), cseek(dat, cast<range<true>>(argv[0]).lo));
               res.insert(res.end(), cseek(dat, cast<range<true>>(argv[0]).hi), dat.cend());
               return res;
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }
         if (MNL_LIKELY(argc > 2)) { // Sequence.Repl[Index; ...; NewElem]
            if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            long index = cast<long long>(argv[0]);
            if (MNL_LIKELY(rc() == 1)) return *seek(dat, index) = op(argc, (argv[0].swap(*seek(dat, index)), argv), argv_out), move(self);
            auto res = dat; *seek(res, index) = op(argc, (argv[0].swap(*seek(res, index)), argv), argv_out); return res;
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case  3: // Size
         // TODO: in CentOS-6, even with SCL, list<T>::size() complexity is O(N), whereas in Ubuntu 18.04 LTS it's O(1)
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return (long long)dat.size();
      case  4: // +
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<list<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return
              MNL_LIKELY(rc() == 1)
            ? (MNL_LIKELY(argv[0].rc() == 1) ? dat.splice(dat.end(), cast<list<val> &>(argv[0]))
              : (void)dat.insert(dat.end(), cast<const list<val> &>(argv[0]).begin(), cast<const list<val> &>(argv[0]).end()),
              move(self))
            : MNL_LIKELY(argv[0].rc() == 1)
            ? (cast<list<val> &>(argv[0]).insert(cast<list<val> &>(argv[0]).begin(), dat.cbegin(), dat.cend()), move(argv[0]))
            : cast<const list<val> &>(argv[0]).empty() ? move(self) : dat.empty() ? move(argv[0])
            : [&]()->val{ val res = dat;
               cast<list<val> &>(res).insert(cast<list<val> &>(res).end(), cast<const list<val> &>(argv[0]).begin(), cast<const list<val> &>(argv[0]).end());
               return res;
            }();
      case  5: // |
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1) ? (dat.push_back(argv[0]), move(self))
            : [&]()->val{ val res = dat; cast<list<val> &>(res).push_back(argv[0]); return res; }();
      case  6: // Elems
         if (MNL_LIKELY(argc == 0)) {
            struct traits {
               typedef list<val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return *it; }
            };
            return mnl_iter<traits>{move(self), (long)dat.size(), dat.cbegin()};
         }
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef list<val>::const_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return *it; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<>>(argv[0]).hi - (long)cast<range<>>(argv[0]).lo,
               cseek(dat, cast<range<>>(argv[0]).lo)};
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            struct traits {
               typedef list<val>::const_reverse_iterator iterator;
               MNL_INLINE static const val &fetch(iterator it) noexcept { return *it; }
            };
            return mnl_iter<traits>{move(self), (long)cast<range<true>>(argv[0]).hi - (long)cast<range<true>>(argv[0]).lo,
               traits::iterator{cseek(dat, cast<range<true>>(argv[0]).hi)}};
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case  7: // Keys
         if (MNL_LIKELY(argc == 0)) return range<>{0, (long long)dat.size()};
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return move(argv[0]);
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return move(argv[0]);
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case  8: // ==
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<list<val>>(argv[0]))) return false;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const list<val> &>(argv[0])) return true;)
         for (auto lhs = dat.cbegin(), rhs = cast<const list<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs == cast<const list<val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const list<val> &>(argv[0]).end())) return false;
            if (MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return false;
         }
      case  9: // <>
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<list<val>>(argv[0]))) return true;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const list<val> &>(argv[0])) return false;)
         for (auto lhs = dat.cbegin(), rhs = cast<const list<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs == cast<const list<val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const list<val> &>(argv[0]).end())) return true;
            if (MNL_UNLIKELY( safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return true;
         }
      case 10: // Order
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<list<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const list<val> &>(argv[0])) return 0;)
         for (auto lhs = dat.cbegin(), rhs = cast<const list<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return -(rhs != cast<const list<val> &>(argv[0]).end());
            if (MNL_UNLIKELY(rhs == cast<const list<val> &>(argv[0]).end())) return +1;
            auto res = safe_cast<long long>(op(args<2>{*lhs, *rhs})); if (MNL_UNLIKELY(res)) return res;
         }
      case 11: // Clone
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1) ? move(self) : dat;
      case 12: // DeepClone
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return [this]()->val{ val res = dat; for (auto &&el: cast<list<val> &>(res)) el = MNL_SYM("DeepClone")(move(el)); return res; }();
      }
      return self.default_invoke(op, argc, argv);
   }

} // namespace MNL_AUX_UUID
