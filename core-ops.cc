// core-ops.cc -- core operations

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-aux-core.tcc"

# include <cstdio>   // sprintf
# include <iterator> // begin, end

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::isdigit; // <cstdlib>
      using std::isnan; using std::isfinite; using std::fmod;   using std::fma;   using std::copysign; using std::signbit; // <cmath>
      using std::exp;   using std::expm1;    using std::log;    using std::log1p; using std::log10;    using std::log2; // <cmath>
      using std::sqrt;  using std::hypot;    using std::cbrt;   using std::pow; // <cmath>
      using std::sin;   using std::cos;      using std::tan;    using std::asin;  using std::acos;     using std::atan;  using std::atan2; // <cmath>
      using std::sinh;  using std::cosh;     using std::tanh;   using std::asinh; using std::acosh;    using std::atanh; // <cmath>
      using std::erf;   using std::erfc;     using std::tgamma; using std::trunc; using std::round;    using std::floor; using std::ceil; // <cmath>
      using std::sprintf; // <cstdio>
      using std::begin; using std::end; // <iterator>
   }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_PRIORITY(1001) decltype(sym::dict) sym::dict = []() MNL_INLINE->decltype(dict){
      int count = {};
   # define MNL_S(LAB) {LAB, static_cast<decltype(rep)>(count++)},
      return {
      # include "wks.tcc"
      };
   # undef MNL_S
   }();
   MNL_PRIORITY(1002) std::remove_extent_t<decltype(sym::inverse)> sym::inverse[(unsigned short)-1 + 1] = {
   # define MNL_S(LAB) dict.find(LAB),
      # include "wks.tcc"
   # undef MNL_S
   };
   std::remove_extent_t<decltype(sym::rc)> sym::rc[(unsigned short)-1 + 1] = {
   # define MNL_S(LAB) 1,
      # include "wks.tcc"
   # undef MNL_S
   };

// Primitive Operations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux {
   constexpr auto err_Overflow  = []() MNL_INLINE{ MNL_ERR(MNL_SYM("Overflow"));  }; // to allow optimizer to blend jump targets
   constexpr auto err_Undefined = []() MNL_INLINE{ MNL_ERR(MNL_SYM("Undefined")); }; // ditto

   // I48 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long> _div(Dat lhs, Dat rhs) {
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(lhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"));
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (int)lhs / (int)rhs :
      # endif
      # if __i386__ && __SIZEOF_LONG_DOUBLE__ == 12 // according to testing (on certain low-end x86-32 CPUs),
         (long long)((long double)lhs / rhs); // x87 division is slightly faster than synthesized 64-bit integer division (and easier to manage than SSE2 division)
      # else
         lhs / rhs;
      # endif
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long> _rem(Dat lhs, Dat rhs) {
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(MNL_SYM("Undefined"));
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (int)lhs % (int)rhs :
      # endif
         lhs % rhs;
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long> fdiv(Dat lhs, Dat rhs) { // flooring division
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(lhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"));
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (MNL_UNLIKELY((int)lhs < 0 ^ (int)rhs < 0) && MNL_LIKELY((int)lhs % (int)rhs) ? (int)lhs / (int)rhs - 1 : (int)lhs / (int)rhs) :
      # endif
         (MNL_UNLIKELY(lhs < 0 ^ rhs < 0) && MNL_LIKELY(lhs % rhs) ? lhs / rhs - 1 : lhs / rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long> _mod(Dat lhs, Dat rhs) {
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(MNL_SYM("Undefined"));
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (MNL_UNLIKELY((int)lhs < 0 ^ (int)rhs < 0) ? MNL_LIKELY((int)lhs % (int)rhs) ? (int)lhs % (int)rhs + (int)rhs : 0 : (int)lhs % (int)rhs) :
      # endif
         (MNL_UNLIKELY(lhs < 0 ^ rhs < 0) ? MNL_LIKELY(lhs % rhs) ? lhs % rhs + rhs : 0 : lhs % rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long> _neg(Dat rhs) {
      return -rhs;
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long> _abs(Dat rhs) {
      return abs(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long, string> _str(Dat rhs) {
      char res[sizeof "+140737488355327"];
      return sprintf(res, "%lld", rhs), res;
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, long long, string> _str(Dat rhs, const string &format) {
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '#': case '+': case '-': case '0': { ++pc; continue; } break; }
      if (isdigit(*pc) && isdigit(*++pc)) ++pc;
      if (*pc == '.' && isdigit(*++pc) && isdigit(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("SyntaxError")); case 'd': case 'i': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("SyntaxError"));
      char res[512];
      return sprintf(res, ("%" + string(format.begin(), format.end() - 1) + "lld").c_str(), rhs), res;
   }

   // F64, F32 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // FP classification by preference (number of instructions for x86+sse2): isnan, isinf, isfinite
   // However, tests for inf go before tests for nan since this is how complex numbers are dealt with
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _div(Dat lhs, Dat rhs) {
      auto res = lhs / rhs;
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? rhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), res, rhs);
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _rem(Dat lhs, Dat rhs) {
      using std::fmod; auto res = fmod(lhs, rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      (err_Undefined)();
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _neg(Dat arg)
      { return -arg; }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _fma(Dat arg0, Dat arg1, Dat arg2) {
      using std::fma; auto res = fma(arg0, arg1, arg2);
      if (MNL_LIKELY(!isinf(res))) return res;
      (err_Overflow)();
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _abs(Dat arg)
      { using std::abs; return abs(arg); }
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sign(Dat arg)
      { using std::copysign; return arg == 0 ? arg : copysign((Dat)1, arg); } // consistent in some sense with POSIX's copysign
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sign(Dat arg0, Dat arg1)
      { using std::copysign; return copysign(arg0, arg1); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _exp(Dat arg) {
      using std::exp; auto res = exp(arg);
      if (MNL_LIKELY(!isinf(res))) return res;
      (err_Overflow)();
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _expm1(Dat arg) {
      using std::expm1; auto res = expm1(arg);
      if (MNL_LIKELY(!isinf(res))) return res;
      (err_Overflow)();
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log(Dat arg) {
      using std::log; auto res = log(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // as per IEEE 754 log never results in overflow
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log(Dat base, Dat arg) {
      using std::log2;
      if (MNL_UNLIKELY(base == 0)) (err_Undefined)();
      auto res = log2(arg) / log2(base);
      if (MNL_LIKELY(isfinite(res))) return res; // Undefined signaled for 0/1 bases (not DivisionByZero)
      MNL_ERR(!isnan(res) & base != 1 ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res, base);
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log1p(Dat arg) {
      using std::log1p; auto res = log1p(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res);
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log10(Dat arg) {
      using std::log10; auto res = log10(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // as per IEEE 754 log never results in overflow
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log2(Dat arg) {
      using std::log2; auto res = log2(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // as per IEEE 754 log never results in overflow
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sqr(Dat arg)
      { return (_mul)(arg, arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sqrt(Dat arg) {
      using std::sqrt; auto res = sqrt(arg);
      if (MNL_LIKELY(!isnan(res))) return res;
      (err_Undefined)();
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _hypot(Dat arg0, Dat arg1) {
      using std::hypot; auto res = hypot(lhs, rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      (err_Overflow)();
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _cbrt(Dat rhs)
      { using cbrt; return cbrt(rhs); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _pow(Dat arg0, Dat arg1) {
      using pow; auto res = pow(lhs, rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? lhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _sin(Dat rhs) {
      return sin(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _cos(Dat rhs) {
      return cos(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _tan(Dat rhs) {
      return tan(rhs); // as per IEEE 754 tan never results in overflow (neither in division by zero)
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _asin(Dat rhs) {
      auto res = asin(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _acos(Dat rhs) {
      auto res = acos(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _atan(Dat rhs) {
      return atan(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _atan(Dat lhs, Dat rhs) {
      return atan2(lhs, rhs); // as per IEEE 754, IEEE Std 1003.1 (POSIX), and MSDN atan2(0., 0.) == 0.
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _sinh(Dat rhs) {
      auto res = sinh(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _cosh(Dat rhs) {
      auto res = cosh(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _tanh(Dat rhs) {
      return tanh(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _asinh(Dat rhs) {
      return asinh(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _acosh(Dat rhs) {
      auto res = acosh(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _atanh(Dat rhs) {
      auto res = atanh(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"));
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _erf(Dat rhs) {
      return erf(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _erfc(Dat rhs) {
      return erfc(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _gamma(Dat rhs) {
      auto res = tgamma(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(rhs <= 0 && trunc(rhs) == rhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
   }
# define MNL_M(DAT, SUFFIX) \
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, DAT> _lgamma(Dat rhs) { \
      int _; auto res = ::lgamma##SUFFIX##_r(rhs, &_); \
      if (MNL_LIKELY(!isinf(res))) return res; \
      MNL_ERR(rhs <= 0 && trunc(rhs) == rhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow")); \
   } \
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, DAT> _jn(long long n, Dat rhs) { \
      if (MNL_UNLIKELY(n < lim<int>::min()) || MNL_UNLIKELY(n > lim<int>::max())) MNL_ERR(MNL_SYM("LimitExceeded")); \
      return (int)n == 0 ? ::j0##SUFFIX(rhs) : (int)n == 1 ? ::j1##SUFFIX(rhs) : ::jn##SUFFIX(n, rhs); /*dynamic specialization*/ \
   } \
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, DAT> _yn(long long n, Dat rhs) { \
      if (MNL_UNLIKELY(n < lim<int>::min()) || MNL_UNLIKELY(n > lim<int>::max())) MNL_ERR(MNL_SYM("LimitExceeded")); \
      auto res = (int)n == 0 ? ::y0##SUFFIX(rhs) : (int)n == 1 ? ::y1##SUFFIX(rhs) : ::yn##SUFFIX(n, rhs); /*dynamic specialization*/ \
      if (MNL_LIKELY(isfinite(res))) return res; \
      MNL_ERR(!isnan(res) ? rhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined")); \
   } \
// end # define MNL_M(DAT, SUFFIX)
   MNL_M(double, /*empty*/) MNL_M(float, f)
# undef MNL_M
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _trunc(Dat rhs) {
      return trunc(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _round(Dat rhs) {
      return round(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _floor(Dat rhs) {
      return floor(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat> _ceil(Dat rhs) {
      return ceil(rhs);
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, double, string> _str(Dat rhs) {
      char res[sizeof "+1.7976931348623157e+308"];
      return sprintf(res, "%.16e", rhs), res;
   }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, float, string> _str(Dat rhs) {
      char res[sizeof "+3.40282347e+38"];
      return sprintf(res, "%.8e", rhs), res;
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat, string> _str(Dat rhs, const string &format) {
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '#': case '+': case '-': case '0': { ++pc; continue; } break; }
      if (isdigit(*pc) && isdigit(*++pc)) ++pc;
      if (*pc == '.' && isdigit(*++pc) && isdigit(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("SyntaxError")); case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("SyntaxError"));
      char res[512];
      return sprintf(res, ("%" + format).c_str(), rhs), res;
   }
   template<typename Dat> MNL_INLINE static inline enable_core_binfloat<Dat, long long> _int(Dat rhs) {
      if (MNL_LIKELY((double)rhs >= min_i48) && MNL_LIKELY((double)rhs <= max_i48)) return (double)rhs;
      MNL_ERR(MNL_SYM("Overflow"));
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, unsigned> _neg(Dat rhs) { return -rhs; }
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, unsigned> _abs(Dat rhs) { return +rhs; }
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE static inline enable_same<Dat, unsigned, string> _str(Dat rhs, const string &format) {
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '#': case '+': case '-': case '0': { ++pc; continue; } break; }
      if (isdigit(*pc) && isdigit(*++pc)) ++pc;
      if (*pc == '.' && isdigit(*++pc) && isdigit(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("SyntaxError")); case 'u': case 'o': case 'x': case 'X': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("SyntaxError"));
      char res[512];
      return sprintf(res, ("%" + format).c_str(), rhs), res;
   }
} // namespace aux

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template val val::_invoke(const val &, const sym &, int, val [], val *);
   template val val::_invoke(val &&, const sym &, int, val [], val *);

   template<typename Self> MNL_NOINLINE MNL_HOT val val::_invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out) {
      switch (self.rep.tag()) {
         static constexpr auto err_InvalidInvocation = []() MNL_INLINE{ MNL_ERR(MNL_SYM("InvalidInvocation")); };
         static constexpr auto err_TypeMismatch      = []() MNL_INLINE{ MNL_ERR(MNL_SYM("TypeMismatch"));      };
      case 0xFFF8 + 0b111: /////////////////////////////////////////////////////////////////////////////////////////////// BoxPtr (fallback)
         return static_cast<root *>(self.rep.template dat<void *>())->_invoke(std::forward<Self>(self), op, argc, argv, argv_out);
      case 0xFFF8 + 0b000: ///////////////////////////////////////////////////////////////////////////////////////////////////////////// Nil
         switch (op) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  is<>(argv[0]);
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !is<>(argv[0]);
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!test<>(argv[0]))) return self.default_order(argv[0]); // TODO: do we need the condition?
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return {};
         case sym::id("Str"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return MNL_AUX_EARLY((val)"Nil");
         case sym::id("Weak"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return {};
         }
         return [&op, argc]() MNL_NOINLINE->val{
            switch (MNL_AUX_EARLY(disp{"^", "Set"})[op]) {
            case 1: // ^
               if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               MNL_ERR(MNL_SYM("IndirectionThruNil"));
            case 2: // Set
               if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               MNL_ERR(MNL_SYM("IndirectionThruNil"));
            default:
               MNL_UNREACHABLE();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      case 0xFFF8 + 0b001: ///////////////////////////////////////////////////////////////////////////////////////////////////////////// I48
         switch (op) {
         case sym::id("+"):   // addition
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_add)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("-"):   // subtraction and negation (unary minus)
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return (_neg)(as<long long>(self));
            }
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_sub)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("*"):   // multiplication
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_mul)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("/"):   // integer division (truncating, i.e. rounding toward zero)
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_div)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Rem"): // remainder after "/"
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_rem)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Div"): // division with flooring (rounding toward -infinity)
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (div2)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Mod"): // remainder after "Div" (modulo)
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_mod)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return std::memcmp(&self, &argv[0], sizeof self) == 0;
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return std::memcmp(&self, &argv[0], sizeof self) != 0;
         case sym::id("<"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return as<long long>(self) <  as<long long>(argv[0]);
         case sym::id("<="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return as<long long>(self) <= as<long long>(argv[0]);
         case sym::id(">"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return as<long long>(self) >  as<long long>(argv[0]);
         case sym::id(">="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return as<long long>(self) >= as<long long>(argv[0]);
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) return self.default_order(argv[0]);
            return (_order)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Abs"):  // magnitude (absolute value)
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_abs)(as<long long>(self));
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return self; // prefer over: as<long long>(self)
         }
         return [&self, &op, argc, argv]() MNL_NOINLINE->val{
            switch (MNL_AUX_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_LIKELY(argc == 0)) return (_str)(as<long long>(self));
               if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               return (_str)(as<long long>(self), as<const std::string &>(argv[0]));
            default:
               MNL_UNREACHABLE();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// F64/F32
         {  static constexpr auto dispatch = [](auto self, auto &op, auto &argc, auto &argv) MNL_INLINE->val{
               switch (op) {
               case sym::id("+"):   // addition
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_add(self, as<decltype(self)>(argv[0]));
               case sym::id("-"):   // subtraction and negation (unary minus)
                  if (MNL_UNLIKELY(argc != 1)) {
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_neg(self);
                  }
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_sub(self, as<decltype(self)>(argv[0]));
               case sym::id("*"):   // multiplication
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_mul(self, as<decltype(self)>(argv[0]));
               case sym::id("/"):   // division
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_div(self, as<decltype(self)>(argv[0]));
               case sym::id("=="):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  return  MNL_LIKELY(is<decltype(self)>(argv[0])) && self == as<decltype(self)>(argv[0]);
               case sym::id("<>"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  return !MNL_LIKELY(is<decltype(self)>(argv[0])) || self != as<decltype(self)>(argv[0]);
               case sym::id("<"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return self <  as<decltype(self)>(argv[0]);
               case sym::id("<="):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return self <= as<decltype(self)>(argv[0]);
               case sym::id(">"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return self >  as<decltype(self)>(argv[0]);
               case sym::id(">="):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return self >= as<decltype(self)>(argv[0]);
               case sym::id("Order"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) return self.default_order(argv[0]);
                  return aux::_order(self, as<decltype(self)>(argv[0]));
               case sym::id("Abs"):  // magnitude (absolute value) -- Abs[(-T[0])] == T[0]
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_abs(self);
               case sym::id("Sign"): // signum and POSIX's "copysign"
                  if (MNL_UNLIKELY(argc == 0)) return aux::_sign(self); // nearly neutral priority
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_sign(self, as<decltype(self)>(argv[0]));
               case sym::id("Sqr"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_mul(self, self);
               case sym::id("Sqrt"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_sqrt(self);
               case sym::id("Trunc"):// truncate
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_trunc(self);
               case sym::id("Round"):// round
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_round(self);
               case sym::id("Floor"):// floor
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_floor(self);
               case sym::id("Ceil"): // ceiling
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_ceil(self);
               case sym::id("Clone"): case sym::id("DeepClone"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return self;
               case sym::id("Int"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_int(self);
               }
               return [self, &op, argc, argv]() MNL_NOINLINE->val{
                  switch (MNL_EARLY(disp{"Rem", "FMA", "Exp", "Expm1", "Log", "Log1p", "Log10", "Log2", "Hypot", "Cbrt", "Pow", "Sin", "Cos", "Tan", "Asin",
                     "Acos", "Atan", "Sinh", "Cosh", "Tanh", "Asinh", "Acosh", "Atanh", "Erf", "Erfc", "Gamma", "LogGamma", "BesJn", "BesYn", "Str"})[op]) {
                  case  1: // Rem      -- POSIX/C99 (*not* IEEE754's "remainder")
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_rem(self, as<decltype(self)>(argv[0]));
                  case  2: // FMA      -- POSIX/C99/IEEE754
                     // TODO: FMA may be used to evaluate polynomials, not only for "scalar product", hence it is not clear which argument order is more convenient!
                     if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0])) || MNL_UNLIKELY(!is<decltype(self)>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_fma(as<decltype(self)>(argv[0]), as<decltype(self)>(argv[1]), self);
                  case  3: // Exp      -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_exp(self);
                  case  4: // Expm1    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_expm1(self);
                  case  5: // Log      -- POSIX/C99/IEEE754
                     if (MNL_LIKELY(argc == 0)) return aux::_log(self);
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_log(self, as<decltype(self)>(argv[0]));
                  case  6: // Log1p    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_log1p(self);
                  case  7: // Log10    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_log10(self);
                  case  8: // Log2     -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_log2(self);
                  case  9: // Hypot    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_hypot(self, as<decltype(self)>(argv[0]));
                  case 10: // Cbrt     -- POSIX/C99/IEEE754 (partially, as "rootn")
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::cbrt(self);
                  case 11: // Pow      -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_pow(self, as<decltype(self)>(argv[0]));
                  case 12: // Sin      -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_sin(self);
                  case 13: // Cos      -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_cos(self);
                  case 14: // Tan      -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_tan(self);
                  case 15: // Asin     -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_asin(self);
                  case 16: // Acos     -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_acos(self);
                  case 17: // Atan     -- POSIX/C99/IEEE754
                     if (MNL_LIKELY(argc == 1)) {
                        if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                        return aux::_atan(self, as<decltype(self)>(argv[0])); // atan(lhs / rhs) with extensions (IEEE 754)
                     }
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_atan(self);
                  case 18: // Sinh     -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_sinh(self);
                  case 19: // Cosh     -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_cosh(self);
                  case 20: // Tanh     -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_tanh(self);
                  case 21: // Asinh    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_asinh(self);
                  case 22: // Acosh    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_acosh(self);
                  case 23: // Atanh    -- POSIX/C99/IEEE754
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_atanh(self);
                  case 24: // Erf      -- POSIX/C99
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_erf(self);
                  case 25: // Erfc     -- POSIX/C99
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_erfc(self);
                  case 26: // Gamma    -- POSIX/C99
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_gamma(self);
                  case 27: // LogGamma -- POSIX/C99
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return aux::_lgamma(self);
                  case 28: // BesJn ("Bessel function of the first kind of integer order")  -- POSIX
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_jn(as<long long>(argv[0]), self);
                  case 29: // BesYn ("Bessel function of the second kind of integer order") -- POSIX
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_yn(as<long long>(argv[0]), self);
                  case 30: // Str
                     if (MNL_LIKELY(argc == 0)) return aux::_str(self);
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<std::string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return aux::_str(self, as<const std::string &>(argv[0]));
                  default:
                     MNL_UNREACHABLE();
                  case int{}:;
                  }
                  MNL_ERR(MNL_SYM("UnrecognizedOperation"));
               }();
            };
         default:
            dispatch(as<double>(self), op, argc, argv);
         case rep::_f32:
            dispatch(as<float> (self), op, argc, argv);
         }
      case 0xFFF8 + 0b110: ///////////////////////////////////////////////////////////////////////////////////////////////////////////// Sym
         switch (op) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  MNL_LIKELY(is<const sym &>(argv[0])) && as<const sym &>(self) == as<const sym &>(argv[0]);
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !MNL_LIKELY(is<const sym &>(argv[0])) || as<const sym &>(self) != as<const sym &>(argv[0]);
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<sym>(argv[0]))) return self.default_order(argv[0]);
            return (as<sym>(self) > as<sym>(argv[0])) - (as<sym>(self) < as<sym>(argv[0]));
         case sym::id("Apply"):
            return as<const sym &>(self)(*argv, argc - 1, argv + 1, argv_out + !!argv_out); // TODO: also a convenience/unification func exists for that
         case sym::id("Clone"): case sym::id("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return std::move(self); // TODO: consider moving out sym and (re-)constructing val
         }
         return [&self, &op, argc]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               return (std::string)as<const sym &>(self);
            default:
               MNL_UNREACHABLE();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      case 0xFFF8 + 0x100: ////////////////////////////////////////////////////////////////////////////////////////////////////// Bool/False
         switch (op) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() == rep::_false; // TODO: we can compare whole words here (which always gives benefit on 64-bit platforms)
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() != rep::_false;
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) return self.default_order(argv[0]);
            return +-as<bool>(argv[0]);
         case sym::id("|"):
         case sym::id("Xor"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) err_TypeMismatch();
            return val{decltype(rep){argv[0].rep.tag()}};
         case sym::id("&"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) err_TypeMismatch();
            return false;
         case sym::id("~"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return true;
         case sym::id("Clone"): case sym::id("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return false;
         case sym::id("Str"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return MNL_EARLY((val)"False");
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0xFFF8 + 0x101: /////////////////////////////////////////////////////////////////////////////////////////////////////// Bool/True
         switch (op) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() == rep::_true;
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() != rep::_true;
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) return self.default_order(argv[0]);
            return +!as<bool>(argv[0]);
         case sym::id("&"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) err_TypeMismatch();
            return val{decltype(rep){argv[0].rep.tag()}};
         case sym::id("|"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) err_TypeMismatch();
            return true;
         case sym::id("Xor"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) err_TypeMismatch();
            return val{decltype(rep){argv[0].rep.tag() ^ 1}};
         case sym::id("~"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return false;
         case sym::id("Clone"): case sym::id("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return true;
         case sym::id("Str"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return MNL_EARLY((val)"True");
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0xFFF8 + 0x011: ///////////////////////////////////////////////////////////////////////////////////////////////////////////// U32
         switch (op) {
         case sym::id("+"):   // addition
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) + as<unsigned>(argv[0]);
         case sym::id("-"):   // subtraction and negation (unary minus)
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return -as<unsigned>(self);
            }
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) - as<unsigned>(argv[0]);
         case sym::id("*"):   // multiplication
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) * as<unsigned>(argv[0]);
         case sym::id("/"): case sym::id("Div"):   // integer division (truncating)
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) / as<unsigned>(argv[0]);
         case sym::id("Rem"): case sym::id("Mod"): // remainder after division
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) % as<unsigned>(argv[0]);
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  MNL_LIKELY(is<unsigned>(argv[0])) && as<unsigned>(self) == as<unsigned>(argv[0]);
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !MNL_LIKELY(is<unsigned>(argv[0])) || as<unsigned>(self) != as<unsigned>(argv[0]);
         case sym::id("<"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) <  as<unsigned>(argv[0]);
         case sym::id("<="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) <= as<unsigned>(argv[0]);
         case sym::id(">"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) >  as<unsigned>(argv[0]);
         case sym::id(">="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) >= as<unsigned>(argv[0]);
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) return self.default_order(argv[0]);
            return aux::_order(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("Abs"):  // identity (absolute value)
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return +as<unsigned>(self);
         case sym::id("&"):    // bitwise conjunction (AND)
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) & as<unsigned>(argv[0]);
         case sym::id("|"):    // bitwise disjunction (OR)
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) | as<unsigned>(argv[0]);
         case sym::id("~"):    // bitwise complement (NOT)
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return ~as<unsigned>(self);
         case sym::id("Xor"):  // bitwise eXclusive-OR
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) ^ as<unsigned>(argv[0]);
         case sym::id("SHL"):  // bitwise (logical) shift left
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _shl(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("SHR"):  // bitwise (logical) shift right
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _shr(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ASR"):  // bitwise arithmetic shift right
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _asr(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ROL"):  // bitwise rotate left
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _rol(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ROR"):  // bitwise rotate right
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _ror(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("CLZ"):  // "count leading zeros"
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return _clz(as<unsigned>(self));
         case sym::id("CTZ"):  // "count trailing zeros"
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return _ctz(as<unsigned>(self));
         case sym::id("Log2"): // Log2[rhs] == ~0 for rhs == 0 (on purpose)
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return 31 - _clz(as<unsigned>(self)); // definition
         case sym::id("BitSum"): // so-called popcount ("population count")
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (unsigned)__builtin_popcount(as<unsigned>(self));
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return self;
         case sym::id("Int"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (long long)as<unsigned>(self);
         }
         return [&self, &op, argc, argv]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_LIKELY(argc == 0)) return aux::_str(as<unsigned>(self));
               if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               return aux::_str(as<unsigned>(self), as<const std::string &>(argv[0]));
            default:
               MNL_UNREACHABLE();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      }
   }





   val sym::operator()(int argc, val argv[], val *argv_out) const {
      if (MNL_UNLIKELY(!argc)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      switch (argv[0].rep.tag()) {
      case 0x7FF8u: // BoxPtr (fallback)
         return static_cast<val::root *>(argv[0].rep.dat<void *>())->invoke(move(argv[0]), *this, argc - 1, argv + 1, argv_out + !!argv_out);
      case 0x7FF9u: // Nil
         switch (*this) {
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return  test<>(argv[1]);
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return !test<>(argv[1]);
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<>(argv[1]))) return argv[0].default_order(argv[1]);
            return 0;
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return {};
         case sym::op_str:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return "Nil";
         case sym::op_caret:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            MNL_ERR(MNL_SYM("IndirectionByNil"));
         case sym::op_set:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            MNL_ERR(MNL_SYM("IndirectionByNil"));
         case sym::op_weak:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return {};
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFAu: // I48
         switch (*this) {
         case sym::op_add:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_add(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_sub:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_sub(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_mul:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_mul(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_div:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_div(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_rem:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_rem(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_divf:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::fdiv(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_mod:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_mod(cast<long long>(argv[0]), cast<long long>(argv[1]));
         case sym::op_neg: case sym::op_not:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return aux::_neg(cast<long long>(argv[0]));
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return  MNL_LIKELY(test<long long>(argv[1])) && cast<long long>(argv[0]) == cast<long long>(argv[1]);
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return !MNL_LIKELY(test<long long>(argv[1])) || cast<long long>(argv[0]) != cast<long long>(argv[1]);
         case sym::op_lt:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<long long>(argv[0]) <  cast<long long>(argv[1]);
         case sym::op_le:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<long long>(argv[0]) <= cast<long long>(argv[1]);
         case sym::op_gt:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<long long>(argv[0]) >  cast<long long>(argv[1]);
         case sym::op_ge:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<long long>(argv[0]) >= cast<long long>(argv[1]);
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) return argv[0].default_order(argv[1]);
            return (cast<long long>(argv[0]) > cast<long long>(argv[1])) - (cast<long long>(argv[0]) < cast<long long>(argv[1]));
         case sym::op_abs:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return aux::_abs(cast<long long>(argv[0]));
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return cast<long long>(argv[0]);
         case sym::op_str:
            if (MNL_LIKELY(argc == 1)) return aux::_str(cast<long long>(argv[0]));
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<string>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_str(cast<long long>(argv[0]), cast<const string &>(argv[1]));
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      # define MNL_M(DAT) \
         switch (*this) { \
         case sym::op_add: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_add(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_sub: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_sub(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_mul: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_mul(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_div: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_div(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_rem: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_rem(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_neg: case sym::op_not: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_neg(cast<DAT>(argv[0])); \
         case sym::op_fma: \
            if (MNL_UNLIKELY(argc != 3)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1])) || MNL_UNLIKELY(!test<DAT>(argv[2]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_fma(cast<DAT>(argv[1]), cast<DAT>(argv[2]), cast<DAT>(argv[0])); \
         case sym::op_eq: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return  MNL_LIKELY(test<DAT>(argv[1])) && cast<DAT>(argv[0]) == cast<DAT>(argv[1]); \
         case sym::op_ne: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return !MNL_LIKELY(test<DAT>(argv[1])) || cast<DAT>(argv[0]) != cast<DAT>(argv[1]); \
         case sym::op_lt: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return cast<DAT>(argv[0]) <  cast<DAT>(argv[1]); \
         case sym::op_le: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return cast<DAT>(argv[0]) <= cast<DAT>(argv[1]); \
         case sym::op_gt: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return cast<DAT>(argv[0]) >  cast<DAT>(argv[1]); \
         case sym::op_ge: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return cast<DAT>(argv[0]) >= cast<DAT>(argv[1]); \
         case sym::op_order: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) return argv[0].default_order(argv[1]); \
            return signbit(cast<DAT>(argv[0])) ^ signbit(cast<DAT>(argv[1])) ? signbit(cast<DAT>(argv[1])) - signbit(cast<DAT>(argv[0])) : \
               cast<DAT>(argv[0]) < cast<DAT>(argv[1]) ? -1 : cast<DAT>(argv[0]) != cast<DAT>(argv[1]); \
         case sym::op_abs: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_abs(cast<DAT>(argv[0])); \
         case sym::op_sign: \
            if (MNL_LIKELY(argc == 1)) return aux::_sign(cast<DAT>(argv[0])); \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_sign(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_exp: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_exp(cast<DAT>(argv[0])); \
         case sym::op_expm1: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_expm1(cast<DAT>(argv[0])); \
         case sym::op_log: \
            if (MNL_LIKELY(argc == 1)) return aux::_log(cast<DAT>(argv[0])); \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_log(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_log1p: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_log1p(cast<DAT>(argv[0])); \
         case sym::op_log10: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_log10(cast<DAT>(argv[0])); \
         case sym::op_log2: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_log2(cast<DAT>(argv[0])); \
         case sym::op_sqr: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_sqr(cast<DAT>(argv[0])); \
         case sym::op_sqrt: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_sqrt(cast<DAT>(argv[0])); \
         case sym::op_hypot: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_hypot(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_cbrt: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_cbrt(cast<DAT>(argv[0])); \
         case sym::op_pow: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_pow(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_sin: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_sin(cast<DAT>(argv[0])); \
         case sym::op_cos: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_cos(cast<DAT>(argv[0])); \
         case sym::op_tan: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_tan(cast<DAT>(argv[0])); \
         case sym::op_asin: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_asin(cast<DAT>(argv[0])); \
         case sym::op_acos: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_acos(cast<DAT>(argv[0])); \
         case sym::op_atan: \
            if (MNL_LIKELY(argc == 1)) return aux::_atan(cast<DAT>(argv[0])); \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_atan(cast<DAT>(argv[0]), cast<DAT>(argv[1])); \
         case sym::op_sinh: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_sinh(cast<DAT>(argv[0])); \
         case sym::op_cosh: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_cosh(cast<DAT>(argv[0])); \
         case sym::op_tanh: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_tanh(cast<DAT>(argv[0])); \
         case sym::op_asinh: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_asinh(cast<DAT>(argv[0])); \
         case sym::op_acosh: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_acosh(cast<DAT>(argv[0])); \
         case sym::op_atanh: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_atanh(cast<DAT>(argv[0])); \
         case sym::op_erf: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_erf(cast<DAT>(argv[0])); \
         case sym::op_erfc: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_erfc(cast<DAT>(argv[0])); \
         case sym::op_gamma: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_gamma(cast<DAT>(argv[0])); \
         case sym::op_lgamma: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_lgamma(cast<DAT>(argv[0])); \
         case sym::op_jn: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_jn(cast<long long>(argv[1]), cast<DAT>(argv[0])); \
         case sym::op_yn: \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_yn(cast<long long>(argv[1]), cast<DAT>(argv[0])); \
         case sym::op_trunc: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_trunc(cast<DAT>(argv[0])); \
         case sym::op_round: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_round(cast<DAT>(argv[0])); \
         case sym::op_floor: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_floor(cast<DAT>(argv[0])); \
         case sym::op_ceil: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_ceil(cast<DAT>(argv[0])); \
         case sym::op_clone: case sym::op_deep_clone: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return cast<DAT>(argv[0]); \
         case sym::op_str: \
            if (MNL_LIKELY(argc == 1)) return aux::_str(cast<DAT>(argv[0])); \
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<string>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            return aux::_str(cast<DAT>(argv[0]), cast<const string &>(argv[1])); \
         case sym::op_int: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return aux::_int(cast<DAT>(argv[0])); \
         } \
         MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
      // end # define MNL_M(DAT)
      default: /* F64 */ MNL_M(double) case 0x7FFCu: /* F32 */ MNL_M(float)
      # undef MNL_M
      case 0x7FFBu: // Sym
         switch (*this) {
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return  MNL_LIKELY(test<sym>(argv[1])) && cast<const sym &>(argv[0]) == cast<const sym &>(argv[1]);
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return !MNL_LIKELY(test<sym>(argv[1])) || cast<const sym &>(argv[0]) != cast<const sym &>(argv[1]);
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<sym>(argv[1]))) return argv[0].default_order(argv[1]);
            return (cast<sym>(argv[0]) > cast<sym>(argv[1])) - (cast<sym>(argv[0]) < cast<sym>(argv[1]));
         case sym::op_apply:
            return cast<const sym &>(argv[0])(argc - 1, argv + 1, argv_out + !!argv_out);
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return move(argv[0]);
         case sym::op_str:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return (string)cast<const sym &>(argv[0]);
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFEu: // Bool/False
         switch (*this) {
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[1].rep.tag() == 0x7FFEu;
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[1].rep.tag() != 0x7FFEu;
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) return argv[0].default_order(argv[1]);
            return +-cast<bool>(argv[1]);
         case sym::op_or:
         case sym::op_xor:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return val{decltype(val::rep){argv[1].rep.tag()}};
         case sym::op_and:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return false;
         case sym::op_not:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return true;
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return false;
         case sym::op_str:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return "False";
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFFu: // Bool/True
         switch (*this) {
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[1].rep.tag() == 0x7FFFu;
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[1].rep.tag() != 0x7FFFu;
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) return argv[0].default_order(argv[1]);
            return +!cast<bool>(argv[1]);
         case sym::op_and:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return val{decltype(val::rep){argv[1].rep.tag()}};
         case sym::op_or:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return true;
         case sym::op_xor:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<bool>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return val{decltype(val::rep){argv[1].rep.tag() ^ 1}};
         case sym::op_not:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return false;
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return true;
         case sym::op_str:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return "True";
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFDu: // U32
         switch (*this) {
         case sym::op_add:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) + cast<unsigned>(argv[1]);
         case sym::op_sub:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) - cast<unsigned>(argv[1]);
         case sym::op_mul:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) * cast<unsigned>(argv[1]);
         case sym::op_div: case sym::op_divf:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!cast<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return cast<unsigned>(argv[0]) / cast<unsigned>(argv[1]);
         case sym::op_rem: case sym::op_mod:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!cast<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return cast<unsigned>(argv[0]) % cast<unsigned>(argv[1]);
         case sym::op_neg:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return -cast<unsigned>(argv[0]);
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return  MNL_LIKELY(test<unsigned>(argv[1])) && cast<unsigned>(argv[0]) == cast<unsigned>(argv[1]);
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return !MNL_LIKELY(test<unsigned>(argv[1])) || cast<unsigned>(argv[0]) != cast<unsigned>(argv[1]);
         case sym::op_lt:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) <  cast<unsigned>(argv[1]);
         case sym::op_le:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) <= cast<unsigned>(argv[1]);
         case sym::op_gt:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) >  cast<unsigned>(argv[1]);
         case sym::op_ge:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) >= cast<unsigned>(argv[1]);
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) return argv[0].default_order(argv[1]);
            return (cast<unsigned>(argv[0]) > cast<unsigned>(argv[1])) - (cast<unsigned>(argv[0]) < cast<unsigned>(argv[1]));
         case sym::op_abs:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return +cast<unsigned>(argv[0]);
         case sym::op_and:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) & cast<unsigned>(argv[1]);
         case sym::op_or:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) | cast<unsigned>(argv[1]);
         case sym::op_not:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return ~cast<unsigned>(argv[0]);
         case sym::op_xor:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) ^ cast<unsigned>(argv[1]);
         case sym::op_shl:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return MNL_LIKELY(cast<unsigned>(argv[1]) < 32) ? cast<unsigned>(argv[0]) << cast<unsigned>(argv[1]) : 0;
         case sym::op_shr:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return MNL_LIKELY(cast<unsigned>(argv[1]) < 32) ? cast<unsigned>(argv[0]) >> cast<unsigned>(argv[1]) : 0;
         case sym::op_ashr: // relies on implementation-specific behavior of C++ implementation but that's OK for our target platforms
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return MNL_LIKELY(cast<unsigned>(argv[1]) < 32) ? (unsigned)((int)cast<unsigned>(argv[0]) >> (int)cast<unsigned>(argv[1])) : 0;
         case sym::op_rotl:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) << (cast<unsigned>(argv[1]) & 0x1F) | cast<unsigned>(argv[0]) >> (-(cast<unsigned>(argv[1]) & 0x1F) & 0x1F);
         case sym::op_rotr:
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return cast<unsigned>(argv[0]) >> (cast<unsigned>(argv[1]) & 0x1F) | cast<unsigned>(argv[0]) << (-(cast<unsigned>(argv[1]) & 0x1F) & 0x1F);
         case sym::op_ctz:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!cast<unsigned>(argv[0]))) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return (unsigned)__builtin_ctz(cast<unsigned>(argv[0]));
         case sym::op_clz:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!cast<unsigned>(argv[0]))) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return (unsigned)__builtin_clz(cast<unsigned>(argv[0]));
         case sym::op_log2:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!cast<unsigned>(argv[0]))) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return 31 - (unsigned)__builtin_clz(cast<unsigned>(argv[0]));
         case sym::op_c1s:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return (unsigned)__builtin_popcount(cast<unsigned>(argv[0]));
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return cast<unsigned>(argv[0]);
         case op_str:
            if (MNL_LIKELY(argc == 1)) { char res[sizeof(unsigned) * 2 + sizeof "0x"]; return sprintf(res, "0x%08X", cast<unsigned>(argv[0])), res; }
            if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!test<string>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return aux::_str(cast<unsigned>(argv[0]), cast<const string &>(argv[1]));
         case op_int:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return (long long)cast<unsigned>(argv[0]);
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      }
   }
# if false
   val pub::_eq(val &&lhs, val &&rhs) {
      switch (lhs.rep.tag()) {
      case 0x7FF8u: return  static_cast<val::root *>(lhs.rep.dat<void *>())->invoke(move(lhs), MNL_SYM("=="), 1, &rhs);
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
   val pub::_ne(val &&lhs, val &&rhs) {
      switch (lhs.rep.tag()) {
      case 0x7FF8u: return  static_cast<val::root *>(lhs.rep.dat<void *>())->invoke(move(lhs), MNL_SYM("<>"), 1, &rhs);
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
   # define MNL_M(OP, SYM) { \
      switch (lhs.rep.tag()) { \
      case 0x7FF8u: /* BoxPtr (fallback) */ \
         return static_cast<val::root *>(lhs.rep.dat<void *>())->invoke(move(lhs), MNL_SYM(SYM), 1, &rhs); \
      case 0x7FF9u: case 0x7FFBu: case 0x7FFEu: case 0x7FFFu: \
         MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
      case 0x7FFAu: /* I48 */ \
         if (MNL_UNLIKELY(!test<long long>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<long long>(lhs) OP cast<long long>(rhs); \
      default: /* F64 */ \
         if (MNL_UNLIKELY(!test<double>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<double>(lhs) OP cast<double>(rhs); \
      case 0x7FFCu: /* F32 */ \
         if (MNL_UNLIKELY(!test<float>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<float>(lhs) OP cast<float>(rhs); \
      case 0x7FFDu: /* U32 */ \
         if (MNL_UNLIKELY(!test<unsigned>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return cast<unsigned>(lhs) OP cast<unsigned>(rhs); \
      } \
   } \
   // end # define MNL_M(OP, SYM)
   val pub::_lt(val &&lhs, val &&rhs) MNL_M(<,  "<")
   val pub::_le(val &&lhs, val &&rhs) MNL_M(<=, "<=")
   val pub::_gt(val &&lhs, val &&rhs) MNL_M(>,  ">")
   val pub::_ge(val &&lhs, val &&rhs) MNL_M(>=, ">=")
   # undef MNL_M
   # define MNL_M(OP, SYM) val pub::OP(val &&lhs, val &&rhs) { \
      switch (lhs.rep.tag()) { \
      case 0x7FF8u: /* BoxPtr (fallback) */ \
         return static_cast<val::root *>(lhs.rep.dat<void *>())->invoke(move(lhs), MNL_SYM(SYM), 1, &rhs); \
      case 0x7FF9u: case 0x7FFBu: case 0x7FFEu: case 0x7FFFu: \
         MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
      case 0x7FFAu: /* I48 */ \
         if (MNL_UNLIKELY(!test<long long>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::OP(cast<long long>(lhs), cast<long long>(rhs)); \
      default: /* F64 */ \
         if (MNL_UNLIKELY(!test<double>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::OP(cast<double>(lhs), cast<double>(rhs)); \
      case 0x7FFCu: /* F32 */ \
         if (MNL_UNLIKELY(!test<float>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::OP(cast<float>(lhs), cast<float>(rhs)); \
      case 0x7FFDu: /* U32 */ \
         if (MNL_UNLIKELY(!test<unsigned>(rhs))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return aux::OP(cast<unsigned>(lhs), cast<unsigned>(rhs)); \
      } \
   } \
   // end # define MNL_M(OP, SYM)
   MNL_M(_add, "+") MNL_M(_sub, "-") MNL_M(_mul, "*")
   # undef MNL_M
   # define MNL_M(OP, SYM) val pub::OP(val &&rhs) { \
      switch (rhs.rep.tag()) { \
      case 0x7FF8u: /* BoxPtr (fallback) */ \
         return static_cast<val::root *>(rhs.rep.dat<void *>())->invoke(move(rhs), MNL_SYM(SYM), 0, {}); \
      case 0x7FF9u: case 0x7FFBu: case 0x7FFEu: case 0x7FFFu: \
         MNL_ERR(MNL_SYM("UnrecognizedOperation")); \
      case 0x7FFAu: return aux::OP(cast<long long>(rhs)); \
      default:      return aux::OP(cast<double>(rhs)); \
      case 0x7FFCu: return aux::OP(cast<float>(rhs)); \
      case 0x7FFDu: return aux::OP(cast<unsigned>(rhs)); \
      } \
   } \
   // end # define MNL_M(OP, SYM)
   MNL_M(_neg, "Neg") MNL_M(_abs, "Abs")
   # undef MNL_M
   val pub::_xor(val &&lhs, val &&rhs) {
      switch (lhs.rep.tag()) {
      case 0x7FF8u: // BoxPtr (fallback)
         return static_cast<val::root *>(lhs.rep.dat<void *>())->invoke(move(lhs), MNL_SYM("Xor"), 1, &rhs);
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
   val pub::_not(val &&rhs) {
      switch (rhs.rep.tag()) {
      case 0x7FF8u: // BoxPtr (fallback)
         return static_cast<val::root *>(rhs.rep.dat<void *>())->invoke(move(rhs), MNL_SYM("~"), 0, {});
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
# endif

   val proc_Min::invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      switch (argv[0].rep.tag()) {
      case 0x7FF8u: case 0x7FF9u: case 0x7FFBu: case 0x7FFEu: case 0x7FFFu: // BoxPtr (fallback)
         return move(argv[safe_cast<int>(MNL_SYM("Order")(args<2>{argv[0], argv[1]})) > 0]);
      case 0x7FFAu: // I48
         if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[cast<long long>(argv[0]) > cast<long long>(argv[1])]);
      default: // F64
         if (MNL_UNLIKELY(!test<double>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[std::signbit(cast<double>(argv[0])) < std::signbit(cast<double>(argv[1])) || cast<double>(argv[0]) > cast<double>(argv[1])]);
      case 0x7FFCu: // F32
         if (MNL_UNLIKELY(!test<float>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[std::signbit(cast<float>(argv[0])) < std::signbit(cast<float>(argv[1])) || cast<float>(argv[0]) > cast<float>(argv[1])]);
      case 0x7FFDu: // U32
         if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[cast<unsigned>(argv[0]) > cast<unsigned>(argv[1])]);
      }
   }
   val proc_Max::invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      switch (argv[0].rep.tag()) {
      case 0x7FF8u: case 0x7FF9u: case 0x7FFBu: case 0x7FFEu: case 0x7FFFu: // BoxPtr (fallback)
         return move(argv[safe_cast<int>(MNL_SYM("Order")(args<2>{argv[0], argv[1]})) < 0]);
      case 0x7FFAu: // I48
         if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[cast<long long>(argv[0]) < cast<long long>(argv[1])]);
      default: // F64
         if (MNL_UNLIKELY(!test<double>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[std::signbit(cast<double>(argv[0])) > std::signbit(cast<double>(argv[1])) || cast<double>(argv[0]) < cast<double>(argv[1])]);
      case 0x7FFCu: // F32
         if (MNL_UNLIKELY(!test<float>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[std::signbit(cast<float>(argv[0])) > std::signbit(cast<float>(argv[1])) || cast<float>(argv[0]) < cast<float>(argv[1])]);
      case 0x7FFDu: // U32
         if (MNL_UNLIKELY(!test<unsigned>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return move(argv[cast<unsigned>(argv[0]) < cast<unsigned>(argv[1])]);
      }
   }
   template class box<proc_Min>;
   template class box<proc_Max>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

# define MNL_STRUCT_ITER(CONTAINER, ITERATOR) \
   struct iter { \
      val base; CONTAINER::ITERATOR begin; long size; \
      iter(val &&base, CONTAINER::ITERATOR begin, long size): base(move(base)), begin(begin), size(size) {} \
      iter(iter &&rhs): iter(move(rhs.base), begin, size) {} \
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) { \
         switch (op) { \
         case sym::op_apply: \
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= size)) MNL_ERR(MNL_SYM("IndexOutOfRange")); \
            return begin[cast<long long>(argv[0])]; \
         case sym::op_size: \
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return (long long)size; \
         case sym::op_elems: \
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return move(self); \
         case sym::op_caret: \
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
            return CONTAINER(begin, begin + size); \
         } \
         return self.default_invoke(op, argc, argv); \
      } \
   } \
// end # define MNL_STRUCT_ITER(CONTAINER, ITERATOR)

   // IMDOOs = Intentionally Missing Dynamic Optimization Opportunities

   template<> val box<string>::invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      static const auto compact = [](string &dat)
         { if (MNL_UNLIKELY(dat.capacity() > dat.size() * 2)) dat.shrink_to_fit(); };
      switch (op) {
      case sym::op_apply:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<long long>(argv[0]))) { // String[Index]
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return dat[cast<long long>(argv[0])];
         }
         if (MNL_LIKELY(test<range<>>(argv[0]))) { // String[Range[Low; High]] - IMDOOs: cast<range<>>(argv[0]).lo
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            if (MNL_LIKELY(rc() == 1) && MNL_LIKELY(!cast<range<>>(argv[0]).lo))
               return dat.resize(cast<range<>>(argv[0]).hi), compact(dat), move(self);
            return string(dat.cbegin() + cast<range<>>(argv[0]).lo, dat.cbegin() + cast<range<>>(argv[0]).hi);
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) { // String[RevRange[Low; High]] - IMDOOs: all
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return string(dat.crend() - cast<range<true>>(argv[0]).hi, dat.crend() - cast<range<true>>(argv[0]).lo);
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case sym::op_repl:
         if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<long long>(argv[0]))) { // String.Repl[Index; NewElem]
            if (MNL_UNLIKELY(!test<unsigned>(argv[1])))
               MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            if (MNL_UNLIKELY(cast<unsigned>(argv[1]) > lim<unsigned char>::max()))
               MNL_ERR(MNL_SYM("ConstraintViolation"));
            return MNL_LIKELY(rc() == 1)
               ? (dat[cast<long long>(argv[0])] = cast<char>(argv[1]), move(self))
               : [&]()->val{ val res = dat; cast<string &>(res)[cast<long long>(argv[0])] = cast<char>(argv[1]); return res; }();
         }
         if (MNL_UNLIKELY(!test<string>(argv[1])))
            MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) { // String.Repl[Range[Low; High]; NewString] - IMDOOs: all
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return
               string(dat.cbegin(), dat.cbegin() + cast<range<>>(argv[0]).lo) +
               cast<const string &>(argv[1]) +
               string(dat.cbegin() + cast<range<>>(argv[0]).hi, dat.cend());
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) { // String.Repl[RevRange[Low; High]; NewString] - IMDOOs: all
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return
               string(dat.cbegin(), dat.cbegin() + cast<range<true>>(argv[0]).lo) +
               string(cast<const string &>(argv[1]).rbegin(), cast<const string &>(argv[1]).rend()) +
               string(dat.cbegin() + cast<range<true>>(argv[0]).hi, dat.cend());
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case sym::op_size:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return (long long)dat.size();
      case sym::op_add: // IMDOOs: cast<const vector<val> &>(argv[0]).empty()
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return MNL_LIKELY(rc() == 1) ? (dat += cast<const string &>(argv[0]), move(self)) : dat.empty() ? move(argv[0]) : dat + cast<const string &>(argv[0]);
      case sym::op_or:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<unsigned>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(cast<unsigned>(argv[0]) > lim<unsigned char>::max())) MNL_ERR(MNL_SYM("ConstraintViolation"));
         return MNL_LIKELY(rc() == 1) ? (dat += cast<char>(argv[0]), move(self)) : dat + cast<char>(argv[0]);
      case sym::op_elems:
         if (MNL_LIKELY(argc == 0)) return move(self); // String.Elems[]
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) { // String.Elems[Range[Low; High]] - IMDOOs: String.Elems[Range[0; Size[String]]]
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            MNL_STRUCT_ITER(string, const_iterator);
            return iter{move(self), dat.cbegin() + cast<range<>>(argv[0]).lo, (long)(cast<range<>>(argv[0]).hi - cast<range<>>(argv[0]).lo)};
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) { // String.Elems[RevRange[Low; High]]
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            MNL_STRUCT_ITER(string, const_reverse_iterator);
            return iter{move(self), dat.crend() - cast<range<true>>(argv[0]).hi, (long)(cast<range<true>>(argv[0]).hi - cast<range<true>>(argv[0]).lo)};
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case sym::op_keys:
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
      case sym::op_caret:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return move(self);
      case sym::op_eq:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return  MNL_LIKELY(test<string>(argv[0])) && (MNL_IF_WITH_IDENT_OPT(&dat == &cast<const string &>(argv[0]) ||) dat == cast<const string &>(argv[0]));
      case sym::op_ne:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return !MNL_LIKELY(test<string>(argv[0])) || (MNL_IF_WITH_IDENT_OPT(&dat != &cast<const string &>(argv[0]) &&) dat != cast<const string &>(argv[0]));
      case sym::op_order:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<string>(argv[0]))) return self.default_order(argv[0]);
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const string &>(argv[0])) return 0;)
         for (auto lhs = dat.cbegin(), rhs = cast<const string &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return -(rhs != cast<const string &>(argv[0]).end());
            if (MNL_UNLIKELY(rhs == cast<const string &>(argv[0]).end())) return +1;
            if (MNL_UNLIKELY((unsigned char)*lhs < (unsigned char)*rhs)) return -1;
            if (MNL_UNLIKELY((unsigned char)*lhs > (unsigned char)*rhs)) return +1;
         }
      case sym::op_clone: case sym::op_deep_clone:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1) ? move(self) : dat;
      case sym::op_str:
         if (MNL_LIKELY(argc == 0)) return move(self);
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         {  auto pc = cast<const string &>(argv[0]).c_str();
            for (;;) { switch (*pc) case ' ': case '#': case '+': case '-': case '0': { ++pc; continue; } break; }
            if (isdigit(*pc) && isdigit(*++pc)) ++pc;
            if (*pc == '.' && isdigit(*++pc) && isdigit(*++pc)) ++pc;
            switch (*pc) { default: MNL_ERR(MNL_SYM("SyntaxError")); case 's': ; }
            if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("SyntaxError"));
            char res[512];
            return sprintf(res, ("%" + cast<const string &>(argv[0])).c_str(), dat.c_str()), res;
         }
      }
      return self.default_invoke(op, argc, argv);
   }

   template<> val box<vector<val>>::invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) { // one instance of List is Array
      static const auto compact = [](vector<val> &dat)
         { if (MNL_UNLIKELY(dat.capacity() > dat.size() * 2)) dat.shrink_to_fit(); };
      switch (op) {
      case sym::op_apply:
         if (MNL_LIKELY(argc == 1)) {
            if (MNL_LIKELY(test<long long>(argv[0]))) { // Array[Index]
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return dat[cast<long long>(argv[0])];
            }
            if (MNL_LIKELY(test<range<>>(argv[0]))) { // Array[Range[Low; High]]
               if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if (MNL_LIKELY(rc() == 1)) return
                  dat.resize(cast<range<>>(argv[0]).hi), dat.erase(dat.begin(), dat.begin() + cast<range<>>(argv[0]).lo), compact(dat), move(self);
               return vector<val>(dat.cbegin() + cast<range<>>(argv[0]).lo, dat.cbegin() + cast<range<>>(argv[0]).hi);
            }
            if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Array[RevRange[Low; High]] - IMDOOs: all
               if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return vector<val>(dat.crend() - cast<range<true>>(argv[0]).hi, dat.crend() - cast<range<true>>(argv[0]).lo);
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }
         if (MNL_LIKELY(argc > 1)) { // Array[Index; ...]
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return dat[cast<long long>(argv[0])](argc - 1, argv + 1);
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case sym::op_repl:
         if (MNL_LIKELY(argc == 2)) {
            if (MNL_LIKELY(test<long long>(argv[0]))) { // Array.Repl[Index; NewElem]
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if (MNL_LIKELY(rc() == 1)) {
                  argv[1].swap(dat[cast<long long>(argv[0])]);
                  if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
                  return move(self);
               }
               return [&]()->val{ val res = dat;
                  argv[1].swap(cast<vector<val> &>(res)[cast<long long>(argv[0])]);
                  if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
                  return res;
               }();
            }
            if (MNL_UNLIKELY(!test<vector<val>>(argv[1])))
               MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_LIKELY(test<range<>>(argv[0]))) { // Array.Repl[Range[Low; High]; NewArray] - IMDOOs: all
               if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               auto size =
                  (size_t)cast<range<>>(argv[0]).lo +
                  cast<const vector<val> &>(argv[1]).size() +
                  (dat.size() - (size_t)cast<range<>>(argv[0]).hi); // assume no wraparound here due to heap limits
               vector<val> res; res.reserve(size);
               res.assign(dat.cbegin(), dat.cbegin() + cast<range<>>(argv[0]).lo);
               res.insert(res.cend(), cast<const vector<val> &>(argv[1]).begin(), cast<const vector<val> &>(argv[1]).end());
               res.insert(res.cend(), dat.cbegin() + cast<range<>>(argv[0]).hi, dat.cend());
               return res;
            }
            if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Array.Repl[RevRange[Low; High]; NewArray] - IMDOOs: all
               if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               auto size =
                  (size_t)cast<range<true>>(argv[0]).lo +
                  cast<const vector<val> &>(argv[1]).size() +
                  (dat.size() - (size_t)cast<range<true>>(argv[0]).hi); // assume no wraparound here due to heap limits
               vector<val> res; res.reserve(size);
               res.assign(dat.cbegin(), dat.cbegin() + cast<range<true>>(argv[0]).lo);
               res.insert(res.cend(), cast<const vector<val> &>(argv[1]).rbegin(), cast<const vector<val> &>(argv[1]).rend());
               res.insert(res.cend(), dat.cbegin() + cast<range<true>>(argv[0]).hi, dat.cend());
               return res;
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }
         if (MNL_LIKELY(argc > 2)) { // Array.Repl[Index; ...; NewElem]
            if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            long index = cast<long long>(argv[0]);
            return MNL_LIKELY(rc() == 1)
               ? (dat[index] = op(argc, (argv[0].swap(dat[index]), argv), argv_out), move(self))
               : [&]()->val{ val res = dat;
                  cast<vector<val> &>(res)[index] = op(argc, (argv[0].swap(cast<vector<val> &>(res)[index]), argv), argv_out);
                  return res;
               }();
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case sym::op_size:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return (long long)dat.size();
      case sym::op_add: // IMDOOs: cast<const string &>(argv[0]).empty()
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<vector<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return MNL_LIKELY(rc() == 1)
            ? (dat.insert(dat.end(), cast<const vector<val> &>(argv[0]).begin(), cast<const vector<val> &>(argv[0]).end()), move(self))
            : dat.empty() ? move(argv[0])
            : [&]()->val{ val res = vector<val>{};
               cast<vector<val> &>(res).reserve(dat.size() + cast<const vector<val> &>(argv[0]).size());
               cast<vector<val> &>(res) = dat;
               cast<vector<val> &>(res).insert(cast<vector<val> &>(res).end(),
                  cast<const vector<val> &>(argv[0]).begin(), cast<const vector<val> &>(argv[0]).end());
               return res;
            }();
      case sym::op_or:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1)
            ? (dat.push_back(move(argv[0])), move(self))
            : [&]()->val{ val res = vector<val>{};
               cast<vector<val> &>(res).reserve(dat.size() + 1);
               cast<vector<val> &>(res) = dat;
               cast<vector<val> &>(res).push_back(move(argv[0]));
               return res;
            }();
      case sym::op_elems:
         if (MNL_LIKELY(argc == 0)) return move(self); // Array.Elems[]
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) { // Array.Elems[Range[Low; High]] - IMDOOs: Array.Elems[Range[0; Size[Array]]]
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            MNL_STRUCT_ITER(vector<val>, const_iterator);
            return iter{move(self), dat.cbegin() + cast<range<>>(argv[0]).lo, (long)(cast<range<>>(argv[0]).hi - cast<range<>>(argv[0]).lo)};
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Array.Elems[RevRange[Low; High]]
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            MNL_STRUCT_ITER(vector<val>, const_reverse_iterator);
            return iter{move(self), dat.crend() - cast<range<true>>(argv[0]).hi, (long)(cast<range<true>>(argv[0]).hi - cast<range<true>>(argv[0]).lo)};
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case sym::op_keys:
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
      case sym::op_caret:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return move(self);
      case sym::op_eq:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<vector<val>>(argv[0]))) return false;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const vector<val> &>(argv[0])) return true;)
         for (auto lhs = dat.cbegin(), rhs = cast<const vector<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs == cast<const vector<val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const vector<val> &>(argv[0]).end())) return false;
            if (MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return false;
         }
      case sym::op_ne:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<vector<val>>(argv[0]))) return true;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const vector<val> &>(argv[0])) return false;)
         for (auto lhs = dat.cbegin(), rhs = cast<const vector<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return rhs != cast<const vector<val> &>(argv[0]).end();
            if (MNL_UNLIKELY(rhs == cast<const vector<val> &>(argv[0]).end())) return true;
            if (MNL_UNLIKELY( safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return true;
         }
      case sym::op_order:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<vector<val>>(argv[0]))) return self.default_order(argv[0]);
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const vector<val> &>(argv[0])) return 0;)
         for (auto lhs = dat.cbegin(), rhs = cast<const vector<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
            if (MNL_UNLIKELY(lhs == dat.cend())) return -(rhs != cast<const vector<val> &>(argv[0]).end());
            if (MNL_UNLIKELY(rhs == cast<const vector<val> &>(argv[0]).end())) return +1;
            auto res = safe_cast<long long>(op(args<2>{*lhs, *rhs})); if (MNL_UNLIKELY(res)) return res;
         }
      case sym::op_clone:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(rc() == 1) ? move(self) : dat;
      case sym::op_deep_clone:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return [this]()->val{ val res = dat; for (auto &&el: cast<vector<val> &>(res)) el = MNL_SYM("DeepClone")(move(el)); return res; }();
      }
      return self.default_invoke(op, argc, argv);
   }

   template<> val box<pair<vector<ast>, loc>>::invoke(val &&self, const sym &op, int argc, val argv[], val *) { // Lists annotated with locations
      switch (MNL_DISP("Apply", "Size", "Elems", "Keys")[op]) {
      case 1: // Apply
         if (MNL_LIKELY(argc == 1)) {
            if (MNL_LIKELY(test<long long>(argv[0]))) { // Array[Index]
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.first.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return dat.first[cast<long long>(argv[0])];
            }
            if (MNL_LIKELY(test<range<>>(argv[0]))) { // Array[Range[Low; High]]
               if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.first.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return vector<val>(dat.first.cbegin() + cast<range<>>(argv[0]).lo, dat.first.cbegin() + cast<range<>>(argv[0]).hi);
            }
            if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Array[RevRange[Low; High]]
               if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.first.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return vector<val>(dat.first.crend() - cast<range<true>>(argv[0]).hi, dat.first.crend() - cast<range<true>>(argv[0]).lo);
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }
         if (MNL_LIKELY(argc > 1)) { // Array[Index; ...]
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= dat.first.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return dat.first[cast<long long>(argv[0])](argc - 1, argv + 1);
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case 2: // Size
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return (long long)dat.first.size();
      case 3: // Elems - IMDOOs: materialized array always returned
         if (MNL_LIKELY(argc == 0)) // Array.Elems[]
            return vector<val>(dat.first.cbegin(), dat.first.cend());
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) { // Array.Elems[Range[Low; High]]
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.first.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return vector<val>(dat.first.cbegin() + cast<range<>>(argv[0]).lo, dat.first.cbegin() + cast<range<>>(argv[0]).hi);
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) { // Array.Elems[RevRange[Low; High]]
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.first.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return vector<val>(dat.first.crend() - cast<range<true>>(argv[0]).hi, dat.first.crend() - cast<range<true>>(argv[0]).lo);
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case 4: // Keys
         if (MNL_LIKELY(argc == 0)) return range<>{0, (long long)dat.first.size()};
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_LIKELY(test<range<>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<>>(argv[0]).hi > dat.first.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return move(argv[0]);
         }
         if (MNL_LIKELY(test<range<true>>(argv[0]))) {
            if (MNL_UNLIKELY(cast<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(cast<range<true>>(argv[0]).hi > dat.first.size()))
               MNL_ERR(MNL_SYM("IndexOutOfRange"));
            return move(argv[0]);
         }
         MNL_ERR(MNL_SYM("TypeMismatch"));
      case 0:
         return self.default_invoke(op, argc, argv);
      }
   }

// Record Composite ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<int Size> val _record<Size>::invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) {
      switch (op) {
      case sym::op_apply:
         if (MNL_LIKELY(argc == 1)) { // Record[Key]
            if (MNL_UNLIKELY(!test<sym>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!has(cast<const sym &>(argv[0])))) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            return (*this)[cast<const sym &>(argv[0])];
         }
         if (MNL_LIKELY(argc > 1)) { // Record[Key; ...]
            if (MNL_UNLIKELY(!test<sym>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!has(cast<const sym &>(argv[0])))) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            return (*this)[cast<const sym &>(argv[0])](argc - 1, argv + 1);
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case sym::op_repl:
         if (MNL_LIKELY(argc == 2)) { // Record.Repl[Key; NewComp]
            if (MNL_UNLIKELY(!test<sym>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!has(cast<const sym &>(argv[0])))) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            if (MNL_LIKELY(self.rc() == 1)) {
               argv[1].swap((*this)[cast<const sym &>(argv[0])]);
               if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
               return move(self);
            }
            return [&]()->val{ val res = *this;
               argv[1].swap(cast<_record &>(res)[cast<const sym &>(argv[0])]);
               if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
               return res;
            }();
         }
         if (MNL_LIKELY(argc > 2)) { // Record.Repl[Key; ...; NewComp]
            if (MNL_UNLIKELY(!test<sym>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!has(cast<const sym &>(argv[0])))) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            auto key = cast<const sym &>(argv[0]);
            return MNL_LIKELY(self.rc() == 1)
               ? ((*this)[key] = op(argc, (argv[0].swap((*this)[key]), argv), argv_out), move(self))
               : [&]()->val{ val res = *this;
                  cast<_record &>(res)[key] = op(argc, (argv[0].swap(cast<_record &>(res)[key]), argv), argv_out);
                  return res;
               }();
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case sym::op_eq:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<_record>(argv[0])) || MNL_UNLIKELY(descr != cast<const _record &>(argv[0]).descr)) return false;
         MNL_IF_WITH_IDENT_OPT(if (this == &cast<const _record &>(argv[0])) return true;)
         for (auto lhs = begin(const_cast<const _record *>(this)->items), rhs = begin(cast<const _record &>(argv[0]).items); lhs != end(items); ++lhs, ++rhs)
            if (MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return false;
         return true;
      case sym::op_ne:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<_record>(argv[0])) || MNL_UNLIKELY(descr != cast<const _record &>(argv[0]).descr)) return true;
         MNL_IF_WITH_IDENT_OPT(if (this == &cast<const _record &>(argv[0])) return false;)
         for (auto lhs = begin(const_cast<const _record *>(this)->items), rhs = begin(cast<const _record &>(argv[0]).items); lhs != end(items); ++lhs, ++rhs)
            if (MNL_UNLIKELY( safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return true;
         return false;
      case sym::op_order:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<_record>(argv[0]))) return self.default_order(argv[0]);
         MNL_IF_WITH_IDENT_OPT(if (this == &cast<const _record &>(argv[0])) return 0;)
         { int res = order(descr, cast<const _record &>(argv[0]).descr); if (MNL_UNLIKELY(res)) return res; }
         for (auto lhs = begin(const_cast<const _record *>(this)->items), rhs = begin(cast<const _record &>(argv[0]).items); lhs != end(items); ++lhs, ++rhs)
            { auto res = safe_cast<long long>(op(args<2>{*lhs, *rhs})); if (MNL_UNLIKELY(res)) return res; }
         return 0;
      case sym::op_clone:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return MNL_LIKELY(self.rc() == 1) ? move(self) : *this;
      case sym::op_deep_clone:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return [this]()->val{ val res = *this; for (auto &&el: cast<_record &>(res).items) el = MNL_SYM("DeepClone")(move(el)); return res; }();
      }
      return self.default_invoke(op, argc, argv);
   }
   template class box<_record<0x1>>;
   template class box<_record<0x2>>;
   template class box<_record<0x3>>;
   template class box<_record<0x4>>;
   template class box<_record<0x5>>;
   template class box<_record<0x6>>;
   template class box<_record<0x7>>;
   template class box<_record<0x8>>;
   template class box<_record<0x9>>;
   template class box<_record<0xA>>;
   template class box<_record<0xB>>;
   template class box<_record<0xC>>;
   template class box<_record<>>;

// I48 Range ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<bool Rev> val range<Rev>::invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      switch (op) {
      case sym::op_apply:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0) || MNL_UNLIKELY(cast<long long>(argv[0]) >= hi - lo)) MNL_ERR(MNL_SYM("IndexOutOfRange"));
         return !Rev ? lo + cast<long long>(argv[0]) : hi - 1 - cast<long long>(argv[0]);
      case sym::op_lo:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return lo;
      case sym::op_hi:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return hi;
      case sym::op_size:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return hi - lo;
      case sym::op_elems:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return move(self);
      case sym::op_caret:
         { vector<val> res; res.reserve(hi - lo); for (auto el = lo; el < hi; ++el) res.push_back(el); return res; }
      }
      return self.default_invoke(op, argc, argv);
   }
   template class box<range<>>;
   template class box<range<true>>;

} // namespace MNL_AUX_UUID
