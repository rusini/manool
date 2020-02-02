// mnl-misc-decimal.hh -- thin wrapper around libdecnumber for IEEE 754 formats

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# pragma once

# include <type_traits> // aligned_storage
# include <utility>     // rel_ops
# include <string>
# include "mnl-aux-mnl0.hh"

namespace MNL_AUX_UUID {
   namespace aux {
      using std::string;
   }

namespace aux { namespace pub {
   template<int Width, bool Bankers = true> class dec/*imal FP*/ {
      static_assert(Width == 64 || Width == 128, "Width == 64 || Width == 128");
   public:
      dec() = default;
      dec &operator=(const dec &) = default;
      MNL_INLINE dec(const char rhs[]) noexcept: dec(make(rhs)) {}
      MNL_INLINE dec(const string &rhs) noexcept: dec(rhs.c_str()) {}
      MNL_INLINE dec(int rhs) noexcept:      dec(MNL_LIKELY(rhs) ? make(rhs) : dec{}) {}
      MNL_INLINE dec(unsigned rhs) noexcept: dec(MNL_LIKELY(rhs) ? make(rhs) : dec{}) {}
      MNL_INLINE dec &operator=(const char rhs[]) noexcept  { return *this = rhs; }
      MNL_INLINE dec &operator=(const string &rhs) noexcept { return *this = rhs; }
      MNL_INLINE dec &operator=(int rhs) noexcept           { return *this = rhs; }
      MNL_INLINE dec &operator=(unsigned rhs) noexcept      { return *this = rhs; }
      MNL_INLINE explicit dec(dec<Width, !Bankers> rhs) noexcept: _(rhs._) {}
   public:
      typedef char str_buf[Width == 64 ? 25 : Width == 128 ? 43 : 0];
      char *to_str(str_buf) const noexcept, *to_eng_str(str_buf) const noexcept;
      MNL_INLINE string to_str() const     { str_buf buf; return to_str(buf), buf; }
      MNL_INLINE string to_eng_str() const { str_buf buf; return to_eng_str(buf), buf; }
      MNL_INLINE operator string() const   { return to_str(); }
   public:
      MNL_INLINE dec &operator+=(dec rhs) noexcept { return *this = *this + rhs; }
      MNL_INLINE dec &operator-=(dec rhs) noexcept { return *this = *this - rhs; }
      MNL_INLINE dec &operator*=(dec rhs) noexcept { return *this = *this * rhs; }
      MNL_INLINE dec &operator/=(dec rhs) noexcept { return *this = *this / rhs; }
   private:
      typename std::aligned_storage<Width / 8, 8>::type _;
      static dec make(const char []) noexcept, make(int) noexcept, make(unsigned) noexcept;
   };
# define MNL_M(TYPE) \
   template<> MNL_NOCLOBBER TYPE TYPE::make(const char []) noexcept; \
   template<> MNL_PURE      TYPE TYPE::make(int) noexcept; \
   template<> MNL_PURE      TYPE TYPE::make(unsigned) noexcept; \
   template<> char *TYPE::to_str(char []) const noexcept; \
   template<> char *TYPE::to_eng_str(char []) const noexcept; \
   MNL_PURE bool isfinite(TYPE) noexcept, isinf(TYPE) noexcept, isnan(TYPE) noexcept, signbit(TYPE) noexcept; \
   MNL_PURE TYPE operator+(TYPE, TYPE) noexcept, operator-(TYPE, TYPE) noexcept, operator*(TYPE, TYPE) noexcept, operator/(TYPE, TYPE) noexcept; \
   MNL_PURE TYPE operator+(TYPE rhs) noexcept, operator-(TYPE) noexcept, abs(TYPE) noexcept, fma(TYPE, TYPE, TYPE) noexcept; \
   MNL_PURE bool operator==(TYPE, TYPE) noexcept, operator<(TYPE, TYPE) noexcept; \
   MNL_PURE int  order(TYPE, TYPE) noexcept; \
   MNL_INLINE inline bool operator!=(TYPE lhs, TYPE rhs) noexcept { return std::rel_ops::operator!=(lhs, rhs); } \
   MNL_INLINE inline bool operator> (TYPE lhs, TYPE rhs) noexcept { return std::rel_ops::operator> (lhs, rhs); } \
   MNL_INLINE inline bool operator<=(TYPE lhs, TYPE rhs) noexcept { return std::rel_ops::operator<=(lhs, rhs); } \
   MNL_INLINE inline bool operator>=(TYPE lhs, TYPE rhs) noexcept { return std::rel_ops::operator>=(lhs, rhs); } \
   MNL_PURE TYPE exp(TYPE) noexcept, log(TYPE) noexcept, log10(TYPE) noexcept, sqr(TYPE) noexcept, sqrt(TYPE) noexcept, pow(TYPE, TYPE) noexcept; \
   MNL_PURE TYPE trunc(TYPE) noexcept, round(TYPE) noexcept, floor(TYPE) noexcept, ceil(TYPE) noexcept, quantize(TYPE, TYPE) noexcept; \
// end # define MNL_M(TYPE)
# define MNL_T(...) __VA_ARGS__
   MNL_M(MNL_T(dec<64, true>))  MNL_M(MNL_T(dec<128, true>))
   MNL_M(MNL_T(dec<64, false>)) MNL_M(MNL_T(dec<128, false>))
# undef MNL_T
# undef MNL_M
}} // namespace aux::pub

} // namespace MNL_AUX_UUID
