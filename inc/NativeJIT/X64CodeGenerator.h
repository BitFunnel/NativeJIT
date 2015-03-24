#pragma once

// TODO: No way to do an operation on sign-extended data. Need to load data first, then sign-extend.


// http://wiki.osdev.org/X86-64_Instruction_Encoding
// http://ref.x86asm.net/coder64.html

#include <ostream>

#include "NativeJIT/CodeBuffer.h"       // Inherits from CodeBuffer.
#include "NativeJIT/Register.h"         // Register parameter.
#include "NativeJIT/ValuePredicates.h"  // Called by template code.
#include "Temporary/NonCopyable.h"      // Inherits from NonCopyable.


// Supress warning about constant expression involving template parameters.
#pragma warning(push)
#pragma warning(disable:4127)


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    // WARNING: When modifying JccType, be sure to also modify the function JccName().
    enum class JccType : unsigned
    {
        JO = 0,
        JNO = 1,
        JB = 2,
        JAE = 3, JNB = 3, JNC = 3,
        JE = 4, JZ = 4,
        JNE = 5, JNZ = 5,
        JBE = 6, JNA = 6,
        JA = 7, JNBE = 7,
        JS = 8,
        JNS = 9,
        JP = 0xa, JPE = 0xa,
        JNP = 0xb, JPO = 0xb,
        JL = 0xc, JNGE = 0xc,
        JNL = 0xd, JGE = 0xd,
        JLE = 0xe, JNG = 0xe,
        JNLE = 0xf, JG = 0xf,
        // The following value must be the last one.
        JccCount
    };


    // WARNING: When modifying OpCode, be sure to also modify the function OpCodeName().
    enum class OpCode : unsigned
    {
        Add,
        And,
        Call,
        Cmp,
        CvtFP2FP,
        CvtFP2SI,
        CvtSI2FP,
        IMul,       // TODO: Consider calling this Mul. These opcodes are not X64 opcodes.
        Lea,
        Mov,
        MovZX,
        Nop,
        Or,
        Pop,
        Push,
        Ret,
        Rol,
        Sal,
        Shld,
        Shr,
        Sub,
        // The following value must be the last one.
        OpCodeCount
    };


    class X64CodeGenerator : public CodeBuffer
    {
    public:
        X64CodeGenerator(Allocators::IAllocator& allocator,
                         unsigned capacity,
                         unsigned maxLabels,
                         unsigned maxCallSites);

        void EnableDiagnostics(std::ostream& out);
        void DisableDiagnostics();

        unsigned GetRXXCount() const;
        unsigned GetXMMCount() const;

        // TODO: Remove this temporary override of CodeBuffer::PlaceLabel().
        // This version is used to print debugging information.
        void PlaceLabel(Label l);

        void Jmp(Label l);
        void Jmp(void* functionPtr);

        // These two methods are public in order to allow access for BinaryNode debugging text.
        static char const * OpCodeName(OpCode op);
        static char const * JccName(JccType jcc);

        //
        // X64 opcode emit methods.
        //

        template <JccType JCC>
        void Emit(Label l);

        // No operand (e.g nop, ret)
        template <OpCode OP>
        void Emit();

        // One register operand (e.g. call, neg, not, push, pop)
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest);

        // Two register operands with the same type and size (e.g. and, mov, or, sub).
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src);

        // Two register operands with different type and/or size (f. ex. movzx).
        template <OpCode OP, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void Emit(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

        // Two operands - register destination and indirect source with the same
        // type and size.
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest, Register<8, false> src, __int32 srcOffset);

        // Two operands - register destination and indirect source. This flavor
        // of the method is used for instructions which can read values of a
        // varying size from the source address. F. ex. movzx with 32-bit
        // target destination can read either a byte or a word from the indirect
        // source.
        template <OpCode OP, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void Emit(Register<SIZE1, ISFLOAT1> dest, Register<8, false> src, __int32 srcOffset);

        // Two operands - indirect destination and register source with the same type and size.
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<8, false> dest, __int32 destOffset, Register<SIZE, ISFLOAT> src);

        // Two operands - indirect destination and register source. This flavor
        // of the method is used for instructions which can write values of a
        // varying size to the destination address.
        template <OpCode OP, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void Emit(Register<8, false> dest, __int32 destOffset, Register<SIZE2, ISFLOAT2> src);

        // Two operands - register destination and immediate source.
        // Note: Method is named EmitImmediate() to avoid clashes with other
        // Emit() methods in case when T gets resolved to f. ex. Register.
        template <OpCode OP, unsigned SIZE, bool ISFLOAT, typename T>
        void EmitImmediate(Register<SIZE, ISFLOAT> dest, T value);


    private:
        void Call(Register<8, false> r);

        template <unsigned SIZE>
        void IMul(Register<SIZE, false> dest,
                  Register<SIZE, false> src);

        template <unsigned SIZE>
        void IMul(Register<SIZE, false> dest,
                  Register<8, false> src,
                  __int32 srcOffset);

        // TODO: Would like some sort of compiletime error when T is quadword or floating point
        template <unsigned SIZE, typename T>
        void IMulImmediate(Register<SIZE, false> dest,
                           T value);

        template <unsigned SIZE>
        void Lea(Register<SIZE, false> dest,
                 Register<8, false> src,
                 __int32 srcOffset);

        template <unsigned SIZE, typename T>
        void MovImmediate(Register<SIZE, false> dest,
                          T value);

        template <typename T>
        void MovImmediate(Register<8, false> dest,
                          T* value);

        void Pop(Register<8, false> r);
        void Push(Register<8, false> r);

        void Ret();

        template <unsigned SIZE>
        void MovD(Register<SIZE, true> dest, Register<SIZE, false> src);

        template <unsigned SIZE1, unsigned SIZE2>
        void MovZX(Register<SIZE1, false> dest, Register<SIZE2, false> src);

        template <unsigned SIZE1, unsigned SIZE2>
        void MovZX(Register<SIZE1, false> dest, Register<8, false> src, __int32 srcOffset);

        template <unsigned SIZE>
        void Shld(Register<SIZE, false> dest, Register<SIZE, false> src);

        // Scalar SSE instructions are encoded as XX 0F OPCODE, where XX is
        // either 0xF2 or 0xF3 depending on the register size. Used for
        // instructions operating on scalars (f. ex. MovSS/SD, AddSS/SD) rather
        // than packed (f. ex. MOVUPS/MOVUPD).

        // Generic method for emitting 0xF2 or 0xF3 prefix depending on the
        // register type and size.
        template <unsigned RMSIZE, bool RMISFLOAT,
                  unsigned REGSIZE, bool REGISFLOAT,
                  unsigned RMREGSIZE, bool RMREGISFLOAT>
        void EmitScalarSSEPrefix(Register<REGSIZE, REGISFLOAT> reg, Register<RMREGSIZE, RMREGISFLOAT> rm);

        // Two direct registers.
        template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void EmitScalarSSEPrefixDirect(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

        // Two registers, the one corresponding to R/M is indirect.
        template <unsigned RMSIZE, bool RMISFLOAT, unsigned REGSIZE, bool REGISFLOAT>
        void EmitScalarSSEPrefixIndirect(Register<REGSIZE, REGISFLOAT> reg, Register<8, false> rm);

        // Variants for emitting scalar SSE instructions.

        template <unsigned __int8 OPCODE, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void ScalarSSE(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

        template <unsigned __int8 OPCODE, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void ScalarSSE(Register<SIZE1, ISFLOAT1> dest, Register<8, false> src, __int32 srcOffset);

        template <unsigned __int8 OPCODE, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void ScalarSSE(Register<8, false> dest, __int32 destOffset, Register<SIZE2, ISFLOAT2> src);

        // Group 1/2 instructions.

        template <unsigned SIZE>
        void Group1(unsigned __int8 baseOpCode,
                    Register<SIZE, false> dest,
                    Register<SIZE, false> src);

        template <unsigned SIZE>
        void Group1(unsigned __int8 baseOpCode,
                    Register<SIZE, false> dest,
                    Register<8, false> src,
                    __int32 srcOffset);

        template <unsigned SIZE>
        void Group1(unsigned __int8 baseOpCode,
                    Register<8, false> dest,
                    __int32 destOffset,
                    Register<SIZE, false> src);

        // TODO: Would like some sort of compiletime error when T is quadword or floating point
        template <unsigned SIZE, typename T>
        void Group1(unsigned __int8 baseOpCode,
                    unsigned __int8 extensionOpCode,
                    Register<SIZE, false> dest,
                    T value);

        template <unsigned SIZE>
        void Group2(unsigned __int8 extensionOpCode,
                    Register<SIZE, false> dest);

        template <unsigned SIZE>
        void Group2(unsigned __int8 extensionOpCode,
                    unsigned __int8 shift,
                    Register<SIZE, false> dest);

        // Methods for emitting the 0x66 operand size override prefix if
        // size of either operand is 16-bit. Note: for indirect addressing, the
        // size of the operand is the size of the memory being accessed, not the
        // size of the indirect base register.
        // Reference: http://wiki.osdev.org/X86-64_Instruction_Encoding#Operand-size_and_address-size_override_prefix

        // Generic version. When used with indirect addressing, the first two
        // template parameters describe the target memory size and type whereas
        // the last two describe the base register. For regular addressing,
        // these two pairs of template parameters have to be the same.
        template <unsigned RMSIZE, bool RMISFLOAT,
                  unsigned REGSIZE, bool REGISFLOAT,
                  unsigned RMREGSIZE, bool RMREGISFLOAT>
        void EmitOpSizeOverride(Register<REGSIZE, REGISFLOAT> reg,
                                Register<RMREGSIZE, RMREGISFLOAT> rm);

        // Single direct register.
        template <unsigned SIZE, bool ISFLOAT>
        void EmitOpSizeOverride(Register<SIZE, ISFLOAT> reg);

        // Two direct registers.
        template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void EmitOpSizeOverrideDirect(Register<SIZE1, ISFLOAT1> dest,
                                      Register<SIZE2, ISFLOAT2> src);

        // Single indirect register.
        template <unsigned RMSIZE, bool RMISFLOAT>
        void EmitOpSizeOverrideIndirect(Register<8, false> rm);

        // Two registers, the one corresponding to R/M is indirect.
        template <unsigned RMSIZE, bool RMISFLOAT, unsigned REGSIZE, bool REGISFLOAT>
        void EmitOpSizeOverrideIndirect(Register<REGSIZE, REGISFLOAT> reg, Register<8, false> rm);

        // Methods for emitting the REX byte. For indirect addressing, the
        // size of the operand is the size of the memory being accessed, not the
        // size of the indirect base register.
        // Reference: http://wiki.osdev.org/X86-64_Instruction_Encoding#REX_prefix

        // Generic version. See the comment for EmitOpSizeOverride() for more information.
        template <unsigned RMSIZE, bool RMISFLOAT,
                  unsigned REGSIZE, bool REGISFLOAT,
                  unsigned RMREGSIZE, bool RMREGISFLOAT>
        void EmitRex(Register<REGSIZE, REGISFLOAT> reg, Register<RMREGSIZE, RMREGISFLOAT> rm);

        // Single direct register.
        template <unsigned SIZE, bool ISFLOAT>
        void EmitRex(Register<SIZE, ISFLOAT> reg);

        // Two direct registers.
        template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void EmitRexDirect(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

        // Single indirect register.
        template <unsigned RMSIZE, bool RMISFLOAT>
        void EmitRexIndirect(Register<8, false> rm);

        // Two registers, the one corresponding to R/M is indirect.
        template <unsigned RMSIZE, bool RMISFLOAT, unsigned REGSIZE, bool REGISFLOAT>
        void EmitRexIndirect(Register<REGSIZE, REGISFLOAT> reg, Register<8, false> rm);

        // Methods for emitting the ModR/M byte.
        // Reference: http://wiki.osdev.org/X86-64_Instruction_Encoding#ModR.2FM

        template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
        void EmitModRM(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

        template <unsigned SIZE>
        void EmitModRM(unsigned __int8 extensionOpCode, Register<SIZE, false> dest);

        template <unsigned SIZE, bool ISFLOAT>
        void EmitModRMOffset(Register<SIZE, ISFLOAT> dest, Register<8, false> src, __int32 srcOffset);


        std::ostream* GetDiagnosticsStream() const;


        // Helper class used to provide partial specializations by OpCode,
        // ISFLOAT and SIZE for the Emit() methods.
        template <OpCode OP>
        struct Helper
        {
            static void Emit(X64CodeGenerator& code);

            // Emit methods where operands have the same type and size. This is
            // another layer used to provide partial specialization for the
            // Emit() methods.
            template <bool ISFLOAT>
            struct ArgTypes1
            {
                template <unsigned SIZE>
                static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest);

                template <unsigned SIZE>
                static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src);

                template <unsigned SIZE>
                static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, Register<8, false> src, __int32 srcOffset);

                template <unsigned SIZE>
                static void Emit(X64CodeGenerator& code, Register<8, false> dest, __int32 destOffset, Register<SIZE, ISFLOAT> src);

                template <unsigned SIZE, typename T>
                static void EmitImmediate(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, T value);
            };


            // Emit methods where operands don't have the same type and size.
            template <bool ISFLOAT1, bool ISFLOAT2>
            struct ArgTypes2
            {
                template <unsigned SIZE1, unsigned SIZE2>
                static void Emit(X64CodeGenerator& code, Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

                template <unsigned SIZE1, unsigned SIZE2>
                static void Emit(X64CodeGenerator& code, Register<SIZE1, ISFLOAT1> dest, Register<8, false> src, __int32 srcOffset);

                template <unsigned SIZE1, unsigned SIZE2>
                static void Emit(X64CodeGenerator& code, Register<8, false> dest, __int32 destOffset, Register<SIZE2, ISFLOAT2> src);
            };
        };


        // CodePrinter is a helper class for formatting X64CodeGenerator opcodes
        // and operands along with the resulting assembly encoding of the
        // instruction into a diagnostics stream, if diagnostics output is enabled.
        //
        // The client is expected to 1. call NoteStartPosition() before emitting
        // the instructions into the X64CodeGenerator and 2. call the relevant
        // Print() method after emitting them.
        // 
        class CodePrinter : public NonCopyable
        {
        public:
            // Calls NoteStartPosition() and stores the output diagnostics stream.
            CodePrinter(X64CodeGenerator& code);

            void NoteStartPosition();

            void PlaceLabel(Label label);

            // The Print() methods will display the last remembered starting
            // point of the code buffer, all the encoded bytes from
            // the starting point to the end of the buffer followed by the
            // X64CodeGenerator opcodes and operands.

            void PrintJump(void *function);
            void PrintJump(Label label);

            template <JccType JCC>
            void Print(Label l);

            void Print(OpCode op);

            template <unsigned SIZE, bool ISFLOAT>
            void Print(OpCode op, Register<SIZE, ISFLOAT> dest);

            template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
            void Print(OpCode op, Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src);

            template <unsigned SIZE, bool ISFLOAT>
            void Print(OpCode op, Register<SIZE, ISFLOAT> dest, Register<8, false> src, __int32 srcOffset);

            template <unsigned SIZE, bool ISFLOAT>
            void Print(OpCode op, Register<8, false> dest, __int32 destOffset, Register<SIZE, ISFLOAT> src);

            template <unsigned SIZE, bool ISFLOAT, typename T>
            void PrintImmediate(OpCode op, Register<SIZE, ISFLOAT> dest, T value);


        private:
            X64CodeGenerator& m_code;
            unsigned m_startPosition;
            std::ostream* m_out;

            void PrintBytes(unsigned startPosition, unsigned endPosition);
        };

        static const unsigned c_rxxRegisterCount = 16;
        static const unsigned c_xmmRegisterCount = 16;

        std::ostream* m_diagnosticsStream;
    };


    //*************************************************************************
    //
    // Helper code printing methods.
    //
    //*************************************************************************

    template <JccType JCC>
    void X64CodeGenerator::CodePrinter::Print(Label label)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << JccName(JCC) << " L" << label.GetId() << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::CodePrinter::Print(OpCode op, Register<SIZE, ISFLOAT> dest)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << OpCodeName(op) << ' ' << dest.GetName() << std::endl;
        }
    }


    template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::CodePrinter::Print(OpCode op,
                                              Register<SIZE1, ISFLOAT1> dest,
                                              Register<SIZE2, ISFLOAT2> src)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << OpCodeName(op) << ' ' << dest.GetName() << ", " << src.GetName() << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::CodePrinter::Print(OpCode op,
                                              Register<SIZE, ISFLOAT> dest,
                                              Register<8, false> src,
                                              __int32 srcOffset)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << OpCodeName(op) << ' ' << dest.GetName();
            *m_out << ", [" << src.GetName();

            if (srcOffset > 0)
            {
                *m_out << " + " << std::hex << srcOffset << "h";
            }
            else if (srcOffset < 0)
            {
                *m_out << " - " << std::hex << -static_cast<__int64>(srcOffset) << "h";
            }

            *m_out << "]"  << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::CodePrinter::Print(OpCode op,
                                              Register<8, false> dest,
                                              __int32 destOffset,
                                              Register<SIZE, ISFLOAT> src)
    {
        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << OpCodeName(op) << " [" << dest.GetName();

            if (destOffset > 0)
            {
                *m_out << " + " << std::hex << destOffset << "h";
            }
            else if (destOffset < 0)
            {
                *m_out << " - " << std::hex << -static_cast<__int64>(destOffset) << "h";
            }

            *m_out << "], " << src.GetName() << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::CodePrinter::PrintImmediate(OpCode op,
                                                       Register<SIZE, ISFLOAT> dest,
                                                       T value)
    {
        static_assert(!std::is_floating_point<T>::value,
                      "Floating point values cannot be used as immediates.");

        // Cast UInt8 to UInt64 to prevent it from being printed as char.
        typedef std::conditional<std::is_same<T, unsigned __int8>::value,
                                 unsigned __int64,
                                 T>::type ValueType;

        if (m_out != nullptr)
        {
            PrintBytes(m_startPosition, m_code.CurrentPosition());

            *m_out << OpCodeName(op)
                   << ' ' << dest.GetName()
                   << ", " << std::hex << static_cast<ValueType>(value) << 'h'
                   << std::endl;
        }
    }



    //*************************************************************************
    //
    // Template definitions for X64CodeGenerator - public methods.
    //
    //*************************************************************************
    template <JccType JCC>
    void X64CodeGenerator::Emit(Label label)
    {
        CodePrinter printer(*this);

        Emit8(0xf);
        Emit8(0x80 + static_cast<unsigned __int8>(JCC));
        EmitCallSite(label, 4);

        printer.Print<JCC>(label);
    }


    template <OpCode OP>
    void X64CodeGenerator::Emit()
    {
        CodePrinter printer(*this);

        Helper<OP>::Emit(*this);

        printer.Print(OP);
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes1<ISFLOAT>::Emit<SIZE>(*this, dest);

        printer.Print(OP, dest);
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes1<ISFLOAT>::Emit<SIZE>(*this, dest, src);

        printer.Print(OP, dest, src);
    }


    template <OpCode OP, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::Emit(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes2<ISFLOAT1, ISFLOAT2>::Emit<SIZE1, SIZE2>(*this, dest, src);

        printer.Print(OP, dest, src);
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest, Register<8, false> src, __int32 srcOffset)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes1<ISFLOAT>::Emit<SIZE>(*this, dest, src, srcOffset);

        printer.Print(OP, dest, src, srcOffset);
    }


    template <OpCode OP, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::Emit(Register<SIZE1, ISFLOAT1> dest, Register<8, false> src, __int32 srcOffset)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes2<ISFLOAT1, ISFLOAT2>::Emit<SIZE1, SIZE2>(*this, dest, src, srcOffset);

        printer.Print(OP, dest, src, srcOffset);
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<8, false> dest, __int32 destOffset, Register<SIZE, ISFLOAT> src)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes1<ISFLOAT>::Emit<SIZE>(*this, dest, destOffset, src);

        printer.Print(OP, dest, destOffset, src);
    }


    template <OpCode OP, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::Emit(Register<8, false> dest, __int32 destOffset, Register<SIZE2, ISFLOAT2> src)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes2<ISFLOAT1, ISFLOAT2>::Emit<SIZE1, SIZE2>(*this, dest, destOffset, src);

        printer.Print(OP, dest, destOffset, src);
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::EmitImmediate(Register<SIZE, ISFLOAT> dest, T value)
    {
        CodePrinter printer(*this);

        Helper<OP>::ArgTypes1<ISFLOAT>::EmitImmediate<SIZE, T>(*this, dest, value);

        printer.PrintImmediate(OP, dest, value);
    }


    //*************************************************************************
    //
    // Template definitions for X64CodeGenerator - private methods.
    //
    //*************************************************************************

    //
    // X64 opcodes
    //
    template <unsigned SIZE>
    void X64CodeGenerator::IMul(Register<SIZE, false> dest,
                                Register<SIZE, false> src)
    {
        EmitOpSizeOverrideDirect(dest, src);
        EmitRexDirect(dest, src);
        Emit8(0x0f);
        Emit8(0xAF);
        EmitModRM(dest, src);
    }


    template <unsigned SIZE>
    void X64CodeGenerator::IMul(Register<SIZE, false> dest,
                                Register<8, false> src,
                                __int32 srcOffset)
    {
        EmitOpSizeOverrideIndirect<SIZE, false>(dest, src);
        EmitRexIndirect<SIZE, false>(dest, src);
        Emit8(0x0f);
        Emit8(0xAF);
        EmitModRMOffset(dest, src, srcOffset);
    }


    // Illegal for SIZE == 1?
    template <unsigned SIZE, typename T>
    void X64CodeGenerator::IMulImmediate(Register<SIZE, false> dest,
                                         T value)
    {
        unsigned valueSize = Size(value);

        EmitOpSizeOverrideDirect(dest, dest);
        EmitRexDirect(dest, dest);

        if (valueSize == 1)
        {
            Emit8(0x6b);
            EmitModRM(dest, dest);
            Emit8(static_cast<unsigned __int8>(value));
        }
        else
        {
            Emit8(0x69);
            EmitModRM(dest, dest);
            if (SIZE == 2)
            {
                Assert(valueSize <= 2, "Expected two-byte value.");
                Emit16(static_cast<unsigned __int16>(value));
            }
            else
            {
                Assert(valueSize <= 4, "Expected four-byte value.");
                Emit32(static_cast<unsigned __int32>(value));
            }
        }
    }


    template <unsigned SIZE>
    void X64CodeGenerator::Lea(Register<SIZE, false> dest,
                               Register<8, false> src,
                               __int32 srcOffset)
    {
        EmitRexIndirect<SIZE, false>(dest, src);
        Emit8(0x8d);
        EmitModRMOffset(dest, src, srcOffset);
    }


    template <unsigned SIZE, typename T>
    void X64CodeGenerator::MovImmediate(Register<SIZE, false> dest,
                                        T value)
    {
        unsigned valueSize = Size(value);

        EmitOpSizeOverride(dest);

        if (SIZE == 1)
        {
            Emit8(0xb0 | static_cast<unsigned __int8>(dest.GetId()));
            Assert(valueSize == 1, "Expected one-byte value.");
            Emit8(static_cast<unsigned __int8>(value));
        }
        else if (SIZE == 8 && valueSize <= 4)
        {
            EmitRex(dest);
            Emit8(0xc7);
            EmitModRM(0, dest);
            Emit32(static_cast<unsigned __int32>(value));
        }
        else
        {
            EmitRex(dest);
            Emit8(0xb8 + dest.GetId8());
            if (SIZE == 2)
            {
                Assert(valueSize <= 2, "Expected two-byte value.");
                Emit16(static_cast<unsigned __int16>(value));
            }
            else if (SIZE == 4)
            {
                Assert(valueSize <= 4, "Expected four-byte value.");
                Emit32(static_cast<unsigned __int32>(value));
            }
            else
            {
                Emit64(static_cast<unsigned __int64>(value));
            }
        }
    }


    // TODO: Need a unit test for this case.
    template <typename T>
    void X64CodeGenerator::MovImmediate(Register<8, false> dest,
                                        T* value)
    {
        MovImmediate(dest, reinterpret_cast<unsigned __int64>(value));
    }


    template <unsigned SIZE>
    void X64CodeGenerator::MovD(Register<SIZE, true> dest, Register<SIZE, false> src)
    {
        Emit8(0x66);
        EmitRexDirect(dest, src);
        Emit8(0x0f);
        Emit8(0x6e);
        EmitModRM(dest, src);
    }


    template <unsigned SIZE1, unsigned SIZE2>
    void X64CodeGenerator::MovZX(Register<SIZE1, false> dest, Register<SIZE2, false> src)
    {
        static_assert(SIZE2 == 1 || SIZE2 == 2, "Invalid source size.");
        static_assert(SIZE1 > SIZE2, "Target size must be larger than the source size.");

        const bool twoByteSource = SIZE2 == 2;

        // Size override is not necessary for 16-bit source since an opcode that
        // defaults to 16 bits is used in that scenario.
        // 
        if (!twoByteSource)
        {
            EmitOpSizeOverrideDirect(dest, src);
        }
        EmitRexDirect(dest, src);
        Emit8(0x0f);
        Emit8(twoByteSource ? 0xb7 : 0xb6);
        EmitModRM(dest, src);
    }


    template <unsigned SIZE1, unsigned SIZE2>
    void X64CodeGenerator::MovZX(Register<SIZE1, false> dest, Register<8, false> src, __int32 srcOffset)
    {
        static_assert(SIZE2 == 1 || SIZE2 == 2, "Invalid source size.");
        static_assert(SIZE1 > SIZE2, "Target size must be larger than the source size.");

        const bool twoByteSource = SIZE2 == 2;

        if (!twoByteSource)
        {
            EmitOpSizeOverrideIndirect<SIZE2, false>(dest, src);
        }
        EmitRexIndirect<SIZE2, false>(dest, src);
        Emit8(0x0f);
        Emit8(twoByteSource ? 0xb7 : 0xb6);
        EmitModRM(dest, src);
    }


    // TODO: Consider coalescing with Group2. Pattern is very similar.
    // TODO: Support for SHRD.
    template <unsigned SIZE>
    void X64CodeGenerator::Shld(Register<SIZE, false> dest, Register<SIZE, false> src)
    {
        // Note: operand encoding is MRC, so the order of arguments for the
        // Emit*() methods is reversed.
        EmitOpSizeOverrideDirect(src, dest);
        EmitRexDirect(src, dest);
        Emit8(0x0f);
        Emit8(0xa5);
        EmitModRM(src, dest);
    }


    //
    // SSE instructions
    //


    template <unsigned RMSIZE, bool RMISFLOAT,
              unsigned REGSIZE, bool REGISFLOAT,
              unsigned RMREGSIZE, bool RMREGISFLOAT>
    void X64CodeGenerator::EmitScalarSSEPrefix(Register<REGSIZE, REGISFLOAT> /* reg */,
                                               Register<RMREGSIZE, RMREGISFLOAT> /* rm */)
    {
        static_assert(RMISFLOAT || REGISFLOAT, "Invalid ScalarSSE reference.");
        static_assert(RMSIZE >= 4 && REGSIZE >= 4, "Invalid integer register.");

        if (RMISFLOAT)
        {
            Emit8(RMSIZE == 8 ? 0xf2 : 0xf3);
        }
        else
        {
            Emit8(REGSIZE == 8 ? 0xf2 : 0xf3);
        }
    }


    template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::EmitScalarSSEPrefixDirect(Register<SIZE1, ISFLOAT1> dest,
                                                     Register<SIZE2, ISFLOAT2> src)
    {
        EmitScalarSSEPrefix<SIZE2, ISFLOAT2>(dest, src);
    }


    template <unsigned RMSIZE, bool RMISFLOAT, unsigned REGSIZE, bool REGISFLOAT>
    void X64CodeGenerator::EmitScalarSSEPrefixIndirect(Register<REGSIZE, REGISFLOAT> reg, Register<8, false> rm)
    {
        EmitScalarSSEPrefix<RMSIZE, RMISFLOAT>(reg, rm);
    }


    template <unsigned __int8 OPCODE, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::ScalarSSE(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src)
    {
        EmitScalarSSEPrefixDirect(dest, src);
        EmitRexDirect(dest, src);
        Emit8(0x0f);
        Emit8(OPCODE);
        EmitModRM(dest, src);
    }


    template <unsigned __int8 OPCODE, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::ScalarSSE(Register<SIZE1, ISFLOAT1> dest,
                                     Register<8, false> src,
                                     __int32 srcOffset)
    {
        EmitScalarSSEPrefixIndirect<SIZE2, ISFLOAT2>(dest, src);
        EmitRexIndirect<SIZE2, ISFLOAT2>(dest, src);
        Emit8(0x0f);
        Emit8(OPCODE);
        EmitModRMOffset(dest, src, srcOffset);
    }


    template <unsigned __int8 OPCODE, unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::ScalarSSE(Register<8, false> dest,
                                     __int32 destOffset,
                                     Register<SIZE2, ISFLOAT2> src)
    {
        // Note: operand encoding is MR, so the order of arguments for the
        // Emit*() methods is reversed.
        EmitScalarSSEPrefixIndirect<SIZE1, ISFLOAT1>(src, dest);
        EmitRexIndirect<SIZE1, ISFLOAT1>(src, dest);
        Emit8(0x0f);
        Emit8(OPCODE);
        EmitModRMOffset(src, dest, destOffset);
    }


    //
    // X64 group1 opcodes
    //

    template <unsigned SIZE>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  Register<SIZE, false> dest,
                                  Register<SIZE, false> src)
    {
        EmitOpSizeOverrideDirect(dest, src);
        EmitRexDirect(dest, src);
        if (SIZE == 1)
        {
            Emit8(baseOpCode + 0x2);
        }
        else
        {
            Emit8(baseOpCode + 0x3);
        }
        EmitModRM(dest, src);
    }


    template <unsigned SIZE>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  Register<SIZE, false> dest,
                                  Register<8, false> src,
                                  __int32 srcOffset)
    {
        EmitOpSizeOverrideIndirect<SIZE, false>(dest, src);
        EmitRexIndirect<SIZE, false>(dest, src);
        if (SIZE == 1)
        {
            Emit8(baseOpCode + 0x2);
        }
        else
        {
            Emit8(baseOpCode + 0x3);
        }
        EmitModRMOffset(dest, src, srcOffset);
    }


    // TODO: It appears that this cases is not directly tested for Group1 opcodes.
    // It might be tested for mov.
    // TODO: This doesn't seem correct for Add, etc. since the base opcode doesn't change.
    template <unsigned SIZE>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  Register<8, false> dest,
                                  __int32 destOffset,
                                  Register<SIZE, false> src)
    {
        Group1(baseOpCode, src, dest, destOffset);
    }


    // TODO: Would like some sort of compiletime error when T is quadword or floating point
    template <unsigned SIZE, typename T>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  unsigned __int8 extensionOpCode,
                                  Register<SIZE, false> dest,
                                  T value)
    {
        unsigned valueSize = Size(value);

        EmitOpSizeOverride(dest);

        if (dest.GetId() == 0 && SIZE == 1)
        {
            // Special case for AL.
            Assert(valueSize == 1, "AL requires one-byte value.");
            Emit8(baseOpCode + 0x04);
            Emit8(static_cast<unsigned __int8>(value));
        }
        else if (dest.GetId() == 0 && valueSize != 1)
        {
            EmitRex(dest);
            Emit8(baseOpCode + 0x05);
            if (SIZE == 2)
            {
                Assert(valueSize <= 2, "AX requires two-byte value.");
                Emit16(static_cast<unsigned __int16>(value));
            }
            else
            {
                Assert(valueSize <= 4, "EAX requires four-byte value.");
                Emit32(static_cast<unsigned __int32>(value));
            }
        }
        else
        {
            // TODO: BUGBUG: This code does not handle 8-bit registers correctly. e.g. AND BH, 5
            EmitRex(dest);

            if (valueSize <= 1)
            {
                if (SIZE == 1)
                {
                    Emit8(0x80 /*+ 3*/);
                }
                else
                {
                    Emit8(0x83);
                }
                EmitModRM(extensionOpCode, dest);
                Emit8(static_cast<unsigned __int8>(value));
            }
            else if (valueSize == 2 || valueSize == 4)
            {
                Emit8(0x80 + 1);
                EmitModRM(extensionOpCode, dest);

                if (SIZE == 2)
                {
                    Emit16(static_cast<unsigned __int16>(value));
                }
                else
                {
                    Emit32(static_cast<unsigned __int32>(value));
                }
            }
            else
            {
                // Can't do 8-byte immdediate values.
                // More accurately, can't do cases where valueSize is 3,5,6, or 7.
                // TODO: Template should be disabled for this size to avoid runtime error.
                throw 0;
            }
        }
    }


    //
    // X64 group2 opcodes
    //

    template <unsigned SIZE>
    void X64CodeGenerator::Group2(unsigned __int8 extensionOpCode,
                                  Register<SIZE, false> dest)
    {
        EmitOpSizeOverride(dest);
        EmitRex(dest);
        if (SIZE == 1)
        {
            Emit8(0xd2);
        }
        else
        {
            Emit8(0xd3);
        }
        EmitModRM(extensionOpCode, dest);
    }


    // Coalesce with previous by calling previous before emitting shift.
    template <unsigned SIZE>
    void X64CodeGenerator::Group2(unsigned __int8 extensionOpCode,
                                  unsigned __int8 shift,
                                  Register<SIZE, false> dest)
    {
        EmitOpSizeOverride(dest);
        EmitRex(dest);
        if (SIZE == 1)
        {
            Emit8(0xc0);
        }
        else
        {
            Emit8(0xc1);
        }
        EmitModRM(extensionOpCode, dest);
        Emit8(shift);
    }


    //
    // X64 opcode encoding - operand size override.
    //

    template <unsigned RMSIZE, bool RMISFLOAT,
              unsigned REGSIZE, bool REGISFLOAT,
              unsigned RMREGSIZE, bool RMREGISFLOAT>
    void X64CodeGenerator::EmitOpSizeOverride(Register<REGSIZE, REGISFLOAT> /* reg */,
                                              Register<RMREGSIZE, RMREGISFLOAT> /* rm */)
    {
        static_assert((RMSIZE == RMREGSIZE && RMISFLOAT == RMREGISFLOAT)
                      || (RMREGSIZE == 8 && !RMREGISFLOAT),
                      "Only direct addressing or indirect addresing with 64-bit "
                      "general purpose base register can be used.");

        // Emit operand size override prefix if necessary.
        // Note: address size override prefix (0x67) is not considered since
        // we don't support 32-bit indirects.
        if (RMSIZE == 2 || REGSIZE == 2)
        {
            Emit8(0x66);
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::EmitOpSizeOverride(Register<SIZE, ISFLOAT> reg)
    {
        EmitOpSizeOverride<SIZE, ISFLOAT>(reg, reg);
    }


    template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::EmitOpSizeOverrideDirect(Register<SIZE1, ISFLOAT1> dest,
                                                    Register<SIZE2, ISFLOAT2> src)
    {
        EmitOpSizeOverride<SIZE2, ISFLOAT2>(dest, src);
    }


    template <unsigned RMSIZE, bool RMISFLOAT>
    void X64CodeGenerator::EmitOpSizeOverrideIndirect(Register<8, false> rm)
    {
        EmitOpSizeOverride<RMSIZE, RMISFLOAT>(Register<RMSIZE, RMISFLOAT>(), rm);
    }


    template <unsigned RMSIZE, bool RMISFLOAT, unsigned REGSIZE, bool REGISFLOAT>
    void X64CodeGenerator::EmitOpSizeOverrideIndirect(Register<REGSIZE, REGISFLOAT> reg,
                                                      Register<8, false> rm)
    {
        EmitOpSizeOverride<RMSIZE, RMISFLOAT>(reg, rm);
    }


    //
    // X64 opcode encoding - REX.
    //

    template <unsigned RMSIZE, bool RMISFLOAT,
              unsigned REGSIZE, bool REGISFLOAT,
              unsigned RMREGSIZE, bool RMREGISFLOAT>
    void X64CodeGenerator::EmitRex(Register<REGSIZE, REGISFLOAT> reg,
                                   Register<RMREGSIZE, RMREGISFLOAT> rm)
    {
        static_assert((RMSIZE == RMREGSIZE && RMISFLOAT == RMREGISFLOAT)
                      || (RMREGSIZE == 8 && !RMREGISFLOAT),
                      "Only direct addressing or indirect addresing with 64-bit "
                      "general purpose base register can be used.");

        // TODO: add cases for X bit.
        //
        // Note that the REX.W bit is never set when two floating point operands
        // are used.
        //
        // In the indirect mode, the size and type of the R/M operand are
        // determined by the size and type of the target memory area (given
        // that the base register is always a general purpose register and size
        // in our case always 8).
        const bool w = (REGSIZE == 8 && !REGISFLOAT) || (RMSIZE == 8 && !RMISFLOAT);

        if (w || reg.IsExtended() || rm.IsExtended())
        {
            // WRXB
            Emit8(0x40
                  | (w ? 8 : 0)
                  | (reg.IsExtended() ? 4 : 0)
                  | (rm.IsExtended() ? 1 : 0));
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::EmitRex(Register<SIZE, ISFLOAT> reg)
    {
        // Use eax for the other register since it will not affect any of the REX flags.
        EmitRex<SIZE, ISFLOAT>(eax, reg);
    }


    template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::EmitRexDirect(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src)
    {
        EmitRex<SIZE2, ISFLOAT2>(dest, src);
    }


    template <unsigned RMSIZE, bool RMISFLOAT>
    void X64CodeGenerator::EmitRexIndirect(Register<8, false> rm)
    {
        // Use eax for the other register since it will not affect any of the REX flags.
        EmitRex<RMSIZE, RMISFLOAT>(eax, rm);
    }


    template <unsigned RMSIZE, bool RMISFLOAT, unsigned REGSIZE, bool REGISFLOAT>
    void X64CodeGenerator::EmitRexIndirect(Register<REGSIZE, REGISFLOAT> reg, Register<8, false> rm)
    {
        EmitRex<RMSIZE, RMISFLOAT>(reg, rm);
    }


    //
    // X64 opcode encoding - ModR/M.
    //

    template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    void X64CodeGenerator::EmitModRM(Register<SIZE1, ISFLOAT1> dest, Register<SIZE2, ISFLOAT2> src)
    {
        Emit8(0xc0 | (dest.GetId8() << 3) | src.GetId8());
        // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
        // this function is only used for Register-Register encoding. Problem will 
        // crop up if caller passes the base register from an X64Indirect.
    }


    template <unsigned SIZE>
    void X64CodeGenerator::EmitModRM(unsigned __int8 extensionOpCode, Register<SIZE, false> dest)
    {
        Emit8(0xc0 | (extensionOpCode << 3) | dest.GetId8());
        // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
        // this function is only used for Register-Register encoding. Problem will 
        // crop up if caller passes the base register from an X64Indirect.
    }


    inline unsigned __int8 Mod(__int32 offset)
    {
        if (offset == 0)
        {
            return 0;
        }
        else if (offset >= -0x80 && offset <= 0x7f)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::EmitModRMOffset(Register<SIZE, ISFLOAT> reg, Register<8, false> rm, __int32 offset)
    {
        if (rm.IsRIP())
        {
            // RIP-relative addressing. Hard-code mod = 0 and rmField = 5.
            const unsigned __int8 mod = 0;
            const unsigned __int8 rmField = 5;
            const unsigned __int8 regField = reg.GetId8();

            Emit8((mod << 6) | (regField << 3) | rmField);

            Emit32(offset - CurrentPosition() - 4);
        }
        else
        {
            // Normal, GPR-indirect addressing.
            unsigned __int8 mod = Mod(offset);
            const unsigned __int8 rmField = rm.GetId8();
            const unsigned __int8 regField = reg.GetId8();

            if (rmField == 5 && mod == 0)
            {
                // The combination of rmField == 5 && mod == 0 is a special case
                // which is used for RIP-relative addressing. Convert to mod 01
                // and emit an 8-bit displacement of 0.
                mod = 1;
            }

            Emit8((mod << 6) | (regField << 3) | rmField);

            if (rmField == 4)
            {
                // When rm is RSP or R12 or XMM4 or XMM12, rmField == 4, which
                // is a special case used for SIB addressing. Emit an SIB byte
                // which encodes the same register with no scaled index.
                // Want SS = 00, Index = 100 (none), and Base = 100 (4).
                Emit8(0x24);
            }

            if (mod == 1)
            {
                Emit8(static_cast<unsigned __int8>(offset));
            }
            else if (mod == 2)
            {
                Emit32(offset);
            }
        }
    }


    //*************************************************************************
    //
    // X64CodeGenerator::Helper definitions for each opcode and addressing mode.
    //
    //*************************************************************************

    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::Lea>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        Register<8, false> src,
        __int32 srcOffset)
    {
        code.Lea(dest, src, srcOffset);
    }


    //
    // Mov
    //

    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::Mov>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        Register<SIZE, false> src)
    {
        code.Group1(0x88, dest, src);
    }


    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::Mov>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        Register<8, false> src,
        __int32 srcOffset)
    {
        code.Group1(0x88, dest, src, srcOffset);
    }


    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::Mov>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<8, false> dest,
        __int32 destOffset,
        Register<SIZE, false> src)
    {
        code.Group1(0x86, dest, destOffset, src);
    }


    template <>
    template <>
    template <unsigned SIZE, typename T>
    void X64CodeGenerator::Helper<OpCode::Mov>::ArgTypes1<false>::EmitImmediate(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        T value)
    {
        code.MovImmediate(dest, value);
    }


    template <>
    template <>
    template <unsigned SIZE1, unsigned SIZE2>
    void X64CodeGenerator::Helper<OpCode::Mov>::ArgTypes2<true, false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE1, true> dest,
        Register<SIZE2, false> src)
    {
        code.MovD(dest, src);
    }


    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::Mov>::ArgTypes1<true>::Emit(
        X64CodeGenerator& code,
        Register<8, false> dest,
        __int32 destOffset,
        Register<SIZE, true> src)
    {
        // MovSS/SD.
        code.ScalarSSE<0x11, SIZE, true, SIZE, true>(dest, destOffset, src);
    }


    //
    // Mul
    //

    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::IMul>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        Register<SIZE, false> src)
    {
        code.IMul(dest, src);
    }


    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::IMul>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        Register<8, false> src,
        __int32 srcOffset)
    {
        code.IMul(dest, src, srcOffset);
    }


    template <>
    template <>
    template <unsigned SIZE, typename T>
    void X64CodeGenerator::Helper<OpCode::IMul>::ArgTypes1<false>::EmitImmediate(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        T value)
    {
        code.IMulImmediate(dest, value);
    }


    //
    // MovZX
    //

    template <>
    template <>
    template <unsigned SIZE1, unsigned SIZE2>
    void X64CodeGenerator::Helper<OpCode::MovZX>::ArgTypes2<false, false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE1, false> dest,
        Register<SIZE2, false> src)
    {
        code.MovZX(dest, src);
    }


    template <>
    template <>
    template <unsigned SIZE1, unsigned SIZE2>
    void X64CodeGenerator::Helper<OpCode::MovZX>::ArgTypes2<false, false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE1, false> dest,
        Register<8, false> src,
        __int32 srcOffset)
    {
        code.MovZX<SIZE1, SIZE2>(dest, src, srcOffset);
    }


    //
    // Shld
    //
    template <>
    template <>
    template <unsigned SIZE>
    void X64CodeGenerator::Helper<OpCode::Shld>::ArgTypes1<false>::Emit(
        X64CodeGenerator& code,
        Register<SIZE, false> dest,
        Register<SIZE, false> src)
    {
        code.Shld(dest, src);
    }


#define DEFINE_GROUP1(name, baseOpCode, extensionOpCode) \
    template <>                                                                                 \
    template <>                                                                                 \
    template <unsigned SIZE>                                                                    \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<false>::Emit(                    \
        X64CodeGenerator& code,                                                                 \
        Register<SIZE, false> dest,                                                             \
        Register<SIZE, false> src)                                                              \
    {                                                                                           \
        code.Group1(baseOpCode, dest, src);                                                     \
    }                                                                                           \
                                                                                                \
                                                                                                \
    template <>                                                                                 \
    template <>                                                                                 \
    template <unsigned SIZE>                                                                    \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<false>::Emit(                    \
        X64CodeGenerator& code,                                                                 \
        Register<SIZE, false> dest,                                                             \
        Register<8, false> src,                                                                 \
        __int32 srcOffset)                                                                      \
    {                                                                                           \
        code.Group1(baseOpCode, dest, src, srcOffset);                                          \
    }                                                                                           \
                                                                                                \
                                                                                                \
    template <>                                                                                 \
    template <>                                                                                 \
    template <unsigned SIZE, typename T>                                                        \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<false>::EmitImmediate(           \
        X64CodeGenerator& code,                                                                 \
        Register<SIZE, false> dest,                                                             \
        T value)                                                                                \
    {                                                                                           \
        code.Group1(baseOpCode, extensionOpCode, dest, value);                                  \
    }

    DEFINE_GROUP1(Add, 0, 0);
    DEFINE_GROUP1(And, 0x20, 4);
    DEFINE_GROUP1(Or, 8, 1);
    DEFINE_GROUP1(Sub, 0x28, 5);
    DEFINE_GROUP1(Cmp, 0x38, 7);

#undef DEFINE_GROUP1


#define DEFINE_GROUP2(name, extensionOpCode)                                                    \
    template <>                                                                                 \
    template <>                                                                                 \
    template <unsigned SIZE>                                                                    \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<false>::Emit(                    \
        X64CodeGenerator& code,                                                                 \
        Register<SIZE, false> dest)                                                             \
    {                                                                                           \
        code.Group2(extensionOpCode, dest);                                                     \
    }                                                                                           \
                                                                                                \
                                                                                                \
    template <>                                                                                 \
    template <>                                                                                 \
    template <unsigned SIZE, typename T>                                                        \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<false>::EmitImmediate(           \
        X64CodeGenerator& code,                                                                 \
        Register<SIZE, false> dest,                                                             \
        T value)                                                                                \
    {                                                                                           \
        code.Group2(extensionOpCode, value, dest);                                              \
    }                                                                                    

    DEFINE_GROUP2(Rol, 0);
    DEFINE_GROUP2(Sal, 4);
    DEFINE_GROUP2(Shr, 5);

#undef DEFINE_GROUP2


// SSE, both arguments of the same type and size.
#define DEFINE_SCALAR_SSE(name, opcode) \
    template <>                                                                         \
    template <>                                                                         \
    template <unsigned SIZE>                                                            \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<true>::Emit(             \
        X64CodeGenerator& code,                                                         \
        Register<SIZE, true> dest,                                                      \
        Register<SIZE, true> src)                                                       \
    {                                                                                   \
        code.ScalarSSE<opcode>(dest, src);                                              \
    }                                                                                   \
                                                                                        \
                                                                                        \
    template <>                                                                         \
    template <>                                                                         \
    template <unsigned SIZE>                                                            \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes1<true>::Emit(             \
        X64CodeGenerator& code,                                                         \
        Register<SIZE, true> dest,                                                      \
        Register<8, false> src,                                                         \
        __int32 srcOffset)                                                              \
    {                                                                                   \
        code.ScalarSSE<opcode, SIZE, true, SIZE, true>(dest, src, srcOffset);           \
    }                                                                                   \

    DEFINE_SCALAR_SSE(Add, 0x58);  // AddSS/AddSD.
    DEFINE_SCALAR_SSE(IMul, 0x59); // MulSS/MulSD.
    DEFINE_SCALAR_SSE(Mov, 0x10);  // MovSS/MovSD.
    DEFINE_SCALAR_SSE(Sub, 0x5c);  // SubSS/SubSD.

#undef DEFINE_SCALAR_SSE


// SSE, arguments of different type or size.
#define DEFINE_SCALAR_SSE(name, opcode, type1, type2, validityCondition)                \
    template <>                                                                         \
    template <>                                                                         \
    template <unsigned SIZE1, unsigned SIZE2>                                           \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes2<type1, type2>::Emit(     \
        X64CodeGenerator& code,                                                         \
        Register<SIZE1, type1> dest,                                                    \
        Register<SIZE2, type2> src)                                                     \
    {                                                                                   \
        static_assert(validityCondition,                                                \
                      "Invalid " #name " instruction, must be " #validityCondition);    \
        code.ScalarSSE<opcode, SIZE1, type1, SIZE2, type2>(dest, src);                  \
    }                                                                                   \
                                                                                        \
                                                                                        \
    template <>                                                                         \
    template <>                                                                         \
    template <unsigned SIZE1, unsigned SIZE2>                                           \
    void X64CodeGenerator::Helper<OpCode::##name##>::ArgTypes2<type1, type2>::Emit(     \
        X64CodeGenerator& code,                                                         \
        Register<SIZE1, type1> dest,                                                    \
        Register<8, false> src,                                                         \
        __int32 srcOffset)                                                              \
    {                                                                                   \
        static_assert(validityCondition,                                                \
                      "Invalid " #name " instruction, must be " #validityCondition);    \
        code.ScalarSSE<opcode, SIZE1, type1, SIZE2, type2>(dest, src, srcOffset);       \
    }                                                                                   \

    DEFINE_SCALAR_SSE(CvtSI2FP, 0x2A, true, false, true == true);   // CvtSI2SD/CvtSI2SS (convert signed int to floating point).
    DEFINE_SCALAR_SSE(CvtFP2SI, 0x2C, false, true, true == true);   // CvtTSD2SI/CvtTSS2SI (convert floating point to signed int with truncation).
    DEFINE_SCALAR_SSE(CvtFP2FP, 0x5A, true, true, SIZE1 != SIZE2);  // CvtSS2SD/CvtSD2SS (convert float to double and vice versa).

#undef DEFINE_SCALAR_SSE
}

#pragma warning(pop)
