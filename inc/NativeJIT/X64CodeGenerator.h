#pragma once

// TODO: No way to do an operation on sign-extended data. Need to load data first, then sign-extend.


// http://wiki.osdev.org/X86-64_Instruction_Encoding

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
        JE = 4, JZ =4,
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
        JNLE = 0xf, JG = 0xf
    };


    // WARNING: When modifying OpCode, be sure to also modify the function OpCodeName().
    enum class OpCode : unsigned
    {
        Add,
        And,
        Call,
        Cmp,
        IMul,
        Lea,
        Mov,
        Nop,
        Or,
        Pop,
        Push,
        Ret,
        Sub,
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

        // Two register operands (e.g. and, mov, or, sub)
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src);

        // Two operands - register destination and indirect source.
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest, Register<8, false> src, unsigned __int32 srcOffset);

        // Two operands - indirect destination and register source.
        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<8, false> dest, unsigned __int32 destOffset, Register<SIZE, ISFLOAT> src);

        // Top operands - register destination and immediate source.
        template <OpCode OP, unsigned SIZE, bool ISFLOAT, typename T>
        void Emit(Register<SIZE, ISFLOAT> dest, T value);


    private:
        void Call(Register<8, false> /*r*/);

        template <unsigned SIZE>
        void IMul(Register<SIZE, false> dest,
                  Register<SIZE, false> src);

        template <unsigned SIZE>
        void IMul(Register<SIZE, false> dest,
                  Register<8, false> src,
                  __int32 srcOffset);

        // TODO: Would like some sort of compiletime error when T is quadword or floating point
        template <unsigned SIZE, typename T>
        void IMul(Register<SIZE, false> dest,
                  T value);

        template <unsigned SIZE>
        void Lea(Register<SIZE, false> dest,
                 Register<8, false> src,
                 __int32 srcOffset);

        template <unsigned SIZE, bool ISFLOAT, typename T>
        void Mov(Register<SIZE, ISFLOAT> dest,
                 T value);

        template <typename T>
        void Mov(Register<8, false> dest,
                 T* value);

        void Pop(Register<8, false> r);
        void Push(Register<8, false> r);

        void Ret();

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

        template <unsigned SIZE1, unsigned SIZE2>
        void EmitRex(Register<SIZE1, false> dest, Register<SIZE2, false> src);

        template <unsigned SIZE>
        void EmitRex(Register<SIZE, false> dest);

        template <unsigned SIZE>
        void EmitModRM(Register<SIZE, false> dest, Register<SIZE, false> src);

        template <unsigned SIZE>
        void EmitModRM(unsigned __int8 extensionOpCode, Register<SIZE, false> dest);

        template <unsigned SIZE>
        void EmitModRMOffset(Register<SIZE, false> dest, Register<8, false> src, __int32 srcOffset);

        void Indent();
        void PrintBytes(unsigned start, unsigned end);


        // Helper class used to provide partial specializations by OpCode for the Emit() methods.
        template <OpCode OP>
        class Helper
        {
        public:
            static void Emit(X64CodeGenerator& code);

            template <unsigned SIZE, bool ISFLOAT>
            static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest);

            template <unsigned SIZE, bool ISFLOAT>
            static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src);

            template <unsigned SIZE, bool ISFLOAT>
            static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, Register<8, false> src, __int32 srcOffset);

            template <unsigned SIZE, bool ISFLOAT>
            static void Emit(X64CodeGenerator& code, Register<8, false> dest, __int32 destOffset, Register<SIZE, ISFLOAT> src);

            template <unsigned SIZE, bool ISFLOAT, typename T>
            static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, T value);
        };

        static const unsigned c_rxxRegisterCount = 16;
        static const unsigned c_xmmRegisterCount = 16;

        std::ostream* m_out;
    };


    //*************************************************************************
    //
    // Template definitions for X64CodeGenerator - public methods.
    //
    //*************************************************************************
    template <JccType JCC>
    void X64CodeGenerator::Emit(Label label)
    {
        unsigned start = CurrentPosition();

        Emit8(0xf);
        Emit8(0x80 + static_cast<unsigned __int8>(JCC));
        EmitCallSite(label, 4);

        if (m_out != nullptr)
        {
            PrintBytes(start, CurrentPosition());
            *m_out << JccName(JCC) << " L" << label.GetId() << std::endl;
        }
    }


    template <OpCode OP>
    void X64CodeGenerator::Emit()
    {
        if (m_out != nullptr)
        {
            unsigned start = CurrentPosition();
            Helper<OP>::Emit(*this);
            PrintBytes(start, CurrentPosition());
            *m_out << OpCodeName(OP) << std::endl;
        }
        else
        {
            Helper<OP>::Emit(*this);
        }
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest)
    {
        if (m_out != nullptr)
        {
            unsigned start = CurrentPosition();
            Helper<OP>::Emit(*this, dest);
            PrintBytes(start, CurrentPosition());
            *m_out << OpCodeName(OP) << ' ' << dest.GetName() << std::endl;
        }
        else
        {
            Helper<OP>::Emit(*this, dest);
        }
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src)
    {
        if (m_out != nullptr)
        {
            unsigned start = CurrentPosition();
            Helper<OP>::Emit(*this, dest, src);
            PrintBytes(start, CurrentPosition());
            *m_out << OpCodeName(OP) << ' ' << dest.GetName() << ", " << src.GetName() << std::endl;
        }
        else
        {
            Helper<OP>::Emit(*this, dest, src);
        }
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest, Register<8, false> src, unsigned __int32 srcOffset)
    {
        if (m_out != nullptr)
        {
            unsigned start = CurrentPosition();
            Helper<OP>::Emit(*this, dest, src, srcOffset);
            PrintBytes(start, CurrentPosition());
            *m_out << OpCodeName(OP) << ' ' << dest.GetName();
            *m_out << ", [" << src.GetName();
            if (srcOffset != 0)
            {
                *m_out << " + " << std::hex << srcOffset << "h";
            }
            *m_out << "]"  << std::endl;
        }
        else
        {
            Helper<OP>::Emit(*this, dest, src, srcOffset);
        }
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Emit(Register<8, false> dest, unsigned __int32 destOffset, Register<SIZE, ISFLOAT> src)
    {
        if (m_out != nullptr)
        {
            unsigned start = CurrentPosition();
            Helper<OP>::Emit(*this, dest, destOffset, src);
            PrintBytes(start, CurrentPosition());
            *m_out << OpCodeName(OP) << " [" << dest.GetName();
            if (destOffset != 0)
            {
                *m_out << " + " << std::hex << destOffset << "h";
            }
            *m_out << "], " << src.GetName() << std::endl;
        }
        else
        {
            Helper<OP>::Emit(*this, dest, destOffset, src);
        }
    }


    template <OpCode OP, unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::Emit(Register<SIZE, ISFLOAT> dest, T value)
    {
        if (m_out != nullptr)
        {
            unsigned start = CurrentPosition();
            Helper<OP>::Emit(*this, dest, value);
            PrintBytes(start, CurrentPosition());
            *m_out << OpCodeName(OP) << ' ' << dest.GetName();
            // TODO: Hex may not be appropriate for float.
            *m_out << ", " << std::hex << value << 'h' << std::endl;
        }
        else
        {
            Helper<OP>::Emit(*this, dest, value);
        }
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
        if (SIZE == 2)
        {
            Emit8(0x66);                // Size override prefix.
        }
        EmitRex(dest, src);
        Emit8(0x0f);
        Emit8(0xAF);
        EmitModRM(dest, src);
    }


    template <unsigned SIZE>
    void X64CodeGenerator::IMul(Register<SIZE, false> dest,
                                Register<8, false> src,
                                __int32 srcOffset)
    {
        if (SIZE == 2)
        {
            Emit8(0x66);                // Size override prefix.
        }
        EmitRex(dest, src);
        Emit8(0x0f);
        Emit8(0xAF);
        EmitModRMOffset(dest, src, srcOffset);
    }


    // Illegal for SIZE == 1?
    template <unsigned SIZE, typename T>
    void X64CodeGenerator::IMul(Register<SIZE, false> dest,
                                T value)
    {
        unsigned valueSize = Size(value);

        if (SIZE == 2)
        {
            Emit8(0x66);                // Size override prefix.
        }

        EmitRex(dest, dest);

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
        EmitRex(dest, src);
        Emit8(0x8d);
        EmitModRMOffset(dest, src, srcOffset);
    }


    template <unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::Mov(Register<SIZE, ISFLOAT> dest,
                               T value)
    {
        unsigned valueSize = Size(value);

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
            EmitModRM(0xc0, dest);
            Emit32(static_cast<unsigned __int32>(value));
        }
        else
        {
            if (SIZE == 2)
            {
                Emit8(0x66);                // Size override prefix.
            }
            EmitRex(dest);
            Emit8(0xb8 + (static_cast<unsigned __int8>(dest.GetId()) & 0x7));       // TODO: Method to get lower three id bits.
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
    void X64CodeGenerator::Mov(Register<8, false> dest,
                               T* value)
    {
        Mov(dest, reinterpret_cast<unsigned __int64>(value));
    }


    //
    // X64 group1 opcodes
    //

    template <unsigned SIZE>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  Register<SIZE, false> dest,
                                  Register<SIZE, false> src)
    {
        if (SIZE == 2)
        {
            Emit8(0x66);                // Size override prefix.
        }
        EmitRex(dest, src);
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
        if (SIZE == 2)
        {
            Emit8(0x66);                // Size override prefix.
        }
        EmitRex(dest, src);
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


    template <unsigned SIZE>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  Register<8, false> dest,
                                  __int32 destOffset,
                                  Register<SIZE, false> src)
    {
        if (SIZE == 2)
        {
            Emit8(0x66);                // Size override prefix.
        }
        EmitRex(src, dest);
        if (SIZE == 1)
        {
            Emit8(baseOpCode + 0x2);
        }
        else
        {
            Emit8(baseOpCode + 0x3);
        }
        EmitModRMOffset(src, dest, destOffset);
    }


    // TODO: Would like some sort of compiletime error when T is quadword or floating point
    template <unsigned SIZE, typename T>
    void X64CodeGenerator::Group1(unsigned __int8 baseOpCode,
                                  unsigned __int8 extensionOpCode,
                                  Register<SIZE, false> dest,
                                  T value)
    {
        unsigned valueSize = Size(value);

        if (dest.GetId() == 0 && SIZE == 1)
        {
            // Special case for AL.
            Assert(valueSize == 1, "AL requires one-byte value.");
            Emit8(baseOpCode + 0x04);
            Emit8(static_cast<unsigned __int8>(value));
        }
        else if (dest.GetId() == 0 && valueSize != 1)
        {
            if (SIZE == 2)
            {
                Emit8(0x66);        // Size override prefix.
            }
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
            if (SIZE == 2)
            {
                Emit8(0x66);        // Size override prefix.
            }
            // TODO: BUGBUG: This code does not handle 8-bit registers correctly. e.g. AND BH, 5
            EmitRex(dest);

            if (valueSize == 1)
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
                // TODO: Template should be disabled for this size to avoid runtime error.
                throw 0;
            }
        }
    }


    //
    // X64 opcode encoding - REX and Mod/RM
    //

    // TODO: Is W bit always determined by SIZE1? Is there any case where SIZE2 should specify the data size?
    // Do we need a separate ptemplate arameter for the data size?
    template <unsigned SIZE1, unsigned SIZE2>
    void X64CodeGenerator::EmitRex(Register<SIZE1, false> reg, Register<SIZE2, false> rm)
    {
        // WRXB
        // TODO: add cases for W and X bits.

        unsigned regId = reg.GetId();
        unsigned rmId = rm.GetId();

        if (regId > 7 || rmId > 7 || SIZE1 == 8)
        {
            Emit8(0x40 | ((SIZE1 == 8) ? 8 : 0) | ((regId > 7) ? 4 : 0) | ((rmId > 7) ? 1 : 0));
        }
    }


    template <unsigned SIZE>
    void X64CodeGenerator::EmitRex(Register<SIZE, false> dest)
    {
        EmitRex(Register<SIZE, false>(0), dest);
    }


    template <unsigned SIZE>
    void X64CodeGenerator::EmitModRM(Register<SIZE, false> dest, Register<SIZE, false> src)
    {
        Emit8(0xc0 | ((dest.GetId() & 7) << 3) | (src.GetId() & 7));
        // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
        // this function is only used for Register-Register encoding. Problem will 
        // crop up if caller passes the base register from an X64Indirect.
    }


    template <unsigned SIZE>
    void X64CodeGenerator::EmitModRM(unsigned __int8 extensionOpCode, Register<SIZE, false> dest)
    {
        Emit8(0xc0 | (extensionOpCode << 3) | (dest.GetId() & 7));
        // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
        // this function is only used for Register-Register encoding. Problem will 
        // crop up if caller passes the base register from an X64Indirect.
    }


    template <unsigned SIZE>
    void X64CodeGenerator::EmitModRMOffset(Register<SIZE, false> dest, Register<8, false> src, __int32 srcOffset)
    {
        unsigned offsetSize = Size(srcOffset);

        unsigned __int8 mod;

        if ((src.GetId() & 0x7) == 5)
        {
            mod = (offsetSize <= 1) ? 0x40 : 0x80;
        }
        else
        {
            mod = (offsetSize == 0) ? 0 : ((offsetSize == 1) ? 0x40 : 0x80);
        }

        Emit8(mod | ((dest.GetId() & 7) << 3) | (src.GetId() & 7));
        // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
        // this function is only used for Register-Register encoding. Problem will 
        // crop up if caller passes the base register from an X64Indirect.

        // Special case for RSP, R12.
        if ((src.GetId() & 0x7) == 4)
        {
            // Special case for RSP, R12.
            Emit8(0x24);
        }

        if (mod == 0x0 && ((src.GetId() & 0x7) == 4) && ((dest.GetId() & 0x7) == 5) && (srcOffset != 0))
        {
            Emit32(srcOffset);
        }
        else if (mod == 0x40)
        {
            Emit8(static_cast<unsigned __int8>(srcOffset));
        }
        else if (mod == 0x80)
        {
            Emit32(srcOffset);
        }
    }


    //*************************************************************************
    //
    // X64CodeGenerator::Helper definitions for each opcode and addressing mode.
    //
    //*************************************************************************

    //
    // IMul
    //

    //template <>
    //template <unsigned SIZE, bool ISFLOAT>
    //void X64CodeGenerator::Helper<OpCode::IMul>::Emit(X64CodeGenerator& code,
    //                                                  Register<SIZE, ISFLOAT> dest,
    //                                                  Register<SIZE, ISFLOAT> src)
    //{
    //    code.Imul(dest, src);
    //}


    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Lea>::Emit(X64CodeGenerator& code,
                                                     Register<SIZE, ISFLOAT> dest,
                                                     Register<8, false> src,
                                                     __int32 srcOffset)
    {
        code.Lea(dest, src, srcOffset);
    }


    //
    // Mov
    //

    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Mov>::Emit(X64CodeGenerator& code,
                                                     Register<SIZE, ISFLOAT> dest,
                                                     Register<SIZE, ISFLOAT> src)
    {
        code.Group1(0x88, dest, src);
    }


    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Mov>::Emit(X64CodeGenerator& code,
                                                     Register<SIZE, ISFLOAT> dest,
                                                     Register<8, false> src,
                                                     __int32 srcOffset)
    {
        code.Group1(0x88, dest, src, srcOffset);
    }


    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Mov>::Emit(X64CodeGenerator& code,
                                                     Register<8, false> dest,
                                                     __int32 destOffset,
                                                     Register<SIZE, ISFLOAT> src)
    {
        code.Group1(0x86, dest, destOffset, src);
    }


    template <>
    template <unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::Helper<OpCode::Mov>::Emit(X64CodeGenerator& code,
                                                     Register<SIZE, ISFLOAT> dest,
                                                     T value)
    {
        code.Mov(dest, value);
    }


    //
    // Mul
    //

    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::IMul>::Emit(X64CodeGenerator& code,
                                                      Register<SIZE, ISFLOAT> dest,
                                                      Register<SIZE, ISFLOAT> src)
    {
        code.IMul(dest, src);
    }


    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::IMul>::Emit(X64CodeGenerator& code,
                                                      Register<SIZE, ISFLOAT> dest,
                                                      Register<8, false> src,
                                                      __int32 srcOffset)
    {
        code.IMul(dest, src, srcOffset);
    }


    template <>
    template <unsigned SIZE, bool ISFLOAT, typename T>
    void X64CodeGenerator::Helper<OpCode::IMul>::Emit(X64CodeGenerator& code,
                                                      Register<SIZE, ISFLOAT> dest,
                                                      T value)
    {
        code.IMul(dest, value);
    }


    //
    // Pop/Push
    //

    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Pop>::Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest)
    {
        code.Pop(dest);
    }


    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Push>::Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest)
    {
        code.Push(dest);
    }


#define DEFINE_GROUP1(name, baseOpCode, extensionOpCode) \
    template <>                                                                          \
    template <unsigned SIZE, bool ISFLOAT>                                               \
    void X64CodeGenerator::Helper<OpCode::##name##>::Emit(X64CodeGenerator& code,        \
                                                          Register<SIZE, ISFLOAT> dest,  \
                                                          Register<SIZE, ISFLOAT> src)   \
    {                                                                                    \
        code.Group1(baseOpCode, dest, src);                                              \
    }                                                                                    \
                                                                                         \
                                                                                         \
    template <>                                                                          \
    template <unsigned SIZE, bool ISFLOAT>                                               \
    void X64CodeGenerator::Helper<OpCode::##name##>::Emit(X64CodeGenerator& code,        \
                                                     Register<SIZE, ISFLOAT> dest,       \
                                                     Register<8, false> src,             \
                                                     __int32 srcOffset)                  \
    {                                                                                    \
        code.Group1(baseOpCode, dest, src, srcOffset);                                   \
    }                                                                                    \
                                                                                         \
                                                                                         \
    template <>                                                                          \
    template <unsigned SIZE, bool ISFLOAT, typename T>                                   \
    void X64CodeGenerator::Helper<OpCode::##name##>::Emit(X64CodeGenerator& code,        \
                                                     Register<SIZE, ISFLOAT> dest,       \
                                                     T value)                            \
    {                                                                                    \
        code.Group1(baseOpCode, extensionOpCode, dest, value);                           \
    }                                                                                    

    DEFINE_GROUP1(Add, 0, 0);
    DEFINE_GROUP1(And, 0x20, 4);
    DEFINE_GROUP1(Or, 8, 1);
    DEFINE_GROUP1(Sub, 0x28, 5);
    DEFINE_GROUP1(Cmp, 0x38, 7);

#undef DEFINE_GROUP1
}

#pragma warning(pop)
