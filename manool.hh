// manool.hh -- MANOOL plug-in API (depends on: core, base; dependency of: base)

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# ifndef MNL_INCLUDED_MANOOL
# define MNL_INCLUDED_MANOOL

# include <system_error>

# include "mnl-misc-memmgm.hh"

# include "mnl-aux-core.tcc"

// Macros //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
   // Temporary Variable Accounting ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Compile-time accounting
   inline MNL_IF_WITH_MT(thread_local) int           tvar_cnt; // count for current frame layout
   inline MNL_IF_WITH_MT(thread_local) std::set<sym> tvar_ids; // all temporaries
   // Run-time accounting
   inline MNL_IF_WITH_MT(thread_local) std::vector<val>              tvar_stk; // stack
   inline MNL_IF_WITH_MT(thread_local) decltype(tvar_stk)::size_type tvar_off; // frame offset
   inline MNL_IF_WITH_MT(thread_local) val                          *tvar_frm; // frame pointer (redundant)

   inline MNL_IF_WITH_MT(thread_local) class tvar_stk {
      std::vector<val>            vector;
      decltype(vector)::size_type frm_off;
      decltype(vector)::pointer   frm_ptr;
   private:
      explicit tvar_stk() = default;
      tvar_stk(const tvar_stk &) = delete;
      tvar_stk &operator=(const tvar_stk &) = delete;
   public:
      MNL_INLINE void reserve(int count = 1)
         { vector.reserve(vector.size() + count), frm_ptr = vector.dat() + frm_off; }
      MNL_INLINE void push(decltype(nullptr), int count = 1)
         { vector.resize(vector.size() + count), frm_ptr = vector.dat() + frm_off; }
      template<typename Val> MNL_INLINE void push(Val &&val)
         { vector.push_back(std::forward<Val>(val)), frm_ptr = vector.dat() + frm_off; }
      MNL_INLINE void pop(int count = 1)
         { MNL_UNROLL(10) for (; count; --count) vector.pop_back(); }
   public:
      MNL_INLINE const val &operator[](int ix) const noexcept { return frm[ix]; }
      MNL_INLINE       val &operator[](int ix)       noexcept { return frm[ix]; }
   public:
      class new_frm_mgr {
         decltype(frm_off) saved_frm_off = _inst.frm_off;
      public:
         MNL_INLINE explicit new_frm_mgr() noexcept { _inst.frm_ptr = _inst.vector.data() + (_inst.frm_off = _inst.vector.size()); }
         MNL_INLINE ~new_frm_mgr()                  { _inst.frm_ptr = _inst.vector.data() + (_inst.frm_off = saved_frm_off); }
      };
   public:
      inline static MNL_IF_WITH_MT(thread_local) tvar_stk _inst;
   } &tvar_stk = tvar_stk::_inst;

   // Essential Stuff //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   code make_lit(const val &);

   // Convenience Stuff ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   code compile_rval(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){}), compile_lval(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});
   sym  eval_sym(const form &, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});
   code compile_rval(form::vci_range, const loc & = MNL_IF_GCC5(loc)MNL_IF_GCC6(loc){});

   class expr_export { MNL_NONVALUE()
   public:
      expr_export() = default;
      expr_export(initializer_list<pair<sym, code>> bind): bind(bind) {}
      expr_export(const vector<pair<sym, code>> &bind): bind(bind) {}
      MNL_INLINE expr_export(vector<pair<sym, code>> &&bind) noexcept: bind(move(bind)) {}
   private:
      vector<pair<sym, code>> bind;
      MNL_INLINE inline code compile(code &&, const form &, const loc &) const;
   };
   extern template class code::box<expr_export>;

   template<typename Dat> MNL_INLINE inline code make_proc_test() {
      struct proc { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
         if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
         return test<Dat>(argv[0]);
      }};
      return make_lit(proc{});
   }
}} // namespace aux::pub

namespace aux { // Helper Stuff ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   code compile_apply(code &&, const form &, const loc &);
   MNL_NORETURN void panic(const decltype(sig_state) &);
} // namespace aux

} // namespace MNL_AUX_UUID

# endif // # ifndef MNL_INCLUDED_MANOOL
