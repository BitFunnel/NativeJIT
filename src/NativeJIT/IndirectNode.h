#pragma once

#include "Node.h"


namespace NativeJIT
{
    // IndirectNode implements the *(base + index) operation when index is known
    // at compile time.
    template <typename T>
    class IndirectNode : public Node<T>
    {
    public:
        IndirectNode(ExpressionTree& tree, Node<T*>& base, __int32 index);

        //
        // Overrides of Node methods.
        //

        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

        // Note: IndirectNode doesn't implement GetBaseAndOffset() method which
        // allows for base object/offset collapsing optimization because it
        // dereferences the target object, preventing continuation of the chain.

    private:
        NodeBase& m_base;
        const __int32 m_index;

        // Multiple accesses to the same base object can sometimes be collapsed
        // as an optimization. In such cases, m_collapsedBase/Offset will point
        // to such base object. Otherwise, they will match the base object/offset
        // from the constructor.
        NodeBase* m_collapsedBase;
        __int32 m_collapsedOffset;
    };


    //*************************************************************************
    //
    // Template definitions for IndirectBase
    //
    //*************************************************************************
    template <typename T>
    IndirectNode<T>::IndirectNode(ExpressionTree& tree, Node<T*>& base, __int32 index)
        : Node(tree),
          m_base(base),
          m_index(index),
          m_collapsedBase(&m_base),
          m_collapsedOffset(sizeof(T) * m_index)
    {
        NodeBase* grandparent;
        __int32 parentOffset;

        // If base can be represented off of another object with an added offset,
        // make the reference off of that object and adjust the offset.
        if (base.GetBaseAndOffset(grandparent, parentOffset))
        {
            m_collapsedBase = grandparent;
            m_collapsedOffset += parentOffset;
            base.MarkInsideTree();
        }

        m_collapsedBase->IncrementParentCount();
    }


    template <typename T>
    typename ExpressionTree::Storage<T> IndirectNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // The base node's type ensures that the storage represent a T* rather
        // than the void* returned by CodeGenAsBase(). The local offset calculated
        // from the index skips the required number of T's, so it still represents
        // a T*. Dereference the calculated T* to get to T.
        return ExpressionTree::Storage<T>(m_collapsedBase->CodeGenAsBase(tree),
                                          m_collapsedOffset);
    }


    template <typename T>
    unsigned IndirectNode<T>::LabelSubtree(bool /*isLeftChild*/)
    {
        // TODO: Should isLeftChild be passed down?
        SetRegisterCount(m_collapsedBase->LabelSubtree(true));
        return GetRegisterCount();
    }


    template <typename T>
    void IndirectNode<T>::Print() const
    {
        PrintCoreProperties("IndirectNode");

        std::cout << ", base ID = " << m_base.GetId()
                  << ", index = " << m_index;

        if (m_base.GetId() != m_collapsedBase->GetId())
        {
            std::cout
                  << ", collapsed base ID = " << m_collapsedBase->GetId()
                  << ", collapsed offset = " << m_collapsedOffset;
        }
    }
}
