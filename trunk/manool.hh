// manool.hh -- MANOOL Plug-In API (depends on: core, basis; dependency of: basis)

/*    Copyright (C) 2018, 2019 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# pragma once

# include "mnl-misc-memmgm.hh"

# include "mnl-aux-core.tcc"

// Macros
# define MNL_RVALUE() \
   MNL_INLINE static ::mnl::code compile(::mnl::code &&self, const ::mnl::form &form, const ::mnl::loc &loc) \
      { return ::mnl::aux::compile_apply(::std::move(self), form, loc); } \
   MNL_INLINE static void        exec_in(::mnl::val &&) { MNL_UNREACHABLE(); } \
   MNL_INLINE static ::mnl::val  exec_out()             { MNL_UNREACHABLE(); } \
   MNL_INLINE static bool        is_rvalue() { return true; } \
   MNL_INLINE static bool        is_lvalue() { return false; } \
   friend ::mnl::code;
# define MNL_LVALUE(COND) \
   MNL_INLINE static ::mnl::code compile(::mnl::code &&self, const ::mnl::form &form, const ::mnl::loc &loc) \
      { return ::mnl::aux::compile_apply(::std::move(self), form, loc); } \
   MNL_INLINE static bool        is_rvalue()       { return true; } \
   MNL_INLINE        bool        is_lvalue() const { return (COND); } \
   friend ::mnl::code;
# define MNL_NONVALUE() \
   MNL_INLINE static ::mnl::val  execute(bool = false)  { MNL_UNREACHABLE(); } \
   MNL_INLINE static void        exec_in(::mnl::val &&) { MNL_UNREACHABLE(); } \
   MNL_INLINE static ::mnl::val  exec_out()             { MNL_UNREACHABLE(); } \
   MNL_INLINE static bool        is_rvalue() { return false; } \
   MNL_INLINE static bool        is_lvalue() { return false; } \
   friend ::mnl::code;
// end
# define MNL_CATCH_UNEXPECTED \
   catch (decltype(::mnl::sig_state) &sig) { ::mnl::aux::panic(sig); } \
   catch (::mnl::stack_overflow &)         { ::mnl::aux::panic({MNL_SYM("StackOverflow"), {}}); } \
   catch (::mnl::heap_exhausted &)         { ::mnl::aux::panic({MNL_SYM("HeapExhausted"), {}}); } \
   catch (::std::bad_alloc &)              { ::mnl::aux::panic({MNL_SYM("LimitExceeded"), {}}); } \
   catch (::std::system_error &)           { ::mnl::aux::panic({MNL_SYM("SystemError"),   {}}); } \
// end

namespace MNL_AUX_UUID {

namespace aux { namespace pub {
   // Temporary Variable Accounting
   extern MNL_IF_WITH_MT(thread_local) int      tmp_cnt; // compile-time accounting
   extern MNL_IF_WITH_MT(thread_local) set<sym> tmp_ids; // ditto
   extern MNL_IF_WITH_MT(thread_local) vector<val>                  tmp_stk; // run-time accounting // could gain 5% on each var access
   extern MNL_IF_WITH_MT(thread_local) decltype(tmp_stk)::size_type tmp_frm; // ditto               // if we turn this into a struct

   code make_lit(const val &);

   // Convenience Functions
   code compile_rval(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){}), compile_lval(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});
   sym  eval_sym(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});
   code compile_rval(form::vci_range, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});
   // Convenience Class
   struct expr_export { MNL_NONVALUE()
      expr_export() = default;
      expr_export(initializer_list<pair<sym, code>> bind): bind(bind) {}
      expr_export(const vector<pair<sym, code>> &bind): bind(bind) {}
      MNL_INLINE expr_export(vector<pair<sym, code>> &&bind): bind(move(bind)) {}
   private:
      vector<pair<sym, code>> bind;
      MNL_INLINE inline code compile(code &&, const form &, const loc &) const;
   };
   extern template class code::box<expr_export>;
}} // namespace aux::pub

namespace aux {
   code compile_apply(code &&, const form &, const loc &);
   MNL_NORETURN void panic(const decltype(sig_state) &);
} // namespace aux

} // namespace MNL_AUX_UUID
