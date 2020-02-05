// base-opt-misc.cc

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "base.tcc"

namespace MNL_AUX_UUID { namespace aux {
   code optimize(expr_lit<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_lit< long long >>          (res) ||
      match<expr_lit< double >>             (res) ||
      match<expr_lit< float >>              (res) ||
      match<expr_lit< const sym & >>        (res) ||
      match<expr_lit< bool >>               (res) ||
      match<expr_lit< decltype(nullptr) >>  (res) ||
      match<expr_lit< unsigned >>           (res) ||
      match<expr_lit< val >>                (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_apply0<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_apply0< expr_lit<> >>  (res) ||
      match<expr_apply0< expr_tmp >>    (res) ||
      match<expr_apply0< code >>        (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_apply1<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_not< expr_tmp >>                                          (res) ||
      match<expr_not< code >>                                              (res) ||
      match<expr_neg< expr_tmp >>                                          (res) ||
      match<expr_neg< code >>                                              (res) ||
      match<expr_abs< expr_tmp >>                                          (res) ||
      match<expr_abs< code >>                                              (res) ||
      match<expr_apply1< expr_lit<const sym &>,  code >>                   (res) || // TODO: missing match<expr_apply1< expr_lit<const sym &>, expr_tmp >> ?
      match<expr_apply1< expr_lit<>,             expr_lit<const sym &> >>  (res) ||
      match<expr_apply1< expr_lit<>,             expr_tmp >>               (res) ||
      match<expr_apply1< expr_lit<>,             code >>                   (res) ||
      match<expr_apply1< expr_tmp,               expr_lit<const sym &> >>  (res) ||
      match<expr_apply1< expr_tmp,               expr_tmp >>               (res) ||
      match<expr_apply1< expr_tmp,               code >>                   (res) ||
      match<expr_apply1< code,                   expr_lit<const sym &> >>  (res) ||
      match<expr_apply1< code,                   expr_tmp >>               (res) ||
      match<expr_apply1< code,                   code >>                   (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_apply3<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_apply3< expr_lit<const sym &> >>  (res) ||
      match<expr_apply3< expr_lit<> >>             (res) ||
      match<expr_apply3< expr_tmp >>               (res) ||
      match<expr_apply3< code >>                   (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_apply4<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_apply4< expr_lit<const sym &> >>  (res) ||
      match<expr_apply4< expr_lit<> >>             (res) ||
      match<expr_apply4< expr_tmp >>               (res) ||
      match<expr_apply4< code >>                   (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_move<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_move< expr_tmp >>  (res) ||
      match<expr_move< code >>      (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_att expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_att_tmp_sym>  (res) ||
      match<expr_att>          (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
   code optimize(expr_on<> expr) {
      code res = move(expr);
   # ifdef MNL_WITH_OPTIMIZE
      match<expr_on< expr_lit<const sym &> >>  (res) ||
      match<expr_on< expr_tmp >>               (res) ||
      match<expr_on< code >>                   (res);
   # endif // # ifdef MNL_WITH_OPTIMIZE
      return res;
   }
}} // namespace MNL_AUX_UUID::aux
