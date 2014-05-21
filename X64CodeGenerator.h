#pragma once

#include <ostream>

#include "Register.h"


namespace NativeJIT
{
    class X64CodeGenerator
    {
    public:
        X64CodeGenerator(std::ostream& out);

        unsigned GetRXXCount() const;
        unsigned GetXMMCount() const;

        // dest <== dest op src
        void Op(char const* op, Register<8, false> dest, Register<8, false> src);

        // dest <== dest op value
        void Op(char const* op, Register<8, false> dest, unsigned __int64 value);

        // dest <== dest op [base + offset]
        void Op(char const* op, Register<8, false> dest, Register<8, false> base, unsigned __int64 offset);

        void Op(char const* op, Register<8, false> dest);

        void Op(char const* op);

    private:
        static const unsigned c_rxxRegisterCount = 16;
        static const unsigned c_xmmRegisterCount = 16;

        std::ostream& m_out;
    };
}
