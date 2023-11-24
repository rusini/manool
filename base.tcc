// base.tcc -- base internal interfaces

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include <system_error>
# include "manool.hh"

namespace MNL_AUX_UUID {

namespace aux {

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Constant value evaluated before evaluation of an expression, during its compilation
   template< typename Val = const val &, typename Value = std::remove_cv_t<std::remove_reference_t<Val>>,
      bool is_special = std::is_empty_v<Value> &&
         std::is_trivially_default_constructible_v<Value> && std::is_trivially_constructible_v<Val, Value>,
      std::enable_if_t<is_special == ( std::is_empty_v<Value> && // consistency check
         std::is_trivially_default_constructible_v<Value> && std::is_trivially_constructible_v<Val, Value> ), int> = int{} >
   struct expr_lit/*eral*/: code::rvalue {
      Value value;
   public:
      template<bool = bool{}, bool nores = bool{}>
      MNL_INLINE std::conditional_t<!nores, Val, void> execute() const noexcept(nores || noexcept(Val(value)))
      { if constexpr (!nores) return value; }
   public:
      using optimal = expr_lit<std::conditional_t<
         std::is_trivially_copy_constructible_v<Value> && sizeof(Value) <= 2 * sizeof(long), Value, const Value & >>;
   };
   template<typename Value = decltype(nullptr)>
   expr_lit(Value)->expr_lit<Value>::optimal;
# if MNL_LEAN
   template<class Value>
   struct expr_lit<Value, Value, true>;
# else
   template<class Value>
   class _expr_lit_blackhole {
      constexpr _expr_lit_blackhole() = default();
      MNL_INLINE constexpr /*implicit*/ _expr_lit_blackhole(const Value &) noexcept {}
   };
   template<class Value>
   struct expr_lit<Value, Value, true>: code::rvalue, _expr_lit_blackhole<Value> {
   public:
      template<bool = bool{}, bool = bool{}>
      MNL_INLINE constexpr Value execute() const noexcept
      { return {}; }
   public:
      using optimal = expr_lit;
   };
# endif

   struct expr_tvar: code::lvalue { // "*t*emporary *var*iable"
      int offset;
      MNL_INLINE const val &execute(bool = {}, bool = {}) const noexcept { return tvar_stack[offset]; } // TODO: make tvar_stack as efficient as possible, not std::vector
      MNL_INLINE void exec_in(const val &value) const noexcept { exec_in((val)value); }
      MNL_INLINE void exec_in(val &&value) const noexcept { tvar_stack[offset].swap(value); }
      MNL_INLINE val exec_out() const noexcept { return std::move(tvar_stack[offset]); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<
      int Argc, class Target = code,
      typename Arg0 = std::conditional_t<Argc >= 1 && Argc <= 2, code, void>,
      typename Arg1 = std::conditional_t<Argc >= 2 && Argc <= 2, code, void>,
      std::enable_if_t<Argc >= 0 && Argc <= 4, decltype(nullptr)> = decltype(nullptr){},
      std::enable_if_t<std::is_class_v<code, Target>, decltype(nullptr)> = decltype(nullptr){},
      std::enable_if_t<Argc >= 1 && Argc <= 2 ? std::is_class_v<Arg0> : std::is_void_v<Arg0>, decltype(nullptr)> = decltype(nullptr){},
      std::enable_if_t<Argc >= 2 && Argc <= 2 ? std::is_class_v<Arg1> : std::is_void_v<Arg1>, decltype(nullptr)> = decltype(nullptr){} >
   class expr_apply;

   // Application specialized for 0 arguments
   template<class Target>
   expr_apply(
      code::rvalue, Target, loc )->
   expr_apply< 0,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>::optimal> >;
   template<class Target>
   struct expr_apply<0, Target>: code::rvalue {
      Target target; loc _loc;
      static_assert(std::is_base_of_v<code, Target> || std::is_base_of_v<rvalue, Target>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         try { return op<sym::id("Apply")>(std::forward<decltype(target)>(target), 0, {}); }
         catch (...) { trace_execute(_loc); }
      }
   };

   template<class Target> class _expr_apply_blackhole {
      constexpr _expr_apply_blackhole() = default;
      MNL_INLINE constexpr /*implicit*/ _expr_apply_blackhole(const Target &) noexcept {}
      MNL_INLINE constexpr /*implicit*/ _expr_apply_blackhole(const expr_lit<Target, Target, true> &) noexcept {}
   };

   // Application specialized for 1 argument
   template<typename Target, typename Arg0>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, loc )->
   expr_apply< 1,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>::optimal>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   || std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   expr_lit<Arg0>  ::optimal> >;
   template<class Target, class Arg0>
   struct expr_apply<1, Target, Arg0>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      Target target; Arg0 arg0; loc _loc;
      static_assert(std::is_base_of_v<code, Target> || std::is_base_of_v<rvalue, Target>);
      static_assert(std::is_base_of_v<code, Arg0>   || std::is_base_of_v<rvalue, Arg0>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&arg0 = this->arg0.execute(); auto &&target = this->target.execute();
         typedef std::conditional_t<has_apply<>{}, decltype(std::forward<decltype(target)>(target)), val> _target;
         try { return ((_target)std::forward<decltype(target)>(target))(std::forward<decltype(arg0)>(arg0)); }
         catch (...) { trace_execute(_loc); }
      }
   private:
      template<class = Target, typename = void> struct has_apply: std::false_type {};
      template<class T_> struct has_apply<T_, decltype((void)T_{}.execute()(Arg0{}.execute()))>: std::true_type {};
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // unreachable (for DCE)
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); val target = this->target.exec_out();
            try { return repl(std::move(target), std::forward<decltype(arg0)>(arg0), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out(); // unreachable (for DCE)
         val argv_out[2 + 1];
         target.exec_in( [&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {arg0.execute()}, target = this->target.exec_out();
            try { return repl(std::move(target), std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   private:
      static constexpr auto repl = op<sym::id("Repl")>;
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };
   template<typename Target, class Arg0>
   struct expr_apply<1, expr_lit<Target, Target, true>, Arg0>:
      code::rvalue, _expr_apply_blackhole<Target> {
      Arg0 arg0; loc _loc;
      static_assert(std::is_base_of_v<code, Arg0> || std::is_base_of_v<rvalue, Arg0>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&arg0 = this->arg0.execute();
         typedef std::conditional_t<has_apply<>{}, decltype(Target{}), val> _target;
         try { return ((_target)Target{})(std::forward<decltype(arg0)>(arg0)); }
         catch (...) { trace_execute(_loc); }
      }
   private:
      template<class = Target, typename = void> struct has_apply: std::false_type {};
      template<class T_> struct has_apply<T_, decltype((void)T_{}(Arg0{}.execute()))>: std::true_type {};
   };

   // Application specialized for 2 arguments
   template<typename Target, typename Arg0, typename Arg1>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, Arg0, Arg1, loc )->
   expr_apply< 2,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>::optimal>,
      std::conditional_t<std::is_base_of_v<code, Arg0>   || std::is_base_of_v<code::rvalue, Arg0>,   Arg0,   expr_lit<Arg0>  ::optimal>,
      std::conditional_t<std::is_base_of_v<code, Arg1>   || std::is_base_of_v<code::rvalue, Arg1>,   Arg1,   expr_lit<Arg1>  ::optimal> >;
   template<class Target, class Arg0, class Arg1>
   struct expr_apply<2, Target, Arg0, Arg1>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      Target target; Arg0 arg0; Arg1 arg1; loc _loc;
      static_assert(std::is_base_of_v<code, Target> || std::is_base_of_v<rvalue, Target>);
      static_assert(std::is_base_of_v<code, Arg0>   || std::is_base_of_v<rvalue, Arg0>);
      static_assert(std::is_base_of_v<code, Arg1>   || std::is_base_of_v<rvalue, Arg1>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); auto &&target = this->target.execute();
         typedef std::conditional_t<has_apply<>{}, decltype(std::forward<decltype(target)>(target)), val> _target;
         try { return ((_target)std::forward<decltype(target)>(target))(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1)); }
         catch (...) { trace_execute(_loc); }
      }
   private:
      template<class = Target, typename = void> struct has_apply: std::false_type {};
      template<class T_> struct has_apply<T_, decltype((void)T_{}.execute()(Arg0{}.execute(), Arg1{}.execute()))>: std::true_type {};
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // unreachable (for DCE)
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); val target = this->target.exec_out();
            try { return repl(std::move(target), std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out(); // unreachable (for DCE)
         val argv_out[3 + 1];
         target.exec_in( [&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {arg0.execute(), arg1.execute()}, target = this->target.exec_out();
            try { return repl(std::move(target), std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   private:
      static constexpr auto repl = op<sym::id("Repl")>;
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   public:



   
      static bool match(const expr_apply<2> &expr, code &res) {
         if (is<Target>(expr.target) && is<Arg0>(expr.arg0) && is<Arg1>(expr.arg1))
            return res = expr_apply{as<Target>(expr.target), as<Arg0>(expr.arg0), as<Arg1>(expr.arg1)}, true;
         returm false;
      }


      static std::optional<expr_apply> match(const expr_apply<2> &src) {
         if (auto _target = Target::match(src.target)))
         if (auto _arg0   = Arg0  ::match(src.arg0)))
         if (auto _arg1   = Arg1  ::match(src.arg1)))
            return expr_apply{*_target, *_arg0, *_arg1, src._loc};
         return {};
      }
      static std::optional<expr_apply> match(code src) {
         if (is<expr_apply>(src)) return as<expr_apply>(src);
         if (is<expr_apply<2>>(src)) return match(as<expr_apply<2>>(src));
         return {};
      }


      bool match(const expr_apply<2> &src) {
         return target.nonvalue::match(src.target) || arg0.nonvalue::match(src.arg0) || arg1.nonvalue::match(src.arg1);
      }



   };
   template<class Target, class Arg0, class Arg1>
   struct expr_apply<2, expr_lit<Target, Target, true>, Arg0, Arg1>:
      code::rvalue, _expr_apply_blackhole<Target> {
      Arg0 arg0; Arg0 arg1; loc _loc;
      static_assert(std::is_base_of_v<code, Arg0> || std::is_base_of_v<rvalue, Arg0>);
      static_assert(std::is_base_of_v<code, Arg1> || std::is_base_of_v<rvalue, Arg1>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute() const {
         auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute();
         typedef std::conditional_t<has_apply<>{}, decltype(Target{}), val> _target;
         try { return ((_target)Target{})(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1)); }
         catch (...) { trace_execute(_loc); }
      }
   private:
      template<typename = Target, typename = void> struct has_apply: std::false_type {};
      template<typename T_> struct has_apply<T_, decltype((void)T_{}(Arg0{}.execute(), Arg1{}.execute()))>: std::true_type {};
   };

   // Application specialized for 3 arguments
   template<typename Target>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, code, code, code, loc )->
   expr_apply< 3,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>::optimal> >;
   template<class Target>
   struct expr_apply<3, Target>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      Target target; code a0, a1, a2; loc _loc; // arg types intentionally not parameterized
      static_assert(std::is_base_of_v<code, Target> || std::is_base_of_v<rvalue, Target>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute(bool = {}) const {
         val argv[] = {a0.execute(), a1.execute(), a2.execute()}; auto &&target = this->target.execute();
         typedef std::conditional_t<has_apply<>{}, decltype(std::forward<decltype(target)>(target)), val> _target;
         try { return ((_target)std::forward<decltype(target)>(target))(std::size(argv), argv); } // NB we benefit from the fact that argc is checked using inlining (potentially eliminated)
         catch (...) { trace_execute(_loc); }
      }
   private:
      template<class = Target, typename = void> struct has_apply: std::false_type {};
      template<class T_> struct has_apply<T_, decltype((void)T_{}.execute()(std::declval<std::size_t>(), std::declval<val *>()))>: std::true_type {};
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // unreachable (for DCE)
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), std::forward<Val>(value)}, target = this->target.exec_out();
            try { return repl(std::move(target), std::size(argv), argv); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out(); // unreachable (for DCE)
         val argv_out[4];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute()}, target = this->target.exec_out();
            try { return repl(std::move(target), std::size(argv), argv, argv_out); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   private:
      static constexpr auto repl = op<sym::id("Repl")>;
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   // Application specialized for 4 arguments
   template<typename Target>
   expr_apply(
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue>,
      Target, code, code, code, code, loc )->
   expr_apply< 4,
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::rvalue, Target>, Target, expr_lit<Target>::optimal> >;
   template<class Target>
   struct expr_apply<4, Target>:
      std::conditional_t<std::is_base_of_v<code, Target> || std::is_base_of_v<code::lvalue, Target>, code::lvalue, code::rvalue> {
      Target target; code a0, a1, a2, a3; loc _loc; // arg types intentionally not parameterized
      static_assert(std::is_base_of_v<code, Target> || std::is_base_of_v<rvalue, Target>);
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE auto execute(bool = {}) const {
         val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}; auto &&target = this->target.execute();
         typedef std::conditional_t<has_apply<>{}, decltype(std::forward<decltype(target)>(target)), val> _target;
         try { return ((_target)std::forward<decltype(target)>(target))(std::size(argv), argv); } // NB we benefit from the fact that argc is checked using inlining (potentially eliminated)
         catch (...) { trace_execute(_loc); }
      }
   private:
      template<class = Target, typename = void> struct has_apply: std::false_type {};
      template<class T_> struct has_apply<T_, decltype((void)T_{}.execute()(std::declval<std::size_t>(), std::declval<val *>()))>: std::true_type {};
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_in(std::forward<Val>(value)); // unreachable (for DCE)
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), std::forward<Val>(value)}, target = this->target.exec_out();
            try { return repl(std::move(target), std::size(argv), argv); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if constexpr (!std::is_base_of_v<lvalue, expr_apply>) return rvalue::exec_out(); // unreachable (for DCE)
         val argv_out[5];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}, target = this->target.exec_out();
            try { return repl(std::move(target), std::size(argv), argv, argv_out); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   private:
      static constexpr auto repl = op<sym::id("Repl")>;
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<class Dest = code, class Src = code> struct expr_set: code::rvalue {
      Dest dest; Src src;
      MNL_INLINE decltype(nullptr) execute(bool = {}) const { dest.exec_in(src.execute()); return {}; }
      MNL_INLINE void exec_nores(bool = {}) const { execute(); }
   public:
      template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Dest>().execute()), decltype(std::declval<Src>().execute()))>
         struct _update;
      template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Src>().execute())), decltype(std::declval<Dest>().execute())>
         struct _update_rhs;
   };
   template<class Src>
   template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Dest>().execute()), decltype(std::declval<Src>().execute()))>
   struct expr_set<expr_tv, Src>::_update: code::rvalue {
      expr_tv dest; Src src;
   public:
      MNL_INLINE decltype(nullptr) execute(bool = {}) const
         { auto &&src = this->src.execute(); dest.exec_in(Op(trace_execute, _loc, dest.execute(), std::forward<decltype(src)>(src))); return {}; }
      MNL_INLINE void exec_nores(bool = {}) const
         { execute(); }
   };
   template<class Src>
   template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Src>().execute())), decltype(std::declval<Dest>().execute())>
   struct expr_set<Src, expr_tv>::_update_rhs: code::rvalue {
      expr_tv dest; Src src;
   public:
      MNL_INLINE decltype(nullptr) execute(bool = {}) const
         { auto &&src = this->src.execute(); dest.exec_in(Op(trace_execute, _loc, std::forward<decltype(src)>(src)), dest.execute()); return {}; }
      MNL_INLINE void exec_nores(bool = {}) const
         { execute(); }
   };

   template<class Dest = code> struct expr_move: code::rvalue {
      Dest dest;
      MNL_INLINE val execute(bool = {}) const { return dest.exec_out(); }
      MNL_INLINE void exec_nores(bool = {}) const { execute(); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   struct _expr_ifelse_misc { code body1, body2; };
   template<class Cond = code, std::enable_if_t<std::is_class_v<Cond>, decltype(nullptr)> = decltype(nullptr){}>
   struct expr_ifelse: code::lvalue {
      Cond cond; _expr_ifelse_misc misc; loc _loc;
      static_assert(std::is_base_of_v<code, Cond> || std::is_base_of_v<rvalue, Cond>);
   public:
      template<bool fast_sig = bool{}, bool nores = bool{}> MNL_INLINE auto execute() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (as<bool>(cond) ? misc.body1 : misc.body2).execute<fast_sig, nores>();
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (as<bool>(cond) ? misc.body1 : misc.body2).exec_in(std::forward<Val>(value));
      }
   public:
      MNL_INLINE val exec_out() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (as<bool>(cond) ? misc.body1 : misc.body2).exec_out();
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept {
         return misc.body1.is_lvalue() && misc.body2.is_lvalue();
      }
   };
   template<class Cond> expr_ifelse(Cond, _expr_ifelse_misc, loc)->expr_ifelse<Cond>;
   template<class Cond> expr_ifelse(Cond, code, code, loc)->expr_ifelse<Cond>;

   struct _expr_if_misc { code body; };
   template<class Cond = code, std::enable_if_t<std::is_class_v<Cond>, decltype(nullptr)> = decltype(nullptr){}>
   struct expr_if: code::rvalue {
      Cond cond; _expr_if_misc misc; loc _loc;
   public:
      template<bool fast_sig = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (as<bool>(cond)) misc.body.execute<fast_sig, true>();
         return {};
      }
   };
   template<class Cond> expr_if(Cond, _expr_if_misc, loc)->expr_if<Cond>;
   template<class Cond> expr_if(Cond, code, loc)->expr_if<Cond>;

   struct _expr_and_misc { code arg1; };
   template<class Arg0 = code, std::enable_if_t<std::is_class_v<Cond>, decltype(nullptr)> = decltype(nullptr){}>
   struct expr_and: code::rvalue {
      Arg0 cond; _expr_and_misc misc; loc _loc;
   public:
      template<bool = bool{}, bool = bool{}> MNL_INLINE val execute() const {
         auto &&arg0 = cond.execute();
         if (MNL_LIKELY(!is<bool>(arg0))) {
            val argv[] = {std::move(arg0), misc.arg1.execute()};
            return MNL_SYM("&")(trace_execute, _loc, std::size(argv), argv);
         }
         if (!as<bool>(arg0))
            return false;
         return [&]() MNL_INLINE{ // RVO
            val arg1 = this->misc.arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!is<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            return arg1;
         }();
      }
   };

   template<class Arg0 = code> struct expr_or: code::rvalue {
      Arg0 arg0; code arg1; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         auto &&arg0 = this->arg0.execute();
         if (MNL_LIKELY(!test<bool>(arg0))) {
            val argv[] = {std::move(arg0), arg1.execute()};
            return MNL_SYM("|")(trace_execute, _loc, std::size(argv), argv);
         }
         if (cast<bool>(arg0))
            return true;
         return [&]() MNL_INLINE{ // RVO
            val arg1 = this->arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!test<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            return arg1;
         }();
      }
   };

   struct _expr_while_misc { code body; };
   template<class Cond = code, std::enable_if_t<std::is_class_v<Cond>, decltype(nullptr)> = decltype(nullptr){}>
   struct expr_while: code::rvalue {
      Cond cond; _expr_while_misc misc; loc _loc;
      static_assert(std::is_base_of_v<code, Cond> || std::is_base_of_v<rvalue, Cond>);
   public:
      template<bool fast_sig = bool{}, bool = bool{}> MNL_INLINE decltype(nullptr) execute() const {
         for (;;) {
            auto &&cond = this->cond.execute(); // no prediction for condition - performing zero iterations for while-loops may be useful
            if (MNL_UNLIKELY(!is<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            if (!as<bool>(cond) || MNL_UNLIKELY(misc.body.execute<fast_sig, true>(), fast_sig && sig_state)) return {};
         }
      }
   };
   template<class Cond> expr_while(Cond, code, loc)->expr_while<Cond>;

   template<class Tag = code> struct expr_on: code::rvalue {
      Tag tag; code trap, body; loc _loc;
   public:
      MNL_INLINE val execute(bool fast_sh = false) const {
         auto &&tag = this->tag.execute();
         if (MNL_UNLIKELY(!test<sym>(tag))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         val res = [&]()MNL_INLINE->val{ try { return body.execute(true); } // res for NRVO
            catch (std::pair<sym, val> &ex) { sig_state.raise(std::move(ex)); }
            catch (stack_overflow &)        { sig_state.raise(MNL_SYM("StackOverflow")); }
            catch (heap_exhausted &)        { sig_state.raise(MNL_SYM("HeapExhausted")); }
            catch (std::bad_alloc &)        { sig_state.raise(MNL_SYM("LimitExceeded")); }
            catch (std::system_error &ex)   { sig_state.raise(MNL_SYM("SystemError"), ex.what()); }
            return {};
         }();
         if (MNL_LIKELY(!sig_state)) // no signal
            return res;
         if (MNL_UNLIKELY(sig_state.tag() == cast<const sym &>(tag))) { // caught!
            val arg = (sig_trace.clear(), sig_state.cancel().second);
            tv_stack.push(std::move(arg)); struct _ { MNL_INLINE ~_() { tv_stack.drop(); } } _;
            res = trap.execute(fast_sh); return res;
         }
         if (MNL_LIKELY(fast_sh)) return res; // fast propagation
         throw sig_state.cancel(); // slow propagation
      }
      MNL_INLINE void exec_nores(bool fast_sh = false) const {
         auto &&tag = this->tag.execute();
         if (MNL_UNLIKELY(!test<sym>(tag))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         try { body.exec_nores(true); }
            catch (std::pair<sym, val> &ex) { sig_state.raise(std::move(ex)); }
            catch (stack_overflow &)        { sig_state.raise(MNL_SYM("StackOverflow")); }
            catch (heap_exhausted &)        { sig_state.raise(MNL_SYM("HeapExhausted")); }
            catch (std::bad_alloc &)        { sig_state.raise(MNL_SYM("LimitExceeded")); }
            catch (std::system_error &ex)   { sig_state.raise(MNL_SYM("SystemError"), ex.what()); }
         if (MNL_LIKELY(!sig_state)) // no signal
            return;
         if (MNL_UNLIKELY(sig_state.tag() == cast<const sym &>(tag))) { // caught!
            val arg = (sig_trace.clear(), sig_state.cancel().second);
            tv_stack.push(std::move(arg)); struct _ { MNL_INLINE ~_() { tv_stack.drop(); } } _;
            trap.exec_nores(fast_sh); return;
         }
         if (MNL_LIKELY(fast_sh)) return; // fast propagation
         throw sig_state.cancel(); // slow propagation
      }
   };

   template<class Tag = code> struct expr_signal: code::rvalue {
      Tag tag; code arg; loc _loc;
      MNL_INLINE val execute(bool fast_sh) const {
         auto &&tag = this->tag.execute();
         if (MNL_UNLIKELY(!test<sym>(tag))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         sig_state.raise(cast<const sym &>(tag), arg.execute());
         if (MNL_LIKELY(fast_sh)) return {}; // fast propagation
         throw sig_state.cancel(); // slow propagation
      }
      MNL_INLINE void exec_nores(bool fast_sh) const {
         execute();
      }
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
      MNL_INLINE void exec_in(const val &value) const { exec_in(val(value)); }
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
      MNL_INLINE void exec_in(const val &value) const { exec_in(val(value)); }
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
