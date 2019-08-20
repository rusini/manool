// core-misc.cc

/*    Copyright (C) 2018, 2019 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <http://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-aux-core.tcc"

# include <cstdio> // sprintf, stderr, fprintf, fputs, fflush
# include <ctime>  // time

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::_Exit; // <cstdlib>
      using std::sprintf; using std::fprintf; using std::fputs; using std::fflush; // <cstdio>
      using std::time; // <time>
   }

// Translation Infrastructure /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_IF_WITH_MT(thread_local) decltype(symtab) pub::symtab;

   code pub::compile(const form &form, const loc &_loc) { return // *** The Compiler Core Dispatcher! ***
      test<sym>(form) && symtab[cast<const sym &>(form)] ?
         symtab[cast<const sym &>(form)] :
   # if 1
      test<sym>(form) && ((const string &)cast<const sym &>(form))[0] >= 'a' && ((const string &)cast<const sym &>(form))[0] <= 'z' ?
         (err_compile("unbound keyword (nested in this context)", form._loc(_loc)), code{}) :
   # endif
      test<long long>(form) || test<string>(form) || test<sym>(form) ?
         [&]()->code{ code make_lit(const val &); return make_lit(form); }() : // actually from Base
      form.is_list() && !form.empty() ?
         compile(form.front(), form._loc(_loc)).compile(form, form._loc(_loc)) :
      test<code>(form) ?
         cast<const code &>(form) :
      ((form.is_list() ? err_compile("invalid form", form._loc(_loc)) : MNL_ERR(MNL_SYM("TypeMismatch"))), code{});
   }

   void pub::err_compile(const char *msg, const loc &loc) {
      char buf[sizeof " 2147483647:2147483647-2147483647:2147483647 Error: "];
      throw pair<sym, val>{MNL_SYM("CompileError"), (!loc.origin ? (string)"<unknown location> Error: " : '(' + *loc.origin + ')' +
         (sprintf(buf, " %d:%d-%d:%d Error: ", loc._start.first, loc._start.second, loc._final.first, loc._final.second - 1), buf)) + msg};
   }

// class sym //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_PRIORITY(1003) decltype(sym::pool) sym::pool;
   MNL_IF_WITH_MT(MNL_PRIORITY(1004) decltype(sym::mutex) sym::mutex;)

   sym::sym(string txt): rep([&]()->decltype(rep){ // precond: txt[0] != '`'
      MNL_IF_WITH_MT( return std::lock_guard<std::mutex>{mutex}, [&]()->decltype(rep){ )
         decltype(dict)::const_iterator it = dict.find(txt);
         if (it != dict.end())
            return addref(it->second), it->second;
         if (!pool.empty()) {
            inverse[pool.back()] = it = dict.insert(make_pair(move(txt), pool.back())).first, pool.pop_back();
         } else {
            if (!static_cast<decltype(rep)>(dict.size())) // wrap-around means no more IDs available
               fputs("MANOOL: FATAL ERROR: Symbol space exhausted\n", stderr), fflush(stderr), _Exit(EXIT_FAILURE);
            inverse.reserve(dict.size() + 1), pool.reserve(dict.size() + 1);
            inverse.push_back(it = dict.insert(make_pair(move(txt), static_cast<decltype(rep)>(dict.size()))).first);
         }
         return rc[it->second] = 1, it->second;
      MNL_IF_WITH_MT( }(); )
   }()) {}
   sym::sym(const char *txt): sym((string)txt)
   {}
   sym::sym(decltype(nullptr)): rep([&]()->decltype(rep){
      MNL_IF_WITH_MT( return std::lock_guard<std::mutex>{mutex}, [&]()->decltype(rep){ )
         auto mask = MNL_AUX_RAND(unsigned short);
         decltype(dict)::const_iterator it;
         if (!pool.empty()) {
            char txt[sizeof "`65535"];
            sprintf(txt, "`%u", (unsigned)pool.back() ^ mask);
            inverse[pool.back()] = it = dict.insert(make_pair(move(txt), pool.back())).first, pool.pop_back();
         } else {
            if (!static_cast<decltype(rep)>(dict.size())) // wrap-around means no more IDs available
               fputs("MANOOL: FATAL ERROR: Symbol space exhausted\n", stderr), fflush(stderr), _Exit(EXIT_FAILURE);
            char txt[sizeof "`65535"];
            sprintf(txt, "`%u", (unsigned)dict.size() ^ mask);
            inverse.reserve(dict.size() + 1), pool.reserve(dict.size() + 1);
            inverse.push_back(it = dict.insert(make_pair(move(txt), static_cast<decltype(rep)>(dict.size()))).first);
         }
         return rc[it->second] = 1, it->second;
      MNL_IF_WITH_MT( }(); )
   }()) {}
   void sym::free(decltype(rep) rep) noexcept {
      MNL_IF_WITH_MT( std::lock_guard<std::mutex>{mutex}, [&]{ if (!__atomic_load_n(&rc[rep], __ATOMIC_RELAXED)) )
         dict.erase(inverse[rep]), pool.push_back(rep);
      MNL_IF_WITH_MT( }(); )
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   val val::default_invoke(const sym &op, int argc, val argv[]) {
   switch (MNL_DISP("==", "<>", "Order", "Clone", "DeepClone", "Str")[op]) {
   case 1: // ==
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      return  MNL_LIKELY(argv[0].rep.tag() == 0x7FF8u) && argv[0].rep.dat<void *>() == rep.dat<void *>();
   case 2: // <>
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      return !MNL_LIKELY(argv[0].rep.tag() == 0x7FF8u) || argv[0].rep.dat<void *>() != rep.dat<void *>();
   case 3: // Order
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(argv[0].rep.tag() != 0x7FF8u) || MNL_UNLIKELY(argv[0].rep.dat<void *>() != rep.dat<void *>())) MNL_ERR(MNL_SYM("TypeMismatch"));
      return 0;
   case 4: // Clone
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      return move(*this);
   case 5: // DeepClone
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      return static_cast<root *>(rep.dat<void *>())->invoke(move(*this), MNL_SYM("Clone"), 0, {});
   case 6: // Str
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      { static const val res = "value/object"; return res; }
   case 0:
      MNL_ERR(MNL_SYM("UnrecognizedOperation"));
   }}

// Signals, Exceptions, and Invocation Traces /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MNL_IF_WITH_MT(thread_local) decltype(sig_state) pub::sig_state;
   MNL_IF_WITH_MT(thread_local) decltype(sig_trace) pub::sig_trace = []()->decltype(sig_trace){ decltype(sig_trace) res; res.reserve(100 + 1); return res; }();
   void pub::trace_execute (const loc &loc) { if (loc.origin && sig_trace.size() < sig_trace.capacity()) sig_trace.push_back({loc, "evaluating"});         throw; }
   void pub::trace_exec_in (const loc &loc) { if (loc.origin && sig_trace.size() < sig_trace.capacity()) sig_trace.push_back({loc, "storing a value"});    throw; }
   void pub::trace_exec_out(const loc &loc) { if (loc.origin && sig_trace.size() < sig_trace.capacity()) sig_trace.push_back({loc, "moving-out a value"}); throw; }

   void aux::error(const sym &err)
      { throw make_pair(err, val{}); }
   void aux::error(const loc &loc, const sym &err)
      { if (loc.origin && sig_trace.size() < sig_trace.capacity()) sig_trace.push_back({loc, "evaluating"}); throw make_pair(err, val{}); }

// Record Descriptors /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   record_descr::record_descr(const record_descr &rhs) noexcept: rep(rhs.rep) { addref(); }
   record_descr::~record_descr() { release(); }
   record_descr &record_descr::operator=(const record_descr &rhs) noexcept { rhs.addref(), release(), rep = rhs.rep; return *this; }

   MNL_INLINE inline void record_descr::addref() const noexcept {
      if (rep == store.end()) return;
      MNL_IF_WITHOUT_MT(++rep->second.second) MNL_IF_WITH_MT(__atomic_add_fetch(&rep->second.second, 1, __ATOMIC_RELAXED));
   }
   MNL_INLINE inline void record_descr::release() const noexcept {
      if (rep == store.end()) return;
      if (MNL_UNLIKELY(! MNL_IF_WITHOUT_MT(--rep->second.second) MNL_IF_WITH_MT(__atomic_sub_fetch(&rep->second.second, 1, __ATOMIC_RELAXED)) ))
         MNL_IF_WITH_MT( std::lock_guard<std::mutex>{mutex}, [=]{ if (!__atomic_load_n(&rep->second.second, __ATOMIC_RELAXED)) )
            store.erase(rep);
         MNL_IF_WITH_MT( }(); )
   }
   record_descr::record_descr(set<sym> items)
   : rep((MNL_IF_WITH_MT(std::lock_guard<std::mutex>{mutex},) [&]()->decltype(rep){
      {  auto it = store.find(items); if (it != store.end())
            return MNL_IF_WITHOUT_MT(++it->second.second) MNL_IF_WITH_MT(__atomic_add_fetch(&it->second.second, 1, __ATOMIC_RELAXED)), it;
      }
      sym::tab<unsigned char> tab(-1); { int disp/*lacement*/ = 0; for (const auto &el: items) tab.update(el, disp++); }
      return store.insert(make_pair(move(items), make_pair(move((tab.shrink_to_fit(), tab)), 1))).first;
   }())) {}
   record_descr::record_descr(initializer_list<const char *> il): record_descr([=]()->set<sym>{
      set<sym> res; for (auto el: il) res.insert(el); return res;
   }()) {}

   decltype(record_descr::store) record_descr::store;
   MNL_IF_WITH_MT(decltype(record_descr::mutex) record_descr::mutex;)

// Seed Legacy Random Number Generator ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace aux { namespace { MNL_PRIORITY(1000) struct { int _ = (srand(time({})), 0); } _srand; }}

} // namespace MNL_AUX_UUID
