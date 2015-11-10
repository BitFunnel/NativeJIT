#include "stdafx.h"

#include <iostream>             // TODO: Remove this temp debug include.

#include "Assert.h"
#include "ExecutionPreconditionTest.h"
#include "ExpressionTree.h"
#include "ImmediateNode.h"
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
          m_basePointer(rbp)
          // m_startOfEpilogue intentionally left uninitialized, see Compile().
    {
        m_reservedRegistersStorages.reserve(RegisterBase::c_maxIntegerRegisterID + 1);
        m_reservedRegistersPins.reserve(RegisterBase::c_maxIntegerRegisterID + 1);

        for (unsigned i = 0 ; i <= RegisterBase::c_maxIntegerRegisterID; ++i)
        {
            const PointerRegister reg(i);

            if (IsAnySharedBaseRegister(reg))
            {
                auto s = Direct<void*>(reg);

                m_reservedRegistersStorages.push_back(s);
                m_reservedRegistersPins.push_back(s.GetPin());
            }
        }

        for (unsigned i = 0 ; i <= RegisterBase::c_maxFloatRegisterID; ++i)
        {
            Assert(!IsAnySharedBaseRegister(Register<8, true>(i)),
                   "Unexpected reserved FP register %u",
                   i);
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


    void ExpressionTree::ReleaseTemporary(__int32 offset)
    {
        m_temporaries.push_back(offset / sizeof(void*));
    }


    unsigned ExpressionTree::GetRXXUsageMask() const
    {
        return m_rxxFreeList.GetUsedMask();
    }


    unsigned ExpressionTree::GetXMMUsageMask() const
    {
        return m_xmmFreeList.GetUsedMask();
    }


    bool ExpressionTree::IsBasePointer(PointerRegister r) const
    {
        return r.GetId() == m_basePointer.GetId();
    }


    PointerRegister ExpressionTree::GetBasePointer() const
    {
        return m_basePointer;
    }

    Label ExpressionTree::GetStartOfEpilogue() const
    {
        return m_startOfEpilogue;
    }


    unsigned ExpressionTree::AddNode(NodeBase& node)
    {
        m_topologicalSort.push_back(&node);
        return static_cast<unsigned>(m_topologicalSort.size() - 1);
    }


    void ExpressionTree::AddParameter(NodeBase& parameter)
    {
        m_parameters.push_back(&parameter);
    }


    void ExpressionTree::AddRIPRelative(RIPRelativeImmediate& node)
    {
        m_ripRelatives.push_back(&node);
    }


    void ExpressionTree::AddExecutionPreconditionTest(ExecutionPreconditionTest& test)
    {
        m_preconditionTests.push_back(&test);
    }


    void ExpressionTree::Compile()
    {
        // Note: the call to Reset() clears all allocated labels, so start of
        // epilogue label must be allocated after that point.
        m_code.Reset();
        m_startOfEpilogue = m_code.AllocateLabel();

        // Generate constants.
        Pass0();

        // Generate code.
        m_code.EmitPrologue();

        Pass1();
        Pass2();
        Print();
        Pass3();

        m_code.PlaceLabel(m_startOfEpilogue);
        m_code.EmitEpilogue();

        m_code.PatchCallSites();

        // Release the reserved registers.
        m_reservedRegistersPins.clear();
        m_reservedRegistersStorages.clear();
        Print();

        Assert(GetRXXUsageMask() == 0,
               "Some integer registers have not been released: 0x%x",
               GetRXXUsageMask());

        Assert(GetXMMUsageMask() == 0,
               "Some floating point registers have not been released: 0x%x",
               GetXMMUsageMask());
    }


    void  const * ExpressionTree::GetUntypedEntryPoint() const
    {
        return m_code.GetEntryPoint();
    }


    void ExpressionTree::Pass0()
    {
        std::cout << "=== Pass0 ===" << std::endl;

        // Emit RIP-relative constants.
        for (unsigned i = 0 ; i < m_ripRelatives.size(); ++i)
        {
            m_ripRelatives[i]->EmitStaticData(*this);
        }

        // Walk the nodes in reverse order of creation (i.e. in potential order
        // of execution) to see whether they can be optimized away.
        //
        // Note: currently it is not important whether the optimize-away pass
        // traverses the nodes in order or in reverse order. This is because the
        // only existing optimization technique (base pointer collapsing) is
        // performed when node is constructed and removing references from
        // children in such case will not create an opportunity for additional
        // optimizations.
        for (auto nodeIt = m_topologicalSort.rbegin();
             nodeIt != m_topologicalSort.rend();
             ++nodeIt)
        {
            auto node = *nodeIt;

            if (!node->IsInsideTree())
            {
                Print();
                Assert(node->IsInsideTree(),
                       "Node with ID %u has been created but not placed inside the tree",
                       node->GetId());
            }

            if (node->CanBeOptimizedAway())
            {
                node->ReleaseReferencesToChildren();
            }
        }
    }


    void ExpressionTree::Pass1()
    {
        std::cout << "=== Pass1 ===" << std::endl;

        // Reserve registers used to pass in parameters.
        for (const auto param : m_parameters)
        {
            param->CodeGenCache(*this);
        }

        // Execute any return-early tests before compiling the expression further.
        for (auto test : m_preconditionTests)
        {
            test->Evaluate(*this);
        }
    }


    void ExpressionTree::Pass2()
    {
        std::cout << "=== Pass2 ===" << std::endl;

        for (unsigned i = 0 ; i < m_topologicalSort.size(); ++i)
        {
            NodeBase& node = *m_topologicalSort[i];

            if (node.GetParentCount() > 1 && !node.IsEvaluated())
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
            m_parameters[i]->Print();
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
        for (unsigned i = 0 ; i <= RegisterBase::c_maxIntegerRegisterID; ++i)
        {
            std::cout << Register<8, false>(i).GetName()
                      << (m_rxxFreeList.IsAvailable(i) ? " free" : " in use")
                      << std::endl;
        }

        std::cout << std::endl;

        std::cout << "XMM Registers:" << std::endl;
        for (unsigned i = 0 ; i <= RegisterBase::c_maxFloatRegisterID; ++i)
        {
            std::cout << Register<8, true>(i).GetName()
                      << (m_xmmFreeList.IsAvailable(i) ? " free" : " in use")
                      << std::endl;
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
    ExpressionTree::Data::Data(ExpressionTree& tree,
                               PointerRegister base,
                               __int32 offset)
        : m_tree(tree),
          m_storageClass(StorageClass::Indirect),
          m_isFloat(base.c_isFloat),
          m_registerId(base.GetId()),
          m_offset(offset),
          m_refCount(0)
    {
        NotifyDataRegisterChange(RegisterChangeType::Initialize);
    }


    ExpressionTree& ExpressionTree::Data::GetTree() const
    {
        return m_tree;
    }


    StorageClass ExpressionTree::Data::GetStorageClass() const
    {
        return m_storageClass;
    }


    unsigned ExpressionTree::Data::GetRegisterId() const
    {
        return m_registerId;
    }


    __int32 ExpressionTree::Data::GetOffset() const
    {
        Assert(m_storageClass == StorageClass::Indirect, "StorageClass must be Indirect, found %u", m_storageClass);
        return m_offset;
    }


    void ExpressionTree::Data::ConvertDirectToIndirect(__int32 offset)
    {
        Assert(m_storageClass == StorageClass::Direct,
               "StorageClass must be Direct, found %u",
               m_storageClass);
        Assert(!IsSharedBaseRegister(),
               "Cannot change type of shared register %u from direct to indirect",
               m_registerId);

        m_storageClass = StorageClass::Indirect;
        m_offset = offset;
    }


    void ExpressionTree::Data::ConvertIndirectToDirect()
    {
        Assert(m_storageClass == StorageClass::Indirect,
               "StorageClass must be Indirect, found %u",
               m_storageClass);
        Assert(!IsSharedBaseRegister(),
               "Cannot change type of shared register %u from indirect to direct",
               m_registerId);

        m_storageClass = StorageClass::Direct;
        m_offset = 0;
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


    void ExpressionTree::Data::SwapContents(Data* other)
    {
        std::swap(m_storageClass, other->m_storageClass);
        std::swap(m_isFloat, other->m_isFloat);
        std::swap(m_registerId, other->m_registerId);
        std::swap(m_offset, other->m_offset);

        std::swap(m_immediate, other->m_immediate);

        // Both Data objects keep their m_refCount.

        NotifyDataRegisterChange(RegisterChangeType::Update);
        other->NotifyDataRegisterChange(RegisterChangeType::Update);
    }


    void ExpressionTree::Data::NotifyDataRegisterChange(RegisterChangeType type)
    {
        if (m_storageClass != StorageClass::Immediate)
        {
            if (m_isFloat)
            {
                NotifyDataRegisterChange<true>(type);
            }
            else
            {
                NotifyDataRegisterChange<false>(type);
            }
        }
    }


    bool ExpressionTree::Data::IsSharedBaseRegister() const
    {
        return m_storageClass != StorageClass::Immediate
            && !m_isFloat
            && m_tree.IsAnySharedBaseRegister(PointerRegister(m_registerId));
    }


    ReferenceCounter::ReferenceCounter()
        : m_counter(nullptr)
    {
        // No-op, for symmetry only.
        AddReference();
    }


    ReferenceCounter::ReferenceCounter(unsigned& counter)
        : m_counter(&counter)
    {
        AddReference();
    }


    ReferenceCounter::ReferenceCounter(ReferenceCounter const & other)
        : ReferenceCounter(*other.m_counter)
    {
    }


    ReferenceCounter::~ReferenceCounter()
    {
        RemoveReference();
    }


    ReferenceCounter& ReferenceCounter::operator=(ReferenceCounter const & other)
    {
        if (m_counter != other.m_counter)
        {
            RemoveReference();

            m_counter = other.m_counter;
            AddReference();
        }

        return *this;
    }


    void ReferenceCounter::Reset()
    {
        RemoveReference();
        m_counter = nullptr;
    }


    void ReferenceCounter::AddReference()
    {
        if (m_counter != nullptr)
        {
            ++(*m_counter);
        }
    }


    void ReferenceCounter::RemoveReference()
    {
        if (m_counter != nullptr)
        {
            if (*m_counter == 0)
            {
                throw std::logic_error("Tried to remove a reference with zero reference count");
            }

            --(*m_counter);
        }
    }
}
