// lib-ieee754-cmpx-main2.cc

/*    Copyright (C) 2018, 2019 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-lib-ieee754-cmpx.hh"

# include <cstdlib>     // strtod, strtof
# include <cmath>       // isinf, isnan, signbit
# include <cstdio>      // sprintf
# include <type_traits> // enable_if, is_same
# include <utility>     // move
# include <string>

namespace rusini {
   using std::strtod; using std::strtof; // <cstdlib>
   using std::isinf; using std::isnan; // <cmath>
   using std::sprintf; // <cstdio>
   using std::string; using std::complex;
   using mnl::sym; using mnl::val; using mnl::test; using mnl::cast;
   using mnl::code; using mnl::make_lit; using mnl::expr_export;
} // namespace rusini

namespace rusini {
   template<typename Lhs, typename Rhs, typename Res = Lhs> using enable_same =
      typename std::enable_if<std::is_same<Lhs, Rhs>::value, Res>::type;
   template<typename Real> MNL_INLINE static inline complex<Real> _add(const complex<Real> &lhs, const complex<Real> &rhs) {
      auto res = lhs + rhs;
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _sub(const complex<Real> &lhs, const complex<Real> &rhs) {
      auto res = lhs - rhs;
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _mul(const complex<Real> &lhs, const complex<Real> &rhs) {
      auto res = lhs * rhs;
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _div(const complex<Real> &lhs, const complex<Real> &rhs) {
      auto res = lhs / rhs;
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(rhs == (Real)0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
      if (MNL_UNLIKELY(isnan(res.real())) || MNL_UNLIKELY(isnan(res.imag()))) MNL_ERR(MNL_SYM("Undefined"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _neg(const complex<Real> &rhs) {
      return -rhs;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _conj(const complex<Real> &rhs) {
      return conj(rhs);
   }
   template<typename Real> MNL_INLINE static inline Real _real(const complex<Real> &rhs) {
      return real(rhs);
   }
   template<typename Real> MNL_INLINE static inline Real _imag(const complex<Real> &rhs) {
      return imag(rhs);
   }
   template<typename Real> MNL_INLINE static inline Real _abs(const complex<Real> &rhs) {
      auto res = abs(rhs);
      if (MNL_UNLIKELY(isinf(res))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline Real _arg(const complex<Real> &rhs) {
      return arg(rhs);
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _exp(const complex<Real> &rhs) {
      auto res = exp(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _log(const complex<Real> &rhs) {
      auto res = log(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(rhs == (Real)0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _log(const complex<Real> &lhs, const complex<Real> &rhs) {
      auto tmp = _log(lhs); return _div(_log(rhs), tmp);
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _log10(const complex<Real> &rhs) {
      auto res = log10(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(rhs == (Real)0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _sqrt(const complex<Real> &rhs) {
      auto res = sqrt(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _pow(const complex<Real> &lhs, const complex<Real> &rhs) {
      auto res = pow(lhs, rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(lhs == (Real)0 ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
      if (MNL_UNLIKELY(isnan(res.real())) || MNL_UNLIKELY(isnan(res.imag()))) return 1; // consistent with F##
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _sin(const complex<Real> &rhs) {
      auto res = sin(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _cos(const complex<Real> &rhs) {
      auto res = cos(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _tan(const complex<Real> &rhs) {
      auto res = tan(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _asin(const complex<Real> &rhs) {
      auto res = asin(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _acos(const complex<Real> &rhs) {
      auto res = acos(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _atan(const complex<Real> &rhs) {
      auto res = atan(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag())))
         MNL_ERR(rhs == complex<Real>{0, +1} || rhs == complex<Real>{0, -1} ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _sinh(const complex<Real> &rhs) {
      auto res = sinh(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _cosh(const complex<Real> &rhs) {
      auto res = cosh(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _tanh(const complex<Real> &rhs) {
      auto res = tanh(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _asinh(const complex<Real> &rhs) {
      auto res = asinh(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _acosh(const complex<Real> &rhs) {
      auto res = acosh(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline complex<Real> _atanh(const complex<Real> &rhs) {
      auto res = atanh(rhs);
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag())))
         MNL_ERR(rhs == complex<Real>{+1, 0} || rhs == complex<Real>{-1, 0} ? MNL_SYM("DivisionByZero") : MNL_SYM("Overflow"));
      return res;
   }
   template<typename Real> MNL_INLINE static inline enable_same<Real, double, string> _str(const complex<Real> &rhs) {
      char res[sizeof "+1.7976931348623157e+308,+1.7976931348623157e+308"];
      return sprintf(res, "%.16e,%.16e", real(rhs), imag(rhs)), res;
   }
   template<typename Real> MNL_INLINE static inline enable_same<Real, float,  string> _str(const complex<Real> &rhs) {
      char res[sizeof "+3.40282347e+38,+3.40282347e+38"];
      return sprintf(res, "%.8e,%.8e", real(rhs), imag(rhs)), res;
   }
} // namespace rusini
# define RUSINI_M(DAT) \
   template<> auto mnl::box<DAT>::invoke(val &&self, const sym &op, int argc, val argv[], val *)->val { \
   switch (MNL_DISP("+", "-", "*", "/", "Neg", "~", "Conj", "==", "<>", "Order", "Re", "Im", "Abs", "Arg", "Exp", "Log", "Log10", \
      "Sqrt", "Pow", "Sin", "Cos", "Tan", "Asin", "Acos", "Atan", "Sinh", "Cosh", "Tanh", "Asinh", "Acosh", "Atanh", "Clone", "Str")[op]) { \
   case  0: return self.default_invoke(op, argc, argv); \
      using ::std::move; namespace rus = ::rusini; using ::std::signbit; \
   case  1: /* + */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_add(dat, cast<const DAT &>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_add(dat, cast<DAT &>(argv[0])), move(argv[0]); \
      return rus::_add(dat, cast<const DAT &>(argv[0])); \
   case  2: /* - */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sub(dat, cast<const DAT &>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_sub(dat, cast<DAT &>(argv[0])), move(argv[0]); \
      return rus::_sub(dat, cast<const DAT &>(argv[0])); \
   case  3: /* * */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_mul(dat, cast<const DAT &>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_mul(dat, cast<DAT &>(argv[0])), move(argv[0]); \
      return rus::_mul(dat, cast<const DAT &>(argv[0])); \
   case  4: /* / */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_div(dat, cast<const DAT &>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_div(dat, cast<DAT &>(argv[0])), move(argv[0]); \
      return rus::_div(dat, cast<const DAT &>(argv[0])); \
   case  5: case  6: /* Neg/~ */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_neg(dat), move(self); \
      return rus::_neg(dat); \
   case  7: /* Conj */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_conj(dat), move(self); \
      return rus::_conj(dat); \
   case  8: /* == */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return  MNL_LIKELY(test<DAT>(argv[0])) && dat == cast<const DAT &>(argv[0]); \
   case  9: /* <> */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return !MNL_LIKELY(test<DAT>(argv[0])) || dat != cast<const DAT &>(argv[0]); \
   case 10: /* Order */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      return \
         signbit(real(dat)) > signbit(real(cast<const DAT &>(argv[0]))) || real(dat) < real(cast<const DAT &>(argv[0])) ? -1 : \
         signbit(real(dat)) < signbit(real(cast<const DAT &>(argv[0]))) || real(dat) > real(cast<const DAT &>(argv[0])) ? +1 : \
         signbit(imag(dat)) > signbit(imag(cast<const DAT &>(argv[0]))) || imag(dat) < imag(cast<const DAT &>(argv[0])) ? -1 : \
         signbit(imag(dat)) < signbit(imag(cast<const DAT &>(argv[0]))) || imag(dat) > imag(cast<const DAT &>(argv[0])) ? +1 : 0; \
   case 11: /* Re */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_real(dat); \
   case 12: /* Im */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_imag(dat); \
   case 13: /* Abs */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_abs(dat); \
   case 14: /* Arg */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_arg(dat); \
   case 15: /* Exp */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_exp(dat), move(self); \
      return rus::_exp(dat); \
   case 16: /* Log */ \
      if (MNL_LIKELY(argc == 0)) return MNL_UNLIKELY(rc() > 1) ? rus::_log(dat) : (dat = rus::_log(dat), move(self)); \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_log(dat, cast<const DAT &>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_log(dat, cast<DAT &>(argv[0])), move(argv[0]); \
      return rus::_log(dat, cast<const DAT &>(argv[0])); \
   case 17: /* Log10 */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_log10(dat), move(self); \
      return rus::_log10(dat); \
   case 18: /* Sqrt */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sqrt(dat), move(self); \
      return rus::_sqrt(dat); \
   case 19: /* Pow */ \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_UNLIKELY(!test<DAT>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_pow(dat, cast<const DAT &>(argv[0])), move(self); \
      if (MNL_LIKELY(argv[0].rc() == 1)) return cast<DAT &>(argv[0]) = rus::_pow(dat, cast<DAT &>(argv[0])), move(argv[0]); \
      return rus::_pow(dat, cast<const DAT &>(argv[0])); \
   case 20: /* Sin */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sin(dat), move(self); \
      return rus::_sin(dat); \
   case 21: /* Cos */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_cos(dat), move(self); \
      return rus::_cos(dat); \
   case 22: /* Tan */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_tan(dat), move(self); \
      return rus::_tan(dat); \
   case 23: /* Asin */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_asin(dat), move(self); \
      return rus::_asin(dat); \
   case 24: /* Acos */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_acos(dat), move(self); \
      return rus::_acos(dat); \
   case 25: /* Atan */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_atan(dat), move(self); \
      return rus::_atan(dat); \
   case 26: /* Sinh */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_sinh(dat), move(self); \
      return rus::_sinh(dat); \
   case 27: /* Cosh */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_cosh(dat), move(self); \
      return rus::_cosh(dat); \
   case 28: /* Tanh */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_tanh(dat), move(self); \
      return rus::_tanh(dat); \
   case 29: /* Asinh */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_asinh(dat), move(self); \
      return rus::_asinh(dat); \
   case 30: /* Acosh */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_acosh(dat), move(self); \
      return rus::_acosh(dat); \
   case 31: /* Atanh */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(rc() == 1)) return dat = rus::_atanh(dat), move(self); \
      return rus::_atanh(dat); \
   case 32: /* Clone */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return MNL_LIKELY(rc() == 1) ? move(self) : dat; \
   case 33: /* Str */ \
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      return rus::_str(dat); \
   }} \
// end # define RUSINI_M(DAT)
   RUSINI_M(::std::complex<double>) RUSINI_M(::std::complex<float>)
# undef RUSINI_M

namespace rusini { extern "C" mnl::code mnl_aux_ieee754_cmpx() {
   # define RUSINI_M(DAT, CONV) { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) { \
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv); \
      if (MNL_LIKELY(argc == 2)) { \
         if (MNL_UNLIKELY(!test<DAT::value_type>(argv[0])) || MNL_UNLIKELY(!test<DAT::value_type>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
         return DAT{cast<DAT::value_type>(argv[0]), cast<DAT::value_type>(argv[1])}; \
      } \
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
      if (MNL_LIKELY(test<long long>(argv[0]))) return (DAT)cast<long long>(argv[0]); \
      if (MNL_LIKELY(test<DAT::value_type>(argv[0]))) return (DAT)cast<DAT::value_type>(argv[0]); \
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch")); \
      \
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
      auto pos = cast<const string &>(argv[0]).find(','); \
      if (pos == string::npos) { \
         char *end; auto res = CONV(cast<const string &>(argv[0]).c_str(), &end); \
         if (MNL_UNLIKELY(*end)) MNL_ERR(MNL_SYM("SyntaxError")); \
         if (MNL_UNLIKELY(isinf(res))) MNL_ERR(MNL_SYM("Overflow")); \
         return (DAT)res; \
      } \
      \
      switch (cast<const string &>(argv[0])[pos + 1]) { \
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': \
         break; \
      case '+': case '-': \
         if (cast<const string &>(argv[0])[pos + 2] >= '0' && cast<const string &>(argv[0])[pos + 2] <= '9' || \
             cast<const string &>(argv[0])[pos + 2] == '.') break; \
         /*[[fallthrough]];*/ \
      default: \
         MNL_ERR(MNL_SYM("SyntaxError")); \
      } \
      char *end; DAT res; \
      res.real(CONV(cast<const string &>(argv[0]).c_str(), &end)); \
      if (MNL_UNLIKELY(*end != ',')) MNL_ERR(MNL_SYM("SyntaxError")); \
      res.imag(CONV(cast<const string &>(argv[0]).c_str() + pos + 1, &end)); \
      if (MNL_UNLIKELY(*end)) MNL_ERR(MNL_SYM("SyntaxError")); \
      if (MNL_UNLIKELY(isinf(res.real())) || MNL_UNLIKELY(isinf(res.imag()))) MNL_ERR(MNL_SYM("Overflow")); \
      return res; \
   }} \
   // end # define RUSINI_M(DAT, CONV)
   struct proc_Z64 RUSINI_M(complex<double>, strtod); struct proc_Z32 RUSINI_M(complex<float>, strtof);
   # undef RUSINI_M
   return expr_export{
      {"Z64", make_lit(proc_Z64{})},
      {"Z32", make_lit(proc_Z32{})},
   };
}}
