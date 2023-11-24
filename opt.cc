
   auto mnl::aux::optimize(expr_lit<> expr)->code {
      if (is<long long>(expr.value))
         return expr_lit{{}, as<long long>(expr.value)};
      if (is<double>(expr.value))
         return expr_lit{{}, as<double>(expr.value)};
      if (is<float>(expr.value))
         return expr_lit{{}, as<float>(expr.value)};
      if (is<sym>(expr.value))
         return expr_lit{{}, std::move(as<sym &>(expr.value))};
      if (is<bool>(expr.value))
         return expr_lit{{}, as<bool>(expr.value)};
      if (is<decltype(nullptr)>(expr.value))
         return expr_lit{{}, {}};
      if (is<unsigned>(expr.value))
         return expr_lit{{}, as<unsigned>(expr.value)};
      return std::move(expr); // NOTE: the stored value will have the only possible type tag - "boxed" - could be used for (very advanced) optimization
   }
   auto mnl::aux::optimize(expr_apply<0> expr)->code {
      if (auto target_p = as_p<expr_lit<>>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr._loc)};
      if (auto target_p = as_p<expr_tvar>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr._loc)};
      return std::move(expr);
   }
   auto mnl::aux::optimize(expr_apply<1> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) {
         const auto optimize = [&](auto op) MNL_INLINE->code{
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_apply{{}, op, *arg0_p}; // ?Ls*[?T]
            return expr_apply{{}, op, std::move(expr.arg0)}; // ?Ls*[?]
         };
         switch (target_p->value) {
         # define MNL_FRAG(LAB) case MNL_SYMID(LAB): return optimize(op<MNL_SYMID(LAB)>{});
            MNL_FRAG("~") MNL_FRAG(Neg) MNL_FRAG(Abs)
         # undef MNL_FRAG
         }
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // Ls[T]
         return expr_apply1{{}, *target_p, std::move(expr.arg0)}; // Ls[E]

         switch (target_p->value) {
         case sym::id("~"):
         }

         // possibly trivial unary operations
         switch (target_p->value) {
         case sym::id("~"):
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_apply{{}, op<sym::id("~")>{},   *arg0_p}; // ?Ls*[?T]
            return expr_apply{{}, op<sym::id("~")>{},   std::move(expr.arg0)}; // ?Ls*[?]
         case sym::id("Neg"):
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_apply{{}, op<sym::id("Neg")>{}, *arg0_p}; // ?Ls*[?T]
            return expr_apply{{}, op<sym::id("Neg")>{}, std::move(expr.arg0)}; // ?Ls*[?]
         case sym::id("Abs"):
            if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
               return expr_apply{{}, op<sym::id("Abs")>{}, *arg0_p}; // ?Ls*[?T]
            return expr_apply{{}, op<sym::id("Abs")>{}, std::move(expr.arg0)}; // ?Ls*[?]
         }
         // assuming a unary operation with no observable side effects
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply{{}, *target_p, *arg0_p}; // ?Ls*[?T]
         return expr_apply{{}, *target_p, std::move(expr.arg0)}; // ?Ls*[?]
      }
      // assuming indexing into fast containers
      if (auto target_p = as_p<expr_lit<>>(expr.target)) {
         if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // L[Ls]
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // L[T]
         return expr_apply1{{}, *target_p, std::move(expr.arg0)}; // L[E]
      }
      if (auto target_p = as_p<expr_tvar>(expr.target)) {
         if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // T[Ls]
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
            return expr_apply1{{}, *target_p, *arg0_p}; // T[T]
         return expr_apply1{{}, *target_p, std::move(expr.arg0)}; // T[E]
      }
      if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0))
         return expr_apply1{{}, *target_p, *arg0_p}; // E[Ls]
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0))
         return expr_apply1{{}, *target_p, *arg0_p}; // E[T]
      return std::move(expr); // E[E]
   }

   auto mnl::aux::optimize(expr_apply<2> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) {
         // (in)equality, except where belongs to (*)
         {  const auto optimize = [&](auto op) MNL_INLINE->code{
               if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Ls; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) // False == V, True <> V might be faster than Not[V]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
                  if (is<std::string>(arg0_p->value)) {
                     if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                        return expr_apply{{}, op, expr_lit{as<std::string>(arg0_p->value)}, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                     return expr_apply{{}, op, expr_lit{as<std::string>(arg0_p->value)}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
                  }
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
                  return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
               }
               if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Ls]
                  if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) {
                     if (is<std::string>(arg1_p->value))
                        return expr_apply{{}, op, *arg0_p, expr_lit{val::typed<std::string>{arg1_p->value}}, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
                  }
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?T; ?]
               }
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Ls]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1)) {
                  if (is<std::string>(arg1_p->value))
                     return expr_apply{{}, op, *arg0_p, expr_lit{val::typed<std::string>{arg1_p->value}}, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
               }
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
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
               if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
                  return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
               }
               if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Ls]
                  if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?T; ?]
               }
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Ls]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?T]
               return expr_apply{{}, op, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?; ?]
            };
            switch (target_p->value) {
            case sym::id("<" ): return optimize(op<sym::id("<" )>{});
            case sym::id("<="): return optimize(op<sym::id("<=")>{});
            case sym::id(">" ): return optimize(op<sym::id(">" )>{});
            case sym::id(">="): return optimize(op<sym::id(">=")>{});
            case sym::id("+" ): return optimize(op<sym::id("+" )>{});
            case sym::id("-" ): return optimize(op<sym::id("-" )>{});
            case sym::id("*" ): return optimize(op<sym::id("*" )>{});
            }
         }
         // exclussive-or, except where belongs to (*)
         {  const auto optimize = [&](auto op) MNL_INLINE->code{
               if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) { // True.Xor[V] might be faster than Not[V]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?Lt; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?Lt; ?]
               }
               if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
                  return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
               }
               if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
                  if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<const sym *>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Ls]
                  if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?Lt]
                  if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                     return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
                  if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                     return expr_apply{{}, op, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls*[?T; ?T]
                  return expr_apply{{}, op, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?T; ?]
               }
               if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Ls]
               if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?Lt]
               if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
                  return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
               if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
                  return expr_apply{{}, op, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls*[?; ?T]
               return expr_apply{{}, op, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls*[?; ?]
            };
            switch (target_p->value) {
            case sym::id("Xor"): return optimize(op<sym::id("Xor")>{});
            }
         }
         // assuming a binary operation with no observable side effects (*)
         if (auto arg0_p = as_p<expr_lit<long long>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<double>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<float>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<bool>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<unsigned>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, expr_lit<>{arg0_p->value}, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_lit<>>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?L; ?T]
            return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?L; ?]
         }
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
            if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
            if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Ls]
            if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
            if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
            if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?Lt]
            if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?L]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?Ls[?T; ?T]
            return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?T; ?E]
         }
         if (auto arg1_p = as_p<expr_lit<long long>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
         if (auto arg1_p = as_p<expr_lit<double>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
         if (auto arg1_p = as_p<expr_lit<float>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Ls]
         if (auto arg1_p = as_p<expr_lit<bool>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
         if (auto arg1_p = as_p<expr_lit<decltype(nullptr)>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
         if (auto arg1_p = as_p<expr_lit<unsigned>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?Lt]
         if (auto arg1_p = as_p<expr_lit<>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?L]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?Ls[?; ?T]
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?; ?]
      }
      // assuming indexing into fast containers
      if (auto target_p = as_p<expr_lit<>>(expr.target)) {
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?L[?T; ?Ls]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?L[?T; ?T]
            return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?L[?T; ?]
         }
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?L[?; ?Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?L[?; ?T]
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?L[?; ?]
      }
      if (auto target_p = as_p<expr_tvar>(expr.target)) {
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
            if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?T[?T; ?Ls]
            if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
               return expr_apply{{}, *target_p, *arg0_p, *arg1_p, std::move(expr._loc)}; // ?T[?T; ?T]
            return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?T[?T; ?]
         }
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?T[?; ?Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply{{}, *target_p, std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?T[?; ?T]
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr._loc)}; // ?T[?; ?]
      }
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) {
         if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
            return expr_apply{{}, std::move(expr.target), *arg0_p, *arg1_p, std::move(expr._loc)}; // ?[?T; ?Ls]
         if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
            return expr_apply{{}, std::move(expr.target), *arg0_p, *arg1_p, std::move(expr._loc)}; // ?[?T; ?T]
         return expr_apply{{}, std::move(expr.target), *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?[?T; ?]
      }
      if (auto arg1_p = as_p<expr_lit<const sym &>>(expr.arg1))
         return expr_apply{{}, std::move(expr.target), std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?[?; ?Ls]
      if (auto arg1_p = as_p<expr_tvar>(expr.arg1))
         return expr_apply{{}, std::move(expr.target), std::move(expr.arg0), *arg1_p, std::move(expr._loc)}; // ?[?; ?T]
      return std::move(expr); // ?[?; ?]
   }

   auto mnl::aux::optimize(expr_apply<3> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym *>>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr._loc)};
      if (auto target_p = as_p<expr_lit<>>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr._loc)};
      if (auto target_p = as_p<expr_tvar>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr._loc)};
      return std::move(expr);
   }
   auto mnl::aux::optimize(expr_apply<4> expr)->code {
      if (auto target_p = as_p<expr_lit<const sym *>>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3), std::move(expr._loc)};
      if (auto target_p = as_p<expr_lit<>>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3), std::move(expr._loc)};
      if (auto target_p = as_p<expr_tvar>(expr.target))
         return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3), std::move(expr._loc)};
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

namespace mnl::aux {
   template <class Cond, template<class> class Expr> static MNL_INLINE inline bool match(Expr<> &expr, code &res) {
      if (auto cond_p = as_p<Cond>(expr.cond)) return res = Expr{*cond_p, std::move(expr._), std::move(expr._loc)}, true;
      return {};
   }
   template<template<class> class Expr> static MNL_INLINE inline code optimize(Expr<> &expr) {
      code res;
      {  const auto optimize = [&](auto op) MNL_INLINE{ return
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<long long>,            expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<long long>,            code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<double>,               expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<double>,               code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<float>,                expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<float>,                code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<const sym &>,          expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<const sym &>,          code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<bool>,                 expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<bool>,                 code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<decltype(nullptr)>,    expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<decltype(nullptr)>,    code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<unsigned>,             expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<unsigned>,             code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<const std::string &>,  expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<const std::string &>,  code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<long long>> >                                (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<double>> >                                   (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<float>> >                                    (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<const sym &>> >                              (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<decltype(nullptr)>> >                        (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<unsigned>> >                                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_lit<const val::typed<const std::string &> &>> >  (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,                      code> >                                               (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<long long>> >                                (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<double>> >                                   (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<float>> >                                    (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<const sym &>> >                              (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<decltype(nullptr)>> >                        (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<unsigned>> >                                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_lit<const val::typed<const std::string &> &>> >  (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           expr_tvar> >                                          (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                           code> >                                               (expr, res) ||
            bool{};
         };
         if (optimize(op<sym::id("==")>) || optimize(op<sym::id("<>")>)) return res;
      }
      {  const auto optimize = [&](auto op) MNL_INLINE{ return
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<long long>,  expr_tvar> >            (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<long long>,  code> >                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<double>,     expr_tvar> >            (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<double>,     code> >                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<float>,      expr_tvar> >            (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<float>,      code> >                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<unsigned>,   expr_tvar> >            (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<unsigned>,   code> >                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,            expr_lit<long long>> >  (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,            expr_lit<double>> >     (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,            expr_lit<float>> >      (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,            expr_lit<unsigned>> >   (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,            expr_tvar> >            (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,            code> >                 (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                 expr_lit<long long>> >  (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                 expr_lit<double>> >     (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                 expr_lit<float>> >      (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                 expr_lit<unsigned>> >   (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                 expr_tvar> >            (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                 code> >                 (expr, res) ||
            bool{};
         };
         if (optimize(op<sym::id("<")>) || optimize(op<sym::id("<=")>) || optimize(op<sym::id(">")>) || optimize(op<sym::id(">=")>)) return res;
      }
      {  const auto optimize = [&](auto op) MNL_INLINE{ return
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<bool>,      expr_tvar> >           (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<bool>,      code> >                (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<unsigned>,  expr_tvar> >           (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_lit<unsigned>,  code> >                (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,           expr_lit<unsigned>> >  (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,           expr_tvar> >           (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, expr_tvar,           code> >                (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                expr_lit<unsigned>> >  (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                expr_tvar> >           (expr, res) ||
            match< expr_apply<2, expr_lit<decltype(op)>, code,                code> >                (expr, res) ||
            bool{};
         };
         if (optimize(op<sym::id("Xor")>)) return res;
      }
      if (
         match< expr_apply<1, expr_lit<decltype(op<sym::id("~")>)>, expr_tvar> > (expr, res) ||
         match< expr_apply<1, expr_lit<decltype(op<sym::id("~")>)>, code> > (expr, res) ||
         match< expr_tvar > (expr, res) ) return res;
      res = std::move(expr);
      return res; // NRVO
   }
}

   auto mnl::aux::optimize(expr_if<> expr)->code { return optimize<expr_if>(expr); }
   auto mnl::aux::optimize(expr_ifelse<> expr)->code { return optimize<expr_ifelse>(expr); }
   auto mnl::aux::optimize(expr_and<> expr)->code { return optimize<expr_and>(expr); }
   auto mnl::aux::optimize(expr_or<> expr)->code { return optimize<expr_or>(expr); }
   auto mnl::aux::optimize(expr_while<> expr)->code { return optimize<expr_while>(expr); }

