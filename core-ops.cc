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
   namespace aux { inline namespace _std {
      using std::isdigit; // <cstdlib>
      using std::isnan; using std::isfinite; using std::fmod;   using std::fma;   using std::copysign; using std::signbit; // <cmath>
      using std::exp;   using std::expm1;    using std::log;    using std::log1p; using std::log10;    using std::log2; // <cmath>
      using std::sqrt;  using std::hypot;    using std::cbrt;   using std::pow; // <cmath>
      using std::sin;   using std::cos;      using std::tan;    using std::asin;  using std::acos;     using std::atan;  using std::atan2; // <cmath>
      using std::sinh;  using std::cosh;     using std::tanh;   using std::asinh; using std::acosh;    using std::atanh; // <cmath>
      using std::erf;   using std::erfc;     using std::tgamma; using std::trunc; using std::round;    using std::floor; using std::ceil; // <cmath>
      using std::sprintf; // <cstdio>
      using std::begin; using std::end; // <iterator>
   }}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# define MNL_M \
   MNL_S(_sym0,          "`") \
   MNL_S(op_shl,         "Shl") \
   MNL_S(op_shr,         "Shr") \
   MNL_S(op_ashr,        "Ashr") \
   MNL_S(op_rotl,        "Rotl") \
   MNL_S(op_rotr,        "Rotr") \
   MNL_S(op_ctz,         "Ctz") \
   MNL_S(op_clz,         "Clz") \
   MNL_S(op_c1s,         "C1s") \
   MNL_S(op_size,        "Size") \
   MNL_S(op_elems,       "Elems") \
   MNL_S(op_keys,        "Keys") \
   MNL_S(op_lo,          "Lo") \
   MNL_S(op_hi,          "Hi") \
   MNL_S(op_repl,        "Repl") \
   MNL_S(op_divf,        "Div") \
   MNL_S(op_mod,         "Mod") \
   MNL_S(op_add,         "+") \
   MNL_S(op_sub,         "-") \
   MNL_S(op_mul,         "*") \
   MNL_S(op_div,         "/") \
   MNL_S(op_rem,         "Rem") \
   MNL_S(op_neg,         "Neg") \
   MNL_S(op_and,         "&") \
   MNL_S(op_or,          "|") \
   MNL_S(op_xor,         "Xor") \
   MNL_S(op_not,         "~") \
   MNL_S(op_apply,       "Apply") \
   MNL_S(op_caret,       "^") \
   MNL_S(op_eq,          "==") \
   MNL_S(op_ne,          "<>") \
   MNL_S(op_order,       "Order") \
   MNL_S(op_clone,       "Clone") \
   MNL_S(op_deep_clone,  "DeepClone") \
   MNL_S(op_str,         "Str") \
   MNL_S(op_int,         "Int") \
   MNL_S(op_lt,          "<") \
   MNL_S(op_le,          "<=") \
   MNL_S(op_gt,          ">") \
   MNL_S(op_ge,          ">=") \
   MNL_S(op_abs,         "Abs") \
   MNL_S(op_log2,        "Log2") \
   MNL_S(op_set,         "Set") \
   MNL_S(op_weak,        "Weak") \
   MNL_S(op_fma,         "Fma") \
   MNL_S(op_sign,        "Sign") \
   MNL_S(op_exp,         "Exp") \
   MNL_S(op_expm1,       "Expm1") \
   MNL_S(op_log,         "Log") \
   MNL_S(op_log1p,       "Log1p") \
   MNL_S(op_log10,       "Log10") \
   MNL_S(op_sqr,         "Sqr") \
   MNL_S(op_sqrt,        "Sqrt") \
   MNL_S(op_hypot,       "Hypot") \
   MNL_S(op_cbrt,        "Cbrt") \
   MNL_S(op_pow,         "Pow") \
   MNL_S(op_sin,         "Sin") \
   MNL_S(op_cos,         "Cos") \
   MNL_S(op_tan,         "Tan") \
   MNL_S(op_asin,        "Asin") \
   MNL_S(op_acos,        "Acos") \
   MNL_S(op_atan,        "Atan") \
   MNL_S(op_sinh,        "Sinh") \
   MNL_S(op_cosh,        "Cosh") \
   MNL_S(op_tanh,        "Tanh") \
   MNL_S(op_asinh,       "Asinh") \
   MNL_S(op_acosh,       "Acosh") \
   MNL_S(op_atanh,       "Atanh") \
   MNL_S(op_erf,         "Erf") \
   MNL_S(op_erfc,        "Erfc") \
   MNL_S(op_gamma,       "Gamma") \
   MNL_S(op_lgamma,      "Lgamma") \
   MNL_S(op_jn,          "Jn") \
   MNL_S(op_yn,          "Yn") \
   MNL_S(op_trunc,       "Trunc") \
   MNL_S(op_round,       "Round") \
   MNL_S(op_floor,       "Floor") \
   MNL_S(op_ceil,        "Ceil") \
// end # define MNL_M
   enum sym::rep: unsigned short { // see: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90324
   # define MNL_S(ID, TXT) ID,
      MNL_M
   # undef MNL_S
   };
   MNL_PRIORITY(1001) decltype(sym::dict) sym::dict{
   # define MNL_S(ID, TXT) {TXT, ID},
      MNL_M
   # undef MNL_S
   };
   MNL_PRIORITY(1002) std::remove_extent<decltype(sym::inverse)>::type sym::inverse[lim<unsigned short>::max() + 1]{
   # define MNL_S(ID, TXT) dict.find(TXT),
      MNL_M
   # undef MNL_S
   };
    std::remove_extent<decltype(sym::rc)>::type sym::rc[lim<unsigned short>::max() + 1]{
   # define MNL_S(ID, TXT) 1,
      MNL_M
   # undef MNL_S
   };
# undef MNL_M

// Primitive Operations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux {
   constexpr auto err_Overflow  = []() MNL_INLINE{ MNL_ERR(MNL_SYM("Overflow"));  }; // to allow optimizer to blend jump targets
   constexpr auto err_Undefined = []() MNL_INLINE{ MNL_ERR(MNL_SYM("Undefined")); }; // ditto

   inline namespace _std {
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

   // I48 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, long long>, long long> _div(Val lhs, Val rhs) {
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(lhs ? MNL_ERR("DivisionByZero") : MNL_SYM("Undefined"), lhs);
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
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, long long>, long long> _rem(Dat lhs, Dat rhs) {
      if (MNL_UNLIKELY(!rhs)) err_Undefined();
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (int)lhs % (int)rhs :
      # endif
         lhs % rhs;
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, long long>, long long> _div_f/*looring*/(Val lhs, Val rhs) {
      if (MNL_UNLIKELY(!rhs)) MNL_ERR(lhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), lhs);
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (MNL_UNLIKELY((int)lhs < 0 ^ (int)rhs < 0) && MNL_LIKELY((int)lhs % (int)rhs) ? (int)lhs / (int)rhs - 1 : (int)lhs / (int)rhs) :
      # endif
         (MNL_UNLIKELY(lhs < 0 ^ rhs < 0) && MNL_LIKELY(lhs % rhs) ? lhs / rhs - 1 : lhs / rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, long long>, long long> _mod(Val lhs, Val rhs) {
      if (MNL_UNLIKELY(!rhs)) err_Undefined();
      return
      # if !__x86_64__ && !__aarch64__ // MAYBE using a 32-bit integer ALU
         MNL_LIKELY(lhs > lim<int>::min()) && MNL_LIKELY(lhs <= lim<int>::max()) && MNL_LIKELY(rhs >= lim<int>::min()) && MNL_LIKELY(rhs <= lim<int>::max()) ?
         (MNL_UNLIKELY((int)lhs < 0 ^ (int)rhs < 0) ? MNL_LIKELY((int)lhs % (int)rhs) ? (int)lhs % (int)rhs + (int)rhs : 0 : (int)lhs % (int)rhs) :
      # endif
         (MNL_UNLIKELY(lhs < 0 ^ rhs < 0) ? MNL_LIKELY(lhs % rhs) ? lhs % rhs + rhs : 0 : lhs % rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, long long>, std::string> _str(Val rhs) {
      char res[sizeof "+140737488355327"];
      return sprintf(res, "%lld", rhs), res;
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, long long>, std::string> _str(Val rhs, const std::string &format) {
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
   inline namespace _std { // from <cmath>
      using std::isnan, std::isfinite, std::fmod, std::fma, std::copysign, std::signbit;
      using std::exp, std::expm1, std::log, std::log1p, std::log10, std::log2, std::sqrt, std::hypot, std::cbrt, std::pow;
      using std::sin, std::cos, std::tan, std::asin, std::acos, std::atan, std::atan2;
      using std::sinh, std::cosh, std::tanh, std::asinh, std::acosh, std::atanh;
      using std::erf, std::erfc, std::tgamma, std::trunc, std::round, std::floor, std::ceil;
   }
# define MNL_M(VAL, SUFFIX) \
   MNL_INLINE static inline VAL lgamma_r(VAL rhs, int *sign) noexcept { return ::lgamma##SUFFIX##_r(rhs, &sign); }
   MNL_INLINE static inline VAL j0(VAL rhs) noexcept { return ::j0##SUFFIX(rhs); }
   MNL_INLINE static inline VAL j1(VAL rhs) noexcept { return ::j1##SUFFIX(rhs); }
   MNL_INLINE static inline VAL jn(int ord, VAL rhs) noexcept { return ::jn##SUFFIX(ord, rhs); }
   MNL_INLINE static inline VAL y0(VAL rhs) noexcept { return ::y0##SUFFIX(rhs); }
   MNL_INLINE static inline VAL y1(VAL rhs) noexcept { return ::y1##SUFFIX(rhs); }
   MNL_INLINE static inline VAL yn(int ord, VAL rhs) noexcept { return ::yn##SUFFIX(ord, rhs); }
// end # define MNL_M(DAT, SUFFIX)
   MNL_M(double, /*empty*/) MNL_M(float, f)
# undef MNL_M

   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _order(Val lhs, Val rhs) {
      return signbit(lhs) ^ signbit(rhs) ? signbit(rhs) - signbit(lhs) : (lhs < rhs) - (lhs > rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _div(Val lhs, Val rhs) {
      auto res = lhs / rhs;
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? rhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), res, rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _rem(Val lhs, Val rhs) {
      auto res = fmod(lhs, rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      err_Undefined();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _fma(Val a1, Val a2, Val a3) {
      auto res = fma(a1, a2, a3);
      if (MNL_LIKELY(!isinf(res))) return res;
      err_Overflow();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _sign(Val rhs) {
      return rhs > 0 ? +1 : rhs < 0 ? -1 : rhs;
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _sign(Val lhs, Val rhs) {
   # if !__aarch64__
      return copysign(lhs, rhs);
   # else // g++ v8 crashes with "internal compiler error: Segmentation fault"
      return copysign(MNL_IF_GCC8((double))lhs, rhs);
   # endif
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _exp(Val rhs) {
      auto res = exp(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      err_Overflow();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _expm1(Val rhs) {
      auto res = expm1(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      err_Overflow();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _log(Val rhs) {
      auto res = log(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // as per C99 and IEEE Std 1003.1 (POSIX) log never results in overflow
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _log(Val lhs, Val rhs) {
      auto res = log2(lhs);
      if (MNL_UNLIKELY(isinf(res)) err_Undefined()
      if (MNL_LIKELY(isfinite(res = log2(rhs) / res))) return res; // Undefined raised for 0/1 bases (not DivisionByZero)
      MNL_ERR(!isnan(res) && lhs != 1 ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res, lhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _log1p(Val rhs) {
      auto res = log1p(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // same as for log
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _log10(Val rhs) {
      auto res = log10(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // as per C99 and IEEE Std 1003.1 (POSIX) log never results in overflow
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _log2(Val rhs) {
      auto res = log2(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // as per C99 and IEEE Std 1003.1 (POSIX) log never results in overflow
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _sqr(Val rhs) {
      return _mul(rhs, rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _sqrt(Val rhs) {
      auto res = sqrt(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      err_Undefined();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _hypot(Val lhs, Val rhs) {
      auto res = hypot(lhs, rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      err_Overflow();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _cbrt(Val rhs) {
      return cbrt(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _pow(Val lhs, Val rhs) {
      auto res = pow(lhs, rhs); // IEEE 754 "pow"
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? lhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), res, lhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _sin(Val rhs) {
      return sin(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _cos(Val rhs) {
      return cos(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _tan(Val rhs) {
      return tan(rhs); // as per IEEE 754 tan never results in overflow (neither in division by zero)
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _asin(Val rhs) {
      auto res = asin(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      err_Undefined();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _acos(Val rhs) {
      auto res = acos(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      err_Undefined();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _atan(Val rhs) {
      return atan(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _atan(Val lhs, Dat rhs) {
      return atan2(lhs, rhs); // as per IEEE 754 and IEEE Std 1003.1 (POSIX) atan2(0, 0) = 0
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _sinh(Val rhs) {
      auto res = sinh(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      err_Overflow();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _cosh(Val rhs) {
      auto res = cosh(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      err_Overflow();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _tanh(Val rhs) {
      return tanh(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _asinh(Val rhs) {
      return asinh(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _acosh(Val rhs) {
      auto res = acosh(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      err_Undefined();
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _atanh(Val rhs) {
      auto res = atanh(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined"), res); // no overflow as for IEEE 754 log
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _erf(Val rhs) {
      return erf(rhs);  // specified by C99/POSIX (not by IEEE 754)
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _erfc(Val rhs) {
      return erfc(rhs); // ditto
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _gamma(Val rhs) {
      auto res = tgamma(rhs); // specified by C99, and more in detail by POSIX (not by IEEE 754)
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(rhs <= 0 && trunc(rhs) == rhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"), rhs); // pole error for negative integers (contrary to POSIX)
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _lgamma(Dat rhs) {
      int _; auto res = (lgamma_r)(rhs, &_); // introduced by POSIX (std::lgamma not thread-safe under POSIX)
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(rhs <= 0 && trunc(rhs) == rhs ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"), rhs); // pole error for nonpositive integers as per POSIX
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _jn(long long _n, Val rhs) {
      // specified by POSIX, and in more detail by glibc (not in C99/11)
      if (MNL_UNLIKELY((int)_n != _n)) MNL_ERR(MNL_SYM("LimitExceeded"));
      return (int)_n == 0 ? (j0)(rhs) : (int)_n == 1 ? (j1)(rhs) : (jn)(_n, rhs); // dynamic specialization
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _yn(long long _n, Val rhs) {
      // specified by POSIX, and in more detail by glibc (not in C99/11)
      if (MNL_UNLIKELY((int)_n != _n)) MNL_ERR(MNL_SYM("LimitExceeded"));
      auto res = (int)_n == 0 ? (y0)(rhs) : (int)_n == 1 ? (y1)(rhs) : (yn)(_n, rhs); // dynamic specialization
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? rhs == 0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"), res, rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _trunc(Val rhs) {
      return trunc(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _round(Val rhs) {
      return round(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _floor(Val rhs) {
      return floor(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, Val> _ceil(Val rhs) {
      return ceil(rhs);
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double>, std::string> _str(Val rhs) {
      char res[sizeof "+1.7976931348623157e+308"];
      return sprintf(res, "%.16e", rhs), res;
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, float>, std::string> _str(Val rhs) {
      char res[sizeof "+3.40282347e+38"];
      return sprintf(res, "%.8e", rhs), res;
   }
   template<typename Val>
   MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, std::string> _str(Val rhs, const string &format) {
      auto pc = format.c_str();
      for (;;) { switch (*pc) case ' ': case '#': case '+': case '-': case '0': { ++pc; continue; } break; }
      if (isdigit(*pc) && isdigit(*++pc)) ++pc;
      if (*pc == '.' && isdigit(*++pc) && isdigit(*++pc)) ++pc;
      switch (*pc) { default: MNL_ERR(MNL_SYM("SyntaxError")); case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A': ; }
      if (MNL_UNLIKELY(*++pc)) MNL_ERR(MNL_SYM("SyntaxError"));
      char res[512];
      return sprintf(res, ("%" + format).c_str(), rhs), res;
   }
   template<typename Val> MNL_INLINE static inline std::enable_if_t<std::is_same_v<Val, double> || std::is_same_v<Val, float>, long long> _int(Val rhs) {
      if (MNL_LIKELY((double)rhs >= min_i48) && MNL_LIKELY((double)rhs <= max_i48)) return (double)rhs;
      err_Overflow();
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
         static constexpr auto err_InvalidInvocation     = []() MNL_INLINE{ MNL_ERR(MNL_SYM("InvalidInvocation"));     };
         static constexpr auto err_TypeMismatch          = []() MNL_INLINE{ MNL_ERR(MNL_SYM("TypeMismatch"));          };
         static constexpr auto err_UnrecognizedOperation = []() MNL_INLINE{ MNL_ERR(MNL_SYM("UnrecognizedOperation")); };

         static constexpr auto _op1 = [](auto op, auto &self, auto &argc, auto &argv) MNL_INLINE{
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return op(self);
         };
         static constexpr auto _op2 = [](auto op, auto &self, auto &argc, auto &argv) MNL_INLINE{
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
            return op(self, as<decltype(self)>(argv[0]));
         };
         static constexpr auto _eq = [](auto &self, auto &argc, auto &argv) MNL_INLINE{
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  MNL_LIKELY(is<decltype(self)>(argv[0])) && self == as<decltype(self)>(argv[0]);
         };
         static constexpr auto _ne = [](auto &self, auto &argc, auto &argv) MNL_INLINE{
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !MNL_LIKELY(is<decltype(self)>(argv[0])) || self != as<decltype(self)>(argv[0]);
         };
      case rep::tag_box: ///////////////////////////////////////////////////////////////////////////////////////////////// BoxPtr (fallback)
         return static_cast<root *>(self.rep.dat<void *>())->invoke(std::forward<Self>(self), op, argc, argv, argv_out);
      case rep::tag_nil: /////////////////////////////////////////////////////////////////////////////////////////////////////////////// Nil
         switch (op) {
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
            switch (MNL_EARLY(disp({"^", "Set"}))[op]) {
            case 1: // ^
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               MNL_ERR(MNL_SYM("IndirectionByNil"));
            case 2: // Set
               if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
               MNL_ERR(MNL_SYM("IndirectionByNil"));
            default:
               MNL_UNREACHABLE();
            case int{}:
            }
            err_UnrecognizedOperation();
         }();
      case rep::tag_i48: /////////////////////////////////////////////////////////////////////////////////////////////////////////////// I48
         switch (op) {
         case sym::id("+"):
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return +as<long long>(self);
            }
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_add(as<long long>(self), as<long long>(argv[0]));
         case sym::id("-"):
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return aux::_neg(as<long long>(self));
            }
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_sub(as<long long>(self), as<long long>(argv[0]));
         case sym::id("*"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_mul(as<long long>(self), as<long long>(argv[0]));
         case sym::id("/"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_div(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Rem"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_rem(as<long long>(self), as<long long>(argv[0]));
         case sym::id("Div"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_dif(as<long long>(self), as<long long>(argv[0])); // like "adc" in asm
         case sym::id("Mod"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
            return aux::_mod(as<long long>(self), as<long long>(argv[0]));
         case sym::id("=="):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return  MNL_LIKELY(is<long long>(argv[0])) && as<long long>(self) == as<long long>(argv[0]);
         case sym::id("<>"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            return !MNL_LIKELY(is<long long>(argv[0])) || as<long long>(self) != as<long long>(argv[0]);
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
         case sym::op_Order:
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<long long>(argv[0]))) return self.default_order(argv[0]);
            return (as<long long>(self) > as<long long>(argv[0])) - (as<long long>(self) < as<long long>(argv[0]));
         case sym::id("Abs"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return aux::_abs(as<long long>(self));
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return as<long long>(self);
         }
         return [&self, &op, argc, argv]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_LIKELY(argc == 0)) return aux::_str(as<long long>(self));
               if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) err_TypeMismatch();
               return aux::_str(as<long long>(self), as<const std::string &>(argv[0]));
            default:
               MNL_UNREACHABLE();
            case int{}:
            }
            err_UnrecognizedOperation();
         }();
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// F64/F32
         {  static constexpr auto dispatch_fNN = [](auto self, auto &op, auto &argc, auto &argv) MNL_INLINE->val{ // TODO: leave as before, static constexpr
               switch (op) {
               case sym::id("+"):
                  if (MNL_UNLIKELY(argc != 1)) {
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return +self;
                  }
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_add(self, as<decltype(self)>(argv[0]));
               case sym::id("-"):
                  if (MNL_UNLIKELY(argc != 1)) {
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_neg(self);
                  }
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_sub(self, as<decltype(self)>(argv[0]));
               case sym::id("*"):
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_mul(self, as<decltype(self)>(argv[0]));
               case sym::id("/"):
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
                  return signbit(self) ^ signbit(as<decltype(self)>(argv[0])) ? signbit(as<decltype(self)>(argv[0])) - signbit(self) :
                     self < as<decltype(self)>(argv[0]) ? -1 : self != as<decltype(self)>(argv[0]);
               case sym::id("Abs"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_abs(self);
               case sym::id("Sign"):
                  if (MNL_LIKELY(argc == 0)) return aux::_sign(self);
                  if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                  if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                  return aux::_sign(self, as<decltype(self)>(argv[0]));
               case sym::id("Sqr"): // abbreviation, should be read "Square"
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_mul(self, self);
               case sym::id("Sqrt"): // read "Square Root"
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_sqrt(self);
               case sym::id("Trunc"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_trunc(self);
               case sym::id("Round"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_round(self);
               case sym::id("Floor"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_floor(self);
               case sym::id("Ceil"):
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return aux::_ceil(self);
               case sym::id("Clone"): case sym::id("DeepClone"):
                  return self;
               case sym::id("Int"):
                  return aux::_int(self);
               }
               return [self, &op, argc, argv]() MNL_NOINLINE->val{
                  switch (MNL_EARLY(disp{"Rem", "Fma", "Exp", "Expm1", "Log", "Log1p", "Log10", "Log2", "Hypot", "Cbrt", "Pow", "Sin", "Cos", "Tan",
                     "Asin", "Acos", "Atan", "Sinh", "Cosh", "Tanh", "Asinh", "Acosh", "Atanh", "Erf", "Erfc", "Gamma", "Lgamma", "Jn", "Yn", "Str"})[op]) {
                  case  1: // Rem
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                     return aux::_rem(self, as<decltype(self)>(argv[0]));
                  case  2: // Fma FMA? the difference in spelling might be due to the diff: acronym/initialism
                     if (MNL_UNLIKELY(argc != 2)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0])) || MNL_UNLIKELY(!is<decltype(self)>(argv[1]))) err_TypeMismatch();
                     return aux::_fma(as<decltype(self)>(argv[0]), as<decltype(self)>(argv[1]), self);
                  case  3: // Exp
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_exp(self);
                  case  4: // Expm1 "Exp minus one"
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_expm1(self);
                  case  5: // Log
                     if (MNL_LIKELY(argc == 0)) return aux::_log(self);
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                     return aux::_log(self, as<decltype(self)>(argv[0]));
                  case  6: // Log1p
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_log1p(self);
                  case  7: // Log10
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_log10(self);
                  case  8: // Log2
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_log2(self);
                  case  9: // Hypot
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                     return aux::_hypot(self, as<decltype(self)>(argv[0]));
                  case 10: // Cbrt CBRT?
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::cbrt(self);
                  case 11: // Pow
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                     return aux::_pow(self, as<decltype(self)>(argv[0]));
                  case 12: // Sin
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_sin(self);
                  case 13: // Cos
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_cos(self);
                  case 14: // Tan
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_tan(self);
                  case 15: // Asin
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_asin(self);
                  case 16: // Acos
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_acos(self);
                  case 17: // Atan
                     if (MNL_LIKELY(argc == 1)) {
                        if (MNL_UNLIKELY(!is<decltype(self)>(argv[0]))) err_TypeMismatch();
                        return aux::_atan(self, as<decltype(self)>(argv[0]));
                     }
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_atan(self);
                  case 18: // Sinh
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_sinh(self);
                  case 19: // Cosh
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_cosh(self);
                  case 20: // Tanh
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_tanh(self);
                  case 21: // Asinh
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_asinh(self);
                  case 22: // Acosh
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_acosh(self);
                  case 23: // Atanh
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_atanh(self);
                  case 24: // Erf
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_erf(self);
                  case 25: // Erfc - ErfCom?
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_erfc(self);
                  case 26: // Gamma
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_gamma(self);
                  case 27: // Lgamma - LogGamma?
                     if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                     return aux::_lgamma(self);
                  case 28: // Jn - BesJ?
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
                     return aux::_jn(as<long long>(argv[0]), self);
                  case 29: // Yn - BesY?
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<long long>(argv[0]))) err_TypeMismatch();
                     return aux::_yn(as<long long>(argv[0]), self);
                  case 30: // Str
                     if (MNL_LIKELY(argc == 0)) return aux::_str(self);
                     if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
                     if (MNL_UNLIKELY(!is<std::string>(argv[0]))) err_TypeMismatch();
                     return aux::_str(self, as<const std::string &>(argv[0]));
                  default:
                     MNL_UNREACHABLE();
                  case int{}:
                  }
                  err_UnrecognizedOperation();
               }();
            };
         default:
            dispatch(as<double>(self), op, argc, argv);
         case val::tag_f32:
            dispatch(as<float >(self), op, argc, argv);
         }
      case val::tag_sym: /////////////////////////////////////////////////////////////////////////////////////////////////////////////// Sym
         {  switch (op) {
            case sym::op_eq/*(==)*/:
               return _eq(as<const sym &>(self), argc, argv);
            case sym::op_ne/*(<>)*/:
               return _ne(as<const sym &>(self), argc, argv);
            case sym::op_Order:
               if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
               if (MNL_UNLIKELY(!is<sym>(argv[0]))) return self.default_order(argv[0]);
               return (as<sym>(self) > as<sym>(argv[0])) - (as<sym>(self) < as<sym>(argv[0]));
            case sym::op_Apply:
               return as<const sym &>(self)(*argv, argc - 1, argv + 1, argv_out + !!argv_out); // TODO: also a convenience/unification func exists for that
            case sym::op_Clone: case sym::op_DeepClone:
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return std::move(self);
            }
            return [&op, argc]() MNL_NOINLINE->val{
               switch (op) {
               case sym::op_Str:
                  if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
                  return (string)as<const sym &>(self);
               }
               err_UnrecognizedOperation();
            }();
         }
      case 0x7FFEu: // Bool/False
         switch (*this) {
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[0].rep.tag() == 0x7FFEu;
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[0].rep.tag() != 0x7FFEu;
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) return self.default_order(argv[0]);
            return +-as<bool>(argv[0]);
         case sym::op_or:
         case sym::op_xor:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return val{decltype(val::rep){argv[0].rep.tag()}};
         case sym::op_and:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return false;
         case sym::op_not:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return true;
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return false;
         case sym::op_str:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return MNL_EARLY((val)"False");
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case 0x7FFFu: // Bool/True
         switch (*this) {
         case sym::op_eq:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[0].rep.tag() == 0x7FFFu;
         case sym::op_ne:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return argv[0].rep.tag() != 0x7FFFu;
         case sym::op_order:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) return self.default_order(argv[0]);
            return +!as<bool>(argv[0]);
         case sym::op_and:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return val{decltype(val::rep){argv[0].rep.tag()}};
         case sym::op_or:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return true;
         case sym::op_xor:
            if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            if (MNL_UNLIKELY(!is<bool>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            return val{decltype(val::rep){argv[0].rep.tag() ^ 1}};
         case sym::op_not:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return false;
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return true;
         case sym::op_str:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return MNL_EARLY((val)"True");
         }
         MNL_ERR(MNL_SYM("UnrecognizedOperation"));
      case rep::tag_u32: /////////////////////////////////////////////////////////////////////////////////////////////////////////////// U32
         switch (op) {
         case sym::id("+"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) + as<unsigned>(argv[0]);
         case sym::id("-"):
            if (MNL_UNLIKELY(argc != 1)) {
               if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
               return -as<unsigned>(self);
            }
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) - as<unsigned>(argv[0]);
         case sym::id("*"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) * as<unsigned>(argv[0]);
         case sym::id("/"): case sym::id("Div"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) / as<unsigned>(argv[0]);
         case sym::id("Rem"): case sym::id("Mod"):
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
            return (as<unsigned>(self) > as<unsigned>(argv[0])) - (as<unsigned>(self) < as<unsigned>(argv[0]));
         case sym::id("Abs"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return +as<unsigned>(self);
         case sym::id("&"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) & as<unsigned>(argv[0]);
         case sym::id("|"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) | as<unsigned>(argv[0]);
         case sym::id("~"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return ~as<unsigned>(self);
         case sym::id("Xor"): // "Exclusive or" or just "Xor" acronym
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return as<unsigned>(self) ^ as<unsigned>(argv[0]);
         case sym::id("SHL"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _shl(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("SHR"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _shr(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("ASR"):
            if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
            if (MNL_UNLIKELY(!is<unsigned>(argv[0]))) err_TypeMismatch();
            return _asr(as<unsigned>(self), as<unsigned>(argv[0]));
         case sym::id("CTZ"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return _ctz(as<unsigned>(self));
         case sym::id("CLZ"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return _clz(as<unsigned>(self));
         case sym::id("Log2"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return 31 - _clz(as<unsigned>(self));
         case sym::id("BitSum"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return _bitsum(as<unsigned>(self));
         case sym::id("Clone"): case sym::op("DeepClone"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return as<unsigned>(self);
         case sym::id("Int"):
            if (MNL_UNLIKELY(argc != 0)) err_InvalidInvocation();
            return (long long)as<unsigned>(self);



         case sym::op_Shl:
            return op2([](auto lhs, auto rhs) MNL_INLINE{ return MNL_LIKELY(rhs < 32) ? lhs << rhs : 0; });
         case sym::op_Shr:
            return op2([](auto lhs, auto rhs) MNL_INLINE{ return MNL_LIKELY(rhs < 32) ? lhs >> rhs : 0; });
         case sym::op_Ashr: // relies on implementation-specific behavior of C++ implementation but that's OK for our target platforms
            return op2([](auto lhs, auto rhs) MNL_INLINE{ return MNL_LIKELY(rhs < 32) ? (unsigned)((int)lhs >> (int)rhs) : 0; });
         case sym::op_Rotl:
            return op2([](unsigned lhs, unsigned rhs) MNL_INLINE{ return lhs << (rhs & 0x1F) | lhs >> (-rhs & 0x1F); });
         case sym::op_Rotr:
            return op2([](unsigned lhs, unsigned rhs) MNL_INLINE{ return lhs >> (rhs & 0x1F) | lhs << (-rhs & 0x1F); });
         case sym::op_Ctz:
            static constexpr auto err_ConstraintViolation =
               []() MNL_INLINE{ MNL_ERR(MNL_SYM("ConstraintViolation")); };
            return op1( [](unsigned rhs) MNL_INLINE{
               if (MNL_UNLIKELY(!rhs)) err_ConstraintViolation();
               return (unsigned)__builtin_ctz(rhs); } );
         case sym::op_Clz:
            return op1( [](unsigned rhs) MNL_INLINE{
               if (MNL_UNLIKELY(!rhs)) err_ConstraintViolation();
               return (unsigned)__builtin_clz(rhs); } );
         case sym::op_Log2:
            return op1( [](unsigned rhs) MNL_INLINE{
               if (MNL_UNLIKELY(!rhs)) err_ConstraintViolation();
               return 31 - (unsigned)__builtin_clz(rhs); } );
         case sym::op_C1s:
            return op1( [](unsigned rhs) MNL_INLINE{
               return (unsigned)__builtin_popcount(rhs); } );
         case sym::op_clone: case sym::op_deep_clone:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return as<unsigned>(self);
         case op_int:
            if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            return (long long)as<unsigned>(self);
         }
         return [&op, argc]() MNL_NOINLINE->val{
            switch (op) {
            case op_str:
               if (MNL_LIKELY(argc == 0)) { char res[sizeof(unsigned) * 2 + sizeof "0x"]; return sprintf(res, "0x%08X", as<unsigned>(self)), res; }
               if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) err_TypeMismatch();
               return aux::_str(as<unsigned>(self), as<const string &>(argv[0]));
            }
            err_UnrecognizedOperation();
         }();
         return [&self, &op, argc, argv]() MNL_NOINLINE->val{
            switch (MNL_EARLY(disp{"Str"})[op]) {
            case 1: // Str
               if (MNL_LIKELY(argc == 0)) return aux::_str(as<unsigned>(self));
               if (MNL_UNLIKELY(argc != 1)) err_InvalidInvocation();
               if (MNL_UNLIKELY(!is<std::string>(argv[0]))) err_TypeMismatch();
               return aux::_str(as<unsigned>(self), as<const std::string &>(argv[0]));
            default:
               MNL_UNREACHABLE();
            case int{}:
            }
            err_UnrecognizedOperation();
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

   template<> template<typename Self, typename Arg0>
   MNL_INLINE val box<string>::apply(Self &&self, Arg0 &&arg0) {
      if (!MNL_LIKELY(test<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0));
      return dat[cast<long long>(arg0)];
   }
   template<> template<typename Self, typename Arg0>
   MNL_INLINE val box<string>::repl(Self &&self, Arg0 &&arg0, val &&arg1) {
      if (!MNL_LIKELY(test<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::move(arg1));
      if (std::is_same_v<Self, val> && MNL_LIKELY(rc() == 1)) {
         dat[cast<long long>(arg0)].assign(std::move(arg1));
         return std::move(self);
      }
   # if true // TODO: measure and/or inspect assembly to see which is better
      return [&]() MNL_INLINE{ val res = dat;
         cast<string &>(res)[cast<long long>(arg0)].assign(std::move(arg1));
         return res;
      }();
   # else
      auto res = dat; res[cast<long long>(arg0)].assign(std::move(arg1)); return res;
   # endif
   }
   template<> template<typename Self>
   val box<string>::invoke(Self &&self, const sym &op, int argc, val argv[], val *) {
      switch (op) {
      case sym::op_apply:
         if (MNL_UNLIKELY(argc != 1))
            []() MNL_NORETURN{ MNL_ERR(MNL_SYM("InvalidInvocation")); }();
         if (MNL_LIKELY(is<long long>(argv[0]))) { // String[Index]
            if (MNL_UNLIKELY((unsigned long long)as<long long>(argv[0]) >= dat.size()))
               []() MNL_NORETURN{ MNL_ERR(MNL_SYM("IndexOutOfRange")); }();
            return dat[as<long long>(argv[0])];
         }
         return [&]() MNL_NOINLINE->val{
            if (MNL_LIKELY(is<range<>>(argv[0]))) { // String[Range[Low; High]] - IMDOOs: as<range<>>(argv[0]).lo
               if (MNL_UNLIKELY(as<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               if constexpr (std::is_same_v<Self, val>)
               if (MNL_LIKELY(rc() == 1) && MNL_LIKELY(!as<range<>>(argv[0]).lo)) {
                  dat.resize(as<range<>>(argv[0]).hi);
                  if (MNL_UNLIKELY(dat.capacity() > dat.size() * 2)) dat.shrink_to_fit();
                  return std::move(self);
               }
               return std::string(dat.cbegin() + as<range<>>(argv[0]).lo, dat.cbegin() + as<range<>>(argv[0]).hi);
            }
            if (MNL_LIKELY(is<range<true>>(argv[0]))) { // String[RevRange[Low; High]] - IMDOOs: all
               if (MNL_UNLIKELY(as<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<true>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return std::string(dat.crend() - as<range<true>>(argv[0]).hi, dat.crend() - as<range<true>>(argv[0]).lo);
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }();
      case sym::op_repl:
         if (MNL_UNLIKELY(argc != 2))
            []() MNL_NORETURN{ MNL_ERR(MNL_SYM("InvalidInvocation")); }();
         if (MNL_LIKELY(is<long long>(argv[0]))) { // String.Repl[Index; NewElem]
            if (MNL_UNLIKELY(!is<unsigned>(argv[1])))
               []() MNL_NORETURN{ MNL_ERR(MNL_SYM("TypeMismatch")); }();
            if (MNL_UNLIKELY((unsigned long long)as<long long>(argv[0]) >= dat.size()))
               []() MNL_NORETURN{ MNL_ERR(MNL_SYM("IndexOutOfRange")); }();
            if (MNL_UNLIKELY(as<unsigned>(argv[1]) > lim<unsigned char>::max()))
               []() MNL_NORETURN{ MNL_ERR(MNL_SYM("ConstraintViolation")); }();
            return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1)
               ? (dat[cast<long long>(argv[0])] = cast<char>(argv[1]), move(self))
               : [&]()->val{ val res = dat; cast<string &>(res)[cast<long long>(argv[0])] = cast<char>(argv[1]); return res; }();


               return [&]() MNL_NOINLINE->val{
                  return [&]() MNL_INLINE{
                     auto res = dat; res[cast<long long>(argv[0])] = cast<char>(argv[1]);
                     return res;
                  }();
               }();


         }
         return [&]() MNL_NOINLINE->val{
            if (MNL_UNLIKELY(!is<std::string>(argv[1])))
               MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_LIKELY(is<range<>>(argv[0]))) { // String.Repl[Range[Low; High]; NewString] - IMDOOs: all
               if (MNL_UNLIKELY(as<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return
                  std::string(dat.cbegin(), dat.cbegin() + cast<range<>>(argv[0]).lo) +
                  as<const std::string &>(argv[1]) +
                  std::string(dat.cbegin() + cast<range<>>(argv[0]).hi, dat.cend());
            }
            if (MNL_LIKELY(is<range<true>>(argv[0]))) { // String.Repl[RevRange[Low; High]; NewString] - IMDOOs: all
               if (MNL_UNLIKELY(as<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<true>>(argv[0]).hi > dat.size()))
                  MNL_ERR(MNL_SYM("IndexOutOfRange"));
               return
                  std::string(dat.cbegin(), dat.cbegin() + cast<range<true>>(argv[0]).lo) +
                  std::string(as<const std::string &>(argv[1]).rbegin(), as<const std::string &>(argv[1]).rend()) +
                  std::string(dat.cbegin() + cast<range<true>>(argv[0]).hi, dat.cend());
            }
            MNL_ERR(MNL_SYM("TypeMismatch"));
         }();
      case sym::op_size:
         if (MNL_UNLIKELY(argc != 0)) []() MNL_NORETURN{ MNL_ERR(MNL_SYM("InvalidInvocation")); }();
         return (long long)dat.size();
      case sym::op_add: // IMDOOs: cast<const vector<val> &>(argv[0]).empty()
         if (MNL_UNLIKELY(argc != 1)) []() MNL_NORETURN{ MNL_ERR(MNL_SYM("InvalidInvocation")); }();
         if (MNL_UNLIKELY(!is<std::string>(argv[0]))) []() MNL_NORETURN{ MNL_ERR(MNL_SYM("TypeMismatch")); }();
         return std::is_same_v<Self &&, val> && MNL_LIKELY(rc() == 1) ? (dat += cast<const string &>(argv[0]), std::move(self)) :
            dat.empty() ? std::move(argv[0]) : [&]() MNL_NOINLINE->val{ return dat + cast<const string &>(argv[0]); }();

         if constexpr (std::is_same_v<Self &&, val>)
         if (MNL_LIKELY(rc() == 1)) return dat += cast<const string &>(argv[0]), std::move(self);
         if (dat.empty()) return std::move(argv[0]);
         return [&]() MNL_NOINLINE->val{ return dat + cast<const string &>(argv[0]); }();

         if constexpr (std::is_same_v<Self &&, val>)
         if (MNL_LIKELY(rc() == 1))
            return [&]() MNL_NOINLINE{ dat += as<const std::string &>(argv[0]); return std::move(self); }();
         if (dat.empty())
            return std::move(argv[0]);
         // otherwise
            return [&]() MNL_NOINLINE{ return (val)(dat + as<const std::string &>(argv[0])); }();

         if (dat.empty())
            return std::move(argv[0]);
         return [&, argv]() MNL_NOINLINE->val{
            if constexpr (std::is_same_v<Self, val>)
            if (MNL_LIKELY(rc() == 1))
               return dat += as<const std::string &>(argv[0]), std::move(self);
            // otherwise
               return dat + as<const std::string &>(argv[0]);
         }();




      case sym::op_or:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<unsigned>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(cast<unsigned>(argv[0]) > lim<unsigned char>::max())) MNL_ERR(MNL_SYM("ConstraintViolation"));
         return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1) ?
            (dat += cast<char>(argv[0]), move(self)) : dat + cast<char>(argv[0]);
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
         return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1) ? move(self) : dat;
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
   template<> template<typename Self, typename Arg0>
   MNL_INLINE val box<vector<val>>::apply(Self &&self, Arg0 &&arg0) { // check: is<long long>(arg0) works for sym
      if (!MNL_LIKELY(is<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0));
      return dat[cast<long long>(arg0)];
   }
   template<> template<typename Self, typename Arg0, typename Arg1>
   MNL_INLINE val box<vector<val>>::apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1) {
      if (!MNL_LIKELY(test<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
      return dat[cast<long long>(argv0)](std::forward<Arg1>(arg1));
   }
   template<> template<typename Self, typename Arg0>
   MNL_INLINE val box<vector<val>>::repl(Self &&self, Arg0 &&arg0, val &&arg1) {
      if (!MNL_LIKELY(test<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::move(arg1));
      if (std::is_same_v<Self, val> && MNL_LIKELY(rc() == 1)) {
         dat[cast<long long>(arg0)].assign(std::move(arg1));
         return std::move(self);
      }
   # if true // TODO: measure and/or inspect assembly to see which is better
      return [&]() MNL_INLINE{ val res = dat;
         cast<vector<val> &>(res)[cast<long long>(arg0)].assign(std::move(arg1));
         return res;
      }();
   # else
      auto res = dat; res[cast<long long>(arg0)].assign(std::move(arg1)); return res;
   # endif
   }

   template<> template<typename Self, typename Arg0>
   MNL_INLINE val box<vector<val>>::repl(Self &&self, Arg0 &&arg0, val &&arg1) {
      if (!MNL_LIKELY(test<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::move(arg1));
      if (std::is_same_v<Self, val> && MNL_LIKELY(rc() == 1)) {
         dat[cast<long long>(arg0)].assign(std::move(arg1));
         return std::move(self);
      }
      return [&]() MNL_NOINLINE->val
         { return [&]() MNL_INLINE{ auto res = dat; res[cast<long long>(arg0)].assign(std::move(arg1)); return res; }(); }();
   }


   template<> template<typename Self, typename Arg0, typename Arg1>
   MNL_INLINE val box<vector<val>>::repl(Self &&self, Arg0 &&arg0, Arg1 &&arg1, val &&arg2) {
      if (!MNL_LIKELY(test<long long>(arg0)) || !MNL_LIKELY((unsigned long long)cast<long long>(arg0) < dat.size()))
         return default_repl(std::forward<Self>(self), std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::move(arg2));
      if (std::is_same_v<Self, val> && MNL_LIKELY(rc() == 1)) {
         dat[cast<long long>(arg0)].assign(
            repl(std::move(dat[cast<long long>(arg0)]), std::forward<Arg1>(arg1), std::move(arg2)));
         return std::move(self);
      }
   # if true // TODO: measure and/or inspect assembly to see which is better
      return [&]() MNL_INLINE{ val res = dat;
         cast<vector<val> &>(res)[cast<long long>(arg0)].assign(
            repl(std::move(cast<vector<val> &>(res)[cast<long long>(arg0)]), std::forward<Arg1>(arg1), std::move(arg2)));
         return res;
      }();
   # else
      auto res = dat; res[cast<long long>(arg0)].assign(repl(std::move(res[cast<long long>(arg0)]), std::forward<Arg1>(arg1), std::move(arg2))); return res;
   # endif
   }
   template<> template<typename Self>
   MNL_INLINE val box<std::vector<val>>::invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out) {
      switch (op) {
      case sym::op_apply:
         if (MNL_LIKELY(argc == 1)) {
            if (MNL_LIKELY(is<long long>(argv[0]))) { // Array[Index]
               if (MNL_UNLIKELY((unsigned long long)as<long long>(argv[0]) >= dat.size()))
                  []() MNL_NORETURN{ MNL_ERR(MNL_SYM("IndexOutOfRange")); }();
               return dat[as<long long>(argv[0])];
            }
            return [&]() MNL_NOINLINE->val{
               if (MNL_LIKELY(is<range<>>(argv[0]))) { // Array[Range[Lo; Hi]]
                  if (MNL_UNLIKELY(as<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<>>(argv[0]).hi > dat.size()))
                     MNL_ERR(MNL_SYM("IndexOutOfRange"));
                  if constexpr (std::is_same_v<Self, val>)
                  if (MNL_LIKELY(rc() == 1)) {
                     dat.resize(as<range<>>(argv[0]).hi), dat.erase(dat.begin(), dat.begin() + as<range<>>(argv[0]).lo);
                     if (MNL_UNLIKELY(dat.capacity() > dat.size() * 2)) dat.shrink_to_fit();
                     return std::move(self);
                  }
                  return std::vector(dat.cbegin() + as<range<>>(argv[0]).lo, dat.cbegin() + as<range<>>(argv[0]).hi);
               }
               if (MNL_LIKELY(is<range<true>>(argv[0]))) { // Array[RevRange[Lo; Hi]] - IMDOOs: all
                  if (MNL_UNLIKELY(as<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<true>>(argv[0]).hi > dat.size()))
                     MNL_ERR(MNL_SYM("IndexOutOfRange"));
                  return std::vector(dat.crend() - as<range<true>>(argv[0]).hi, dat.crend() - as<range<true>>(argv[0]).lo);
               }
               MNL_ERR(MNL_SYM("TypeMismatch"));
            }();
         }
         if (MNL_LIKELY(argc > 1)) { // Array[Index; ...]
            if (MNL_UNLIKELY(!is<long long>(argv[0])))
               []() MNL_NORETURN{ MNL_ERR(MNL_SYM("TypeMismatch")); }();
            if (MNL_UNLIKELY((unsigned long long)as<long long>(argv[0]) >= dat.size()))
               []() MNL_NORETURN{ MNL_ERR(MNL_SYM("IndexOutOfRange")); }();
            return dat[as<long long>(argv[0])](argc - 1, argv + 1);
         }
         []() MNL_NORETURN{ MNL_ERR(MNL_SYM("InvalidInvocation")); }();
      case sym::op_repl:
         if (MNL_LIKELY(argc == 2)) {
            if (MNL_LIKELY(is<long long>(argv[0]))) { // Array.Repl[Index; NewElem]
               if (MNL_UNLIKELY((unsigned long long)as<long long>(argv[0]) >= dat.size()))
                  []() MNL_NOINLINE{ MNL_ERR(MNL_SYM("IndexOutOfRange")); }();
               if constexpr (std::is_same_v<Self, val>) if (MNL_LIKELY(rc() == 1)) {
                  argv[1].swap(dat[as<long long>(argv[0])]);
                  if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
                  return std::move(self);
               }
               return [&]() MNL_NOINLINE->val{
                  return [&]() MNL_INLINE{
                     auto res = dat; argv[1].swap(res[as<long long>(argv[0])]);
                     if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
                     return res;
                  }();
               }();
            }
            return [&]() MNL_NOINLINE->val{
               if (MNL_UNLIKELY(!is<std::vector<val>>(argv[1])))
                  MNL_ERR(MNL_SYM("TypeMismatch"));
               if (MNL_LIKELY(is<range<>>(argv[0]))) { // Array.Repl[Range[Low; High]; NewArray] - IMDOOs: all
                  if (MNL_UNLIKELY(as<range<>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<>>(argv[0]).hi > dat.size()))
                     MNL_ERR(MNL_SYM("IndexOutOfRange"));
                  return [&]() MNL_INLINE{
                     std::vector<val> res; res.reserve( // assume no wraparound here due to heap limits
                        (std::size_t)as<range<>>(argv[0]).lo +
                        as<const std::vector<val> &>(argv[1]).size() +
                        (dat.size() - (std::size_t)as<range<>>(argv[0]).hi) );
                     res.assign(dat.cbegin(), dat.cbegin() + as<range<>>(argv[0]).lo);
                     res.insert(res.cend(), as<const std::vector<val> &>(argv[1]).begin(), as<const std::vector<val> &>(argv[1]).end());
                     res.insert(res.cend(), dat.cbegin() + as<range<>>(argv[0]).hi, dat.cend());
                     return res;
                  }();
               }
               if (MNL_LIKELY(is<range<true>>(argv[0]))) { // Array.Repl[RevRange[Low; High]; NewArray] - IMDOOs: all
                  if (MNL_UNLIKELY(as<range<true>>(argv[0]).lo < 0) || MNL_UNLIKELY(as<range<true>>(argv[0]).hi > dat.size()))
                     MNL_ERR(MNL_SYM("IndexOutOfRange"));
                  return [&]() MNL_INLINE{
                     std::vector<val> res; res.reserve( // assume no wraparound here due to heap limits
                        (std::size_t)as<range<true>>(argv[0]).lo +
                        as<const std::vector<val> &>(argv[1]).size() +
                        (dat.size() - (std::size_t)as<range<true>>(argv[0]).hi) );
                     res.assign(dat.cbegin(), dat.cbegin() + as<range<true>>(argv[0]).lo);
                     res.insert(res.cend(), as<const std::vector<val> &>(argv[1]).rbegin(), as<const std::vector<val> &>(argv[1]).rend());
                     res.insert(res.cend(), dat.cbegin() + as<range<true>>(argv[0]).hi, dat.cend());
                     return res;
                  }();
               }
               MNL_ERR(MNL_SYM("TypeMismatch"));
            }();
         }
         if (MNL_LIKELY(argc > 2)) { // Array.Repl[Index; ...; NewElem]
            if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY((unsigned long long)cast<long long>(argv[0]) >= dat.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
            long index = cast<long long>(argv[0]);
            return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1)
               ? (dat[index] = op(argc, (argv[0].swap(dat[index]), argv), argv_out), move(self))
               : [&]()->val{ val res = dat;
                  cast<vector<val> &>(res)[index] = op(argc, (argv[0].swap(cast<vector<val> &>(res)[index]), argv), argv_out);
                  return res;
               }();
         }
         MNL_ERR(MNL_SYM("InvalidInvocation"));
      case sym::op_size:
         if (MNL_UNLIKELY(argc != 0)) []() MNL_NORETURN{ MNL_ERR(MNL_SYM("InvalidInvocation")); }();
         return (long long)dat.size();
      case sym::op_add: // IMDOOs: cast<const string &>(argv[0]).empty()
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<vector<val>>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1)
            ? (dat.insert(dat.end(), cast<const vector<val> &>(argv[0]).begin(), cast<const vector<val> &>(argv[0]).end()), move(self))
            : dat.empty() ? move(argv[0])
            : [&]()->val{ val res = vector<val>{};
               cast<vector<val> &>(res).reserve(dat.size() + cast<const vector<val> &>(argv[0]).size());
               cast<vector<val> &>(res) = dat;
               cast<vector<val> &>(res).insert(cast<vector<val> &>(res).end(),
                  cast<const vector<val> &>(argv[0]).begin(), cast<const vector<val> &>(argv[0]).end());
               return res;
            }();

            if constexpr (std::is_same_v<Self &&, val>)
            if (MNL_LIKELY(rc() == 1))
               return [&]() MNL_NOINLINE
                  { dat.insert(dat.end(), cast<const vector<val> &>(argv[0]).begin(), cast<const vector<val> &>(argv[0]).end()); return std::move(self); }();
            if (dat.empty())
               return std::move(argv[0]);
            // otherwise
               return [&]() MNL_NOINLINE{ return (val)(dat + as<const std::string &>(argv[0])); }();



      case sym::op_or:
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1)
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
         MNL_IF_WITHOUT_IDENT_OPT([&]() MNL_NOINLINE{)
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!is<vector<val>>(argv[0]))) return false;
         MNL_IF_WITH_IDENT_OPT(if (&dat == &cast<const vector<val> &>(argv[0])) return true; [&]() MNL_NOINLINE{)
            for (auto lhs = dat.cbegin(), rhs = cast<const vector<val> &>(argv[0]).begin();; ++lhs, ++rhs) {
               if (MNL_UNLIKELY(lhs == dat.cend())) return rhs == cast<const vector<val> &>(argv[0]).end();
               if (MNL_UNLIKELY(rhs == cast<const vector<val> &>(argv[0]).end())) return false;
               if (MNL_UNLIKELY(!safe_cast<bool>(op(args<2>{*lhs, *rhs})))) return false;
            }
         }();
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
         return std::is_same_v<Self &&, val &&> && MNL_LIKELY(rc() == 1) ? move(self) : dat;
      case sym::op_deep_clone:
         if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return [this]()->val{ val res = dat; for (auto &&el: cast<vector<val> &>(res)) el = MNL_SYM("DeepClone")(move(el)); return res; }();
      }
      return self.default_invoke(op, argc, argv);
   }

   template<> template<typename Self> val box<pair<vector<ast>, loc>>::invoke(Self &&self, const sym &op, int argc, val argv[], val *) {
      // Lists annotated with locations
      switch (MNL_DISP("Apply", "Size", "Elems", "Keys")[op]) {
      case 1: // Apply
         if (MNL_LIKELY(argc == 1)) {
            if (MNL_LIKELY(test<long long>(argv[0]))) { // Array[Index]
               if (MNL_UNLIKELY((unsigned long long)cast<long long>(argv[0]) >= dat.first.size()))
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
            if (MNL_UNLIKELY((unsigned long long)cast<long long>(argv[0]) >= dat.first.size())) MNL_ERR(MNL_SYM("IndexOutOfRange"));
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
   template<int Size> template<typename Self, typename Arg0>
   MNL_INLINE val box<_record<Size>>::apply(Self &&self, Arg0 &&arg0) {
      if (!MNL_LIKELY(test<sym>(arg0)) || !MNL_LIKELY(has(cast<const sym &>(arg0))))
         return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0));
      return (*this)[cast<const sym &>(arg0)];
   }
   template<int Size> template<typename Self, typename Arg0>
   MNL_INLINE val box<_record<Size>>::repl(Self &&self, Arg0 &&arg0, val &&arg1) {
      if (!MNL_LIKELY(test<sym>(arg0)) || !MNL_LIKELY(has(cast<const sym &>(arg0))))
         return default_apply(std::forward<Self>(self), std::forward<Arg0>(arg0));
      if (std::is_same_v<Self, val> && MNL_LIKELY(rc() == 1)) {
         (*this)[cast<const sym &>(arg0)].assign(std::move(arg1));
         return std::move(self);
      }
      return [&]() MNL_INLINE{ val res = *this;
         cast<_record &>(res)[cast<const sym &>(arg0)].assign(std::move(arg1));
         return res;
      }();
   }
   template<int Size> template<typename Self>
   MNL_INLINE val _record<Size>::invoke(Self &&self, const sym &op, int argc, val argv[], val *argv_out) {
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
            if (std::is_same_v<Self, val> && MNL_LIKELY(self.rc() == 1)) {
               argv[1].swap((*this)[cast<const sym &>(argv[0])]);
               if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
               return move(self);
            }
            return [&]() MNL_INLINE{ val res = *this;
               argv[1].swap(cast<_record &>(res)[cast<const sym &>(argv[0])]);
               if (MNL_UNLIKELY(argv_out)) argv[1].swap(argv_out[1]);
               return res;
            }();
         }
         if (MNL_LIKELY(argc > 2)) { // Record.Repl[Key; ...; NewComp]
            if (MNL_UNLIKELY(!test<sym>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(!has(cast<const sym &>(argv[0])))) MNL_ERR(MNL_SYM("KeyLookupFailed"));
            auto key = cast<const sym &>(argv[0]);
            return std::is_same_v<Self, val> && MNL_LIKELY(self.rc() == 1)
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
