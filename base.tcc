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

   template<typename Val = const val &> struct expr_lit/*eral*/: code::rvalue { // constant value known by MANOOL translator at compile time
      std::remove_cv_t<std::remove_reference_t<Val>> value;
      MNL_INLINE Val execute(bool = {}) const noexcept { return value; }
      template<Val> struct _const;
   private: // What types actually make sense and do not eventually cause more run-time performance problems than they solve
      static_assert( // (this also corresponds to the set of overloads for `exec_in` in `code::root`)
         std::is_same_v<Val, const val &> ||
         std::is_same_v<Val, decltype(nullptr)> ||
         std::is_same_v<Val, long long> ||
         std::is_same_v<Val, double> ||
         std::is_same_v<Val, float> ||
         std::is_same_v<Val, const sym &> ||
         std::is_same_v<Val, bool> ||
         std::is_same_v<Val, unsigned> );
   };
   template<typename Val> template<Val Value> struct expr_lit<Val>::_const/*ant*/: code::rvalue { // to produce value-specialized code paths in the translator
      MNL_INLINE static Val execute(bool = {}) noexcept { return Value; }
   };

   struct expr_tv: code::lvalue { // "temporary variable"
      int offset;
      MNL_INLINE const val &execute(bool = {}) const noexcept { return tv_stack[offset]; }
      MNL_INLINE void exec_in(const val &value) const noexcept { exec_in((val)value); }
      MNL_INLINE void exec_in(val &&value) const noexcept { tv_stack[offset].swap(value); }
      MNL_INLINE val exec_out() const noexcept { return std::move(tv_stack[offset]); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<class Target = code> struct expr_apply0: code::rvalue { // application specialized for 0 arguments
      Target target; loc _loc;
      MNL_INLINE val execute(bool = {}) const { return MNL_SYM("Apply")(trace_execute, _loc, target.execute(), 0 , {}); }
      MNL_INLINE void exec_nores(bool = {}) const { execute(); }
   };

   template<class Target = code, class Arg0 = code> struct expr_apply1: code::lvalue { // application specialized for 1 argument
      Target target; Arg0 arg0; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const {
         auto &&arg0 = this->arg0.execute(); auto &&target = this->target.execute();
         try {
            return std::forward<decltype(target)>(target)(std::forward<decltype(arg0)>(arg0));
         } catch (...) { trace_execute(_loc); }
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         if (!target.maybe_lvalue())) MNL_UNREACHABLE();
         target.exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); val target = this->target.exec_out();
            try {
               return std::move(target).repl(std::forward<decltype(arg0)>(arg0), std::forward<Val>(value));
            } catch (...) { trace_exec_in(_loc); }
         }() );
      }
      MNL_INLINE val exec_out() const {
         if (MNL_IS_CTFOLDABLE(target.is_lvalue()) && !target.is_lvalue()) MNL_UNREACHABLE();
         val argv_out[2 + 1];
         target.exec_in( [&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {arg0.execute()}, target = this->target.exec_out();
            try {
               return MNL_SYM("Repl")(std::move(target), std::size(argv), argv, argv_out + 1);
            } catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   };
   template<class Arg0> class expr_apply1<void, Arg0> {
   public:
      expr_apply1() = delete;
      expr_apply1(expr_apply1 &&) = delete;
      ~expr_apply1() = delete;
      template<typename Res,
         Res (void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()))> struct _op;
   };
   template<class Arg0>
   template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()))>
   struct expr_apply1<void, Arg0>::_op/*erator*/: code::rvalue { // to produce operator-specialized code paths in the translator
      Arg0 arg0; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const
         { return Op(trace_execute, _loc, arg0.execute()); }
      MNL_INLINE void exec_nores(bool = {}) const
         { execute(); }
   };

   template<class Target = code, class Arg0 = code, class Arg1 = code> struct expr_apply2: code::lvalue { // application specialized for 2 arguments
      Target target; Arg0 arg0; Arg1 arg1; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const {
         auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); auto &&target = Target::execute();
         try { return std::forward<decltype(target)>(target)(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg0)>(arg0)); }
         catch (...) { trace_execute(_loc); }
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
   public:
      MNL_INLINE void exec_in(const val &value) const { _exec_in(value); }
      MNL_INLINE void exec_in(val &&value) const { _exec_in(std::move(value)); }
   private:
      template<typename Val> MNL_INLINE void _exec_in(Val &&value) const {
         if constexpr (!Target::maybe_lvalue) MNL_UNREACHABLE();
         Target::exec_in( [&]() MNL_INLINE{
            auto &&arg0 = this->arg0.execute(); auto &&arg1 = this->arg1.execute(); val target = Target::exec_out();
            try { return std::move(target).repl(std::forward<decltype(arg0)>(arg0), std::forward<decltype(arg1)>(arg1), std::forward<Val>(value)); }
            catch (...) { trace_exec_in(_loc); }
         }() );
      }
   public:
      MNL_INLINE val exec_out() const {
         if constexpr (!Target::maybe_lvalue) MNL_UNREACHABLE();
         val argv_out[3 + 1];
         Target::exec_in( [&]() MNL_INLINE{
            val argv[std::size(argv_out) - 1] = {arg0.execute(), arg1.execute()}, target = Target::exec_out();
            try { return MNL_SYM("Repl")(std::move(target), std::size(argv), argv, argv_out + 1); }
            catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
   public:
      static constexpr bool maybe_lvalue = Target::maybe_lvalue;
      MNL_INLINE bool is_lvalue() const noexcept { return target.is_lvalue(); }
   };



   template<class Arg0, class Arg1> class expr_apply2<void, Arg0, Arg1> {
   public:
      expr_apply2() = delete;
      expr_apply2(expr_apply2 &&) = delete;
      ~expr_apply2() = delete;
      template<typename Res,
         Res (void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()), decltype(std::declval<Arg1>().execute()))> struct _op;
   };
   template<class Arg0, class Arg1>
   template<typename Res, Res Op(void (const loc &), const loc &, decltype(std::declval<Arg0>().execute()), decltype(std::declval<Arg1>().execute()))>
   struct expr_apply2<void, Arg0, Arg1>::_op/*erator*/: code::rvalue { // to produce operator-specialized code paths in the translator
      Arg0 arg0; Arg0 arg1; loc _loc;
   public:
      MNL_INLINE auto execute(bool = {}) const
         { auto &&arg0 = this->arg0.execute(); return Op(trace_execute, _loc, std::forward<decltype(arg0)>(arg0), arg1.execute()); }
      MNL_INLINE void exec_nores(bool = {}) const
         { execute(); }
   };

   template<class Target = code> struct apply3: code::lvalue { // application specialized for 3 arguments (args intentionally not parameterized)
      Target target; code a0, a1, a2; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         val argv[] = {a0.execute(), a1.execute(), a2.execute()}; auto &&target = this->target.execute();
         try {
            return std::forward<decltype(target)>(target)(std::size(argv), argv); // NB we benefit from the fact that argc is checked using inlining (potentially eliminated)
         } catch (...) { trace_execute(_loc); }
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), std::forward<Val>(value)}; val target = this->target.exec_out();
            try {
               return MNL_SYM("Repl")(std::move(target), std::size(argv), argv);
            } catch (...) { trace_exec_in(_loc); }
         }() );
      }
      MNL_INLINE val exec_out() const {
         val argv_out[4];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute()}; val target = this->target.exec_out();
            try {
               return MNL_SYM("Repl")(std::move(target), std::size(argv), argv, argv_out);
            } catch (...) { trace_exec_out(_loc); }
         }() );
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   };

   template<class Target = code> struct apply4: code::lvalue { // application specialized for 4 arguments (args intentionally not parameterized)
      Target target; code a0, a1, a2, a3; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute()}; auto &&target = target.execute();
         try {
            return std::forward<decltype(target)>(target)(std::size(argv), argv);
         } catch (...) { trace_execute(_loc); }
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      MNL_INLINE void exec_in(val &&value) const {
         target.exec_in([&]() MNL_INLINE{
            val argv[] = {arg0.execute(), arg1.execute(), arg2.execute(), arg3.execute(), std::move(value)};
            return MNL_SYM("Repl")(trace_exec_in, _loc, target.exec_out(), std::size(argv), argv, argv_out);
         }());
      }
      MNL_INLINE val exec_out() const {
         val argv_out[5];
         target.exec_in([&]() MNL_INLINE{
            val argv[std::size(argv_out)] = {arg0.execute(), arg1.execute(), arg2.execute(), arg3.execute()};
            return MNL_SYM("Repl")(trace_exec_out, _loc, target.exec_out(), std::size(argv), argv, argv_out);
         }());
         return std::move(argv_out[std::size(argv_out) - 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
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

   template<class Cond = code> struct expr_ifelse: code::lvalue {
      Cond cond; code body1, body2; loc _loc;
   public:
      MNL_INLINE val execute(bool fast_sh = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (cast<bool>(cond) ? body1 : body2).execute(fast_sh);
      }
      MNL_INLINE void exec_nores(bool fast_sh = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (cast<bool>(cond) ? body1 : body2).exec_nores(fast_sh);
      }
      MNL_INLINE void exec_in(val &&value) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         (cast<bool>(cond) ? body1 : body2).exec_in(std::move(value));
      }
      MNL_INLINE val exec_out() const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         return (cast<bool>(cond) ? body1 : body2).exec_out();
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return body1.is_lvalue() && body2.is_lvalue();
      }
   };

   template<class Cond = code> struct expr_if: code::rvalue {
      Cond cond; code body; loc _loc;
   public:
      MNL_INLINE decltype(nullptr) execute(bool fast_sh = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (cast<bool>(cond)) body.exec_nores(fast_sh);
         return {};
      }
      MNL_INLINE void exec_nores(bool fast_sh = false) const {
         auto &&cond = this->cond.execute();
         if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
         if (cast<bool>(cond)) body.exec_nores(fast_sh);
      }
   };

   template<class Arg0 = code> struct expr_and: code::rvalue {
      Arg0 arg0; code arg1; loc _loc;
   public:
      MNL_INLINE val execute(bool = {}) const {
         auto &&arg0 = this->arg0.execute();
         if (MNL_LIKELY(!test<bool>(arg0))) {
            val argv[] = {std::move(arg0), arg1.execute()};
            return MNL_SYM("&")(trace_execute, _loc, std::size(argv), argv);
         }
         if (!cast<bool>(arg0))
            return false;
         return [&]() MNL_INLINE{ // RVO
            val arg1 = this->arg1.execute(); // NRVO
            if (MNL_UNLIKELY(!test<bool>(arg1))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
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

   template<class Cond = code> struct expr_while: code::rvalue {
      Cond cond; code body; loc _loc;
   public:
      MNL_INLINE decltype(nullptr) execute(bool fast_sh = false) const {
         for (;;) {
            auto &&cond = this->cond.execute(); // no prediction for condition - performing zero iterations for while-loops may be useful
            if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            if (!cast<bool>(cond) || MNL_UNLIKELY(body.exec_nores(fast_sh), sig_state)) return {};
         }
      }
      MNL_INLINE void exec_nores(bool fast_sh = false) const {
         for (;;) {
            auto &&cond = this->cond.execute(); // no prediction for condition - performing zero iterations for while-loops may be useful
            if (MNL_UNLIKELY(!test<bool>(cond))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
            if (!cast<bool>(cond) || MNL_UNLIKELY(body.exec_nores(fast_sh), sig_state)) return;
         }
      }
   };

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
