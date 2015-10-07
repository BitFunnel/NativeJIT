#include "stdafx.h"

#include <iostream>             // TODO: Remove this temp debug include.

#include "Assert.h"
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


    void ExpressionTree::AddRIPRelative(RIPRelativeImmediate& node)
    {
        m_ripRelatives.push_back(&node);
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
        std::cout << "=== Pass0 ===" << std::endl;

        // Emit RIP-relative constants.
        for (unsigned i = 0 ; i < m_ripRelatives.size(); ++i)
        {
            m_ripRelatives[i]->EmitStaticData(*this);
        }
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
        for (unsigned i = 0 ; i <= RegisterBase::c_maxIntegerRegisterID; ++i)
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
