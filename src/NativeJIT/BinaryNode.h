#pragma once

#include "CodeGenHelpers.h"
#include "NativeJIT/X64CodeGenerator.h"     // OpCode type.
#include "Node.h"


namespace NativeJIT
{
    template <OpCode OP, typename L, typename R>
    class BinaryNode : public Node<L>
    {
    public:
        BinaryNode(ExpressionTree& tree, Node<L>& left, Node<R>& right);

        virtual ExpressionTree::Storage<L> CodeGenValue(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<L>& m_left;
        Node<R>& m_right;
    };


    //*************************************************************************
    //
    // Template definitions for BinaryNode
    //
    //*************************************************************************
    template <OpCode OP, typename L, typename R>
    BinaryNode<OP, L, R>::BinaryNode(ExpressionTree& tree,
                                     Node<L>& left,
                                     Node<R>& right)
        : Node(tree),
          m_left(left),
          m_right(right)
    {
        left.IncrementParentCount();
        right.IncrementParentCount();
    }


    template <OpCode OP, typename L, typename R>
    typename ExpressionTree::Storage<L> BinaryNode<OP, L, R>::CodeGenValue(ExpressionTree& tree)
    {
        unsigned l = m_left.GetRegisterCount();
        unsigned r = m_right.GetRegisterCount();

        Storage<L> sLeft;
        Storage<R> sRight;

        // Evaluate the side which uses more registers first to minimize spilling.
        if (l >= r)
        {
            sLeft = m_left.CodeGen(tree);
            sRight = m_right.CodeGen(tree);
        }
        else
        {
            sRight = m_right.CodeGen(tree);
            sLeft = m_left.CodeGen(tree);
        }

        CodeGenHelpers::Emit<OP>(tree.GetCodeGenerator(), sLeft.ConvertToDirect(true), sRight);

        return sLeft;
    }


    template <OpCode OP, typename L, typename R>
    unsigned BinaryNode<OP, L, R>::LabelSubtree(bool /*isLeftChild*/)
    {
        unsigned left = m_left.LabelSubtree(true);
        unsigned right = m_right.LabelSubtree(false);

        SetRegisterCount(ComputeRegisterCount(left, right));

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <OpCode OP, typename L, typename R>
    void BinaryNode<OP, L, R>::Print() const
    {
        std::cout << "Operation (" << X64CodeGenerator::OpCodeName(OP) << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", left = " << m_left.GetId();
        std::cout << ", right = " << m_right.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
