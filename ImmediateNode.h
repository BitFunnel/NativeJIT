#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    class ImmediateNode : public Node<T>
    {
    public:
        ImmediateNode(ExpressionTree& tree, T value);

        T GetValue() const;

        //
        // Overrides of Node methods
        //
        virtual void Print() const override;
        virtual bool IsIndirect() const override;
        virtual bool IsImmediate() const override;


        //
        // Overrrides of ValueNode methods
        //
        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;

    private:
        T m_value;
    };


    //*************************************************************************
    //
    // RXXImmediateValue
    //
    //*************************************************************************
    template <typename T>
    ImmediateNode<T>::ImmediateNode(ExpressionTree& tree,
                         T value)
        : Node(tree),
          m_value(value)
    {
    }


    template <typename T>
    T ImmediateNode<T>::GetValue() const
    {
        return m_value;
    }


    template <typename T>
    void ImmediateNode<T>::Print() const
    {
        std::cout << "ImmediateNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", value = " << m_value;
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T>
    bool ImmediateNode<T>::IsIndirect() const
    {
        return false;
    }


    template <typename T>
    bool ImmediateNode<T>::IsImmediate() const
    {
        return true;
    }


    template <typename T>
    typename Node<T>::RegisterType ImmediateNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        // TODO: Should we ever cache immediate values?
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            RegisterType r;
            tree.AllocateRegister<RegisterType>();
            tree.GetCodeGenerator().Op("mov", r, m_value);
            return r;
        }
    }
}
