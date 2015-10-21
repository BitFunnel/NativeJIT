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
        m_stackStorage = tree.Temporary<T>();
        auto storageCopy = m_stackStorage;

        // Transfer ownership of the copy to a direct storage of variable's address.
        auto addressOfStorage = Storage<T*>::AddressOfIndirect(std::move(storageCopy));

        // Convert the pointer to a reference and return it.
        return Storage<T&>(addressOfStorage);
    }
}
