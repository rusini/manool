// base-opt-apply2.cc -- base optimization rules

/*    Copyright (C) 2018-2024 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "base.tcc"

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
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]] {
                  if (is<std::string>(arg1_p->value)) [[unlikely]]
                     return expr_apply{{}, op, *arg0_p, expr_lit{val::typed<std::string>{arg1_p->value}}, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
               }
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
            if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) [[unlikely]] {
               if (is<std::string>(arg1_p->value)) [[unlikely]]
                  return expr_apply{{}, op, *arg0_p, expr_lit{val::typed<std::string>{arg1_p->value}}, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
            }
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
            if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) [[unlikely]] {
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
               if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1)) [[unlikely]]
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
      // exclussive-or, except where belongs to (*)
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
         }
      }
      // plain and/or
      {  const auto optimize = [&](auto op) MNL_INLINE->code{
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
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]] {
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1)) [[unlikely]]
                  return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1)) [[unlikely]]
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
         case sym::id("&"): return optimize(op<sym::id("&")>);
         case sym::id("|"): return optimize(op<sym::id("|")>);
         }
      }
      // assuming a binary operation with no observable side effects (*) // talking side effects because there's no opt for lit,lit, etc. assuming the operation returns, not producing side effects, and thus lit,lit should be treated as constant folding!!!
      if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
      }
      if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) [[unlikely]] {
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1)) [[unlikely]]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
         return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
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
