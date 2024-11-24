// base.tcc -- base internal interfaces

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "manool.hh"

namespace MNL_AUX_UUID {

namespace aux { // TODO: think about expr_seq optimization

   template<typename Val = val> struct expr_lit: code::rvalue { // *lit*eral (immediate)
      [[no_unique_address]] Val value;
      template<bool = bool{}, bool = bool{}> MNL_INLINE const Val &execute() const noexcept { return value; } // sometimes stored as T &&
   };
   template<typename Val = decltype(nullptr)>
   expr_lit(code::rvalue, Val)->expr_lit<Val>;

   template<> struct expr_lit<decltype(nullptr)>: code::rvalue {
      expr_lit() = default;
      MNL_INLINE expr_lit(rvalue base, decltype(nullptr)) noexcept: rvalue(base) {}
      template<bool = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const noexcept { return {}; }
   };

   struct expr_tvar: code::lvalue { // *t*emporary *var*iable
      int offset;
      template<bool = bool{}, bool = bool{}> MNL_INLINE const val &execute() const { return tstack[offset]; }
   # if true
      MNL_INLINE void exec_in(val value) const { tstack[offset] = std::move(value); }
   # else
      MNL_INLINE void exec_in(const val &value) const { tstack[offset] = value; }
      MNL_INLINE void exec_in(val &&value) const { tstack[offset].swap(value); }
         // We deem a check followed by a not-taken, correctly predicted branch to be better for performance than an extra store-after-load;
         // besides, the latter might result in wrong destruction order. Benchmarking also supports this assumption.
   # endif
      MNL_INLINE val exec_out() const { return std::move(tstack[offset]); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<
      int Argc, class Target = code,
      typename Arg0 = std::conditional_t<Argc >= 1 && Argc <= 3, code, void>,
      typename Arg1 = std::conditional_t<Argc >= 2 && Argc <= 2, code, void>
   # if !MNL_LEAN
      , // TODO: the difference here is that Target and Arg* are required to be complete types at the point of mere mentioning of expr_apply,
        // not at the point of its instantiation! Also, mentioning expr_apply implies instantiation of the args (if needed)
      std::enable_if_t<Argc >= 0 && Argc <= 4,                                                                decltype(nullptr)> = decltype(nullptr){},
      std::enable_if_t<std::is_base_of_v<code::rvalue, Target>,                                               decltype(nullptr)> = decltype(nullptr){},
      std::is_base_of_v<appliable, std::remove_reference_t<decltype(std::declval<Target>().execute())>>,      decltype(nullptr)> = decltype(nullptr){},
      std::enable_if_t<Argc >= 1 && Argc <= 3 ? std::is_base_of_v<code::rvalue, Arg0> : std::is_void_v<Arg0>, decltype(nullptr)> = decltype(nullptr){},
      std::enable_if_t<Argc >= 2 && Argc <= 2 ? std::is_base_of_v<code::rvalue, Arg1> : std::is_void_v<Arg1>, decltype(nullptr)> = decltype(nullptr){}
   # endif
      >
   class expr_apply;


   // Application specialized for 0 arguments
   template<class Target>
   expr_apply(
      code::rvalue, Target, loc )->
   expr_apply< 0,
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>> >;
   template<class Target>
   struct expr_apply<0, Target>: code::rvalue {
      [[no_unique_address]] Target target; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&target = this->target.execute();
         try { return std::forward<decltype(target)>(target)(); }
         catch (...) { trace_execute(_loc); }
      }
   };

   // Application specialized for 1 argument
   template<class Target, class Arg0>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, loc )->
   expr_apply< 1,
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   | std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   expr_lit<Arg0>> >;
   template<class Target, class Arg0>
   struct expr_apply<1, Target, Arg0>:
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      [[no_unique_address]] Target target; [[no_unique_address]] Arg0 arg0; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&arg0 = this->arg0.execute(); auto &&target = this->target.execute();
         try { return std::forward<decltype(target)>(target)(std::forward<decltype(arg0)>(arg0)); }
         catch (...) { trace_execute(_loc); }
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // to aid DCE opt
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); val target = this->target.exec_out();
            try { return std::move(target).repl(std::forward<decltype(arg0)>(arg0), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_out(); // to aid DCE opt
         val argv_out[2 + 1];
         target.exec_in( [&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {arg0.execute()}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   // Application specialized for 2 arguments
   template<class Target, class Arg0, class Arg1>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, Arg1, loc )->
   expr_apply< 2,
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   | std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   expr_lit<Arg0>>,
      std::conditional_t<std::is_base_of_v<code, Arg1>   | std::is_base_of_v<code::rvalue, Arg1>,   Arg1,   expr_lit<Arg1>> >;
   template<class Target, class Arg0, class Arg1>
   struct expr_apply<2, Target, Arg0, Arg1>:
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      [[no_unique_address]] Target target; [[no_unique_address]] Arg0 arg0; [[no_unique_address]] Arg1 arg1; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); auto &&target = this->target.execute();
         try { return std::forward<decltype(target)>(target)(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1)); }
         catch (...) { trace_execute(_loc); }
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // to aid DCE opt
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); val target = this->target.exec_out();
            try { return std::move(target).repl(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_out(); // to aid DCE opt
         val argv_out[3 + 1];
         target.exec_in( [&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {arg0.execute(), arg1.execute()}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   // Application specialized for 3 arguments
   template<class Target, class Arg0>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, code, code, loc )->
   expr_apply< 3,
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   | std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   code >;
   template<class Target, class Arg0>
   struct expr_apply<3, Target, Arg0>:
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      [[no_unique_address]] Target target; [[no_unique_address]] Arg0 a0; code a1, a2; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&a0 = this->a0.execute(); val a1 = this->a1.execute(), a2 = this->a2.execute(); auto &&target = this->target.execute();
         try { return std::forward<decltype(target)>(target)(std::forward<a0>(a0), std::move(a1), std::move(a2)); }
         catch (...) { trace_execute(_loc); }
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // to aid DCE opt
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), std::forward<Val>(value)}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_out(); // to aid DCE opt
         val argv_out[4 + 1];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {a0.execute(), a1.execute(), a2.execute()}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   // Application specialized for 4 arguments
   template<class Target>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, code, code, code, code, loc )->
   expr_apply< 4,
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>> >;
   template<class Target>
   struct expr_apply<4, Target>:
      std::conditional_t<std::is_base_of_v<code, Target> | std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      [[no_unique_address]] Target target; code a0, a1, a2, a3; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}; auto &&target = this->target.execute();
         try { return std::forward<decltype(target)>(target)(std::size(argv), argv); }
         catch (...) { trace_execute(_loc); }
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // to aid DCE opt
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), std::forward<Val>(value)}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<code::lvalue, expr_apply>) return rvalue::exec_out(); // to aid DCE opt
         val argv_out[5 + 1];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<class Dest = code, class Src = code, std::enable_if_t<
      std::is_base_of_v<code, Dest> | std::is_base_of_v<code::lvalue, Dest> &&
      std::is_base_of_v<code, Src>  | std::is_base_of_v<code::rvalue, Src>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_set: code::rvalue {
      [[no_unique_address]] Dest dest; [[no_unique_address]] Src src;
      template<bool = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const { dest.exec_in(src.execute()); return {}; }
   };
   template<class Dest, class Src> expr_set(code::rvalue, Dest, Src, loc)->expr_set<Dest, Src>;

   template<class Op, class Src = code, bool Rhs = bool{}, std::enable_if_t<
      std::is_base_f_v<appliable, Op> &&
      std::is_base_of_v<code, Src> | std::is_base_of_v<code::rvalue, Src>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_update;
   template<class Op, class Src, std::enable_if_t<!std::is_base_f_v<appliable, Src>, decltype(nullptr)> = decltype(nullptr){}>
      expr_update(code::rvalue, expr_tvar, Op, Src, loc)->expr_update<Op, Src>;
   template<class Op, class Src, std::enable_if_t< std::is_base_f_v<appliable, Op>,  decltype(nullptr)> = decltype(nullptr){}>
      expr_update(code::rvalue, expr_tvar, Src, Op, loc)->expr_update<Op, Src, true>;
   template<class Op, class Src>
   struct expr_update<Op, Src>: code::rvalue {
      expr_tvar dest; [[no_unique_address]] Op op; [[no_unique_address]] Src src; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const { // relies on PRE optimization
         dest.exec_in([&]() MNL_INLINE{
            const val &lhs = dest.execute(); auto &&rhs = src.execute();
            try { return op(lhs, std::forward<decltype(rhs)>(rhs)); } catch (...) { trace_execute(_loc); }
         }() );
         return {};
      }
   };
   template<class Op, class Src>
   struct expr_update<Op, Src, true>: code::rvalue {
      expr_tvar dest; [[no_unique_address]] Src src; [[no_unique_address]] Op op; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const { // relies on PRE optimization
         dest.exec_in([&]() MNL_INLINE{
            auto &&lhs = src.execute(); const val &rhs = dest.execute();
            try { return op(std::forward<decltype(lhs)>(lhs), rhs); } catch (...) { trace_execute(_loc); }
         }() );
         return {};
      }
   };

   template<class Dest = code> struct expr_move: code::rvalue {
      [[no_unique_address]] Dest dest;
      template<bool = bool{}, bool = bool{}> MNL_INLINE val execute() const { return dest.exec_out(); }
   };
   template<class Dest> expr_move(code::rvalue, Dest)->expr_move<Dest>;

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   struct _expr_ifelse_misc { code body1, body2; };
   template<class Cond = code, std::enable_if_t<
      std::is_base_of_v<code, Cond> | std::is_base_of_v<code::rvalue, Cond>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_ifelse: code::lvalue {
      [[no_unique_address]] Cond cond; _expr_ifelse_misc _; loc _loc;
   public:
      template<bool fast_sig = bool{}, bool nores = bool{}> MNL_INLINE auto execute() const {
         const val &cond = this->cond.execute();
         if (MNL_LIKELY(op<sym::id<"==">>(true,  cond))) return _.body1.execute<fast_sig, nores>();
         if (MNL_LIKELY(op<sym::id<"==">>(false, cond))) return _.body2.execute<fast_sig, nores>();
         MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         const val &cond = this->cond.execute();
         if (MNL_LIKELY(op<sym::id<"==">>(true,  cond))) return _.body1.exec_in(std::forward<Val>(value));
         if (MNL_LIKELY(op<sym::id<"==">>(false, cond))) return _.body2.exec_in(std::forward<Val>(value));
         MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
      }
   public:
      MNL_INLINE val exec_out() const {
         const val &cond = this->cond.execute();
         if (MNL_LIKELY(op<sym::id<"==">>(true,  cond))) return _.body1.exec_out();
         if (MNL_LIKELY(op<sym::id<"==">>(false, cond))) return _.body2.exec_out();
         MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return _.body1.is_lvalue() && _.body2.is_lvalue(); }
   };
   template<class Cond> expr_ifelse(code::lvalue, Cond, _expr_ifelse_misc, loc)->expr_ifelse<Cond>;
   template<class Cond> expr_ifelse(code::lvalue, Cond, code, code, loc)->expr_ifelse<Cond>;

   struct _expr_if_misc { code body; };
   template<class Cond = code, std::enable_if_t<
      std::is_base_of_v<code, Cond> | std::is_base_of_v<code::rvalue, Cond>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_if: code::rvalue {
      [[no_unique_address]] Cond cond; _expr_if_misc _; loc _loc;
   public:
      template<bool fast_sig = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const {
         const val &cond = this->cond.execute();
         if (MNL_LIKELY(op<sym::id<"==">>(true,  cond))) return _.body.execute<fast_sig, true>();
         if (MNL_LIKELY(op<sym::id<"==">>(false, cond))) return {};
         MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
      }
   };
   template<class Cond> expr_if(code::rvalue, Cond, _expr_if_misc, loc)->expr_if<Cond>;
   template<class Cond> expr_if(code::rvalue, Cond, code, loc)->expr_if<Cond>;

   struct _expr_and_misc { code arg1; };
   template<class Arg0 = code, std::enable_if_t<
      std::is_base_of_v<code, Arg0> | std::is_base_of_v<code::rvalue, Arg0>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_and: code::rvalue {
      [[no_unique_address]] Arg0 cond; _expr_and_misc _; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE val execute() const {
         const val &arg0 = cond.execute();
         if (MNL_UNLIKELY(op<sym::id<"==">>(false, arg0)))
            return false;
         if (MNL_LIKELY(op<sym::id<"<>">>(true, arg0))) {
            auto &&arg1 = _.arg1.execute();
            try { return op<sym::id("&")>(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1)); }
            catch (...) { trace_execute(_loc); }
         }
         return [&]() MNL_INLINE{ // RVO
            val arg1 = _.arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!is<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            return arg1;
         }();
      }
   };
   template<class Arg0> expr_and(code::rvalue, Arg0, _expr_and_misc, loc)->expr_and<Arg0>;
   template<class Arg0> expr_and(code::rvalue, Arg0, code, loc)->expr_and<Arg0>;

   struct _expr_or_misc { code arg1; };
   template<class Arg0 = code, std::enable_if_t<
      std::is_base_of_v<code, Arg0> | std::is_base_of_v<code::rvalue, Arg0>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_or: code::rvalue {
      [[no_unique_address]] Arg0 cond; _expr_or_misc _; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE val execute() const {
         const val &arg0 = cond.execute();
         if (MNL_UNLIKELY(op<sym::id<"==">>(true, arg0)))
            return true;
         if (MNL_LIKELY(op<sym::id<"<>">>(false, arg0))) {
            auto &&arg1 = _.arg1.execute();
            try { return op<sym::id("|")>(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1)); }
            catch (...) { trace_execute(_loc); }
         }
         return [&]() MNL_INLINE{ // RVO
            val arg1 = _.arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!is<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            return arg1;
         }();
      }
   };
   template<class Arg0> expr_or(code::rvalue, Arg0, _expr_or_misc, loc)->expr_or<Arg0>;
   template<class Arg0> expr_or(code::rvalue, Arg0, code, loc)->expr_or<Arg0>;

   struct _expr_while_misc { code body; };
   template<class Cond = code, std::enable_if_t<
      std::is_base_of_v<code, Cond> | std::is_base_of_v<code::rvalue, Cond>,
      decltype(nullptr) > = decltype(nullptr){}>
   struct expr_while: code::rvalue {
      [[no_unique_address]] Cond cond; _expr_while_misc _; loc _loc;
   public:
      template<bool fast_sig = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const {
         for (;;) {
            const val &cond = this->cond.execute();
            if (MNL_LIKELY(op<sym::id<"==">>(true, cond))) {
               _.body.execute<fast_sig, true>();
               if (fast_sig && MNL_UNLIKELY(sig_state.first)) return {};
               continue;
            }
            if (MNL_LIKELY(op<sym::id<"==">>(false, cond))) return {};
            MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         }
      }
   };
   template<class Cond> expr_while(code::rvalue, Cond, _expr_while_misc, loc)->expr_while<Cond>;
   template<class Cond> expr_while(code::rvalue, Cond, code, loc)->expr_while<Cond>;



   template<typename Key = code> struct expr_on {
      MNL_RVALUE()
      Key key; code trap, body; loc _loc;
   public:
      MNL_INLINE val execute(bool fast_sig = false) const {
         auto &&key = this->key.execute();
         if (MNL_UNLIKELY(!test<sym>(key))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         auto res = [&]()->val{ try { return body.execute(true); } // res for NRVO
            catch (decltype(sig_state) &sig) { sig_state = move(sig); }
            catch (stack_overflow &)         { sig_state = {MNL_SYM("StackOverflow"), {}}; }
            catch (heap_exhausted &)         { sig_state = {MNL_SYM("HeapExhausted"), {}}; }
            catch (std::bad_alloc &)         { sig_state = {MNL_SYM("LimitExceeded"), {}}; }
            catch (std::system_error &)      { sig_state = {MNL_SYM("SystemError"),   {}}; }
            return {};
         }();
         if (MNL_LIKELY(!sig_state.first)) return res; // no signal
         if (MNL_UNLIKELY(sig_state.first == cast<const sym &>(key))) { // caught!
            sig_state.first = {}; auto arg = move(sig_state.second); sig_trace.clear();
            tmp_stk.push_back(arg); struct _ { MNL_INLINE ~_() { tmp_stk.pop_back(); } } _;
            res = trap.execute(fast_sig); return res;
         }
         if (MNL_LIKELY(fast_sig)) return res; // fast propagation
         auto sig = move(sig_state); sig_state.first = {}; throw move(sig); // slow propagation
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_on<>>(rhs) &&
            aux::match(cast<const expr_on<> &>(rhs).key,  key)  &&
            aux::match(cast<const expr_on<> &>(rhs).trap, trap) &&
            aux::match(cast<const expr_on<> &>(rhs).body, body) &&
            (_loc = cast<const expr_on<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_on &);
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   class object: public _record<> {
   public:
      shared_ptr<const _record<>> methods; val cleanup;
   public:
      MNL_INLINE object(const object &rhs) noexcept: _record<>(rhs), methods(rhs.methods), cleanup(rhs.cleanup) {}
      MNL_INLINE object(object &&rhs) noexcept: _record<>(move(rhs)), methods(move(rhs.methods)), cleanup(move(rhs.cleanup)) {}
      ~object();
   public:
      MNL_INLINE object(const record_descr &descr, const val &cleanup, const shared_ptr<const _record<>> &methods) noexcept
         : _record<>{descr}, cleanup(cleanup), methods(methods) {}
      MNL_INLINE inline object(_record<> &&, shared_ptr<const _record<>> &&) noexcept;
   private:
      MNL_INLINE inline val invoke(val &&, const sym &, int argc, val [], val *);
      friend box<object>;
   };

   struct expr_att { MNL_LVALUE(obj.is_lvalue())
      code obj, att; loc _loc;
   public:
      MNL_INLINE val execute(bool) const {
         auto obj = this->obj.execute(), att = this->att.execute();
         if (MNL_UNLIKELY(!test<sym>(att)))
            MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(!test<object>(obj)) || MNL_UNLIKELY(!cast<const object &>(obj).has(cast<const sym &>(att))))
            MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
         return cast<const object &>(obj)[cast<const sym &>(att)];
      }
      MNL_INLINE void exec_in(val &&value) const {
         auto obj = this->obj.exec_out(), att = this->att.execute();
         if (MNL_UNLIKELY(!test<sym>(att)))
            MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(!test<object>(obj)) || MNL_UNLIKELY(!cast<const object &>(obj).has(cast<const sym &>(att))))
            MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
         if (MNL_UNLIKELY(obj.rc() != 1)) obj = cast<const object &>(obj);
         cast<object &>(obj)[cast<const sym &>(att)] = move(value);
         this->obj.exec_in(move(obj));
      }
      MNL_INLINE val exec_out() const {
         auto obj = this->obj.exec_out(), att = this->att.execute();
         if (MNL_UNLIKELY(!test<sym>(att)))
            MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(!test<object>(obj)) || MNL_UNLIKELY(!cast<const object &>(obj).has(cast<const sym &>(att))))
            MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
         if (MNL_UNLIKELY(obj.rc() != 1)) obj = cast<const object &>(obj);
         auto res = move(cast<object &>(obj)[cast<const sym &>(att)]);
         this->obj.exec_in(move(obj));
         return res;
      }
   private:
      MNL_INLINE bool match(const code &) { return {}; }
      friend bool aux::match<>(const code &, expr_att &);
   };
   struct expr_att_tmp_sym { MNL_LVALUE(true)
      expr_tmp obj; expr_lit<const sym &> att; loc _loc;
   public:
      MNL_INLINE val execute(bool) const {
         auto &obj = tmp_stk[tmp_frm + this->obj.off];
         if (MNL_UNLIKELY(!test<object>(obj)) || MNL_UNLIKELY(!cast<const object &>(obj).has(att.value)))
            MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
         return cast<const object &>(obj)[att.value];
      }
      MNL_INLINE void exec_in(val &&value) const {
         auto &obj = tmp_stk[tmp_frm + this->obj.off];
         try {
            if (MNL_UNLIKELY(!test<object>(obj)) || MNL_UNLIKELY(!cast<const object &>(obj).has(att.value)))
               MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
            if (MNL_UNLIKELY(obj.rc() != 1)) obj = cast<const object &>(obj);
            cast<object &>(obj)[att.value] = move(value);
         } catch (...) { obj = {}; throw; }
      }
      MNL_INLINE val exec_out() const {
         auto &obj = tmp_stk[tmp_frm + this->obj.off];
         try {
            if (MNL_UNLIKELY(!test<object>(obj)) || MNL_UNLIKELY(!cast<const object &>(obj).has(att.value)))
               MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
            if (MNL_UNLIKELY(obj.rc() != 1)) obj = cast<const object &>(obj);
            return move(cast<object &>(obj)[att.value]);
         } catch (...) { obj = {}; throw; }
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_att>(rhs) &&
            aux::match(cast<const expr_att &>(rhs).obj, obj) &&
            aux::match(cast<const expr_att &>(rhs).att, att) &&
            (_loc = cast<const expr_att &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_att_tmp_sym &);
   };

} // namespace aux

   template<> MNL_INLINE inline box<aux::object>::~box() { while (!dat.items.empty()) dat.items.pop_back(); }
   extern template class box<aux::object>;

} // namespace MNL_AUX_UUID
