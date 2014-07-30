#pragma once

#include "CodeGenHelpers.h"
#include "ExpressionTree.h"
#include "NativeJIT/X64CodeGenerator.h"
#include "Node.h"


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
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;


        //
        // Overrides of Node<T> methods.
        //
        virtual Storage<T> CodeGenValue(ExpressionTree& tree) override;


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
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;


        //
        // Overrides of Node<T> methods.
        //
        virtual Storage<bool> CodeGenValue(ExpressionTree& tree) override;


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
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;


        //
        // Overrides of Node<T> methods.
        //
        virtual Storage<bool> CodeGenValue(ExpressionTree& tree) override;


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
    unsigned ConditionalNode<T, JCC>::LabelSubtree(bool /*isLeftChild*/)
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
        std::cout << "Conditional(" << X64CodeGenerator::JccName(JCC) << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", condition = " << m_condition.GetId();
        std::cout << ", trueExpression = " << m_trueExpression.GetId();
        std::cout << ", right = " << m_falseExpression.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T, JccType JCC>
    typename Storage<T> ConditionalNode<T, JCC>::CodeGenValue(ExpressionTree& tree)
    {
        m_condition.CodeGenFlags(tree);

        X64CodeGenerator& code = tree.GetCodeGenerator();
        Label l1 = code.AllocateLabel();
        code.Jcc(JCC, l1);

        auto trueValue = m_trueExpression.CodeGen(tree);
        trueValue.ConvertToValue(tree, true);
        auto rTrue = trueValue.GetDirectRegister();

        Label l2 = code.AllocateLabel();
        code.Jmp(l2);

        code.PlaceLabel(l1);

        auto falseValue = m_falseExpression.CodeGen(tree);
        falseValue.ConvertToValue(tree, false);
        auto rFalse = falseValue.GetDirectRegister();

        // TODO: Use Register::operator==()
        if (rTrue.GetId() != rFalse.GetId())
        {
            // TODO: Do we always need to move the value?
            // In the case of caching, r2 may not be equal to r.
            // TODO: unit test for this case
            code.Op(OpCode::Mov, rTrue, rFalse);
        }

        code.PlaceLabel(l2);

        return trueValue;
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
    unsigned IsTrue<T>::LabelSubtree(bool /*isLeftChild*/)
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
    typename Storage<bool> IsTrue<T>::CodeGenValue(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        CodeGenFlags(tree);

        Label l1 = code.AllocateLabel();
        code.Jcc(JccType::JZ, l1);

        RegisterType r = tree.AllocateRegister<RegisterType>();
        code.Op(OpCode::Mov, r, 1);

        Label l2 = code.AllocateLabel();
        code.Jmp(l2);
        code.PlaceLabel(l1);

        code.Op(OpCode::Mov, r, 1);
        code.PlaceLabel(l2);

        return Storage<bool>(tree, r);
    }


    template <typename T>
    void IsTrue<T>::CodeGenFlags(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        if (IsCached())
        {
            auto value = GetCache();
            ReleaseCache();
            auto r = value.GetDirectRegister();
            code.Op(OpCode::Or, r, r);
        }
        else
        {
            auto value = m_value.CodeGenValue(tree);
            value.ConvertToValue(tree, false);
            auto r = value.GetDirectRegister();

            code.Op(OpCode::Or, r, r);
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
    unsigned RelationalOperatorNode<T, JCC>::LabelSubtree(bool /*isLeftChild*/)
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
        std::cout << "RelationalOperatorNode(" << X64CodeGenerator::JccName(JCC) << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", left = " << m_left.GetId();
        std::cout << ", right = " << m_right.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T, JccType JCC>
    typename Storage<bool> RelationalOperatorNode<T, JCC>::CodeGenValue(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        CodeGenFlags(tree);

        Label l1 = code.AllocateLabel();
        code.Jcc(JccType::JZ, l1);

        RegisterType r = tree.AllocateRegister<RegisterType>();
        code.Op(OpCode::Mov, r, 1);

        Label l2 = code.AllocateLabel();
        code.Jmp(l2);
        code.PlaceLabel(l1);

        code.Op(OpCode::Mov, r, 1);
        code.PlaceLabel(l2);

        return Storage<bool>(tree, r);
    }


    template <typename T, JccType JCC>
    void RelationalOperatorNode<T, JCC>::CodeGenFlags(ExpressionTree& tree)
    {
        if (IsCached())
        {
            auto result = GetCache();
            ReleaseCache();

            // TODO: This code is wrong - need to set the correct JCC - not just the zero flag.
            // TODO: For this opcode to work, result must be direct. Might consider putting flags check into storage.
            auto direct = result.GetDirectRegister();
            tree.GetCodeGenerator().Op(OpCode::Or, direct, direct);
            throw 0;
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
                auto sLeft = m_left.CodeGen(tree);
                sLeft.ConvertToValue(tree, true);
                auto sRight = m_right.CodeGen(tree);

                CodeGenHelpers::Emit(tree.GetCodeGenerator(), OpCode::Cmp, sLeft.GetDirectRegister(), sRight);
            }
            else if (l < r && l < a)
            {
                // Evaluate right first. Once evaluation completes, right will use one register,
                // leaving at least a-1 registers for left.
                auto sRight = m_right.CodeGen(tree);
                auto sLeft = m_left.CodeGen(tree);
                sLeft.ConvertToValue(tree, true);

                CodeGenHelpers::Emit(tree.GetCodeGenerator(), OpCode::Cmp, sLeft.GetDirectRegister(), sRight);
            }
            else
            {
                // The smaller of l and r is greater than a, therefore
                // both l and r are greater than a. Since there are not
                // enough registers available, need to spill to memory.
                auto sRight = m_right.CodeGen(tree);
                sRight.Spill(tree);

                auto sLeft = m_left.CodeGen(tree);
                sLeft.ConvertToValue(tree, true);

                CodeGenHelpers::Emit(tree.GetCodeGenerator(), OpCode::Cmp, sLeft.GetDirectRegister(), sRight);
            }
        }
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
}
