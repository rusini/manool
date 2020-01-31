// misc-decimal.cc -- thin wrapper around libdecnumber for IEEE 754 formats

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "mnl-misc-decimal.hh"

# define DECNUMDIGITS 34 // struct decNumber definitions differ in TUs - technically UB but in practice not an issue
# include "libdecnumber/decDouble.h"
# include "libdecnumber/decimal64.h"
# include "libdecnumber/decimal128.h"

namespace MNL_AUX_UUID { using namespace aux;

namespace aux {
   typedef dec<64, true>  df64b; typedef dec<128, true>  df128b;
   typedef dec<64, false> df64c; typedef dec<128, false> df128c;
   static MNL_IF_WITH_MT(thread_local) decContext
      ctx_df64b{16, +384, -383, DEC_ROUND_HALF_EVEN, 0, 0, 1}, ctx_df128b{34, +6144, -6143, DEC_ROUND_HALF_EVEN, 0, 0, 1},
      ctx_df64c{16, +384, -383, DEC_ROUND_HALF_UP,   0, 0, 1}, ctx_df128c{34, +6144, -6143, DEC_ROUND_HALF_UP,   0, 0, 1};
} // namespace aux

# define MNL_M(TYPE, PREC) \
   template<> TYPE TYPE::make(const char rhs[]) noexcept { TYPE res; dec##PREC##FromString(reinterpret_cast<dec##PREC *>(&res), rhs, &ctx_##TYPE); return res; } \
   template<> TYPE TYPE::make(int rhs) noexcept          { TYPE res; dec##PREC##FromInt32 (reinterpret_cast<dec##PREC *>(&res), rhs); return res; } \
   template<> TYPE TYPE::make(unsigned rhs) noexcept     { TYPE res; dec##PREC##FromUInt32(reinterpret_cast<dec##PREC *>(&res), rhs); return res; } \
   template<> char *TYPE::to_str(str_buf res) const noexcept     { return dec##PREC##ToString   (reinterpret_cast<const dec##PREC *>(this), res); } \
   template<> char *TYPE::to_eng_str(str_buf res) const noexcept { return dec##PREC##ToEngString(reinterpret_cast<const dec##PREC *>(this), res); } \
   bool pub::isfinite(TYPE rhs) noexcept { return dec##PREC##IsFinite  (reinterpret_cast<dec##PREC *>(&rhs)); } \
   bool pub::isinf   (TYPE rhs) noexcept { return dec##PREC##IsInfinite(reinterpret_cast<dec##PREC *>(&rhs)); } \
   bool pub::isnan   (TYPE rhs) noexcept { return dec##PREC##IsNaN     (reinterpret_cast<dec##PREC *>(&rhs)); } \
   bool pub::signbit (TYPE rhs) noexcept { return dec##PREC##IsSigned  (reinterpret_cast<dec##PREC *>(&rhs)); } \
   \
   TYPE pub::operator+(TYPE lhs, TYPE rhs) noexcept { TYPE res; dec##PREC##Add(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::operator-(TYPE lhs, TYPE rhs) noexcept { TYPE res; dec##PREC##Subtract(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::operator*(TYPE lhs, TYPE rhs) noexcept { TYPE res; dec##PREC##Multiply(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::operator/(TYPE lhs, TYPE rhs) noexcept { TYPE res; dec##PREC##Divide(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::operator+(TYPE rhs) noexcept { TYPE res; dec##PREC##Plus(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::operator-(TYPE rhs) noexcept { TYPE res; dec##PREC##Minus(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::abs(TYPE rhs) noexcept { TYPE res; dec##PREC##Abs(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
   TYPE pub::fma(TYPE a1, TYPE a2, TYPE a3) noexcept { TYPE res; dec##PREC##FMA(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&a1), reinterpret_cast<dec##PREC *>(&a2), reinterpret_cast<dec##PREC *>(&a3), &ctx_##TYPE); return res; \
   } \
   bool pub::operator==(TYPE lhs, TYPE rhs) noexcept { dec##PREC res; dec##PREC##Compare(&res, \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return dec##PREC##IsZero(&res); \
   } \
   bool pub::operator< (TYPE lhs, TYPE rhs) noexcept { dec##PREC res; dec##PREC##Compare(&res, \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return dec##PREC##IsNegative(&res); \
   } \
   int  pub::order(TYPE lhs, TYPE rhs) noexcept { dec##PREC res; dec##PREC##CompareTotal(&res, \
      reinterpret_cast<dec##PREC *>(&lhs), reinterpret_cast<dec##PREC *>(&rhs)); return dec##PREC##ToInt32(&res, &ctx_##TYPE, DEC_ROUND_DOWN); \
   } \
   TYPE pub::exp(TYPE rhs) noexcept { \
      decNumber num; dec##PREC##ToNumber(&reinterpret_cast<dec##PREC &>(rhs), &num); decNumberExp(&num, &num, &ctx_##TYPE); \
      TYPE res; dec##PREC##FromNumber(&reinterpret_cast<dec##PREC &>(res), &num, &ctx_##TYPE); return res; \
   } \
   TYPE pub::log(TYPE rhs) noexcept { \
      decNumber num; dec##PREC##ToNumber(&reinterpret_cast<dec##PREC &>(rhs), &num); decNumberLn(&num, &num, &ctx_##TYPE); \
      TYPE res; dec##PREC##FromNumber(&reinterpret_cast<dec##PREC &>(res), &num, &ctx_##TYPE); return res; \
   } \
   TYPE pub::log10(TYPE rhs) noexcept { \
      decNumber num; dec##PREC##ToNumber(&reinterpret_cast<dec##PREC &>(rhs), &num); decNumberLog10(&num, &num, &ctx_##TYPE); \
      TYPE res; dec##PREC##FromNumber(&reinterpret_cast<dec##PREC &>(res), &num, &ctx_##TYPE); return res; \
   } \
   TYPE pub::sqrt(TYPE rhs) noexcept { \
      decNumber num; dec##PREC##ToNumber(&reinterpret_cast<dec##PREC &>(rhs), &num); decNumberSquareRoot(&num, &num, &ctx_##TYPE); \
      TYPE res; dec##PREC##FromNumber(&reinterpret_cast<dec##PREC &>(res), &num, &ctx_##TYPE); return res; \
   } \
   TYPE pub::pow(TYPE lhs, TYPE rhs) noexcept { \
      decNumber num, _num; \
      dec##PREC##ToNumber(&reinterpret_cast<dec##PREC &>(lhs), &num); \
      dec##PREC##ToNumber(&reinterpret_cast<dec##PREC &>(rhs), &_num); \
      decNumberPower(&num, &num, &_num, &ctx_##TYPE); \
      TYPE res; dec##PREC##FromNumber(&reinterpret_cast<dec##PREC &>(res), &num, &ctx_##TYPE); return res; \
   } \
   TYPE pub::trunc(TYPE rhs) noexcept { TYPE res; dec##PREC##ToIntegralValue(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE, DEC_ROUND_DOWN); return res; \
   } \
   TYPE pub::round(TYPE rhs) noexcept { TYPE res; dec##PREC##ToIntegralValue(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE, DEC_ROUND_HALF_UP); return res; \
   } \
   TYPE pub::floor(TYPE rhs) noexcept { TYPE res; dec##PREC##ToIntegralValue(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE, DEC_ROUND_FLOOR); return res; \
   } \
   TYPE pub::ceil(TYPE rhs) noexcept { TYPE res; dec##PREC##ToIntegralValue(reinterpret_cast<dec##PREC *>(&res), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE, DEC_ROUND_CEILING); return res; \
   } \
   TYPE pub::quantize(TYPE lhs, TYPE rhs) noexcept { TYPE res; dec##PREC##Quantize( \
      reinterpret_cast<dec##PREC *>(&res), reinterpret_cast<dec##PREC *>(&lhs), \
      reinterpret_cast<dec##PREC *>(&rhs), &ctx_##TYPE); return res; \
   } \
// end # define MNL_M(TYPE, PREC)
   MNL_M(df64b, Double) MNL_M(df128b, Quad)
   MNL_M(df64c, Double) MNL_M(df128c, Quad)
# undef MNL_M

} // namespace MNL_AUX_UUID
