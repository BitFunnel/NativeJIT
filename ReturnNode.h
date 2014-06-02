#pragma once

#include "Node.h"


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
        virtual Storage<T> CodeGenValue2(ExpressionTree& tree) override;
        virtual void CompileAsRoot(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<T>& m_child;
    };


    //*************************************************************************
    //
    // Template definitions for ReturnNode
    //
    //*************************************************************************
    template <typename T>
    ReturnNode<T>::ReturnNode(ExpressionTree& tree,
                              Node& child)
        : Node(tree),
          m_child(child)
    {
        child.IncrementParentCount();
    }


    template <typename T>
    typename Storage<T> ReturnNode<T>::CodeGenValue2(ExpressionTree& tree)
    {
        // TODO: Prevent ReturnNode node from ever having a parent and being cached.
        if (IsCached2())
        {
            auto result = GetCache2();
            ReleaseCache2();
            return result;
        }
        else
        {
            return m_child.CodeGenValue2(tree);
        }
    }


    template <typename T>
    void ReturnNode<T>::CompileAsRoot(ExpressionTree& tree)
    {
        Storage<T> s = CodeGenValue2(tree);
        s.ConvertToValue(tree, false);
        auto r = s.GetDirectRegister();

        if (r.GetId() != 0)
        {
            auto dest = RegisterType(0);
            tree.GetCodeGenerator().Op("mov", dest, r);
        }

        tree.GetCodeGenerator().Op("ret");
    }


    template <typename T>
    unsigned ReturnNode<T>::LabelSubtree(bool isLeftChild)
    {
        unsigned child = m_child.LabelSubtree(true);

        SetRegisterCount(child);

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename T>
    void ReturnNode<T>::Print() const
    {
        std::cout << "ReturnNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
