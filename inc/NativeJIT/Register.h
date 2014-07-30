#pragma once

#include "Temporary/Assert.h"


namespace NativeJIT
{
    class RegisterBase
    {
    protected:
        static unsigned c_sizes[9];
        static char const * c_names[2][4][16];
    };


    template <unsigned SIZE, bool ISFLOAT>
    class Register : public RegisterBase
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


        template <unsigned SIZE>
        Register(Register<SIZE, false> r)
            : m_id(r.GetId())
        {
        }


        unsigned GetId() const
        {
            return m_id;
        }


        char const * GetName() const
        {
            return c_names[ISFLOAT ? 1 : 0][c_sizes[SIZE]][m_id];
        }

    private:
        unsigned m_id;
    };


    extern Register<8, false> ral;
    extern Register<8, false> rbl;
    extern Register<8, false> rcl;
    extern Register<8, false> rdl;
    extern Register<8, false> bpl;
    extern Register<8, false> sil;
    extern Register<8, false> dil;
    extern Register<8, false> spl;
    extern Register<8, false> r8b;
    extern Register<8, false> r9b;
    extern Register<8, false> r10b;
    extern Register<8, false> r11b;
    extern Register<8, false> r12b;
    extern Register<8, false> r13b;
    extern Register<8, false> r14b;
    extern Register<8, false> r15b;

    extern Register<8, false> rax;
    extern Register<8, false> rbx;
    extern Register<8, false> rcx;
    extern Register<8, false> rdx;
    extern Register<8, false> rbp;
    extern Register<8, false> rsi;
    extern Register<8, false> rdi;
    extern Register<8, false> rsp;
    extern Register<8, false> r8;
    extern Register<8, false> r9;
    extern Register<8, false> r10;
    extern Register<8, false> r11;
    extern Register<8, false> r12;
    extern Register<8, false> r13;
    extern Register<8, false> r14;
    extern Register<8, false> r15;
}
