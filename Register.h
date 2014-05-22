#pragma once

#include "Assert.h"

namespace NativeJIT
{
    template <unsigned SIZE, bool ISFLOAT>
    class Register;


    template <>
    class Register<8, false>
    {
    public:
        Register()
            : m_id(0)
        {
        }


        Register(unsigned id)
            : m_id(id)
        {
            Assert(id < 16, "Register id cannot exceed 15.");
        }


        unsigned GetId() const
        {
            return m_id;
        }


        char const * GetName() const
        {
            static char const * names[] = {
            "rax", "rbx", "rcx", "rdx", "rbp", "rsi", "rdi", "rsp",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            };

            return names[m_id];
        }

    private:
        unsigned m_id;
    };


    template <>
    class Register<4, false>
    {
    public:
        Register()
            : m_id(0)
        {
        }


        Register(unsigned id)
            : m_id(id)
        {
            Assert(id < 16, "Register id cannot exceed 15.");
        }


        unsigned GetId() const
        {
            return m_id;
        }


        char const * GetName() const
        {
            static char const * names[] = {
            "eax", "ebx", "ecx", "edx", "ebp", "esi", "edi", "esp",
            "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
            };

            return names[m_id];
        }

    private:
        unsigned m_id;
    };


    template <>
    class Register<1, false>
    {
    public:
        Register(unsigned id)
            : m_id(id)
        {
            Assert(id != 4, "Register bp not supported.");
            Assert(id != 5, "Register si not supported.");
            Assert(id != 6, "Register di not supported.");
            Assert(id != 7, "Register sp not supported.");
        }


        unsigned GetId() const
        {
            return m_id;
        }


        char const * GetName() const
        {
            static char const * names[] = {
            "al", "bl", "cl", "dl", "xxx", "xxx", "xxx", "xxx",
            "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
            };

            return names[m_id];
        }

    private:
        unsigned m_id;
    };


    template <>
    class Register<8, true>
    {
    public:
        Register()
            : m_id(0)
        {
        }


        Register(unsigned id)
            : m_id(id)
        {
            Assert(id < 16, "Register id cannot exceed 15.");
        }


        unsigned GetId() const
        {
            return m_id;
        }


        char const * GetName() const
        {
            static char const * names[] = {
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", 
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
            };

            return names[m_id];
        }

    private:
        unsigned m_id;
    };
}
