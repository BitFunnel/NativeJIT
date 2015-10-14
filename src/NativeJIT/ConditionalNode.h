#pragma once

#include <algorithm>    // For std::max

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
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;


    private:
        FlagExpressionNode<JCC>& m_condition;
        Node<T>& m_trueExpression;
        Node<T>& m_falseExpression;
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
        virtual ExpressionTree::Storage<bool> CodeGenValue(ExpressionTree& tree) override;


        //
        // Overrides of FlagExpression methods.
        //
        virtual void CodeGenFlags(ExpressionTree& tree) override;

    private:
        Node<T>& m_left;
        Node<T>& m_right;
    };


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
    typename ExpressionTree::Storage<T> ConditionalNode<T, JCC>::CodeGenValue(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        Label conditionIsTrue = code.AllocateLabel();
        Label testCompleted = code.AllocateLabel();

        // TODO: Evaluating both expressions in advance of the test is
        // sub-optimal, but it is currently required to guarantee consistent
        // state: the execution in NativeJIT has a continuous flow regardless
        // of the outcome of the (runtime) condition whereas the generated x64
        // code has two branches and each of them can have independent impact on
        // allocated and spilled registers. To have each expression evaluated
        // only if its branch is hit, the NativeJIT code would have to ensure
        // that the state of the allocated/spilled registers (i.e. all Storages)
        // is consistent once those two x64 branches converge back.

        Storage<T> trueValue;
        Storage<T> falseValue;

        CodeGenInPreferredOrder(tree,
                                m_trueExpression, trueValue,
                                m_falseExpression, falseValue);

        // Enum that specifies whether the result storage currently holds the
        // true value, false value or neither of them.
        enum class ResultContents { NeitherValue, TrueValue, FalseValue };
        ResultContents resultContents;
        Storage<T> result;

        {
            // Evaluate the condition to update the CPU flags. No code in this
            // block is allowed to modify the flags up until the
            // EmitConditionalJump() call at the end. Spilling (i.e. the MOV
            // instructions used used to copy the spilled value from register
            // to stack) does not affect any flags.
            m_condition.CodeGenFlags(tree);

            // Try to re-use a direct register from true/false expressions if
            // possible, otherwise allocate a register. The allocation must be
            // done before the conditional jump so that any register spills
            // apply to both branches.
            if (trueValue.GetStorageClass() == StorageClass::Direct
                && trueValue.IsSoleDataOwner())
            {
                result = trueValue;
                resultContents = ResultContents::TrueValue;
            }
            else if (falseValue.GetStorageClass() == StorageClass::Direct
                     && falseValue.IsSoleDataOwner())
            {
                result = falseValue;
                resultContents = ResultContents::FalseValue;
            }
            else
            {
                result = tree.Direct<T>();
                resultContents = ResultContents::NeitherValue;
            }

            code.EmitConditionalJump<JCC>(conditionIsTrue);
        }

        // Emit the code for the "condition is false" branch.

        // Move the false value to the result register unless it's already there.
        if (resultContents != ResultContents::FalseValue)
        {
            CodeGenHelpers::Emit<OpCode::Mov>(code, result.GetDirectRegister(), falseValue);
        }

        // Jump behind the true branch, unless the true branch is empty. The true
        // branch is empty only if the true value is already in the result storage.
        if (!(resultContents == ResultContents::TrueValue))
        {
            code.Jmp(testCompleted);
        }

        // Emit the code for the "condition is true" branch.

        code.PlaceLabel(conditionIsTrue);

        // Move the true value in the result register unless it's already there.
        if (resultContents != ResultContents::TrueValue)
        {
            CodeGenHelpers::Emit<OpCode::Mov>(code, result.GetDirectRegister(), trueValue);
        }

        code.PlaceLabel(testCompleted);

        return result;
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
        m_left.IncrementParentCount();
        m_right.IncrementParentCount();
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
    typename ExpressionTree::Storage<bool> RelationalOperatorNode<T, JCC>::CodeGenValue(ExpressionTree& tree)
    {
        X64CodeGenerator& code = tree.GetCodeGenerator();

        Label conditionIsTrue = code.AllocateLabel();
        Label testCompleted = code.AllocateLabel();

        // Evaluate the condition and react based on it.
        CodeGenFlags(tree);
        // Allocate the result register before the conditional jump so that
        // if any register gets spilled, the spill applies to both branches.
        // The spilling (i.e. the MOV instruction that is used to copy the
        // spilled value from the register onto stack) does not affect any flags.
        auto result = tree.Direct<bool>();
        code.EmitConditionalJump<JCC>(conditionIsTrue);

        code.EmitImmediate<OpCode::Mov>(result.GetDirectRegister(), false);
        code.Jmp(testCompleted);

        code.PlaceLabel(conditionIsTrue);
        code.EmitImmediate<OpCode::Mov>(result.GetDirectRegister(), true);

        code.PlaceLabel(testCompleted);

        return result;
    }


    template <typename T, JccType JCC>
    void RelationalOperatorNode<T, JCC>::CodeGenFlags(ExpressionTree& tree)
    {
        Storage<T> sLeft;
        Storage<T> sRight;

        CodeGenInPreferredOrder(tree,
                                m_left, sLeft,
                                m_right, sRight);

        CodeGenHelpers::Emit<OpCode::Cmp>(tree.GetCodeGenerator(), sLeft.ConvertToDirect(false), sRight);
    }
}
