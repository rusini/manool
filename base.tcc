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

   template<typename Expr> MNL_INLINE inline bool match(code &expr)
      { Expr res; return match(expr, res) && (expr = res, true); }

   template<typename Expr> MNL_NOINLINE inline bool match(const code &expr, Expr &res)
      { return test<Expr>(expr) ? (res = cast<const Expr &>(expr), true) : res.match(expr); }
   MNL_NOINLINE inline bool match(const code &expr, code &res)
      { return res = expr, true; }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   inline constexpr class tv_stack {
      inline static MNL_IF_WITH_MT(thread_local) val *base;
      inline static MNL_IF_WITH_MT(thread_local) val *top, *limit, *frame;
   public:
      MNL_INLINE void extend(int size) const {
         _extend(size);
         auto top = this->top + size;
         for (auto pv = this->top; pv < top;) new(pv++) val;
         this->top = top;
      }
      MNL_INLINE void drop(int size) const noexcept {
         auto top = this->top;
         for (; size; --size) --top->~val();
         this->top = top
      }
   public:
      template<typename Val> MNL_INLINE void push(Val &&value) const { _extend(); new(top) val(std::forward<decltype(value)>(value)), ++top; }
      MNL_INLINE val *enter() const noexcept { auto frame = this->frame; this->frame = top; return frame; }
      MNL_INLINE void leave(val *frame) const noexcept { this->frame = frame; }
      MNL_INLINE void finalize() const noexcept { std::free(base); }
      MNL_INLINE val &operator[](int index) const noexcept { return frame[index]; } // the reason why everything is wrapped is a class
   private:
      MNL_INLINE static void _extend(int size = 1) {
         if ((unsigned long)top + size * sizeof *top < (unsigned long)limit) return; // comparing addresses to avoid UB
         val *base; auto top = this->top - base, limit = top + size, frame = this->frame - base;
         if (!MNL_LIKELY(base = std::realloc(this->base, sizeof(val [limit = (limit << 1) + limit + 1 >> 1])))) throw std::bad_alloc{};
         this->base = base; this->top = base + top, this->limit = base + limit, this->frame = base + frame;
      }
   } tv_stack;


   template<typename Val = val> struct expr_lit: expr_rvalue { // literal
      std::remove_cv_t<std::remove_reference_t<Val>> value;
      MNL_INLINE Val execute(bool = {}) const noexcept { return value; }
   public:
      template<Val Value> struct _const;
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_lit<>>(rhs) && test<Val>(cast<const expr_lit<> &>(rhs).value) &&
            (value = cast<Val>(cast<const expr_lit<> &>(rhs).value), true);
      }
      friend bool aux::match<>(const code &, expr_lit &);
   };
   template<typename Val> template<Val Value>
   struct _const: expr_rvalue {
      MNL_INLINE Val execute(bool = {}) const noexcept { return Value; }
   };

   struct expr_tv: expr_lvalue { // temporary variable
      int off;
      MNL_INLINE const val &execute(bool = {}) const noexcept { return tv_stack[off]; }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const noexcept { tv_stack[off].assign(std::forward<decltype(value)>(value); }
      MNL_INLINE val exec_out() const noexcept { return std::move(tv_stack[off]); }
   private:
      MNL_INLINE bool match(const code &) { return {}; }
      friend bool aux::match<>(const code &, expr_tmp &);
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Target = code> struct expr_apply0: expr_rvalue {
      Target target; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const {
         return target.execute()(trace_execute, _loc);
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_apply0<>>(rhs) &&
            aux::match(cast<const expr_apply0<> &>(rhs).target, target) &&
            (_loc = cast<const expr_apply0<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_apply0 &);
   };

   template<typename Target = code, typename Arg0 = code> struct expr_apply1: expr_lvalue {
      Target target; Arg0 arg0; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const {
         auto &&arg0 = this->arg0.execute();
         return target.execute()(trace_execute, _loc, std::forward<decltype(arg0)>(arg0));
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         target.exec_in([&]()MNL_INLINE{
            auto &&arg0 = this->arg0.execute();
            return target.exec_out().repl(trace_exec_in, _loc, std::forward<decltype(arg0)>(arg0),
               std::forward<decltype(value)>(value));
         }());
      }
      MNL_INLINE val exec_out() const {
         val argv_out[2];
         target.exec_in([&]()MNL_INLINE{
            val argv[std::size(argv_out)] = {arg0.execute()};
            return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
         }());
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   public:
      template<typename Res, Res (void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()))>
      struct _op;
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_apply1<>>(rhs) &&
            aux::match(cast<const expr_apply1<> &>(rhs).target, target) &&
            aux::match(cast<const expr_apply1<> &>(rhs).arg0,   arg0) &&
            (_loc = cast<const expr_apply1<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_apply1 &);
   };
   template<typename Target, typename Arg0>
   template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()))>
   struct expr_apply1<Target, Arg0>::_op<Res, Op>: expr_rvalue {
      Arg0 arg0; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const
         { return Op(trace_execute, _loc, arg0.execute()); }
      MNL_INLINE void exec_nores(bool = {}) const
         { execute(); }
   };


   template<typename Arg0 = code, typename Res, Res (*Op)(decltype(Op(std::declval<Arg0>().execute())))> struct expr_apply_op1: expr_rvalue { \
      Arg0 arg0; loc _loc; \
   public: \
      MNL_INLINE decltype(mnl::OP(arg0.execute())) execute(bool = {}) const { \
         auto &&arg0 = this->arg0.execute(); \
         try { return mnl::OP((move)(arg0)); } catch (...) { trace_execute(_loc); } \
      } \
   private: \
      MNL_INLINE bool match(const code &rhs) { \
         return test<expr_apply1<>>(rhs) && \
            test<expr_lit<const sym &>>(cast<const expr_apply1<> &>(rhs).target) && \
               cast<const expr_lit<const sym &> &>(cast<const expr_apply1<> &>(rhs).target).value == MNL_SYM(SYM) && \
            aux::match(cast<const expr_apply1<> &>(rhs).arg0, arg0) && \
            (_loc = cast<const expr_apply1<> &>(rhs)._loc, true); \
      } \
      friend bool aux::match<>(const code &, expr##OP &); \
   }; \
   // end # define MNL_M(OP, SYM)
   MNL_M(_neg, "Neg") MNL_M(_abs, "Abs") MNL_M(_not, "~")
# undef MNL_M




   template<typename Target = code, typename Arg0 = code, typename Arg1 = code> struct expr_apply2: expr_lvalue {
      Target target; Arg0 arg0; Arg1 arg1; loc _loc;
   public:
      MNL_INLINE void execute(bool = {}) const {
         auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute();
         return target.execute()(trace_execute, _loc, std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1));
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         target.exec_in([&]()MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute();
            return target.exec_out().repl(trace_exec_in, _loc, std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1),
               std::forward<decltype(value)>(value));
         }());
      }
      MNL_INLINE val exec_out() const {
         val argv_out[3];
         target.exec_in([&]()MNL_INLINE{
            val argv[std::size(argv_out)] = {arg0.execute(), arg1.execute()};
            return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
         }());
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   public:
      template<typename Res, Res (void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()), decltype(std::declval<Arg1>().execute()))>
      struct _op;
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_apply2<>>(rhs) &&
            aux::match(cast<const expr_apply2<> &>(rhs).target, target) &&
            aux::match(cast<const expr_apply2<> &>(rhs).arg0,   arg0) &&
            aux::match(cast<const expr_apply2<> &>(rhs).arg1,   arg1) &&
            (_loc = cast<const expr_apply2<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_apply2 &);
   };
   template<typename Target, typename Arg0, typename Arg1>
   template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()), decltype(std::declval<Arg1>().execute()))>
   struct expr_apply1<Target, Arg0, Arg1>::_op<Res, Op>: expr_rvalue {
      Arg0 arg0; Arg0 arg1; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const
         { auto &&arg0 = this->arg0.execute(); return Op(trace_execute, _loc, std::forward<decltype(arg0)>(arg0), arg1.execute()); }
      MNL_INLINE void exec_nores(bool = {}) const
         { execute(); }
   };




# define MNL_M(OP, SYM) \
   template<typename Res, Res (*Op)(decltype(Op(std::declval<Arg0>().execute(), std::declval<Arg1>().execute()))),
      typename Arg0 = code, typename Arg1 = code> struct expr_apply_op2: expr_rvalue {
      Arg0 arg0; Arg1 arg1; loc _loc; \
   public: \
      MNL_INLINE decltype(mnl::OP(arg0.execute(), arg1.execute())) execute(bool = {}) const { \
         auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); \
         try { return mnl::OP((move)(arg0), (move)(arg1)); } catch (...) { trace_execute(_loc); } \
      } \
   private: \
      MNL_INLINE bool match(const code &rhs) { \
         return test<expr_apply2<>>(rhs) && \
            test<expr_lit<const sym &>>(cast<const expr_apply2<> &>(rhs).target) && \
               cast<const expr_lit<const sym &> &>(cast<const expr_apply2<> &>(rhs).target).value == MNL_SYM(SYM) && \
            aux::match(cast<const expr_apply2<> &>(rhs).arg0, arg0) && \
            aux::match(cast<const expr_apply2<> &>(rhs).arg1, arg1) && \
            (_loc = cast<const expr_apply2<> &>(rhs)._loc, true); \
      } \
      friend bool aux::match<>(const code &, expr##OP &); \
   }; \
   // end # define MNL_M(OP, SYM)
   MNL_M(_eq, "==") MNL_M(_ne, "<>") MNL_M(_lt, "<") MNL_M(_le, "<=") MNL_M(_gt, ">") MNL_M(_ge, ">=")
   MNL_M(_add, "+") MNL_M(_sub, "-") MNL_M(_mul, "*") MNL_M(_xor, "Xor")
# undef MNL_M



   template<typename Target = code, typename Arg0 = code, typename Arg1 = code, typename Arg2 = code> struct expr_apply3: expr_lvalue {
      Target target; Arg0 arg0; Arg1 arg1; Arg2 arg2; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         val argv[] = {arg0.execute(), arg1.execute(), arg2.execute()};
         return target.execute()(trace_execute, _loc, std::size(argv), argv);
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         target.exec_in([&]()MNL_INLINE{
            val argv[] = {arg0.execute(), arg1.execute(), arg2.execute(), std::forward<decltype(value)>(value)};
            return target.exec_out().repl(trace_exec_in, _loc, std::size(argv), argv);
         }());
      }
      MNL_INLINE val exec_out() const {
         val argv_out[4];
         target.exec_in([&]()MNL_INLINE{
            val argv[std::size(argv_out)] = {arg0.execute(), arg1.execute(), arg2.execute()};
            return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
         }());
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_apply3<>>(rhs) &&
            aux::match(cast<const expr_apply3<> &>(rhs).target, target) &&
            aux::match(cast<const expr_apply3<> &>(rhs).arg0,   arg0) &&
            aux::match(cast<const expr_apply3<> &>(rhs).arg1,   arg1) &&
            aux::match(cast<const expr_apply3<> &>(rhs).arg2,   arg2) &&
            (_loc = cast<const expr_apply3<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_apply3 &);
   };

   template<typename Target = code, typename Arg0 = code, typename Arg1 = code, typename Arg2 = code, typename Arg3 = code> struct expr_apply4: expr_lvalue {
      Target target; Arg0 arg0; Arg1 arg1; Arg2 arg2; Arg3 arg3; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         val argv[] = {arg0.execute(), arg1.execute(), arg2.execute(), arg3.execute()};
         return target.execute()(trace_execute, _loc, std::size(argv), argv);
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         target.exec_in([&]()MNL_INLINE{
            val argv[] = {arg0.execute(), arg1.execute(), arg2.execute(), arg3.execute(), std::forward<decltype(value)>(value)};
            return target.exec_out().repl(trace_exec_in, _loc, std::size(argv), argv);
         }());
      }
      MNL_INLINE val exec_out() const {
         val argv_out[5];
         target.exec_in([&]()MNL_INLINE{
            val argv[std::size(argv_out)] = {arg0.execute(), arg1.execute(), arg2.execute(), arg3.execute()};
            return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
         }());
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_apply4<>>(rhs) &&
            aux::match(cast<const expr_apply4<> &>(rhs).target, target) &&
            aux::match(cast<const expr_apply4<> &>(rhs).arg0,   arg0) &&
            aux::match(cast<const expr_apply4<> &>(rhs).arg1,   arg1) &&
            aux::match(cast<const expr_apply4<> &>(rhs).arg2,   arg2) &&
            aux::match(cast<const expr_apply4<> &>(rhs).arg3,   arg3) &&
            (_loc = cast<const expr_apply4<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_apply4 &);
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Dest = code, typename Src = code> struct expr_set: expr_rvalue {
      Dest dest; Src src;
      MNL_INLINE decltype(nullptr) execute(bool = {}) const { dest.exec_in(src.execute()); return {}; }
      MNL_INLINE void exec_nores(bool = {}) const { dest.exec_in(src.execute()); }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_set<>>(rhs) &&
            aux::match(cast<const expr_set<> &>(rhs).dest, dest) &&
            aux::match(cast<const expr_set<> &>(rhs).src,  src);
      }
      friend bool aux::match<>(const code &, expr_set &);
   };

   template<typename Dest = code> struct expr_move: expr_rvalue {
      Dest dest;
      MNL_INLINE val execute(bool = {}) const { return dest.exec_out(); }
      MNL_INLINE void exec_nores(bool = {}) const { dest.exec_out(); }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_move<>>(rhs) &&
            aux::match(cast<const expr_move<> &>(rhs).dest, dest);
      }
      friend bool aux::match<>(const code &, expr_move &);
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Cond = code> struct expr_ifelse: expr_lvalue {
      Cond cond; code body1, body2; loc _loc;
   public:
      MNL_INLINE val execute(bool fast_sig = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (cast<bool>(cond) ? body1 : body2).execute(fast_sig);
      }
      MNL_INLINE void exec_nores(bool fast_sig = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (cast<bool>(cond) ? body1 : body2).exec_nores(fast_sig);
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (cast<bool>(cond) ? body1 : body2).exec_in(std::forward<decltype(value)>(value));
      }
      MNL_INLINE val exec_out() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (cast<bool>(cond) ? body1 : body2).exec_out();
      }
   public:
      MNL_INLINE bool is_lvalue() const noexcept { return body1.is_lvalue() && body2.is_lvalue(); }
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

   template<typename Cond = code> struct expr_if: expr_rvalue {
      Cond cond; code body; loc _loc;
   public:
      MNL_INLINE decltype(nullptr) execute(bool fast_sig = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (cast<bool>(cond)) body.exec_nores(fast_sig);
         return {};
      }
      MNL_INLINE void exec_nores(bool fast_sig = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (cast<bool>(cond)) body.exec_nores(fast_sig);
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
            if (MNL_LIKELY(!test<bool>(arg0)))
               { val argv[] = {(move)(arg0), arg1.execute()}; return MNL_SYM("&")(trace_execute, _loc, std::size(argv), argv); }
            if (!cast<bool>(arg0)) return false;
         }
         return [&]()MNL_INLINE{ // RVO
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

   template<typename Cond = code> struct expr_while: expr_rvalue {
      Cond cond; code body; loc _loc;
   public:
      MNL_INLINE decltype(nullptr) execute(bool fast_sig = false) const {
         for (;;) {
            auto &&cond = this->cond.execute(); // no prediction for "cond" on purpose - we admit here performing zero iterations for while-loops may be useful
            if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            if (!cast<bool>(cond) || MNL_UNLIKELY(body.exec_nores(fast_sig), sig_state.first)) return {};
         }
      }
      MNL_INLINE void exec_nores(bool fast_sig = false) const {
         for (;;) {
            auto &&cond = this->cond.execute(); // no prediction for "cond" on purpose - we admit here performing zero iterations for while-loops may be useful
            if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            if (!cast<bool>(cond) || MNL_UNLIKELY(body.exec_nores(fast_sig), sig_state.first)) return;
         }
      }
   private:
      MNL_INLINE bool match(const code &rhs) {
         return test<expr_while<>>(rhs) &&
            aux::match(cast<const expr_while<> &>(rhs).cond, cond) &&
            aux::match(cast<const expr_while<> &>(rhs).body, body) &&
            (_loc = cast<const expr_while<> &>(rhs)._loc, true);
      }
      friend bool aux::match<>(const code &, expr_while &);
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
