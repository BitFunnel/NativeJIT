#pragma once

#include "NativeJIT/CodeGenHelpers.h"
#include "NativeJIT/Nodes/Node.h"


namespace NativeJIT
{
    template <typename T>
    class ReturnNode : public Node<T>
    {
    public:
        ReturnNode(ExpressionTree& tree, Node<T>& child);

        //
        // Overrides of Node methods.
        //
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;
        virtual void CompileAsRoot(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print(std::ostream& out) const override;

    private:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~ReturnNode();

        Node<T>& m_child;
    };


    //*************************************************************************
    //
    // Template definitions for ReturnNode
    //
    //*************************************************************************
    template <typename T>
    ReturnNode<T>::ReturnNode(ExpressionTree& tree,
                              Node<T>& child)
        : Node<T>(tree),
          m_child(child)
    {
        // There's an implicit parent to the return node: the function it's used by.
        this->IncrementParentCount();
        child.IncrementParentCount();
    }


    template <typename T>
    typename ExpressionTree::Storage<T> ReturnNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        LogThrowAssert(this->GetParentCount() == 1,
                       "Unexpected parent count for the root node: %u",
                       this->GetParentCount());

        return m_child.CodeGen(tree);
    }


    template <typename T>
    void ReturnNode<T>::CompileAsRoot(ExpressionTree& tree)
    {
        ExpressionTree::Storage<T> s = this->CodeGen(tree);

        auto resultRegister = tree.GetResultRegister<T>();

        // Move result into the result register unless already there.
        if (!(s.GetStorageClass() == StorageClass::Direct
              && s.GetDirectRegister().IsSameHardwareRegister(resultRegister)))
        {
            CodeGenHelpers::Emit<OpCode::Mov>(tree.GetCodeGenerator(), resultRegister, s);
        }
    }


    template <typename T>
    unsigned ReturnNode<T>::LabelSubtree(bool /*isLeftChild*/)
    {
        unsigned child = m_child.LabelSubtree(true);

        this->SetRegisterCount(child);

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return this->GetRegisterCount();
    }


    template <typename T>
    void ReturnNode<T>::Print(std::ostream& out) const
    {
        this->PrintCoreProperties(out, "ReturnNode");
    }
}
