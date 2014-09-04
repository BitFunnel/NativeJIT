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
    class RIPRelativeImmediate
    {
    public:
        virtual void EmitStaticData(ExpressionTree& tree) = 0;
    };


    template <typename T>
    class ImmediateNode<T, typename std::enable_if<std::is_floating_point<T>::value>::type> : public Node<T>, public RIPRelativeImmediate
    {
    public:
        ImmediateNode(ExpressionTree& tree, T value)
            : Node<T>(tree),
              m_value(value)
        {
            tree.AddRIPRelative(*this);

            // m_offset will be initialized with the correct value during pass0
            // of compilation in the call to EmitStaticData().
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

        //
        // Overrrides of RIPRelativeImmediate methods
        //
        virtual void EmitStaticData(ExpressionTree& tree) override
        {
            auto & code = tree.GetCodeGenerator();
            code.AdvanceToAlignment<T>();
            m_offset = code.CurrentPosition();
            code.EmitFloatingPoint(m_value);
        }

    private:
        T m_value;
        __int32 m_offset;
    };
}
