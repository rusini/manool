// objcode-test.cc

# include <cstdio> // printf

# include <stdio.h> // ::printf

# include "objcode.hh"

int main() {
   rsn::objcode oc;

   {  auto l0 = oc.label(), l1 = oc.label(), l2 = oc.label();
      auto ts = oc.text(), ds = oc.rodata();

      // first piece for the main text section

      ts .reserve(128);

      ts .align(16)        // .global _proc; _proc:
         .sl(0x4883EC'08); // subq $8, %rsp
      ts .b(0x45).sw(0x31F6) .sw(0x41BF).l(1) // xorl %r14d, %r14d; movl $1, %r15d
         .b(0x45).sw(0x31E4);                 // xorl %r12d, %r12d

      // loop header
      auto l0 = oc.label();
      ts .sw(0x89FB)           // movl %edi, %ebx
         .align(16).label(l0);

      auto l1 = oc.label(), l2 = oc.label();
      ts .b(0x4C).sb(0x89F8) .b(0xBA).l(13) .b(0x48).sw(0xF7F2) // movq %r15, %rax; movl $13, %edx; divq %edx
         .b(0x48).sw(0x09D2) .sw(0x0F84).rl(l1);                // orq %rdx, %rdx; jz.d32 l1
      auto l3 = oc.label();
      oc.text().reserve(64).label(l1)
      .b(0x48).sw(0x8D3D).rl(ds.reserve(32).align(2).label()) // leaq str_1(%rip), %rdi
      .b(0x4C).sw(0x89F6)                                     // movq %r14, %rsi
      .sw(0x48B8).q(::printf).sw(0xFFD0)                      // movabsq $printf, %rax; call *%rax
      // spin loop begin
      .b(0xB8).l(1'000'000'000)                               // movl $1*1000*1000*1000, %eax
      .align(16).label(l3)
      .b(0x90)                                                // nop
      .sw(0x83E8).b(1) .b(0x75).rb(l3);                       // subl $1, %eax; jnz.d8 0b
      // spin loop end
      .b(0xE9).sl(l2);                                        // jmp.d32 l2
      ds.b("%llu\n");
      ts
      .align(16, 10).label(l2)
      .sl(0x4B8D043E) .b(0x4D).sw(0x89FE) .b(0x49).sw(0x89C7) // leaq (%r14,%r15), %rax; movq %r15, %r14; movq %rax, %r15

      // loop end
      ts .sw(0x83EB).b(1) .sw(0x0F85).rl(l0); // subl $1, %ebx; jnz.d8 0b

      // another piece for the main text section

      ts.reserve(128);

      .b(0x48).sw(0x8D3D).rl(ds.reserve(32).align(2).label()) // leaq str_2(%rip), %rdi
      .b(0x4C).sw(0x89E6)                                     // movq %r12, %rsi
      .sw(0x48B8).q(::printf).sw(0xFFD0);                     // movabsq $printf, %rax; call *%rax
      ds.b("Found %d answer(s)\n");

      ts
      .b(0xB8).l(123)           // movl $123, %eax
      .sl(0x4883C4'08) .b(0xC3) // addq $8, %rsp; ret
   }

   std::printf("Return value: %d\n", static_cast<int (*)(int)>(oc.load())(98));
   return 0;
}
