
auto mnl::aux::optimize(expr_lit<> expr)->code {
   if (is<long long>(expr.value)) [[unlikely]]
      return expr_lit{{}, as<long long>(expr.value)};
   if (is<double>(expr.value)) [[unlikely]]
      return expr_lit{{}, as<double>(expr.value)};
   if (is<float>(expr.value)) [[unlikely]]
      return expr_lit{{}, as<float>(expr.value)};
   if (is<sym>(expr.value)) [[unlikely]]
      return expr_lit{{}, std::move(as<sym &>(expr.value))};
   if (is<bool>(expr.value)) [[unlikely]]
      return expr_lit{{}, as<bool>(expr.value)};
   if (is<decltype(nullptr)>(expr.value)) [[unlikely]]
      return expr_lit{{}, {}};
   if (is<unsigned>(expr.value)) [[unlikely]]
      return expr_lit{{}, as<unsigned>(expr.value)};
   return expr; // move semantics!
} // NOTE/TODO: the stored value will have the only possible type tag - "boxed" - could be used for (very advanced) optimization
auto mnl::aux::optimize(expr_apply<0> expr)->code {
   if (auto target_p = as_p<expr_lit<>>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr._loc)};
   if (auto target_p = as_p<expr_tvar>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr._loc)};
   return expr; // move semantics!
}
auto mnl::aux::optimize(expr_apply<1> expr)->code {
   if (auto target_p = as_p<expr_lit<const sym &>>(expr.target)) [[unlikely]] {
      // possibly trivial unary operations
      switch (target_p->value) {
      case sym::id("~"):
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
            return expr_apply{{}, op<sym::id("~")>,   *arg0_p, std::move(expr._loc)}; // ?Ls*[?T]
         return expr_apply{{}, op<sym::id("~")>,   std::move(expr.arg0), std::move(expr._loc)}; // ?Ls*[?]
      case sym::id("Neg"):
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
            return expr_apply{{}, op<sym::id("Neg")>, *arg0_p, std::move(expr._loc)}; // ?Ls*[?T]
         return expr_apply{{}, op<sym::id("Neg")>, std::move(expr.arg0), std::move(expr._loc)}; // ?Ls*[?]
      case sym::id("Abs"):
         if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
            return expr_apply{{}, op<sym::id("Abs")>, *arg0_p, std::move(expr._loc)}; // ?Ls*[?T]
         return expr_apply{{}, op<sym::id("Abs")>, std::move(expr.arg0), std::move(expr._loc)}; // ?Ls*[?]
      }
      // assuming a unary operation with no observable side effects
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
         return expr_apply{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?Ls[?T]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr._loc)}; // ?Ls[?]
   }
   // assuming indexing into fast containers
   if (auto target_p = as_p<expr_lit<>>(expr.target)) [[unlikely]] {
      if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) [[unlikely]]
         return expr_apply1{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?L[?Ls]
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
         return expr_apply1{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?L[?T]
      return expr_apply1{{}, *target_p, std::move(expr.arg0), std::move(expr._loc)}; // ?L[?]
   }
   if (auto target_p = as_p<expr_tvar>(expr.target)) [[unlikely]] {
      if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) [[unlikely]]
         return expr_apply1{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?T[?Ls]
      if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
         return expr_apply1{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?T[?T]
      return expr_apply1{{}, *target_p, std::move(expr.arg0), std::move(expr._loc)}; // ?T[?]
   }
   if (auto arg0_p = as_p<expr_lit<const sym &>>(expr.arg0)) [[unlikely]]
      return expr_apply1{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?[?Ls]
   if (auto arg0_p = as_p<expr_tvar>(expr.arg0)) [[unlikely]]
      return expr_apply1{{}, *target_p, *arg0_p, std::move(expr._loc)}; // ?[?T]
   return expr; // ?[?] (move semantics!)
}
auto mnl::aux::optimize(expr_apply<3> expr)->code {
   if (auto target_p = as_p<expr_lit<const sym *>>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr._loc)};
   if (auto target_p = as_p<expr_lit<>>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr._loc)};
   if (auto target_p = as_p<expr_tvar>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr._loc)};
   return expr; // move semantics!
}
auto mnl::aux::optimize(expr_apply<4> expr)->code {
   if (auto target_p = as_p<expr_lit<const sym *>>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3), std::move(expr._loc)};
   if (auto target_p = as_p<expr_lit<>>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3), std::move(expr._loc)};
   if (auto target_p = as_p<expr_tvar>(expr.target)) [[unlikely]]
      return expr_apply{{}, *target_p, std::move(expr.arg0), std::move(expr.arg1), std::move(expr.arg2), std::move(expr.arg3), std::move(expr._loc)};
   return expr; // move semantics!
}
auto mnl::aux::optimize(expr_move<> expr)->code {
   if (auto dest_p = as_p<expr_tvar>(expr.dest)) [[unlikely]]
      return expr_move{{}, *dest_p};
   return expr; // move semantics!
}

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
      } // if (auto target_p = as_p<expr_lit<const sym &>>(expr.target))
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
         case sym::id("|"): return optimize(op<sym::id("|" )>);
         }
      }
      // assuming a binary operation with no observable side effects (*)
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
         return expr_apply{{}, *target_p, *arg0_p, std::move(expr.arg1), std::move(expr._loc)}; // ?Ls[?T; ?E]
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
   }
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
}


namespace mnl::aux {
   template<class Src>  static MNL_INLINE inline bool match_src (expr_set<> &expr, const expr_tvar *dest_p, code &res) {
      if (auto src_p  = as_p<Src> (expr.src))  [[unlikely]] return res = expr_set{*dest_p, *src_p, std::move(expr._loc)}, true;
      return {};
   }
   template<class Src>  static MNL_INLINE inline bool match_src (expr_set<> &expr, code &res) {
      auto src_p  = as_p<Src> (expr.src);
      if (auto src_p  = as_p<Src> (expr.src))  [[unlikely]] return res = expr_set{std::move(expr.dest), *src_p, std::move(expr._loc)}, true;
      return {};
   }
   template<class Dest> static MNL_INLINE inline bool match_dest(expr_set<> &expr, code &res) {
      if (auto dest_p = as_p<Dest>(expr.dest)) [[unlikely]] return res = expr_set{*dest_p, std::move(expr.src), std::move(expr._loc)}, true;
      return {};
   }
}

auto mnl::aux::optimize(expr_set<> expr)->code {
   code res;
   if (auto dest_p = as_p<expr_tvar>(expr.dest)) [[unlikely]] {
      if (
         MNL_UNLIKELY(match_src< expr_lit<long long> >          (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<double> >             (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<float> >              (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<const sym &> >        (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<bool> >               (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<decltype(nullptr)> >  (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<unsigned> >           (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_lit<> >                   (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_apply<1, expr_lit<decltype(op<sym::id("~")>)>,    expr_tvar> >  (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_apply<1, expr_lit<decltype(op<sym::id("~")>)>,    code> >       (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_apply<1, expr_lit<decltype(op<sym::id("Neg")>)>,  expr_tvar> >  (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_apply<1, expr_lit<decltype(op<sym::id("Neg")>)>,  code> >       (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_apply<1, expr_lit<decltype(op<sym::id("Abs")>)>,  expr_tvar> >  (expr, dest_p, res)) ||
         MNL_UNLIKELY(match_src< expr_apply<1, expr_lit<decltype(op<sym::id("Abs")>)>,  code> >       (expr, dest_p, res)) ||
      bool{} ) return res;
      {  const auto optimize = [&](auto op) MNL_INLINE{
            typedef expr_lit<decltype(op)> op;
            return
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<long long>,            expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<long long>,            code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<double>,               expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<double>,               code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<float>,                expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<float>,                code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<const sym &>,          expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<const sym &>,          code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<bool>,                 expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<bool>,                 code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<decltype(nullptr)>,    expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<decltype(nullptr)>,    code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,             expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,             code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<const std::string &>,  expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<const std::string &>,  code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<long long>> >                                (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<double>> >                                   (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<float>> >                                    (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<const sym &>> >                              (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<decltype(nullptr)>> >                        (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<unsigned>> >                                 (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<const val::typed<const std::string &> &>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_tvar> >                                          (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  code> >                                               (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<long long>> >                                (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<double>> >                                   (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<float>> >                                    (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<const sym &>> >                              (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<decltype(nullptr)>> >                        (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<unsigned>> >                                 (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<const val::typed<const std::string &> &>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_tvar> >                                          (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       code> >                                               (expr, dest_p, res)) ||
            bool{};
         };
         if (
            MNL_UNLIKELY(optimize(op<sym::id("==")>)) || MNL_UNLIKELY(optimize(op<sym::id("<>")>)) ||
         bool{} ) return res;
      }
      {  const auto optimize = [&](auto op) MNL_INLINE{
            typedef expr_lit<decltype(op)> op;
            return
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<long long>,  expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<long long>,  code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<double>,     expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<double>,     code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<float>,      expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<float>,      code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,   expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,   code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<long long>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<double>> >     (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<float>> >      (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<unsigned>> >   (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_tvar> >            (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  code> >                 (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<long long>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<double>> >     (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<float>> >      (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<unsigned>> >   (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_tvar> >            (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       code> >                 (expr, dest_p, res)) ||
            bool{};
         };
         if (
            MNL_UNLIKELY(optimize(op<sym::id("<")>)) || MNL_UNLIKELY(optimize(op<sym::id("<=")>)) ||
            MNL_UNLIKELY(optimize(op<sym::id(">")>)) || MNL_UNLIKELY(optimize(op<sym::id(">=")>)) ||
            MNL_UNLIKELY(optimize(op<sym::id("+")>)) || MNL_UNLIKELY(optimize(op<sym::id("-" )>)) || MNL_UNLIKELY(optimize(op<sym::id("*")>)) ||
         bool{} ) return res;
      }
      {  const auto optimize = [&](auto op) MNL_INLINE{
            typedef expr_lit<decltype(op)> op;
            return
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<bool>,      expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<bool>,      code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,  expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,  code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<unsigned>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_tvar> >           (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  code> >                (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<unsigned>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_tvar> >           (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       code> >                (expr, dest_p, res)) ||
            bool{};
         };
         if (
            MNL_UNLIKELY(optimize(op<sym::id("Xor")>)) ||
         bool{} ) return res;
      }
      {  const auto optimize = [&](auto op) MNL_INLINE{
            typedef expr_lit<decltype(op)> op;
            return
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,  expr_tvar> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_lit<unsigned>,  code> >       (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_lit<unsigned>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  expr_tvar> >           (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, expr_tvar,  code> >                (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_lit<unsigned>> >  (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       expr_tvar> >           (expr, dest_p, res)) ||
               MNL_UNLIKELY(match_src< expr_apply<2, op, code,       code> >                (expr, dest_p, res)) ||
            bool{};
         };
         if (
            MNL_UNLIKELY(optimize(op<sym::id("&")>)) || MNL_UNLIKELY(optimize(op<sym::id("|")>)) ||
         bool{} ) return res;
      }
      if (
         MNL_UNLIKELY(match_src<expr_tvar>(expr, res)) ||
      bool{} ) return res;
      // otherwise
      res = expr_set{*dest_p, std::move(expr.src), std::move(expr._loc)};
      return res; // NRVO
   } // if (auto dest_p = as_p<expr_tvar>(expr.dest))
   if (
      MNL_UNLIKELY(match_dest< expr_apply<1, expr_tvar,  expr_lit<const sym &>> >  (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<1, expr_tvar,  expr_tvar> >              (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<1, expr_tvar,  code> >                   (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<1, code,       expr_lit<const sym &>> >  (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<1, code,       expr_tvar> >              (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<1, code,       code> >                   (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, expr_tvar,  expr_tvar,  expr_lit<const sym &>> >  (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, expr_tvar,  expr_tvar,  expr_tvar> >              (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, expr_tvar,  expr_tvar,  code> >                   (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, expr_tvar,  code,       expr_lit<const sym &>> >  (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, expr_tvar,  code,       expr_tvar> >              (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, expr_tvar,  code,       code> >                   (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, code,       expr_tvar,  expr_lit<const sym &>> >  (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, code,       expr_tvar,  expr_tvar> >              (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, code,       expr_tvar,  code> >                   (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, code,       code,       expr_lit<const sym &>> >  (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, code,       code,       expr_tvar> >              (expr, res)) ||
      MNL_UNLIKELY(match_dest< expr_apply<2, code,       code,       code> >                   (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<long long> >          (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<double> >             (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<float> >              (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<const sym &> >        (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<bool> >               (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<decltype(nullptr)> >  (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<unsigned> >           (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_lit<> >                   (expr, res)) ||
      MNL_UNLIKELY(match_src< expr_tvar >                    (expr, res)) ||
   bool{} ) return res;
   // otherwise
   res = std::move(expr);
   return res; // NRVO
}

namespace MNL_AUX_UUID::aux {
   template <class Cond, template<class> class Expr> static MNL_INLINE inline bool match(Expr<> &expr, code &res) {
      if (auto cond_p = as_p<Cond>(expr.cond)) [[unlikely]] return res = Expr{*cond_p, std::move(expr._), std::move(expr._loc)}, true;
      return {};
   }

   template<template<class> class Expr> static MNL_INLINE inline code optimize(Expr<>);
}

template<template<class> class Expr> MNL_INLINE inline auto mnl::aux::optimize(Expr<> &expr)->code {
   code res;
   if (
      MNL_UNLIKELY(match< expr_apply<1, expr_lit<decltype(op<sym::id("~")>)>, expr_tvar> >  (expr, res)) ||
      MNL_UNLIKELY(match< expr_apply<1, expr_lit<decltype(op<sym::id("~")>)>, code> >       (expr, res)) ||
      bool{} ) return res;
   {  const auto optimize = [&](auto op) MNL_INLINE{
         typedef expr_lit<decltype(op)> op;
         return
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<long long>,            expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<long long>,            code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<double>,               expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<double>,               code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<float>,                expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<float>,                code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<const sym &>,          expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<const sym &>,          code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<decltype(nullptr)>,    expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<decltype(nullptr)>,    code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<unsigned>,             expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<unsigned>,             code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<const std::string &>,  expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<const std::string &>,  code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<long long>> >                                (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<double>> >                                   (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<float>> >                                    (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<const sym &>> >                              (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<decltype(nullptr)>> >                        (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<unsigned>> >                                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_lit<const val::typed<const std::string &> &>> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_tvar> >                                          (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  code> >                                               (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<long long>> >                                (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<double>> >                                   (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<float>> >                                    (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<const sym &>> >                              (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<decltype(nullptr)>> >                        (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<unsigned>> >                                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_lit<const val::typed<const std::string &> &>> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_tvar> >                                          (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       code> >                                               (expr, res)) ||
         bool{};
      };
      if (
         MNL_UNLIKELY(optimize(op<sym::id("==")>)) || MNL_UNLIKELY(optimize(op<sym::id("<>")>)) ||
      bool{} ) return res;
   }
   {  const auto optimize = [&](auto op) MNL_INLINE{
         typedef expr_lit<decltype(op)> op;
         return
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<long long>,  expr_tvar> >            (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<long long>,  code> >                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<double>,     expr_tvar> >            (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<double>,     code> >                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<float>,      expr_tvar> >            (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<float>,      code> >                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<unsigned>,   expr_tvar> >            (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_lit<unsigned>,   code> >                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,            expr_lit<long long>> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,            expr_lit<double>> >     (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,            expr_lit<float>> >      (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,            expr_lit<unsigned>> >   (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,            expr_tvar> >            (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,            code> >                 (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,                 expr_lit<long long>> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,                 expr_lit<double>> >     (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,                 expr_lit<float>> >      (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,                 expr_lit<unsigned>> >   (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,                 expr_tvar> >            (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,                 code> >                 (expr, res)) ||
         bool{};
      };
      if (
         MNL_UNLIKELY(optimize(op<sym::id("<")>)) || MNL_UNLIKELY(optimize(op<sym::id("<=")>)) ||
         MNL_UNLIKELY(optimize(op<sym::id(">")>)) || MNL_UNLIKELY(optimize(op<sym::id(">=")>)) ||
      bool{} ) return res;
   }
   {  const auto optimize = [&](auto op) MNL_INLINE{
         return
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, expr_tvar,  code> >       (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       expr_tvar> >  (expr, res)) ||
            MNL_UNLIKELY(match< expr_apply<2, op, code,       code> >       (expr, res)) ||
         bool{};
      };
      if (
         MNL_UNLIKELY(optimize(op<sym::id("Xor")>)) ||
      bool{} ) return res;
   }
   // TODO: array/record access?
   // TODO: op invocation for other ops (some might return bool)
   // if we coved, say, if ? == ? ... (where ? represent kinda cold path), then we also should cover other situations that
   // do not lead to too much exponential code expansion!
   // even T * T might be covered, but involving despecialization of the operator
   if (
      MNL_UNLIKELY(match<expr_tvar>(expr, res)) ||
   bool{} ) return res;
   // otherwise
   res = std::move(expr);
   return res; // NRVO
}

auto mnl::aux::optimize(expr_if<> expr)->code { return optimize<expr_if>(expr); }
auto mnl::aux::optimize(expr_ifelse<> expr)->code { return optimize<expr_ifelse>(expr); }
auto mnl::aux::optimize(expr_and<> expr)->code { return optimize<expr_and>(expr); }
auto mnl::aux::optimize(expr_or<> expr)->code { return optimize<expr_or>(expr); }
auto mnl::aux::optimize(expr_while<> expr)->code { return optimize<expr_while>(expr); }

