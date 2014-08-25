#pragma once

#include <iostream>     // Accessed by template definition for Print().

#include "Node.h"       // Base class.


namespace NativeJIT
{
    class ExpressionTree;


    class SaveRestoreVolatilesHelper
    {
    protected:
        SaveRestoreVolatilesHelper();

    public:
        // These methods need to be public for access by
        //     CallNodeBase::FunctionChild
        //     CallNodeBase::ParameterChild
        void SaveVolatiles(ExpressionTree& tree);
        void RestoreVolatiles(ExpressionTree& tree);

        template <unsigned SIZE>
        void RecordParameterRegister(Register<SIZE, false> r);

    private:
        // TODO: m_xmmParameters
        unsigned m_rxxParameters;

        // TODO: Figure out how to initialize with something like RAX | RCX | RDX | R8 | R9| R10| R11
        static const unsigned c_rxxVolatiles = 0xf07; //1111 0000 0111
    };


    template <typename R, unsigned PARAMETERCOUNT>
    class CallNodeBase : public Node<R>, public SaveRestoreVolatilesHelper
    {
    public:
        CallNodeBase(ExpressionTree& tree);

        //
        // Overrides of Node methods.
        //
        virtual ExpressionTree::Storage<R> CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    protected:
        class Child
        {
        public:
            virtual unsigned LabelSubtree(bool isLeftChild) = 0;
            virtual void EmitStaging(ExpressionTree& tree, SaveRestoreVolatilesHelper& volatiles) = 0;
            virtual void Release() = 0;
            virtual void Print() = 0;
        };


        template <typename T>
        class TypedChild : public Child
        {
        public:
            TypedChild(Node<T>& expression);

            //
            // Overrides of Child methods.
            //
            virtual unsigned LabelSubtree(bool isLeftChild);
            virtual void Release();

        protected:
            Node<T>& m_expression;
            ExpressionTree::Storage<T> m_storage;
        };


        template <typename T>
        class ParameterChild : public TypedChild<T>
        {
        public:
            ParameterChild(Node<T>& expression, unsigned position);

            typename ExpressionTree::Storage<T>::DirectRegister GetRegister() const;

            //
            // Overrides of Child methods.
            //
            virtual void EmitStaging(ExpressionTree& tree,
                                     SaveRestoreVolatilesHelper& volatiles) override;
            virtual void Print() override;

        private:
            typename ExpressionTree::Storage<T>::DirectRegister m_destination;
        };


        class FunctionChildBase
        {
        public:
            virtual void EmitCall(ExpressionTree& tree) = 0;
        };


        template <typename T>
        class FunctionChild : public FunctionChildBase, public TypedChild<T>
        {
        public:
            FunctionChild(Node<T>& expression);

            //
            // Overrides of Child methods.
            //
            virtual void EmitStaging(ExpressionTree& tree,
                                     SaveRestoreVolatilesHelper& volatiles) override;

            //
            // Overrides of FunctionChildBase methods.
            //
            virtual void EmitCall(ExpressionTree& tree) override;
            virtual void Print() override;
        };


        // One child for each parameter plus one for the function pointer.
        static const unsigned c_childCount = PARAMETERCOUNT + 1;
        Child* m_children[c_childCount];
        FunctionChildBase* m_function;

        // TODO: m_xmmParameters
        unsigned m_rxxParameters;

        // TODO: Figure out how to initialize with something like RAX | RCX | RDX | R8 | R9| R10| R11
        static const unsigned c_rxxVolatiles = 0xf07; //1111 0000 0111
    };


    template <typename R, typename P1, typename P2>
    class CallNode2 : public CallNodeBase<R, 2>
    {
    public:
        typedef R (*FunctionPointer)(P1, P2);

        CallNode2(ExpressionTree& tree,
                  Node<FunctionPointer>& function,
                  Node<P1>& p1,
                  Node<P2>& p2);

    private:
        FunctionChild<FunctionPointer> m_f;
        ParameterChild<P1> m_p1;
        ParameterChild<P1> m_p2;
    };


    //*************************************************************************
    //
    // Template definitions for SaveRestoreVolatilesHelper.
    //
    //*************************************************************************
    template <unsigned SIZE>
    void SaveRestoreVolatilesHelper::RecordParameterRegister(Register<SIZE, false> r)
    {
        m_rxxParameters |= (1ul << r.GetId());
    }


    //*************************************************************************
    //
    // Template definitions for CallNodeBase<R, PARAMETERCOUNT>
    //
    //*************************************************************************
    template <typename R, unsigned PARAMETERCOUNT>
    CallNodeBase<R, PARAMETERCOUNT>::CallNodeBase(ExpressionTree& tree)
        : Node(tree)
    {
    }


    template <typename R, unsigned PARAMETERCOUNT>
    ExpressionTree::Storage<R> CallNodeBase<R, PARAMETERCOUNT>::CodeGenValue(ExpressionTree& tree)
    {
        // Evaluate the function pointer and stage each of the parameters in
        // the correct register.
        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
            // TODO: What if there are not enough registers available for EmitStaging()?
            // TODO: Also, how do we unit test the 
            m_children[i]->EmitStaging(tree, *this);
        }

        // Allocate the result register (register 0 for RXX And RMM).
        ExpressionTree::Storage<R>::DirectRegister resultRegister;
        ExpressionTree::Storage<R> result = tree.Direct<R>(resultRegister);

        SaveVolatiles(tree);

        m_function->EmitCall(tree);

        RestoreVolatiles(tree);

        // Free up registers used for function pointer and parameters.
        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
            m_children[i]->Release();
        }

        return result;
    }


    template <typename R, unsigned PARAMETERCOUNT>
    unsigned CallNodeBase<R, PARAMETERCOUNT>::LabelSubtree(bool isLeftChild)
    {
        static unsigned counts[c_childCount];

        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
            // TODO: REVIEW: Pass true or false for isLeftChild?
            // TODO: Need to account for the fact the extra registers may be used
            // when the specific parameter home is different than the one returned
            // duing code generation.
            counts[i] = m_children[i]->LabelSubtree(true);
        }

        // TODO: sort m_children by decreasing register count.

        unsigned count = 0;
        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
            count = (std::max)(count, 1 + counts[i]);
        }

        return count;
    }

    template <typename R, unsigned PARAMETERCOUNT>
    void CallNodeBase<R, PARAMETERCOUNT>::Print() const
    {
        std::cout << "CallNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();

        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
            std::cout << ", ";
            m_children[i]->Print();
        }

        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    //*************************************************************************
    //
    // Template definitions for
    //     CallNodeBase<R, PARAMETERCOUNT>::TypedChild<T>
    //
    //*************************************************************************
    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    CallNodeBase<R, PARAMETERCOUNT>::TypedChild<T>::TypedChild(Node<T>& expression)
        : m_expression(expression)
    {
        m_expression.IncrementParentCount();
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    unsigned CallNodeBase<R, PARAMETERCOUNT>::TypedChild<T>::LabelSubtree(bool isLeftChild)
    {
        return m_expression.LabelSubtree(isLeftChild);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    void CallNodeBase<R, PARAMETERCOUNT>::TypedChild<T>::Release()
    {
        m_storage.Reset();
    }


    //*************************************************************************
    //
    // Template definitions for
    //     CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>
    //
    //*************************************************************************
    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::FunctionChild(Node<F>& expression)
        : TypedChild(expression)
    {
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    void CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::EmitStaging(ExpressionTree& tree,
                                                                        SaveRestoreVolatilesHelper& volatiles)
    {
        m_storage = m_expression.CodeGen(tree);
        if (m_storage.GetStorageClass() != StorageClass::Direct)
        {
            m_storage.ConvertToValue(tree, false);
        }
        volatiles.RecordParameterRegister(m_storage.GetDirectRegister());
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    void CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::EmitCall(ExpressionTree& tree)
    {
        tree.GetCodeGenerator().Emit<OpCode::Call>(m_storage.GetDirectRegister());
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    void CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::Print()
    {
        std::cout << "function(" << m_expression.GetId() << ")";
    }


    //*************************************************************************
    //
    // Template definitions for
    //     CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<F>
    //
    //*************************************************************************
    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<T>::ParameterChild(Node<T>& expression, unsigned position)
        : TypedChild(expression)
    {
        GetParameterRegister(position, m_destination);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    void CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<T>::EmitStaging(ExpressionTree& tree,
                                                                         SaveRestoreVolatilesHelper& volatiles)
    {
        m_storage = m_expression.CodeGen(tree);

        if (m_storage.GetStorageClass() != StorageClass::Direct || m_storage.GetDirectRegister().GetId() != m_destination.GetId())
        {
            // Need to move the expression into the correct register.
            // NOTE: destination will be released in Parameter destructor.
            ExpressionTree::Storage<T> destination = tree.Direct<T>(m_destination);
            CodeGenHelpers::Emit<OpCode::Mov>(tree.GetCodeGenerator(), m_destination, m_storage);
            m_storage = destination;
        }
        volatiles.RecordParameterRegister(m_destination);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    void CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<T>::Print()
    {
        std::cout << "parameter(" << m_expression.GetId() << ")";
    }


    //*************************************************************************
    //
    // Template definitions for CallNode2<R, P1, P2>
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2>
    CallNode2<R, P1, P2>::CallNode2(ExpressionTree& tree,
                                    Node<FunctionPointer>& function,
                                    Node<P1>& p1,
                                    Node<P2>&p2)
        : CallNodeBase(tree),
          m_f(function),
          m_p1(p1, 0),
          m_p2(p2, 1)
    {
        m_function = &m_f;
        m_children[0] = &m_f;
        m_children[1] = &m_p1;
        m_children[2] = &m_p2;
    }
}
