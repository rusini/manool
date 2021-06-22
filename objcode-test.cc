// objcode-test.cc

# include <cstdio>

# include "objcode.hh"

static void f() {
   std::puts("Hi");
}

int main() {
   rsn::objcode oc;
   auto l0 = oc.label(), l1 = oc.label();

   auto ts0 = oc.text(); ts0.reserve(128);
   ts0
   .align(16,10)
   .sl(0x4883EC'08)            // subq $8, %rsp
   .sw(0x48B8).q(f).sw(0xFFD0) // movabsq f, %rax; call *%rax
   .sl(0x4883C408)             // addq $8, %rsp
   .b(0xE9).rl(l0)             // jmp l0
   ;
   auto ts1 = oc.text().reserve(128);
   ts1
   .align(16, 10).label(l0)
   .b(0xB9).l(1'000'000'000)       // movl $1000000000, %ecx
   .align(16, 10).label(l1)
   .sw(0x83E9).b(1).b(0x75).rb(l1) // subl $1, %ecx; jnz l1
   .b(0xB8).l(123).b(0xC3)         // movl $123, %eax; ret
   ;

   std::printf("%d\n", static_cast<int (*)()>(oc.load())());
   return 0;
}
