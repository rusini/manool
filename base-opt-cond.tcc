// base-opt-cond.tcc

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "base.tcc"

namespace MNL_AUX_UUID { namespace aux {
   code optimize(MNL_EXPR<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<MNL_EXPR< expr_not<expr_tmp> >>  (res) ||
      match<MNL_EXPR< expr_not<code> >>      (res) ||
   # define MNL_M(OP) \
      match<MNL_EXPR< expr##OP<expr_lit<long long>,          expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<long long>,          code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<double>,             expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<double>,             code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<float>,              expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<float>,              code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<const sym &>,        expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<const sym &>,        code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<decltype(nullptr)>,  expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<decltype(nullptr)>,  code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<unsigned>,           expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<unsigned>,           code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<const string &>,     expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<const string &>,     code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<long long>> >>          (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<double>> >>             (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<float>> >>              (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<const sym &>> >>        (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<decltype(nullptr)>> >>  (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<unsigned>> >>           (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_lit<const string &>> >>     (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,                     code> >>                         (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<long long>> >>          (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<double>> >>             (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<float>> >>              (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<const sym &>> >>        (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<decltype(nullptr)>> >>  (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<unsigned>> >>           (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_lit<const string &>> >>     (res) || \
      match<MNL_EXPR< expr##OP<code,                         expr_tmp> >>                     (res) || \
      match<MNL_EXPR< expr##OP<code,                         code> >>                         (res) ||
      MNL_M(_eq) MNL_M(_ne)
   # undef MNL_M
   # define MNL_M(OP) \
      match<MNL_EXPR< expr##OP<expr_lit<long long>,  expr_tmp> >>             (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<long long>,  code> >>                 (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<double>,     expr_tmp> >>             (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<double>,     code> >>                 (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<float>,      expr_tmp> >>             (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<float>,      code> >>                 (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<unsigned>,   expr_tmp> >>             (res) || \
      match<MNL_EXPR< expr##OP<expr_lit<unsigned>,   code> >>                 (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,             expr_lit<long long>> >>  (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,             expr_lit<double>> >>     (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,             expr_lit<float>> >>      (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,             expr_lit<unsigned>> >>   (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,             expr_tmp> >>             (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,             code> >>                 (res) || \
      match<MNL_EXPR< expr##OP<code,                 expr_lit<long long>> >>  (res) || \
      match<MNL_EXPR< expr##OP<code,                 expr_lit<double>> >>     (res) || \
      match<MNL_EXPR< expr##OP<code,                 expr_lit<float>> >>      (res) || \
      match<MNL_EXPR< expr##OP<code,                 expr_lit<unsigned>> >>   (res) || \
      match<MNL_EXPR< expr##OP<code,                 expr_tmp> >>             (res) || \
      match<MNL_EXPR< expr##OP<code,                 code> >>                 (res) ||
      MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge)
   # undef MNL_M
   # define MNL_M(OP) \
      match<MNL_EXPR< expr##OP<expr_tmp,  expr_tmp> >>  (res) || \
      match<MNL_EXPR< expr##OP<expr_tmp,  code> >>      (res) || \
      match<MNL_EXPR< expr##OP<code,      expr_tmp> >>  (res) || \
      match<MNL_EXPR< expr##OP<code,      code> >>      (res) ||
      MNL_M(_xor)
   # undef MNL_M
      match<MNL_EXPR< expr_tmp >>  (res) ||
      match<MNL_EXPR< code >>      (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
}} // namespace MNL_AUX_UUID::aux
