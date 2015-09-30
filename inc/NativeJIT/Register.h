#pragma once

#include "Temporary/Assert.h"


namespace NativeJIT
{
    class RegisterBase
    {
    public:
        static const unsigned c_maxSize = 8;
        static const unsigned c_maxIntegerRegisterID = 16;
        static const unsigned c_maxFloatRegisterID = 15;

    protected:
        static const unsigned c_maxRegisterID = 16;

        static const unsigned c_validSizesCount = 4;
        static const unsigned c_typesCount = 2;

        // Add 1 to adjust for the fact that array is zero based, unlike size.
        static unsigned c_sizes[c_maxSize + 1];

        static char const * c_names[c_typesCount][c_validSizesCount][c_maxRegisterID + 1];
    };


    template <unsigned SIZE, bool ISFLOAT>
    class Register : public RegisterBase
    {
    public:
        static_assert((ISFLOAT == 0 && (SIZE == 1 || SIZE == 2 || SIZE == 4 || SIZE == 8))
                      || (ISFLOAT == 1 && (SIZE == 4 || SIZE == 8)),
                      "Invalid register definition.");

        typedef Register<c_maxSize, ISFLOAT> FullRegister;

        // Templates that don't explicitly receive SIZE and ISFLOAT but rather
        // have access to a Register need to have a way to access the size and
        // type, so provide them here.
        static const unsigned c_size = SIZE;
        static const bool c_isFloat = ISFLOAT;

        Register()
            : m_id(0)
        {
        }


        explicit Register(unsigned id)
            : m_id(id)
        {
            Assert((!ISFLOAT && id <= c_maxIntegerRegisterID)
                       || (ISFLOAT && id <= c_maxFloatRegisterID),
                   "Invalid register id.");
        }


        template <unsigned SIZE>
        explicit Register(Register<SIZE, false> r)
            : m_id(r.GetId())
        {
        }


        unsigned GetId() const
        {
            return m_id;
        }


        // Returns the lower three bits of the ID. Equivalent to GetId() for
        // non-extended registers.
        unsigned __int8 GetId8() const
        {
            return m_id & 7;
        }


        unsigned GetMask() const
        {
            return 1 << m_id;
        }


        // Returns whether the register belongs to the set of extended registers
        // in the long mode (i.e. r8, xmm8 etc).
        bool IsExtended() const
        {
            return m_id > 7;
        }


        char const * GetName() const
        {
            char const * name = c_names[ISFLOAT ? 1 : 0][c_sizes[SIZE]][m_id];
            Assert(name != nullptr, "Attempting to get name for invalid register.");

            return name;
        }


        bool IsRIP() const
        {
            return IsSameHardwareRegister(rip);
        }


        bool IsStackPointer() const
        {
            return IsSameHardwareRegister(rsp);
        }


        // Returns whether the registers are exactly the same (size, type and ID).
        // This will return false for comparison between f. ex. rax and eax.
        template <unsigned SIZE2, bool ISFLOAT2>
        bool operator==(Register<SIZE2, ISFLOAT2> other) const
        {
            return SIZE2 == SIZE
                   && ISFLOAT2 == ISFLOAT
                   && other.GetId() == GetId();
        }


        // Returns whether the two registers have the same type and ID.
        // This will return true for comparison between f. ex. rax and eax but
        // false for comparison between rax and xmm0 (even though they have the
        // same register ID).
        template <unsigned SIZE2, bool ISFLOAT2>
        bool IsSameHardwareRegister(Register<SIZE2, ISFLOAT2> other) const
        {
            return ISFLOAT2 == ISFLOAT
                && other.GetId() == GetId();
        }


    protected:
        unsigned m_id;
    };


    typedef Register<sizeof(void*), false> PointerRegister;

    // TODO: Need to avoid "static initialization order fiasco" for register definitions.
    // See http://www.parashift.com/c++-faq/static-init-order.html.
    // Plan is to use constexpr when VS2013 becomes available to Bing build.

    extern Register<1, false> al;
    extern Register<1, false> cl;
    extern Register<1, false> dl;
    extern Register<1, false> bl;
    extern Register<1, false> spl;
    extern Register<1, false> bpl;
    extern Register<1, false> sil;
    extern Register<1, false> dil;
    extern Register<1, false> r8b;
    extern Register<1, false> r9b;
    extern Register<1, false> r10b;
    extern Register<1, false> r11b;
    extern Register<1, false> r12b;
    extern Register<1, false> r13b;
    extern Register<1, false> r14b;
    extern Register<1, false> r15b;

    extern Register<2, false> ax;
    extern Register<2, false> cx;
    extern Register<2, false> dx;
    extern Register<2, false> bx;
    extern Register<2, false> sp;
    extern Register<2, false> bp;
    extern Register<2, false> si;
    extern Register<2, false> di;
    extern Register<2, false> r8w;
    extern Register<2, false> r9w;
    extern Register<2, false> r10w;
    extern Register<2, false> r11w;
    extern Register<2, false> r12w;
    extern Register<2, false> r13w;
    extern Register<2, false> r14w;
    extern Register<2, false> r15w;

    extern Register<4, false> eax;
    extern Register<4, false> ecx;
    extern Register<4, false> edx;
    extern Register<4, false> ebx;
    extern Register<4, false> esp;
    extern Register<4, false> ebp;
    extern Register<4, false> esi;
    extern Register<4, false> edi;
    extern Register<4, false> r8d;
    extern Register<4, false> r9d;
    extern Register<4, false> r10d;
    extern Register<4, false> r11d;
    extern Register<4, false> r12d;
    extern Register<4, false> r13d;
    extern Register<4, false> r14d;
    extern Register<4, false> r15d;

    extern Register<8, false> rax;
    extern Register<8, false> rcx;
    extern Register<8, false> rdx;
    extern Register<8, false> rbx;
    extern Register<8, false> rsp;
    extern Register<8, false> rbp;
    extern Register<8, false> rsi;
    extern Register<8, false> rdi;
    extern Register<8, false> r8;
    extern Register<8, false> r9;
    extern Register<8, false> r10;
    extern Register<8, false> r11;
    extern Register<8, false> r12;
    extern Register<8, false> r13;
    extern Register<8, false> r14;
    extern Register<8, false> r15;
    extern Register<8, false> rip;

    extern Register<4, true> xmm0s;
    extern Register<4, true> xmm1s;
    extern Register<4, true> xmm2s;
    extern Register<4, true> xmm3s;
    extern Register<4, true> xmm4s;
    extern Register<4, true> xmm5s;
    extern Register<4, true> xmm6s;
    extern Register<4, true> xmm7s;
    extern Register<4, true> xmm8s;
    extern Register<4, true> xmm9s;
    extern Register<4, true> xmm10s;
    extern Register<4, true> xmm11s;
    extern Register<4, true> xmm12s;
    extern Register<4, true> xmm13s;
    extern Register<4, true> xmm14s;
    extern Register<4, true> xmm15s;

    extern Register<8, true> xmm0;
    extern Register<8, true> xmm1;
    extern Register<8, true> xmm2;
    extern Register<8, true> xmm3;
    extern Register<8, true> xmm4;
    extern Register<8, true> xmm5;
    extern Register<8, true> xmm6;
    extern Register<8, true> xmm7;
    extern Register<8, true> xmm8;
    extern Register<8, true> xmm9;
    extern Register<8, true> xmm10;
    extern Register<8, true> xmm11;
    extern Register<8, true> xmm12;
    extern Register<8, true> xmm13;
    extern Register<8, true> xmm14;
    extern Register<8, true> xmm15;
}
