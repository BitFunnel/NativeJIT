#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    class ReturnNode : public DirectValue<T>
    {
    public:
        ReturnNode(ExpressionTree& tree, Node<T>& child);

        //
        // Overrides of Node methods.
        //
        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;
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
        : DirectValue(tree),
          m_child(child)
    {
        child.IncrementParentCount();
    }


    template <typename T>
    typename Node<T>::RegisterType ReturnNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // TODO: Prevent ReturnNode node from ever having a parent and being cached.
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            return m_child.CodeGenValue(tree);
        }
    }


    template <typename T>
    void ReturnNode<T>::CompileAsRoot(ExpressionTree& tree)
    {
        RegisterType r = CodeGenValue(tree);

        RegisterType result = r;
        if (r.GetId() != 0)
        {
            result = RegisterType(0);
            tree.GetCodeGenerator().Op("mov", result, r);
        }

        tree.GetCodeGenerator().Op("ret");

        tree.ReleaseRegister(r);
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
