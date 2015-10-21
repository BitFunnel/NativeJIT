#pragma once

#include "Node.h"


namespace NativeJIT
{
    // A node representing a reference to type T.
    template <typename T>
    class ReferenceNode : public Node<T&>
    {
    public:
        // Constructs the reference node from a pointer. Since references are
        // represented as pointers, code generation simply casts the pointer
        // storage to the reference storage.
        ReferenceNode(ExpressionTree& tree, Node<T*>& pointer);

        //
        // Overrides of Node methods
        //

        virtual void Print() const override;
        virtual Storage<T&> CodeGenValue(ExpressionTree& tree) override;

    private:
        Node<T*>& m_pointer;
    };


    template <typename T>
    ReferenceNode<T>::ReferenceNode(ExpressionTree& tree, Node<T*>& pointer)
        : Node(tree),
          m_pointer(pointer)
    {
        m_pointer.IncrementParentCount();
    }


    template <typename T>
    void ReferenceNode<T>::Print() const
    {
        PrintCoreProperties("ReferenceNode");

        std::cout << ", pointer " << m_pointer.GetId();
    }


    template <typename T>
    ExpressionTree::Storage<T&> ReferenceNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // T* and T& have a compatible storage.
        // WARNING: Representation of a reference is implementation-specific.
        // However, (almost?) all compilers implement it as a pointer.
        return Storage<T&>(m_pointer.CodeGen(tree));
    }
}
