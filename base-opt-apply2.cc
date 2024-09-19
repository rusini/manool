// base-opt-apply2.cc -- base optimization rules

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


auto mnl::aux::optimize(expr_apply<2> expr)->code {
   if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) [[unlikely]] {
      // (in)equality, except where belongs to (*)
      {  const auto optimize = [&](auto op) MNL_INLINE->code{
            if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Ls; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) [[unlikely]] { // False == V, True <> V might be faster than Not[V]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) [[unlikely]] {
               if (is<std::string>(arg0_p->value)) [[unlikely]] {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                     return expr_apply{{}, op, expr_lit{as<std::string>(arg0_p->value)}, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, expr_lit{as<std::string>(arg0_p->value)}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               // part of (*)
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
               return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
            }
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Ls]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]] // for string
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?T; ?]
            }
            if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Ls]
            if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]] // for string
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?L]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?T]
            return expr_apply{{}, op, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?; ?]
         };
         switch (target_p->value) {
         case sym::id("=="): return optimize(op<sym::id("==")>);
         case sym::id("<>"): return optimize(op<sym::id("<>")>);
         }
      }
      // all numeric, except where belongs to (*)
      {  const auto optimize = [&](auto op) MNL_INLINE->code{
            if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) [[unlikely]] { // part of (*)
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
               return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
            }
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Ls]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?T; ?]
            }
            if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Ls]
            if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?T]
            return expr_apply{{}, op, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?; ?]
         };
         switch (target_p->value) {
         case sym::id("<" ): return optimize(op<sym::id("<" )>);
         case sym::id("<="): return optimize(op<sym::id("<=")>);
         case sym::id(">" ): return optimize(op<sym::id(">" )>);
         case sym::id(">="): return optimize(op<sym::id(">=")>);
         case sym::id("+" ): return optimize(op<sym::id("+" )>);
         case sym::id("-" ): return optimize(op<sym::id("-" )>);
         case sym::id("*" ): return optimize(op<sym::id("*" )>);
         }
      }
      // exclussive-or and plain and/or, except where belongs to (*)
      {  const auto optimize = [&](auto op) MNL_INLINE->code{
            if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) [[unlikely]] { // True.Xor[V] might be faster than Not[V]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
            }
            if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
               return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
            }
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] { // TODO: having a lit of any type T on the right indicates that for the left operand we do not need to check for full range of tags (like in the most general handler), if T is not supported by the op, then we check only for rep::_box, on the other hand, we are trying to speed up relatively cold code here (boxed rep, etc)
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Ls]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]] // TODO: A.Xor[True] also might be faster than Not[A] ??? nope, because only bool and u32 support Not
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?T]
               return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?T; ?]
            }
            if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Ls]
            if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
            if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
               return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?T]
            return expr_apply{{}, op, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?; ?]
         };
         switch (target_p->value) {
         case sym::id("Xor"): return optimize(op<sym::id("Xor")>);
         case sym::id( "&" ): return optimize(op<sym::id( "&" )>);
         case sym::id( "|" ): return optimize(op<sym::id( "|" )>);
         }
      }
      // assuming a binary operation with no observable side effects (*) // talking side effects because there's no opt for lit,lit, etc. assuming the operation returns, not producing side effects, and thus lit,lit should be treated as constant folding!!!
      if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Lt; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Lt; ?]
      }
      if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Lt; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Lt; ?]
      }
      if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Lt; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Lt; ?]
      }
      if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Ls; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Ls; ?]
      }
      if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Lt; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Lt; ?]
      }
      if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Lt; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Lt; ?]
      }
      if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?Lt; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?Lt; ?]
      }
      if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
         if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
         if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Ls]
         if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
         if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
         if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
         if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?T]
         return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?T; ?]
      }
      if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
      if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
      if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
      if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Ls]
      if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
      if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
      if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
      if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
      if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?T]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?; ?]
   } // if (auto target_p = as_p<expr_lit<const sym &>>(expr.target))
   // assuming indexing into fast containers
   if (auto target_p = as_p<expr_lit<>>(expr.target)) [[unlikely]] {
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?L[?T; ?Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?L[?T; ?T]
         return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?L[?T; ?]
      }
      if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?L[?; ?Ls]
      if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?L[?; ?T]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?L[?; ?]
   }
   if (auto target_p = as_p<expr_tvar>(expr.target)) [[unlikely]] {
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?T[?T; ?Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?T[?T; ?T]
         return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?T[?T; ?]
      }
      if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?T[?; ?Ls]
      if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?T[?; ?T]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?T[?; ?]
   }
   if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
      if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, std::move(expr.target), *arg0_p, *arg1_p, std::move(expr._loc)}; // ?[?T; ?Ls]
      if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
         return expr_apply{{}, std::move(expr.target), *arg0_p, *arg1_p, std::move(expr._loc)}; // ?[?T; ?T]
      return expr_apply{{}, std::move(expr.target), *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?[?T; ?]
   }
   if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1)) [[unlikely]]
      return expr_apply{{}, std::move(expr.target), std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?[?; ?Ls]
   if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
      return expr_apply{{}, std::move(expr.target), std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?[?; ?T]
   return expr; // ?[?; ?] (move semantics!)
} // auto mnl::aux::optimize(expr_apply<2> expr)->code
