
   auto mnl::aux::optimize(expr_lit<> expr)->code {
      if (is<long long>(expr.value))
         return expr_lit{as<long long>(expr.value)};
      if (is<double>(expr.value))
         return expr_lit{as<double>(expr.value)};
      if (is<float>(expr.value))
         return expr_lit{as<float>(expr.value)};
      if (is<const sym &>(expr.value))
         return expr_lit{std::move(as<sym &>(expr.value))};
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
               return expr_op1{{}, _not{}, *arg0_p}; // ~T
            return expr_op1{{}, _not{}, std::move(expr.arg0)}; // ~E
         }
         if (target_p->value == MNL_SYM("Neg")) {
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_op1{{}, _neg{}, *arg0_p}; // Neg[T]
            return expr_op1{{}, _neg{}, std::move(expr.arg0)}; // Neg[E]
         }
         if (target_p->value == MNL_SYM("Abs")) {
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_op1{{}, _abs{}, *arg0_p}; // Abs[T]
            return expr_op1{{}, _abs{}, std::move(expr.arg0)}; // Abs[E]
         }
         return expr_apply1{{}, *target_p, std::move(expr.arg0)};
      }
      if (auto target_p = as_p<expr_lit<>>(expr.target)) {
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // L[T]
         return expr_apply1{{}, *target_p, std::move(expr.arg0)}; // L[E]
      }
      if (auto target_p = as_p<expr_tvar>(expr.target)) {
         if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // T[LK]
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // T[T]
         return expr_apply1{{}, *target_p, std::move(expr.arg0)}; // T[E]
      }
      if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0))
         return expr_apply1{{}, *target_p, *arg0_p}; // E[LK]
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
         return expr_apply1{{}, *target_p, *arg0_p}; // E[LK]
      return std::move(expr); // E
   }

   auto mnl::aux::optimize(expr_apply2<> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) {
         // (in)equality
         {  auto optimize = [&](auto op) MNL_INLINE{
               if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
                  if (is<std::string>(arg0_p->value)) {
                     if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                        return expr_apply2{{}, op, expr_lit{as<std::string>(arg0_p->value)}, *arg1_p}; // Ls*[L*; T]
                     return expr_apply2{{}, op, expr_lit{as<std::string>(arg0_p->value)}, std::move(expr.arg1)}; // Ls*[L*; E]
                  }
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[L; T]
                  return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // Ls[L; E]
               }
               if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[T; E]
               }
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; T]
               return expr_apply2{{}, op, std::move(expr.arg0), std::move(expr.arg1)}; // Ls*[E; E]
            };
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("=="))) return optimize(_eq{});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("<>"))) return optimize(_ne{});
         }
         // all numeric
         {  auto optimize = [&](auto op) MNL_INLINE{
               if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[L; T]
                  return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // Ls[L; E]
               }
               if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[T; E]
               }
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; T]
               return expr_apply2{{}, op, std::move(expr.arg0), std::move(expr.arg1)}; // Ls*[E; E]
            };
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("<" ))) return optimize(_lt {});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("<="))) return optimize(_le {});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM(">" ))) return optimize(_gt {});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM(">="))) return optimize(_ge {});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("+" ))) return optimize(_add{});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("-" ))) return optimize(_sub{});
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("*" ))) return optimize(_mul{});
         }
         // exclussive-or
         {  auto optimize = [&](auto op) MNL_INLINE{
               if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[L*; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[L*; E]
               }
               if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[L; T]
                  return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // Ls[L; E]
               }
               if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
                  if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; L*]
                  if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                     return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply2{{}, op, *arg0_p, *arg1_p}; // Ls*[T; T]
                  return expr_apply2{{}, op, *arg0_p, std::move(expr.arg1)}; // Ls*[T; E]
               }
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; L*]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                  return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                  return expr_apply2{{}, op, std::move(expr.arg0), *arg1_p}; // Ls*[E; T]
               return expr_apply2{{}, op, std::move(expr.arg0), std::move(expr.arg1)}; // Ls*[E; E]
            };
            if (MNL_UNLIKELY(target_p->value == MNL_SYM("Xor"))) return optimize(_xor{});
         }
         // assuming a binary operation with no observable side effects
         if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[L; T]
            return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // Ls[L; E]
         }
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L*]
            if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; L]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // Ls[T; T]
            return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // Ls[T; E]
         }
         if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L*]
         if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; L]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // Ls[E; T]
         return expr_apply2{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1)}; // Ls[E; E]
      }
      // assuming indexing into fast containers
      if (auto target_p = as_p<expr_lit<>>(expr.target)) {
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // L[T; Ls]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // L[T; T]
            return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // L[T; E]
         }
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // L[E; Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // L[E; T]
         return expr_apply2{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1)}; // L[E; E]
      }
      if (auto target_p = as_p<expr_tvar>(expr.target)) {
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // T[T; Ls]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply2{{}, *target_p, *arg0_p, *arg1_p}; // T[T; T]
            return expr_apply2{{}, *target_p, *arg0_p, std::move(expr.arg1)}; // T[T; E]
         }
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // T[E; Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply2{{}, *target_p, std::move(expr.arg0), *arg1_p}; // T[E; T]
         return expr_apply2{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1)}; // T[E; E]
      }
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply2{{}, std::move(expr.target), *arg0_p, *arg1_p}; // E[T; Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply2{{}, std::move(expr.target), *arg0_p, *arg1_p}; // E[T; T]
         return expr_apply2{{}, std::move(expr.target), *arg0_p, std::move(expr.arg1)}; // E[T; E]
      }
      if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
         return expr_apply2{{}, std::move(expr.target), std::move(expr.arg0), *arg1_p}; // E[E; Ls]
      if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
         return expr_apply2{{}, std::move(expr.target), std::move(expr.arg0), *arg1_p}; // E[T; T]
      return std::move(expr); // E[E; E]
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

