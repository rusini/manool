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


   template<class Functor> struct defer: Functor {
      ~defer() { (*this)(); }
   private:
      defer(const defer &) = delete;
      defer &operator=(const defer &) = delete;
      using Functor::operator();
   };
   template<class Functor> defer(Functor)->defer<Functor>;


   //extern MNL_IF_WITH_MT(thread_local) union tvar_stack tvar_stack;

   // TODO: the fact we disable new does not preclude at all "dynamic" scoping (we can wrap in a struct)

   template<typename Elem> class vector {
      Elem *base;
      Elem *top, *limit;
   public:
      void reserve(int size) {
         if ((unsigned long)top + size * sizeof *top < (unsigned long)limit) return; // comparing addresses to avoid UB
      }
   };


   inline MNL_IF_WITH_MT(thread_local) struct vtable {
      int           cnt, max;
      std::set<sym> ids;
   } vtable;

   inline MNL_IF_WITH_MT(thread_local) class vstack {
      union mem {
         val _;
         mem() = default;
         ~mem() = default;
      } *frame, *limit;
   public:
      MNL_INLINE void push(decltype(nullptr) = {}) noexcept                           { push(nullptr); }
      template<typename Val> MNL_INLINE void push(Val &&_) noexcept(noexcept(val(_))) { new(&limit++->_) val(std::forward<Val>(_)); }
      MNL_INLINE const val &operator[](int index) const noexcept { return frame[index]; }
      MNL_INLINE       val &operator[](int index)       noexcept { return frame[index]; }
   private:
      MNL_INLINE void pop() noexcept { --limit->_.~val(); }
   public:
      MNL_INLINE auto scope_guard(int &count) noexcept {
         return finally{[&count]() MNL_INLINE{ MNL_UNROLL(10) for (; count; --count) pop(); }};
      }
      MNL_INLINE auto frame_guard(mem mem[]) noexcept {
         auto saved_frame = frame, saved_limit = limit; limit = frame = mem;
         return finally{[saved_frame, saved_limit]() MNL_INLINE{ frame = saved_frame, limit = saved_limit; }};
      }
   } vstack;



   inline MNL_IF_WITH_MT(thread_local) union tvar_stack {
      struct {
         std::vector<val> vec;
         int frm_off  = {};
         val *frm_ptr = {};
      } rep;
   public:
      MNL_INLINE explicit tvar_stack() noexcept {}
      MNL_INLINE ~tvar_stack() {}
   public:
      MNL_INLINE auto stack_guard() noexcept {
         new(&rep) struct rep;
         return finally{[this]() MNL_INLINE{ rep.~rep(); }};
      }
      MNL_INLINE auto frame_guard() noexcept {
         auto saved_frm_off = rep.frm_off; rep.frm_ptr = rep.vector.data() + (rep.frm_off = rep.vector.size());
         return finally{[this, saved_frm_off]() MNL_INLINE{ rep.frm_ptr = rep.vector.data() + (rep.frm_off = saved_frm_off); }};
      }
      MNL_INLINE auto scope_guard(int &count) {
         return finally{[this, &size]() MNL_INLINE{ MNL_UNROLL(10) for (; count; --count) rep.vector.pop_back(); }};
      }
      MNL_INLINE auto scope_guard(int &&size = 1) {
         return scope_guard(size);
      }
   public:
      MNL_INLINE void push(decltype(nullptr) = {})
         { rep.vec.emplace_back(nullptr), rep.frm_ptr = rep.vec.dat() + rep.frm_off; }
      template<typename Val> MNL_INLINE void push(Val &&val)
         { rep.vec.emplace_back(std::forward<Val>(val)), rep.frm_ptr = rep.vec.dat() + rep.frm_off; }
   public:
      MNL_INLINE const val &operator[](int ix) const noexcept { return rep.frm_ptr[ix]; }
      MNL_INLINE       val &operator[](int ix)       noexcept { return rep.frm_ptr[ix]; }




   public:
      class stack_manager: manager {
         decltype(rep) &_; // should not escape in practice and value propagation should be in place
      public:
         MNL_INLINE explicit stack_manager(tvar_stack &stack = mnl::tvar_stack) noexcept: _(stack.rep)
            { new(&stack) decltype(rep); }
         MNL_INLINE ~stack_manager()
            { typedef decltype(rep) rep; _.~rep(); }
      };
      class frame_manager: manager {
         decltype(rep) &_; // should not escape in practice and value propagation should be in place
         int saved_frm_off;
      public:
         MNL_INLINE explicit frame_manager(tvar_stack &stack = mnl::tvar_stack) noexcept: _(stack.rep), saved_frm_off(_.frm_ptr)
            { _.frm_ptr = _.vector.data() + (_.frm_off = _.vector.size()); }
         MNL_INLINE ~stack_manager()
            { _.frm_ptr = _.vector.data() + (_.frm_off = saved_frm_off); }
      };
      class scope_manager: manager {
         decltype(rep) &_;
         int size;
      public:
         MNL_INLINE explicit scope_manager(int size = 1, tvar_stack &stack = mnl::tvar_stack): _(stack.rep), size(size)
            { _.vector.reserve(size), _.frm_ptr = _.vector.data() + _.frm_off; }
         MNL_INLINE ~stack_manager()
            { MNL_UNROLL(10) for (; size; --size) _.vector.pop_back(); }
      };
   public:
      MNL_INLINE void push(decltype(nullptr), int count = 1)
         { vector.resize(vector.size() + count), frm_ptr = vector.dat() + frm_off; }
      template<typename Val> MNL_INLINE void push(Val &&val)
         { vector.push_back(std::forward<Val>(val)), frm_ptr = vector.dat() + frm_off; }
   public:
      MNL_INLINE const val &operator[](int ix) const noexcept { return rep.frm_ptr[ix]; }
      MNL_INLINE       val &operator[](int ix)       noexcept { return rep.frm_ptr[ix]; }


   public:
      MNL_INLINE void reserve(int count = 1)
         { vector.reserve(vector.size() + count), frm_ptr = vector.dat() + frm_off; }
      MNL_INLINE void push(decltype(nullptr), int count = 1)
         { vector.resize(vector.size() + count), frm_ptr = vector.dat() + frm_off; }
      template<typename Val> MNL_INLINE void push(Val &&val)
         { vector.push_back(std::forward<Val>(val)), frm_ptr = vector.dat() + frm_off; }
      MNL_INLINE void pop(int count = 1) noexcept
         { MNL_UNROLL(10) for (; count; --count) vector.pop_back(); }
   public:
      MNL_INLINE const val &operator[](int ix) const noexcept { return frm[ix]; }
      MNL_INLINE       val &operator[](int ix)       noexcept { return frm[ix]; }
   public:
      class buffer {
         std::byte buf[sizeof(tvar_stack)];
         friend tvar_stack;
      };

      class manager {
         void *operator new(std::size_t) = delete;
         void *operator new[](std::size_t) = delete;
         void operator delete(void *) noexcept = delete;
         void operator delete[](void *) noexcept = delete;
      };
      class stack_manager: manager {
         tvar_stack &stack;
      public:
         MNL_INLINE explicit stack_manager(buffer &stack): stack(*reinterpret_cast<tvar_stack *>(stack.buf)) { new(&stack) tvar_stack; }
         MNL_INLINE ~stack_manager() { stack.~tvar_stack(); }
      };



      // frame_manager
      // scope_manager
      // stack_manager -- construct/destruct (thread_local) stack (instead of using more expensive normal C++ construction)
      class new_frm_mgr {
         decltype(frm_off) saved_frm_off = _inst.frm_off;
      public:
         MNL_INLINE explicit new_frm_mgr() noexcept { _inst.frm_ptr = _inst.vector.data() + (_inst.frm_off = _inst.vector.size()); }
         MNL_INLINE ~new_frm_mgr()                  { _inst.frm_ptr = _inst.vector.data() + (_inst.frm_off = saved_frm_off); }
      private:
         void *operator new(std::size_t) = delete;
         void *operator new[](std::size_t) = delete;
         void operator delete(void *) noexcept = delete;
         void operator delete[](void *) noexcept = delete;
    };
   public:
      inline static MNL_IF_WITH_MT(thread_local) tvar_stk _inst;
   } tvar_stack;

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
