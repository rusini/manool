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

namespace aux {

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
         try { return op<sym::id("Apply")>(std::forward<decltype(target)>(target), 0, {}); }
         catch (...) { trace_execute(_loc); }
      }
   };

   // Application specialized for 1 argument
   template<typename Target, typename Arg0>
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
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value));
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); val target = this->target.exec_out();
            try { return std::move(target).repl(std::forward<decltype(arg0)>(arg0), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out();
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
   template<typename Target, typename Arg0, typename Arg1>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, Arg1, loc )->
   expr_apply< 2,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   || std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   expr_lit<Arg0>>,
      std::conditional_t<std::is_base_of_v<code, Arg1>   || std::is_base_of_v<code::rvalue, Arg1>,   Arg1,   expr_lit<Arg1>> >;
   template<class Target, class Arg0, class Arg1>
   struct expr_apply<2, Target, Arg0, Arg1>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
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
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value));
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); val target = this->target.exec_out();
            try { return std::move(target).repl(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out();
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
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, code, code, loc )->
   expr_apply< 3,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   || std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   expr_lit<Arg0>> >;
   template<class Target, class Arg0>
   struct expr_apply<3, Target, Arg0>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
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
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value));
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), std::forward<Val>(value)}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out();
         val argv_out[4];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute()}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv, argv_out); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   // Application specialized for 4 arguments
   template<typename Target>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, code, code, code, code, loc )->
   expr_apply< 4,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>> >;
   template<class Target>
   struct expr_apply<4, Target>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      [[no_unique_address]] Target target; code a0, a1, a2, a3; loc _loc; // arg types intentionally not parameterized
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}; auto &&target = this->target.execute();
         try { return std::forward<decltype(target)>(target)(std::size(argv), argv); } // argc != 0 check eliminated due to inlining
         catch (...) { trace_execute(_loc); }
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value));
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), std::forward<Val>(value)}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out();
         val argv_out[5];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}, target = this->target.exec_out();
            try { return std::move(target).repl(std::size(argv), argv, argv_out); }
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


   struct _expr_ifelse_branch { code body1, body2; };
   template<class Cond = code, std::enable_if_t<std::is_class_v<Cond>, decltype(nullptr)> = decltype(nullptr){}>
   struct expr_ifelse: code::lvalue {
      Cond cond; _expr_ifelse_misc _; loc _loc;
      static_assert(std::is_base_of_v<code, Cond> || std::is_base_of_v<rvalue, Cond>);
   public:
      template<bool fast_sig = bool{}, bool nores = bool{}> MNL_INLINE auto execute() const {
         auto &&cond = this->cond.execute();
         //if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         //return (as<bool>(cond) ? _.body1 : _.body2).execute<fast_sig, nores>();
         if (MNL_LIKELY(is<true>(cond)))  return _.body1.execute<fast_sig, nores>();
         if (MNL_LIKELY(is<false>(cond))) return _.body2.execute<fast_sig, nores>();
         MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         // !!! it's sometimes beneficial to male a copy/move of val before use if the object comes as a return value (may be marked as escaped!)
         // BEWARE of potentially aliased objects!!!
         // due to bitwise packaging of tag/value for bool clang fails to fully propagate conditions (but gcc does), also clang seems to "reuse" the return object (potentially aliased!)
         // we could ty to avoid ORing when constructing bools and instead ...
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (as<bool>(cond) ? _.body1 : _.body2).exec_in(std::forward<Val>(value));
      }
   public:
      MNL_INLINE val exec_out() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (as<bool>(cond) ? _.body1 : _.body2).exec_out();
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept {
         return _.body1.is_lvalue() && _.body2.is_lvalue();
      }
   };
   template<class Cond> expr_ifelse(code::lvalue, Cond, _expr_ifelse_misc, loc)->expr_ifelse<Cond>;
   template<class Cond> expr_ifelse(code::lvalue, Cond, code, code, loc)->expr_ifelse<Cond>;








   template<typename Cond = code> struct expr_ifelse {
      MNL_LVALUE(body1.is_lvalue() && body2.is_lvalue())
      Cond cond; code body1, body2; loc _loc;
   public:
      template<bool fast_sig = bool{}, bool nores = bool{}> MNL_INLINE auto execute() const {
         auto &&cond = this->cond.execute();
         //if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         //return (as<bool>(cond) ? _.body1 : _.body2).execute<fast_sig, nores>();
         if (MNL_LIKELY(cond == true))  return body1.execute<fast_sig, nores>();
         if (MNL_LIKELY(cond == false)) return body2.execute<fast_sig, nores>();
         MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         // !!! it's sometimes beneficial to male a copy/move of val before use if the object comes as a return value (may be marked as escaped!)
         // BEWARE of potentially aliased objects!!!
         // due to bitwise packaging of tag/value for bool clang fails to fully propagate conditions (but gcc does), also clang seems to "reuse" the return object (potentially aliased!)
         // we could ty to avoid ORing when constructing bools and instead ...
      }



      MNL_INLINE val execute(bool fast_sig = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (cast<bool>(cond) ? body1 : body2).execute(fast_sig);
      }
      MNL_INLINE void exec_in(val &&value) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (cast<bool>(cond) ? body1 : body2).exec_in(move(value));
      }
      MNL_INLINE val exec_out() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (cast<bool>(cond) ? body1 : body2).exec_out();
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_ifelse<>>(rhs) &&
            aux::match(cast<const expr_ifelse<> &>(rhs).cond,  cond)  &&
            aux::match(cast<const expr_ifelse<> &>(rhs).body1, body1) &&
            aux::match(cast<const expr_ifelse<> &>(rhs).body2, body2) &&
            (_loc = cast<const expr_ifelse<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_ifelse &);
   };

   template<typename Cond = code> struct expr_if {
      MNL_RVALUE()
      Cond cond; code body; loc _loc;
   public:
      MNL_INLINE decltype(nullptr) execute(bool fast_sig = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (cast<bool>(cond)) body.execute(fast_sig);
         return {};
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_if<>>(rhs) &&
            aux::match(cast<const expr_if<> &>(rhs).cond, cond) &&
            aux::match(cast<const expr_if<> &>(rhs).body, body) &&
            (_loc = cast<const expr_if<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_if &);
   };

   template<typename Arg0 = code> struct expr_and {
      MNL_RVALUE()
      Arg0 arg0; code arg1; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         {  auto &&arg0 = this->arg0.execute();
            if (MNL_LIKELY(!test<bool>(arg0))) { val argv[]{(move)(arg0), arg1.execute()}; return MNL_SYM("&")(_loc, 2, argv); }
            if (!cast<bool>(arg0)) return false;
         }
         return [&]()->val{ // RVO
            auto arg1 = this->arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!test<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            return arg1;
         }();
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_and<>>(rhs) &&
            aux::match(cast<const expr_and<> &>(rhs).arg0, arg0) &&
            aux::match(cast<const expr_and<> &>(rhs).arg1, arg1) &&
            (_loc = cast<const expr_and<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_and &);
   };

   template<typename Arg0 = code> struct expr_or {
      MNL_RVALUE()
      Arg0 arg0; code arg1; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         {  auto &&arg0 = this->arg0.execute();
            if (MNL_LIKELY(!test<bool>(arg0))) { val argv[]{(move)(arg0), arg1.execute()}; return MNL_SYM("|")(_loc, 2, argv); }
            if (cast<bool>(arg0)) return true;
         }
         return [&]()->val{ // RVO
            auto arg1 = this->arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!test<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            return arg1;
         }();
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_or<>>(rhs) &&
            aux::match(cast<const expr_or<> &>(rhs).arg0, arg0) &&
            aux::match(cast<const expr_or<> &>(rhs).arg1, arg1) &&
            (_loc = cast<const expr_or<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_or &);
   };

   template<typename Cond = code> struct expr_while {
      MNL_RVALUE()
      Cond cond; code body; loc _loc;
   public:
      template<bool fast_sig = bool{}, bool = bool{}> MNL_INLINE val execute() const {
         for (;;) {
            const val &cond = this->cond.execute();
            if (MNL_LIKELY(cond == true)) {
               body.execute<fast_sig, true>();
               if constexpr(fast_sig) if (MNL_UNLIKELY(sig_state.first)) return {};
               continue;
            }
            if (MNL_LIKELY(cond == false)) return {};
            MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         }
      }


      MNL_INLINE decltype(nullptr) execute(bool fast_sig = false) const {
         for (;;) {
            auto &&cond = this->cond.execute(); // no prediction for "cond" on purpose - we admit here performing zero iterations for while-loops may be useful
            if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            if (!cast<bool>(cond) || MNL_UNLIKELY(body.execute(fast_sig), sig_state.first)) return {};
         }
      }
   };



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
