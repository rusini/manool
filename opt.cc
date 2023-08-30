
   auto mnl::aux::optimize(expr_lit<> expr)->code {
      if (is<long long>(expr.value))
         return expr_lit{as<long long>(expr.value)};
      if (is<double>(expr.value))
         return expr_lit{as<double>(expr.value)};
      if (is<float>(expr.value))
         return expr_lit{as<float>(expr.value)};
      if (is<const sym &>(expr.value))
         return expr_lit<const sym &>{std::move(as<sym &>(expr.value))};
      if (is<bool>(expr.value))
         return expr_lit{as<bool>(expr.value)};
      if (is<decltype(nullptr)>(expr.value))
         return expr_lit{as<decltype(nullptr)>(expr.value)};
      if (is<unsigned>(expr.value))
         return expr_lit{as<unsigned>(expr.value)};
      return std::move(expr);
   }
   auto mnl::aux::optimize(expr_apply0<> expr)->code {
      if (auto target_p = as_p<expr_lit<>>(expr.target))
         return expr_apply0{{}, *target_p};
      if (auto target_p = as_p<expr_tvar>(expr.target))
         return expr_apply0{{}, *target_p};
      return std::move(expr);
   }
   auto mnl::aux::optimize(expr_apply1<> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) {
         if (target_p->value == MNL_SYM("~")) {
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_op1{{}, _not{}, *arg0_p};
            return expr_op1{{}, _not{}, std::move(expr.arg0)};
         }
         if (target_p->value == MNL_SYM("Neg")) {
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_op1{{}, _neg{}, *arg0_p};
            return expr_op1{{}, _neg{}, std::move(expr.arg0)};
         }
         if (target_p->value == MNL_SYM("Abs")) {
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_op1{{}, _abs{}, *arg0_p};
            return expr_op1{{}, _abs{}, std::move(expr.arg0)};
         }
         return expr_apply1{{}, *target_p, std::move(expr.arg0)};
      }
      if (auto target_p = as_p<expr_lit<>>(expr.target)) {
         if (auto arg0_p = as_p<expr_lit<sym>>(expr.arg0))
            return expr_apply1{{}, std::move(*target_p), std::move(*arg0_p)}; // WHY?
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, std::move(*target_p), std::move(*arg0_p)};
         return expr_apply1{{}, std::move(*target_p), std::move(expr.arg0)};
      }
      if (auto target_p = as_p<expr_tvar>(expr.target)) {
         if (auto arg0_p = as_p<expr_lit<sym>>(expr.arg0))
            return expr_apply1{{}, std::move(*target_p), std::move(*arg0_p)};
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, std::move(*target_p), std::move(*arg0_p)};
         return expr_apply1{{}, std::move(*target_p), std::move(expr.arg0)};
      }
      if (auto arg0_p = as_p<expr_lit<sym>>(expr.arg0))
         return expr_apply1{{}, std::move(expr.target), std::move(*arg0_p)};
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
         return expr_apply1{{}, std::move(expr.target), std::move(*arg0_p)};
      return std::move(expr);
   }

   template<class Op> static auto mnl::aux::optimize_op2(expr_apply2<> &&expr)->code {
      if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
         if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0))
         return expr_apply1{{}, std::move(*target_p), std::move(*arg0_p)};
      }
   }

   auto mnl::aux::optimize(expr_apply2<> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) {
        {   auto optimize = [&](auto op) MNL_INLINE{
               if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, std::move(*arg0_p), std::move(*arg1_p)};
                  return expr_apply2{{}, op, std::move(*arg0_p), std::move(expr.arg1)};
               }
               return expr_apply2{{}, op, std::move(expr.arg0), std::move(expr.arg1)};
            };
            if (target_p->value == MNL_SYM("==")) return optimize(_eq{}, std::move(expr), target_p);
            if (target_p->value == MNL_SYM("<>")) return optimize(_ne{}, std::move(expr), target_p);
         }
         if (target_p->value == MNL_SYM("<" )) return optimize2<_lt>(std::move(expr), target_p);
         if (target_p->value == MNL_SYM("<=")) return optimize2<_le>(std::move(expr), target_p);
         if (target_p->value == MNL_SYM(">" )) return optimize2<_gt>(std::move(expr), target_p);
         if (target_p->value == MNL_SYM(">=")) return optimize2<_ge>(std::move(expr), target_p);
         if (target_p->value == MNL_SYM("+" )) return optimize2<_ge>(std::move(expr), target_p);
         if (target_p->value == MNL_SYM("-" )) return optimize2<_ge>(std::move(expr), target_p);
         if (target_p->value == MNL_SYM("*" )) return optimize2<_ge>(std::move(expr), target_p);
         return expr_apply2{{}, std::move(*target_p), std::move(expr.arg0), std::move(expr.arg1)};
      }
      return std::move(expr);
   }

   auto mnl::aux::optimize(expr_apply3<> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym *>>(expr.target))
         return expr_apply3{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2)};
      if (auto target_p = as_p<expr_lit<>>(expr.target))
         return expr_apply3{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2)};
      if (auto target_p = as_p<expr_tvar>(expr.target))
         return expr_apply3{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2)};
      return std::move(expr);
   }
   auto mnl::aux::optimize(expr_apply4<> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym *>>(expr.target))
         return expr_apply4{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3)};
      if (auto target_p = as_p<expr_lit<>>(expr.target))
         return expr_apply4{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3)};
      if (auto target_p = as_p<expr_tvar>(expr.target))
         return expr_apply4{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3)};
      return std::move(expr);
   }

   auto mnl::aux::optimize(expr_set<> expr)->code {
      if (auto dest_p = as_p<expr_tvar>(expr.dest)) {
         if (auto src_p = as_p<expr_lit<long long>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<double>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<float>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<const sym &>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<bool>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<decltype(nullptr)>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<unsigned>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_lit<>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
         if (auto src_p = as_p<expr_apply2<op_not>>(expr.src))
            return expr_set{std::move(*dest_p), std::move(*src_p), std::move(expr._loc)};
      }
   }

