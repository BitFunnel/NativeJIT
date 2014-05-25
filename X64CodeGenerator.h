#pragma once

#include <ostream>

#include "Assert.h"
#include "Register.h"


namespace NativeJIT
{
    class Label
    {
    public:
        Label(unsigned id);

        unsigned GetId() const;

    private:
        unsigned m_id;
    };


    // WARNING: When modifying JccType, be sure to also modify the function JccName().
    enum class JccType : unsigned
    {
        JA,
        JNA,
        JB,
        JNB,
        JG,
        JNG,
        JL,
        JNL,
        JZ,
        JNZ
    };

    char const * JccName(JccType jcc);

    class X64CodeGenerator
    {
    public:
        X64CodeGenerator(std::ostream& out);

        unsigned GetRXXCount() const;
        unsigned GetXMMCount() const;

        Label AllocateLabel();
        void PlaceLabel(Label l);


        void Jmp(Label l);
        void Jcc(JccType type, Label l);
//        void Jz(Label l);

        void Nop();


        void Op(char const* op);

        //
        // 8-byte RXX operations.
        //

        // dest <== dest op src
        void Op(char const* op, Register<1, false> dest, Register<1, false> src);

        // dest <== dest op value
        void Op(char const* op, Register<1, false> dest, unsigned __int64 value);

        // dest <== dest op [base + offset]
        void Op(char const* op, Register<1, false> dest, Register<8, false> base, unsigned __int64 offset);

        void Op(char const* op, Register<1, false> dest);


        // dest <== dest op src
        void Op(char const* op, Register<8, false> dest, Register<8, false> src);

        // dest <== dest op value
        void Op(char const* op, Register<8, false> dest, unsigned __int64 value);

        // dest <== dest op [base + offset]
        void Op(char const* op, Register<8, false> dest, Register<8, false> base, unsigned __int64 offset);

        void Op(char const* op, Register<8, false> dest);



        //
        // 8-byte XMM floating point operations.
        //

        // dest <== dest op src
        void Op(char const* op, Register<8, true> dest, Register<8, true> src);

        // dest <== dest op value
        void Op(char const* op, Register<8, true> dest, double value);

        // dest <== dest op [base + offset]
        void Op(char const* op, Register<8, true> dest, Register<8, false> base, unsigned __int64 offset);

        void Op(char const* op, Register<8, true> dest);


    private:
        void Indent();

        static const unsigned c_rxxRegisterCount = 16;
        static const unsigned c_xmmRegisterCount = 16;

        unsigned m_labelCount;

        std::ostream& m_out;
    };
}
