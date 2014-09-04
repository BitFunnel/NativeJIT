#pragma once

#include <type_traits>

#include "Node.h"


namespace NativeJIT
{
    template <typename T, typename ENABLE = void>
    class ImmediateNode : public Node<T>
    {
    public:
        ImmediateNode(ExpressionTree& tree, T value)
            : Node(tree),
              m_value(value)
        {
        }


        //
        // Overrides of Node methods
        //
        virtual void Print() const override
        {
            std::cout << "ImmediateNode id=" << GetId();
            std::cout << ", parents = " << GetParentCount();
            std::cout << ", value = " << m_value;
            std::cout << ", ";
            PrintRegisterAndCacheInfo();
        }


        //
        // Overrrides of ValueNode methods
        //
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override
        {
            return tree.Immediate(m_value);
        }

    private:
        T m_value;
    };


    //*************************************************************************
    //
    // Template specializations for ImmediateNode<double>
    //
    //*************************************************************************
    template <typename T>
    class ImmediateNode<T, typename std::enable_if<std::is_floating_point<T>::value>::type> : public Node<T>
    {
    public:
        ImmediateNode(ExpressionTree& tree, T value)
            : Node<T>(tree),
              m_value(value)
        {
            // TODO: Correct offset.
            m_offset = 0;
        }

        //
        // Overrides of Node methods
        //
        virtual void Print() const override
        {
            std::cout << "ImmediateNode id=" << GetId();
            std::cout << ", parents = " << GetParentCount();
            std::cout << ", value = " << m_value;
            std::cout << "  RIP-indirect, ";
            std::cout << ", ";
            PrintRegisterAndCacheInfo();
        }


        //
        // Overrrides of ValueNode methods
        //
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override
        {
            return tree.RIPRelative<T>(m_offset);
        }

    private:
        T m_value;
        __int32 m_offset;
    };
}
