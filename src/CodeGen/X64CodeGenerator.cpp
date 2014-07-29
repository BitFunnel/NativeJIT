#include "stdafx.h"

#include "Temporary/Assert.h"
#include "NativeJIT/X64CodeGenerator.h"


namespace NativeJIT
{
    char const * JccName(JccType jcc)
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
    // Label
    //
    //*************************************************************************
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
    // X64CodeGenerator
    //
    //*************************************************************************
    X64CodeGenerator::X64CodeGenerator(std::ostream& out)
        : m_out(out),
          m_labelCount(0)
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


    Label X64CodeGenerator::AllocateLabel()
    {
        return Label(m_labelCount++);
    }


    void X64CodeGenerator::PlaceLabel(Label l)
    {
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


    // Op
    void X64CodeGenerator::Op(char const* op)
    {
        Indent();
        m_out << op << std::endl;
    }


    void X64CodeGenerator::Indent()
    {
        m_out << "    ";
    }
}
