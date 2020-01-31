// lib-ieee754-dec-main2.cc

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-lib-ieee754-dec.hh"

# include <cstdlib> // strtoll
# include <cstring> // strchr
# include <cstdio>  // sprintf
# include <utility> // move
# include <string>

namespace rusini {
   using std::strtoll; // <cstdlib>
   using std::strchr;  // <cstring>
   using std::sprintf; // <cstdio>
   using std::string; using mnl::dec;
   using mnl::sym; using mnl::val; using mnl::test; using mnl::cast;
   using mnl::code; using mnl::make_lit; using mnl::expr_export;
}

namespace rusini {
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _add(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto res = lhs + rhs;
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _sub(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto res = lhs - rhs;
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _mul(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto res = lhs * rhs;
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _div(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto res = lhs / rhs;
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? rhs == (dec<Width, Bankers>)0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _neg(dec<Width, Bankers> rhs) {
      return -rhs;
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _fma(dec<Width, Bankers> a1, dec<Width, Bankers> a2, dec<Width, Bankers> a3) {
      auto res = fma(a1, a2, a3);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _abs(dec<Width, Bankers> rhs) {
      return abs(rhs);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _exp(dec<Width, Bankers> rhs) {
      auto res = exp(rhs);
      if (MNL_LIKELY(!isinf(res))) return res;
      MNL_ERR(MNL_SYM("Overflow"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _log(dec<Width, Bankers> rhs) {
      auto res = log(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined")); // log never results in overflow (provided a balanced exponent range)
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _log(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto tmp = _log(lhs); return _div(_log(rhs), tmp);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _log10(dec<Width, Bankers> rhs) {
      auto res = log10(rhs);
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? MNL_SYM("DivisionByZero") : MNL_SYM("Undefined")); // log never results in overflow (provided a balanced exponent range)
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _sqr(dec<Width, Bankers> rhs) {
      return _mul(rhs, rhs);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _sqrt(dec<Width, Bankers> rhs) {
      auto res = sqrt(rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _pow(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto res = pow(lhs, rhs); // 0 raised to 0 is undefined, unlike the case of F## and Z##
      if (MNL_LIKELY(isfinite(res))) return res;
      MNL_ERR(!isnan(res) ? lhs == (dec<Width, Bankers>)0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow") : MNL_SYM("Undefined"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _trunc(dec<Width, Bankers> rhs) {
      return trunc(rhs);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _round(dec<Width, Bankers> rhs) {
      return round(rhs);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _floor(dec<Width, Bankers> rhs) {
      return floor(rhs);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _ceil(dec<Width, Bankers> rhs) {
      return ceil(rhs);
   }
   template<int Width, bool Bankers> MNL_INLINE static inline string _str(dec<Width, Bankers> rhs) {
      typename dec<Width, Bankers>::str_buf res; if (auto pc = strchr((+rhs).to_str(res), 'E')) *pc = 'e';
      return res;
   }
   template<int Width, bool Bankers> MNL_INLINE static inline dec<Width, Bankers> _quantize(dec<Width, Bankers> lhs, dec<Width, Bankers> rhs) {
      auto res = quantize(lhs, rhs);
      if (MNL_LIKELY(!isnan(res))) return res;
      MNL_ERR(MNL_SYM("Undefined"));
   }
   template<int Width, bool Bankers> MNL_INLINE static inline long long _int(dec<Width, Bankers> rhs) {
      if (MNL_UNLIKELY(isnan(rhs = quantize(trunc(rhs), (dec<Width, Bankers>)0)))) MNL_ERR(MNL_SYM("Overflow"));
      typename dec<Width, Bankers>::str_buf tmp; auto res = strtoll(rhs.to_str(tmp), {}, 10);
      if (MNL_UNLIKELY(res < mnl::min_i48) || MNL_UNLIKELY(res > mnl::max_i48)) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
} // namespace rusini
# define RUSINI_M(DAT) \
   template<> auto mnl::box<DAT>::invoke(val &&self, const sym &op, int argc, val argv[], val *)->val { \
   switch (MNL_DISP("+", "-", "*", "/", "Neg", "~", "Fma", "==", "<>", "<", "<=", ">", ">=", "Order", "Abs", "Exp", \
      "Log", "Log10", "Sqr", "Sqrt", "Pow", "Trunc", "Round", "Floor", "Ceil", "Clone", "Str", "Quantize", "Int")[op]) { \
   case  0: return self.default_invoke(op, argc, argv); \
      using ::std::move; namespace rus = ::rusini; \
   case  1: /* + */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_add(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_add(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_add(dat, cast<DAT>(argv[0])); \
   case  2: /* - */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sub(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_sub(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_sub(dat, cast<DAT>(argv[0])); \
   case  3: /* * */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_mul(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_mul(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_mul(dat, cast<DAT>(argv[0])); \
   case  4: /* / */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_div(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_div(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_div(dat, cast<DAT>(argv[0])); \
   case  5: case  6: /* Neg/~ */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_neg(dat), move(self); \
      return rus::_neg(dat); \
   case  7: /* Fma */ \
      if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0])) || MNL_UNLIKELY(!test<DAT>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(argv[1].rc() == 1)) return cast<DAT &>(argv[1]) = \
         rus::_fma(cast<DAT>(argv[0]), cast<DAT>(argv[1]), dat), move(argv[1]); \
      if (MNL_LIKELY(rc() == 1)) return dat = \
         rus::_fma(cast<DAT>(argv[0]), cast<DAT>(argv[1]), dat), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = \
         rus::_fma(cast<DAT>(argv[0]), cast<DAT>(argv[1]), dat), move(argv[0]); \
      return \
         rus::_fma(cast<DAT>(argv[0]), cast<DAT>(argv[1]), dat); \
   case  8: /* == */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return  MNL_LIKELY(test<DAT>(argv[0])) && dat == cast<DAT>(argv[0]); \
   case  9: /* <> */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return !MNL_LIKELY(test<DAT>(argv[0])) || dat != cast<DAT>(argv[0]); \
   case 10: /* < */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      return dat <  cast<DAT>(argv[0]); \
   case 11: /* <= */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      return dat <= cast<DAT>(argv[0]); \
   case 12: /* > */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      return dat >  cast<DAT>(argv[0]); \
   case 13: /* >= */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      return dat >= cast<DAT>(argv[0]); \
   case 14: /* Order */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      return order(+dat, +cast<DAT>(argv[0])); \
   case 15: /* Abs */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_abs(dat), move(self); \
      return rus::_abs(dat); \
   case 16: /* Exp */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_exp(dat), move(self); \
      return rus::_exp(dat); \
   case 17: /* Log */ \
      if (MNL_LIKELY(argc == 0)) return MNL_UNLIKELY(rc() > 1) ? rus::_log(dat) : (dat = rus::_log(dat), move(self)); \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_log(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_log(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_log(dat, cast<DAT>(argv[0])); \
   case 18: /* Log10 */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_log10(dat), move(self); \
      return rus::_log10(dat); \
   case 19: /* Sqr */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sqr(dat), move(self); \
      return rus::_sqr(dat); \
   case 20: /* Sqrt */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sqrt(dat), move(self); \
      return rus::_sqrt(dat); \
   case 21: /* Pow */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_pow(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_pow(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_pow(dat, cast<DAT>(argv[0])); \
   case 22: /* Trunc */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_trunc(dat), move(self); \
      return rus::_trunc(dat); \
   case 23: /* Round */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_round(dat), move(self); \
      return rus::_round(dat); \
   case 24: /* Floor */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_floor(dat), move(self); \
      return rus::_floor(dat); \
   case 25: /* Ceil */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_ceil(dat), move(self); \
      return rus::_ceil(dat); \
   case 26: /* Clone */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return MNL_LIKELY(rc() == 1) ? move(self) : dat; \
   case 27: /* Str */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_str(dat); \
   case 28: /* Quantize */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_quantize(dat, cast<DAT>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_quantize(dat, cast<DAT>(argv[0])), move(argv[0]); \
      return rus::_quantize(dat, cast<DAT>(argv[0])); \
   case 29: /* Int */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_int(dat); \
   }} \
// end # define RUSINI_M(DAT)
# define RUSINI_T(...) __VA_ARGS__
   RUSINI_M(RUSINI_T(mnl::dec<64, true>))  RUSINI_M(RUSINI_T(mnl::dec<128, true>))
   RUSINI_M(RUSINI_T(mnl::dec<64, false>)) RUSINI_M(RUSINI_T(mnl::dec<128, false>))
# undef RUSINI_T
# undef RUSINI_M

namespace rusini { extern "C" code mnl_aux_ieee754_dec() {
   # define RUSINI_M(DAT) { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) { \
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv); \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(test<long long>(argv[0]))) { char res[sizeof "+140737488355327"]; return sprintf(res, "%lld", cast<long long>(argv[0])), (DAT)res; } \
      if (MNL_LIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      switch (cast<const string &>(argv[0])[0]) { \
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': \
         break; \
      case '+': case '-': \
         if (cast<const string &>(argv[0])[1] >= '0' && cast<const string &>(argv[0])[1] <= '9' || \
             cast<const string &>(argv[0])[1] == '.') break; \
         /*[[fallthrough]];*/ \
      default: \
         MNL_ERR(MNL_SYM("SyntaxError")); \
      } \
      DAT res = cast<const string &>(argv[0]); \
      if (MNL_UNLIKELY(isnan(res))) MNL_ERR(MNL_SYM("SyntaxError")); \
      if (MNL_UNLIKELY(isinf(res))) MNL_ERR(MNL_SYM("Overflow")); \
      return res; \
   }} \
   // end # define RUSINI_M(DAT)
   # define RUSINI_T(...) __VA_ARGS__
   struct proc_D64 RUSINI_M(RUSINI_T(dec<64, true>));  struct proc_D128 RUSINI_M(RUSINI_T(dec<128, true>));
   struct proc_C64 RUSINI_M(RUSINI_T(dec<64, false>)); struct proc_C128 RUSINI_M(RUSINI_T(dec<128, false>));
   # undef RUSINI_T
   # undef RUSINI_M
   return expr_export{
      {"D64",  make_lit(proc_D64{})},
      {"C64",  make_lit(proc_C64{})},
      {"D128", make_lit(proc_D128{})},
      {"C128", make_lit(proc_C128{})},
   };
}}
