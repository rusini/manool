// manool.cc -- MANOOL plug-in API

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "base.tcc"

# include <cstdio> // stderr, fprintf, fflush
# include <deque>

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::_Exit; // <cstdlib>
      using std::fprintf; using std::fflush; // <cstdio>
      using std::deque;
   }

   // Compile-time accounting
   MNL_IF_WITH_MT(thread_local) decltype(tmp_cnt) pub::tmp_cnt; // count for current frame layout
   MNL_IF_WITH_MT(thread_local) decltype(tmp_ids) pub::tmp_ids; // all temporaries
   // Run-time accounting
   MNL_IF_WITH_MT(thread_local) decltype(tmp_stk) pub::tmp_stk; // stack
   MNL_IF_WITH_MT(thread_local) decltype(tmp_frm) pub::tmp_frm; // frame pointer

   namespace aux {
      code optimize(expr_lit<>);
      code optimize(expr_apply0<>), optimize(expr_apply1<>), optimize(expr_apply2<>), optimize(expr_apply3<>), optimize(expr_apply4<>);
   }

   code pub::make_lit(const val &value) { return optimize(expr_lit<>{value}); }


   struct expr_applyN: code::lvalue {
      code target; std::vector<code> args; loc _loc; // implementation-defined destruction order for "args"
   public:
      MNL_INLINE val execute(bool) const {
         int argc = args.size(); auto args = this->args.data();
         val argv[argc];
         for (int sn = 0; sn < argc; ++sn) args[sn].execute().swap(argv[sn]); val target = this->target.execute();
         try { return move(target)(argc, argv); } catch (...) { trace_execute(_loc); }
      }
      MNL_INLINE void exec_nores(bool = {}) const {
         execute();
      }
      MNL_INLINE void exec_in(val &&value) const {
         int argc = args.size(); auto args = this->args.data();
         target.exec_in([&]()->val{
            val argv[argc + 2];
            for (int sn = 0; sn < argc; ++sn) args[sn].execute().swap(argv[sn + 1]); target.exec_out().swap(argv[0]); value.swap(argv[argc + 1]);
            try { return MNL_SYM("Repl")(argc, argv); } catch (...) { trace_exec_in(_loc); }
         }());
      }
      MNL_INLINE val exec_out() const {
         int argc = args.size(); auto args = this->args.data();
         val argv_out[argc + 2];
         target.exec_in([&]()->val{
            val argv[argc + 2];
            for (int sn = 0; sn < argc; ++sn) args[sn].execute().swap(argv[sn + 1]); target.exec_out().swap(argv[0]);
            try { return MNL_SYM("Repl")(argc, argv, argv_out); } catch (...) { trace_exec_out(_loc); }
         }());
         return move(argv_out[argc + 1]);
      }
      MNL_INLINE bool is_lvalue() const noexcept {
         return target.is_lvalue();
      }
   };

   static code optimize(expr_applyN expr) {
      struct expr_apply5: code::lvalue {
         code target, a0, a1, a2, a3, a4; loc _loc;
      public:
         MNL_INLINE val execute(bool) const {
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute()};
            return target.execute()(trace_execute, _loc, std::size(argv), argv);
         }
         MNL_INLINE void exec_nores(bool = {}) const {
            execute();
         }
         MNL_INLINE void exec_in(val &&value) const {
            target.exec_in([&]() MNL_INLINE{
               val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), std::move(value)};
               return target.exec_out().repl(trace_exec_in, _loc, std::size(argv), argv);
            }());
         }
         MNL_INLINE val exec_out() const {
            val argv_out[6];
            target.exec_in([&]() MNL_INLINE{
               val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute()};
               return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
            }());
            return std::move(argv_out[std::size(argv_out) - 1]);
         }
         MNL_INLINE bool is_lvalue() const noexcept {
            return target.is_lvalue();
         }
      };
      struct expr_apply6: code::lvalue {
         code target, a0, a1, a2, a3, a4, a5; loc _loc;
      public:
         MNL_INLINE val execute(bool) const {
            val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute()};
            return target.execute()(trace_execute, _loc, std::size(argv), argv);
         }
         MNL_INLINE void exec_nores(bool = {}) const {
            execute();
         }
         MNL_INLINE void exec_in(val &&value) const {
            target.exec_in([&]() MNL_INLINE{
               val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute(), std::move(value)};
               return target.exec_out().repl(trace_exec_in, _loc, std::size(argv), argv);
            }());
         }
         MNL_INLINE val exec_out() const {
            val argv_out[7];
            target.exec_in([&]() MNL_INLINE{
               val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute()};
               return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
            }());
            return std::move(argv_out[std::size(argv_out) - 1]);
         }
         MNL_INLINE bool is_lvalue() const noexcept {
            return target.is_lvalue();
         }
      };
      switch (expr.args.size()) {
      case 0: return optimize(expr_apply<0>{move(target), _loc});
      case 1: return optimize(expr_apply<1>{move(target), move(args[0]), _loc});
      case 2: return optimize(expr_apply2<>{move(target), move(args[0]), move(args[1]), _loc});
      case 3: return optimize(expr_apply3<>{move(target), move(args[0]), move(args[1]), move(args[2]), _loc});
      case 4: return optimize(expr_apply4<>{move(target), move(args[0]), move(args[1]), move(args[2]), move(args[3]), _loc});
      case 5: return expr_apply5{_mv(expr.target), _mv(expr.args[0]), _mv(expr.args[1]), _mv(expr.args[2]), _mv(expr.args[3]), _mv(expr.args[4]), _loc};
      case 6: return expr_apply6{_mv(expr.target),
         std::move(expr.args[0]), std::move(expr.args[1]), std::move(expr.args[2]), std::move(expr.args[3]), std::move(expr.args[4]), std::move(expr.args[5]), _loc};
      }
      return std::move(expr);
   }


   code aux::compile_apply(code &&target, const form &form, const loc &_loc) {
      if (form.size() - 1 > val::max_argc) MNL_ERR(MNL_SYM("LimitExceeded"));
   opt1: // Application without input/output arguments
      for (auto &&el: form + 1)
         if (!el.is_list() || el.empty() || el[0] != MNL_SYM("?")); else goto opt2;
      {  vector<code> args; args.reserve(form.size() - 1);
         for (auto &&el: form + 1) args.push_back(compile_rval(el, _loc));

         struct expr_apply5: code::lvalue {
            code target, a0, a1, a2, a3, a4; loc _loc;
         public:
            MNL_INLINE val execute(bool) const {
               val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute()};
               return target.execute()(trace_execute, _loc, std::size(argv), argv);
            }
            MNL_INLINE void exec_nores(bool = {}) const {
               execute();
            }
            MNL_INLINE void exec_in(val &&value) const {
               target.exec_in([&]() MNL_INLINE{
                  val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), std::move(value)};
                  return target.exec_out().repl(trace_exec_in, _loc, std::size(argv), argv);
               }());
            }
            MNL_INLINE val exec_out() const {
               val argv_out[6];
               target.exec_in([&]() MNL_INLINE{
                  val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute()};
                  return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
               }());
               return std::move(argv_out[std::size(argv_out) - 1]);
            }
            MNL_INLINE bool is_lvalue() const noexcept {
               return target.is_lvalue();
            }
         };
         struct expr_apply6: code::lvalue {
            code target, a0, a1, a2, a3, a4, a5; loc _loc;
         public:
            MNL_INLINE val execute(bool) const {
               val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute()};
               return target.execute()(trace_execute, _loc, std::size(argv), argv);
            }
            MNL_INLINE void exec_nores(bool = {}) const {
               execute();
            }
            MNL_INLINE void exec_in(val &&value) const {
               target.exec_in([&]() MNL_INLINE{
                  val argv[] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute(), std::move(value)};
                  return target.exec_out().repl(trace_exec_in, _loc, std::size(argv), argv);
               }());
            }
            MNL_INLINE val exec_out() const {
               val argv_out[7];
               target.exec_in([&]() MNL_INLINE{
                  val argv[std::size(argv_out)] = {a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute()};
                  return target.exec_out().repl(trace_exec_out, _loc, std::size(argv), argv, argv_out);
               }());
               return std::move(argv_out[std::size(argv_out) - 1]);
            }
            MNL_INLINE bool is_lvalue() const noexcept {
               return target.is_lvalue();
            }
         };
         switch (args.size()) {
         case 0: return optimize(expr_apply0<>{move(target), _loc});
         case 1: return optimize(expr_apply1<>{move(target), move(args[0]), _loc});
         case 2: return optimize(expr_apply2<>{move(target), move(args[0]), move(args[1]), _loc});
         case 3: return optimize(expr_apply3<>{move(target), move(args[0]), move(args[1]), move(args[2]), _loc});
         case 4: return optimize(expr_apply4<>{move(target), move(args[0]), move(args[1]), move(args[2]), move(args[3]), _loc});
         case 5: return expr_apply5{move(target), move(args[0]), move(args[1]), move(args[2]), move(args[3]), move(args[4]), _loc};
         case 6: return expr_apply6{move(target), move(args[0]), move(args[1]), move(args[2]), move(args[3]), move(args[4]), move(args[5]), _loc};
         }
         struct expr_apply: code::lvalue {
            code target; vector<code> args; loc _loc; // implementation-defined destruction order for "args"
         public:
            MNL_INLINE val execute(bool) const {
               int argc = args.size(); auto args = this->args.data();
               val argv[argc];
               for (int sn = 0; sn < argc; ++sn) args[sn].execute().swap(argv[sn]); val target = this->target.execute();
               try { return move(target)(argc, argv); } catch (...) { trace_execute(_loc); }
            }
            MNL_INLINE void exec_nores(bool = {}) const {
               execute();
            }
            MNL_INLINE void exec_in(val &&value) const {
               int argc = args.size(); auto args = this->args.data();
               target.exec_in([&]()->val{
                  val argv[argc + 2];
                  for (int sn = 0; sn < argc; ++sn) args[sn].execute().swap(argv[sn + 1]); target.exec_out().swap(argv[0]); value.swap(argv[argc + 1]);
                  try { return MNL_SYM("Repl")(argc, argv); } catch (...) { trace_exec_in(_loc); }
               }());
            }
            MNL_INLINE val exec_out() const {
               int argc = args.size(); auto args = this->args.data();
               val argv_out[argc + 2];
               target.exec_in([&]()->val{
                  val argv[argc + 2];
                  for (int sn = 0; sn < argc; ++sn) args[sn].execute().swap(argv[sn + 1]); target.exec_out().swap(argv[0]);
                  try { return MNL_SYM("Repl")(argc, argv, argv_out); } catch (...) { trace_exec_out(_loc); }
               }());
               return move(argv_out[argc + 1]);
            }
            MNL_INLINE bool is_lvalue() const noexcept {
               return target.is_lvalue();
            }
         };
         return expr_apply{move(target), move(args), _loc};
      }
   opt2: // Application with at least one input/output argument
      for (auto &&el: form + 1)
         if (!el.is_list() || el.empty() || el[0] != MNL_SYM("?") || el.size() == 2); else goto opt3;
      {  vector<pair<code, bool>> args; args.reserve(form.size() - 1);
         for (auto &&el: form + 1) args.push_back(el.is_list() && el.size() == 2 && el[0] == MNL_SYM("?") ?
            make_pair(compile_lval(el[1], _loc), true) : make_pair(compile_rval(el, _loc), false));

         switch (args.size()) {
            {  struct expr { MNL_RVALUE() // 1 argument
                  code target, args[1]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:                  // "const" might help Alias Analysis and it's definitely harmless here (but in fact, compilers seem to ignore it)
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 1: return expr{move(target),
                  move(args[0].first),
                  move(args[0].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 2 arguments
                  code target, args[2]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 2: return expr{move(target),
                  move(args[0].first),  move(args[1].first),
                  move(args[0].second), move(args[1].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 3 arguments
                  code target, args[3]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute(),
                           MNL_UNLIKELY(mods[2]) ? args[2].exec_out() : args[2].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[2])) args[2].exec_in(move(argv_out[2]));
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 3: return expr{move(target),
                  move(args[0].first),  move(args[1].first),  move(args[2].first),
                  move(args[0].second), move(args[1].second), move(args[2].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 4 arguments
                  code target, args[4]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute(),
                           MNL_UNLIKELY(mods[2]) ? args[2].exec_out() : args[2].execute(),
                           MNL_UNLIKELY(mods[3]) ? args[3].exec_out() : args[3].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[3])) args[3].exec_in(move(argv_out[3]));
                     if (MNL_UNLIKELY(mods[2])) args[2].exec_in(move(argv_out[2]));
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 4: return expr{move(target),
                  move(args[0].first),  move(args[1].first),  move(args[2].first),  move(args[3].first),
                  move(args[0].second), move(args[1].second), move(args[2].second), move(args[3].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 5 arguments
                  code target, args[5]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute(),
                           MNL_UNLIKELY(mods[2]) ? args[2].exec_out() : args[2].execute(),
                           MNL_UNLIKELY(mods[3]) ? args[3].exec_out() : args[3].execute(),
                           MNL_UNLIKELY(mods[4]) ? args[4].exec_out() : args[4].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[4])) args[4].exec_in(move(argv_out[4]));
                     if (MNL_UNLIKELY(mods[3])) args[3].exec_in(move(argv_out[3]));
                     if (MNL_UNLIKELY(mods[2])) args[2].exec_in(move(argv_out[2]));
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 5: return expr{move(target),
                  move(args[0].first),  move(args[1].first),  move(args[2].first),  move(args[3].first),  move(args[4].first),
                  move(args[0].second), move(args[1].second), move(args[2].second), move(args[3].second), move(args[4].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 6 arguments
                  code target, args[6]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute(),
                           MNL_UNLIKELY(mods[2]) ? args[2].exec_out() : args[2].execute(),
                           MNL_UNLIKELY(mods[3]) ? args[3].exec_out() : args[3].execute(),
                           MNL_UNLIKELY(mods[4]) ? args[4].exec_out() : args[4].execute(),
                           MNL_UNLIKELY(mods[5]) ? args[5].exec_out() : args[5].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[5])) args[5].exec_in(move(argv_out[5]));
                     if (MNL_UNLIKELY(mods[4])) args[4].exec_in(move(argv_out[4]));
                     if (MNL_UNLIKELY(mods[3])) args[3].exec_in(move(argv_out[3]));
                     if (MNL_UNLIKELY(mods[2])) args[2].exec_in(move(argv_out[2]));
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 6: return expr{move(target),
                  move(args[0].first),  move(args[1].first),  move(args[2].first),
                  move(args[3].first),  move(args[4].first),  move(args[5].first),
                  move(args[0].second), move(args[1].second), move(args[2].second),
                  move(args[3].second), move(args[4].second), move(args[5].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 7 arguments
                  code target, args[7]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute(),
                           MNL_UNLIKELY(mods[2]) ? args[2].exec_out() : args[2].execute(),
                           MNL_UNLIKELY(mods[3]) ? args[3].exec_out() : args[3].execute(),
                           MNL_UNLIKELY(mods[4]) ? args[4].exec_out() : args[4].execute(),
                           MNL_UNLIKELY(mods[5]) ? args[5].exec_out() : args[5].execute(),
                           MNL_UNLIKELY(mods[6]) ? args[6].exec_out() : args[6].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[6])) args[6].exec_in(move(argv_out[6]));
                     if (MNL_UNLIKELY(mods[5])) args[5].exec_in(move(argv_out[5]));
                     if (MNL_UNLIKELY(mods[4])) args[4].exec_in(move(argv_out[4]));
                     if (MNL_UNLIKELY(mods[3])) args[3].exec_in(move(argv_out[3]));
                     if (MNL_UNLIKELY(mods[2])) args[2].exec_in(move(argv_out[2]));
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 7: return expr{move(target),
                  move(args[0].first),  move(args[1].first),  move(args[2].first),
                  move(args[3].first),  move(args[4].first),  move(args[5].first),  move(args[6].first),
                  move(args[0].second), move(args[1].second), move(args[2].second),
                  move(args[3].second), move(args[4].second), move(args[5].second), move(args[6].second), _loc};
            }
            {  struct expr { MNL_RVALUE() // 8 arguments
                  code target, args[8]; const bool mods[std::extent<decltype(args)>::value]; loc _loc;
               public:
                  MNL_INLINE val execute(bool) const {
                     val argv_out[std::extent<decltype(args)>::value], res = [&]()->val{
                        val argv[]{
                           MNL_UNLIKELY(mods[0]) ? args[0].exec_out() : args[0].execute(),
                           MNL_UNLIKELY(mods[1]) ? args[1].exec_out() : args[1].execute(),
                           MNL_UNLIKELY(mods[2]) ? args[2].exec_out() : args[2].execute(),
                           MNL_UNLIKELY(mods[3]) ? args[3].exec_out() : args[3].execute(),
                           MNL_UNLIKELY(mods[4]) ? args[4].exec_out() : args[4].execute(),
                           MNL_UNLIKELY(mods[5]) ? args[5].exec_out() : args[5].execute(),
                           MNL_UNLIKELY(mods[6]) ? args[6].exec_out() : args[6].execute(),
                           MNL_UNLIKELY(mods[7]) ? args[7].exec_out() : args[7].execute() }, target = this->target.execute();
                        try { return move(target)(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_execute(_loc); }
                     }();
                     if (MNL_UNLIKELY(mods[7])) args[7].exec_in(move(argv_out[7]));
                     if (MNL_UNLIKELY(mods[6])) args[6].exec_in(move(argv_out[6]));
                     if (MNL_UNLIKELY(mods[5])) args[5].exec_in(move(argv_out[5]));
                     if (MNL_UNLIKELY(mods[4])) args[4].exec_in(move(argv_out[4]));
                     if (MNL_UNLIKELY(mods[3])) args[3].exec_in(move(argv_out[3]));
                     if (MNL_UNLIKELY(mods[2])) args[2].exec_in(move(argv_out[2]));
                     if (MNL_UNLIKELY(mods[1])) args[1].exec_in(move(argv_out[1]));
                     if (MNL_UNLIKELY(mods[0])) args[0].exec_in(move(argv_out[0]));
                     return res;
                  }
               };
            case 8: return expr{move(target),
                  move(args[0].first),  move(args[1].first),  move(args[2].first),  move(args[3].first),
                  move(args[4].first),  move(args[5].first),  move(args[6].first),  move(args[7].first),
                  move(args[0].second), move(args[1].second), move(args[2].second), move(args[3].second),
                  move(args[4].second), move(args[5].second), move(args[6].second), move(args[7].second), _loc};
            }
         }
         {  struct expr { MNL_RVALUE() // more than 8 arguments
               code target; vector<pair<code, bool>> args; loc _loc; // implementation-defined destruction order for "args"
            public:
               MNL_INLINE val execute(bool) const {
                  int argc = args.size(); auto args = this->args.data();
                  val argv_out[argc]; val res = [&]()->val{
                     val argv[argc];
                     for (int sn = 0; sn < argc; ++sn) (MNL_UNLIKELY(args[sn].second) ? args[sn].first.exec_out() : args[sn].first.execute()).swap(argv[sn]);
                     val target = this->target.execute();
                     try { return move(target)(argc, argv, argv_out); } catch (...) { trace_execute(_loc); }
                  }();
                  for (int sn = argc; sn--;) if (MNL_UNLIKELY(args[sn].second)) args[sn].first.exec_in(move(argv_out[sn]));
                  return res;
               }
            };
            return expr{move(target), move(args), _loc};
         }
      }
   opt3:
      err_compile("invalid form", _loc);
   }

   code pub::compile_rval(const form &form, const loc &_loc) {
      auto res = compile(form, _loc);
      if (!res.is_rvalue()) err_compile("not an R-value expression (nested in this context)", _loc);
      return res;
   }
   code pub::compile_lval(const form &form, const loc &_loc) {
      auto res = compile(form, _loc);
      if (!res.is_lvalue()) err_compile("not an L-value expression (nested in this context)", _loc);
      return res;
   }
   sym  pub::eval_sym(const form &form, const loc &_loc) {
      auto res = compile_rval(form, _loc).execute();
      if (!test<sym>(res)) err_compile("type mismatch", _loc);
      return cast<const sym &>(res);
   }
   code pub::compile_rval(form::vci_range range, const loc &_loc) {
      struct expr_seq { MNL_LVALUE(second.is_lvalue())
         code first, second;
         MNL_INLINE val execute(bool fast_sig) const { if (MNL_UNLIKELY(first.execute(fast_sig), sig_state.first)) return {}; return second.execute(fast_sig); }
         MNL_INLINE void exec_in(val &&value) const { first.execute(); second.exec_in(move(value)); }
         MNL_INLINE val exec_out() const { first.execute(); return second.exec_out(); }
      };
      auto res = compile_rval(*range.begin(), _loc);
      for (auto &&el: form::vci_range{range.begin() + 1, range.end()}) res = expr_seq{move(res), compile_rval(el, _loc)};
      return res;
   }

   int val::default_order(const val &rhs) const noexcept {
      auto mask1 = MNL_AUX_RAND(unsigned);
      auto mask2 = MNL_AUX_RAND(size_t);
      bool mask3 = MNL_AUX_RAND(unsigned) & 1;
      return
      MNL_LIKELY(rep.tag() != 0x7FF8u) || MNL_LIKELY(rhs.rep.tag() != 0x7FF8u) ?
         (((~rep.tag() & 0x7FF0u ? 0 : rep.tag()) ^ mask1) < ((~rhs.rep.tag() & 0x7FF0u ? 0 : rhs.rep.tag()) ^ mask1)) -
         (((~rhs.rep.tag() & 0x7FF0u ? 0 : rhs.rep.tag()) ^ mask1) < ((~rep.tag() & 0x7FF0u ? 0 : rep.tag()) ^ mask1)) :
      MNL_LIKELY(!test<object>()) || MNL_LIKELY(!rhs.test<object>()) ?
         (typeid(*static_cast<root *>(rep.dat<void *>())).hash_code() ^ mask2) < (typeid(*static_cast<root *>(rhs.rep.dat<void *>())).hash_code() ^ mask2) ? -1 :
         (typeid(*static_cast<root *>(rhs.rep.dat<void *>())).hash_code() ^ mask2) < (typeid(*static_cast<root *>(rep.dat<void *>())).hash_code() ^ mask2) ? +1 :
         typeid(*static_cast<root *>(rep.dat<void *>())).before(typeid(*static_cast<root *>(rhs.rep.dat<void *>()))) ? mask3 ? -1 : +1 :
         typeid(*static_cast<root *>(rhs.rep.dat<void *>())).before(typeid(*static_cast<root *>(rep.dat<void *>()))) ? mask3 ? +1 : -1 : 0 :
      // else
         order(cast<const object &>().descr, rhs.cast<const object &>().descr);
   }

   code expr_export::compile(code &&, const pub::form &form, const loc &_loc) const {
      if (form.size() < 3 || form[1] != MNL_SYM("in")) err_compile("invalid form", _loc);
      if (form.size() == 3 && test<sym>(form[2])) for (auto &&el: bind) if (MNL_UNLIKELY(el.first == cast<const sym &>(form[2]))) return el.second; // shortcut
      deque<code> overriden_ents;
      for (auto &&el: bind) overriden_ents.push_back(symtab[el.first]), symtab.update(el);
      auto body = form.size() == 3 ? pub::compile(form[2], _loc) : compile_rval(form + 2, _loc);
      for (auto &&el: bind) symtab.update(el.first, move(overriden_ents.front())), overriden_ents.pop_front();
      return body;
   }
   template class code::box<expr_export>;

   void aux::panic(const decltype(sig_state) &sig) {
      if (sig.first == MNL_SYM("CompileError")) { // should be uninterned?
         sig_trace.clear();
         if (!test<string>(sig.second)) panic({MNL_SYM("TypeMismatch"), {}});
         fprintf(stderr, "%s\n", cast<const string &>(sig.second).c_str());
      } else {
         fprintf(stderr, "Uncaught signal %s\n", (const char *)sig.first);
         if (sig_trace.empty())
            fprintf(stderr, "*** invocation backtrace empty ***\n");
         else {
            fprintf(stderr, "====== invocation backtrace ======\n");
            int sn = 0;
            for (auto &&el: sig_trace) {
               if (sn == sig_trace.capacity() - 1) break;
               fprintf(stderr, "%02d at (%s) %d:%d-%d:%d %s\n", sn++,
                  el.first.origin->c_str(), el.first._start.first, el.first._start.second, el.first._final.first, el.first._final.second - 1, el.second);
            }
            if (sn != sig_trace.capacity() - 1)
               fprintf(stderr, "======== end of backtrace ========\n");
            else
               fprintf(stderr, "==== more invocations omitted ====\n");
         }
      }
      fflush(stderr), _Exit(EXIT_FAILURE);
   }




constexpr struct tstack {
public:
   MNL_INLINE static void init() noexcept { done; }
public:
   MNL_INLINE template<typename Rsh = decltype(nullptr)> static void push(Rsh &&rsh = {}) { // essential
      if (MNL_UNLIKELY(top == limit)) []() MNL_NOINLINE{
         std::size_t
            top   = reinterpret_cast<std::byte *>(tstack::top)   - reinterpret_cast<std::byte *>(bottom),
            limit = MNL_LIKELY(top) ? top << 1 : 4/*KiB*/ * (1 << 10), // starting from 1 "typical" page
            frame = reinterpret_cast<std::byte *>(tstack::frame) - reinterpret_cast<std::byte *>(bottom);
         bottom = static_cast<val *>(aux::realloc(bottom, limit)); // may throw like ::new TODO: 3/2 exp base
         tstack::top   = reinterpret_cast<val *>(reinterpret_cast<std::byte *>(bottom) + top);
         tstack::limit = reinterpret_cast<val *>(reinterpret_cast<std::byte *>(bottom) + limit);
         tstack::frame = reinterpret_cast<val *>(reinterpret_cast<std::byte *>(bottom) + frame);
      }();
      new(top++) val(std::forward<Rhs>(rsh));
   }
   MNL_INLINE static void pop() noexcept { // essential
      (--top)->~val();
   }
public: // convenience
   MNL_INLINE template<typename Rsh = decltype(nullptr)> static void push(const Rsh &rsh, int count) { for (; MNL_LIKELY(count); --count) push(rsh); }
   MNL_INLINE static void pop(int count) noexcept                                                    { for (; MNL_LIKELY(count); --count) pop(); }
public:
   MNL_INLINE val &operator[](int index) const noexcept { return frame[index]; }
public:
   MNL_INLINE static std::size_t enter() noexcept
      { std::size_t frame = reinterpret_cast<std::byte *>(tstack::frame) - reinterpret_cast<std::byte *>(bottom); tstack::frame = top; return frame; }
   MNL_INLINE static void leave(std::size_t frame) noexcept
      { tstack::frame = reinterpret_cast<val *>(reinterpret_cast<std::byte *>(bottom) + frame); }
private:
   static thread_local val *bottom, *top, *limit;
   static thread_local val *frame;
   static thread_local const class done { public: MNL_INLINE ~done() { ::free(bottom); } } done;
} tstack = {};

inline thread_local val *tstack::bottom, *tstack::top, *tstack::limit;
inline thread_local val *tstack::frame;
inline thread_local const class tstack::done tstack::done;




} // namespace MNL_AUX_UUID
