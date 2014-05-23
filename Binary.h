#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename L, typename R>
    class BinaryNode : public Node<L>
    {
    public:
        BinaryNode(ExpressionTree& tree, char const* operation, Node<L>& left, Node<R>& right);

        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;
        virtual bool IsIndirect() const override;
        virtual bool IsImmediate() const override;
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
    typename BinaryNode<L, R>::RegisterType BinaryNode<L, R>::CodeGenValue(ExpressionTree& tree)
    {
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else if (m_right.IsImmediate() && !m_right.IsCached())
        {
            RegisterType r = m_left.CodeGenValue(tree);

            if (m_left.IsCached())
            {
                r = tree.CopyRegister(r);
            }

            // op r, x
            ImmediateNode<R> const & right = static_cast<ImmediateNode<R> const &>(m_right);
            tree.GetCodeGenerator().Op(m_operation, r, right.GetValue());

            return r;
        }
        else if (m_right.IsIndirect() && !m_right.IsCached())
        {
            RegisterType r = m_left.CodeGenValue(tree);

            if (m_left.IsCached())
            {
                r = tree.CopyRegister(r);
            }

            // op r, [src + offset]
            Indirect<R>& right = static_cast<Indirect<R>&>(m_right);

            auto base = right.CodeGenBase(tree);
            unsigned __int64 offset = right.GetOffset();

            tree.GetCodeGenerator().Op(m_operation, r, base, offset);

            if (!right.IsBaseRegisterCached())
            {
               tree.ReleaseRegister(base);
            }

            return r;
        }
        else
        {
            unsigned l = m_left.GetRegisterCount();
            unsigned r = m_right.GetRegisterCount();
            unsigned a = tree.GetAvailableRegisterCount<RegisterType>();

            if (r <= l && r < a)
            {
                auto r1 = m_left.CodeGenValue(tree);
                if (m_left.IsCached())
                {
                    r1 = tree.CopyRegister(r1);
                }

                auto r2 = m_right.CodeGenValue(tree);

                // op r1, r2
                tree.GetCodeGenerator().Op(m_operation, r1, r2);

                if (!m_right.IsCached())
                {
                    tree.ReleaseRegister(r2);
                }

                return r1;
            }
            else if (r > l && l < a)
            {
                auto r2 = m_right.CodeGenValue(tree);

                auto r1 = m_left.CodeGenValue(tree);
                if (m_left.IsCached())
                {
                    r1 = tree.CopyRegister(r1);
                }

                // op r1, r2
                tree.GetCodeGenerator().Op(m_operation, r1, r2);

                if (!m_right.IsCached())
                {
                    tree.ReleaseRegister(r2);
                }

                return r1;
            }
            else
            {
                Assert(false, "Register spilling not implemented.");

                //auto r2 = m_right.CodeGenValue(tree);
                //// store r2
                //// TODO: Implement store.
                //throw 0;
                //tree.ReleaseRegister(r2);

                //auto r1 = m_left.CodeGenValue(tree);
                //if (m_left.IsCached())
                //{
                //    r1 = tree.CopyRegister(r1);
                //}

                //// op r1, tmp
                //// TODO: Reinstate
                ////                tree.GetCodeGenerator().Op(m_operation, r1, "[temp]");

                //return r1;
            }
        }

        // Should never get here.

        throw 0;
    }


    template <typename L, typename R>
    bool BinaryNode<L, R>::IsIndirect() const
    {
        return false;
    }


    template <typename L, typename R>
    bool BinaryNode<L, R>::IsImmediate() const
    {
        return false;
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
