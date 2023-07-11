// main.cc -- mnlexec main translation unit

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"

# include <stdlib.h>   // ::getenv, ::setenv
# include <fcntl.h>    // ::open, ::posix_fadvise
# include <sys/stat.h> // ::open, ::fstat, struct ::stat
# include <unistd.h>   // ::read, ::close
# include <sys/mman.h> // ::mmap, ::munmap, ::posix_madvise
# include <dlfcn.h>    // ::dlopen, ::dlsym, ::dlclose
# include <libgen.h>   // ::dirname
# include <cerrno>
# include <cctype>     // isdigit
# include <cstdio>     // printf, stderr, fputs, fflush, perror
# include <deque>

extern "C" char **environ;

# include "mnl-lib-base.hh" // MNL_NONVALUE(), class expr_export, class box<dict>, etc.

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::strtoll; using std::_Exit; using std::abort; // <cstdlib>
      using std::isdigit; // <cctype>
      using std::strlen; using std::strchr; // <cstring>
      using std::printf; using std::fputs; using std::fflush; using std::perror; // <cstdio>
      using std::deque;
   }

namespace aux { namespace {
   template<typename Functor> struct _finally { const Functor _; MNL_INLINE ~_finally() { _(); } };
   template<typename Functor> MNL_INLINE inline _finally<Functor> finally(Functor &&_) { return { (move)(_) }; }

   vector<string> mnl_path;

   struct comp_extern { MNL_NONVALUE()
      const char *local_lib_path;   // parameter for private lookup
      string     local_path_prefix; // ditto
   private:
      MNL_INLINE code compile(code &&, const form &form, const loc &_loc) const {
         if (form.size() != 2) err_compile("invalid form", _loc);

         deque<code> saved_tmp_ents;
         for (auto &&el: tmp_ids) saved_tmp_ents.push_back(symtab[el]), symtab.update(el, {});
         auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
         auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();

         auto path = compile_rval(form[1], _loc).execute(); if (!test<string>(path)) err_compile("type mismatch", _loc);

         tmp_ids = move(saved_tmp_ids);
         tmp_cnt = move(saved_tmp_cnt);
         for (auto &&el: tmp_ids) symtab.update(el, move(saved_tmp_ents.front())), saved_tmp_ents.pop_front();

         return do_extern(cast<const string &>(path), _loc);
      }
   private:
      code do_extern(const string &path, const loc &) const, try_path(const char *lib_path, const string &) const;
   };
   MNL_INLINE inline code comp_extern::do_extern(const string &path, const loc &_loc) const {
      if (path.empty() || strlen(path.c_str()) != path.size() || path.front() == '/' || path.back() == '/' ||
         path.find("//") != string::npos || path.find("/_") != string::npos) err_compile("invalid extern path", _loc);
      if (path.front() != '_') { // public lookup
         for (auto &&lib_path: mnl_path)
            if (auto res = try_path(lib_path.c_str(), path)) return res;
      } else { // private lookup
         for (auto prefix = local_path_prefix;;
            prefix.resize(prefix.size() - 1), prefix.resize(prefix.find('/') == string::npos ? 0 : prefix.rfind('/') + 1)) {
            if (auto res = try_path(local_lib_path, prefix + path)) return res;
            if (prefix.empty()) break;
         }
      }
      err_compile("missing extern entity", _loc);
   }
   code comp_extern::try_path(const char *lib_path, const string &path) const {
      auto inv_path /*inventory path*/ = lib_path + path;
      MNL_IF_WITH_MT(std::lock_guard<std::recursive_mutex> _lg([]()noexcept->std::recursive_mutex &{ static std::recursive_mutex _; return _; }());)
      static map<decltype(inv_path), const code> ent_by_path;
      {  auto it = ent_by_path.find(inv_path);
         if (it != ent_by_path.end()) return it->second;
      }
   opt1:
      {  auto fd = ::open((inv_path + ".mnl-strip").c_str(), O_RDONLY); if (fd < 0) goto opt2;
         auto _finally_fd = finally([&]()noexcept{ if (fd >= 0) ::close(fd); });
         struct ::stat st; if (::fstat(fd, &st) || !S_ISREG(st.st_mode) || !st.st_size || (::size_t)st.st_size != st.st_size) goto opt2;
         static map<pair<decltype(st.st_dev), decltype(st.st_ino)>, const code> ent_by_ino;
         {  auto it = ent_by_ino.find({st.st_dev, st.st_ino});
            if (it != ent_by_ino.end()) return ent_by_path.insert(make_pair(move(inv_path), it->second)), it->second;
         }
         auto ptr = ::mmap({}, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0); if (ptr == MAP_FAILED) goto opt2;
         auto _finally_ptr = finally([&]()noexcept{ if (ptr) ::munmap(ptr, st.st_size); }); ::close(fd), fd = -1;
         // everything is OK now - proceed with loading; read errors are fatal, as with ::dlopen
      # if !__ANDROID__
         ::posix_madvise(ptr, st.st_size, POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED);
      # endif
         string buf(static_cast<const char *>(ptr), st.st_size);
         auto res = [&]()->code{
            auto saved_symtab = move(symtab); symtab =
               {{"extern", comp_extern{lib_path, path.find('/') == string::npos ? "" : path.substr(0, path.rfind('/') + 1)}}};
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            auto res = mnl::compile((parse)(move(buf), ""));
            tmp_ids = move(saved_tmp_ids), tmp_cnt = move(saved_tmp_cnt); symtab  = move(saved_symtab);
            return res;
         }();
         if (res.is_rvalue()) res = make_lit(res.execute());
         ent_by_ino.insert(make_pair(make_pair(st.st_dev, st.st_ino), res)), ptr = {};
         ent_by_path.insert(make_pair(move(inv_path), res)); // consistent state left on exceptions
         return res; // ptr left ::memmap'ed until process termination, as with ::dlopen
      }
   opt2:
      {  auto fd = ::open((inv_path + ".mnl").c_str(), O_RDONLY); if (fd < 0) goto opt3;
         auto _finally_fd = finally([&]()noexcept{ if (fd >= 0) ::close(fd); });
         struct ::stat st; if (::fstat(fd, &st) || !S_ISREG(st.st_mode) || !st.st_size || (::size_t)st.st_size != st.st_size) goto opt3;
         static map<pair<decltype(st.st_dev), decltype(st.st_ino)>, const code> ent_by_ino;
         {  auto it = ent_by_ino.find({st.st_dev, st.st_ino});
            if (it != ent_by_ino.end()) return ent_by_path.insert(make_pair(move(inv_path), it->second)), it->second;
         }
         auto ptr = ::mmap({}, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0); if (ptr == MAP_FAILED) goto opt3;
         auto _finally_ptr = finally([&]()noexcept{ if (ptr) ::munmap(ptr, st.st_size); }); ::close(fd), fd = -1;
         // everything is OK now - proceed with loading; read errors are fatal, as with ::dlopen
      # if !__ANDROID__
         ::posix_madvise(ptr, st.st_size, POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED);
      # endif
         string buf(static_cast<const char *>(ptr), st.st_size);
         auto res = [&]()->code{
            auto saved_symtab = move(symtab); symtab =
               {{"extern", comp_extern{lib_path, path.find('/') == string::npos ? "" : path.substr(0, path.rfind('/') + 1)}}};
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            auto res = mnl::compile((parse)(move(buf), inv_path + ".mnl"));
            tmp_ids = move(saved_tmp_ids), tmp_cnt = move(saved_tmp_cnt); symtab  = move(saved_symtab);
            return res;
         }();
         if (res.is_rvalue()) res = make_lit(res.execute());
         ent_by_ino.insert(make_pair(make_pair(st.st_dev, st.st_ino), res)), ptr = {};
         ent_by_path.insert(make_pair(move(inv_path), res)); // consistent state left on exceptions
         return res; // ptr left ::memmap'ed until process termination, as with ::dlopen
      }
   opt3:
      {  auto saved_heap_res = heap_reserve(0);
         auto handle = (finally([&]()noexcept{ heap_reserve(saved_heap_res); }), ::dlopen)
            (((inv_path.find('/') != string::npos ? inv_path : "./" + inv_path) + ".mnl-plugin").c_str(), RTLD_LAZY | RTLD_LOCAL); // use DT_NEEDED to override LOCAL
         if (!handle) goto opt4;
         auto _finally_handle = finally([&]()noexcept{ if (handle) ::dlclose(handle); });
         static map<decltype(handle), const code> ent_by_handle;
         {  auto it = ent_by_handle.find(handle);
            if (it != ent_by_handle.end()) return ent_by_path.insert(make_pair(move(inv_path), it->second)), it->second;
         }
         auto main = reinterpret_cast<code (*)()>(::dlsym(handle, "mnl_main"));
         if (!main) goto opt4;
         // everything is OK now - proceed with loading
         auto res = [&]()->code{ // main() might want to compile MANOOL forms, so the binding environment is switched here as above
            auto saved_symtab = move(symtab); symtab =
               {{"extern", comp_extern{lib_path, path.find('/') == string::npos ? "" : path.substr(0, path.rfind('/') + 1)}}};
            auto saved_tmp_cnt = move(tmp_cnt); tmp_cnt = 0;
            auto saved_tmp_ids = move(tmp_ids); tmp_ids.clear();
            auto res = main();
            tmp_ids = move(saved_tmp_ids), tmp_cnt = move(saved_tmp_cnt); symtab  = move(saved_symtab);
            return res;
         }();
         ent_by_handle.insert(make_pair(handle, res)), handle = {};
         ent_by_path.insert(make_pair(move(inv_path), res)); // consistent state left on exceptions
         return res; // handle left ::dlopen until process termination
      }
   opt4:
      return {};
   }
}} // namespace aux::<unnamed>

namespace aux { static char **argv; }
namespace aux { extern "C" code mnl_aux_runtime() {
   struct proc_ProcessExit  { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      fflush({}), _Exit(cast<long long>(argv[0]));
   }};
   struct proc_ReserveStack { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
      return stk_reserve (cast<long long>(argv[0]));
   }};
   struct proc_ReserveHeap  { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
      return heap_reserve(cast<long long>(argv[0]));
   }};
   return expr_export{
      {"ProcessArgv", make_lit([]()->val{
         val res = vector<val>{};
         for (auto ppc = argv; *ppc; ++ppc) cast<vector<val> &>(res).push_back(*ppc);
         cast<vector<val> &>(res).shrink_to_fit();
         return res;
      }())},
      {"ProcessEnviron", make_lit([]()->val{
         val res = dict<val, val>{};
         for (auto ppc = ::environ; *ppc; ++ppc) if (strchr(*ppc, '='))
            cast<dict<val, val> &>(res).set(string(*ppc, strchr(*ppc, '=')), strchr(*ppc, '=') + 1);
         return res;
      }())},
      {"ProcessExit",  make_lit(proc_ProcessExit {})},
      {"ReserveStack", make_lit(proc_ReserveStack{})},
      {"ReserveHeap",  make_lit(proc_ReserveHeap {})},
   };
}}

namespace aux {
   MNL_INLINE static inline int main(char *argv[]) {
      aux::argv = argv;
      heap_limit(strtoll(::getenv("MNL_HEAP"),  {}, {}));
      stk_limit (strtoll(::getenv("MNL_STACK"), {}, {}));

      try {
         string buf;
         {  auto fd = ::open(*argv, O_RDONLY); if (fd < 0) return perror("Cannot open"), EXIT_FAILURE;
            struct ::stat st; if (::fstat(fd, &st)) return perror("Cannot stat"), EXIT_FAILURE;
         # if !__ANDROID__
            ::posix_fadvise(fd, {}, {}, POSIX_FADV_SEQUENTIAL | POSIX_FADV_WILLNEED | POSIX_FADV_NOREUSE); // NOREUSE -> EINVAL on CentOS 6
         # endif
            for (constexpr auto size = 1 * 1024*1024/*MiB*/;;) { // HACK: ?mostly optimal according to testing on *my* system
               buf.resize(buf.size() + size);
            # if 1
               auto count = ::read(fd, &*(buf.end() - size), size);
               if (MNL_UNLIKELY(count < 0)) return perror("Cannot read"), EXIT_FAILURE; // EINTR won't apply here, at least on glibc systems
            # else // paranoid - for broken systems
               int count; while (MNL_UNLIKELY((count = ::read(fd, &*(buf.end() - size), size)) < 0))
                  if (MNL_UNLIKELY(errno != EINTR)) return perror("Cannot read"), EXIT_FAILURE;
            # endif
               buf.resize(buf.size() - size + count);
               if (MNL_UNLIKELY(!count)) break;
            }
            buf.shrink_to_fit(); ::close(fd);
            if (buf[0] == '#' && buf[1] == '!') buf[1] = buf[0] = '-'; // #! strip shebang
            if (!S_ISREG(st.st_mode)) **argv = {};
         }

         mnl_path.push_back(**argv ? ::dirname(&string(*argv).front()) : "");
         if (!mnl_path.back().empty() && mnl_path.back().back() != '/') mnl_path.back() += '/';
         mnl_path.back().shrink_to_fit();
         for (const char *begin = ::getenv("MNL_PATH"), *end;; begin = end + 1) {
            mnl_path.push_back(string(begin, (end = strchr(begin, ':')) ? end : begin + strlen(begin)));
            if (!mnl_path.back().empty() && mnl_path.back().back() != '/') mnl_path.back() += '/';
            mnl_path.back().shrink_to_fit();
            if (!end) break;
         }
         mnl_path.shrink_to_fit();

         auto main = [&]()->code{ symtab.update("extern", comp_extern{mnl_path.front().c_str()});
            return finally([]()noexcept{ tmp_ids.clear(), tmp_cnt = 0; symtab.clear(); }), compile_rval((parse)(move(buf), **argv ? *argv : "<anonymous>"));
         }(); main.execute();
         fflush({}), _Exit(EXIT_SUCCESS)/*MT-friendly*/;
      }
      MNL_CATCH_UNEXPECTED
   }
   extern "C" code mnl_main() { // overrides definitions in other SOs under ordinary lookup
      fputs("mnl_main() was called via ordinary symbol lookup.\n", stderr), fflush(stderr), abort();
   }
} // namespace aux

} // namespace MNL_AUX_UUID

int main(int argc, char *argv[]) {
   if (!argc)
      fputs("A NULL argv[0] was passed through an exec system call.\n", stderr), fflush(stderr), abort();
   if (argc == 1) return printf(
      "Usage: %s <file> [<arg>...]\n%s", *argv, "There are no options. Environment variables:\n"
      "MNL_STACK Maximum allowed utilization of stack memory, in bytes (per thread)\n"
      "MNL_HEAP  Maximum allowed utilization of heap memory, in bytes\n"
      "MNL_PATH  ':'-separated list of directories in which to look for extern entities\n\n"
      "mnlexec (MANOOL) 0.6.0\n"
      "Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)\nLicense GPLv3: GNU GPL version 3 <https://gnu.org/licenses/gpl-3.0.html>\n"
      "This is free software: you are free to modify and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n"
      "Report bugs to: bugs@manool.org\nMANOOL home page: <https://manool.org>\n\n"), EXIT_SUCCESS;
   if (::setenv("MNL_MNLEXEC", *argv++, true))
      return perror("Cannot setenv"), EXIT_FAILURE;

   if (::setenv("MNL_STACK",
# ifdef MNL_STACK
   MNL_STACK
# else
   "6291456" // 6 MiB
# endif
   , false)) return perror("Cannot setenv"), EXIT_FAILURE;
   {  char *end; long long val; static const char nonzero = !0;
      if (isdigit(*::getenv("MNL_STACK"))) val = strtoll(::getenv("MNL_STACK"), &end, {}); else end = const_cast<char *>(&nonzero);
      if (*end && (errno = EINVAL) || val >= 1ll << 47 && (errno = ERANGE)) return perror("Ill-formed value MNL_STACK"), EXIT_FAILURE;
   }

   if (::setenv("MNL_HEAP",
# ifdef MNL_HEAP
   MNL_HEAP
# else
   "268435456" // 256 MiB
# endif
   , false)) return perror("Cannot setenv"), EXIT_FAILURE;
   {  char *end; long long val; static const char nonzero = !0;
      if (isdigit(*::getenv("MNL_HEAP")))  val = strtoll(::getenv("MNL_HEAP"),  &end, {}); else end = const_cast<char *>(&nonzero);
      if (*end && (errno = EINVAL) || val >= 1ll << 47 && (errno = ERANGE)) return perror("Ill-formed value MNL_HEAP"),  EXIT_FAILURE;
   }

   if (::setenv("MNL_PATH",
# ifdef MNL_PATH
   MNL_PATH
# else
   "/usr/local/lib/manool:/usr/lib/manool"
# endif
   , false)) return perror("Cannot setenv"), EXIT_FAILURE;

   return mnl::main(argv);
}
