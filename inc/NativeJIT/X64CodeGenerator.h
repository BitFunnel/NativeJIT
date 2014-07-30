#pragma once

#include <ostream>

#include "NativeJIT/CodeBuffer.h"       // Inherits from CodeBuffer.
#include "NativeJIT/Register.h"         // Register parameter.
#include "Temporary/NonCopyable.h"      // Inherits from NonCopyable.


namespace NativeJIT
{
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


    enum class OpCode : unsigned
    {
        Add = 0,
        Call,
        Cmp,
        Mov,
        Mul,
        Nop,
        Or,
        Pop,
        Push,
        Ret,
        Sub,
    };


    class X64CodeGenerator : public CodeBuffer, public NonCopyable
    {
    public:
        X64CodeGenerator(std::ostream& out);

        X64CodeGenerator(std::ostream& out,
                         unsigned __int8* buffer,
                         unsigned capacity,
                         unsigned maxLabels,
                         unsigned maxCallSites);

        unsigned GetRXXCount() const;
        unsigned GetXMMCount() const;

        Label AllocateLabel();
        void PlaceLabel(Label l);


        void Jmp(Label l);
        void Jcc(JccType type, Label l);

        // TODO: Should this be generalized?
        template <unsigned SIZE, bool ISFLOAT>
        void Mov(Register<sizeof(void*), false>  base, size_t offset, Register<SIZE, ISFLOAT> src);

        //
        // Zero parameters.
        //

        void Nop();
        void Op(OpCode op);

        //
        // One parameter.
        //

        template <unsigned SIZE, bool ISFLOAT>
        void Op(OpCode op, Register<SIZE, ISFLOAT> dest);

        //
        // Two parameters.
        //

        template <unsigned SIZE, bool ISFLOAT>
        void Op(OpCode op, Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src);

        template <unsigned SIZE, bool ISFLOAT, typename T>
        void Op(OpCode op, Register<SIZE, ISFLOAT> dest, T value);

        template <unsigned SIZE, bool ISFLOAT>
        void Op(OpCode op, Register<SIZE, ISFLOAT> dest, Register<sizeof(void*), false> base, size_t offset);

        static char const * OpCodeName(OpCode op);
        static char const * JccName(JccType jcc);

    private:
        void Indent();

        static const unsigned c_rxxRegisterCount = 16;
        static const unsigned c_xmmRegisterCount = 16;

        unsigned m_labelCount;

        std::ostream& m_out;
    };


    //*************************************************************************
    //
    // Template definitions for X64CodeGenerator
    //
    //*************************************************************************
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Mov(Register<sizeof(void*), false>  base, size_t offset, Register<SIZE, ISFLOAT> src)
    {
        Indent();

        if (offset > 0)
        {
            m_out << "mov [" << base.GetName() << " + " << offset << "], " << src.GetName() << std::endl;
        }
        else
        {
            m_out << "mov [" << base.GetName() << "], " << src.GetName() << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Op(OpCode op, Register<SIZE, ISFLOAT> dest)
    {
        Indent();
        m_out << OpCodeName(op) << " " << dest.GetName() << std::endl;
    }



    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Op(OpCode op, Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src)
    {
        Indent();
        m_out << OpCodeName(op) << " " << dest.GetName() << ", " << src.GetName() << std::endl;
    }


    template <unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::Op(OpCode op, Register<SIZE, ISFLOAT> dest, T value)
    {
        Indent();
#pragma warning(push)
#pragma warning(disable:4127)
        if (ISFLOAT)
#pragma warning(pop)
        {
            m_out << OpCodeName(op) << " " << dest.GetName() << ", " << value << std::endl;
        }
        else
        {
            // TODO: HACK: Cast to unsigned __int64 to force character types to display as integers.
            // TODO: Need to add support for ISSIGNED.
            m_out << OpCodeName(op) << " " << dest.GetName() << ", " << (unsigned __int64)value << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Op(OpCode op,
                              Register<SIZE, ISFLOAT> dest,
                              Register<sizeof(void*), false> base,
                              size_t offset)
    {
        Indent();
        if (offset > 0)
        {
            m_out << OpCodeName(op) << " " << dest.GetName() << ", [" << base.GetName() << " + " << offset << "]" << std::endl;
        }
        else
        {
            m_out << OpCodeName(op) << " " << dest.GetName() << ", [" << base.GetName() << "]" << std::endl;
        }
    }
}
