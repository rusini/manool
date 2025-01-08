// lib-base-main2.cc -- base facilities

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-lib-base.hh"
# include "base.tcc"

# include <deque>
# include <exception>

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::make_shared; // <memory>
      using std::deque; using std::list;
   }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux {
   code optimize(expr_lit<>);
   code optimize(expr_set<>), optimize(expr_move<>);
   code optimize(expr_ifelse<>), optimize(expr_if<>), optimize(expr_and<>), optimize(expr_or<>), optimize(expr_while<>), optimize(expr_on<>);
   code optimize(expr_att);
} // namespace aux

namespace aux { namespace {

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   class comp_set { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 3) err_compile("invalid form", _loc);
         return optimize(expr_set<>{compile_lval(form[1], _loc), compile_rval(form[2], _loc)});
      }
   };

   class comp_move { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2) err_compile("invalid form", _loc);
         return optimize(expr_move<>{compile_lval(form[1], _loc)});
      }
   };

   class comp_deref { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2) err_compile("invalid form", _loc);
         struct expr { MNL_LVALUE(true)
            code arg; loc _loc;
         public:
            MNL_INLINE val execute(bool) const {
               auto arg = this->arg.execute();
               try { return MNL_SYM("^")(move(arg)); } catch (...) { trace_execute(_loc); }
            }
            MNL_INLINE void exec_in(val &&value) const {
               val argv[]{arg.execute(), move(value)};
               try { MNL_SYM("Set")(std::extent<decltype(argv)>::value, argv); } catch (...) { trace_exec_in(_loc); }
            }
            MNL_INLINE val exec_out() const {
               val argv_out[2], argv[2]{arg.execute()};
               try { MNL_SYM("Set")(std::extent<decltype(argv)>::value, argv, argv_out); } catch (...) { trace_exec_out(_loc); }
               return move(argv_out[1]);
            }
         };
         return expr{compile_rval(form[1], _loc), _loc};
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   struct comp_if: code::nonvalue {
      MNL_INLINE static code compile(code &&, const ast &form, const loc &loc) {
      opt1: // {if C then B else B}
         if (form.size() == 6); else goto opt2;
         if (form[2] == MNL_SYM("then")); else goto opt2;
         if (form[4] == MNL_SYM("else")); else goto opt2;
         return optimize(expr_ifelse<>{compile_rval(form[1], loc), compile_rval(form[3], loc), compile_rval(form[5], loc), loc});
      opt2: // {if C then B}
         if (form.size() == 4); else goto opt3;
         if (form[2] == MNL_SYM("then")); else goto opt3;
         return optimize(expr_if<>{compile_rval(form[1], loc), compile_rval(form[3], loc), loc});
      opt3:
         err_compile("invalid form", loc);
      }
   };

   struct comp_and: code::nonvalue {
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() == 3) err_compile("invalid form", _loc);
         return optimize(expr_and<>{compile_rval(form[1], _loc), compile_rval(form[2], _loc), _loc});
      }
   };

   struct comp_or:  code::nonvalue {
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() == 3) err_compile("invalid form", _loc);
         return optimize(expr_or<> {compile_rval(form[1], _loc), compile_rval(form[2], _loc), _loc});
      }
   };

   struct comp_while: code::nonvalue {
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {while C do B}
         if (form.size() == 4); else goto opt2;
         if (form[2] == MNL_SYM("do")); else goto opt2;
         return optimize(expr_while<>{compile_rval(form[1], _loc), compile_rval(form[3], _loc), _loc});
      opt2:
         err_compile("invalid form", _loc);
      }
   };

   class comp_repeat { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {repeat N do B; B; ...}
         if (form.size() >= 4); else goto opt2;
         if (form[2] == MNL_SYM("do")); else goto opt2;
         {  struct expr { MNL_RVALUE()
               code count, body; loc _loc;
               MNL_INLINE val execute(bool fast_sig) const {
                  auto count = safe_cast<long long>(_loc, this->count.execute());
                  if (MNL_UNLIKELY(count < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                  for (MNL_IF_WITH_MT(auto &sig_state = mnl::sig_state); count && (body.execute(fast_sig), !sig_state.first); --count);
                  return {};
               }
            };
            return expr{compile_rval(form[1], _loc), compile_rval(form + 3, _loc), _loc};
         }
      opt2: // {repeat B; B; ...}
         if (form.size() >= 2); else goto opt2;
         {  struct expr { MNL_RVALUE()
               code body;
               MNL_INLINE val execute(bool fast_sig) const {
                  for (MNL_IF_WITH_MT(auto &sig_state = mnl::sig_state); body.execute(fast_sig), !sig_state.first;);
                  return {};
               }
            };
            return expr{compile_rval(form + 1, _loc)};
         }
      opt3:
         err_compile("invalid form", _loc);
      }
   };

   class comp_on { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {on K with I do B after B; B; ...}
         if (form.size() >= 8); else goto opt2;
         if (form[2] == MNL_SYM("with")); else goto opt2;
         if (form[4] == MNL_SYM("do")); else goto opt2;
         if (form[6] == MNL_SYM("after")); else goto opt2;
         if (test<sym>(form[3])); else goto opt2;
         if (tmp_cnt == lim<decltype(tmp_cnt)>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         {  auto key = compile_rval(form[1], _loc);
            auto overriden_ent = symtab[cast<const sym &>(form[3])]; symtab.update(cast<const sym &>(form[3]), expr_tmp{tmp_cnt++});
            auto inserted_tmp_id = tmp_ids.insert(cast<const sym &>(form[3])).second;
            auto trap = compile_rval(form[5], _loc);
            if (inserted_tmp_id) tmp_ids.erase(cast<const sym &>(form[3]));
            --tmp_cnt, symtab.update(cast<const sym &>(form[3]), move(overriden_ent));
            return optimize(expr_on<>{move(key), move(trap), compile_rval(form + 7, _loc), _loc});
         }
      opt2:
         err_compile("invalid form", _loc);
      }
   };

   class comp_signal { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         struct expr { MNL_RVALUE()
            code key, value; loc _loc;
            MNL_INLINE val execute(bool fast_sig) const {
               sig_state = {safe_cast<const sym &>(_loc, this->key.execute()), value.execute()};
               if (MNL_LIKELY(fast_sig)) return {}; // fast unwinding
               auto sig = move(sig_state); sig_state.first = {}; throw move(sig); // slow unwinding
            }
         };
      opt1: // {signal K with V}
         if (form.size() == 4); else goto opt2;
         if (form[2] == MNL_SYM("with")); else goto opt2;
         return expr{compile_rval(form[1], _loc), compile_rval(form[3], _loc), _loc};
      opt2:
         err_compile("invalid form", _loc);
      }
   };

   class comp_for { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {for {I = V; I = V; ...} do B; B; ...}
         {  if (form.size() >= 4); else goto opt2;
            if (form[1].is_list() && form[1].size() >= 1); else goto opt2;
            if (form[2] == MNL_SYM("do")); else goto opt2;
            for (auto &&el: form[1]) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=") && test<sym>(el[1])); else goto opt2;
         }
         {  if (tmp_cnt + (long)form[1].size() > lim<decltype(tmp_cnt)>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[cast<const sym &>(el[1])])
               tab.update(cast<const sym &>(el[1]), true); else err_compile("ambiguous bindings", _loc);
         }
         {  vector<code> view; view.reserve(form[1].size()); for (auto &&el: form[1]) view.push_back(compile_rval(el[2], _loc));

            deque<code> overriden_ents;
            for (auto &&el: form[1]) overriden_ents.push_back(symtab[cast<const sym &>(el[1])]),
               symtab.update(cast<const sym &>(el[1]), expr_tmp{tmp_cnt++});
            vector<sym> inserted_tmp_ids;
            for (auto &&el: form[1]) if (tmp_ids.insert(cast<const sym &>(el[1])).second) inserted_tmp_ids.push_back(cast<const sym &>(el[1]));

            auto body = compile_rval(form + 3, _loc);

            for (auto &&el: inserted_tmp_ids) tmp_ids.erase(move(el));
            tmp_cnt -= form[1].size();
            for (auto &&el: form[1]) symtab.update(cast<const sym &>(el[1]), move(overriden_ents.front())), overriden_ents.pop_front();

            if (view.size() == 1) {
               struct expr { MNL_RVALUE()
                  code view, body; loc _loc;
                  MNL_INLINE val execute(bool fast_sig) const {
                     auto view = this->view.execute();
                     tmp_stk.push_back({});
                     struct _ { MNL_INLINE ~_() { tmp_stk.pop_back(); } } _;
                     MNL_IF_WITH_MT(auto &tmp_stk = mnl::tmp_stk; auto &sig_state = mnl::sig_state;)
                     if (MNL_UNLIKELY(test<range<>>(view)))
                     for (auto lo = cast<const range<> &>(view).lo, hi = cast<const range<> &>(view).hi;;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() =  lo++, body.execute(fast_sig), sig_state.first )) return {};
                     if (MNL_UNLIKELY(test<range<true>>(view)))
                     for (auto lo = cast<const range<true> &>(view).lo, hi = cast<const range<true> &>(view).hi;;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() =  --hi, body.execute(fast_sig), sig_state.first )) return {};
                     if (MNL_UNLIKELY(test<vector<val>>(view)))
                     for (auto lo = cast<const vector<val> &>(view).begin(), hi = cast<const vector<val> &>(view).end();;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() = *lo++, body.execute(fast_sig), sig_state.first )) return {};
                     // else
                     view = MNL_SYM("Elems")(_loc, move(view));
                     if (MNL_UNLIKELY(test<vector<val>>(view)))
                     for (auto lo = cast<const vector<val> &>(view).begin(), hi = cast<const vector<val> &>(view).end();;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() = *lo++, body.execute(fast_sig), sig_state.first )) return {};
                     if (MNL_UNLIKELY(test<string>(view)))
                     for (auto lo = cast<const string &>(view).begin(), hi = cast<const string &>(view).end();;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() = *lo++, body.execute(fast_sig), sig_state.first )) return {};
                     if (MNL_UNLIKELY(test<range<>>(view)))
                     for (auto lo = cast<const range<> &>(view).lo, hi = cast<const range<> &>(view).hi;;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() =  lo++, body.execute(fast_sig), sig_state.first )) return {};
                     if (MNL_UNLIKELY(test<range<true>>(view)))
                     for (auto lo = cast<const range<true> &>(view).lo, hi = cast<const range<true> &>(view).hi;;)
                        if (!MNL_LIKELY(lo != hi) || MNL_UNLIKELY( tmp_stk.back() =  --hi, body.execute(fast_sig), sig_state.first )) return {};
                     // else
                     auto hi = safe_cast<long long>(_loc, MNL_SYM("Size")(_loc, view));
                     if (MNL_UNLIKELY(hi < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                     for (long long lo = 0; lo < hi; ++lo) {
                        try { tmp_stk.back() = view(_loc, lo); }
                        catch (decltype(::mnl::sig_state) &sig) { if (sig.first == MNL_SYM("EndOfData")) return {}; throw; }
                        if (MNL_UNLIKELY( body.execute(fast_sig), sig_state.first )) return {};
                     }
                     return {};
                  }
               };
               return expr{move(view.front()), move(body), _loc};
            }
            if (view.size() == 2) {
               struct expr { MNL_RVALUE()
                  code view0, view1, body; loc _loc;
                  MNL_INLINE val execute(bool fast_sig) const {
                     auto view0 = MNL_SYM("Elems")(_loc, this->view0.execute()), view1 = MNL_SYM("Elems")(_loc, this->view1.execute());
                     auto size = safe_cast<long long>(_loc, MNL_SYM("Size")(_loc, view0));
                     if (MNL_UNLIKELY(size < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                     {  auto _size = safe_cast<long long>(_loc, MNL_SYM("Size")(_loc, view1));
                        if (MNL_UNLIKELY(_size < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                        if (MNL_UNLIKELY(_size < size)) size = _size;
                     }
                     tmp_stk.resize(tmp_stk.size() + 2);
                     struct _ { MNL_INLINE ~_() { tmp_stk.pop_back(), tmp_stk.pop_back(); } } _;
                     MNL_IF_WITH_MT(auto &tmp_stk = mnl::tmp_stk; auto &sig_state = mnl::sig_state;)
                     for (long long sn = 0; sn < size; ++sn) {
                        try { tmp_stk.end()[-2] = view0(_loc, sn), tmp_stk.end()[-1] = view1(_loc, sn); }
                        catch (decltype(::mnl::sig_state) &sig) { if (sig.first == MNL_SYM("EndOfData")) return {}; throw; }
                        if (MNL_UNLIKELY( body.execute(fast_sig), sig_state.first )) return {};
                     }
                     return {};
                  }
               };
               return expr{move(view[0]), move(view[1]), move(body), _loc};
            }
            {  struct expr { MNL_RVALUE()
                  vector<code> view; code body; loc _loc;
                  MNL_INLINE val execute(bool fast_sig) const {
                     struct view: vector<val> { MNL_INLINE ~view() { while (!empty()) pop_back(); } } view;
                     view.reserve(this->view.size()); for (auto &&el: this->view) view.push_back(MNL_SYM("Elems")(_loc, el.execute()));
                     auto size = safe_cast<long long>(_loc, MNL_SYM("Size")(_loc, view.front()));
                     if (MNL_UNLIKELY(size < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                     for (auto it = view.begin() + 1; it != view.end(); ++it)
                     {  auto _size = safe_cast<long long>(_loc, MNL_SYM("Size")(_loc, *it));
                        if (MNL_UNLIKELY(_size < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                        if (MNL_UNLIKELY(_size < size)) size = _size;
                     }
                     tmp_stk.resize(tmp_stk.size() + view.size());
                     struct _ { int sn; MNL_INLINE ~_() { for (; sn; --sn) tmp_stk.pop_back(); } } _{(int)view.size()};
                     MNL_IF_WITH_MT(auto &tmp_stk = mnl::tmp_stk; auto &sig_state = mnl::sig_state;)
                     for (long long sn1 = 0; sn1 < size; ++sn1) {
                        try { for (int sn2 = 0; sn2 < (int)view.size(); ++sn2) (tmp_stk.end() - view.size())[sn2] = view[sn2](_loc, sn1); }
                        catch (decltype(::mnl::sig_state) &sig) { if (sig.first == MNL_SYM("EndOfData")) return {}; throw; }
                        if (MNL_UNLIKELY( body.execute(fast_sig), sig_state.first )) return {};
                     }
                     return {};
                  }
               };
               return expr{move(view), move(body), _loc};
            }
         }
      opt2:
         err_compile("invalid form", _loc);
      }
   };

   class comp_case: code::nonvalue {
      friend code::box<comp_var>;
      MNL_INLINE static code compile(code &&, const ast &form, const loc &loc) {
      opt1: // {case V of {K = B; ...} else B}
         {  if (form.size() == 6); else goto opt2;
            if (form[2] == MNL_SYM("of")); else goto opt2;
            if (form[4] == MNL_SYM("else")); else goto opt2;
            for (auto &&el: form[3]) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt2;
         }
         {  if (form[3].size() > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  auto key = compile_rval(form[1], _loc);

            std::set<sym> descr; std::vector<code> arms;

            std::deque<code> saved_tmp_ents;
            for (auto &&el: tvar_ids) saved_tvar_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tvar_cnt = move(tvar_cnt); tvar_cnt = 0;
            auto saved_tvar_ids = move(tvar_ids); tvar_ids.clear();
            std::deque<sym> keys; for (auto &&el: form[3])
               if (keys.push_back(eval_sym(el[1], _loc)), !descr.insert(keys.back()).second) err_compile("ambiguous bindings", _loc);
            tvar_ids = move(saved_tvar_ids);
            tvar_cnt = move(saved_tvar_cnt);
            for (auto &&el: tvar_ids) symtab.update(el, move(saved_tvar_ents.front())), saved_tvar_ents.pop_front();

            {  sym::tab<> tab; for (auto &&el: form[3]) tab.update(move(keys.front()), compile_rval(el[2], _loc)), keys.pop_front(); // TODO: ineffective move
               arms.reserve(descr.size()); for (auto &&el: descr) arms.push_back(tab[el]);
            }

            struct expr: code::lvalue {
               code key; record_descr descr; std::vector<code> arms; code else_arm;
            public:
               MNL_INLINE val execute(bool fast_sig) const {
                  auto key = this->key.execute();
                  return (MNL_LIKELY(test<sym>(key)) && descr.has(cast<const sym &>(key)) ?
                     arms[descr[cast<const sym &>(key)]] : else_arm).execute(fast_sig);
               }
               MNL_INLINE void exec_in(val &&value) const {
                  auto key = this->key.execute();
                  (MNL_LIKELY(test<sym>(key)) && descr.has(cast<const sym &>(key)) ?
                     arms[descr[cast<const sym &>(key)]] : else_arm).exec_in(move(value));
               }
               MNL_INLINE val exec_out() const {
                  auto key = this->key.execute();
                  return (MNL_LIKELY(test<sym>(key)) && descr.has(cast<const sym &>(key)) ?
                     arms[descr[cast<const sym &>(key)]] : else_arm).exec_out();
               }
            private:
               MNL_INLINE bool is_lvalue() const noexcept
                  { for (auto &&el: arms) if (!el.is_lvalue()) return false; return else_arm.is_lvalue(); }
            };
            return expr{move(key), move(descr), move(arms), compile_rval(form + 5, _loc)};
         }
      opt2: // {case V of K = B; K = B; ...}
         {  if (form.size() >= 4); else goto opt3;
            if (form[2] == MNL_SYM("of")); else goto opt3;
            for (auto &&el: form + 3) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt3;
         }
         {  if (form.size() - 3 > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  auto key = compile_rval(form[1], _loc);

            set<sym> descr; vector<code> arms;

            deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            deque<sym> keys; for (auto &&el: form + 3)
               if (keys.push_back(eval_sym(el[1], _loc)), !descr.insert(keys.back()).second) err_compile("ambiguous bindings", _loc);
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            {  sym::tab<> tab; for (auto &&el: form + 3) tab.update(move(keys.front()), compile_rval(el[2], _loc)), keys.pop_front(); // TODO: ineffective move
               arms.reserve(descr.size()); for (auto &&el: descr) arms.push_back(tab[el]);
            }

            struct expr { MNL_LVALUE(_is_lvalue())
               code key; record_descr descr; vector<code> arms; loc _loc;
            public:
               MNL_INLINE val execute(bool fast_sig) const {
                  auto key = this->key.execute();
                  if (MNL_UNLIKELY(!test<sym>(key))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
                  if (MNL_UNLIKELY(!descr.has(cast<const sym &>(key)))) MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
                  return arms[descr[cast<const sym &>(key)]].execute(fast_sig);
               }
               MNL_INLINE void exec_in(val &&value) const {
                  auto key = this->key.execute();
                  if (MNL_UNLIKELY(!test<sym>(key))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
                  if (MNL_UNLIKELY(!descr.has(cast<const sym &>(key)))) MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
                  arms[descr[cast<const sym &>(key)]].exec_in(move(value));
               }
               MNL_INLINE val exec_out() const {
                  auto key = this->key.execute();
                  if (MNL_UNLIKELY(!test<sym>(key))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
                  if (MNL_UNLIKELY(!descr.has(cast<const sym &>(key)))) MNL_ERR_LOC(_loc, MNL_SYM("KeyLookupFailed"));
                  return arms[descr[cast<const sym &>(key)]].exec_out();
               }
            private:
               MNL_INLINE bool _is_lvalue() const noexcept
                  { for (auto &&el: arms) if (!el.is_lvalue()) return false; return true; }
            };
            return expr{move(key), move(descr), move(arms), _loc};
         }
      opt3:
         err_compile("invalid form", _loc);
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   template<typename Arg_count> struct _expr_proc {
      [[no_unique_address]] Arg_count arg_count; code body;
   };

   template<typename Arg_count> template<typename Self, typename Arg0>
   MNL_INLINE val box<_expr_proc<Arg_count>>::apply(Self &&self, Arg0 &&arg0) {
      stk_check();
      if (MNL_UNLIKELY(1 != dat.arg_count)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      int index = 0;
      return tstack.frame_guard(), tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
         { tstack.push(std::forward<Arg0>(arg0)), ++index; }(),
         dat.body.execute();
   }
   template<typename Arg_count> template<typename Self, typename Arg0, typename Arg1>
   MNL_INLINE val box<_expr_proc<Arg_count>>::apply(Self &&self, Arg0 &&arg0, Arg1 &&arg1) {
      stk_check();
      if (MNL_UNLIKELY(2 != dat.arg_count)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      int index = 0;
      return tstack.frame_guard(), tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
         { tstack.push(std::forward<Arg0>(arg0)), ++index; tstack.push(std::forward<Arg1>(arg1)), ++index; }(),
         dat.body.execute();
   }
   template<typename Arg_count> template<typename Self>
   MNL_INLINE val box<_expr_proc<Arg_count>>::apply(Self &&self, int argc, val argv[]) {
      stk_check();
      if (MNL_UNLIKELY(argc != dat.arg_count)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      int index = 0;
      return tstack.frame_guard(), tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
         { MNL_UNROLL(10) for (; index < argc; ++index) tstack.push(std::move(argv[index])); }(),
         dat.body.execute();
   }
   template<typename Arg_count> template<typename Self>
   MNL_INLINE val box<_expr_proc<Arg_count>>::invoke(Self &&self, const sym &op, int argc, val argv[], val *) {
      stk_check();
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != dat.arg_count)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      int index = 0;
      return tstack.frame_guard(), tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
         { MNL_UNROLL(10) for (; index < argc; ++index) tstack.push(std::move(argv[index])); }(),
         dat.body.execute();
   }

   class comp_proc { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {proc {I; ...} as B; B; ...}
         {  if (form.size() >= 4); else goto opt2;
            if (form[1].is_list()); else goto opt2;
            if (form[2] == MNL_SYM("as")); else goto opt2;
            for (auto &&el: form[1]) if (test<sym>(el)); else goto opt2;
         }
         {  if (form[1].size() > val::max_argc) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[cast<const sym &>(el)])
               tab.update(cast<const sym &>(el), true); else err_compile("ambiguous bindings", _loc);
         }
         {  deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            deque<code> overriden_ents;
            for (auto &&el: form[1]) overriden_ents.push_back(symtab[cast<const sym &>(el)]),
               symtab.update(cast<const sym &>(el), expr_tmp{tmp_cnt++});
            for (auto &&el: form[1]) tmp_ids.insert(cast<const sym &>(el));

            auto body = compile_rval(form + 3, _loc);

            for (auto &&el: form[1]) symtab.update(cast<const sym &>(el), move(overriden_ents.front())), overriden_ents.pop_front();
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            switch (form[1].size()) {
            # define MNL_M1(ARG_COUNT) \
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) { \
                  stk_check(); \
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv); \
                  if (MNL_UNLIKELY(argc != ARG_COUNT)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
                  struct _ { \
                     decltype(tmp_frm) saved_tmp_frm; int sn; \
                     MNL_INLINE ~_() { for (; sn; --sn) tmp_stk.pop_back(); tmp_frm = move(saved_tmp_frm); } \
                  } _; \
                  _.saved_tmp_frm = move(tmp_frm), tmp_frm = tmp_stk.size(); \
                  for (_.sn = 0; _.sn < ARG_COUNT; ++_.sn) tmp_stk.push_back(move(argv[_.sn])); \
                  return body.execute(); \
               } \
            // end # define MNL_M1(ARG_COUNT)
               {  struct proc { const int arg_count; code body; MNL_M1(arg_count) };
               default: return optimize(expr_lit<>{proc{(int)form[1].size(), move(body)}});
               }
            # define MNL_M2(ARG_COUNT) \
               {  struct proc { code body; MNL_M1(ARG_COUNT) }; \
               case ARG_COUNT: return optimize(expr_lit<>{proc{move(body)}}); \
               } \
            // end # define MNL_M2(ARG_COUNT)
               MNL_M2(0) MNL_M2(1) MNL_M2(2) MNL_M2(3) MNL_M2(4) MNL_M2(5) MNL_M2(6)
            # undef MNL_M2
            # undef MNL_M1
            }
         }
      opt2: // {proc {I,I?; ...} as B; B; ...}
         {  if (form.size() >= 4); else goto opt3;
            if (form[1].is_list()); else goto opt3;
            if (form[2] == MNL_SYM("as")); else goto opt3;
            for (auto &&el: form[1]) if (test<sym>(el) ||
               el.is_list() && el.size() == 2 && el[0] == MNL_SYM("?") && test<sym>(el[1])); else goto opt3;
         }
         {  if (form[1].size() > val::max_argc) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[cast<const sym &>(test<sym>(el) ? el : el[1])])
               tab.update(cast<const sym &>(test<sym>(el) ? el : el[1]), true); else err_compile("ambiguous bindings", _loc);
         }
         {  deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            deque<code> overriden_ents;
            for (auto &&el: form[1]) overriden_ents.push_back(symtab[cast<const sym &>(test<sym>(el) ? el : el[1])]),
               symtab.update(cast<const sym &>(test<sym>(el) ? el : el[1]), expr_tmp{tmp_cnt++});
            for (auto &&el: form[1]) tmp_ids.insert(cast<const sym &>(test<sym>(el) ? el : el[1]));

            vector<unsigned char> mode; for (auto &&el: form[1]) mode.push_back(!test<sym>(el));
            auto body = compile_rval(form + 3, _loc);

            for (auto &&el: form[1]) symtab.update(cast<const sym &>(test<sym>(el) ? el : el[1]), move(overriden_ents.front())), overriden_ents.pop_front();
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            struct proc {
               vector<unsigned char> mode; code body;
            public:
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) {
                  stk_check();
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
                  if (MNL_UNLIKELY(argc != (int)mode.size())) MNL_ERR(MNL_SYM("InvalidInvocation"));
                  auto saved_tmp_frm = move(tmp_frm); tmp_frm = tmp_stk.size();
                  int sn; for (sn = 0; sn < argc; ++sn) tmp_stk.push_back(move(argv[sn]));
                  try {
                     auto res = body.execute();
                     if (!argv_out) for (; sn; --sn) tmp_stk.pop_back();
                        else for (; sn; tmp_stk.pop_back()) if (MNL_UNLIKELY(mode[--sn])) argv_out[sn].swap(tmp_stk.back());
                     tmp_frm = move(saved_tmp_frm);
                     return res;
                  } catch (...) {
                     while (sn--) tmp_stk.pop_back(); tmp_frm = move(saved_tmp_frm);
                     throw;
                  }
               }
            };
            return optimize(expr_lit<>{proc{move(mode), move(body)}});
         }
      opt3:
         err_compile("invalid form", _loc);
      }
   };

   class comp_var: code::nonvalue {
      friend code::box<comp_var>;
      MNL_INLINE static code compile(code &&, const ast &form, const loc &loc) {
      opt1: // {var {I; ...} in B}
         {  if (form.size() >= 4); else goto opt2;
            if (is<ast::list>(form[1])); else goto opt2;
            if (form[2] == MNL_SYM("in")); else goto opt2;
            for (auto &&el: form[1]) if (is<sym>(el)); else goto opt2;
         }
         {  if (tvar_cnt + (long)form[1].size() > std::numeric_limits<decltype(tvar_cnt)>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[as<const sym &>(el)])
               tab.update(as<const sym &>(el), true); else err_compile("ambiguous bindings", loc);
         }
         {  std::deque<code> overriden_ents;
            for (auto &&el: form[1]) overriden_ents.push_back(symtab[as<const sym &>(el)]),
               symtab.update(as<const sym &>(el), expr_tvar{tvar_cnt++});
            std::vector<sym> inserted_tvar_ids;
            for (auto &&el: form[1]) if (tvar_ids.insert(as<const sym &>(el)).second) inserted_tvar_ids.push_back(as<const sym &>(el));

            auto body = compile_rval(form[3], loc);

            for (auto &&el: inserted_tvar_ids) tvar_ids.erase(std::move(el)); // TODO: ineffective move
            tvar_cnt -= form[1].size();
            for (auto &&el: form[1]) symtab.update(as<const sym &>(el), std::move(overriden_ents.front())), overriden_ents.pop_front();

            const auto compile = [&] MNL_INLINE(auto _var_count){
               struct expr: code::lvalue {
                  [[no_unique_address]] decltype(_var_count) var_count; code body;
               public:
                  template<bool fast_sig, bool nores> MNL_INLINE val execute() const {
                     int index = 0;
                     int count = var_count;
                     return tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
                        { MNL_UNROLL(10) for (; index < count; ++index) tstack.push(); }(),
                        body.execute<fast_sig, nores>();
                  }
                  template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
                     int index = 0;
                     int count = var_count;
                     return tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
                        { MNL_UNROLL(10) for (; index < count; ++index) tstack.push(); }(),
                        body.exec_in(std::forward<Val>(value));
                  }
                  MNL_INLINE val exec_out() const {
                     int index = 0;
                     int count = var_count;
                     return tstack.scope_guard(index), [&] MNL_INLINE(decltype(tstack) &MNL_RESTRICT tstack = tstack)
                        { MNL_UNROLL(10) for (; index < count; ++index) tstack.push(); }(),
                        body.exec_out();
                  }
               public:
                  MNL_INLINE bool is_lvalue() const noexcept { return body.is_lvalue(); }
               };
               return expr{_var_count, std::move(body)};
            };
            switch (form[1].size()) {
            default: return compile((int)form[1].size());
            case 0:  return body; // automatic move
            case 1:  return compile(std::integral_constant<int, 1>{});
            case 2:  return compile(std::integral_constant<int, 2>{});
            case 3:  return compile(std::integral_constant<int, 3>{});
            case 4:  return compile(std::integral_constant<int, 4>{});
            case 5:  return compile(std::integral_constant<int, 5>{});
            case 6:  return compile(std::integral_constant<int, 6>{});
            case 7:  return compile(std::integral_constant<int, 7>{});
            case 8:  return compile(std::integral_constant<int, 8>{});
            }
         }
      opt2: // {var {I = V; ...} in B}
         {  if (form.size() == 4); else goto opt3;
            if (is<ast::list>(form[1])); else goto opt3;
            if (form[2] == MNL_SYM("in")); else goto opt3;
            for (auto &&el: form[1]) if (is<ast::list>(el) && el.size() == 3 && el[0] == MNL_SYM("=") && is<sym>(el[1])); else goto opt3;
         }
         {  if (tvar_cnt + (long)form[1].size() > std::numeric_limits<decltype(tvar_cnt)>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[as<const sym &>(el[1])])
               tab.update(as<const sym &>(el[1]), true); else err_compile("ambiguous bindings", loc);
         }
         {  std::vector<code> init; init.reserve(form[1].size()); for (auto &&el: form[1]) init.push_back(compile_rval(el[2], loc));

            std::deque<code> overriden_ents;
            for (auto &&el: form[1]) overriden_ents.push_back(symtab[as<const sym &>(el)]),
               symtab.update(as<const sym &>(el), expr_tvar{tvar_cnt++});
            std::vector<sym> inserted_tvar_ids;
            for (auto &&el: form[1]) if (tvar_ids.insert(as<const sym &>(el)).second) inserted_tvar_ids.push_back(as<const sym &>(el));

            auto body = compile_rval(form[3], loc);

            for (auto &&el: inserted_tvar_ids) tvar_ids.erase(std::move(el)); // TODO: ineffective move
            tvar_cnt -= form[1].size();
            for (auto &&el: form[1]) symtab.update(as<const sym &>(el), std::move(overriden_ents.front())), overriden_ents.pop_front();

            const auto compile = [&] MNL_INLINE(auto &&_init){
               struct expr: code::lvalue {
                  std::remove_reference_t<decltype(_init)> init; code body;
               public:
                  template<bool fast_sig, bool nores> MNL_INLINE val execute() const {
                     int index = 0;
                     int count = init.size(); const code *init = this->init.data();
                     return tstack.scope_guard(index), [&] MNL_INLINE()
                        { MNL_UNROLL(10) for (; index < count; ++index) tstack.push(init[index].execute()); }(),
                        body.execute<fast_sig, nores>();
                  }
                  template<typename Val> MNL_INLINE void exec_in(Val &&value) const {
                     int index = 0;
                     int count = init.size(); const code *init = this->init.data();
                     return tstack.scope_guard(index), [&] MNL_INLINE()
                        { MNL_UNROLL(10) for (; index < count; ++index) tstack.push(init[index].execute()); }(),
                        body.exec_in(std::forward<Val>(value));
                  }
                  MNL_INLINE val exec_out() const {
                     int index = 0;
                     int count = init.size(); const code *init = this->init.data();
                     return tstack.scope_guard(index), [&] MNL_INLINE()
                        { MNL_UNROLL(10) for (; index < count; ++index) tstack.push(init[index].execute()); }(),
                        body.exec_out();
                  }
               public:
                  MNL_INLINE bool is_lvalue() const noexcept { return body.is_lvalue(); }
               };
               return expr{std::move(_init), std::move(body)};
            };
            switch (form[1].size()) {
            default: return compile(
               std::move(init) );
            case 1:  return compile(
               std::array{std::move(init[0])} );
            case 2:  return compile(
               std::array{std::move(init[0]), std::move(init[1])} );
            case 3:  return compile(
               std::array{std::move(init[0]), std::move(init[1]), std::move(init[2])} );
            case 4:  return compile(
               std::array{std::move(init[0]), std::move(init[1]), std::move(init[2]), std::move(init[3])} );
            case 5:  return compile(
               std::array{std::move(init[0]), std::move(init[1]), std::move(init[2]), std::move(init[3]), std::move(init[4])} );
            case 6:  return compile(
               std::array{std::move(init[0]), std::move(init[1]), std::move(init[2]), std::move(init[3]), std::move(init[4]), std::move(init[5])} );
            }
         }
      opt3:
         err_compile("invalid form", loc);
      }
   };

   struct comp_let: code::nonvalue {
      MNL_INLINE static code compile(code &&, const ast &form, const loc &loc) {
      opt1: // {let {I = E; ...} in E}
         {  if (form.size() == 4); else goto opt2;
            if (is<ast::list>(form[1])); else goto opt2;
            if (form[2] == MNL_SYM("in")); else goto opt2;
            for (auto &&el: form[1]) if (is<ast::list>(el) && el.size() == 3 && el[0] == MNL_SYM("=") && is<sym>(el[1])); else goto opt2;
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[as<const sym &>(el[1])])
               tab.update(as<const sym &>(el[1]), true); else err_compile("ambiguous bindings", loc);
         }
         return [&]() MNL_INLINE{
            std::deque<code> saved_tvar_ents;
            for (auto &&el: tvar_ids) saved_tvar_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tvar_cnt = std::move(tvar_cnt); tvar_cnt = 0;
            auto saved_tvar_ids = std::move(tvar_ids); tvar_ids.clear();

            std::deque<code> overriding_ents;
            for (auto &&el: form[1]) { auto ent = pub::compile(el[2], loc);
               overriding_ents.push_back(ent.is_rvalue() ? optimize(expr_lit<>{ent.execute()}) : std::move(ent)); }

            tvar_ids = std::move(saved_tvar_ids);
            tvar_cnt = std::move(saved_tvar_cnt);
            for (auto &&el: tvar_ids) symtab.update(el, std::move(saved_tvar_ents.front())), saved_tvar_ents.pop_front();

            std::deque<code> overriden_ents;
            for (auto &&el: form[1]) overriden_ents.push_back(symtab[as<const sym &>(el[1])]),
               symtab.update(as<const sym &>(el[1]), std::move(overriding_ents.front())), overriding_ents.pop_front();
            std::vector<sym> erased_tvar_ids;
            for (auto &&el: form[1]) if (tvar_ids.erase(as<const sym &>(el[1]))) erased_tvar_ids.push_back(as<const sym &>(el[1]));

            auto body = pub::compile(form[3], loc);

            for (auto &&el: erased_tvar_ids) tvar_ids.insert(std::move(el)); // TODO: ineffective move
            for (auto &&el: form[1]) symtab.update(as<const sym &>(el[1]), std::move(overriden_ents.front())), overriden_ents.pop_front();

            return body;
         }();
      opt2: // {let rec {I = V; ...} in E}
         {  if (form.size() == 5); else goto opt3;
            if (form[1] == MNL_SYM("rec")); else goto opt3;
            if (is<ast::list>(form[2])); else goto opt3;
            if (form[3] == MNL_SYM("in")); else goto opt3;
            for (auto &&el: form[2]) if (is<ast::list>(el) && el.size() == 3 && el[0] == MNL_SYM("=") && is<sym>(el[1])); else goto opt3;
         }
         {  sym::tab<bool> tab; for (auto &&el: form[2]) if (!tab[as<const sym &>(el[1])])
               tab.update(as<const sym &>(el[1]), true); else err_compile("ambiguous bindings", loc);
         }
         return [&]() MNL_INLINE{
            std::deque<code> saved_tvar_ents;
            for (auto &&el: tvar_ids) saved_tvar_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tvar_cnt = std::move(tvar_cnt); tvar_cnt = 0;
            auto saved_tvar_ids = std::move(tvar_ids); tvar_ids.clear();
            struct expr_inner_lit: code::rvalue {
               mutable /*atomic*/ bool defined{}; mutable val value;
            public:
               MNL_INLINE void set(const val &value) const noexcept {
                  this->value = value, MNL_IF_WITHOUT_MT(defined = true) MNL_IF_WITH_MT(__atomic_store_n(&defined, true, __ATOMIC_RELEASE));
               }
               template<bool = bool{}, bool = bool{}> MNL_INLINE val execute() const {
                  if (MNL_LIKELY(MNL_IF_WITHOUT_MT(defined) MNL_IF_WITH_MT(__atomic_load_n(&defined, __ATOMIC_ACQUIRE)))) return value;
                  MNL_ERR(MNL_SYM("LetRecUndefined"));
               }
            };
            std::deque<code> overriden_ents;
            for (auto &&el: form[2]) overriden_ents.push_back(symtab[as<const sym &>(el[1])]),
               symtab.update(as<const sym &>(el[1]), expr_inner_lit{});

            std::deque<val> overriding_values;
            for (auto &&el: form[2]) overriding_values.push_back(compile_rval(el[2], loc).execute());
            for (auto &&el: form[2]) as<const expr_inner_lit &>(symtab[as<const sym &>(el[1])]).set(overriding_values.front()),
               overriding_values.push_back(val(overriding_values.front())), overriding_values.pop_front(); // resource leak possible, by design

            for (auto &&el: form[2]) symtab.update(as<const sym &>(el[1]), std::move(overriden_ents.front())), overriden_ents.pop_front();
            tvar_ids = std::move(saved_tvar_ids);
            tvar_cnt = std::move(saved_tvar_cnt);
            for (auto &&el: tvar_ids) symtab.update(el, std::move(saved_tvar_ents.front())), saved_tvar_ents.pop_front();

            for (auto &&el: form[2]) overriden_ents.push_back(symtab[as<const sym &>(el[1])]),
               symtab.update(as<const sym &>(el[1]), optimize(expr_lit<>{std::move(overriding_values.front())})), overriding_values.pop_front();
            std::vector<sym> erased_tvar_ids;
            for (auto &&el: form[2]) if (tvar_ids.erase(as<const sym &>(el[1]))) erased_tvar_ids.push_back(as<const sym &>(el[1]));

            auto body = pub::compile(form[4], loc);

            for (auto &&el: erased_tvar_ids) tvar_ids.insert(std::move(el)); // TODO: ineffective move
            for (auto &&el: form[2]) symtab.update(as<const sym &>(el[1]), std::move(overriden_ents.front())), overriden_ents.pop_front();

            return body;
         }();
      opt3:
         err_compile("invalid form", loc);
      }
   };

   class comp_export { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {export I; I; ...}
         {  if (form.size() >= 1); else goto opt2;
            for (auto &&el: form + 1) if (test<sym>(el)); else goto opt2;
         }
         {  sym::tab<bool> tab; for (auto &&el: form + 1) if (!tab[cast<const sym &>(el)])
               tab.update(cast<const sym &>(el), true); else err_compile("ambiguous bindings", _loc);
         }
         {  vector<pair<sym, code>> bind; bind.reserve(form.size() - 1);
            for (auto &&el: form + 1) bind.push_back(make_pair(cast<const sym &>(el), symtab[cast<const sym &>(el)]));
            return expr_export{move(bind)};
         }
      opt2:
         err_compile("invalid form", _loc);
      }
   };

   class comp_scope { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {scope {I; ...} in E}, {scope {I; ...} in B; B; B; ...}
         {  if (form.size() >= 4); else goto opt2;
            if (form[1].is_list()); else goto opt2;
            if (form[2] == MNL_SYM("in")); else goto opt2;
            for (auto &&el: form[1]) if (test<sym>(el)); else goto opt2;
         }
         {  sym::tab<bool> tab; for (auto &&el: form[1]) if (!tab[cast<const sym &>(el)])
               tab.update(cast<const sym &>(el), true); else err_compile("ambiguous bindings", _loc);
         }
         return [&]()->code{
            auto saved_symtab = move(symtab); symtab.clear();
            for (auto &&el: form[1]) symtab.update(cast<const sym &>(el), saved_symtab[cast<const sym &>(el)]);
            auto body = form.size() == 4 ? pub::compile(form[3], _loc) : compile_rval(form + 3, _loc);
            symtab = move(saved_symtab);
            return body;
         }();
      opt2:
         err_compile("invalid form", _loc);
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   class comp_array { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {array}
         {  if (form.size() == 1); else goto opt2;
         }
         {  static const auto res = optimize(expr_lit<>{vector<val>{}}); return res;
         }
      opt2: // {array of V; V; ...}
         {  if (form.size() >= 3); else goto opt3;
            if (form[1] == MNL_SYM("of")); else goto opt3;
         }
         {  vector<code> elems; elems.reserve(form.size() - 2);
            for (auto &&el: form + 2) elems.push_back(compile_rval(el, _loc));
            switch (elems.size()) {
               {  struct expr { MNL_RVALUE()
                     vector<code> elems;
                     MNL_INLINE val execute(bool) const {
                        val res = vector<val>{}; cast<vector<val> &>(res).reserve(elems.size());
                        for (auto &&el: elems) cast<vector<val> &>(res).push_back(el.execute()); return res;
                     }
                  };
               default: return expr{move(elems)};
               }
               {  struct expr { MNL_RVALUE()
                     code elems[1];
                     MNL_INLINE val execute(bool) const {
                        val res = vector<val>{}; cast<vector<val> &>(res).reserve(std::extent<decltype(elems)>::value);
                        cast<vector<val> &>(res).push_back(elems[0].execute()); return res;
                     }
                  };
               case  1: return expr{move(elems[0])};
               }
               {  struct expr { MNL_RVALUE()
                     code elems[2];
                     MNL_INLINE val execute(bool) const {
                        val res = vector<val>{}; cast<vector<val> &>(res).reserve(std::extent<decltype(elems)>::value);
                        cast<vector<val> &>(res).push_back(elems[0].execute()); cast<vector<val> &>(res).push_back(elems[1].execute()); return res;
                     }
                  };
               case  2: return expr{move(elems[0]), move(elems[1])};
               }
               {  struct expr { MNL_RVALUE()
                     code elems[3];
                     MNL_INLINE val execute(bool) const {
                        val res = vector<val>{}; cast<vector<val> &>(res).reserve(std::extent<decltype(elems)>::value);
                        cast<vector<val> &>(res).push_back(elems[0].execute()); cast<vector<val> &>(res).push_back(elems[1].execute());
                        cast<vector<val> &>(res).push_back(elems[2].execute()); return res;
                     }
                  };
               case  3: return expr{move(elems[0]), move(elems[1]), move(elems[2])};
               }
               {  struct expr { MNL_RVALUE()
                     code elems[4];
                     MNL_INLINE val execute(bool) const {
                        val res = vector<val>{}; cast<vector<val> &>(res).reserve(std::extent<decltype(elems)>::value);
                        cast<vector<val> &>(res).push_back(elems[0].execute()); cast<vector<val> &>(res).push_back(elems[1].execute());
                        cast<vector<val> &>(res).push_back(elems[2].execute()); cast<vector<val> &>(res).push_back(elems[3].execute()); return res;
                     }
                  };
               case  4: return expr{move(elems[0]), move(elems[1]), move(elems[2]), move(elems[3])};
               }
            }
         }
      opt3: // {array N of V}
         {  if (form.size() == 4); else goto opt4;
            if (form[2] == MNL_SYM("of")); else goto opt4;
         }
         {  struct expr { MNL_RVALUE()
               code size, elem; loc _loc;
               MNL_INLINE val execute(bool) const {
                  auto size = this->size.execute();
                  if (MNL_UNLIKELY(!test<long long>(size))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
                  if (MNL_UNLIKELY(cast<long long>(size) < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                  auto elem = this->elem.execute();
                  return [&]()->vector<val>{ try { return vector<val>(MNL_LIKELY(cast<long long>(size) <= lim<vector<val>::size_type>::max()) ?
                     cast<long long>(size) : throw std::bad_alloc{}, elem); } catch (...) { trace_execute(_loc); } }(); // do trace alloc errors
               }
            };
            return expr{compile_rval(form[1], _loc), compile_rval(form[3], _loc), _loc};
         }
      opt4: // {array N}
         {  if (form.size() == 2); else goto opt5;
         }
         {  struct expr { MNL_RVALUE()
               code size; loc _loc;
               MNL_INLINE val execute(bool) const {
                  auto size = this->size.execute();
                  if (MNL_UNLIKELY(!test<long long>(size))) MNL_ERR_LOC(_loc, MNL_SYM("TypeMismatch"));
                  if (MNL_UNLIKELY(cast<long long>(size) < 0)) MNL_ERR_LOC(_loc, MNL_SYM("ConstraintViolation"));
                  return [&]()->vector<val>{ try { return vector<val>(MNL_LIKELY(cast<long long>(size) <= lim<vector<val>::size_type>::max()) ?
                     cast<long long>(size) : throw std::bad_alloc{}); } catch (...) { trace_execute(_loc); } }(); // do trace alloc errors
               }
            };
            return expr{compile_rval(form[1], _loc), _loc};
         }
      opt5:
         err_compile("invalid form", _loc);
      }
   };

   class comp_record { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {record utils K; K; ...}
         {  if (form.size() >= 3); else goto opt2;
            if (form[1] == MNL_SYM("utils")); else goto opt2;
         }
         {  if (form.size() - 2 > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  set<sym> descr;

            deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            for (auto &&el: form + 2)
               if (!descr.insert(eval_sym(el, _loc)).second) err_compile("ambiguous bindings", _loc);
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            switch (descr.size()) {
            # define MNL_M(CASE, SIZE) \
               {  struct proc { \
                     record_descr descr; \
                     MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) { \
                        if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv); \
                        if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation")); \
                        return test<_record<SIZE>>(argv[0]) && cast<const _record<SIZE> &>(argv[0]).descr == descr; \
                     } \
                  }; \
               CASE SIZE: return expr_export{{"IsInst", optimize(expr_lit<>{proc{move(descr)}})}}; \
               } \
            // end # define MNL_M(CASE, SIZE)
               MNL_M(default,)
               MNL_M(case, 0x1) MNL_M(case, 0x2) MNL_M(case, 0x3) MNL_M(case, 0x4)
               MNL_M(case, 0x5) MNL_M(case, 0x6) MNL_M(case, 0x7) MNL_M(case, 0x8)
               MNL_M(case, 0x9) MNL_M(case, 0xA) MNL_M(case, 0xB) MNL_M(case, 0xC)
            # undef MNL_M
            }
         }
      opt2: // {record of K = V; K = V; ...}
         {  if (form.size() >= 3); else goto opt3;
            if (form[1] == MNL_SYM("of")); else goto opt3;
            for (auto &&el: form + 2) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt3;
         }
         {  if (form.size() - 2 > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  set<sym> descr; vector<code> items;

            deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            deque<sym> keys; for (auto &&el: form + 2)
               if (keys.push_back(eval_sym(el[1], _loc)), !descr.insert(keys.back()).second) err_compile("ambiguous bindings", _loc);
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            {  sym::tab<> tab; for (auto &&el: form + 2) tab.update(move(keys.front()), compile_rval(el[2], _loc)), keys.pop_front(); // TODO: ineffective move
               items.reserve(descr.size()); for (auto &&el: descr) items.push_back(tab[el]);
            }

            switch (descr.size()) {
               {  struct expr { MNL_RVALUE() // more than 12 items
                     record_descr descr; vector<code> items;
                     MNL_INLINE val execute(bool) const {
                        val res = _record<>{descr}; cast<_record<> &>(res).items.reserve(items.size());
                        for (auto &&el: items) cast<_record<> &>(res).items.push_back(el.execute()); return res;
                     }
                  };
               default:  return expr{move(descr), move(items)};
               }
               {  struct expr { MNL_RVALUE() // 1 item
                     record_descr descr; code items[0x1];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x1>{descr,
                           items[0x0].execute()};
                     }
                  };
               case 0x1: return expr{move(descr),
                            move(items[0x0])};
               }
               {  struct expr { MNL_RVALUE() // 2 items
                     record_descr descr; code items[0x2];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x2>{descr,
                           items[0x0].execute(), items[0x1].execute()};
                     }
                  };
               case 0x2: return expr{move(descr),
                            move(items[0x0]), move(items[0x1])};
               }
               {  struct expr { MNL_RVALUE() // 3 items
                     record_descr descr; code items[0x3];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x3>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute()};
                     }
                  };
               case 0x3: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2])};
               }
               {  struct expr { MNL_RVALUE() // 4 items
                     record_descr descr; code items[0x4];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x4>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute()};
                     }
                  };
               case 0x4: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3])};
               }
               {  struct expr { MNL_RVALUE() // 5 items
                     record_descr descr; code items[0x5];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x5>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute()};
                     }
                  };
               case 0x5: return expr{move(descr), move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4])};
               }
               {  struct expr { MNL_RVALUE() // 6 items
                     record_descr descr; code items[0x6];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x6>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute()};
                     }
                  };
               case 0x6: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5])};
               }
               {  struct expr { MNL_RVALUE() // 7 items
                     record_descr descr; code items[0x7];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x7>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute(), items[0x6].execute()};
                     }
                  };
               case 0x7: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5]), move(items[0x6])};
               }
               {  struct expr { MNL_RVALUE() // 8 items
                     record_descr descr; code items[0x8];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x8>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute(), items[0x6].execute(), items[0x7].execute()};
                     }
                  };
               case 0x8: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5]), move(items[0x6]), move(items[0x7])};
               }
               {  struct expr { MNL_RVALUE() // 9 items
                     record_descr descr; code items[0x9];
                     MNL_INLINE val execute(bool) const {
                        return _record<0x9>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute(), items[0x6].execute(), items[0x7].execute(),
                           items[0x8].execute()};
                     }
                  };
               case 0x9: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5]), move(items[0x6]), move(items[0x7]),
                            move(items[0x8])};
               }
               {  struct expr { MNL_RVALUE() // 10 items
                     record_descr descr; code items[0xA];
                     MNL_INLINE val execute(bool) const {
                        return _record<0xA>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute(), items[0x6].execute(), items[0x7].execute(),
                           items[0x8].execute(), items[0x9].execute()};
                     }
                  };
               case 0xA: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5]), move(items[0x6]), move(items[0x7]),
                            move(items[0x8]), move(items[0x9])};
               }
               {  struct expr { MNL_RVALUE() // 11 items
                     record_descr descr; code items[0xB];
                     MNL_INLINE val execute(bool) const {
                        return _record<0xB>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute(), items[0x6].execute(), items[0x7].execute(),
                           items[0x8].execute(), items[0x9].execute(), items[0xA].execute()};
                     }
                  };
               case 0xB: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5]), move(items[0x6]), move(items[0x7]),
                            move(items[0x8]), move(items[0x9]), move(items[0xA])};
               }
               {  struct expr { MNL_RVALUE() // 12 items
                     record_descr descr; code items[0xC];
                     MNL_INLINE val execute(bool) const {
                        return _record<0xC>{descr,
                           items[0x0].execute(), items[0x1].execute(), items[0x2].execute(), items[0x3].execute(),
                           items[0x4].execute(), items[0x5].execute(), items[0x6].execute(), items[0x7].execute(),
                           items[0x8].execute(), items[0x9].execute(), items[0xA].execute(), items[0xB].execute()};
                     }
                  };
               case 0xC: return expr{move(descr),
                            move(items[0x0]), move(items[0x1]), move(items[0x2]), move(items[0x3]),
                            move(items[0x4]), move(items[0x5]), move(items[0x6]), move(items[0x7]),
                            move(items[0x8]), move(items[0x9]), move(items[0xA]), move(items[0xB])};
               }
            }
         }
      opt3: // {record K; K; ...}
         {  if (form.size() >= 2); else goto opt4;
         }
         {  if (form.size() - 1 > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  set<sym> descr;

            deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            for (auto &&el: form + 1)
               if (!descr.insert(eval_sym(el, _loc)).second) err_compile("ambiguous bindings", _loc);
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            switch (descr.size()) {
            case 0x1: return expr_lit<>{_record<0x1>{move(descr)}};
            case 0x2: return expr_lit<>{_record<0x2>{move(descr)}};
            case 0x3: return expr_lit<>{_record<0x3>{move(descr)}};
            case 0x4: return expr_lit<>{_record<0x4>{move(descr)}};
            case 0x5: return expr_lit<>{_record<0x5>{move(descr)}};
            case 0x6: return expr_lit<>{_record<0x6>{move(descr)}};
            case 0x7: return expr_lit<>{_record<0x7>{move(descr)}};
            case 0x8: return expr_lit<>{_record<0x8>{move(descr)}};
            case 0x9: return expr_lit<>{_record<0x9>{move(descr)}};
            case 0xA: return expr_lit<>{_record<0xA>{move(descr)}};
            case 0xB: return expr_lit<>{_record<0xB>{move(descr)}};
            case 0xC: return expr_lit<>{_record<0xC>{move(descr)}};
            }
            auto size = descr.size();
            return optimize(expr_lit<>{_record<>{move(descr), vector<val>(size)}});
         }
      opt4:
         err_compile("invalid form", _loc);
      }
   };

   class comp_object { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         struct expr { MNL_RVALUE()
            record_descr descr; vector<code> items;
            val cleanup; shared_ptr<const _record<>> methods;
         public:
            MNL_INLINE val execute(bool) const {
               val res = object{descr, cleanup, methods}; cast<object &>(res).items.reserve(items.size());
               for (auto &&el: items) cast<object &>(res).items.push_back(el.execute()); return res;
            }
         };
         static const auto no_methods =
            make_shared<const _record<>>(_record<>{set<sym>{}});
      opt1: // {object utils K; K; ...}
         {  if (form.size() >= 3); else goto opt2;
            if (form[1] == MNL_SYM("utils")); else goto opt2;
         }
         {  if (form.size() - 2 > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  set<sym> descr;

            deque<code> saved_tmp_ents;
            for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            for (auto &&el: form + 2)
               if (!descr.insert(eval_sym(el, _loc)).second) err_compile("ambiguous bindings", _loc);
            tmp_ids = move(saved_tmp_ids);
            tmp_cnt = move(saved_tmp_cnt);
            for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

            struct proc_isinst {
               record_descr descr;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
                  if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                  return test<object>(argv[0]) && cast<const object &>(argv[0]).descr == descr;
               };
            };
            struct proc_refcount {
               record_descr descr;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
                  if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                  if (MNL_UNLIKELY(!test<object>(argv[0])) || MNL_UNLIKELY(cast<const object &>(argv[0]).descr != descr)) MNL_ERR(MNL_SYM("NotAnInstance"));
                  return (long long)(argv[0].rc() - 1);
               };
            };
            struct proc_sameinst {
               record_descr descr;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
                  if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
                  if (MNL_UNLIKELY(!test<object>(argv[0])) || MNL_UNLIKELY(cast<const object &>(argv[0]).descr != descr)) MNL_ERR(MNL_SYM("NotAnInstance"));
                  return test<object>(argv[1]) && cast<const object &>(argv[1]).descr == descr &&
                     &cast<const object &>(argv[0]) == &cast<const object &>(argv[1]);
               };
            };
            record_descr _descr = move(descr);
            return expr_export{
               {"IsInst",   optimize(expr_lit<>{proc_isinst  {_descr}})},
               {"RefCount", optimize(expr_lit<>{proc_refcount{_descr}})},
               {"SameInst", optimize(expr_lit<>{proc_sameinst{_descr}})},
            };
         }
      opt2: // {object {K = V; ...} cleanup A with K = A; K = A; ...}
         {  if (form.size() >= 6); else goto opt3;
            if (form[1].is_list()); else goto opt3;
            if (form[2] == MNL_SYM("cleanup")); else goto opt3;
            if (form[4] == MNL_SYM("with")); else goto opt3;
            for (auto &&el: form[1]) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt3;
            for (auto &&el: form + 5) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt3;
         }
         {  if (form[1].size() > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
            if (form.size() - 5 > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  set<sym> descr; vector<code> items;
            {  deque<code> saved_tmp_ents;
               for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
               auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
               auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
               deque<sym> keys; for (auto &&el: form[1])
                  if (keys.push_back(eval_sym(el[1], _loc)), !descr.insert(keys.back()).second) err_compile("ambiguous bindings", _loc);
               tmp_ids = move(saved_tmp_ids);
               tmp_cnt = move(saved_tmp_cnt);
               for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

               sym::tab<> tab; for (auto &&el: form[1]) tab.update(move(keys.front()), compile_rval(el[2], _loc)), keys.pop_front();
               items.reserve(descr.size()); for (auto el: descr) items.push_back(tab[el]);
            }
            auto cleanup = [&]()->val{
               deque<code> saved_tmp_ents;
               for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
               auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
               auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
               auto res = compile_rval(form[3], _loc).execute();
               tmp_ids = move(saved_tmp_ids);
               tmp_cnt = move(saved_tmp_cnt);
               for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();
               return res;
            }();
            struct { set<sym> descr; vector<val> items; } methods;
            {  deque<code> saved_tmp_ents;
               for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
               auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
               auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();

               deque<sym> keys; for (auto &&el: form + 5)
                  if (keys.push_back(eval_sym(el[1], _loc)), !methods.descr.insert(keys.back()).second) err_compile("ambiguous bindings", _loc);
               sym::tab<> tab; for (auto &&el: form + 5) tab.update(move(keys.front()), compile_rval(el[2], _loc)), keys.pop_front();
               methods.items.reserve(methods.descr.size()); for (auto &&el: methods.descr) methods.items.push_back(tab[el].execute());

               tmp_ids = move(saved_tmp_ids);
               tmp_cnt = move(saved_tmp_cnt);
               for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();
            }
            return expr{move(descr), move(items), move(cleanup), make_shared<const _record<>>(_record<>{move(methods.descr), move(methods.items)})};
         }
      opt3: // {object {K = V; ...} cleanup A}
         {  if (form.size() == 4); else goto opt4;
            if (form[1].is_list()); else goto opt4;
            if (form[2] == MNL_SYM("cleanup")); else goto opt4;
            for (auto &&el: form[1]) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt4;
         }
         {  if (form[1].size() > lim<unsigned char>::max()) MNL_ERR(MNL_SYM("LimitExceeded"));
         }
         {  set<sym> descr; vector<code> items;
            {  deque<code> saved_tmp_ents;
               for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
               auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
               auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
               deque<sym> keys; for (auto &&el: form[1])
                  if (keys.push_back(eval_sym(el[1], _loc)), !descr.insert(keys.back()).second) err_compile("ambiguous bindings", _loc);
               tmp_ids = move(saved_tmp_ids);
               tmp_cnt = move(saved_tmp_cnt);
               for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

               sym::tab<> tab; for (auto &&el: form[1]) tab.update(move(keys.front()), compile_rval(el[2], _loc)), keys.pop_front();
               items.reserve(descr.size()); for (auto &&el: descr) items.push_back(tab[el]);
            }
            auto cleanup = [&]()->val{
               deque<code> saved_tmp_ents;
               for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
               auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
               auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
               auto res = compile_rval(form[3], _loc).execute();
               tmp_ids = move(saved_tmp_ids);
               tmp_cnt = move(saved_tmp_cnt);
               for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();
               return res;
            }();
            return expr{move(descr), move(items), move(cleanup), no_methods};
         }
      opt4:
         err_compile("invalid form", _loc);
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   class comp_att { MNL_NONVALUE()
      MNL_INLINE static inline code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2 || !form[1].is_list() || form[1].size() != 2) err_compile("invalid form", _loc);
         return optimize(expr_att{compile_rval(form[1][0], _loc), compile_rval(form[1][1], _loc), _loc});
      }
   };

}} // namespace aux::<unnamed>

// Map, Set, and Sequence Composites ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux { namespace {

   class comp_map { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {map}
         {  if (form.size() == 1); else goto opt2;
         }
         {  static const auto res = optimize(expr_lit<>{dict<val, val>{}}); return res;
         }
      opt2: // {map of K = V; K = V; ...}
         {  if (form.size() >= 3); else goto opt3;
            if (form[1] == MNL_SYM("of")); else goto opt3;
            for (auto &&el: form + 2) if (el.is_list() && el.size() == 3 && el[0] == MNL_SYM("=")); else goto opt3;
         }
         {  vector<pair<code, code>> elems;
            for (auto &&el: form + 2) elems.push_back({compile_rval(el[1], _loc), compile_rval(el[2], _loc)});
            struct expr { MNL_RVALUE()
               vector<pair<code, code>> elems;
               MNL_INLINE val execute(bool) const {
                  val res = dict<val, val>{};
                  for (auto &&el: elems) cast<dict<val, val> &>(res).set({el.first.execute(), el.second.execute()});
                  return res;
               }
            };
            return expr{move(elems)};
         }
      opt3: // {map K; K; ...}
         {  if (form.size() >= 2); else goto opt4;
         }
         {  vector<code> elems;
            for (auto &&el: form + 1) elems.push_back({compile_rval(el, _loc)});
            struct expr { MNL_RVALUE()
               vector<code> elems;
               MNL_INLINE val execute(bool) const {
                  val res = dict<val, val>{};
                  for (auto &&el: elems) cast<dict<val, val> &>(res).set({el.execute()});
                  return res;
               }
            };
            return expr{move(elems)};
         }
      opt4:
         err_compile("invalid form", _loc);
      }
   };

   class comp_set_c { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {set}
         {  if (form.size() == 1); else goto opt2;
         }
         {  static const auto res = optimize(expr_lit<>{dict<val>{}}); return res;
         }
      opt2: // {set of V; V; ...}
         {  if (form.size() >= 3); else goto opt3;
            if (form[1] == MNL_SYM("of")); else goto opt3;
         }
         {  vector<code> elems;
            for (auto &&el: form + 2) elems.push_back(compile_rval(el, _loc));
            struct expr { MNL_RVALUE()
               vector<code> elems;
               MNL_INLINE val execute(bool) const {
                  val res = dict<val>{};
                  for (auto &&el: elems) cast<dict<val> &>(res).set(el.execute());
                  return res;
               }
            };
            return expr{move(elems)};
         }
      opt3: // {set K; K; ...}
         {  if (form.size() >= 2); else goto opt4;
         }
         {  vector<code> elems;
            for (auto &&el: form + 1) elems.push_back(compile_rval(el, _loc));
            struct expr { MNL_RVALUE()
               vector<code> elems;
               MNL_INLINE val execute(bool) const {
                  val res = dict<val>{};
                  for (auto &&el: elems) cast<dict<val> &>(res).set(el.execute());
                  return res;
               }
            };
            return expr{move(elems)};
         }
      opt4:
         err_compile("invalid form", _loc);
      }
   };

   class comp_sequence { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
      opt1: // {sequence}
         {  if (form.size() == 1); else goto opt2;
         }
         {  static const auto res = optimize(expr_lit<>{list<val>{}}); return res;
         }
      opt2: // {sequence of V; V; ...}
         {  if (form.size() >= 3); else goto opt3;
            if (form[1] == MNL_SYM("of")); else goto opt3;
         }
         {  vector<code> elems;
            for (auto &&el: form + 2) elems.push_back(compile_rval(el, _loc));
            struct expr { MNL_RVALUE()
               vector<code> elems;
               MNL_INLINE val execute(bool) const {
                  val res = list<val>{};
                  for (auto &&el: elems) cast<list<val> &>(res).push_back(el.execute());
                  return res;
               }
            };
            return expr{move(elems)};
         }
      opt3:
         err_compile("invalid form", _loc);
      }
   };

}} // namespace aux::<unnamed>

// Metaprogramming Facilities //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux { namespace {

   class comp_macro { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2) err_compile("invalid form", _loc);

         deque<code> saved_tmp_ents;
         for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
         auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
         auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();

         auto res = compile_rval(form[1], _loc);

         tmp_ids = move(saved_tmp_ids);
         tmp_cnt = move(saved_tmp_cnt);
         for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

         struct expr { MNL_NONVALUE() val macro;
            MNL_INLINE code compile(code &&, const pub::form &form, const loc &_loc) const { return pub::compile(macro(form), _loc); }
         };
         return expr{res.execute()};
      }
   };

   class comp_code { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2) err_compile("invalid form", _loc);
         auto res = pub::compile(form[1], _loc);
         if (!res.is_rvalue()) return optimize(expr_lit<>{move(res)});
         struct expr { MNL_RVALUE()
            code value;
            MNL_INLINE val execute(bool) const { return optimize(expr_lit<>{value.execute()}); }
         };
         return expr{move(res)};
      }
   };

   class comp_error { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2) err_compile("invalid form", _loc);

         deque<code> saved_tmp_ents;
         for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
         auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
         auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();

         auto res = compile_rval(form[1], _loc).execute();

         tmp_ids = move(saved_tmp_ids);
         tmp_cnt = move(saved_tmp_cnt);
         for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

         if (!test<string>(res)) err_compile("type mismatch", _loc);
         err_compile(cast<const string &>(res).c_str(), _loc);
      }
   };

   class comp_eval { MNL_NONVALUE()
      MNL_INLINE static code compile(code &&, const form &form, const loc &_loc) {
         if (form.size() != 2) err_compile("invalid form", _loc);
         struct expr { MNL_LVALUE(true)
            code form; decltype(symtab) _symtab; decltype(tmp_cnt) _tmp_cnt; decltype(tmp_ids) _tmp_ids;
         public:
            MNL_INLINE val execute(bool fast_sig) const {
               return [&]()->code{
                  auto form = this->form.execute();
                  auto saved_tmp_frm = move(tmp_frm); tmp_frm = 0;
                  auto _finally_tmp_frm = finally([&]{ tmp_frm = move(saved_tmp_frm); });
                  auto saved_symtab = move(symtab); symtab = _symtab;
                  auto _finally_symtab = finally([&]{ symtab = move(saved_symtab); });
                  auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = _tmp_cnt;
                  auto _finally_tmp_cnt = finally([&]{ tmp_cnt = move(saved_tmp_cnt); });
                  auto saved_tmp_ids = move(tmp_ids); tmp_ids = _tmp_ids;
                  auto _finally_tmp_ids = finally([&]{ tmp_ids = move(saved_tmp_ids); });
                  return compile_rval(form);
               }().execute(fast_sig);
            }
            MNL_INLINE void exec_in(val &&value) const {
               [&]()->code{
                  auto form = this->form.execute();
                  auto saved_tmp_frm = move(tmp_frm); tmp_frm = 0;
                  auto _finally_tmp_frm = finally([&]{ tmp_frm = move(saved_tmp_frm); });
                  auto saved_symtab = move(symtab); symtab = _symtab;
                  auto _finally_symtab = finally([&]{ symtab = move(saved_symtab); });
                  auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = _tmp_cnt;
                  auto _finally_tmp_cnt = finally([&]{ tmp_cnt = move(saved_tmp_cnt); });
                  auto saved_tmp_ids = move(tmp_ids); tmp_ids = _tmp_ids;
                  auto _finally_tmp_ids = finally([&]{ tmp_ids = move(saved_tmp_ids); });
                  return compile_lval(form);
               }().exec_in(move(value));
            }
            MNL_INLINE val exec_out() const {
               return [&]()->code{
                  auto form = this->form.execute();
                  auto saved_tmp_frm = move(tmp_frm); tmp_frm = 0;
                  auto _finally_tmp_frm = finally([&]{ tmp_frm = move(saved_tmp_frm); });
                  auto saved_symtab = move(symtab); symtab = _symtab;
                  auto _finally_symtab = finally([&]{ symtab = move(saved_symtab); });
                  auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = _tmp_cnt;
                  auto _finally_tmp_cnt = finally([&]{ tmp_cnt = move(saved_tmp_cnt); });
                  auto saved_tmp_ids = move(tmp_ids); tmp_ids = _tmp_ids;
                  auto _finally_tmp_ids = finally([&]{ tmp_ids = move(saved_tmp_ids); });
                  return compile_lval(form);
               }().exec_out();
            }
         };
         return expr{compile_rval(form[1], _loc), symtab, tmp_cnt, tmp_ids};
      }
   private:
      template<typename Functor> struct _finally { const Functor _; MNL_INLINE ~_finally() noexcept(noexcept(_())) { _(); } };
      template<typename Functor> MNL_INLINE static inline _finally<Functor> finally(Functor &&_) { return { (move)(_) }; }
   };

}} // namespace aux::<unnamed>

// Functional Programming Utilities ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux { namespace {

   class proc_VarArg { friend box<proc_VarArg>; MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 2)) {
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      } else {
         if (MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(cast<long long>(argv[1]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
         if (MNL_UNLIKELY(cast<long long>(argv[1]) + 1 > val::max_argc)) MNL_ERR(MNL_SYM("LimitExceeded"));
      }
      struct proc { val target; int min_argc;
         MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
            stk_check();
            if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
            if (MNL_UNLIKELY(argc < min_argc)) MNL_ERR(MNL_SYM("InvalidInvocation"));
            val _argv[min_argc + 1];
            for (int sn = 0; sn < min_argc; ++sn) _argv[sn].swap(argv[sn]);
            if (MNL_LIKELY(argc == min_argc))
               _argv[min_argc] = []()->const val &{ static MNL_IF_WITH_MT(thread_local) const val _ = vector<val>{}; return _; }();
            else {
               cast<vector<val> &>(_argv[min_argc] = vector<val>{}).reserve(argc - min_argc);
               int sn = min_argc; do cast<vector<val> &>(_argv[min_argc]).push_back(move(argv[sn])); while (++sn < argc);
            }
            return MNL_LIKELY(!argv_out) ? target(min_argc + 1, _argv) : [&]()->val{
               val _argv_out[min_argc + 1], res = target(min_argc + 1, _argv, _argv_out);
               for (int sn = 0; sn < min_argc; ++sn) argv_out[sn].swap(_argv_out[sn]);
               if (MNL_LIKELY(!_argv_out[min_argc])) return res;
               if (MNL_UNLIKELY(!test<vector<val>>(_argv_out[min_argc]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               if (MNL_UNLIKELY(cast<const vector<val> &>(_argv_out[min_argc]).size() != argc - min_argc)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               if (MNL_LIKELY(_argv_out[min_argc].rc() == 1))
                  for (int sn = min_argc; sn < argc; ++sn) argv_out[sn].swap(cast<vector<val> &>(_argv_out[min_argc])[sn - min_argc]); else
                  for (int sn = min_argc; sn < argc; ++sn) argv_out[sn] = cast<const vector<val> &>(_argv_out[min_argc])[sn - min_argc];
               return res;
            }();
         }
      };
      return proc{move(argv[0]), MNL_UNLIKELY(argc != 2) ? 0 : (int)cast<long long>(argv[1])};
   }};

   class proc_VarApply { friend box<proc_VarApply>; MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc < 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<vector<val>>(argv[argc - 1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (MNL_UNLIKELY(cast<const vector<val> &>(argv[argc - 1]).size() + argc - 2 > val::max_argc)) MNL_ERR(MNL_SYM("LimitExceeded"));
      val _argv[cast<const vector<val> &>(argv[argc - 1]).size() + argc - 2];
      {  int sn = 0;
         for (; sn < argc - 2; ++sn) _argv[sn].swap(argv[sn + 1]);
         if (MNL_LIKELY(argv[argc - 1].rc() == 1))
            for (auto &&el: cast<vector<val> &>(argv[argc - 1])) _argv[sn++].swap(el); else
            for (auto &&el: cast<const vector<val> &>(argv[argc - 1])) _argv[sn++] = el;
      }
      return MNL_LIKELY(!argv_out) ? move(argv[0])(cast<const vector<val> &>(argv[argc - 1]).size() + argc - 2 , _argv) : [&]()->val{
         val _argv_out[cast<const vector<val> &>(argv[argc - 1]).size() + argc - 2],
            res = move(argv[0])(cast<const vector<val> &>(argv[argc - 1]).size() + argc - 2, _argv, _argv_out);
         int sn = 0;
         for (; sn < argc - 2; ++sn) argv_out[sn + 1].swap(_argv_out[sn]);
         argv_out[argc - 1] = MNL_LIKELY(argv[argc - 1].rc() == 1) ? move(argv[argc - 1]) :
            (val)vector<val>(cast<const vector<val> &>(argv[argc - 1]).size());
         for (auto &&el: cast<vector<val> &>(argv_out[argc - 1])) el.swap(_argv_out[sn++]);
         return res;
      }();
   }};

   class proc_Bind { friend box<proc_Bind>; MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc < 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      switch (argc) {
         {  static constexpr int argc_bound = 0;
         case argc_bound + 1: return move(argv[0]);
         }
         {  struct proc { val target; vector<val> args;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
                  stk_check();
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
                  if (MNL_UNLIKELY(argc + args.size() > val::max_argc)) MNL_ERR(MNL_SYM("LimitExceeded"));
                  val _argv[argc + args.size()]
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {args[0], args[1], args[2], args[3], args[4], args[5]}
               # else
                  ; _argv[0] = args[0], _argv[1] = args[1], _argv[2] = args[2], _argv[3] = args[3], _argv[4] = args[4], _argv[5] = args[5]
               # endif
                  ; { int sn = 6; do _argv[sn] = args[sn]; while (++sn < args.size()); }
                  for (int sn = 0; sn < argc; ++sn) _argv[sn + args.size()].swap(argv[sn]);
                  return MNL_LIKELY(!argv_out) ? target(argc + args.size(), _argv) : [&]()->val{
                     val _argv_out[argc + args.size()], res = target(argc + args.size(), _argv, _argv_out);
                     for (int sn = 0; sn < argc; ++sn) argv_out[sn].swap(_argv_out[sn + args.size()]);
                     return res;
                  }();
               }
            };
         default:
            vector<val> args; args.reserve(argc - 1);
            for (int sn = 1; sn < argc; ++sn) args.push_back(move(argv[sn]));
            return proc{move(argv[0]), move(args)};
         }
         {  static constexpr int argc_bound = 1;
            struct proc { val target, arg0;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
               # define MNL_M1 \
                  stk_check(); \
                  if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv); \
                  if (MNL_UNLIKELY(argc + argc_bound > val::max_argc)) MNL_ERR(MNL_SYM("LimitExceeded")); \
                  val _argv[argc + argc_bound] \
               // end # define MNL_M1
                  MNL_M1
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {arg0}
               # else
                  ; _argv[0] = arg0
               # endif
               # define MNL_M2 \
                  ; \
                  for (int sn = 0; sn < argc; ++sn) _argv[sn + argc_bound].swap(argv[sn]); \
                  return MNL_LIKELY(!argv_out) ? target(argc + argc_bound, _argv) : [&]()->val{ \
                     val _argv_out[argc + argc_bound], res = target(argc + argc_bound, _argv, _argv_out); \
                     for (int sn = 0; sn < argc; ++sn) argv_out[sn].swap(_argv_out[sn + argc_bound]); \
                     return res; \
                  }(); \
               // end # define MNL_M2
                  MNL_M2
               }
            };
         case argc_bound + 1: return proc{move(argv[0]), move(argv[1])};
         }
         {  static constexpr int argc_bound = 2;
            struct proc { val target, arg0, arg1;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
                  MNL_M1
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {arg0, arg1}
               # else
                  ; _argv[0] = arg0, _argv[1] = arg1
               # endif
                  MNL_M2
               }
            };
         case argc_bound + 1: return proc{move(argv[0]), move(argv[1]), move(argv[2])};
         }
         {  static constexpr int argc_bound = 3;
            struct proc { val target, arg0, arg1, arg2;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
                  MNL_M1
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {arg0, arg1, arg2}
               # else
                  ; _argv[0] = arg0, _argv[1] = arg1, _argv[2] = arg2
               # endif
                  MNL_M2
               }
            };
         case argc_bound + 1: return proc{move(argv[0]), move(argv[1]), move(argv[2]), move(argv[3])};
         }
         {  static constexpr int argc_bound = 4;
            struct proc { val target, arg0, arg1, arg2, arg3;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
                  MNL_M1
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {arg0, arg1, arg2, arg3}
               # else
                  ; _argv[0] = arg0, _argv[1] = arg1, _argv[2] = arg2, _argv[3] = arg3
               # endif
                  MNL_M2
               }
            };
         case argc_bound + 1: return proc{move(argv[0]), move(argv[1]), move(argv[2]), move(argv[3]), move(argv[4])};
         }
         {  static constexpr int argc_bound = 5;
            struct proc { val target, arg0, arg1, arg2, arg3, arg4;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
                  MNL_M1
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {arg0, arg1, arg2, arg3, arg4}
               # else
                  ; _argv[0] = arg0, _argv[1] = arg1, _argv[2] = arg2, _argv[3] = arg3, _argv[4] = arg4
               # endif
                  MNL_M2
               }
            };
         case argc_bound + 1: return proc{move(argv[0]), move(argv[1]), move(argv[2]), move(argv[3]), move(argv[4]), move(argv[5])};
         }
         {  static constexpr int argc_bound = 6;
            struct proc { val target, arg0, arg1, arg2, arg3, arg4, arg5;
               MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *argv_out) const {
                  MNL_M1
               # undef MNL_M1
               # if !__clang__ && !__INTEL_COMPILER // true GCC
                  {arg0, arg1, arg2, arg3, arg4, arg5}
               # else
                  ; _argv[0] = arg0, _argv[1] = arg1, _argv[2] = arg2, _argv[3] = arg3, _argv[4] = arg4, _argv[5] = arg5
               # endif
                  MNL_M2
               # undef MNL_M2
               }
            };
         case argc_bound + 1: return proc{move(argv[0]), move(argv[1]), move(argv[2]), move(argv[3]), move(argv[4]), move(argv[5]), move(argv[6])};
         }
      }
   }};

}} // namespace aux::<unnamed>

// I48 Range Constructors //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace {
   template<bool Rev> class proc_Range {
      MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
         if (MNL_LIKELY(argc == 1)) {
            if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
            if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
            return range<Rev>{0, cast<long long>(argv[0])};
         }
         if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<long long>(argv[0])) || MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         if (MNL_UNLIKELY(cast<long long>(argv[0]) > cast<long long>(argv[1]))) MNL_ERR(MNL_SYM("ConstraintViolation"));
         return range<Rev>{cast<long long>(argv[0]), cast<long long>(argv[1])};
      }
      friend box<proc_Range>;
   };
   template<bool Rev> class proc_RangeExt {
      MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
         if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         if (MNL_UNLIKELY(!test<long long>(argv[0])) || MNL_UNLIKELY(!test<long long>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
         return range<Rev>{cast<long long>(argv[0]), _add(cast<long long>(argv[0]), cast<long long>(argv[1]))};
      }
      friend box<proc_RangeExt>;
   };
}} // namespace aux::<unnamed>

namespace aux { extern "C" code mnl_aux_base() { // main ///////////////////////////////////////////////////////////////////////////////////////////////////////
   struct proc_F64 { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_LIKELY(test<long long>(argv[0]))) return (double)cast<long long>(argv[0]);
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      switch (cast<const string &>(argv[0])[0]) {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.':
         break;
      case '+': case '-':
         if (cast<const string &>(argv[0])[1] >= '0' && cast<const string &>(argv[0])[1] <= '9' || cast<const string &>(argv[0])[1] == '.') break;
         //[[fallthrough]];
      default:
         MNL_ERR(MNL_SYM("SyntaxError"));
      }
      char *end; auto res = strtod(cast<const string &>(argv[0]).c_str(), &end);
      if (MNL_UNLIKELY(*end)) MNL_ERR(MNL_SYM("SyntaxError"));
      if (MNL_UNLIKELY(isinf(res))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }};
   struct proc_F32 { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_LIKELY(test<long long>(argv[0]))) return (float)cast<long long>(argv[0]);
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      switch (cast<const string &>(argv[0])[0]) {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.':
         break;
      case '+': case '-':
         if (cast<const string &>(argv[0])[1] >= '0' && cast<const string &>(argv[0])[1] <= '9' || cast<const string &>(argv[0])[1] == '.') break;
         //[[fallthrough]];
      default:
         MNL_ERR(MNL_SYM("SyntaxError"));
      }
      char *end; auto res = strtof(cast<const string &>(argv[0]).c_str(), &end);
      if (MNL_UNLIKELY(*end)) MNL_ERR(MNL_SYM("SyntaxError"));
      if (MNL_UNLIKELY(isinf(res))) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }};
   struct proc_I48 { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_LIKELY(test<long long>(argv[0]))) return cast<long long>(argv[0]);
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (cast<const string &>(argv[0])[0] >= '0' && cast<const string &>(argv[0])[0] <= '9' ||
          cast<const string &>(argv[0])[0] == '+' || cast<const string &>(argv[0])[0] == '-'); else MNL_ERR(MNL_SYM("SyntaxError"));
      char *end; auto res = strtoll(cast<const string &>(argv[0]).c_str(), &end, {});
      if (MNL_UNLIKELY(*end)) MNL_ERR(MNL_SYM("SyntaxError"));
      if (MNL_UNLIKELY(res < min_i48) || MNL_UNLIKELY(res > max_i48)) MNL_ERR(MNL_SYM("Overflow"));
      return res;
   }};
   struct proc_MakeSym { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc == 0)) return (sym)nullptr;
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (cast<const string &>(argv[0])[0] != '`'); else MNL_ERR(MNL_SYM("SyntaxError"));
      return (sym)cast<const string &>(argv[0]);
   }};
   struct proc_U32 { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_LIKELY(test<long long>(argv[0]))) return (unsigned)cast<long long>(argv[0]);
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (cast<const string &>(argv[0])[0] >= '0' && cast<const string &>(argv[0])[0] <= '9'); else MNL_ERR(MNL_SYM("SyntaxError"));
      char *end; auto res = strtoull(cast<const string &>(argv[0]).c_str(), &end, {});
      if (MNL_UNLIKELY(*end)) MNL_ERR(MNL_SYM("SyntaxError"));
      if (MNL_UNLIKELY((unsigned)res != res)) MNL_ERR(MNL_SYM("ConstraintViolation"));
      return (unsigned)res;
   }};
   struct proc_MakePtr { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      switch (argc) {
      case 0: return pointer{};
      case 1: return pointer{move(argv[0])};
      case 2: return pointer{move(argv[0]), move(argv[1])};
      }
      MNL_ERR(MNL_SYM("InvalidInvocation"));
   }};
   struct proc_Parse { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      return parse(cast<const string &>(argv[0]));
   }};
   struct proc_IsList { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      return argv[0].is_list();
   }};
   struct proc_OrderEx { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      int res = argv[0].default_order(argv[1]);
      return MNL_LIKELY(res) ? res : MNL_SYM("Order")(2, argv);
   }};
   return expr_export{
      {"=",           comp_set{}},
      {"!",           comp_move{}},
      {"^",           comp_deref{}},
      {"if",          comp_if{}},
      {"&",           comp_and{}},
      {"|",           comp_or{}},
      {"while",       comp_while{}},
      {"repeat",      comp_repeat{}},
      {"on",          comp_on{}},
      {"signal",      comp_signal{}},
      {"for",         comp_for{}},
      {"case",        comp_case{}},
      {"proc",        comp_proc{}},
      {"var",         comp_var{}},
      {"let",         comp_let{}},
      {"export",      comp_export{}},
      {"scope",       comp_scope{}},
      {"array",       comp_array{}},
      {"record",      comp_record{}},
      {"object",      comp_object{}},
      {"@",           comp_att{}},
      {"map",         comp_map{}},
      {"set",         comp_set_c{}},
      {"sequence",    comp_sequence{}},
      {"macro",       comp_macro{}},
      {"#",           comp_code{}},
      {"error",       comp_error{}},
      {"eval",        comp_eval{}},
      {"F64",         make_lit(proc_F64{})},
      {"F32",         make_lit(proc_F32{})},
      {"I48",         make_lit(proc_I48{})},
      {"MakeSym",     make_lit(proc_MakeSym{})},
      {"U32",         make_lit(proc_U32{})},
      {"MakePtr",     make_lit(proc_MakePtr{})},
      {"Range",       make_lit(proc_Range<false>{})},
      {"RevRange",    make_lit(proc_Range<true>{})},
      {"RangeExt",    make_lit(proc_RangeExt<false>{})},
      {"RevRangeExt", make_lit(proc_RangeExt<true>{})},
      {"Parse",       make_lit(proc_Parse{})},
      {"IsI48",       make_proc_test<long long>()},
      {"IsF64",       make_proc_test<double>()},
      {"IsF32",       make_proc_test<float>()},
      {"IsSym",       make_proc_test<sym>()},
      {"IsS8",        make_proc_test<string>()},
      {"IsBool",      make_proc_test<bool>()},
      {"IsU32",       make_proc_test<unsigned>()},
      {"IsPtr",       make_proc_test<pointer>()},
      {"IsWeakPtr",   make_proc_test<w_pointer>()},
      {"IsRange",     make_proc_test<range<>>()},
      {"IsRevRange",  make_proc_test<range<true>>()},
      {"IsArray",     make_proc_test<vector<val>>()},
      {"IsMap",       make_proc_test<dict<val, val>>()},
      {"IsSet",       make_proc_test<dict<val>>()},
      {"IsSequence",  make_proc_test<list<val>>()},
      {"IsList",      make_lit(proc_IsList{})},
      {"IsCode",      make_proc_test<code>()},
      {"VarArg",      make_lit(proc_VarArg{})},
      {"VarApply",    make_lit(proc_VarApply{})},
      {"Bind",        make_lit(proc_Bind{})},
      {"Min",         make_lit(proc_Min{})},
      {"Max",         make_lit(proc_Max{})},
      {"OrderEx",     make_lit(proc_OrderEx{})},
   };
}}

} // namespace MNL_AUX_UUID
