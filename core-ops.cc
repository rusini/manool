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

   using std::isnan, std::isfinite;

   constexpr auto err_Undefined             = []() MNL_INLINE { MNL_ERR(MNL_SYM("Undefined")); }; // to enable basic block merging (esp. in hot section)
   constexpr auto err_PreconditionViolation = []() MNL_INLINE { MNL_ERR(MNL_SYM("PreconditionViolation")); }; // ditto

   constexpr auto err_DivisionByZero_Undefined = [](long long lhs) MNL_INLINE // ditto
      { MNL_ERR(lhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), lhs);  };

   // I48 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat>
   _div(Dat lhs, Dat rhs) { // integer division (truncating, i.e. rounding toward zero)
      if (MNL_UNLIKELY(!rhs)) err_DivisionByZero_Undefined(lhs);
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
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat>
   _rem(Dat lhs, Dat rhs) { // remainder after `/`
      if (MNL_UNLIKELY(!rhs)) err_Undefined();
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (int)lhs % (int)rhs :
      # endif
         lhs % rhs;
   }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat>
   div2(Dat lhs, Dat rhs) { // flooring division (rounding toward -infinity)
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(lhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), lhs);
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (MNL_UNLIKELY((int)lhs < 0 ^ (int)rhs < 0) && MNL_LIKELY((int)lhs % (int)rhs) ? (int)lhs / (int)rhs - 1 : (int)lhs / (int)rhs) :
      # endif
         (MNL_UNLIKELY(lhs < 0 ^ rhs < 0) && MNL_LIKELY(lhs % rhs) ? lhs / rhs - 1 : lhs / rhs);
   }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, long long>, Dat>
   _mod(Dat lhs, Dat rhs) { // modulo (remainder after `Div`)
      if (MNL_UNLIKELY(!rhs)) err_Undefined();
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (MNL_UNLIKELY((int)lhs < 0 ^ (int)rhs < 0) ? MNL_LIKELY((int)lhs % (int)rhs) ? (int)lhs % (int)rhs + (int)rhs : 0 : (int)lhs % (int)rhs) :
      # endif
         (MNL_UNLIKELY(lhs < 0 ^ rhs < 0) ? MNL_LIKELY(lhs % rhs) ? lhs % rhs + rhs : 0 : lhs % rhs);
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, long long>, std::string>
   _str(Dat arg) {
      char res[sizeof "+140737488355327"];
      return std::sprintf(res, "%lld", arg), res;
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, long long>, std::string>
   _str(Dat arg, const std::string &format) {
      using std::isdigit;
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '+': case '-': case '0': { ++pc; continue; } break; }
      if ((isdigit)(*pc) && (isdigit)(*++pc)) ++pc;
      if (*pc == '.' && (isdigit)(*++pc) && (isdigit)(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("MalformedArgument")); case 'd': case 'i': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("MalformedArgument"));
      char res[512];
      return std::sprintf(res, ("%" + std::string(format.begin(), format.end() - 1) + "lld").c_str(), arg), res;
   }

   // F64, F32 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // FP classification by preference (number of instructions for x86+sse2): isnan, isinf, isfinite
   // However, tests for inf go before tests for nan since this is how complex numbers are dealt with
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _div(Dat lhs, Dat rhs) { // division
      auto res = lhs / rhs;
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? rhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), res, rhs); // for hot paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _rem(Dat lhs, Dat rhs) { // C99/POSIX (*not* IEEE754's "remainder")
      using std::fmod; auto res = fmod(lhs, rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _neg(Dat arg) // negation (unary minus)
      { return -arg; }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _fma(Dat x, Dat y, Dat z) { // C99/POSIX/IEEE754
      using std::fma; auto res = fma(x, y, z);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _order(Dat lhs, Dat rhs) {
      std::conditional_t<std::is_same_v<Dat, double> unsigned long long, int> _lhs, _rhs;
      std::memcpy(&_lhs, &lhs, sizeof _lhs), std::memcpy(&_rhs, &rhs, sizeof _rhs);
      _lhs ^= 1u << (sizeof _lhs) * 8 - 1, _rhs ^= 1u << (sizeof _rhs) * 8 - 1;
      return (lhs > rhs) - (lhs < rhs);
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _abs(Dat arg) // magnitude (absolute value)
      { using std::abs; return abs(arg); }
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sign(Dat arg) // traditional `sign` function preserving "negative zero"
      { using std::copysign; return arg == 0 ? arg : copysign((Dat)1, arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sign(Dat abs, Dat sign) // C99/POSIX/IEEE754 `copysign`
      { using std::copysign; return copysign(abs, sign); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _exp(Dat arg) { // C99/POSIX/IEEE754
      using std::exp; auto res = exp(arg);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _expm1(Dat arg) { // C99/POSIX/IEEE754
      using std::expm1; auto res = expm1(arg);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log(Dat arg) { // C99/POSIX/IEEE754
      using std::log; auto res = log(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // for regular paths
      // as per IEEE 754 `log` never results in overflow
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log(Dat base, Dat arg) { // arbitrary-base `log` with "proper" error reporting (useful outside of numerical modeling)
      using std::log2;
      if (MNL_UNLIKELY(base == 0)) (err_Undefined)();
      auto res = log2(arg) / log2(base);
      if (MNL_LIKELY(isfinite(res))) return res; // Undefined signaled for 0/1 bases (not DivisionByZero)
      MNL_ERR(!isnan(res) && base != 1 ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res, base);
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log1p(Dat arg) { // C99/POSIX/IEEE754
      using std::log1p; auto res = log1p(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log10(Dat arg) { // C99/POSIX/IEEE754
      using std::log10; auto res = log10(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // for regular paths
      // as per IEEE 754 `log` never results in overflow
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _log2(Dat arg) { // C99/POSIX/IEEE754
      using std::log2; auto res = log2(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // for regular paths
      // as per IEEE 754 `log` never results in overflow
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sqr(Dat arg) // Wirth's `sqr`; also complementary to sqrt and hypot
      { return (_mul)(arg, arg); } // TODO: maybe put into the use point - makes no sense ho hide error signaling here, it's just an "alias"
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sqrt(Dat arg) { // C99/POSIX/IEEE754
      using std::sqrt; auto res = sqrt(arg);
      if (MNL_LIKELY(!isnan(res))) return res;
      err_Undefined(); // for hot paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _hypot(Dat x, Dat y) { // C99/POSIX/IEEE754
      using std::hypot; auto res = hypot(x, y);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _cbrt(Dat arg) // C99/POSIX/IEEE754
      { using cbrt; return cbrt(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _pow(Dat base, Dat arg) { // generalized (algebraic/analytic) exponent as defined in C99/POSIX
      using pow; auto res = pow(base, arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? base == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), res, base); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sin(Dat arg) // C99/POSIX/IEEE754
      { using std::sin; return sin(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _cos(Dat arg) // C99/POSIX/IEEE754
      { using std::cos; return cos(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _tan(Dat arg) // C99/POSIX/IEEE754
      { using std::tan; return tan(arg); } // as per IEEE 754 tan never results in overflow (neither in division by zero)
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _asin(Dat arg) { // C99/POSIX/IEEE754
      using std::asin; auto res = asin(arg);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _acos(Dat arg) { // C99/POSIX/IEEE754
      using std::acos; auto res = acos(arg);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _atan(Dat arg) // C99/POSIX/IEEE754
      { using std::atan; return atan(rhs); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _atan(Dat y, Dat x) // C99/POSIX/IEEE754 `atan2`
      { using std::atan2; return atan2(y, x); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _sinh(Dat arg) { // C99/POSIX/IEEE754
      using std::sinh; auto res = sinh(arg);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _cosh(Dat arg) { // C99/POSIX/IEEE754
      using std::cosh; auto res = cosh(arg);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _tanh(Dat arg) // C99/POSIX/IEEE754
      { using std::tanh; return tanh(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _asinh(Dat arg) // C99/POSIX/IEEE754
      { using std::asinh; return asinh(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _acosh(Dat arg) { // C99/POSIX/IEEE754
      using std::acosh; auto res = acosh(arg);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined")); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _atanh(Dat arg) { // C99/POSIX/IEEE754
      using std::atanh; auto res = atanh(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _erf(Dat arg) // C99/POSIX
      { using std::erf; return erf(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _erfc(Dat arg) // C99/POSIX
      { using std::erfc; return erfc(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _gamma(Dat arg) { // C99/POSIX (incl. error reporting deviations)
      using std::tgamma, std::trunc;
      auto res = tgamma(arg);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(arg <= 0 && trunc(arg) == arg ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"), arg); // for regular paths
      // pole error for negative integers (contrary to POSIX)
   }
   # define MNL_M(DAT, SUFFIX) /* c++ overloads for some POSIX-specific stuff from <math.h> */ \
      MNL_INLINE static inline DAT lgamma_r(DAT arg, int *sign) { return ::lgamma##SUFFIX##_r(arg, sign); } \
      MNL_INLINE static inline DAT j0(DAT arg)                  { return ::j0##SUFFIX(arg); } \
      MNL_INLINE static inline DAT j1(DAT arg)                  { return ::j1##SUFFIX(arg); } \
      MNL_INLINE static inline DAT jn(int n, DAT arg)           { return ::jn##SUFFIX(n, arg); } \
      MNL_INLINE static inline DAT y0(DAT arg)                  { return ::y0##SUFFIX(arg); } \
      MNL_INLINE static inline DAT y1(DAT arg)                  { return ::y1##SUFFIX(arg); } \
      MNL_INLINE static inline DAT yn(int n, DAT arg)           { return ::yn##SUFFIX(n, arg); } \
   // end # define MNL_M(DAT, SUFFIX)
      MNL_M(double, /*empty*/) MNL_M(float, f)
   # undef MNL_M
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _lgamma(Dat arg) { // C99/POSIX
      using std::trunc;
      int _; Dat res = (lgamma_r)(arg, &_); // introduced by POSIX (std::lgamma is not MT-safe under POSIX)
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(arg <= 0 && trunc(arg) == arg ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"), arg); // for regular paths
      // pole error for nonpositive integers as per POSIX
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _jn(long long n, Dat arg) { // POSIX (incl. error reporting deviations) - Bessel function of the first kind of integer order
      if (MNL_UNLIKELY((int)n != n)) MNL_ERR(MNL_SYM("UnsupportedArgument"));
      return (int)n == 0 ? (j0)(arg) : (int)n == 1 ? (j1)(arg) : (jn)(n, arg); // dynamic (run-time) specialization
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _yn(long long n, Dat arg) { // POSIX (incl. error reporting deviations) - Bessel function of the second kind of integer order
      if (MNL_UNLIKELY((int)n != n)) MNL_ERR(MNL_SYM("UnsupportedArgument"));
      Dat res = (int)n == 0 ? (y0)(arg) : (int)n == 1 ? (y1)(arg) : (yn)(n, arg); // dynamic (run-time) specialization
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(arg >= 0 ? arg == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), arg); // for regular paths
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _trunc(Dat arg) // C99/POSIX/IEEE754
      { using std::trunc; return trunc(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _round(Dat arg) // C99/POSIX/IEEE754
      { using std::round; return round(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _floor(Dat arg) // C99/POSIX/IEEE754
      { using std::floor; return floor(arg); }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, Dat>
   _ceil(Dat arg) // C99/POSIX/IEEE754
      { using std::ceil; return ceil(arg); }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double>, std::string>
   _str(Dat arg) {
      char res[sizeof "+1.7976931348623157e+308"];
      return std::sprintf(res, "%.16e", arg), res;
   }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, float>, std::string>
   _str(Dat arg) {
      char res[sizeof "+3.40282347e+38"];
      return std::sprintf(res, "%.8e", arg), res;
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, std::string>
   _str(Dat arg, const std::string &format) {
      using std::isdigit;
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '+': case '-': case '0': { ++pc; continue; } break; }
      if ((isdigit)(*pc) && (isdigit)(*++pc)) ++pc;
      if (*pc == '.' && (isdigit)(*++pc) && (isdigit)(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("MalformedArgument")); case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("MalformedArgument"));
      char res[512];
      return std::sprintf(res, ("%" + format).c_str(), arg), res;
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, double> | std::is_same_v<Dat, float>, long long>
   _int(Dat arg) {
      if (MNL_LIKELY((double)arg >= val::min_i48) && MNL_LIKELY((double)arg <= val::max_i48)) return (double)arg;
      (err_Overflow)(); // for hot paths
   }

   // U32 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _div(Dat lhs, Dat rhs)
      { if (MNL_UNLIKELY(!rhs)) err_PreconditionViolation(); return lhs / rhs; }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _rem(Dat lhs, Dat rhs)
      { if (MNL_UNLIKELY(!rhs)) err_PreconditionViolation(); return lhs % rhs; }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _shl(Dat lhs, Dat rhs)
      { return rhs < 32 ? lhs << rhs : 0; }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _shr(Dat lhs, Dat rhs)
      { return rhs < 32 ? lhs >> rhs : 0; }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _asr(Dat lhs, Dat rhs)
      { return rhs < 32 ? (int)lhs >> rsh : 0; }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _rol(Dat lhs, Dat rhs)
      { return lhs << (rhs & 0x1F) | lhs >> (-(rhs & 0x1F) & 0x1F); }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _ror(Dat lhs, Dat rhs)
      { return lhs >> (rhs & 0x1F) | lhs << (-(rhs & 0x1F) & 0x1F); }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _clz(Dat arg)
      { if (MNL_UNLIKELY(!arg)) err_PreconditionViolation(); return __builtin_clz(arg); }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   _ctz(Dat arg)
      { if (MNL_UNLIKELY(!arg)) err_PreconditionViolation(); return __builtin_ctz(arg); }
   template<typename Dat> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, Dat>
   bitsum(Dat arg)
      { return __builtin_popcount(arg); }

   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, std::string>
   _str(Dat arg, const std::string &format) {
      using std::isdigit;
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '+': case '-': case '0': { ++pc; continue; } break; }
      if ((isdigit)(*pc) && (isdigit)(*++pc)) ++pc;
      if (*pc == '.' && (isdigit)(*++pc) && (isdigit)(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("MalformedArgument")); case 'u': case 'o': case 'x': case 'X': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("MalformedArgument"));
      char res[512];
      return std::sprintf(res, ("%" + format).c_str(), arg), res;
   }
   template<typename Dat>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Dat, unsigned>, long long>
   _int(Dat arg)
      { return arg; }
} // namespace aux

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template val val::_invoke(const val  &, const sym &, int, val [], val *);
   template val val::_invoke(      val &&, const sym &, int, val [], val *);

   template<typename Self> MNL_NOINLINE MNL_HOT val val::_invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out) {
      switch (self.rep.tag()) MNL_NOTE(jumptable) {
         static constexpr auto err_InvalidInvocation = []() MNL_INLINE{ MNL_ERR(MNL_SYM("InvalidInvocation")); };
         static constexpr auto err_TypeMismatch      = []() MNL_INLINE{ MNL_ERR(MNL_SYM("TypeMismatch"));      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// F64/F32 //
         {  static constexpr auto dispatch = [](auto self, auto &op, auto &argc, auto &argv, auto &_self) MNL_INLINE->val{
               switch (op) MNL_NOTE(jumptable) {
               case sym::id("+"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return (_add)(self, as<decltype(self)>(argv[0]));
               case sym::id("-"):
                  if (MNL_UNLIKELY(argc != 1)) {
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return (_neg)(self);
                  }
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return (_sub)(self, as<decltype(self)>(argv[0]));
               case sym::id("*"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return (_mul)(self, as<decltype(self)>(argv[0]));
               case sym::id("/"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return (_div)(self, as<decltype(self)>(argv[0]));
               case sym::id("=="):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  return  MNL_LIKELY(is<decltype(self)>(argv[0])) && self == as<decltype(self)>(argv[0]); // maybe suboptimal but straightforward
               case sym::id("<>"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  return !MNL_LIKELY(is<decltype(self)>(argv[0])) || self != as<decltype(self)>(argv[0]); // maybe suboptimal but straightforward
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
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) return _self.default_order(argv[0]); // TODO: left type is actually known
                  return (_order)(self, as<decltype(self)>(argv[0]));
               case sym::id("Abs"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_abs)(self);
               case sym::id("Sign"):
                  if (MNL_UNLIKELY(argc == 1)) {
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                     return (_sign)(self, as<decltype(self)>(argv[0]));
                  }
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_sign)(self);
               case sym::id("Sqr"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_sqr)(self);
               case sym::id("Sqrt"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_sqrt)(self);
               case sym::id("Trunc"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_trunc)(self);
               case sym::id("Round"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_round)(self);
               case sym::id("Floor"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_floor)(self);
               case sym::id("Ceil"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_ceil)(self);
               case sym::id("Clone"): case sym::id("DeepClone"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return val{_self.rep};
               case sym::id("Int"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (_int)(self);
               }
               return [self, &op, argc, argv]() MNL_NOINLINE->val{
                  switch (MNL_EARLY(disp{"Rem", "FMA", "Exp", "Expm1", "Log", "Log1p", "Log10", "Log2", "Hypot", "Cbrt", "Pow", "Sin", "Cos", "Tan", "Asin",
                     "Acos", "Atan", "Sinh", "Cosh", "Tanh", "Asinh", "Acosh", "Atanh", "Erf", "Erfc", "Gamma", "LogGamma", "BesJn", "BesYn", "Str"})[op]) {
                  case  1: // Rem
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_rem)(self, as<decltype(self)>(argv[0]));
                  case  2: // FMA
                     if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0])) || MNL_UNLIKELY(!is<decltype(self)>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_fma)(self, as<decltype(self)>(argv[0]), as<decltype(self)>(argv[1]));
                  case  3: // Exp
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_exp)(self);
                  case  4: // Expm1
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_expm1)(self);
                  case  5: // Log
                     if (MNL_LIKELY(argc == 0)) return (_log)(self);
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_log)(self, as<decltype(self)>(argv[0]));
                  case  6: // Log1p
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_log1p)(self);
                  case  7: // Log10
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_log10)(self);
                  case  8: // Log2
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_log2)(self);
                  case  9: // Hypot
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_hypot)(self, as<decltype(self)>(argv[0]));
                  case 10: // Cbrt
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_cbrt)(self);
                  case 11: // Pow
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_pow)(self, as<decltype(self)>(argv[0]));
                  case 12: // Sin
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_sin)(self);
                  case 13: // Cos
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_cos)(self);
                  case 14: // Tan
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_tan)(self);
                  case 15: // Asin
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_asin)(self);
                  case 16: // Acos
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_acos)(self);
                  case 17: // Atan
                     if (MNL_LIKELY(argc == 1)) {
                        if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                        return (_atan)(self, as<decltype(self)>(argv[0]));
                     }
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_atan)(self);
                  case 18: // Sinh
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_sinh)(self);
                  case 19: // Cosh
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_cosh)(self);
                  case 20: // Tanh
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_tanh)(self);
                  case 21: // Asinh
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_asinh)(self);
                  case 22: // Acosh
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_acosh)(self);
                  case 23: // Atanh
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_atanh)(self);
                  case 24: // Erf
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_erf)(self);
                  case 25: // Erfc
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_erfc)(self);
                  case 26: // Gamma
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_gamma)(self);
                  case 27: // LogGamma
                     if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     return (_lgamma)(self);
                  case 28: // BesJn
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_jn)(as<long long>(argv[0]), self);
                  case 29: // BesYn
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_yn)(as<long long>(argv[0]), self);
                  case 30: // Str
                     if (MNL_LIKELY(argc == 0)) return (_str)(self);
                     if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                     if (MNL_UNLIKELY(!is<std::string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
                     return (_str)(self, as<const std::string &>(argv[0]));
                  default:
                     __builtin_unreachable();
                  case int{}:;
                  }
                  MNL_ERR(MNL_SYM("UnrecognizedOperation"));
               }();
            };
         default: // F64
            dispatch(as<double>(self), op, argc, argv, self);
         case 0xFFF8 + 0b010: // F32
            dispatch(as<float> (self), op, argc, argv, self);
         }
      case 0xFFF8 + 0b111: //////////////////////////////////////////////////////////////////////////////////////////// BoxPtr (fallback) //
         return static_cast<root *>(self.rep.template dat<void *>())->_invoke(std::forward<Self>(self), op, argc, argv, argv_out);
      case 0xFFF8 + 0b000: ////////////////////////////////////////////////////////////////////////////////////////////////////////// Nil //
         switch (op) MNL_NOTE(jumptable) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  is<>(argv[0]);
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !is<>(argv[0]);
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return self.default_order(argv[0]);
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return {};
         case sym::id("Str"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return MNL_EARLY((val)"Nil");
         case sym::id("Weak"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return {};
         }
         return [&op, argc]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"^", "Set"})[op]) {
            case 1: // ^
               if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               MNL_ERR(MNL_SYM("IndirectionByNil"));
            case 2: // Set
               if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               MNL_ERR(MNL_SYM("IndirectionByNil"));
            default:
               __builtin_unreachable();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      case 0xFFF8 + 0b001: ////////////////////////////////////////////////////////////////////////////////////////////////////////// I48 //
         switch (op) MNL_NOTE(jumptable) {
         case sym::id("+"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_add)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("-"):
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return (_neg)(as<long long>(self));
            }
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_sub)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("*"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_mul)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("/"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_div)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Rem"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_rem)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Div"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (div2)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Mod"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return (_mod)(as<long long>(self), as<long long>(argv[0]));
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return std::memcmp(&self, &argv[0], sizeof val) == 0;
            // better than ` MNL_LIKELY(is<long long>(argv[0])) && as<long long>(self) == as<long long>(argv[0])`
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return std::memcmp(&self, &argv[0], sizeof val) != 0;
            // better than `!MNL_LIKELY(is<long long>(argv[0])) || as<long long>(self) != as<long long>(argv[0])`
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
            return (as<long long>(self) > as<long long>(argv[0])) - (as<long long>(self) < as<long long>(argv[0]));
         case sym::id("Abs"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_abs)(as<long long>(self));
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return val{self.rep}; // better than `as<long long>(self)`
         }
         return [&self, &op, argc, argv]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_LIKELY(argc == 0)) return (_str)(as<long long>(self));
               if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               return (_str)(as<long long>(self), as<const std::string &>(argv[0]));
            default:
               __builtin_unreachable();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      case 0xFFF8 + 0b110: ////////////////////////////////////////////////////////////////////////////////////////////////////////// Sym //
         switch (op) MNL_NOTE(jumptable) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  MNL_LIKELY(is<const sym &>(argv[0])) && as<const sym &>(self) == as<const sym &>(argv[0]);
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !MNL_LIKELY(is<const sym &>(argv[0])) || as<const sym &>(self) != as<const sym &>(argv[0]);
         case sym::id("Order"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<sym>(argv[0]))) return self.default_order(argv[0]);
            return (as<const sym &>(self) > as<const sym &>(argv[0])) - (as<const sym &>(self) < as<const sym &>(argv[0]));
         case sym::id("Apply"):
            return as<const sym &>(self)(argc, argv, argv_out);
         case sym::id("Clone"): case sym::id("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return std::forward<Self>(self);
         }
         return [&self, &op, argc]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               return (std::string)as<const sym &>(self);
            default:
               __builtin_unreachable();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      case 0xFFF8 + 0x100 | false: /////////////////////////////////////////////////////////////////////////////////////////// Bool/False //
         switch (op) MNL_NOTE(jumptable) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() == 0xFFF8 + 0x100 | false;
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() != 0xFFF8 + 0x100 | false;
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
      case 0xFFF8 + 0x100 | true: ///////////////////////////////////////////////////////////////////////////////////////////// Bool/True //
         switch (op) MNL_NOTE(jumptable) {
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() == 0xFFF8 + 0x100 | true;
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return argv[0].rep.tag() != 0xFFF8 + 0x100 | true;
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
            return val{decltype(rep){argv[0].rep.tag() ^ true}};
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
      case 0xFFF8 + 0x011: ////////////////////////////////////////////////////////////////////////////////////////////////////////// U32 //
         switch (op) MNL_NOTE(jumptable) {
         case sym::id("+"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_add)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("-"):
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return (_neg)(as<unsigned>(self));
            }
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_sub)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("*"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_mul)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("/"): case sym::id("Div"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_div)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("Rem"): case sym::id("Mod"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_rem)(as<unsigned>(self), as<unsigned>(argv[0]));
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
            return (as<unsigned>(self) > as<unsigned>(argv[0])) - (as<unsigned>(self) < as<unsigned>(argv[0]));
         case sym::id("Abs"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_abs)(as<unsigned>(self));
         case sym::id("&"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_and)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("|"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_or )(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("~"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_not)(as<unsigned>(self));
         case sym::id("Xor"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_xor)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("SHL"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_shl)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("SHR"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_shr)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ASR"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_asr)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ROL"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_rol)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ROR"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return (_ror)(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("CLZ"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_clz)(as<unsigned>(self));
         case sym::id("CTZ"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_ctz)(as<unsigned>(self));
         case sym::id("Log2"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return 31 - (_clz)(as<unsigned>(self)); // definition
         case sym::id("BitSum"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (bitsum)(as<unsigned>(self));
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return val{self.rep}; // better than `self`
         case sym::id("Int"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (_int)(as<unsigned>(self));
         }
         return [&self, &op, argc, argv]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_LIKELY(argc == 0)) return (_str)(as<unsigned>(self));
               if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               return (_str)(as<unsigned>(self), as<const std::string &>(argv[0]));
            default:
               __builtin_unreachable();
            case int{}:;
            }
            MNL_ERR(MNL_SYM("UnrecognizedOperation"));
         }();
      }
   }



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

   // one instance of List is Array

   template<> template<bool Op, typename Self, typename Arg0>
   MNL_INLINE val box<std::vector<val>>::apply_or_fetch(Self &&self, Arg0 &&arg0) {
      if constexpr (!std::is_same_v<Arg0, const sym &>)
      if (MNL_LIKELY(is<long long>(arg0)) && MNL_LIKELY((unsigned long long)as<long long>(arg0) < dat.size()))
         return dat[as<long long>(arg0)];
      return default_apply_or_fetch<Op>(std::forward<Self>(self), std::forward<Key0>(arg0));
   }
   template<> template<typename Self, typename Arg0, typename Arg1>
   MNL_INLINE val box<std::vector<val>>::apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1) {
      if constexpr (!std::is_same_v<Arg1, const sym &>)
      if (MNL_LIKELY(is<long long>(arg0)) && MNL_LIKELY((unsigned long long)as<long long>(arg0) < dat.size()))
         return dat[as<long long>(arg0)].fetch(std::forward<Arg1>(arg1));
      return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
   }
   template<> template<bool Op, typename Self>
   MNL_INLINE val box<std::vector<val>>::apply_or_fetch(Self &&self, int argc, val argv[]) {
      if (MNL_LIKELY(argc > 1)) {
         if (MNL_LIKELY(is<long long>(argv[0])) && MNL_LIKELY((unsigned long long)as<long long>(argv[0]) < dat.size()))
            return dat[as<long long>(argv[0])].fetch(--argc, argc ? ++argv : nullptr);
      } else
      if (!Op || MNL_LIKELY(argc == 1)) {
         if (MNL_LIKELY(is<long long>(argv[0])) && MNL_LIKELY((unsigned long long)as<long long>(argv[0]) < dat.size()))
            return dat[as<long long>(argv[0])];
      }
      return default_apply_or_fetch<Op>(std::forward<Self>(self), argc, argv);
   }
   template<> template<typename Key0, typename Val>
   MNL_INLINE val box<std::vector<val>>::repl(val &&self, Key0 &&key0, Val &&value) {
      if constexpr (!std::is_same_v<Key0, const sym &>)
      if (MNL_LIKELY(is<long long>(key0)) && MNL_LIKELY((unsigned long long)as<long long>(key0) < dat.size()))
      if (std::is_same_v<Self, val> && MNL_LIKELY(!shared())) {
      # if true // We deem a check followed by a not-taken, correctly predicted branch better for performance than an extra store-after-load;
         dat[as<long long>(key0)] = std::move(value); // besides, the latter might result in wrong destruction order.
      # else
         dat[as<long long>(key0)].swap(value);
      # endif
         return std::move(self);
      } else return [this, index = as<long long>(key0), &value]() MNL_NOINLINE->val{ return [&]() MNL_INLINE{
         auto res = dat;
         res[index] = std::move(value);
         return res;
      }(); }();
      return default_repl(std::forward<Self>(self), std::forward<Key0>(key0), std::move(value));
   }
   template<> template<typename Key0, typename Key1, typename Val>
   MNL_INLINE val box<std::vector<val>>::repl(val &&self, Key0 &&key0, Key1 &&key1, Val &&value) {
      if constexpr (!std::is_same_v<Key1, const sym &>)
      if (MNL_LIKELY(is<long long>(key0)) && MNL_LIKELY((unsigned long long)as<long long>(key0) < dat.size()))
      if (std::is_same_v<Self, val> && MNL_LIKELY(!shared())) {
         auto &elem = dat[as<long long>(key0)];
         elem = std::move(elem).repl(std::forward<Key1>(key1), std::move(value));
         return std::move(self);
      } else return [this, index = as<long long>(key0), &key1, &value]() MNL_NOINLINE->val{ return [&]() MNL_INLINE{
         auto res = dat;
         auto &elem = res[index];
         elem = std::move(elem).repl(std::forward<Key1>(key1), std::move(value));
         return res;
      }(); }();
      return default_repl(std::forward<Self>(self), std::forward<Key0>(key0), std::forward<Key1>(key1), std::move(value));
   }
   template<> template<decltype(nullptr)>
   MNL_INLINE val box<std::vector<val>>::repl(val &&self, int argc, val argv[], val *argv_out) {
      if (MNL_LIKELY(argc > 2)) {
         if (MNL_LIKELY(is<long long>(argv[0])) && MNL_LIKELY((unsigned long long)as<long long>(argv[0]) < dat.size()))
         if (std::is_same_v<Self, val> && MNL_LIKELY(!shared())) {
            auto &elem = dat[as<long long>(argv[0])];
            elem = std::move(elem).repl(--argc, argc ? ++argv : nullptr, argv_out + !!argv_out); // relies on C++17 eval order
            return std::move(self);
         } else return [this, index = as<long long>(argv[0]), argv, argc]() MNL_NOINLINE->val{ return [&]() MNL_INLINE{
            auto res = dat;
            auto &elem = res[index];
            elem = std::move(elem).repl(--argc, argc ? ++argv : nullptr, argv_out + !!argv_out); // relies on C++17 eval order
            return res;
         }(); }();
      } else
      if (MNL_LIKELY(argc == 2)) {
         if (MNL_LIKELY(is<long long>(argv[0])) && MNL_LIKELY((unsigned long long)as<long long>(argv[0]) < dat.size()))
         if (std::is_same_v<Self, val> && MNL_LIKELY(!shared())) {
            auto &elem = dat[as<long long>(argv[0])];
            if (MNL_LIKELY(!argv_out)) elem = std::move(argv[1]); else argv_out[1] = std::move(elem), elem = std::move(argv[1]);
            return std::move(self);
         } else return [this, index = as<long long>(argv[0]), argv]() MNL_NOINLINE->val{ return [&]() MNL_INLINE{
            auto res = dat;
            auto &elem = res[index];
            if (MNL_LIKELY(!argv_out)) elem = std::move(argv[1]); else argv_out[1] = std::move(elem), elem = std::move(argv[1]);
            return res;
         }(); }();
      }
      return default_repl(std::forward<Self>(self), argc, argv, argv_out);
   }

   template<> template<typename Self> val box<std::vector<val>>::invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out) {
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

PreconditionFailed
ConstraintViolation
PrevalidationFailed

PreconditionViolation
MalformedArgument
UnsupportedArgument
IndexOutOfRange
KeyLookupFailed
DivisionByZero
Overflow
Undefined
IndirectionByNil
InvalidInvocation
TypeMismatch
UnrecognizedOperation

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
