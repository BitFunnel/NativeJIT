#pragma once

#include <iostream>     // TODO: Delete this file after removing all references to std::cout.

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
        virtual Storage<R> CodeGenValue(ExpressionTree& tree) override;
//        virtual void CompileAsRoot(ExpressionTree& tree) override;
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
    }


    template <typename R, typename P1, typename P2>
    Storage<R> CallNode<R, P1, P2>::CodeGenValue(ExpressionTree& tree)
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
        std::cout << "// Push volatile registers." << std::endl;

        {
            auto f = m_function.CodeGen(tree);
            f.ConvertToValue(tree, false);

            tree.Prefer(rcx);
            auto s1 = m_p1.CodeGen(tree);
            s1.ConvertToValue(tree, false);

            tree.Prefer(rdx);
            auto s2 = m_p2.CodeGen(tree);
            s2.ConvertToValue(tree, false);

            std::cout << "// Set up parameter registers." << std::endl;

            tree.GetCodeGenerator().Op("call", f.GetDirectRegister());
        }

        std::cout << "// Pop volatile registers." << std::endl;

        auto x = tree.AllocateRegister<Storage<R>::DirectRegister>();
        Storage<R> result(tree, x);
//        Storage<R> result2(tree, Storage<R>::DirectRegister(0));
        tree.GetCodeGenerator().Op("mov", result.GetDirectRegister(), Storage<R>::DirectRegister(0));

        // TODO: This is wrong. Need copy RAX or XMM0 to correct register.
        // Return storage has to be compatible with type R.
        return result;
    }


    //template <typename R, typename P1, typename P2>
    //void CallNode<R, P1, P2>::CompileAsRoot(ExpressionTree& /*tree*/)
    //{
    //}


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
