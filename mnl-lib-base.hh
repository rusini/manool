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
   MNL_INLINE inline long long order(val lhs, val rhs) { return safe_cast<long long>(MNL_SYM("Order")(args<2>{move(lhs), move(rhs)})); } // Ordering for dict<val, *>

   // MANOOL Pointers //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   class w_pointer /*weak pointer*/ {
   public:
      val *value;
      MNL_IF_WITH_MT(mutable std::mutex mutex; std::mutex &val_mutex;)
      MNL_INLINE w_pointer(w_pointer &&rhs) noexcept: value(rhs.value) MNL_IF_WITH_MT(,val_mutex(rhs.val_mutex)) {}
      MNL_INLINE w_pointer(val *value MNL_IF_WITH_MT(,std::mutex &val_mutex)) noexcept: value(value) MNL_IF_WITH_MT(,val_mutex(val_mutex)) {}
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int, val [], val *) const;
      friend box<w_pointer>;
   };
   typedef class s_pointer /*strong pointer*/ {
   public:
      val value, weak; val cleanup;
      MNL_IF_WITH_MT(mutable std::mutex mutex;)
      s_pointer() = default;
      MNL_INLINE s_pointer(s_pointer &&rhs) noexcept: value(move(rhs.value)), cleanup(move(rhs.cleanup)) {}
      ~s_pointer();
      MNL_INLINE s_pointer(val &&value) noexcept: value(move(value)) {}
      MNL_INLINE s_pointer(val &&value, val &&cleanup) noexcept: value(move(value)), cleanup(move(cleanup)) {}
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int, val [], val *);
      friend box<s_pointer>;
   } pointer;
}} // namespace aux::pub
   template<> val box<dict<val, val>> :: invoke(val &&, const sym &, int, val [], val *); // Map
   template<> val box<dict<val>>      :: invoke(val &&, const sym &, int, val [], val *); // Set
   template<> val box<std::list<val>> :: invoke(val &&, const sym &, int, val [], val *); // Sequence
   extern template class box<w_pointer>;
   extern template class box<s_pointer>;

} // namespace MNL_AUX_UUID

# endif // # ifndef MNL_INCLUDED_BASE
