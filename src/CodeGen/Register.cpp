#include "stdafx.h"

#include "NativeJIT/Register.h"


namespace NativeJIT
{
    unsigned RegisterBase::c_sizes[9] = {
        0,
        0,  // 1 bytes
        1,  // 2 bytes
        0,
        2,  // 4 bytes
        0,
        0,
        0,
        3   // 8 bytes
    };


    // TODO: These names are not in the same order as the declarations below.
    char const * RegisterBase::c_names[2][4][16] =
    {
        {
            { "al", "bl", "cl", "dl", "bpl", "spl", "dil", "sil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" },
            { "ax", "bx", "cx", "dx", "bp", "si", "di", "sp", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" },
            { "eax", "ebx", "ecx", "edx", "ebp", "esi", "edi", "esp", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" },
            { "rax", "rbx", "rcx", "rdx", "rbp", "rsi", "rdi", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" },
        },
        {
            {},
            {},
            { "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"},
            { "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"}
        }
    };


    Register<1, false> ral(0);
    Register<1, false> rbl(1);
    Register<1, false> rcl(2);
    Register<1, false> rdl(3);
    Register<1, false> bpl(4);
    Register<1, false> sil(5);
    Register<1, false> dil(6);
    Register<1, false> spl(7);
    Register<1, false> r8b(8);
    Register<1, false> r9b(9);
    Register<1, false> r10b(10);
    Register<1, false> r11b(11);
    Register<1, false> r12b(12);
    Register<1, false> r13b(13);
    Register<1, false> r14b(14);
    Register<1, false> r15b(15);


    Register<8, false> rax(0);
    Register<8, false> rbx(1);
    Register<8, false> rcx(2);
    Register<8, false> rdx(3);
    Register<8, false> rbp(4);
    Register<8, false> rsi(5);
    Register<8, false> rdi(6);
    Register<8, false> rsp(7);
    Register<8, false> r8(8);
    Register<8, false> r9(9);
    Register<8, false> r10(10);
    Register<8, false> r11(11);
    Register<8, false> r12(12);
    Register<8, false> r13(13);
    Register<8, false> r14(14);
    Register<8, false> r15(15);
}
