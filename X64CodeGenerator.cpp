#include "stdafx.h"

#include "Assert.h"
#include "X64CodeGenerator.h"


namespace NativeJIT
{
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


    void X64CodeGenerator::Jz(Label l)
    {
        Indent();
        m_out << "jz L" << l.GetId() << std::endl;
    }


    void X64CodeGenerator::Nop()
    {
        Indent();
        m_out << "nop" << std::endl;
    }


    // op
    void X64CodeGenerator::Op(char const* op)
    {
        Indent();
        m_out << op << std::endl;
    }


    //*************************************************************************
    //
    // 1-byte RXX operations.
    //
    //*************************************************************************

    // dest <== dest op src
    void X64CodeGenerator::Op(char const* op, Register<1, false> dest, Register<1, false> src)
    {
        Indent();
        m_out << op << " " << dest.GetName() << ", " << src.GetName() << std::endl;
    }


    // dest <== dest op value
    void X64CodeGenerator::Op(char const* op, Register<1, false> dest, unsigned __int64 value)
    {
        Indent();
        m_out << op << " " << dest.GetName() << ", " << value << std::endl;
    }


    // dest <== dest op [base + offset]
    void X64CodeGenerator::Op(char const* op, Register<1, false> dest, Register<8, false> base, unsigned __int64 offset)
    {
        Indent();
        if (offset == 0)
        {
            m_out << op << " " << dest.GetName() << ", [" << base.GetName() << "]" << std::endl;
        }
        else
        {
            m_out << op << " " << dest.GetName() << ", [" << base.GetName() << " + " << offset << "]" << std::endl;
        }
    }


    // dest <== op dest
    void X64CodeGenerator::Op(char const* op, Register<1, false> dest)
    {
        Indent();
        m_out << op << " " << dest.GetName() << std::endl;
    }


    //*************************************************************************
    //
    // 8-byte RXX operations.
    //
    //*************************************************************************

    // dest <== dest op src
    void X64CodeGenerator::Op(char const* op, Register<8, false> dest, Register<8, false> src)
    {
        Indent();
        m_out << op << " " << dest.GetName() << ", " << src.GetName() << std::endl;
    }


    // dest <== dest op value
    void X64CodeGenerator::Op(char const* op, Register<8, false> dest, unsigned __int64 value)
    {
        Indent();
        m_out << op << " " << dest.GetName() << ", " << value << std::endl;
    }


    // dest <== dest op [base + offset]
    void X64CodeGenerator::Op(char const* op, Register<8, false> dest, Register<8, false> base, unsigned __int64 offset)
    {
        Indent();
        if (offset == 0)
        {
            m_out << op << " " << dest.GetName() << ", [" << base.GetName() << "]" << std::endl;
        }
        else
        {
            m_out << op << " " << dest.GetName() << ", [" << base.GetName() << " + " << offset << "]" << std::endl;
        }
    }


    // dest <== op dest
    void X64CodeGenerator::Op(char const* op, Register<8, false> dest)
    {
        Indent();
        m_out << op << " " << dest.GetName() << std::endl;
    }




    //*************************************************************************
    //
    // 8-byte XMM operations.
    //
    //*************************************************************************

    // dest <== dest op src
    void X64CodeGenerator::Op(char const* op, Register<8, true> dest, Register<8, true> src)
    {
        Indent();
        m_out << op << " " << dest.GetName() << ", " << src.GetName() << std::endl;
    }


    // dest <== dest op value
    void X64CodeGenerator::Op(char const* op, Register<8, true> dest, double value)
    {
        Indent();
        m_out << op << " " << dest.GetName() << ", " << value << std::endl;
    }


    // dest <== dest op [base + offset]
    void X64CodeGenerator::Op(char const* op, Register<8, true> dest, Register<8, false> base, unsigned __int64 offset)
    {
        Indent();
        if (offset == 0)
        {
            m_out << op << " " << dest.GetName() << ", [" << base.GetName() << "]" << std::endl;
        }
        else
        {
            m_out << op << " " << dest.GetName() << ", [" << base.GetName() << " + " << offset << "]" << std::endl;
        }
    }


    // dest <== op dest
    void X64CodeGenerator::Op(char const* op, Register<8, true> dest)
    {
        Indent();
        m_out << op << " " << dest.GetName() << std::endl;
    }


    void X64CodeGenerator::Indent()
    {
        m_out << "    ";
    }
}
