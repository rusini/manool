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

# include "mnl-misc-memmgm.hh"

# include "mnl-aux-core.tcc"

   struct expr_nonvalue {
      static code compile(code &&self, const form &, const loc &) = delete;
      MNL_INLINE static decltype(nullptr) execute(bool = {}) noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static void exec_nores(bool = {}) noexcept { MNL_UNREACHABLE(); }
      template<typename Val> MNL_INLINE static void exec_in(Val &&) noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static MNL_INLINE decltype(nullptr) exec_out() noexcept { MNL_UNREACHABLE(); }
      MNL_INLINE static bool is_rvalue() noexcept { return false; }
      MNL_INLINE static bool is_lvalue() noexcept { return false; }
   };
   struct expr_rvalue: expr_nonvalue {
      MNL_INLINE static code compile(code &&self, const form &form, const loc &loc) { return aux::compile_apply(std::move(self), form, loc); }
      static decltype(nullptr) execute(bool = {}) = delete;
      MNL_INLINE static void exec_nores(bool = {}) noexcept {} // may still be shadowed
      MNL_INLINE static bool is_rvalue() { return true; }
   };
   struct expr_lvalue: expr_rvalue {
      template<typename Val> static void exec_in(Val &&) = delete;
      static decltype(nullptr) exec_out() = delete;
      MNL_INLINE static bool is_lvalue() noexcept { return true; } // may still be shadowed
   };
// Macros //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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





   // Compile-time accounting
   extern MNL_IF_WITH_MT(thread_local) int      tmp_cnt; // count for current frame layout
   extern MNL_IF_WITH_MT(thread_local) set<sym> tmp_ids; // all temporaries
   // Run-time accounting
   extern MNL_IF_WITH_MT(thread_local) vector<val>                  tmp_stk; // stack
   extern MNL_IF_WITH_MT(thread_local) decltype(tmp_stk)::size_type tmp_frm; // frame pointer

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
