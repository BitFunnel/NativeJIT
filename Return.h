#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    class Return : public DirectValue<T>
    {
    public:
        Return(ExpressionTree& tree, Node<T>& child);

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
    // Template definitions for Return
    //
    //*************************************************************************
    template <typename T>
    Return<T>::Return(ExpressionTree& tree,
                     Node& child)
        : DirectValue(tree),
          m_child(child)
    {
        child.IncrementParentCount();
    }


    template <typename T>
    typename Node<T>::RegisterType Return<T>::CodeGenValue(ExpressionTree& tree)
    {
        // TODO: Prevent Return node from ever having a parent and being cached.
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
    void Return<T>::CompileAsRoot(ExpressionTree& tree)
    {
        RegisterType r = CodeGenValue(tree);

        RegisterType result = r;
        if (r.GetId() != 0)
        {
            result = RegisterType(0);
            tree.GetCodeGenerator().Op("mov", result, r);
        }

        tree.GetCodeGenerator().Op("ret");
    }


    template <typename T>
    unsigned Return<T>::LabelSubtree(bool isLeftChild)
    {
        unsigned child = m_child.LabelSubtree(true);

        SetRegisterCount(child);

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename T>
    void Return<T>::Print() const
    {
        std::cout << "Return id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
