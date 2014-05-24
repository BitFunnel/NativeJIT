#pragma once

#include "ExpressionTree.h"
#include "Node.h"
#include "X64CodeGenerator.h"


namespace NativeJIT
{
    class ExpressionTree;

    class FlagExpressionNode : public Node<bool>
    {
    public:
        FlagExpressionNode(ExpressionTree& tree);

        virtual void CodeGenFlags(ExpressionTree& tree) = 0;
    };


    template <typename T>
    class ConditionalNode : public Node<T>
    {
    public:
        ConditionalNode(ExpressionTree& tree,
                        FlagExpressionNode& condition,
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
        FlagExpressionNode& m_condition;
        Node<T>& m_trueExpression;
        Node<T>& m_falseExpression;
    };


    template <typename T>
    class IsTrue : public FlagExpressionNode
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


    template <typename T>
    class RelationalOperatorNode : public FlagExpressionNode
    {
    public:
        RelationalOperatorNode(char const* jmpType,
                               Node<T>& left,
                               Node<T>& right);

    private:
        char const* m_jmpType;
        Node<T>& m_left;
        Node<T>& m_right;
    };


    class LogicalAndNode : public FlagExpressionNode
    {
    public:
        LogicalAndNode(FlagExpressionNode& left, FlagExpressionNode& right);

    private:
        FlagExpressionNode& m_left;
        FlagExpressionNode& m_right;
    };


    //*************************************************************************
    //
    // Template definitions for ConditionalNode
    //
    //*************************************************************************
    template <typename T>
    ConditionalNode<T>::ConditionalNode(ExpressionTree& tree,
                                        FlagExpressionNode& condition,
                                        Node<T>& trueExpression,
                                        Node<T>& falseExpression)
        : Node(tree),
          m_condition(condition),
          m_trueExpression(trueExpression),
          m_falseExpression(falseExpression)
    {
    }


    template <typename T>
    bool ConditionalNode<T>::IsImmediate() const
    {
        return false;
    }


    template <typename T>
    bool ConditionalNode<T>::IsIndirect() const
    {
        return false;
    }


    template <typename T>
    unsigned ConditionalNode<T>::LabelSubtree(bool isLeftChild)
    {
        unsigned condition = m_condition.LabelSubtree(true);
        unsigned trueExpression = m_trueExpression.LabelSubtree(true);
        unsigned falseExpression = m_falseExpression.LabelSubtree(true);

        // TODO: Might want to store the counts separately and only spill when necessary.

        SetRegisterCount((std::max)(condition, (std::max)(trueExpression, falseExpression)));

        return GetRegisterCount();
    }


    template <typename T>
    void ConditionalNode<T>::Print() const
    {
        std::cout << "Conditional id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", condition = " << m_condition.GetId();
        std::cout << ", trueExpression = " << m_trueExpression.GetId();
        std::cout << ", right = " << m_falseExpression.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T>
    typename Node<T>::RegisterType ConditionalNode<T>::CodeGenValue(ExpressionTree& tree)
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
            code.Jz(l1);

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
    typename FlagExpressionNode::RegisterType IsTrue<T>::CodeGenValue(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        CodeGenFlags(tree);
        Label l1 = code.AllocateLabel();
        code.Jz(l1);

        RegisterType r = tree.AllocateRegister<RegisterType>();
        code.Op("mov", r, 1);

        Label l2 = code.AllocateLabel();
        code.Jmp(l2);
        code.PlaceLabel(l1);

        code.Op("mov", r, 1);
        code.PlaceLabel(l2);

        return r;
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
    //
    //
    //*************************************************************************


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
