#include "stdafx.h"

#include <iostream>             // TODO: Remove this temp debug include.

#include "Assert.h"
#include "ExpressionTree.h"
#include "NativeJIT/FunctionBuffer.h"
#include "ParameterNode.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // ExpressionTree
    //
    //*************************************************************************
    ExpressionTree::ExpressionTree(Allocators::IAllocator& allocator, FunctionBuffer& code)
        : m_allocator(allocator),
          m_code(code),
          m_temporaryCount(0),
          m_basePointer(Register<sizeof(void*), false>(4))     // TODO: Magic number RBP
    {

        // TODO: Free up correct registers (e.g. not RSP, RBP. Also reserve RAX?)
        // TODO: Free up xmm registers.
        // TODO: Deal with parameters.
        for (unsigned i = 0 ; i < c_rxxCount; ++i)
        {
            m_rxxFreeList.Release(i);
        }

        for (unsigned i = 0 ; i < c_xmmCount; ++i)
        {
            m_xmmFreeList.Release(i);
        }
    }


    Allocators::IAllocator& ExpressionTree::GetAllocator() const
    {
        return m_allocator;
    }


    FunctionBuffer& ExpressionTree::GetCodeGenerator() const
    {
        return m_code;
    }


    //Register<sizeof(void*), false> ExpressionTree::GetBasePointer() const
    //{
    //    return m_basePointer;
    //}


    void ExpressionTree::ReleaseTemporary(__int32 offset)
    {
        m_temporaries.push_back(offset / sizeof(void*));
    }


    unsigned ExpressionTree::GetRXXUsageMask() const
    {
        return m_rxxFreeList.GetUsageMask();
    }


    bool ExpressionTree::IsBasePointer(Register<8, false> r) const
    {
        return r.GetId() == m_basePointer.GetId();
    }


    Register<sizeof(void*), false> ExpressionTree::GetBasePointer() const
    {
        return m_basePointer;
    }


    unsigned ExpressionTree::AddNode(NodeBase& node)
    {
        m_topologicalSort.push_back(&node);
        return static_cast<unsigned>(m_topologicalSort.size() - 1);
    }


    unsigned ExpressionTree::AddParameter(ParameterBase& parameter)
    {
        m_parameters.push_back(&parameter);
        return static_cast<unsigned>(m_parameters.size() - 1);
    }


    void ExpressionTree::Compile()
    {
        m_code.Reset();

        // Generate constants.
        Pass0();

        // Generate code.
        m_code.EmitPrologue();
        Pass1();
        Pass2();
        Print();
        Pass3();
        m_code.PatchCallSites();

        Print();
    }


    void  const * ExpressionTree::GetUntypedEntryPoint() const
    {
        return m_code.GetEntryPoint();
    }


    void ExpressionTree::Pass0()
    {
        // TODO: Emit RIP-relative constants.
    }


    void ExpressionTree::Pass1()
    {
        std::cout << "=== Pass1 ===" << std::endl;

        // Reserve registers used to pass in parameters.
        for (unsigned i = 0 ; i < m_parameters.size(); ++i)
        {
            m_parameters[i]->ReserveRegister(*this);
        }
    }


    //
    // TODO: NOTE: With the addition of conditional nodes, the common
    // subexpression (CSE) code is now inefficient because some CSEs
    // may not need to be evaluated. Consider, for example
    //   (v == 1)? a : ((v == 2) ? a + b : b + c)
    // Depending on the value of v, either a or c may not need to be
    // evaluated. Should probably lazily evaluate CSEs as they are needed.
    //
    void ExpressionTree::Pass2()
    {
        std::cout << "=== Pass2 ===" << std::endl;

        for (unsigned i = 0 ; i < m_topologicalSort.size(); ++i)
        {
            NodeBase& node = *m_topologicalSort[i];

            // Assert(parentCount of zero legal only for last node);

            if (node.GetParentCount() > 1 && !node.IsCached())
            {
                node.CodeGenCache(*this);
            }
        }
    }


    void ExpressionTree::Pass3()
    {
        std::cout << "=== Pass3 ===" << std::endl;

        NodeBase& root = *m_topologicalSort.back();

        // Assert(Root node is return, "Root must be a return node.");

        root.LabelSubtree(true);
        root.CompileAsRoot(*this);
    }


    void ExpressionTree::Print() const
    {
        std::cout << "Parameters:" << std::endl;
        for (unsigned i = 0 ; i < m_parameters.size(); ++i)
        {
            m_parameters[i]->PrintParameter();
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Topological sort:" << std::endl;
        for (unsigned i = 0 ; i < m_topologicalSort.size(); ++i)
        {
            m_topologicalSort[i]->Print();
            std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "RXX Registers:" << std::endl;
        for (unsigned i = 0 ; i < c_rxxCount;  ++i)
        {
            std::cout << Register<8, false>(i).GetName();
            if (m_rxxFreeList.IsAvailable(i))
            {
                std::cout << " free";
            }
            else
            {
                std::cout << " in use";
            }
            std::cout << std::endl;
        }

        std::cout << "Temporaries used: " << m_temporaryCount << std::endl;
        std::cout << "Temporaries still in use: " << m_temporaryCount - m_temporaries.size() << std::endl;

        std::cout << std::endl;
    }


    //*************************************************************************
    //
    // ExpressionTree::Data
    //
    //*************************************************************************
    ExpressionTree& ExpressionTree::Data::GetTree() const
    {
        return m_tree;
    }


    StorageClass ExpressionTree::Data::GetStorageClass() const
    {
        return m_storageClass;
    }


    void ExpressionTree::Data::SetStorageClass(StorageClass storageClass)
    {
        m_storageClass = storageClass;
    }


    unsigned ExpressionTree::Data::GetRegisterId() const
    {
        return m_registerId;
    }


    void ExpressionTree::Data::SetRegisterId(unsigned id)
    {
        m_registerId = id;
    }


    __int32 ExpressionTree::Data::GetOffset() const
    {
        Assert(m_storageClass == StorageClass::Indirect, "StorageClass must be Indirect.");
        return m_offset;
    }


    void ExpressionTree::Data::ConvertToIndirect(__int32 offset)
    {
        Assert(m_storageClass == StorageClass::Direct, "StorageClass must be Direct.");
        m_storageClass = StorageClass::Indirect;
        m_offset = offset;
    }


    unsigned ExpressionTree::Data::GetRefCount() const
    {
        return m_refCount;
    }


    unsigned ExpressionTree::Data::Decrement()
    {
         Assert(m_refCount > 0, "Attempting to decrement zero ref count.");
        --m_refCount;
//        std::cout << "Decrement " << this << " to " << m_refCount << std::endl;
        return m_refCount;
    }


    void ExpressionTree::Data::Increment()
    {
        ++m_refCount;
//        std::cout << "Increment " << this << " to " << m_refCount << std::endl;
    }
}
