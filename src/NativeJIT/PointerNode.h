#pragma once

#include "Node.h"


namespace NativeJIT
{
    // A node representing a pointer to type T.
    template <typename T>
    class PointerNode : public Node<T*>
    {
    public:
        // Constructs the pointer node from a reference. Since references are
        // represented as pointers, code generation simply casts the reference
        // storage to the pointer storage.
        PointerNode(ExpressionTree& tree, Node<T&>& reference);

        //
        // Overrides of Node methods
        //

        virtual void Print() const override;
        virtual Storage<T*> CodeGenValue(ExpressionTree& tree) override;

    private:
        Node<T&>& m_reference;
    };


    template <typename T>
    PointerNode<T>::PointerNode(ExpressionTree& tree, Node<T&>& reference)
        : Node(tree),
          m_reference(reference)
    {
        m_reference.IncrementParentCount();
    }


    template <typename T>
    void PointerNode<T>::Print() const
    {
        PrintCoreProperties("PointerNode");

        std::cout << ", reference " << m_reference.GetId();
    }


    template <typename T>
    ExpressionTree::Storage<T*> PointerNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // T* and T& have a compatible storage.
        // WARNING: Representation of a reference is implementation-specific.
        // However, (almost?) all compilers implement it as a pointer.
        return Storage<T*>(m_reference.CodeGen(tree));
    }
}
