#pragma once

#include <iostream>                 // Accessed by template definition for Print().

#include "CodeGenHelpers.h"
#include "Node.h"                   // Base class.
#include "Temporary/StlAllocator.h" // Used as allocator for vector.

// https://software.intel.com/en-us/articles/introduction-to-x64-assembly

namespace NativeJIT
{
    class ExpressionTree;


    class SaveRestoreVolatilesHelper
    {
    protected:
        SaveRestoreVolatilesHelper(Allocators::IAllocator& allocator);

    public:
        // These methods need to be public for access by
        //     CallNodeBase::FunctionChild
        //     CallNodeBase::ParameterChild

        void SaveVolatiles(ExpressionTree& tree);
        void RestoreVolatiles(ExpressionTree& tree);

        template <unsigned SIZE, bool ISFLOAT>
        void RecordCallRegister(Register<SIZE, ISFLOAT> r, bool isSoleOwner);

    private:
        // Returns the mask for the registers it's necessary to preserve accross
        // the function call that's being made.
        template <bool ISFLOAT>
        unsigned GetRegistersToPreserve(ExpressionTree& tree) const;

        // A bit-mask of registers that are exclusively owned for the function
        // call and thus don't need to be preserved.
        unsigned m_rxxCallExclusiveRegisterMask;
        unsigned m_xmmCallExclusiveRegisterMask;

        template <typename T>
        using AllocatorVector = std::vector<T, Allocators::StlAllocator<T>>;

        // Temporary storage used to preserve volatile registers.
        AllocatorVector<Storage<void*>> m_preservationStorage;
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
        class Child : private NonCopyable
        {
        public:
            // Returns the number of registers needed to evaluate this child for
            // the Sethi-Ullman algorithm.
            virtual unsigned LabelSubtree(bool isLeftChild) = 0;

            // Generates the code to evaluate the expression inside the child.
            virtual void Evaluate(ExpressionTree& tree) = 0;

            // Emits the code to place the child's value into the appropriate
            // register or stack location depending on the type and position
            // of the child.
            // The staging must not modify any other registers reserved for the
            // function call by the calling convention, so the child must already
            // be evaluated before it can be staged.
            virtual void EmitStaging(ExpressionTree& tree, SaveRestoreVolatilesHelper& volatiles) = 0;

            // Releases any registers used during the evaluation of the child
            // expression in Evaluate().
            virtual void Release() = 0;

            // Prints the contents of the child to standard output for debugging.
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
            // Pins the storage register so that it cannot be spilled until
            // the Release() call.
            void PinStorageRegister();

            Node<T>& m_expression;
            ExpressionTree::Storage<T> m_storage;
            ReferenceCounter m_registerPin;
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
            virtual void Evaluate(ExpressionTree& tree) override;
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
            FunctionChild(Node<T>& expression,
                          typename Storage<R>::DirectRegister resultRegister);

            //
            // Overrides of Child methods.
            //
            virtual void Evaluate(ExpressionTree& tree) override;
            virtual void EmitStaging(ExpressionTree& tree,
                                     SaveRestoreVolatilesHelper& volatiles) override;

            //
            // Overrides of FunctionChildBase methods.
            //
            virtual void EmitCall(ExpressionTree& tree) override;
            virtual void Print() override;

        private:
            typename Storage<R>::DirectRegister m_resultRegister;
        };

        // One child for each parameter plus one for the function pointer.
        static const unsigned c_childCount = PARAMETERCOUNT + 1;
        Child* m_children[c_childCount];

        // Pointer to function's two base classes.
        FunctionChildBase* m_functionBase;
        Child* m_functionChild;
    };


    template <typename R, typename P1 = void, typename P2 = void, typename P3 = void, typename P4 = void>
    class CallNode;


    template <typename R>
    class CallNode<R> : public CallNodeBase<R, 0>
    {
    public:
        typedef R (*FunctionPointer)();

        CallNode(ExpressionTree& tree,
                 Node<FunctionPointer>& function);

    private:
        FunctionChild<FunctionPointer> m_f;
    };


    template <typename R, typename P1>
    class CallNode<R, P1> : public CallNodeBase<R, 1>
    {
    public:
        typedef R (*FunctionPointer)(P1);

        CallNode(ExpressionTree& tree,
                 Node<FunctionPointer>& function,
                 Node<P1>& p1);

    private:
        FunctionChild<FunctionPointer> m_f;
        ParameterChild<P1> m_p1;
    };


    template <typename R, typename P1, typename P2>
    class CallNode<R, P1, P2> : public CallNodeBase<R, 2>
    {
    public:
        typedef R (*FunctionPointer)(P1, P2);

        CallNode(ExpressionTree& tree,
                 Node<FunctionPointer>& function,
                 Node<P1>& p1,
                 Node<P2>& p2);

    private:
        FunctionChild<FunctionPointer> m_f;
        ParameterChild<P1> m_p1;
        ParameterChild<P2> m_p2;
    };


    template <typename R, typename P1, typename P2, typename P3>
    class CallNode<R, P1, P2, P3> : public CallNodeBase<R, 3>
    {
    public:
        typedef R (*FunctionPointer)(P1, P2, P3);

        CallNode(ExpressionTree& tree,
                 Node<FunctionPointer>& function,
                 Node<P1>& p1,
                 Node<P2>& p2,
                 Node<P3>& p3);

    private:
        FunctionChild<FunctionPointer> m_f;
        ParameterChild<P1> m_p1;
        ParameterChild<P2> m_p2;
        ParameterChild<P3> m_p3;
    };


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    class CallNode : public CallNodeBase<R, 4>
    {
    public:
        typedef R (*FunctionPointer)(P1, P2, P3, P4);

        CallNode(ExpressionTree& tree,
                 Node<FunctionPointer>& function,
                 Node<P1>& p1,
                 Node<P2>& p2,
                 Node<P3>& p3,
                 Node<P4>& p4);

    private:
        FunctionChild<FunctionPointer> m_f;
        ParameterChild<P1> m_p1;
        ParameterChild<P2> m_p2;
        ParameterChild<P3> m_p3;
        ParameterChild<P4> m_p4;
    };


    //*************************************************************************
    //
    // Template definitions for SaveRestoreVolatilesHelper.
    //
    //*************************************************************************
    template <unsigned SIZE, bool ISFLOAT>
    void SaveRestoreVolatilesHelper::RecordCallRegister(Register<SIZE, ISFLOAT> r, bool isSoleOwner)
    {
        if (isSoleOwner)
        {
            BitOp::SetBit(ISFLOAT ? &m_xmmCallExclusiveRegisterMask : &m_rxxCallExclusiveRegisterMask,
                          r.GetId());
        }
    }


    //*************************************************************************
    //
    // Template definitions for CallNodeBase<R, PARAMETERCOUNT>
    //
    //*************************************************************************
    template <typename R, unsigned PARAMETERCOUNT>
    CallNodeBase<R, PARAMETERCOUNT>::CallNodeBase(ExpressionTree& tree)
        : Node(tree),
          SaveRestoreVolatilesHelper(tree.GetAllocator())
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        tree.ReportFunctionCallNode(PARAMETERCOUNT);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    ExpressionTree::Storage<R> CallNodeBase<R, PARAMETERCOUNT>::CodeGenValue(ExpressionTree& tree)
    {
        // Make sure that the result register is not pinned at this point.
        const auto resultRegister = tree.GetResultRegister<R>();
        Assert(!tree.IsPinned(resultRegister), "The result register must not be pinned before the call");

        // Evaluate the function pointer and each parameter.
        for (Child* child : m_children)
        {
            child->Evaluate(tree);
        }

        for (Child* child : m_children)
        {
            // Stage the parameters first since they need to be placed into
            // fixed registers.
            if (child != m_functionChild)
            {
                child->EmitStaging(tree, *this);
            }
        }

        // Stage the function pointer into a register last since it can be staged
        // staged into any register.
        m_functionChild->EmitStaging(tree, *this);

        // If the result register is still not pinned (and thus unused by the
        // call), enforce ownership over it.
        if (!tree.IsPinned(resultRegister))
        {
            // Release the register right away by not keeping the Storage.
            tree.Direct<R>(resultRegister);
            RecordCallRegister(resultRegister, true);
        }

        SaveVolatiles(tree);
        m_functionBase->EmitCall(tree);
        RestoreVolatiles(tree);

        // Free up registers used for function pointer and parameters.
        for (Child* child : m_children)
        {
            child->Release();
        }

        // At this point, the result register was either used by a parameter or
        // the function pointer and then released, or it was empty after the
        // explicit bump further above so the following call will bump nothing.
        return tree.Direct<R>(resultRegister);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    unsigned CallNodeBase<R, PARAMETERCOUNT>::LabelSubtree(bool /* isLeftChild */)
    {
        static unsigned counts[c_childCount];

        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
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
        PrintCoreProperties("CallNode");

        for (unsigned i = 0 ; i < c_childCount; ++i)
        {
            std::cout << ", ";
            m_children[i]->Print();
        }
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
        // Release the register pin and reset the storage to release the register.
        m_registerPin.Reset();
        m_storage.Reset();
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    void CallNodeBase<R, PARAMETERCOUNT>::TypedChild<T>::PinStorageRegister()
    {
        Assert(!m_storage.IsNull(), "Storage must be initialized");

        if (m_storage.GetStorageClass() == StorageClass::Direct)
        {
            m_registerPin = m_storage.GetPin();
        }
    }


    //*************************************************************************
    //
    // Template definitions for
    //     CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>
    //
    //*************************************************************************
    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::FunctionChild(
        Node<F>& expression,
        typename Storage<R>::DirectRegister resultRegister)
        : TypedChild(expression),
          m_resultRegister(resultRegister)
    {
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    void CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::Evaluate(ExpressionTree& tree)
    {
        m_storage = m_expression.CodeGen(tree);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename F>
    void CallNodeBase<R, PARAMETERCOUNT>::FunctionChild<F>::EmitStaging(ExpressionTree& /* tree */,
                                                                        SaveRestoreVolatilesHelper& volatiles)
    {
        // The CALL instruction requires a direct register, ensure that's the case.
        // Convert for modification to ensure that the register doesn't need
        // to be preserved accross the call.
        if (m_storage.GetStorageClass() != StorageClass::Direct)
        {
            m_storage.ConvertToDirect(true);
        }

        // If function pointer happens to be in the result register and there
        // are other owners, they must be spilled out of the register. Otherwise,
        // the attempt to restore the contents into the result register after
        // the call would overwrite the returned result.
        if (!m_storage.IsSoleDataOwner()
            && m_storage.GetDirectRegister().IsSameHardwareRegister(m_resultRegister))
        {
            m_storage.TakeSoleOwnershipOfDirect();
        }

        volatiles.RecordCallRegister(m_storage.GetDirectRegister(),
                                     m_storage.IsSoleDataOwner());

        // Make sure that nothing takes away this register.
        // NOTE: This depends on the fact that the function pointer is staged
        // last to ensure that a fixked parameter register is not picked.
        PinStorageRegister();
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
    void CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<T>::Evaluate(ExpressionTree& tree)
    {
        m_storage = m_expression.CodeGen(tree);
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    void CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<T>::EmitStaging(ExpressionTree& tree,
                                                                         SaveRestoreVolatilesHelper& volatiles)
    {
        if (m_storage.GetStorageClass() != StorageClass::Direct
            || !m_storage.GetDirectRegister().IsSameHardwareRegister(m_destination))
        {
            ExpressionTree::Storage<T> regStorage = tree.Direct<T>(m_destination);
            CodeGenHelpers::Emit<OpCode::Mov>(tree.GetCodeGenerator(), m_destination, m_storage);
            m_storage = regStorage;
        }

        // TODO: There's room for optimization if the data was already in the
        // correct register and shared. If there are some free non-volatile
        // registers, it would be better to enforce sole ownership of m_storage
        // by spilling to such register (the callee may not be using it and may
        // not need to push it to/pop it from the stack). Currently, in such
        // case the register will always get pushed/popped by SaveRestoreVolatilesHelper.

        volatiles.RecordCallRegister(m_storage.GetDirectRegister(),
                                     m_storage.IsSoleDataOwner());

        // The parameter needs to remain in the specified register.
        PinStorageRegister();
    }


    template <typename R, unsigned PARAMETERCOUNT>
    template <typename T>
    void CallNodeBase<R, PARAMETERCOUNT>::ParameterChild<T>::Print()
    {
        std::cout << "parameter(" << m_expression.GetId() << ")";
    }


    //*************************************************************************
    //
    // Template definitions for CallNode<R, P1, P2>
    //
    //*************************************************************************
    template <typename R>
    CallNode<R>::CallNode(ExpressionTree& tree,
                          Node<FunctionPointer>& function)
        : CallNodeBase(tree),
          m_f(function, tree.GetResultRegister<R>())
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");

        m_functionBase = &m_f;
        m_functionChild = &m_f;
        m_children[0] = m_functionChild;
    }


    template <typename R, typename P1>
    CallNode<R, P1>::CallNode(ExpressionTree& tree,
                                  Node<FunctionPointer>& function,
                                  Node<P1>& p1)
        : CallNodeBase(tree),
          m_f(function, tree.GetResultRegister<R>()),
          m_p1(p1, 0)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");

        m_functionBase = &m_f;
        m_functionChild = &m_f;
        m_children[0] = m_functionChild;
        m_children[1] = &m_p1;
    }


    template <typename R, typename P1, typename P2>
    CallNode<R, P1, P2>::CallNode(ExpressionTree& tree,
                                  Node<FunctionPointer>& function,
                                  Node<P1>& p1,
                                  Node<P2>& p2)
        : CallNodeBase(tree),
          m_f(function, tree.GetResultRegister<R>()),
          m_p1(p1, 0),
          m_p2(p2, 1)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");
        static_assert(std::is_pod<P2>::value, "P2 must be a POD type.");

        m_functionBase = &m_f;
        m_functionChild = &m_f;
        m_children[0] = m_functionChild;
        m_children[1] = &m_p1;
        m_children[2] = &m_p2;
    }


    template <typename R, typename P1, typename P2, typename P3>
    CallNode<R, P1, P2, P3>::CallNode(ExpressionTree& tree,
                                      Node<FunctionPointer>& function,
                                      Node<P1>& p1,
                                      Node<P2>& p2,
                                      Node<P3>& p3)
        : CallNodeBase(tree),
          m_f(function, tree.GetResultRegister<R>()),
          m_p1(p1, 0),
          m_p2(p2, 1),
          m_p3(p3, 2)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");
        static_assert(std::is_pod<P2>::value, "P2 must be a POD type.");
        static_assert(std::is_pod<P3>::value, "P3 must be a POD type.");

        m_functionBase = &m_f;
        m_functionChild = &m_f;
        m_children[0] = m_functionChild;
        m_children[1] = &m_p1;
        m_children[2] = &m_p2;
        m_children[3] = &m_p3;
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    CallNode<R, P1, P2, P3, P4>::CallNode(ExpressionTree& tree,
                                          Node<FunctionPointer>& function,
                                          Node<P1>& p1,
                                          Node<P2>& p2,
                                          Node<P3>& p3,
                                          Node<P4>& p4)
        : CallNodeBase(tree),
          m_f(function, tree.GetResultRegister<R>()),
          m_p1(p1, 0),
          m_p2(p2, 1),
          m_p3(p3, 2),
          m_p4(p4, 3)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");
        static_assert(std::is_pod<P2>::value, "P2 must be a POD type.");
        static_assert(std::is_pod<P3>::value, "P3 must be a POD type.");
        static_assert(std::is_pod<P4>::value, "P4 must be a POD type.");

        m_functionBase = &m_f;
        m_functionChild = &m_f;
        m_children[0] = m_functionChild;
        m_children[1] = &m_p1;
        m_children[2] = &m_p2;
        m_children[3] = &m_p3;
        m_children[4] = &m_p4;
    }
}
