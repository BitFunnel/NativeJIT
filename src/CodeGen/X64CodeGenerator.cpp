#include "stdafx.h"

#include "Temporary/Assert.h"
#include "NativeJIT/X64CodeGenerator.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // Label
    //
    //*************************************************************************
    Label::Label()
        : m_id(0)
    {
    }


    Label::Label(unsigned id)
        : m_id(id)
    {
    }


    unsigned Label::GetId() const
    {
        return m_id;
    }


    //*************************************************************************
    //
    // X64CodeGenerator public methods
    //
    //*************************************************************************
    X64CodeGenerator::X64CodeGenerator(std::ostream& out)
        : CodeBuffer(nullptr, 0, 0, 0),
          m_out(out)
    {
    }


    X64CodeGenerator::X64CodeGenerator(std::ostream& out,
                                       unsigned __int8* buffer,
                                       unsigned capacity,
                                       unsigned maxLabels,
                                       unsigned maxCallSites)
        : CodeBuffer(buffer, capacity, maxLabels, maxCallSites),
          m_out(out)
    {
    }


    unsigned X64CodeGenerator::GetRXXCount() const
    {
        return c_rxxRegisterCount;
    }


    unsigned X64CodeGenerator::GetXMMCount() const
    {
        return c_xmmRegisterCount;
    }


    // TODO: Remove this temporary overload of CodeBuffer::PlaceLabel.
    void X64CodeGenerator::PlaceLabel(Label l)
    {
        CodeBuffer::PlaceLabel(l);
        m_out << "L" << l.GetId() << ":" << std::endl;
    }


    void X64CodeGenerator::Jmp(Label l)
    {
        Indent();
        m_out << "jmp L" << l.GetId() << std::endl;
    }


    void X64CodeGenerator::Jcc(JccType jcc, Label l)
    {
        Indent();
        m_out << JccName(jcc) << " L" << l.GetId() << std::endl;
    }


    void X64CodeGenerator::Nop()
    {
        Indent();
        m_out << "nop" << std::endl;
    }


    void X64CodeGenerator::Op(OpCode op)
    {
        Indent();
        m_out << OpCodeName(op) << std::endl;
    }


    char const * X64CodeGenerator::OpCodeName(OpCode op)
    {
        static char const * names[] = {
            "add",
            "call",
            "cmp",
            "lea",
            "mov",
            "mul",
            "nop",
            "or",
            "pop",
            "push",
            "ret",
            "sub",
        };

        Assert(static_cast<unsigned>(op)  < sizeof(names)/sizeof(char const*), "Invalid OpCode");

        return names[static_cast<unsigned>(op)];
    }


    char const * X64CodeGenerator::JccName(JccType jcc)
    {
        static char const * names[] = {
            "ja", "jna",
            "jb", "jnb",
            "jg", "jng",
            "jl", "jnl",
            "jz", "jnz"
        };

        Assert(static_cast<unsigned>(jcc)  < sizeof(names)/sizeof(char const*), "Invalid JCC");

        return names[static_cast<unsigned>(jcc)];
    }


    //*************************************************************************
    //
    // X64CodeGenerator private methods
    //
    //*************************************************************************
    void X64CodeGenerator::Indent()
    {
        m_out << "    ";
    }
}
