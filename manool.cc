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

namespace aux { namespace pub { // Temporary Variables (or Temporaries)
   // Compile-time
   MNL_IF_WITH_MT(thread_local) decltype(tmp_cnt) tmp_cnt; // count for current frame layout
   MNL_IF_WITH_MT(thread_local) decltype(tmp_ids) tmp_ids; // all temporaries
   // Run-time
   MNL_IF_WITH_MT(thread_local) decltype(tmp_stk) tmp_stk; // stack
   MNL_IF_WITH_MT(thread_local) decltype(tmp_frm) tmp_frm; // frame pointer
}} // namespace aux::pub

   namespace aux {
      code optimize(expr_lit<>);
      code optimize(expr_apply0<>), optimize(expr_apply1<>), optimize(expr_apply2<>), optimize(expr_apply3<>), optimize(expr_apply4<>);
   }

   code pub::make_lit(const val &value) { return optimize(expr_lit<>{value}); }

   code aux::compile_apply(code &&target, const form &form, const loc &_loc) {
      if (form.size() - 1 > val::max_argc) MNL_ERR(MNL_SYM("LimitExceeded"));
      struct arg_vector: vector<val> { using vector::vector; ~arg_vector() { while (!empty()) pop_back(); } };
   opt1: // Application without input-output parameters
      for (auto &&el: form + 1)
         if (!el.is_list() || el.empty() || el[0] != MNL_SYM("?")); else goto opt2;
      {  vector<code> args; args.reserve(form.size() - 1);
         for (auto &&el: form + 1) args.push_back(compile_rval(el, _loc));

         struct expr_apply5 { MNL_LVALUE(target.is_lvalue())
            code target, a0, a1, a2, a3, a4; loc _loc;
         public:
            MNL_INLINE val execute(bool) const {
               val argv[]{a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute()}, target = this->target.execute();
               try { return move(target)(std::extent<decltype(argv)>::value, argv); } catch (...) { trace_execute(_loc); }
            }
            MNL_INLINE void exec_in(val &&value) const {
               target.exec_in([&]()->val{
                  val argv[]{move(value), a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), target.exec_out()};
                  argv->swap(argv[std::extent<decltype(argv)>::value - 1]);
                  try { return MNL_SYM("Repl")(std::extent<decltype(argv)>::value, argv); } catch (...) { trace_exec_in(_loc); }
               }());
            }
            MNL_INLINE val exec_out() const {
               val argv_out[7];
               target.exec_in([&]()->val{
                  val argv[]{{}, a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), target.exec_out()};
                  argv->swap(argv[std::extent<decltype(argv)>::value - 1]);
                  try { return MNL_SYM("Repl")(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_exec_out(_loc); }
               }());
               return move(argv_out[std::extent<decltype(argv_out)>::value - 1]);
            }
         };
         struct expr_apply6 { MNL_LVALUE(target.is_lvalue())
            code target, a0, a1, a2, a3, a4, a5; loc _loc;
         public:
            MNL_INLINE val execute(bool) const {
               val argv[]{a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute()}, target = this->target.execute();
               try { return move(target)(std::extent<decltype(argv)>::value, argv); } catch (...) { trace_execute(_loc); }
            }
            MNL_INLINE void exec_in(val &&value) const {
               target.exec_in([&]()->val{
                  val argv[]{move(value), a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute(), target.exec_out()};
                  argv->swap(argv[std::extent<decltype(argv)>::value - 1]);
                  try { return MNL_SYM("Repl")(std::extent<decltype(argv)>::value, argv); } catch (...) { trace_exec_in(_loc); }
               }());
            }
            MNL_INLINE val exec_out() const {
               val argv_out[8];
               target.exec_in([&]()->val{
                  val argv[]{{}, a0.execute(), a1.execute(), a2.execute(), a3.execute(), a4.execute(), a5.execute(), target.exec_out()};
                  argv->swap(argv[std::extent<decltype(argv)>::value - 1]);
                  try { return MNL_SYM("Repl")(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_exec_out(_loc); }
               }());
               return move(argv_out[std::extent<decltype(argv_out)>::value - 1]);
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
         struct expr_apply { MNL_LVALUE(target.is_lvalue())
            code target; vector<code> args; loc _loc; // implementation-defined destruction order for "args"
         public:
            MNL_INLINE val execute(bool) const {
               arg_vector argv; argv.reserve(args.size());
               for (auto &&el: args) argv.push_back(el.execute()); val target = this->target.execute();
               try { return move(target)(argv.size(), argv.data()); } catch (...) { trace_execute(_loc); }
            }
            MNL_INLINE void exec_in(val &&value) const {
               target.exec_in([&]()->val{
                  arg_vector argv; argv.reserve(args.size() + 2); argv.push_back(move(value));
                  for (auto &&el: args) argv.push_back(el.execute()); argv.push_back(target.exec_out()), argv.front().swap(argv.back());
                  try { return MNL_SYM("Repl")(argv.size(), argv.data()); } catch (...) { trace_exec_in(_loc); }
               }());
            }
            MNL_INLINE val exec_out() const {
               arg_vector argv_out(args.size() + 2);
               target.exec_in([&]()->val{
                  arg_vector argv; argv.reserve(args.size() + 2); argv.push_back({});
                  for (auto &&el: args) argv.push_back(el.execute()); argv.push_back(target.exec_out()), argv.front().swap(argv.back());
                  try { return MNL_SYM("Repl")(argv.size(), argv.data(), argv_out.data()); } catch (...) { trace_exec_out(_loc); }
               }());
               return move(argv_out.back());
            }
         };
         return expr_apply{move(target), move(args), _loc};
      }
   opt2: // Application with at least one input-output parameter
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
                  arg_vector argv_out(args.size()); val res = [&]()->val{
                     arg_vector argv; argv.reserve(args.size());
                     for (auto &&el: args) argv.push_back(MNL_UNLIKELY(el.second) ? el.first.exec_out() : el.first.execute()); val target = this->target.execute();
                     try { return move(target)(argv.size(), argv.data(), argv_out.data()); } catch (...) { trace_execute(_loc); }
                  }();
                  auto it1 = args.end(); for (auto it2 = argv_out.rbegin(); it2 != argv_out.rend(); ++it2)
                     if (MNL_UNLIKELY((--it1)->second)) it1->first.exec_in(move(*it2));
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
   code expr_export::compile(code &&, const pub::form &form, const loc &_loc) const {
      if (form.size() < 3 || form[1] != MNL_SYM("in")) err_compile("invalid form", _loc);
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

} // namespace MNL_AUX_UUID
