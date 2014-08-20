#pragma once

#include <iostream>     // TODO: Delete this file after removing all references to std::cout.

#include "NativeJIT/X64CodeGenerator.h"
#include "Node.h"


namespace NativeJIT
{
    class ExpressionTree;

    //*************************************************************************
    //
    // CallNode
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2>
    class CallNode : public Node<R>
    {
    public:
        typedef R (*FunctionPointer)(P1, P2);

        CallNode(ExpressionTree& tree,
                 Node<FunctionPointer>& function,
                 Node<P1>& p1,
                 Node<P2>& p2);

        //
        // Overrides of Node methods.
        //
        virtual ExpressionTree::Storage<R> CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        static unsigned ComputeRegisterCount(unsigned p1, unsigned p2);

        Node<FunctionPointer>& m_function;
        Node<P1>& m_p1;
        Node<P2>& m_p2;
    };


    //*************************************************************************
    //
    // Template definitions for CallNode
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2>
    CallNode<R, P1, P2>::CallNode(ExpressionTree& tree,
                                  Node<FunctionPointer>& function,
                                  Node<P1>& p1,
                                  Node<P2>& p2)
        : Node(tree),
          m_function(function),
          m_p1(p1),
          m_p2(p2)
    {
        p1.IncrementParentCount();
        p2.IncrementParentCount();
    }


    template <typename R, typename P1, typename P2>
    ExpressionTree::Storage<R> CallNode<R, P1, P2>::CodeGenValue(ExpressionTree& tree)
    {
        // Save non-volatile registers.
        // Evaluate parameters.
        //   Into correct registers if possible.
        //   Otherwise into temporary registers or parameter homes.
        // Move parameters into correct registers.
        // Evaluate function pointer.
        // Make call
        // Potentially move result into new register.
        // Restore non-volatile registers.

        {
            auto f = m_function.CodeGen(tree);
            f.ConvertToValue(tree, false);

            auto s1 = m_p1.CodeGen(tree);
            s1.ConvertToValue(tree, false);
            ExpressionTree::Storage<P1> s1a;
            if (s1.GetDirectRegister().GetId() != 1)
            {
                Node<P1>::RegisterType rcx(1);
                s1a = tree.Direct<P1>(rcx);
                std::cout << "P1" << std::endl;
                tree.GetCodeGenerator().Emit<OpCode::Mov>(rcx, s1.GetDirectRegister());
            }
            s1.Reset();

            auto s2 = m_p2.CodeGen(tree);
            s2.ConvertToValue(tree, false);
            ExpressionTree::Storage<P2> s2a;
            if (s2.GetDirectRegister().GetId() != 2)
            {
                Node<P2>::RegisterType rdx(2);
                s2a = tree.Direct<P2>(rdx);
                std::cout << "P2" << std::endl;
                tree.GetCodeGenerator().Emit<OpCode::Mov>(rdx, s2.GetDirectRegister());

                // PROBLEM1: When s2a goes out of scope, RCX gets released.
                // PROBLEM2: Even if s2a doesn't go out of scope, RCX will be bumped.
                // Issue is criss cross: rcx = rdx, rdx = rcx.
                // Can't rely solely on bumping.
            }
            s2.Reset();

            std::cout << "// Set up parameter registers." << std::endl;

            tree.GetCodeGenerator().Emit<OpCode::Call>(f.GetDirectRegister());

            // TODO: Need to reserve register 0 system wide. Otherwise need to allocate register 0.
            auto result = tree.Direct<R>();
            if (result.GetDirectRegister().GetId() != 0)
            {
                tree.GetCodeGenerator().Emit<OpCode::Mov>(result.GetDirectRegister(), Node<R>::RegisterType(0));
            }

            return result;
        }
    }


    template <typename R, typename P1, typename P2>
    unsigned CallNode<R, P1, P2>::LabelSubtree(bool /*isLeftChild*/)
    {
        unsigned p1 = m_p1.LabelSubtree(true);
        unsigned p2 = m_p2.LabelSubtree(false);

        SetRegisterCount(ComputeRegisterCount(p1, p2));

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename R, typename P1, typename P2>
    void CallNode<R, P1, P2>::Print() const
    {
        std::cout << "CallNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", p1 = " << m_p1.GetId();
        std::cout << ", p2 = " << m_p2.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename R, typename P1, typename P2>
    unsigned CallNode<R, P1, P2>::ComputeRegisterCount(unsigned left, unsigned right)
    {
        // TODO: Replace this placeholder code.
        // TODO: Is this correct?
        // TODO: Need register count for function pointer as well.
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
