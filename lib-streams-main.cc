// lib-streams-main.cc

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"

# include <stdio.h>
# include <sys/wait.h> // WEXITSTATUS
# include <string>
# if MNL_WITH_MULTITHREADING
   # include <mutex>
# endif

# include "manool.hh"

extern "C" mnl::code mnl_main() {
   using std::string;
   using mnl::sym; using mnl::val; using mnl::test; using mnl::cast;
   using mnl::make_lit; using mnl::expr_export;
   class stream {
      ::FILE *fp; void (*close_noexcept)(::FILE *) noexcept; val (*close)(::FILE *);
      enum { none, read, write } prev_op{};
      MNL_IF_WITH_MT(std::mutex mutex;)
   public:
      MNL_INLINE stream(::FILE *fp, void close_noexcept(::FILE *) noexcept, val close(::FILE *)) noexcept: fp(fp), close_noexcept(close_noexcept), close(close) {}
      MNL_INLINE stream(stream &&rhs) noexcept: fp(rhs.fp), close_noexcept(rhs.close_noexcept), close(rhs.close) { rhs.fp = {}; }
      MNL_INLINE ~stream() { if (fp) close_noexcept(fp); }
   private:
      MNL_INLINE val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
         switch (MNL_DISP("Write", "WriteLine", "Read", "ReadLine", "Flush", "Seek", "SeekCur", "SeekEnd", "CurPos", "Close")[op]) {
         case  1: // Write
            {  MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == read) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = write;
               for (int sn = 0; sn < argc; ++sn) {
                  auto res = MNL_SYM("Str")(argv[sn]);
                  if (MNL_UNLIKELY(!test<string>(res))) MNL_ERR(MNL_SYM("TypeMismatch"));
                  if (MNL_LIKELY(cast<const string &>(res).size()) &&
                     MNL_UNLIKELY(!::fwrite(cast<const string &>(res).c_str(), cast<const string &>(res).size(), 1, fp))) MNL_ERR(MNL_SYM("SystemError"));
               }
               return {};
               MNL_IF_WITH_MT(}();)
            }
         case  2: // WriteLine
            {  MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == read) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = write;
               for (int sn = 0; sn < argc; ++sn) {
                  auto res = MNL_SYM("Str")(argv[sn]);
                  if (MNL_UNLIKELY(!test<string>(res))) MNL_ERR(MNL_SYM("TypeMismatch"));
                  if (MNL_LIKELY(cast<const string &>(res).size()) &&
                     MNL_UNLIKELY(!::fwrite(cast<const string &>(res).c_str(), cast<const string &>(res).size(), 1, fp))) MNL_ERR(MNL_SYM("SystemError"));
               }
               if (MNL_UNLIKELY(putc_unlocked('\n', fp) < 0)) MNL_ERR(MNL_SYM("SystemError"));
               return {};
               MNL_IF_WITH_MT(}();)
            }
         case  3: // Read
            if (MNL_LIKELY(argc == 0)) {
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == write) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = read;
               string res;
               for (;;) { auto ch = getc_unlocked(fp);
                  if (MNL_UNLIKELY(::feof(fp))) return "";
                  if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
                  switch (ch) case ' ': case '\t': case '\f': case '\v': case '\r': case '\n': continue; res += ch; break;
               }
               for (;;) { auto ch = getc_unlocked(fp);
                  if (MNL_UNLIKELY(::feof(fp))) return res;
                  if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
                  switch (ch) case ' ': case '\t': case '\f': case '\v': case '\r': case '\n': { ::ungetc(ch, fp); return res; } res += ch;
               }
               MNL_IF_WITH_MT(}();)
            }
            if (MNL_LIKELY(argc == 1))
            if (MNL_LIKELY(test<long long>(argv[0]))) {
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               if (MNL_UNLIKELY((string::size_type)cast<long long>(argv[0]) != cast<long long>(argv[0]))) MNL_ERR(MNL_SYM("LimitExceeded"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == write) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = read;
               string res((string::size_type)cast<long long>(argv[0]), {});
               res.resize(::fread(&res.front(), 1, cast<long long>(argv[0]), fp)); if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError")); return res;
               MNL_IF_WITH_MT(}();)
            } else {
               if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == write) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = read;
               if (MNL_UNLIKELY(cast<const string &>(argv[0]).empty())) return read_all();
               string res;
               for (;;) { auto ch = getc_unlocked(fp);
                  if (MNL_UNLIKELY(::feof(fp))) return "";
                  if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
                  if (MNL_LIKELY(cast<const string &>(argv[0]).find(ch) != string::npos)) continue; res += ch; break;
               }
               for (;;) { auto ch = getc_unlocked(fp);
                  if (MNL_UNLIKELY(::feof(fp))) return res;
                  if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
                  if (MNL_UNLIKELY(cast<const string &>(argv[0]).find(ch) != string::npos)) { ::ungetc(ch, fp); return res; } res += ch;
               }
               MNL_IF_WITH_MT(}();)
            }
            MNL_ERR(MNL_SYM("InvalidInvocation"));
         case  4: // ReadLine
            if (MNL_LIKELY(argc == 0)) {
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == write) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = read;
               for (string res;;) { auto ch = getc_unlocked(fp);
                  if (MNL_UNLIKELY(::feof(fp))) if (MNL_LIKELY(!res.empty())) return res; else return {};
                  if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
                  if (MNL_UNLIKELY(ch == '\n')) return res; res += ch;
               }
               MNL_IF_WITH_MT(}();)
            }
            if (MNL_LIKELY(argc == 1)) {
               if (MNL_UNLIKELY(!test<string>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op == write) && MNL_UNLIKELY(::fseek(fp, {}, SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError")); prev_op = read;
               if (MNL_UNLIKELY(cast<const string &>(argv[0]).empty())) { auto res = read_all(); if (MNL_LIKELY(!res.empty())) return res; return {}; }
               for (string res;;) { auto ch = getc_unlocked(fp);
                  if (MNL_UNLIKELY(::feof(fp))) if (MNL_LIKELY(!res.empty())) return res; else return {};
                  if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
                  if (MNL_UNLIKELY(cast<const string &>(argv[0]).find(ch) != string::npos)) return res; res += ch;
               }
               MNL_IF_WITH_MT(}();)
            }
            MNL_ERR(MNL_SYM("InvalidInvocation"));
         case  5: // Flush
            {  if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(::fflush(fp))) MNL_ERR(MNL_SYM("SystemError"));
               return {};
               MNL_IF_WITH_MT(}();)
            }
         case  6: // Seek
            {  if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               if (MNL_UNLIKELY((::off_t)cast<long long>(argv[0]) != cast<long long>(argv[0]))) MNL_ERR(MNL_SYM("LimitExceeded"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op = {}, ::fseeko(fp, cast<long long>(argv[0]), {}))) MNL_ERR(MNL_SYM("SystemError"));
               return {};
               MNL_IF_WITH_MT(}();)
            }
         case  7: // SeekCur
            {  if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               if (MNL_UNLIKELY((::off_t)cast<long long>(argv[0]) != cast<long long>(argv[0]))) MNL_ERR(MNL_SYM("LimitExceeded"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op = {}, ::fseeko(fp, cast<long long>(argv[0]), SEEK_CUR))) MNL_ERR(MNL_SYM("SystemError"));
               return {};
               MNL_IF_WITH_MT(}();)
            }
         case  8: // SeekEnd
            {  if (MNL_UNLIKELY(argc != 1)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               if (MNL_UNLIKELY(!test<long long>(argv[0]))) MNL_ERR(MNL_SYM("TypeMismatch"));
               if (MNL_UNLIKELY(cast<long long>(argv[0]) < 0)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               if (MNL_UNLIKELY((::off_t)cast<long long>(argv[0]) != cast<long long>(argv[0]))) MNL_ERR(MNL_SYM("LimitExceeded"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               if (MNL_UNLIKELY(prev_op = {}, ::fseeko(fp, cast<long long>(argv[0]), SEEK_END))) MNL_ERR(MNL_SYM("SystemError"));
               return {};
               MNL_IF_WITH_MT(}();)
            }
         case  9: // CurPos
            {  if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
               MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               struct _ { ::FILE *fp; MNL_INLINE ~_() { ::funlockfile(fp); } } _{(::flockfile(fp), fp)}; ::clearerr(fp);
               auto res = ::ftello(fp); if (MNL_UNLIKELY(res < 0)) MNL_ERR(MNL_SYM("SystemError"));
               return (long long)res;
               MNL_IF_WITH_MT(}();)
            }
         case 10: // Close
            {  MNL_IF_WITH_MT(return std::lock_guard<std::mutex>(mutex), [&]()->val{)
               if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("ConstraintViolation"));
               auto fp = this->fp; this->fp = {}; return close(fp);
               MNL_IF_WITH_MT(}();)
            }
         }
         return self.default_invoke(op, argc, argv);
      }
      friend mnl::box<stream>;
   private:
      MNL_INLINE string read_all() const { string res;
         for (static const auto size = 1 * 1024 * 1024 /*1 MiB*/;;) {
            res.resize(res.size() + size); res.resize(res.size() - size + ::fread(&res.back() + 1 - size, 1, size, fp));
            if (MNL_UNLIKELY(::feof(fp))) return res; if (MNL_UNLIKELY(::ferror(fp))) MNL_ERR(MNL_SYM("SystemError"));
         }
      };
   };
   struct proc_OpenFile { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<string>(argv[0])) || MNL_UNLIKELY(!test<string>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      auto fp = ::fopen(cast<const string &>(argv[0]).c_str(), cast<const string &>(argv[1]).c_str());
      if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("SystemError"));
      return stream{fp, [](::FILE *fp)noexcept{ ::fclose(fp); }, [](::FILE *fp)->val{
         if (MNL_UNLIKELY(::fclose(fp))) MNL_ERR(MNL_SYM("SystemError")); return {}; }};
   }};
   struct proc_OpenPipe { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 2)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      if (MNL_UNLIKELY(!test<string>(argv[0])) || MNL_UNLIKELY(!test<string>(argv[1]))) MNL_ERR(MNL_SYM("TypeMismatch"));
      auto fp = ::popen(cast<const string &>(argv[0]).c_str(), cast<const string &>(argv[1]).c_str());
      if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("SystemError"));
      return stream{fp, [](::FILE *fp)noexcept{ ::pclose(fp); }, [](::FILE *fp)->val{
         auto res = ::pclose(fp); if (MNL_UNLIKELY(!WIFEXITED(res))) MNL_ERR(MNL_SYM("SystemError")); return (long long)WEXITSTATUS(res); }};
   }};
   struct proc_OpenTemp { MNL_INLINE static val invoke(val &&self, const sym &op, int argc, val argv[], val *) {
      if (MNL_UNLIKELY(op != MNL_SYM("Apply"))) return self.default_invoke(op, argc, argv);
      if (MNL_UNLIKELY(argc != 0)) MNL_ERR(MNL_SYM("InvalidInvocation"));
      auto fp = ::tmpfile();
      if (MNL_UNLIKELY(!fp)) MNL_ERR(MNL_SYM("SystemError"));
      return stream{fp, [](::FILE *fp)noexcept{ ::fclose(fp); }, [](::FILE *fp)->val{
         if (MNL_UNLIKELY(::fclose(fp))) MNL_ERR(MNL_SYM("SystemError")); return {}; }};
   }};
   return expr_export{
      {"Out",      make_lit(stream{stdout, [](::FILE *)noexcept{}, [](::FILE *fp)->val{
         if (MNL_UNLIKELY(::fflush(fp))) MNL_ERR(MNL_SYM("SystemError")); return {}; }})},
      {"Err",      make_lit(stream{stderr, [](::FILE *)noexcept{}, [](::FILE *fp)->val{
         if (MNL_UNLIKELY(::fflush(fp))) MNL_ERR(MNL_SYM("SystemError")); return {}; }})},
      {"In",       make_lit(stream{stdin,  [](::FILE *)noexcept{}, [](::FILE *)->val{ return {}; }})},
      {"OpenFile", make_lit(proc_OpenFile{})},
      {"OpenPipe", make_lit(proc_OpenPipe{})},
      {"OpenTemp", make_lit(proc_OpenTemp{})},
   };
}
