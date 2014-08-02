#pragma once

// TODO: No way to do an operation on sign-extended data. Need to load data first, then sign-extend.


#include <ostream>

#include "NativeJIT/CodeBuffer.h"       // Inherits from CodeBuffer.
#include "NativeJIT/Register.h"         // Register parameter.
#include "Temporary/NonCopyable.h"      // Inherits from NonCopyable.


// Supress warning about constant expression involving template parameters.
#pragma warning(push)
#pragma warning(disable:4127)

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


    // WARNING: When modifying OpCode, be sure to also modify the function OpCodeName().
    enum class OpCode : unsigned
    {
        Add,
        Call,
        Cmp,
        Lea,
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


        // TODO: Remove this temporary override of CodeBuffer::PlaceLabel().
        // This version is used to print debugging information.
        void PlaceLabel(Label l);


        void Jmp(Label l);
        void Jcc(JccType type, Label l);

        // TODO: Should this be generalized? (e.g. just an Op() overload).
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

        // Intel manual section 2.2.1.3 says x64 displacements are either 8 or 32 bits.
        template <unsigned SIZE, bool ISFLOAT>
        void Op(OpCode op,
                Register<SIZE, ISFLOAT> dest,
                Register<sizeof(void*), false> base,
                __int32 offset);



        // These two methods are public in order to allow access for BinaryNode debugging text.
        static char const * OpCodeName(OpCode op);
        static char const * JccName(JccType jcc);


        template <OpCode OP>
        class Helper;

        template <OpCode OP>
        void Emit()
        {
            Helper<OP>::Emit(*this);
        }


        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest)
        {
            Helper<OP>::Emit(*this, dest);
        }


        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest, Register<SIZE, ISFLOAT> src)
        {
            Helper<OP>::Emit(*this, dest, src);
        }


        template <OpCode OP, unsigned SIZE, bool ISFLOAT>
        void Emit(Register<SIZE, ISFLOAT> dest, Register<8, false> src, unsigned __int32 srcOffset)
        {
            Helper<OP>::Emit(*this, dest, src, srcOffset);
        }


        template <OpCode OP, unsigned SIZE, bool ISFLOAT, typename T>
        void Emit(Register<SIZE, ISFLOAT> dest, T value)
        {
            Helper<OP>::Emit(*this, dest, value);
        }

    private:
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

            template <unsigned SIZE, bool ISFLOAT, typename T>
            static void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> dest, T value);
        };


        template <unsigned SIZE>
        void Lea(Register<SIZE, false> dest,
                 Register<8, false> src,
                 __int32 srcOffset)
        {
            EmitRex(dest, src);
            Emit8(0x8d);
            EmitModRMOffset(dest, src, srcOffset);
        }


        void Pop(Register<8, false> r)
        {
            if (r.GetId() > 7)
            {
                Emit8(0x41);  // TODO: Should be able to use EmitREX() here except it sets W.
            }
            Emit8(0x58 + (r.GetId() & 7));
        }


        void Push(Register<8, false> r)
        {
            if (r.GetId() > 7)
            {
                Emit8(0x41);  // TODO: Should be able to use EmitREX() here except it sets W.
            }
            Emit8(0x50 + (r.GetId() & 7));
        }


        void Ret()
        {
            Emit8(0xc3);
        }


        template <unsigned SIZE>
        void Group1(unsigned __int8 baseOpCode,
                    Register<SIZE, false> dest,
                    Register<SIZE, false> src)
        {
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
        void Group1(unsigned __int8 baseOpCode,
                    Register<SIZE, false> dest,
                    Register<SIZE, false> src,
                    __int32 srcOffset)
        {
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




        template <unsigned SIZE, typename T>
        void Group1(unsigned __int8 baseOpCode,
                    unsigned __int8 extensionOpCode,
                    Register<SIZE, false> dest,
                    T value)
        {
            bool isByteValue = (value >= -128 && value <= 127);

            if (SIZE == 1 && dest.GetId() == 0)
            {
                // Special case for AL.
                Emit8(baseOpCode + 0x04);
                Emit8(static_cast<unsigned __int8>(value));
            }
            else if (SIZE == 8 && dest.GetId() == 0 && !isByteValue)
            {
                // Special case for RAX.
                Emit8(0x48);
                Emit8(baseOpCode + 0x05);
                Emit32(value);
            }
            else
            {
                // TODO: BUGBUG: This code does not handle 8-bit registers correctly. e.g. AND BH, 5
                EmitRex(dest);

                if (isByteValue)
                {
                    Emit8(0x80 + 3);
                    EmitModRM(extensionOpCode, dest);
                    Emit8(static_cast<unsigned __int8>(value));
                }
                else
                {
                    Emit8(0x80 + 1);
                    EmitModRM(extensionOpCode, dest);
                    Emit32(value);
                }
            }
        }


    private:

        template <unsigned SIZE>
        void EmitRex(Register<SIZE, false> dest, Register<SIZE, false> src)
        {
            // WRXB
            // TODO: add cases for W and X bits.

            unsigned d = dest.GetId();
            unsigned s = src.GetId();

            if (d > 7 || s > 7 || SIZE == 8)
            {
                Emit8(0x40 | ((SIZE == 8) ? 8 : 0) | ((d > 7) ? 4 : 0) | ((s > 7) ? 1 : 0));
            }
        }


        template <unsigned SIZE>
        void EmitRex(Register<SIZE, false> dest)
        {
            EmitRex(Register<SIZE, false>(0), dest);
        }


        template <unsigned SIZE>
        void EmitModRM(Register<SIZE, false> dest, Register<SIZE, false> src)
        {
            Emit8(0xc0 | ((dest.GetId() & 7) << 3) | (src.GetId() & 7));
            // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
            // this function is only used for Register-Register encoding. Problem will 
            // crop up if caller passes the base register from an X64Indirect.
        }


        template <unsigned SIZE>
        void EmitModRM(unsigned __int8 extensionOpCode, Register<SIZE, false> dest)
        {
            Emit8(0xc0 | (extensionOpCode << 3) | (dest.GetId() & 7));
            // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
            // this function is only used for Register-Register encoding. Problem will 
            // crop up if caller passes the base register from an X64Indirect.
        }


        template <unsigned SIZE>
        void EmitModRMOffset(Register<SIZE, false> dest, Register<SIZE, false> src, __int32 srcOffset)
        {
            unsigned __int8 mod = (srcOffset <= 127 && srcOffset >= -128)? 0x40 : 0x80;

            Emit8(mod | ((dest.GetId() & 7) << 3) | (src.GetId() & 7));
            // BUGBUG: check special cases for RSP, R12. Shouldn't be necessary here if
            // this function is only used for Register-Register encoding. Problem will 
            // crop up if caller passes the base register from an X64Indirect.

            if (mod == 0x40)
            {
                Emit8(static_cast<unsigned __int8>(srcOffset));
            }
            else
            {
                Emit32(srcOffset);
            }
        }


        void Emit8(unsigned __int8 x)
        {
            m_out.width(2);
            m_out.fill('0');
            m_out << std::hex << static_cast<unsigned>(x) << std::dec << " ";
        }

        void Emit32(__int32 x)
        {
            m_out.width(2);
            m_out.fill('0');
            m_out << std::hex << static_cast<unsigned>(x & 0xff) << " ";
            m_out << std::hex << static_cast<unsigned>((x >> 8) & 0xff) << " ";
            m_out << std::hex << static_cast<unsigned>((x >> 16) & 0xff) << " ";
            m_out << std::hex << static_cast<unsigned>((x >> 24) & 0xff) << " ";
            m_out << std::dec;
        }

        void Indent();

        static const unsigned c_rxxRegisterCount = 16;
        static const unsigned c_xmmRegisterCount = 16;

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
            // TODO: This hack is probably wrong for signed values.
            // TODO: Need to add support for ISSIGNED.
            m_out << OpCodeName(op) << " " << dest.GetName() << ", " << (unsigned __int64)value << std::endl;
        }
    }


    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Op(OpCode op,
                              Register<SIZE, ISFLOAT> dest,
                              Register<sizeof(void*), false> base,
                              __int32 offset)
    {
        Indent();
        if (offset > 0)
        {
            m_out << OpCodeName(op) << " " << dest.GetName() << ", [" << base.GetName() << " + " << offset << "]" << std::endl;
        }
        else if (offset < 0)
        {
            m_out << OpCodeName(op) << " " << dest.GetName() << ", [" << base.GetName() << " - " << -offset << "]" << std::endl;
        }
        else
        {
            m_out << OpCodeName(op) << " " << dest.GetName() << ", [" << base.GetName() << "]" << std::endl;
        }
    }


    //*************************************************************************
    //
    // X64CodeGenerator::Helper definitions for each opcode and addressing mode.
    //
    //*************************************************************************

    template <>
    template <unsigned SIZE, bool ISFLOAT>
    void X64CodeGenerator::Helper<OpCode::Lea>::Emit(X64CodeGenerator& code,
                                                     Register<SIZE, ISFLOAT> dest,
                                                     Register<8, false> src,
                                                     __int32 srcOffset)
    {
        code.Lea(dest, src, srcOffset);
    }


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
    DEFINE_GROUP1(Sub, 0x28, 5);
    DEFINE_GROUP1(Cmp, 0x38, 7);

#undef DEFINE_GROUP1
}

#pragma warning(pop)
