#pragma once

#include <type_traits>

#include "NativeJIT/CodeGen/X64CodeGenerator.h"     // OpCode type.
#include "NativeJIT/Nodes/Node.h"


namespace NativeJIT
{
    // Implements a binary node that requires a constant as a right hand
    // argument. Unlike the regular binary node, the constant cannot end up
    // in a register, be spilled to a temporary etc, it's always a constant.
    template <OpCode OP, typename L, typename R>
    class BinaryImmediateNode : public Node<L>
    {
    public:
        static_assert(std::is_integral<R>::value,
                      "The right side must have an integral immediate type");

        BinaryImmediateNode(ExpressionTree& tree, Node<L>& left, R right);

        virtual Storage<L> CodeGenValue(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print(std::ostream& out) const override;

    private:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~BinaryImmediateNode();

        Node<L>& m_left;
        const R m_right;
    };


    //*************************************************************************
    //
    // Template definitions for BinaryImmediateNode
    //
    //*************************************************************************
    template <OpCode OP, typename L, typename R>
    BinaryImmediateNode<OP, L, R>::BinaryImmediateNode(
        ExpressionTree& tree, Node<L>& left, R right)
        : Node<L>(tree),
          m_left(left),
          m_right(right)
    {
        m_left.IncrementParentCount();
        // m_right is not a Node, so no IncrementParentCount() call.
    }


    template <OpCode OP, typename L, typename R>
    Storage<L> BinaryImmediateNode<OP, L, R>::CodeGenValue(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        auto left = m_left.CodeGen(tree);
        code.EmitImmediate<OP>(left.ConvertToDirect(true), m_right);

        return left;
    }


    template <OpCode OP, typename L, typename R>
    unsigned BinaryImmediateNode<OP, L, R>::LabelSubtree(bool /* isLeftChild */)
    {
        return m_left.LabelSubtree(true);
    }


    template <OpCode OP, typename L, typename R>
    void BinaryImmediateNode<OP, L, R>::Print(std::ostream& out) const
    {
        const std::string name = std::string("Operation (")
            + X64CodeGenerator::OpCodeName(OP)
            + ") ";
        this->PrintCoreProperties(out, name.c_str());

        out << ", left = " << m_left.GetId()
            << ", right = " << m_right;
    }
}
