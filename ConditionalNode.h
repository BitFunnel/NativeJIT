#pragma once

#include "ExpressionTree.h"
#include "Node.h"
#include "X64CodeGenerator.h"


namespace NativeJIT
{
    class ExpressionTree;

    template <JccType JCC>
    class FlagExpressionNode : public Node<bool>
    {
    public:
        FlagExpressionNode(ExpressionTree& tree);

        virtual void CodeGenFlags(ExpressionTree& tree) = 0;
    };


    template <typename T, JccType JCC>
    class ConditionalNode : public Node<T>
    {
    public:
        ConditionalNode(ExpressionTree& tree,
                        FlagExpressionNode<JCC>& condition,
                        Node<T>& trueExpression,
                        Node<T>& falseExpression);


        //
        // Overrides of Node methods.
        //
        virtual bool IsImmediate() const override;
        virtual bool IsIndirect() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;


        //
        // Overrides of Node<T> methods.
        //
        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;


    private:
        FlagExpressionNode<JCC>& m_condition;
        Node<T>& m_trueExpression;
        Node<T>& m_falseExpression;
    };


    // TODO: This should be a cast node instead of IsTrue.
    template <typename T>
    class IsTrue : public FlagExpressionNode<JccType::JZ>
    {
    public:
        IsTrue(ExpressionTree& tree, Node<T>& value);


        //
        // Overrides of Node methods.
        //
        virtual bool IsImmediate() const override;
        virtual bool IsIndirect() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;


        //
        // Overrides of Node<T> methods.
        //
        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;


        //
        // Overrides of FlagExpression methods.
        //
        virtual void CodeGenFlags(ExpressionTree& tree) override;

    private:
        Node<T>& m_value;
    };


    template <typename T, JccType JCC>
    class RelationalOperatorNode : public FlagExpressionNode<JCC>
    {
    public:
        RelationalOperatorNode(ExpressionTree& tree,
                               Node<T>& left,
                               Node<T>& right);


        //
        // Overrides of Node methods.
        //
        virtual bool IsImmediate() const override;
        virtual bool IsIndirect() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;


        //
        // Overrides of Node<T> methods.
        //
        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;


        //
        // Overrides of FlagExpression methods.
        //
        virtual void CodeGenFlags(ExpressionTree& tree) override;

    private:
        static unsigned ComputeRegisterCount(unsigned left, unsigned right);

        Node<T>& m_left;
        Node<T>& m_right;
    };


    //class LogicalAndNode : public FlagExpressionNode
    //{
    //public:
    //    LogicalAndNode(FlagExpressionNode& left, FlagExpressionNode& right);

    //private:
    //    FlagExpressionNode& m_left;
    //    FlagExpressionNode& m_right;
    //};


    //*************************************************************************
    //
    // Template definitions for FlagExpressionNode
    //
    //*************************************************************************
    template <JccType JCC>
    FlagExpressionNode<JCC>::FlagExpressionNode(ExpressionTree& tree)
        : Node(tree)
    {
    }


    //*************************************************************************
    //
    // Template definitions for ConditionalNode
    //
    //*************************************************************************
    template <typename T, JccType JCC>
    ConditionalNode<T, JCC>::ConditionalNode(ExpressionTree& tree,
                                            FlagExpressionNode<JCC>& condition,
                                            Node<T>& trueExpression,
                                            Node<T>& falseExpression)
        : Node(tree),
          m_condition(condition),
          m_trueExpression(trueExpression),
          m_falseExpression(falseExpression)
    {
    }


    template <typename T, JccType JCC>
    bool ConditionalNode<T, JCC>::IsImmediate() const
    {
        return false;
    }


    template <typename T, JccType JCC>
    bool ConditionalNode<T, JCC>::IsIndirect() const
    {
        return false;
    }


    template <typename T, JccType JCC>
    unsigned ConditionalNode<T, JCC>::LabelSubtree(bool isLeftChild)
    {
        unsigned condition = m_condition.LabelSubtree(true);
        unsigned trueExpression = m_trueExpression.LabelSubtree(true);
        unsigned falseExpression = m_falseExpression.LabelSubtree(true);

        // TODO: Might want to store the counts separately and only spill when necessary.

        SetRegisterCount((std::max)(condition, (std::max)(trueExpression, falseExpression)));

        return GetRegisterCount();
    }


    template <typename T, JccType JCC>
    void ConditionalNode<T, JCC>::Print() const
    {
        std::cout << "Conditional(" << JccName(JCC) << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", condition = " << m_condition.GetId();
        std::cout << ", trueExpression = " << m_trueExpression.GetId();
        std::cout << ", right = " << m_falseExpression.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T, JccType JCC>
    typename Node<T>::RegisterType ConditionalNode<T, JCC>::CodeGenValue(ExpressionTree& tree)
    {
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            m_condition.CodeGenFlags(tree);

            X64CodeGenerator& code = tree.GetCodeGenerator();
            Label l1 = code.AllocateLabel();
            code.Jcc(JCC, l1);

            RegisterType r = m_trueExpression.CodeGenValue(tree);
            if (m_trueExpression.IsCached())
            {
                r = tree.CopyRegister(r);
            }

            Label l2 = code.AllocateLabel();
            code.Jmp(l2);

            code.PlaceLabel(l1);

            RegisterType r2 = m_falseExpression.CodeGenValue(tree);
            // TODO: Use Register::operator==()
            if (r2.GetId() != r.GetId())
            {
                // TODO: Do we always need to move the value?
                // In the case of caching, r2 may not be equal to r.
                // TODO: unit test for this case
                code.Op("mov", r, r2);
            }

            code.PlaceLabel(l2);

            return r;
        }
    }


    //*************************************************************************
    //
    // Template definitions for IsTrue
    //
    //*************************************************************************
    template <typename T>
    IsTrue<T>::IsTrue(ExpressionTree& tree, Node<T>& value)
        : FlagExpressionNode(tree),
          m_value(value)
    {
    }


    template <typename T>
    bool IsTrue<T>::IsImmediate() const
    {
        return false;
    }


    template <typename T>
    bool IsTrue<T>::IsIndirect() const
    {
        return false;
    }


    template <typename T>
    unsigned IsTrue<T>::LabelSubtree(bool isLeftChild)
    {
        SetRegisterCount(m_value.LabelSubtree(true));

        return GetRegisterCount();
    }


    template <typename T>
    void IsTrue<T>::Print() const
    {
        std::cout << "IsTrue id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", value = " << m_value.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T>
    typename FlagExpressionNode<JccType::JZ>::RegisterType IsTrue<T>::CodeGenValue(ExpressionTree& tree)
    {
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            X64CodeGenerator& code = tree.GetCodeGenerator();

            CodeGenFlags(tree);

            Label l1 = code.AllocateLabel();
            code.Jcc(JccType::JZ, l1);

            RegisterType r = tree.AllocateRegister<RegisterType>();
            code.Op("mov", r, 1);

            Label l2 = code.AllocateLabel();
            code.Jmp(l2);
            code.PlaceLabel(l1);

            code.Op("mov", r, 1);
            code.PlaceLabel(l2);

            return r;
        }
    }


    template <typename T>
    void IsTrue<T>::CodeGenFlags(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            code.Op("or", r, r);
        }
        else
        {
            // TODO: This Register copy constructor doesn't work with double.
            Node<T>::RegisterType r = m_value.CodeGenValue(tree);

            code.Op("or", r, r);

            if (!m_value.IsCached())
            {
                tree.ReleaseRegister(r);
            }
        }
    }


    //*************************************************************************
    //
    // Template definitions for RelationalOperator
    //
    //*************************************************************************
    template <typename T, JccType JCC>
    RelationalOperatorNode<T, JCC>::RelationalOperatorNode(ExpressionTree& tree,
                                                           Node<T>& left,
                                                           Node<T>& right)
        : FlagExpressionNode(tree),
          m_left(left),
          m_right(right)
    {
    }


    template <typename T, JccType JCC>
    bool RelationalOperatorNode<T, JCC>::IsImmediate() const
    {
        return false;
    }


    template <typename T, JccType JCC>
    bool RelationalOperatorNode<T, JCC>::IsIndirect() const
    {
        return false;
    }


    template <typename T, JccType JCC>
    unsigned RelationalOperatorNode<T, JCC>::LabelSubtree(bool isLeftChild)
    {
        unsigned left = m_left.LabelSubtree(true);
        unsigned right = m_right.LabelSubtree(false);

        SetRegisterCount(ComputeRegisterCount(left, right));

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename T, JccType JCC>
    void RelationalOperatorNode<T, JCC>::Print() const
    {
        std::cout << "RelationalOperatorNode(" << JccName(JCC) << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", left = " << m_left.GetId();
        std::cout << ", right = " << m_right.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T, JccType JCC>
    typename RelationalOperatorNode<T, JCC>::RegisterType RelationalOperatorNode<T, JCC>::CodeGenValue(ExpressionTree& tree)
    {
        // TODO: High duplication with IsTrue<T>::CodeGenValue().
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            X64CodeGenerator& code = tree.GetCodeGenerator();

            CodeGenFlags(tree);

            Label l1 = code.AllocateLabel();
            code.Jcc(JccType::JZ, l1);

            RegisterType r = tree.AllocateRegister<RegisterType>();
            code.Op("mov", r, 1);

            Label l2 = code.AllocateLabel();
            code.Jmp(l2);
            code.PlaceLabel(l1);

            code.Op("mov", r, 1);
            code.PlaceLabel(l2);

            return r;
        }
    }


    template <typename T, JccType JCC>
    void RelationalOperatorNode<T, JCC>::CodeGenFlags(ExpressionTree& tree)
    {
        // TODO: High duplication with BinaryNode<L,R>::CodeGenValue().
        if (IsCached())
        {
            auto r = GetCacheRegister();
            ReleaseCache();

            // TODO: This code is wrong - need to set the correct JCC - not just the zero flag.
            tree.GetCodeGenerator().Op("or", r, r);
            throw 0;

            if (!IsCached())
            {
               tree.ReleaseRegister(r);
            }

            return;
        }
        else if (m_right.IsImmediate() && !m_right.IsCached())
        {
            auto r = m_left.CodeGenValue(tree);

            // cmp r, x
            ImmediateNode<T> const & right = static_cast<ImmediateNode<T> const &>(m_right);
            tree.GetCodeGenerator().Op("cmp", r, right.GetValue());

            if (!m_left.IsCached())
            {
                tree.ReleaseRegister(r);
            }

            return;
        }
        else if (m_right.IsIndirect() && !m_right.IsCached())
        {
            auto r = m_left.CodeGenValue(tree);

            // cmp r, [src + offset]
            IndirectNode<T>& right = static_cast<IndirectNode<T>&>(m_right);

            auto base = right.CodeGenBase(tree);
            unsigned __int64 offset = right.GetOffset();

            tree.GetCodeGenerator().Op("cmp", r, base, offset);

            if (!m_left.IsCached())
            {
                tree.ReleaseRegister(r);
            }

            if (!right.IsBaseRegisterCached())
            {
               tree.ReleaseRegister(base);
            }

            return;
        }
        else
        {
            unsigned l = m_left.GetRegisterCount();
            unsigned r = m_right.GetRegisterCount();
            unsigned a = tree.GetAvailableRegisterCount<RegisterType>();

            if (r <= l && r < a)
            {
                auto r1 = m_left.CodeGenValue(tree);
                auto r2 = m_right.CodeGenValue(tree);

                // cmp r1, r2
                tree.GetCodeGenerator().Op("cmp", r1, r2);

                if (!m_left.IsCached())
                {
                    tree.ReleaseRegister(r1);
                }

                if (!m_right.IsCached())
                {
                    tree.ReleaseRegister(r2);
                }

                return;
            }
            else if (r > l && l < a)
            {
                auto r2 = m_right.CodeGenValue(tree);
                auto r1 = m_left.CodeGenValue(tree);

                // cmp r1, r2
                tree.GetCodeGenerator().Op("cmp", r1, r2);

                if (!m_left.IsCached())
                {
                    tree.ReleaseRegister(r1);
                }

                if (!m_right.IsCached())
                {
                    tree.ReleaseRegister(r2);
                }

                return;
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

                return;
            }
        }

        // Should never get here.
        throw 0;
    }


    template <typename T, JccType JCC>
    unsigned RelationalOperatorNode<T, JCC>::ComputeRegisterCount(unsigned left, unsigned right)
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


    //*************************************************************************
    //
    //
    //
    //*************************************************************************


    //*************************************************************************
    //
    //
    //
    //*************************************************************************
}
