// mnl-lib-base.hh -- C++ API to base facilities

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# ifndef MNL_INCLUDED_BASE
# define MNL_INCLUDED_BASE

# include <list>

# include "manool.hh"
# include "mnl-misc-dict.hh"

namespace MNL_AUX_UUID {

namespace aux { namespace pub {

   // Ordering for dict<val, *> ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_INLINE inline int order(val lhs, val rhs) { return safe_cast<long long>(MNL_SYM("Order")(args<2>{move(lhs), move(rhs)})); }

   // MANOOL Pointers //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   struct weak_pointer {
      val *value;
   # if MNL_WITH_MULTITHREADING
      mutable std::mutex mutex; std::mutex &val_mutex;
   public:
      //MNL_INLINE weak_pointer() = default;
      MNL_INLINE weak_pointer(weak_pointer &&rhs) noexcept: value(rhs.value), val_mutex(rhs.val_mutex) {}
      MNL_INLINE weak_pointer(val *value, std::mutex &val_mutex) noexcept: value(value), val_mutex(val_mutex) {}
   # endif // # if MNL_WITH_MULTITHREADING
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int, val [], val *) const;
      friend box<weak_pointer>;
   };
   struct strong_pointer {
      val value, weak; val cleanup;
      MNL_IF_WITH_MT(mutable std::mutex mutex;)
      strong_pointer() = default;
      MNL_INLINE strong_pointer(strong_pointer &&rhs) noexcept: value(move(rhs.value)), cleanup(move(rhs.cleanup)) {}
      MNL_INLINE strong_pointer(val &&value) noexcept: value(move(value)) {}
      MNL_INLINE strong_pointer(val &&value, val &&cleanup) noexcept: value(move(value)), cleanup(move(cleanup)) {}
   public:
      MNL_INLINE ~strong_pointer() {
         if (MNL_LIKELY(!cleanup)) {
            if (weak) MNL_IF_WITH_MT(std::lock_guard<std::mutex>(cast<weak_pointer &>(weak).mutex),) cast<weak_pointer &>(weak).value = {};
         } else try {
            auto saved_heap_res = heap_reserve(0), saved_stk_res = stk_reserve(0);
            auto saved_sig_state = move(sig_state); sig_state.first = {};
            if (MNL_LIKELY(sig_trace.empty())) {
               if (!weak) weak = weak_pointer{&value MNL_IF_WITH_MT(,mutex)}; // in the rare case that this throws, the stack trace will be empty
               move(cleanup)(weak); // ditto
            } else {
               auto saved_sig_trace = move(sig_trace); sig_trace.clear(), sig_trace.reserve(saved_sig_trace.capacity()); // ditto
               if (!weak) weak = weak_pointer{&value MNL_IF_WITH_MT(,mutex)}; // ditto
               move(cleanup)(weak); // ditto
               sig_trace = move(saved_sig_trace);
            }
            sig_state = move(saved_sig_state);
            heap_reserve(saved_heap_res), stk_reserve(saved_stk_res);
            MNL_IF_WITH_MT(std::lock_guard<std::mutex>(cast<weak_pointer &>(weak).mutex),) cast<weak_pointer &>(weak).value = {};
         } MNL_CATCH_UNEXPECTED
      }
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int, val [], val *);
      friend box<strong_pointer>;
   };

}} // namespace aux::pub

   template<> val box<dict<val, val>> :: invoke(val &&, const sym &, int, val [], val *); // {map}
   template<> val box<dict<val>>      :: invoke(val &&, const sym &, int, val [], val *); // {set}
   template<> val box<std::list<val>> :: invoke(val &&, const sym &, int, val [], val *); // {sequence}
   extern template class box<weak_pointer>;
   extern template class box<strong_pointer>;

} // namespace MNL_AUX_UUID

# endif // # ifndef MNL_INCLUDED_BASE
