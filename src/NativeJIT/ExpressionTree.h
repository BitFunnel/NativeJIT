#pragma once

#include <vector>

#include "NativeJIT/FunctionBuffer.h"
#include "NativeJIT/Register.h"
#include "Temporary/NonCopyable.h"


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    class NodeBase;
    class ParameterBase;
//    class X64CodeGenerator;


    class RegisterFile
    {
    public:
        RegisterFile(unsigned rxxCount, unsigned xmmCount);

        template <unsigned SIZE>
        void ReserveRegister(Register<SIZE, false> r);

        template <unsigned SIZE>
        void ReserveRegister(Register<SIZE, true> r);

        unsigned GetRXXRegisterCount() const;
        unsigned GetXMMRegisterCount() const;

        unsigned GetReservedRXX() const;
        unsigned GetReservedXMM() const;

    private:
        unsigned m_rxxCount;
        unsigned m_xmmCount;
        unsigned m_reservedRXX;
        unsigned m_reservedXMM;
    };


    class ExpressionTree : public NonCopyable
    {
    public:
        ExpressionTree(Allocators::IAllocator& allocator, FunctionBuffer& code);

        Allocators::IAllocator& GetAllocator() const;

        unsigned AddNode(NodeBase& node);
        unsigned AddParameter(ParameterBase& parameter);

        template <typename REGISTERTYPE>
        REGISTERTYPE AllocateRegister();

        template <unsigned SIZE>
        void Prefer(Register<SIZE, false> r);

        template <unsigned SIZE>
        void Prefer(Register<SIZE, true> r);

        template <typename REGISTERTYPE>
        unsigned GetAvailableRegisterCount() const;

        template <unsigned SIZE, bool ISFLOAT>
        Register<SIZE, ISFLOAT> CopyRegister(Register<SIZE, ISFLOAT> r);

        template <unsigned SIZE>
        void ReleaseRegister(Register<SIZE, false> r);
        template <unsigned SIZE>
        void ReleaseRegister(Register<SIZE, true> r);

        RegisterFile& GetParameterRegisters();

        FunctionBuffer& GetCodeGenerator() const;

        template <unsigned SIZE, bool ISFLOAT>
        bool IsBasePointer(Register<SIZE, ISFLOAT> r) const;

        Register<sizeof(void*), false> GetBasePointer() const;
        __int32 AllocateTemporary();
        void ReleaseTemporary(__int32 offset);

        void Print() const;
        void Compile();

    protected:
        void  const * GetUntypedEntryPoint() const;

        Allocators::IAllocator& m_allocator;
        FunctionBuffer & m_code;


    private:
        void SetBasePointer(Register<sizeof(void*), false> r);

        // Assigns registers to parameters.
        void Pass1();

        // Generates code that evaluates common subexpressions into registers.
        void Pass2();

        // Generates code to evaluate the entire tree.
        void Pass3();

        template <unsigned SIZE>
        Register<SIZE, false> AllocateRegisterInternal(Register<SIZE, false> ignore);
        template <unsigned SIZE>
        Register<SIZE, true> AllocateRegisterInternal(Register<SIZE, true> ignore);

        template <unsigned SIZE>
        unsigned GetAvailableRegisterCountInternal(Register<SIZE, false> ignore) const;
        template <unsigned SIZE>
        unsigned GetAvailableRegisterCountInternal(Register<SIZE, true> ignore) const;


        std::vector<NodeBase*> m_topologicalSort;

        std::vector<ParameterBase*> m_parameters;
        RegisterFile m_parameterRegisters;

        std::vector<unsigned> m_rxxRegisters;
        std::vector<unsigned> m_xmmRegisters;

        unsigned m_rxxRegistersAvailable;
        unsigned m_xmmRegistersAvailable;


        Register<8, false> m_stackPointer;
        Register<8, false> m_basePointer;

        unsigned m_temporaryCount;
        std::vector<__int32> m_temporaries;
    };


    //*************************************************************************
    //
    // Template definitions for RegisterFile.
    //
    //*************************************************************************
    template <unsigned SIZE>
    void RegisterFile::ReserveRegister(Register<SIZE, false> r)
    {
        m_reservedRXX &= ~(1 << r.GetId());
    }


    template <unsigned SIZE>
    void RegisterFile::ReserveRegister(Register<SIZE, true> r)
    {
        m_reservedXMM &= ~(1 << r.GetId());
    }


    //*************************************************************************
    //
    // Template definitions for ExpressionTree.
    //
    //*************************************************************************
    template <typename REGISTERTYPE>
    REGISTERTYPE ExpressionTree::AllocateRegister()
    {
        return AllocateRegisterInternal(REGISTERTYPE());
    }


    template <unsigned SIZE>
    Register<SIZE, false> ExpressionTree::AllocateRegisterInternal(Register<SIZE, false> /*ignore*/)
    {
        Assert(m_rxxRegisters.size() > 0, "No RXX registers available.");

        unsigned id = m_rxxRegisters.back();
        m_rxxRegisters.pop_back();
        m_rxxRegistersAvailable &= ~(1 << id);
        return Register<SIZE, false>(id);
    }


    template <unsigned SIZE>
    Register<SIZE, true> ExpressionTree::AllocateRegisterInternal(Register<SIZE, true> /*ignore*/)
    {
        Assert(m_xmmRegisters.size() > 0, "No XMM registers available.");

        unsigned id = m_xmmRegisters.back();
        m_xmmRegisters.pop_back();
        m_xmmRegistersAvailable &= ~(1 << id);
        return Register<SIZE, true>(id);
    }


    template <unsigned SIZE>
    void ExpressionTree::Prefer(Register<SIZE, false> r)
    {
        unsigned id = r.GetId();
        if ((m_rxxRegistersAvailable & (1 << id)) != 0)
        {
            // The preferred register is available.
            // Move it to the head of the free list.
            unsigned last = static_cast<unsigned>(m_rxxRegisters.size() - 1);
            for (unsigned i = 0 ; i < last; ++i)
            {
                if (m_rxxRegisters[i] == id)
                {
                    // We've found the preferred register in a position before the last.
                    // Swap it with the last register.
                    m_rxxRegisters[i] = m_rxxRegisters[last];
                    m_rxxRegisters[last] = id;
                    break;
                }
            }
        }
    }


    template <unsigned SIZE>
    void ExpressionTree::Prefer(Register<SIZE, true> r)
    {
        // TODO: Implement
    }


    template <typename REGISTERTYPE>
    unsigned ExpressionTree::GetAvailableRegisterCount() const
    {
        return GetAvailableRegisterCountInternal(REGISTERTYPE());
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::GetAvailableRegisterCountInternal(Register<SIZE, false> /*ignore*/) const
    {
        return static_cast<unsigned>(m_rxxRegisters.size());
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::GetAvailableRegisterCountInternal(Register<SIZE, true> /*ignore*/) const
    {
        return static_cast<unsigned>(m_xmmRegisters.size());
    }


    template <unsigned SIZE, bool ISFLOAT>
    Register<SIZE, ISFLOAT> ExpressionTree::CopyRegister(Register<SIZE, ISFLOAT> src)
    {
        auto dest = AllocateRegister<Register<SIZE, ISFLOAT>>();
        m_code.Emit<OpCode::Mov>(dest, src);
        return dest;
    }


    template <unsigned SIZE>
    void ExpressionTree::ReleaseRegister(Register<SIZE, false> r)
    {
//        std::cout << "// release " << r.GetName() << std::endl;

        // TODO: Verify that this register isn't already free.

        m_rxxRegisters.push_back(r.GetId());
        m_rxxRegistersAvailable |= (1 << r.GetId());
    }


    template <unsigned SIZE>
    void ExpressionTree::ReleaseRegister(Register<SIZE, true> r)
    {
//        std::cout << "// release " << r.GetName() << std::endl;

        // TODO: Verify that this register isn't already free.

        m_xmmRegisters.push_back(r.GetId());
        m_xmmRegistersAvailable |= (1 << r.GetId());
    }


    template <unsigned SIZE, bool ISFLOAT>
    bool ExpressionTree::IsBasePointer(Register<SIZE, ISFLOAT> r) const
    {
        return !ISFLOAT && SIZE == sizeof(void*) && r.GetId() == m_basePointer.GetId();
    }
}
