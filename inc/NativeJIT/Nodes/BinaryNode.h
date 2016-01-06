#pragma once

#include "NativeJIT/CodeGen/X64CodeGenerator.h"     // OpCode type.
#include "NativeJIT/CodeGenHelpers.h"
#include "NativeJIT/Nodes/Node.h"


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
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~BinaryNode();

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
        Storage<L> sLeft;
        Storage<R> sRight;

        CodeGenInPreferredOrder(tree,
                                m_left, sLeft,
                                m_right, sRight);

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
        const std::string name = std::string("Operation (")
            + X64CodeGenerator::OpCodeName(OP)
            + ") ";
        PrintCoreProperties(name.c_str());

        std::cout << ", left = " << m_left.GetId();
        std::cout << ", right = " << m_right.GetId();
    }
}
