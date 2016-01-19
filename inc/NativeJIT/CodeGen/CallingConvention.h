#pragma once


namespace NativeJIT
{
#ifdef NATIVEJIT_PLATFORM_WINDOWS
    namespace CallingConvention
    {
        // Register masks of volatile integer and floating point registers.
        // TODO: Once constexpr is available, add operator| to Register and use it to define the masks.
        // Note: RIP is included in volatiles.
        static const unsigned c_rxxVolatileRegistersMask = 0xf07;     // 1 0000 1111 0000 0111 (RAX | RCX | RDX | R8 | R9 | R10 | R11 | RIP)
        static const unsigned c_xmmVolatileRegistersMask = 0x3f;      //   0000 0000 0011 1111 (XMM0-XMM5)

        // Register masks of non-volatile integer and floating point registers.
        // Note: RIP not included.
        static const unsigned c_rxxNonvolatileRegistersMask = 0xf0f8; // 0 1111 0000 1111 1000 (RBX | RSP | RBP | RSI | RDI | R12-R15)
        static const unsigned c_xmmNonvolatileRegistersMask = 0xffc0; //   1111 1111 1100 0000 (XMM6-XMM15)

        // Register masks of registers that can be written to.
        static const unsigned c_rxxWritableRegistersMask = 0xFFFF;    // Everything except RIP.
        static const unsigned c_xmmWritableRegistersMask = 0xFFFF;    // All XMM registers.
    }
#else
    namespace CallingConvention
    {
        // Register masks of volatile integer and floating point registers.
        // TODO: Once constexpr is available, add operator| to Register and use it to define the masks.
        // Note: RIP is included in volatiles.
        static const unsigned c_rxxVolatileRegistersMask = 0xfc7;     // 1 0000 1111 1100 0111 (RAX | RCX | RDX | RSI | RDI | R8 | R9 | R10 | R11 | RIP)
        static const unsigned c_xmmVolatileRegistersMask = 0xffff;    //   1111 1111 1111 1111 (XMM0-XMM15)
    
        // Register masks of non-volatile integer and floating point registers.
        // Note: RIP not included.
        static const unsigned c_rxxNonvolatileRegistersMask = 0xf038; // 0 1111 0000 0011 1000 (RBX | RSP | RBP | R12-R15)
        static const unsigned c_xmmNonvolatileRegistersMask = 0x0000; //   0000 0000 0000 0000 (none)
    
        // Register masks of registers that can be written to.
        static const unsigned c_rxxWritableRegistersMask = 0xFFFF;    // Everything except RIP.
        static const unsigned c_xmmWritableRegistersMask = 0xFFFF;    // All XMM registers.
    }
#endif
}
