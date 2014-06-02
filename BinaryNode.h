#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename L, typename R>
    class BinaryNode : public Node<L>
    {
    public:
        BinaryNode(ExpressionTree& tree, char const* operation, Node<L>& left, Node<R>& right);

        virtual Storage<L> CodeGenValue2(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        static unsigned ComputeRegisterCount(unsigned leftCount, unsigned rightCount);

        char const* m_operation;
        Node<L>& m_left;
        Node<R>& m_right;
    };


    //*************************************************************************
    //
    // Template definitions for BinaryNode
    //
    //*************************************************************************
    template <typename L, typename R>
    BinaryNode<L, R>::BinaryNode(ExpressionTree& tree,
                                 char const* operation,
                                 Node<L>& left,
                                 Node<R>& right)
        : Node(tree),
          m_operation(operation),
          m_left(left),
          m_right(right)
    {
        left.IncrementParentCount();
        right.IncrementParentCount();
    }


    template <typename L, typename R>
    typename Storage<L> BinaryNode<L, R>::CodeGenValue2(ExpressionTree& tree)
    {
        if (IsCached2())
        {
            auto result = GetCache2();
            ReleaseCache2();
            return result;
        }
        else
        {
            unsigned l = m_left.GetRegisterCount();
            unsigned r = m_right.GetRegisterCount();
            unsigned a = tree.GetAvailableRegisterCount<RegisterType>();

            if (r <= l && r < a)
            {
                // Evaluate left first. Once evaluation completes, left will use one register,
                // leaving at least a-1 registers for right.
                auto sLeft = m_left.CodeGenValue2(tree);
                sLeft.ConvertToValue(tree, true);
                auto sRight = m_right.CodeGenValue2(tree);

                tree.GetCodeGenerator().Op(m_operation, sLeft.GetDirectRegister(), sRight);
                return sLeft;
            }
            else if (l < r && l < a)
            {
                // Evaluate right first. Once evaluation completes, right will use one register,
                // leaving at least a-1 registers for left.
                auto sRight = m_right.CodeGenValue2(tree);
                auto sLeft = m_left.CodeGenValue2(tree);
                sLeft.ConvertToValue(tree, true);

                tree.GetCodeGenerator().Op(m_operation, sLeft.GetDirectRegister(), sRight);
                return sLeft;
            }
            else
            {
                // The smaller of l and r is greater than a, therefore
                // both l and r are greater than a. Since there are not
                // enough registers available, need to spill to memory.
                auto sRight = m_right.CodeGenValue2(tree);
                sRight.Spill(tree);

                auto sLeft = m_left.CodeGenValue2(tree);
                sLeft.ConvertToValue(tree, true);

                tree.GetCodeGenerator().Op(m_operation, sLeft.GetDirectRegister(), sRight);

                return sLeft;
            }
        }
    }


    template <typename L, typename R>
    unsigned BinaryNode<L, R>::LabelSubtree(bool isLeftChild)
    {
        unsigned left = m_left.LabelSubtree(true);
        unsigned right = m_right.LabelSubtree(false);

        SetRegisterCount(ComputeRegisterCount(left, right));

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename L, typename R>
    void BinaryNode<L, R>::Print() const
    {
        std::cout << "Operation (" << m_operation << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", left = " << m_left.GetId();
        std::cout << ", right = " << m_right.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename L, typename R>
    unsigned BinaryNode<L, R>::ComputeRegisterCount(unsigned left, unsigned right)
    {
        if (left > right)
        {
            return left;
        }
        else if (right > left)
        {
            return right;
        }
        else
        {
            return left + 1;
        }
    }
}
