// objcode-test.cc

# include <cstdio> // printf

# include <stdio.h> // ::printf

# include "objcode.hh"

int main() {
   rsn::objcode oc;

   {  auto ts = oc.text(), ds = oc.rodata();

      // first piece for the main text section
      ts .reserve(128);

      ts .align(16)                                    // .global proc; proc:
         .sl(0x4883EC'08)                              // subq $8, %rsp
         .b(0x53) .sw(0x4154) .sw(0x4156) .sw(0x4157); // push %rbx; push %r12; push %r14; push %r15
      ts .b(0x45).sw(0x31F6) .sw(0x41BF).l(1) // xorl %r14d, %r14d; movl $1, %r15d
         .b(0x45).sw(0x31E4);                 // xorl %r12d, %r12d

      auto l0 = oc.label();
      // loop begin
      ts .sw(0x89FB)               // movl %edi, %ebx
         .align(16, 10).label(l0);

      // another piece for the main text section
      ts .reserve(64);
      auto l1 = oc.label();
      ts .b(0x4C).sw(0x89F8) .sw(0x31D2) .b(0xB9).l(13) .b(0x48).sw(0xF7F1) // movq %r15, %rax; xorl %edx, %edx; movl $13, %ecx; divq %rcx
         .b(0x48).sw(0x09D2) .sw(0x0F84).rl(l1);                            // orq %rdx, %rdx; jz.d32 l1
      // auxiliary text section begin
      auto l2 = oc.label(), l_str = ds.label();
      ts.owner.text().reserve(64) .align(16).label(l1)
         .b(0x48).sw(0x8D3D).rl(l_str)                 // leaq l_str(%rip), %rdi
         .b(0x4C).sw(0x89FE)                           // movq %r15, %rsi
         .sw(0x48B8).q(::printf) .sw(0xFFD0)           // movabsq $printf, %rax; call *%rax
         .b(0x41).sw(0x83C4).b(1)                      // addl $1, %r12d
         // spin loop begin
         .b(0xB9).l(1'000'000'000)                     // movl $1*1000*1000*1000, %ecx
         .align(16, 6).label(l2)
         .b(0x90)                                      // nop
         .sw(0x83E9).b(1) .b(0x75).rb(l2)              // subl $1, %ecx; jnz.d8 l2
         // spin loop end
         .b(0xE9).rl(ts.align(16, 10).label());        // jmp.d32 0f
      ds .reserve(16) .label(l_str).b("x = %llu\n", sizeof "x = %llu\n");
      // auxiliary text section end
      ts                                                          // 0:
         .sl(0x4B8D043E) .b(0x4D).sw(0x89FE) .b(0x49).sw(0x89C7); // leaq (%r14,%r15), %rax; movq %r15, %r14; movq %rax, %r15
      // first piece continues here again

      ts .sw(0x83EB).b(1) .sw(0x0F85).rl(l0); // subl $1, %ebx; jnz.d32 l0
      // loop end

      ts .b(0x44).sw(0x89E0)                          // movl %r12d, %eax
         .sw(0x415F) .sw(0x415E) .sw(0x415C) .b(0x5B) // pop %r15; pop %r14; pop %r12; pop %rbx
         .sl(0x4883C4'08) .b(0xC3);                   // addq $8, %rsp; ret
   }

   std::printf("Found %d solutions\n", static_cast<int (*)(int)>(oc.load())(78));
   return 0;
}
