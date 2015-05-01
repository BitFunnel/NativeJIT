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

    private:
        Node<T*>& m_base;
        __int32 m_index;
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
          m_index(index)
    {
        m_base.IncrementParentCount();
    }


    template <typename T>
    typename ExpressionTree::Storage<T> IndirectNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // The base's offset makes the storage represent a T*. The local offset
        // skips the required number T's, so it still represents a T*.
        const __int32 localOffset = sizeof(T) * m_index;

        // Dereference the T*.
        return ExpressionTree::Storage<T>(tree,
                                          m_base.CodeGenBase(tree),
                                          m_base.GetOffset() + localOffset);
    }


    template <typename T>
    unsigned IndirectNode<T>::LabelSubtree(bool /*isLeftChild*/)
    {
        // TODO: Should isLeftChild be passed down?
        SetRegisterCount(m_base.LabelSubtree(true));
        return GetRegisterCount();
    }


    template <typename T>
    void IndirectNode<T>::Print() const
    {
        std::cout << "IndirectNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", index = " << m_index;
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
