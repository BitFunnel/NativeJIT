#include "stdafx.h"

#include <iostream>         // TODO: Remove - temporary for debugging.

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
    X64CodeGenerator::X64CodeGenerator(Allocators::IAllocator& allocator,
                                       unsigned capacity,
                                       unsigned maxLabels,
                                       unsigned maxCallSites)
        : CodeBuffer(allocator, capacity, maxLabels, maxCallSites),
          // TODO: Handle stream correctly
          m_out(&std::cout)
    {
    }


    void X64CodeGenerator::EnableDiagnostics(std::ostream& out)
    {
        m_out = &out;
    }


    void X64CodeGenerator::DisableDiagnostics()
    {
        m_out = nullptr;
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
        *m_out << "L" << l.GetId() << ":" << std::endl;
    }


    void X64CodeGenerator::Jmp(Label label)
    {
        unsigned start = CurrentPosition();

        Emit8(0xe9);
        EmitCallSite(label, 4);

        if (m_out != nullptr)
        {
            PrintBytes(start, CurrentPosition());
            *m_out << "jmp L" << label.GetId() << std::endl;
        }
    }


    void X64CodeGenerator::Jmp(void* functionPtr)
    {
        Emit8(0xe9);
        Emit64(reinterpret_cast<unsigned __int64>(functionPtr));
    }


    char const * X64CodeGenerator::OpCodeName(OpCode op)
    {
        static char const * names[] = {
            "add",
            "and",
            "call",
            "cmp",
            "imul",
            "lea",
            "mov",
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
            "jo",
            "jno", 
            "jb",
            "jae",
            "je",
            "jne",
            "jbe",
            "ja",
            "js",
            "jns",
            "jp",
            "jnp",
            "jl",
            "jnl",
            "jle",
            "jnle"
        };

        Assert(static_cast<unsigned>(jcc)  < sizeof(names)/sizeof(char const*), "Invalid JCC");

        return names[static_cast<unsigned>(jcc)];
    }


    void X64CodeGenerator::Indent()
    {
        *m_out << "    ";
    }

    const unsigned c_asmDataWidth = 36;

    void X64CodeGenerator::PrintBytes(unsigned start, unsigned end)
    {
        // TODO: Support for printing '/' after REX prefixes?
        // Alternative is unit test function can strip out white space and '/'.
        unsigned __int8* startPtr = BufferStart() + start;
        unsigned __int8* endPtr = BufferStart() + end;

        *m_out << " ";
        m_out->width(8);
        m_out->fill('0');
        *m_out << std::uppercase << std::hex << start << "  ";

        unsigned column = 11;
        while (startPtr < endPtr)
        {
            m_out->width(2);
            m_out->fill('0');
            *m_out << std::hex << static_cast<unsigned>(*startPtr++);
            *m_out << " ";
            column += 3;
        }

        while (column < c_asmDataWidth)
        {
            *m_out << ' ';
            column++;
        }
    }


    void X64CodeGenerator::Call(Register<8, false> r)
    {
        if (r.GetId() > 7)
        {
            Emit8(0x41);
        }
        Emit8(0xff);
        Emit8(0xD0 | r.GetId() & 0x7);
    }


    void X64CodeGenerator::Pop(Register<8, false> r)
    {
        if (r.GetId() > 7)
        {
            Emit8(0x41);  // TODO: Should be able to use EmitREX() here except it sets W.
        }
        Emit8(0x58 + (r.GetId() & 7));
    }


    void X64CodeGenerator::Push(Register<8, false> r)
    {
        if (r.GetId() > 7)
        {
            Emit8(0x41);  // TODO: Should be able to use EmitREX() here except it sets W.
        }
        Emit8(0x50 + (r.GetId() & 7));
    }


    void X64CodeGenerator::Ret()
    {
        Emit8(0xc3);
    }


    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::Call>::Emit(X64CodeGenerator& code, Register<8, false> dest)
    {
        code.Call(dest);
    }


    void X64CodeGenerator::Helper<OpCode::Ret>::Emit(X64CodeGenerator& code)
    {
        code.Ret();
    }
}
