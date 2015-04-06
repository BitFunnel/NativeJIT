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
          m_diagnosticsStream(&std::cout)
    {
    }


    void X64CodeGenerator::EnableDiagnostics(std::ostream& out)
    {
        m_diagnosticsStream = &out;
    }


    void X64CodeGenerator::DisableDiagnostics()
    {
        m_diagnosticsStream = nullptr;
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
        CodePrinter printer(*this);

        CodeBuffer::PlaceLabel(l);
        printer.PlaceLabel(l);
    }


    void X64CodeGenerator::Jmp(Label label)
    {
        CodePrinter printer(*this);

        Emit8(0xe9);
        EmitCallSite(label, 4);

        printer.PrintJump(label);
    }


    void X64CodeGenerator::Jmp(void* functionPtr)
    {
        CodePrinter printer(*this);

        Emit8(0xe9);
        Emit64(reinterpret_cast<unsigned __int64>(functionPtr));

        printer.PrintJump(functionPtr);
    }


    char const * X64CodeGenerator::OpCodeName(OpCode op)
    {
        static char const * names[] = {
            "add",
            "and",
            "call",
            "cmp",
            "cvtfp2fp",
            "cvtfp2si",
            "cvtsi2fp",
            "imul",
            "lea",
            "mov",
            "movzx",
            "nop",
            "or",
            "pop",
            "push",
            "ret",
            "rol",
            "sal",
            "shld",
            "shr",
            "sub",
        };

        static_assert(static_cast<unsigned>(OpCode::OpCodeCount) == _countof(names),
                      "Mismatched number of opcode names.");
        Assert(static_cast<unsigned>(op)  < _countof(names), "Invalid OpCode");

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

        static_assert(static_cast<unsigned>(JccType::JccCount) == _countof(names),
                      "Mismatched number of JCC names.");
        Assert(static_cast<unsigned>(jcc)  < _countof(names), "Invalid JCC");

        return names[static_cast<unsigned>(jcc)];
    }


    std::ostream* X64CodeGenerator::GetDiagnosticsStream() const
    {
        return m_diagnosticsStream;
    }


    void X64CodeGenerator::Call(Register<8, false> r)
    {
        // EmitRex() would set REX.W, but this instruction defaults to
        // 64-bit operands in 64-bit mode and doesn't need it.
        if (r.IsExtended())
        {
            Emit8(0x41);
        }
        Emit8(0xff);
        Emit8(0xD0 | r.GetId8());
    }


    void X64CodeGenerator::Pop(Register<8, false> r)
    {
        // EmitRex() would set REX.W, but this instruction defaults to
        // 64-bit operands in 64-bit mode and doesn't need it.
        if (r.IsExtended())
        {
            Emit8(0x41);
        }
        Emit8(0x58 + r.GetId8());
    }


    void X64CodeGenerator::Push(Register<8, false> r)
    {
        // EmitRex() would set REX.W, but this instruction defaults to
        // 64-bit operands in 64-bit mode and doesn't need it.
        if (r.IsExtended())
        {
            Emit8(0x41);
        }
        Emit8(0x50 + r.GetId8());
    }


    void X64CodeGenerator::Ret()
    {
        Emit8(0xc3);
    }


    //*************************************************************************
    //
    // X64CodeGenerator::Helper<Op> methods.
    //
    //*************************************************************************
    template <>
    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::Call>::ArgTypes1<false>::Emit(X64CodeGenerator& code, Register<8, false> dest)
    {
        code.Call(dest);
    }


    template <>
    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::Pop>::ArgTypes1<false>::Emit(X64CodeGenerator& code, Register<8, false> dest)
    {
        code.Pop(dest);
    }


    template <>
    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::Push>::ArgTypes1<false>::Emit(X64CodeGenerator& code, Register<8, false> dest)
    {
        code.Push(dest);
    }


    void X64CodeGenerator::Helper<OpCode::Ret>::Emit(X64CodeGenerator& code)
    {
        code.Ret();
    }


    template <>
    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::MovZX>::ArgTypes2<false, false>::Emit(
        X64CodeGenerator& code,
        Register<4, false> dest,
        Register<4, false> src)
    {
        Helper<OpCode::Mov>::ArgTypes1<false>::Emit<4>(code, dest, src);
    }

    // Full specialization of MovZX opcode for "movzx dest64, src32" which is
    // implemented in terms of "mov dest32, src32".

    template <>
    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::MovZX>::ArgTypes2<false, false>::Emit(
        X64CodeGenerator& code,
        Register<8, false> dest,
        Register<4, false> src)
    {
        const Register<4, false> dest4(dest);

        Helper<OpCode::Mov>::ArgTypes1<false>::Emit<4>(code, dest4, src);
    }


    template <>
    template <>
    template <>
    void X64CodeGenerator::Helper<OpCode::MovZX>::ArgTypes2<false, false>::Emit<8, 4>(
        X64CodeGenerator& code,
        Register<8, false> dest,
        Register<8, false> src,
        __int32 srcOffset)
    {
        const Register<4, false> dest4(dest);

        Helper<OpCode::Mov>::ArgTypes1<false>::Emit<4>(code, dest4, src, srcOffset);
    }



    //*************************************************************************
    //
    // Helper code printing methods.
    //
    //*************************************************************************

    X64CodeGenerator::CodePrinter::CodePrinter(X64CodeGenerator& code)
        : m_code(code),
          m_out(code.GetDiagnosticsStream())
    {
        NoteStartPosition();
    }


    void X64CodeGenerator::CodePrinter::NoteStartPosition()
    {
        m_startPosition = m_code.CurrentPosition();
    }


    void X64CodeGenerator::CodePrinter::PlaceLabel(Label label)
    {
        if (m_out != nullptr)
        {
            *m_out << "L" << label.GetId() << ":" << std::endl;
        }
    }


    void X64CodeGenerator::CodePrinter::PrintJump(Label label)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << "jmp L" << label.GetId() << std::endl;
        }
    }


    void X64CodeGenerator::CodePrinter::PrintJump(void* function)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << "jmp " << std::hex << function << 'h' << std::endl;
        }
    }


    void X64CodeGenerator::CodePrinter::Print(OpCode op)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << OpCodeName(op) << std::endl;
        }
    }


    const unsigned c_asmDataWidth = 36;

    void X64CodeGenerator::CodePrinter::PrintBytes(unsigned start, unsigned end)
    {
        // TODO: Support for printing '/' after REX prefixes?
        // Alternative is unit test function can strip out white space and '/'.
        unsigned __int8* startPtr = m_code.BufferStart() + start;
        unsigned __int8* endPtr = m_code.BufferStart() + end;

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
}
