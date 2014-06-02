#include "stdafx.h"


#include "Register.h"


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
}
