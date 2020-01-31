// core-reader.cc

/*    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# include "config.tcc"
# include "mnl-aux-core.tcc"

# include <cctype> // isalpha, isdigit, isalnum

namespace MNL_AUX_UUID { using namespace aux;
   namespace aux {
      using std::strtoll; // <cstdlib>
      using std::isalpha; using std::isdigit; using std::isalnum; // <cctype>
      using std::make_shared; // <memory>
   }

namespace aux { namespace {

   MNL_IF_WITH_MT(thread_local) enum {
      tk_end,
      tk_lit,
      tk_lpar,
      tk_rpar,
      tk_period,
      tk_colon,
      tk_semicolon,
      tk_lbrack,
      tk_rbrack,
      tk_lbrace,
      tk_rbrace,
      tk_add,
      tk_mul,
      tk_rel,
      tk_equ,
      tk_pref,
      tk_post,
   }
      curr_typ;
   MNL_IF_WITH_MT(thread_local) ast
      curr_ast;
   MNL_IF_WITH_MT(thread_local) loc
      curr_loc;
   MNL_IF_WITH_MT(thread_local) decltype(curr_typ)
      prev_typ;
   MNL_IF_WITH_MT(thread_local) decltype(curr_loc._final)
      prev_loc_final;

   void scan_startup(const string &, string &&), scan_cleanup() noexcept;
   void scan(); // precond: C's locale is "C"

}} // namespace aux::<unnamed>

// Parser /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux { namespace {

   MNL_INLINE inline void err_compile(const char *msg) { err_compile(msg, curr_loc); }

   MNL_INLINE inline void err_syntax_error() { // TODO: MNL_NORETURN for diagnostic purposes in g++-8?
      err_compile("syntax error (unexpected token)");
   }
   MNL_INLINE inline ast parse(decltype(curr_typ) typ = curr_typ) {
      if (curr_typ != typ) err_syntax_error();
      auto res = move(curr_ast);
      prev_loc_final = curr_loc._final;
      prev_typ = curr_typ, scan();
      return res;
   }

   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   ast parse_expr(), parse_expr0(), parse_simple(), parse_term(), parse_factor(), parse_prim(), parse_prim0();
   vector<ast> parse_args(vector<ast> = {}), parse_list(), parse_list0();

   ast parse_expr() { // expr: expr0 | expr0 "<equ-op>" expr0
      auto start = curr_loc._start; auto lhs = parse_expr0();
      if (curr_typ == tk_equ) lhs = {{parse(), move(lhs), parse_expr0()}, {curr_loc.origin, start, prev_loc_final}};
      return lhs;
   }
   ast parse_expr0() { // expr0: simple | simple "<rel-op>" simple
      auto start = curr_loc._start; auto lhs = parse_simple();
      if (curr_typ == tk_rel) lhs = {{parse(), move(lhs), parse_simple()}, {curr_loc.origin, start, prev_loc_final}};
      return lhs;
   }
   ast parse_simple() { // simple: term | simple "<add-op>" term
      auto start = curr_loc._start; auto lhs = parse_term();
      while (curr_typ == tk_add) lhs = {{parse(), move(lhs), parse_term()}, {curr_loc.origin, start, prev_loc_final}};
      return lhs;
   }
   ast parse_term() { // term: factor | term "<mul-op>" factor
      auto start = curr_loc._start; auto lhs = parse_factor();
      while (curr_typ == tk_mul) lhs = {{parse(), move(lhs), parse_factor()}, {curr_loc.origin, start, prev_loc_final}};
      return lhs;
   }
   ast parse_factor() { // NEARLY factor: prim | "<pref-op>" factor
      if (curr_typ != tk_pref) return parse_prim();
      auto start = curr_loc._start;
      if (prev_typ != tk_lpar) return {{parse(), parse_factor()}, {curr_loc.origin, start, prev_loc_final}};
      return [&]()->ast{ auto res = parse();
         if (curr_typ == tk_rpar) return res;
         res = {{move(res), parse_factor()}, {curr_loc.origin, start, prev_loc_final}}; return res; }();
   }
   ast parse_prim() { // prim: prim0 | prim "[" args "]" | prim "." prim0 "[" args "]" | prim "<post-op>"
      auto start = curr_loc._start; auto lhs = parse_prim0();
      for (;;) switch (curr_typ) {
      default:
         return lhs;
      case tk_lbrack:
         lhs = {parse_args({(parse(), move(lhs))}), {curr_loc.origin, start, (parse(tk_rbrack), prev_loc_final)}};
         continue;
      case tk_period:
         lhs = {parse_args({(parse(), parse_prim0()), (parse(tk_lbrack), move(lhs))}), {curr_loc.origin, start, (parse(tk_rbrack), prev_loc_final)}};
         continue;
      case tk_post:
         lhs = {{parse(), move(lhs)}, {curr_loc.origin, start, prev_loc_final}};
         continue;
      }
   }
   ast parse_prim0() {
      // NEARLY prim0: "<lit>" | "(" op ")" | "{" list "}" | "(" expr ")"
      // op: "<equ-op>" | "<rel-op>" | "<add-op>" | "<mul-op>" | "<pref-op>" | "<post-op>"
      switch (curr_typ) {
      case tk_lit:
         return parse();
      case tk_lbrace:
         {  auto start = curr_loc._start;
            return {(parse(), parse_list()), {curr_loc.origin, start, (parse(tk_rbrace), prev_loc_final)}};
         }
      case tk_lpar:
         switch (parse(), curr_typ) case tk_equ: case tk_rel: case tk_add: case tk_mul: case tk_post:
            return []()->ast{ auto res = parse(); parse(tk_rpar); return res; }();
         return []()->ast{ auto res = parse_expr(); parse(tk_rpar); return res; }();
      }
      err_syntax_error(); // @@@ warning: control reaches end of non-void function
   }
   vector<ast> parse_args(vector<ast> lhs) {
      // args: args0 | /*epsilon*/
      // args0: expr args | expr ";" args0
      if (curr_typ == tk_rbrack) return lhs;
      for (;;) {
         lhs.push_back(parse_expr());
         switch (curr_typ) {
         case tk_rbrack: return lhs;
         case tk_semicolon: parse();
         }
      }
   }
   vector<ast> parse_list() { // list: list0 | /*epsilon*/
      if (curr_typ == tk_rbrace) return {};
      return parse_list0();
   }
   vector<ast> parse_list0() { // list0: expr list | expr ";" list0 | expr ":" list0
      vector<ast> lhs;
      for (;;) {
         lhs.push_back(parse_expr());
         switch (curr_typ) {
         case tk_rbrace:
            return lhs;
         case tk_colon:
            {  auto start = curr_loc._start;
               lhs.push_back({(parse(), parse_list0()), {curr_loc.origin, start, curr_loc._final}}); return lhs;
            }
         case tk_semicolon:
            parse();
         }
      }
   }

}} // namespace aux::<unnamed>

namespace aux { namespace pub {
   ast parse(const string &source, string origin) {
      try {
         scan_startup(source, move(origin));
         auto res = parse_expr(); parse(tk_end);
         scan_cleanup();
         return res;
      } catch (...) {
         scan_cleanup();
         throw;
      }
   }
}} // namespace aux::pub

// Scanner ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace aux { namespace {

   MNL_IF_WITH_MT(thread_local) const char *pc;

   void scan_startup(const string &source, string &&origin) {
      pc = source.c_str();
      curr_loc.origin = make_shared<string>(move(origin)), curr_loc._final = {1, 1};
      prev_typ = tk_end, scan();
   }
   void scan_cleanup() noexcept {
      curr_ast = {}, curr_loc.origin.reset();
   }

   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   void scan() {
      for (;;) switch (*pc) {
      case ' ': case '\t': case '\f': case '\v':
         ++curr_loc._final.second, ++pc;
         continue;
      case '\n':
         ++curr_loc._final.first, curr_loc._final.second = 1, ++pc;
         continue;
      case 0:
         curr_loc._start = curr_loc._final, ++curr_loc._final.second;
         curr_typ = tk_end, curr_ast = {};
         return;
      case '-':
         if (*(++curr_loc._final.second, ++pc) != '-') {
            curr_loc._start = {curr_loc._final.first, curr_loc._final.second - 1};
            curr_typ = tk_add, curr_ast = MNL_SYM("-");
            return;
         }
         do ++curr_loc._final.second, ++pc; while (*pc != '\n' && *pc);
         continue;
      case '/':
         if (*(++curr_loc._final.second, ++pc) != '*') {
            curr_loc._start = {curr_loc._final.first, curr_loc._final.second - 1};
            curr_typ = tk_mul, curr_ast = MNL_SYM("/");
            return;
         }
         ++curr_loc._final.second, ++pc;
         for (int depth = 1;;) {
            switch (*pc) {
            case '/':
               if (*(++curr_loc._final.second, ++pc) != '*') continue;
               ++depth;
               //[[fallthrough]];
            default:
               ++curr_loc._final.second, ++pc;
               continue;
            case '\n':
               ++curr_loc._final.first, curr_loc._final.second = 1, ++pc;
               continue;
            case 0:
               curr_loc._start = curr_loc._final, ++curr_loc._final.second;
               err_compile("unexpected end of input");
            case '-':
               if (*(++curr_loc._final.second, ++pc) == '-') do ++curr_loc._final.second, ++pc; while (*pc != '\n' && *pc);
               continue;
            case '"':
               do if (*(++curr_loc._final.second, ++pc) == '"') { ++curr_loc._final.second, ++pc; break; } while (*pc != '\n' && *pc);
               continue;
            case '\\':
               if (*(++curr_loc._final.second, ++pc) == '}') for (;;) {
                  switch (*(++curr_loc._final.second, ++pc))
                  case '\\': if (*(++curr_loc._final.second, ++pc) != '{') default: continue; else case 0: ; // hacky :-)
                  break;
               }
               continue;
            case '*':
               if (*(++curr_loc._final.second, ++pc) != '/') continue;
               if (*(++curr_loc._final.second, ++pc) == '*') { ++curr_loc._final.second, ++pc; ++depth; continue; }
               if (--depth) continue;
               //break;
            }
            break;
         }
         continue;
      case '(':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_lpar,      curr_ast = {}; return;
      case ')':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_rpar,      curr_ast = {}; return;
      case '.':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_period,    curr_ast = {}; return;
      case ':':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_colon,     curr_ast = {}; return;
      case ';':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_semicolon, curr_ast = {}; return;
      case '[':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_lbrack,    curr_ast = {}; return;
      case ']':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_rbrack,    curr_ast = {}; return;
      case '{':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_lbrace,    curr_ast = {}; return;
      case '}':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_rbrace,    curr_ast = {}; return;
      case '+':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_add,       curr_ast = MNL_SYM("+"); return;
      case '|':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_add,       curr_ast = MNL_SYM("|"); return;
      case '*':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_mul,       curr_ast = MNL_SYM("*"); return;
      case '&':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_mul,       curr_ast = MNL_SYM("&"); return;
      case '~':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_pref,      curr_ast = MNL_SYM("~"); return;
      case '!':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("!"); return;
      case '#':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("#"); return;
      case '$':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("$"); return;
      case '%':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("%"); return;
      case '\'': curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("'"); return;
      case '?':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("?"); return;
      case '@':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("@"); return;
      case '^':  curr_loc._start = curr_loc._final, ++curr_loc._final.second, ++pc; curr_typ = tk_post,      curr_ast = MNL_SYM("^"); return;
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      case '=':
         curr_loc._start = curr_loc._final, ++curr_loc._final.second;
         switch (*++pc) {
         case '=': ++curr_loc._final.second, ++pc; curr_typ = tk_rel, curr_ast = MNL_SYM("=="); return;
         }
         curr_typ = tk_equ, curr_ast = MNL_SYM("=");
         return;
      case '<':
         curr_typ = tk_rel;
         curr_loc._start = curr_loc._final, ++curr_loc._final.second;
         switch (*++pc) {
         case '=': ++curr_loc._final.second, ++pc; curr_ast = MNL_SYM("<="); return;
         case '>': ++curr_loc._final.second, ++pc; curr_ast = MNL_SYM("<>"); return;
         }
         curr_ast = MNL_SYM("<");
         return;
      case '>':
         curr_typ = tk_rel;
         curr_loc._start = curr_loc._final, ++curr_loc._final.second;
         switch (*++pc) {
         case '=': ++curr_loc._final.second, ++pc; curr_ast = MNL_SYM(">="); return;
         }
         curr_ast = MNL_SYM(">");
         return;
      case '"':
         curr_loc._start = curr_loc._final;
         for (;;) switch (*(++curr_loc._final.second, ++pc)) {
         case '"':
            curr_typ = tk_lit, curr_ast = string(pc - (curr_loc._final.second - curr_loc._start.second) + 1, pc);
            ++curr_loc._final.second, ++pc;
            return;
         case '\n':
            curr_loc._start = curr_loc._final, ++curr_loc._final.second;
            err_compile("unexpected line break");
         case 0:
            curr_loc._start = curr_loc._final, ++curr_loc._final.second;
            err_compile("unexpected end of input");
         }
      case '\\':
         curr_loc._start = curr_loc._final;
         if (*(++curr_loc._final.second, ++pc) == '}')
         for (auto start = ++pc;;) switch (*(++curr_loc._final.second, ++pc)) {
         case '\\':
            if (*(++curr_loc._final.second, ++pc) != '{') continue;
            curr_typ = tk_lit, curr_ast = string(start, pc - 1);
            ++curr_loc._final.second, ++pc;
            return;
         case 0:
            curr_loc._start = curr_loc._final, ++curr_loc._final.second;
            err_compile("unexpected end of input");
         }
         curr_loc._start = curr_loc._final, ++curr_loc._final.second;
         err_compile("unexpected character");
      default:
         if (isalpha(*pc) || *pc == '_') {
            curr_loc._start = curr_loc._final;
            do ++curr_loc._final.second, ++pc; while (isalnum(*pc) || *pc == '_');
            curr_typ = tk_lit, curr_ast = (sym)string(pc - (curr_loc._final.second - curr_loc._start.second), pc);
            if (cast<const sym &>(curr_ast) == MNL_SYM("_")) curr_ast = (sym)nullptr;
            return;
         }
         if (isdigit(*pc)) {
            curr_loc._start = curr_loc._final;
            do ++curr_loc._final.second, ++pc; while (isdigit(*pc));
            if (isalpha(*pc) || *pc == '_') {
               curr_loc._start = curr_loc._final, ++curr_loc._final.second;
               err_compile("unexpected alphabetic character");
            }
            auto res = strtoll(pc - (curr_loc._final.second - curr_loc._start.second), {}, 10);
            if (res > max_i48) err_compile("value too large");
            curr_typ = tk_lit, curr_ast = res;
            return;
         }
         curr_loc._start = curr_loc._final, ++curr_loc._final.second;
         err_compile("unexpected character");
      }
   } // void scan()

}} // namespace aux::<unnamed>

} // namespace MNL_AUX_UUID

/* Grammar:

start: expr

expr: expr0
expr: expr0 "<equ-op>" expr0

expr0: simple
expr0: simple "<rel-op>" simple

simple: term
simple: simple "<add-op>" term

term: factor
term: term "<mul-op>" factor

factor: prim
factor: "<pref-op>" factor

prim: prim0
prim: prim "[" args "]"
prim: prim "." prim0 "[" args "]"
prim: prim "<post-op>"

prim0: "<lit>" | "(" op ")"
prim0: "{" list "}" | "(" expr ")"

op: "<equ-op>" | "<rel-op>" | "<add-op>" | "<mul-op>" | "<pref-op>" | "<post-op>"

args: args0 |

args0: expr args
args0: expr ";" args0

list: list0 |

list0: expr list
list0: expr ";" list0
list0: expr ":" list0

*/
