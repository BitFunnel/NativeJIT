#pragma once

#include "Node.h"


namespace NativeJIT
{
    // Represents a variable on the stack of the function being compiled.
    // The variable is valid as long as the function it is being compiled for
    // is running. Once the function returns and its stack gets freed, the
    // variable is invalid. This means that StackVariableNode cannot be used as
    // return value from the function.
    template <typename T>
    class StackVariableNode : public Node<T&>
    {
    public:
        StackVariableNode(ExpressionTree& tree);

        //
        // Overrides of Node methods
        //

        virtual void Print() const override;
        virtual Storage<T&> CodeGenValue(ExpressionTree& tree) override;

    private:
        Storage<T> m_stackStorage;
    };


    template <typename T>
    StackVariableNode<T>::StackVariableNode(ExpressionTree& tree)
        : Node(tree)
    {
    }


    template <typename T>
    void StackVariableNode<T>::Print() const
    {
        PrintCoreProperties("StackVariableNode");

        if (!m_stackStorage.IsNull())
        {
            std::cout << ", offset " << m_stackStorage.GetOffset()
                      << " off " << m_stackStorage.GetBaseRegister().GetName();
        }
        else
        {
            std::cout << ", storage not yet assigned";
        }
    }


    template <typename T>
    ExpressionTree::Storage<T&> StackVariableNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // Allocate temporary storage and store it as a class variable to keep
        // it alive. Make a copy which we can give up ownership on.
        //
        // TODO: The storage returned by CodeGenValue() is a pointer to the
        // stack space allocated by m_stackStorage. Thus, m_stackStorage has to
        // be kept alive for the address to remain valid. Currently the space
        // is kept for the whole lifetime of the expression, which may be an issue
        // if something uses many short-lived stack variables. Consider making it
        // possible for one storage to extend the lifetime of another storage
        // (for example, a concept of parent storage of similar).
        m_stackStorage = tree.Temporary<T>();
        auto storageCopy = m_stackStorage;

        // Transfer ownership of the copy to a direct storage of variable's address.
        auto addressOfStorage = Storage<T*>::AddressOfIndirect(std::move(storageCopy));

        // Convert the pointer to a reference and return it.
        return Storage<T&>(addressOfStorage);
    }
}
