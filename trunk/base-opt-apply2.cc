// base-opt-apply2.cc

/*    Copyright (C) 2018, 2019 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "base.tcc"

namespace MNL_AUX_UUID { namespace aux {
   code optimize(expr_apply2<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
   # define MNL_M(OP) \
      match<expr##OP< expr_lit<long long>,          expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<long long>,          code >>                         (res) || \
      match<expr##OP< expr_lit<double>,             expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<double>,             code >>                         (res) || \
      match<expr##OP< expr_lit<float>,              expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<float>,              code >>                         (res) || \
      match<expr##OP< expr_lit<const sym &>,        expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<const sym &>,        code >>                         (res) || \
      match<expr##OP< expr_lit<decltype(nullptr)>,  expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<decltype(nullptr)>,  code >>                         (res) || \
      match<expr##OP< expr_lit<unsigned>,           expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<unsigned>,           code >>                         (res) || \
      match<expr##OP< expr_lit<const string &>,     expr_tmp >>                     (res) || \
      match<expr##OP< expr_lit<const string &>,     code >>                         (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<long long> >>          (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<double> >>             (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<float> >>              (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<const sym &> >>        (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<decltype(nullptr)> >>  (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<unsigned> >>           (res) || \
      match<expr##OP< expr_tmp,                     expr_lit<const string &> >>     (res) || \
      match<expr##OP< expr_tmp,                     expr_tmp >>                     (res) || \
      match<expr##OP< expr_tmp,                     code >>                         (res) || \
      match<expr##OP< code,                         expr_lit<long long> >>          (res) || \
      match<expr##OP< code,                         expr_lit<double> >>             (res) || \
      match<expr##OP< code,                         expr_lit<float> >>              (res) || \
      match<expr##OP< code,                         expr_lit<const sym &> >>        (res) || \
      match<expr##OP< code,                         expr_lit<decltype(nullptr)> >>  (res) || \
      match<expr##OP< code,                         expr_lit<unsigned> >>           (res) || \
      match<expr##OP< code,                         expr_lit<const string &> >>     (res) || \
      match<expr##OP< code,                         expr_tmp >>                     (res) || \
      match<expr##OP< code,                         code >>                         (res) ||
      MNL_M(_eq) MNL_M(_ne)
   # undef MNL_M
   # define MNL_M(OP) \
      match<expr##OP< expr_lit<long long>,  expr_tmp >>             (res) || \
      match<expr##OP< expr_lit<long long>,  code >>                 (res) || \
      match<expr##OP< expr_lit<double>,     expr_tmp >>             (res) || \
      match<expr##OP< expr_lit<double>,     code >>                 (res) || \
      match<expr##OP< expr_lit<float>,      expr_tmp >>             (res) || \
      match<expr##OP< expr_lit<float>,      code >>                 (res) || \
      match<expr##OP< expr_lit<unsigned>,   expr_tmp >>             (res) || \
      match<expr##OP< expr_lit<unsigned>,   code >>                 (res) || \
      match<expr##OP< expr_tmp,             expr_lit<long long> >>  (res) || \
      match<expr##OP< expr_tmp,             expr_lit<double> >>     (res) || \
      match<expr##OP< expr_tmp,             expr_lit<float> >>      (res) || \
      match<expr##OP< expr_tmp,             expr_lit<unsigned> >>   (res) || \
      match<expr##OP< expr_tmp,             expr_tmp >>             (res) || \
      match<expr##OP< expr_tmp,             code >>                 (res) || \
      match<expr##OP< code,                 expr_lit<long long> >>  (res) || \
      match<expr##OP< code,                 expr_lit<double> >>     (res) || \
      match<expr##OP< code,                 expr_lit<float> >>      (res) || \
      match<expr##OP< code,                 expr_lit<unsigned> >>   (res) || \
      match<expr##OP< code,                 expr_tmp >>             (res) || \
      match<expr##OP< code,                 code >>                 (res) ||
      MNL_M(_lt) MNL_M(_le) MNL_M(_gt) MNL_M(_ge) MNL_M(_add) MNL_M(_sub) MNL_M(_mul)
   # undef MNL_M
   # define MNL_M(OP) \
      match<expr##OP< expr_tmp,  expr_tmp >>  (res) || \
      match<expr##OP< expr_tmp,  code >>      (res) || \
      match<expr##OP< code,      expr_tmp >>  (res) || \
      match<expr##OP< code,      code >>      (res) || \
      MNL_M(_xor)
   # undef MNL_M
      match<expr_apply2< expr_lit<const sym &>,  expr_lit<>,  expr_tmp >>               (res) ||
      match<expr_apply2< expr_lit<const sym &>,  expr_lit<>,  code >>                   (res) ||
      match<expr_apply2< expr_lit<const sym &>,  expr_tmp,    expr_lit<> >>             (res) ||
      match<expr_apply2< expr_lit<const sym &>,  expr_tmp,    expr_tmp >>               (res) ||
      match<expr_apply2< expr_lit<const sym &>,  expr_tmp,    code >>                   (res) ||
      match<expr_apply2< expr_lit<const sym &>,  code,        expr_lit<> >>             (res) ||
      match<expr_apply2< expr_lit<const sym &>,  code,        expr_tmp >>               (res) ||
      match<expr_apply2< expr_lit<const sym &>,  code,        code >>                   (res) ||
      match<expr_apply2< expr_lit<>,             expr_tmp,    expr_lit<const sym &> >>  (res) ||
      match<expr_apply2< expr_lit<>,             expr_tmp,    expr_tmp >>               (res) ||
      match<expr_apply2< expr_lit<>,             expr_tmp,    code >>                   (res) ||
      match<expr_apply2< expr_lit<>,             code,        expr_lit<const sym &> >>  (res) ||
      match<expr_apply2< expr_lit<>,             code,        expr_tmp >>               (res) ||
      match<expr_apply2< expr_lit<>,             code,        code >>                   (res) ||
      match<expr_apply2< expr_tmp,               expr_tmp,    expr_lit<const sym &> >>  (res) ||
      match<expr_apply2< expr_tmp,               expr_tmp,    expr_tmp >>               (res) ||
      match<expr_apply2< expr_tmp,               expr_tmp,    code >>                   (res) ||
      match<expr_apply2< expr_tmp,               code,        expr_lit<const sym &> >>  (res) ||
      match<expr_apply2< expr_tmp,               code,        expr_tmp >>               (res) ||
      match<expr_apply2< expr_tmp,               code,        code >>                   (res) ||
      match<expr_apply2< code,                   expr_tmp,    expr_lit<const sym &> >>  (res) ||
      match<expr_apply2< code,                   expr_tmp,    expr_tmp >>               (res) ||
      match<expr_apply2< code,                   expr_tmp,    code >>                   (res) ||
      match<expr_apply2< code,                   code,        expr_lit<const sym &> >>  (res) ||
      match<expr_apply2< code,                   code,        expr_tmp >>               (res) ||
      match<expr_apply2< code,                   code,        code >>                   (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
}} // namespace MNL_AUX_UUID::aux
